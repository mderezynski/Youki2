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

#include <sigc++/trackable.h>
#include <sigc++/adaptors/bind.h>
#include <glib/gatomic.h>
#include <glib.h>	// message macros
#include <glibmm/thread.h>
#include "sigx/tunnel_validity_tracker.h"
#include "sigx/bad_dispatcher.h"
#include "sigx/tunnel_functor.h"
#include "__sigx_pchfence__.h"
#include <algorithm> // std::for_each
#include "sigx/validity_trackable.h"

using namespace std;


namespace sigx
{

	namespace internal
	{

validity_trackable::validity_trackable(const shared_dispatchable& _A_disp): 
	m_refcount(), 
	m_tunnel_refcount(), 
	m_valid(true), 
	m_connections(), 
	m_disp(_A_disp), 
	m_dispatcher_change_is_cleanup(false), 
	m_creator_thread(Glib::Thread::self())
{}


	} // namespace internal


tunnel_validity_tracker::tunnel_validity_tracker(const shared_dispatchable& _A_disp): 
	m_info(new internal::validity_trackable(_A_disp))
{
	m_info->m_refcount = 1;
	m_info->m_tunnel_refcount = 1;
}

tunnel_validity_tracker::tunnel_validity_tracker(const tunnel_validity_tracker& other): 
	m_info(other.m_info)
{
	g_atomic_int_inc(&m_info->m_refcount);
	g_atomic_int_inc(&m_info->m_tunnel_refcount);
}

tunnel_validity_tracker& tunnel_validity_tracker::operator =(const tunnel_validity_tracker& other)
{
	// destroy this first
	this->~tunnel_validity_tracker();

	m_info = other.m_info;
	g_atomic_int_inc(&m_info->m_refcount);
	g_atomic_int_inc(&m_info->m_tunnel_refcount);
	
	return *this;
}

tunnel_validity_tracker::~tunnel_validity_tracker()
{
	// test whether last tunnel functor died
	if (TRUE == g_atomic_int_dec_and_test(&m_info->m_tunnel_refcount))
	{
		// test whether we are responsible to clean up the validity tracking stuff
		if (1 == g_atomic_int_get(&m_info->m_dispatcher_change_is_cleanup))
		{
			if (m_info->m_creator_thread == Glib::Thread::self())
			{	// short cut - no need to tunnel a message because 
				// validity trackable was created with the same 
				// thread executing the dtor;
				// 
				// this can happen e.g. if threadA has connected a tunnel functor to a signal 
				// of threadB, threadB emits this very signal, threadA executes the emitted signal and ends threadB.
				// threadB might have emitted the signal again in the meanwhile and threadA has a message pending 
				// in the dispatcher queue. threadB might finish its execution, destroying threadA's connection to 
				// threadB's signal. Because threadA has still the message pending however, the validity trackable 
				// is still alive with the message in the queue.
				// Then threadA executes the pending message and the last validity trackable will go out of scope 
				// along with the processed message; because the validity trackable and the dispatcher belong to the 
				// same thread as the executing one we can just call on_last_functor directly
				tunnel_validity_tracker::on_last_functor(*this);
			}
			else
			{
				// little hack to ensure that the dtor doesn't trigger this message again
				m_info->m_tunnel_refcount = 1;

				// send a maintenance message to the client thread;
				// note that the tunnel functor will make a copy of this (even we are 
				// currently in the dtor) and thus
				// ensures that still everything will be valid;
				// 
				// attention: never use any sigc::trackableS here, otherwise the maintenance message
				// would be triggered without end! (open_tunnel() creates again a validity trackable
				// which goes out of scope after the message is dispatched and comes here again...)
				sigc::exception_catch(
					open_tunnel_with(&tunnel_validity_tracker::on_last_functor, m_info->m_disp), 
					bad_dispatcher_catcher<void>()
				)(*this);
			}
		}
	}

	if (TRUE == g_atomic_int_dec_and_test(&m_info->m_refcount))
		delete m_info;
	m_info = 0;
}

void tunnel_validity_tracker::on_last_functor(tunnel_validity_tracker& data)
{
	// because the last tunnel functor died we can just clear the connections container because 
	// there are obviously no connections open anymore
	data.m_info->m_connections.clear();
	cleanup(data.m_info, true);
}

//static 
void tunnel_validity_tracker::cleanup(void* data, bool cleanup_dispatcher)
{
	internal::validity_trackable* const vt = (internal::validity_trackable*) data;
	g_assert(vt->m_creator_thread == Glib::Thread::self());

	if (vt->m_valid)
	{
		// deactivate validity tracking
		// (dtor doesn't need to send a cleanup message anymore)
		g_atomic_int_set(&vt->m_dispatcher_change_is_cleanup, 0);
		// invalidate callback
		vt->m_valid = false;
		// callback is invalid, disconnect slots containing 
		// tunnel functors from a signal eventually;
		tunnel_validity_tracker::clear_connections(vt);
		if (cleanup_dispatcher)
		{
			// notify the associated dispatchable that the tunnel is not valid 
			// anymore;
			// if cleanup_dispatcher = false then cleanup was called from notify_dispatcher_change()
			// which again was called by the dispatchable itself, just having removed us from its list
			vt->m_disp.remove_dispatcher_change_notify_callback(data);
		}
		// remove the validity trackable from all sigc::trackables
		for_each(vt->m_trackables.begin(), vt->m_trackables.end(), 
			sigc::bind(sigc::mem_fun(&sigc::trackable::remove_destroy_notify_callback), vt)
		);
		g_atomic_int_add(&vt->m_refcount, - (gint) vt->m_trackables.size());
		vt->m_trackables.clear();

		if (TRUE == g_atomic_int_dec_and_test(&vt->m_refcount))
			delete vt;
	}
}

//static 
void* tunnel_validity_tracker::notify_from_trackable(void* data)
{
	cleanup(data, true);
	return 0;
}

//static 
void tunnel_validity_tracker::notify_dispatcher_change(void* data)
{
	cleanup(data, false);
}

void tunnel_validity_tracker::clear_connections(internal::validity_trackable* vt)
{
	g_assert(vt->m_creator_thread == Glib::Thread::self());

	// disconnect each sigx::connection_wrapper and ignore bad_dispatcher exceptions;
	// they would be thrown if the thread that owns the signal to which this connection
	// is related has ended and with it the signal was destroyed;
	// thus we can't disconnect anymore and there is no need to
	for_each(vt->m_connections.begin(), vt->m_connections.end(), 
		sigc::exception_catch(sigc::mem_fun(&connection_wrapper::disconnect), bad_dispatcher_catcher<void>())
	);
	vt->m_connections.clear();
}

void tunnel_validity_tracker::do_bind_to_trackable(const sigc::trackable* t) const
{
	g_assert(m_info->m_creator_thread == Glib::Thread::self());

	t->add_destroy_notify_callback(m_info, &tunnel_validity_tracker::notify_from_trackable);
	m_info->m_trackables.push_back(t);
	// increase the reference count for the trackable
	++m_info->m_refcount;
}

void tunnel_validity_tracker::add_connection(const connection_wrapper& c)
{
	g_assert(m_info->m_creator_thread == Glib::Thread::self());

	m_info->m_connections.push_back(c);
}

bool tunnel_validity_tracker::is_valid() const
{
	//g_assert(m_info->dispatcher()->m_creator_thread == Glib::Thread::self());

	return m_info->m_valid;
}

void tunnel_validity_tracker::activate()
{
	g_assert(m_info->m_creator_thread == Glib::Thread::self());
	g_assert(m_info->m_disp.dispatcher());

	// add the validity_trackable to the dispatchable;
	// once for the destruction of the dispatcher
	m_info->m_disp.add_dispatcher_change_notify_callback(m_info, &tunnel_validity_tracker::notify_dispatcher_change);
	// validity_trackable added to dispatchable
	++m_info->m_refcount;
	m_info->m_dispatcher_change_is_cleanup = 1;
}



} // namespace sigx
