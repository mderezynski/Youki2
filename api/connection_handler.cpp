/*
 * Copyright 2005 Klaus Triendl
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

#include <glib.h>	// message macros
#include <sigc++/connection.h>
#include "sigx/connection_handler.h"
#include "sigx/shared_dispatchable.h"
#include "__sigx_pchfence__.h"


// needed for using g_atomic_pointer_set to cast a typed pointer pointer to 
// a void**
typedef void** pptr;


namespace sigx
{

// statics
Glib::StaticPrivate<connection_handler::connections_container_wrapper> 
	connection_handler::thread_specific_connections = GLIBMM_STATIC_PRIVATE_INIT;

//static 
void connection_handler::store(
	const std::tr1::shared_ptr<sigc_connection_ptr>& _A_refconnptr, 
	const sigc::connection& c
)
{
	connections_container_wrapper* pcont_wrapper = thread_specific_connections.get();
	if (!pcont_wrapper)
	{
		pcont_wrapper = new connections_container_wrapper;
		thread_specific_connections.set(pcont_wrapper);
	}
	connections_container_wrapper::container_type& conns = pcont_wrapper->m_connections;

	sigc_connection_ptr& pc = *_A_refconnptr;
	// create a copy of c on the heap;
	// must create on the heap because we need control in the current thread over the connection's lifetime
	g_atomic_pointer_set(pptr(&pc), new sigc::connection(c));
	conns[_A_refconnptr.get()] = _A_refconnptr;
}

//static 
void connection_handler::destroy(const sigc_connection_ptr* handle)
{
	connections_container_wrapper* const pcont_wrapper = thread_specific_connections.get();
	g_return_if_fail(pcont_wrapper != 0);

	// go searching the sigc connection
	connections_container_wrapper::container_type& conns = pcont_wrapper->m_connections;
	const connections_container_wrapper::container_type::iterator it = conns.find(handle);

	// silently ignore connections not found anymore	
	if (it == conns.end())
		return;

	sigc_connection_ptr& pc = *it->second;
	// delete the sigc connection object
	delete pc;
	// reset the sig connection pointer such that client threads know that 
	// no connection exists anymore
	g_atomic_pointer_set(pptr(&pc), 0);
	// note that this doesn't necessarily delete the sigc connection pointer itself
	// because it is a shared pointer, and some client threads might have it shared 
	// within a connection_wrapper
	conns.erase(it);
}

connection_handler::connections_container_wrapper::~connections_container_wrapper()
{
	// disconnect all connections
	const container_type::iterator itEnd = m_connections.end();
	for (container_type::iterator it = m_connections.begin(); it != itEnd; ++it)
	{
		sigc_connection_ptr& pc = *it->second;
		pc->disconnect();
		// delete the sigc connection object
		delete pc;
		g_atomic_pointer_set(pptr(&pc), 0);
	}
}




//template<>
//static 
void typed_connection_handler<Glib::SignalIdle, internal::SIGGROUP_IRRELEVANT>::
connect(
	const std::tr1::shared_ptr<sigc_connection_ptr>& _A_refconnptr, 
	const std::tr1::shared_ptr<signal_source_base>& psigsource, 
	const slot_type& _A_slot, 
	int priority)
{
	// must have a valid signal source
	g_return_if_fail(psigsource.get());

	// get the signal from the signal source ...
	typedef signal_type (*fp_sig_getter)(signal_source_ptr);
	fp_sig_getter getsig = 
		reinterpret_cast<fp_sig_getter>(psigsource->getter());

	// ... and connect the slot
	const sigc::connection& c = getsig(psigsource.get()).connect(_A_slot, priority);
	// ... store the resulting connection in the container of the thread's connections
	connection_handler::store(_A_refconnptr, c);
}

void typed_connection_handler<Glib::SignalTimeout, internal::SIGGROUP_IRRELEVANT>::
connect(
	const std::tr1::shared_ptr<sigc_connection_ptr>& _A_refconnptr, 
	const std::tr1::shared_ptr<signal_source_base>& psigsource, 
	const slot_type& _A_slot, 
	unsigned int interval, int priority)
{
	// must have a valid signal source
	g_return_if_fail(psigsource.get());

	// get the signal from the signal source ...
	typedef signal_type (*fp_sig_getter)(signal_source_ptr);
	const fp_sig_getter getsig = 
		reinterpret_cast<fp_sig_getter>(psigsource->getter());

	// ... and connect the slot
	const sigc::connection& c = 
		getsig(psigsource.get()).connect(_A_slot, interval, priority);
	// ... store the resulting connection in the container of the thread's connections
	connection_handler::store(_A_refconnptr, c);
}

void typed_connection_handler<Glib::SignalIO, internal::SIGGROUP_IRRELEVANT>::
connect(
	const std::tr1::shared_ptr<sigc_connection_ptr>& _A_refconnptr, 
	const std::tr1::shared_ptr<signal_source_base>& psigsource, 
	const slot_type& _A_slot, 
	int fd, Glib::IOCondition condition, int priority)
{
	// must have a valid signal source
	g_return_if_fail(psigsource.get());

	// get the signal from the signal source ...
	typedef signal_type (*fp_sig_getter)(signal_source_ptr);
	const fp_sig_getter getsig = 
		reinterpret_cast<fp_sig_getter>(psigsource->getter());

	// ... and connect the slot
	const sigc::connection& c = 
		getsig(psigsource.get()).connect(_A_slot, fd, condition, priority);
	// ... store the resulting connection in the container of the thread's connections
	connection_handler::store(_A_refconnptr, c);
}

void typed_connection_handler<Glib::SignalChildWatch, internal::SIGGROUP_IRRELEVANT>::
connect(
	const std::tr1::shared_ptr<sigc_connection_ptr>& _A_refconnptr, 
	const std::tr1::shared_ptr<signal_source_base>& psigsource, 
	const slot_type& _A_slot, GPid pid, int priority)
{
	// must have a valid signal source
	g_return_if_fail(psigsource.get());

	// get the signal from the signal source ...
	typedef signal_type (*fp_sig_getter)(signal_source_ptr);
	const fp_sig_getter getsig = 
		reinterpret_cast<fp_sig_getter>(psigsource->getter());

	// ... and connect the slot
	const sigc::connection& c = 
		getsig(psigsource.get()).connect(_A_slot, pid, priority);
	// ... store the resulting connection in the container of the thread's connections
	connection_handler::store(_A_refconnptr, c);
}


} // namespace sigx
