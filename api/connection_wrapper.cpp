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

#if 0	//	rvalue references
#include <utility>	//	std::move
#endif
#include <sigc++/connection.h>
#include <sigc++/functors/mem_fun.h>
#include <sigc++/adaptors/bind.h>
#include <glib/gatomic.h>
#include "sigx/types.h"
#include "sigx/connection_wrapper.h"
#include "sigx/connection_handler.h"
#include "sigx/signal_wrapper_base.h"
#include "sigx/tunnel_functor.h"
#include "__sigx_pchfence__.h"


namespace sigx
{

connection_wrapper::connection_wrapper(): 
	m_sigc_conn(), 
	m_shared_disp(), 
	m_sigcconn_refcount()
{}


connection_wrapper::connection_wrapper(const shared_dispatchable& _A_disp, const shared_sigc_conn_ptr& _A_conn): 
	m_sigc_conn(_A_conn), 
	m_shared_disp(_A_disp), 
	m_sigcconn_refcount(new int(1))
{}

connection_wrapper::connection_wrapper(const connection_wrapper& other) throw(): 
	m_sigc_conn(other.m_sigc_conn), 
	m_shared_disp(other.m_shared_disp), 
	m_sigcconn_refcount(other.m_sigcconn_refcount)
{
	if (m_sigcconn_refcount)
		g_atomic_int_inc(m_sigcconn_refcount);
}

#if 0	//	rvalue references
connection_wrapper::connection_wrapper(connection_wrapper&& other) throw(): 
	m_sigc_conn(std::move(other.m_sigc_conn)), 
	m_shared_disp(std::move(other.m_shared_disp)), 
	m_sigcconn_refcount(other.m_sigcconn_refcount)
{
	// destroy other
	other.m_sigcconn_refcount = 0;
}
#endif

connection_wrapper::~connection_wrapper() throw()
{
	try
	{
		destroy_self();
	}
	catch (...)
	{}
}

void connection_wrapper::destroy_self()
{
	if (m_sigcconn_refcount)
	{
		// test whether we are the last one pointing to the remote 
		// sigc::connection and whether it's still valid;
		// destroy this connection if yes
		if (TRUE == g_atomic_int_dec_and_test(m_sigcconn_refcount))
		{
			// make a local copy of the shared sigc connection pointer (nothrow)
			const shared_sigc_conn_ptr sigc_conn(m_sigc_conn);

			delete m_sigcconn_refcount;
			m_sigcconn_refcount = 0;
			m_sigc_conn.reset();
			// destroying m_shared_disp is not part of self-destruction;
			// it will be destroyed by the dtor or assigned anew in operator =

			try
			{
				// note: we tunnel here a message to the static connection_handler::destroy() method to 
				// destroy/delete the sigc connection object itself living at the server thread side;
				// 
				// the sigc connection object itself is bound to the lifetime of the server thread.
				// the server thread keeps a sigc connection pointer (the address of the connection object 
				// in a map where the address of the 
				// connection pointer (not the object!) serves as a handle (key).
				// in order to ensure the lifetime of the connection pointer and thus its address (= handle) 
				// we send the shared pointer to the connection pointer and turn it at the server thread's side
				// into the address of the connection pointer (a sigc::connection**) with a call 
				// to shared_sigc_conn_ptr::get() - the destroy() function expects a sigc_connection_ptr*
				open_tunnel_with(
					sigc::compose(
						sigc::ptr_fun(&connection_handler::destroy), 
						sigc::retype_return<const sigc_connection_ptr*>(sigc::mem_fun(&shared_sigc_conn_ptr::get))
					), 
					m_shared_disp
				)
				(sigc_conn);
			}
			catch (const bad_dispatcher&)
			{
				// silently ignore;
				// don't mind if the message can't get dispatched, the server 
				// thread has probably already ended and deleted the 
				// sigc::connection
			}
			// what about other exceptions?
			// well, in this case we find ourselves in a non-recoverable state because the 
			// server thread didn't get the message to destroy the connection object. 
			// The earliest point of destruction is the end of the server thread itself
			// (the lifetime of connection objects is bound to the server thread's lifetime);
			// the connection_wrapper object is cleaned up nevertheless...
		}
	}
}

connection_wrapper& connection_wrapper::operator =(const connection_wrapper& other)
{
	if (this != &other)
	{
		destroy_self();

		m_sigc_conn = other.m_sigc_conn;
		m_shared_disp = other.m_shared_disp;
		m_sigcconn_refcount = other.m_sigcconn_refcount;

		if (m_sigcconn_refcount)
			g_atomic_int_inc(m_sigcconn_refcount);
	}

	return *this;
}

#if 0	//	rvalue references
connection_wrapper& connection_wrapper::operator =(connection_wrapper&& other)
{
	destroy_self();

	m_sigc_conn = std::move(other.m_sigc_conn);
	m_shared_disp = std::move(other.m_shared_disp);
	m_sigcconn_refcount = other.m_sigcconn_refcount;

	// destroy other
	other.m_sigcconn_refcount = 0;

	return *this;
}
#endif

bool connection_wrapper::empty() const
{
	if ((!m_sigcconn_refcount					)	|| 
		(!g_atomic_pointer_get(&*m_sigc_conn)	)	)
		return false;

	return 
		open_sync_tunnel_with(
			sigc::mem_fun(&sigc::connection::empty), 
			m_shared_disp
		// read volatile at the other side of the tunnel;
		)(sigc::ref(*m_sigc_conn));
}

bool connection_wrapper::connected() const
{
	if ((!m_sigcconn_refcount					)	|| 
		(!g_atomic_pointer_get(&*m_sigc_conn)	)	)
		return false;

	return 
		open_sync_tunnel_with(
			sigc::mem_fun(&sigc::connection::connected), 
			m_shared_disp
		// read volatile at the other side of the tunnel;
		)(sigc::ref(*m_sigc_conn));
}

bool connection_wrapper::blocked() const
{
	if ((!m_sigcconn_refcount					)	|| 
		(!g_atomic_pointer_get(&*m_sigc_conn)	)	)
		return false;

	return 
		open_sync_tunnel_with(
			sigc::mem_fun(&sigc::connection::blocked), 
			m_shared_disp
		// read volatile at the other side of the tunnel;
		)(sigc::ref(*m_sigc_conn));
}

bool connection_wrapper::block(bool should_block /*= true*/)
{
	if ((!m_sigcconn_refcount					)	|| 
		(!g_atomic_pointer_get(&*m_sigc_conn)	)	)
		return false;

	return 
		open_sync_tunnel_with(
			sigc::mem_fun(&sigc::connection::block), 
			m_shared_disp
		// read volatile at the other side of the tunnel;
		// the ref() avoids copying the volatile pointer
		)(sigc::ref(*m_sigc_conn), should_block);
}

bool connection_wrapper::unblock()
{
	if ((!m_sigcconn_refcount					)	|| 
		(!g_atomic_pointer_get(&*m_sigc_conn)	)	)
		return false;

	return 
		open_sync_tunnel_with(
			sigc::mem_fun(&sigc::connection::unblock), 
			m_shared_disp
		// read volatile at the other side of the tunnel;
		// the ref() avoids copying the volatile pointer
		)(sigc::ref(*m_sigc_conn));
}

void connection_wrapper::disconnect()
{
	if ((!m_sigcconn_refcount					)	|| 
		(!g_atomic_pointer_get(&*m_sigc_conn)	)	)
		return;

	return 
		open_tunnel_with(
			sigc::compose(
				sigc::mem_fun(&sigc::connection::disconnect), 
				sigc::mem_fun(&shared_sigc_conn_ptr::operator *)
			), 
			m_shared_disp
		// read volatile at the other side of the tunnel;
		// don't ref() because it's an async call
		)(m_sigc_conn);
}

connection_wrapper::operator bool()
{
	if ((!m_sigcconn_refcount					)	|| 
		(!g_atomic_pointer_get(&*m_sigc_conn)	)	)
		return false;

	return 
		open_sync_tunnel_with(
			sigc::mem_fun(&sigc::connection::operator bool), 
			m_shared_disp
		// read volatile at the other side of the tunnel;
		// the ref() avoids copying the volatile pointer
		)(sigc::ref(*m_sigc_conn));
}


} // namespace sigx
