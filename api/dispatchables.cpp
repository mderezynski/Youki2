/*
 * Copyright 2008 Klaus Triendl
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free 
 * Software Foundation, 51 Franklin Street, Fifth Floor, 
 * Boston, MA 02110-1301, USA.
*/

#include <glib.h>	// message macros, atomic operations
#include <glibmm/refptr.h>
#include <glibmm/main.h>
#include "sigx/dispatchable.h"
#include "sigx/tunnel_validity_tracker.h"
#include "sigx/auto_dispatchable.h"
#include "sigx/glib_auto_dispatchable.h"
#include "sigx/manual_dispatchable.h"
#include "sigx/shared_dispatchable.h"
#include "sigx/glib_dispatcher.h"
#include "__sigx_pchfence__.h"
#include <memory>	// std::auto_ptr
#include "sigx/lock_acquirer.h"
#include "sigx/glib_lockables.h"

using namespace std;


namespace sigx
{

dispatchable::dispatchable(): 
	noncopyable(), 
	m_shared_count(), 
	m_dispatcher_change_callback_list(), 
	m_disp_ptr()
{
	std::auto_ptr<int> p1(new int(1));
	std::auto_ptr<callback_list_ptr_type> p2(new callback_list_ptr_type(0));
	std::auto_ptr<internal::rwlockable_dispatcher_ptr> p3(new internal::rwlockable_dispatcher_ptr(0));
	m_shared_count = p1.release();
	m_dispatcher_change_callback_list = p2.release();
	m_disp_ptr = p3.release();
}
	
dispatchable::~dispatchable() throw()
{
	release();
}

dispatchable::dispatchable(const dispatchable& other) throw(): 
	noncopyable(), 
	m_shared_count(other.m_shared_count), 
	m_dispatcher_change_callback_list(other.m_dispatcher_change_callback_list), 
	m_disp_ptr(other.m_disp_ptr)
{
	g_atomic_int_inc(m_shared_count);
}

dispatchable& dispatchable::operator =(const dispatchable& other) throw()
{
	if (this == &other)
		return *this;

	release();

	m_shared_count = other.m_shared_count;
	m_disp_ptr = other.m_disp_ptr;
	m_dispatcher_change_callback_list = other.m_dispatcher_change_callback_list;

	g_atomic_int_inc(m_shared_count);
	return *this;
}

void dispatchable::release() throw()
{
	if (g_atomic_int_dec_and_test(m_shared_count))
	{
		delete m_shared_count;
		delete m_disp_ptr;
		// only free the lockable, not the callback list itself;
		// this is the responsibility of derived dispatchables.
		g_assert(0 == *m_dispatcher_change_callback_list);
		delete m_dispatcher_change_callback_list;
		m_shared_count = 0;
		m_disp_ptr = 0;
		m_dispatcher_change_callback_list = 0;
	}
}

void dispatchable::add_dispatcher_change_notify_callback(void* data, func_dispatcher_change_notify func) const
{
	callback_list_ptr_type& callbacklist = *m_dispatcher_change_callback_list;
	if (!callbacklist)
		callbacklist = new callback_list_type;
	callbacklist->push_back(make_pair(data, func));
}

void dispatchable::remove_dispatcher_change_notify_callback(void* data) const
{
	callback_list_ptr_type& callbacklist = *m_dispatcher_change_callback_list;
	const callback_list_type::iterator itEnd = callbacklist->end();
	for (callback_list_type::iterator it = callbacklist->begin(); it != itEnd; /**/)
	{
		if (it->first == data)
			callbacklist->erase(it++);
		else
			++it;
	}
}

void dispatchable::invalidate_tunnels()
{
	callback_list_ptr_type& callbacklist = *m_dispatcher_change_callback_list;
	if (callbacklist)
	{
		const callback_list_type::iterator itEnd = callbacklist->end();
		for (callback_list_type::iterator it = callbacklist->begin(); it != itEnd; ++it)
			it->second(it->first);

		delete callbacklist;
		callbacklist = 0;
	}
}




auto_dispatchable::auto_dispatchable(dispatcher_ptr disp): 
	dispatchable()
{
	// the auto_dispatchable sets up the dispatcher immediately in its ctor;
	// no need to lock the dispatcher pointer, invoke() is threadsafe
	m_disp_ptr->access_nonvolatile() = disp;
}

auto_dispatchable::~auto_dispatchable()
{
	// scope for locker
	{
		// it's time to set the pointer to the dispatcher to 0
		writelock_acquirer<internal::rwlockable_dispatcher_ptr> locker(*m_disp_ptr);
		dispatcher_ptr& disp = access_acquiree(locker);
		delete disp;
		disp = 0;
	}
	
	invalidate_tunnels();
	
	// note that m_disp_ptr might still be referenced by others;
	// this probably means that some thread still has some
	// connections to a signal in this dispatchable's realm;
	// 
	// We don't care here: tunnels connected to a signal will be disconnected 
	// (invalidate_tunnels() notified the validity 
	// trackable about the dispatcher change), messages to be sent will throw 
	// a bad_dispatcher exception
}



glib_auto_dispatchable::glib_auto_dispatchable(): 
	auto_dispatchable(new glib_dispatcher)
{}

glib_auto_dispatchable::glib_auto_dispatchable(const Glib::RefPtr<Glib::MainContext>& context): 
	auto_dispatchable(new glib_dispatcher(context))
{}



manual_dispatchable::manual_dispatchable():
	dispatchable()
{}

manual_dispatchable::~manual_dispatchable()
{
	invalidate_tunnels();
	// Dispatcher should have been set to 0 already
	// when the thread ends, not when the dispatchable is destroyed
	//set_dispatcher(0);
	g_assert(dispatcher() == 0);

	// m_disp_ptr might still be referenced by others;
	// this probably means that some thread still has some
	// connections to a signal in this dispatchable's realm;
}

void manual_dispatchable::set_dispatcher(dispatcher_ptr pdisp)
{
	// scope for locker
	{
		writelock_acquirer<internal::rwlockable_dispatcher_ptr> locker(*m_disp_ptr);
		dispatcher_ptr& disp = access_acquiree(locker);
		disp = pdisp;
	}

	// this will call back to all validity trackables invalidating their tunnel
	// functors operating on that dispatchable
	invalidate_tunnels();
}




shared_dispatchable::shared_dispatchable(): 
	dispatchable()
{}

shared_dispatchable::shared_dispatchable(const dispatchable& d) throw(): 
	dispatchable(d)
{}

shared_dispatchable& shared_dispatchable::operator =(const dispatchable& d) throw()
{
	dispatchable::operator =(d);
	return *this;
}


} // namespace sigx
