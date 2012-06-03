/* Copyright (C) 2005 Tim Mayberry and Klaus Triendl
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free 
 * Software Foundation, 51 Franklin Street, Fifth Floor, 
 * Boston, MA 02110-1301, USA.
 */

#include <glib/gatomic.h> // atomic operations
#include <glibmm/thread.h>
#include "sigx/bad_caller.h"
#include "sigx/bad_sync_call.h"
#include "sigx/dispatcher.h"
#include "sigx/tunnel_context_base.h"
#include "__sigx_pchfence__.h"
#include "sigx/lock_acquirer.h"

using namespace std;


namespace sigx
{

//statics
bool dispatcher::deadlock_detection = false;
dld::lockable_sync_messages_type dispatcher::thread_paired_sync_messages;


dispatcher::dispatcher(): 
	m_tunnel_contexts(), 
	m_contexts_count(0), 
	m_creator_thread(Glib::Thread::self())
{}

dispatcher::~dispatcher()
{
	// might throw bad_caller
	test_calling_thread();
}

void dispatcher::test_calling_thread()
{
	// must be executed in the context of the owner thread, i.e. the thread that
	// created this glib_dispatcher instance;
	// note that the Glib::dispatcher requires this, too;
	if (m_creator_thread != Glib::Thread::self())
		throw bad_caller();
}

void dispatcher::send(tunnel_context_base* context)
{
	// deadlock detection
	if (context->is_sync() && dispatcher::deadlock_detection)
	{
		Glib::Thread* self = Glib::Thread::self();
		if (self == m_creator_thread)
			// sending a sync message to ourselves will block
			throw bad_sync_call();
		else
			increase_sync_messages(dld::make_thread_pair(self, m_creator_thread));
	}

	g_atomic_int_inc(&m_contexts_count);

	writelock_acquirer<lockable_tunnel_contexts> locker(m_tunnel_contexts);
	access_acquiree(locker).push(context);
}

gint dispatcher::queued_contexts() const
{
	return g_atomic_int_get(&m_contexts_count);
}

bool dispatcher::process_next()
{
	bool more_work = false;
	tunnel_context_base* context = 0;

	// scope for locker
	{
		writelock_acquirer<lockable_tunnel_contexts> locker(m_tunnel_contexts);
		context_container_type& contexts = access_acquiree(locker);
		if (!contexts.empty())
		{
			context = contexts.front();
			contexts.pop();
		}
	}
	
	if (context)
	{
		// XXX catch exceptions??

		more_work = (g_atomic_int_dec_and_test(&m_contexts_count));
		const threadhandle_type context_creator_thread = context->creator_thread();
		const bool detect_deadlock = dispatcher::deadlock_detection && context->is_sync();
		try
		{
			if (context->is_valid())
				context->invoke();
		}
		catch (...)
		{
			// must decrease the sync message counter
			if (detect_deadlock)
				decrease_sync_messages(dld::make_thread_pair(context_creator_thread, m_creator_thread));

			throw;
		}
		
		// must decrease the sync message counter
		if (detect_deadlock)
			decrease_sync_messages(dld::make_thread_pair(context_creator_thread, m_creator_thread));
	}
	
	return more_work;
}


//static 
void dispatcher::increase_sync_messages(const dld::thread_pair_type& threadpair)
{
	// Glib::Thread::self() is the creator of the tunnel context
	const dld::thread_pair_type& key = threadpair;
	// a thread can't send a synchronous message to itself
	if (key.first == key.second)
		throw bad_sync_call();

	writelock_acquirer<dld::lockable_sync_messages_type> locker(thread_paired_sync_messages);
	dld::sync_messages_type& sync_messages = access_acquiree(locker);

	const pair<dld::sync_messages_type::iterator, bool>& pairIt = 
		sync_messages.insert(make_pair(key, dld::syncmessages_counter(key.first)));
	dld::syncmessages_counter& counter = pairIt.first->second;

	// are there already synchronous messages pending from the thread we want to send
	// a message?
	if (counter)
		throw bad_sync_call();

	++counter;
}

//static 
void dispatcher::decrease_sync_messages(const dld::thread_pair_type& threadpair)
{
	writelock_acquirer<dld::lockable_sync_messages_type> locker(thread_paired_sync_messages);
	dld::sync_messages_type& sync_messages = access_acquiree(locker);

	const dld::thread_pair_type& key = threadpair;
	const pair<dld::sync_messages_type::iterator, bool>& pairIt = 
		sync_messages.insert(make_pair(key, dld::syncmessages_counter(key.first)));
	dld::syncmessages_counter& counter = pairIt.first->second;

	--counter;
}


	namespace dld
	{

thread_pair_type make_thread_pair(threadhandle_type threadA, threadhandle_type threadB)
{
	if (threadA < threadB)
		return std::make_pair(threadA, threadB);
	else
		return std::make_pair(threadB, threadA);
}


syncmessages_counter::syncmessages_counter(const threadhandle_type& threadA): 
	m_threadA(threadA), m_countThreadA(), m_countThreadB()
{}

syncmessages_counter& syncmessages_counter::operator ++()
{
	int& count = 
		(Glib::Thread::self() == m_threadA) ? m_countThreadB : m_countThreadA;
	++count;
	
	return *this;
}

syncmessages_counter& syncmessages_counter::operator --()
{
	int& count = 
		(Glib::Thread::self() == m_threadA) ? m_countThreadA : m_countThreadB;
	--count;
	
	return *this;
}

syncmessages_counter::operator bool() const
{
	const int& count = 
		(Glib::Thread::self() == m_threadA) ? m_countThreadA : m_countThreadB;
	return (count != 0);
}


	} // namespace dld


} // namespace sigx
