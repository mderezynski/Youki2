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

#include "sigx/tunnel_context_base.h"
#include "sigx/dispatcher.h"
#include "sigx/bad_dispatcher.h"
#include "__sigx_pchfence__.h"
//#include "sigx/lock_acquirer.h"
//#include "sigx/glib_lockables.h"
#include "sigx/locking_dispatcher_ptr.h"


namespace sigx
{

tunnel_context_base::tunnel_context_base(const shared_dispatchable& _A_disp, const tunnel_validity_tracker& _A_validity_tracker, bool sync /*= false*/): 
	m_validity_tracker(_A_validity_tracker), 
	m_disp(_A_disp), 
	m_sync(sync), 
	m_creator_thread(Glib::Thread::self())
{}

//virtual 
tunnel_context_base::~tunnel_context_base()
{}

void tunnel_context_base::dispatch_me()
{
	shared_dispatchable::DispatcherPtr dispatcher(m_disp);
	if (!dispatcher)
		throw bad_dispatcher();

	dispatcher->send(this);
}


sync_tunnel_context_base::sync_tunnel_context_base(const shared_dispatchable& _A_disp, const tunnel_validity_tracker& _A_validity_tracker): 
	tunnel_context_base(_A_disp, _A_validity_tracker, true), 
	m_cond(), 
	m_mutex()
{}

void sync_tunnel_context_base::dispatch_me()
{
	Glib::Mutex::Lock lock(m_mutex);
	tunnel_context_base::dispatch_me();
	// synchronize with other end of the tunnel
	m_cond.wait(m_mutex);
}


} // namespace sigx
