#ifndef _SIGX_CONNECTION_HANDLER_HPP_
#define _SIGX_CONNECTION_HANDLER_HPP_

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

#include <tr1/memory>	// std::tr1::shared_ptr
#include <map>
#include <sigc++/signal.h>
#include <glib.h>	// message macros
#include <glibmm/thread.h> // Glib::StaticPrivate
#include <glibmm/main.h>
#include <sigxconfig.h>
#include <sigx/fwddecl.h>
#include <sigx/noninstantiatable.h>
#include <sigx/signal_traits.h>
#include <sigx/signal_source_base.h>
#include <sigx/connection_wrapper.h>


namespace sigx
{

/**	@short Stores connections of any client thread to a server thread's signal and destroys them along with the 
 *	thread's lifetime.
 *	@note A static class only.
 */
class SIGX_API connection_handler: noninstantiatable
{
public:
	/**	@short Destroys a sigc::connection in the context of the server thread.
	 *	
	 *	Called when the last connection sharing a sigc::connection
	 *	goes out of scope and tunnels a message.
	 *	
	 *	@note message handler for sigx::connection_wrapper::~connection_wrapper().
	 */
	static void destroy(const sigc_connection_ptr* handle);

	/**	@short Stores a sigc::connection in the context of the server thread.
	 *	@param _A_refconn Shared connection pointer
	 *	@param c The connection
	 */
	static void store(
		const std::tr1::shared_ptr<sigc_connection_ptr>& _A_refconn, 
		const sigc::connection& c);


protected:
	/**	A wrapper for a container of sigc::connectionS.
	 *	This wrapper is necessary because the destructor ensures 
	 *	disconnecting the connections when the owner thread 
	 *	finishes.
	 *	Disconnecting the connections explicitly is only necessary if the signals
	 *	don't live in the server thread's local storage but in the thread managed
	 *	dispatchable which is a wrapper object surviving the thread itself.
	 */
	struct connections_container_wrapper
	{
		/**	@short a map of connections living in the context of the
		 *	owner thread.
		 *	@note We must use a shared_ptr because auto_ptr doesn't meet the copy-constructible requirement
		 */
		typedef std::map<const sigc_connection_ptr* const /*handle*/, std::tr1::shared_ptr<sigc_connection_ptr> > container_type;

		container_type m_connections;
		~connections_container_wrapper();
	};

	static Glib::StaticPrivate<connections_container_wrapper> thread_specific_connections;
};


template<typename T_signal, internal::signal_group I_oneof>
class typed_connection_handler;

template<typename T_signal>
class typed_connection_handler<T_signal, internal::SIGGROUP_SIGC>: noninstantiatable
{
public:
	typedef T_signal signal_type;
	typedef typename signal_type::slot_type slot_type;

	/**	@param _A_refconnptr A prepared connection pointer
	 *	@param psigsource Shared signal source
	 *	@note Executed by the server thread only.
	 */
	static void connect(const std::tr1::shared_ptr<sigc_connection_ptr>& _A_refconnptr, const std::tr1::shared_ptr<signal_source_base>& psigsource, const slot_type& _A_slot)
	{
		// must have a valid signal source
		g_return_if_fail(psigsource.get());

		// get the signal from the signal source ...
		typedef signal_type (*fp_sig_getter)(signal_source_ptr);
		const fp_sig_getter getsig = 
			reinterpret_cast<fp_sig_getter>(psigsource->getter());

		// ... and connect the slot
		const sigc::connection& c = getsig(psigsource.get()).connect(_A_slot);
		// ... store the resulting connection in the container of the thread's connections
		connection_handler::store(_A_refconnptr, c);
	}
};

/**	@short Specialization for a Glib::SignalProxyN
 */
template<typename T_signal>
class typed_connection_handler<T_signal, internal::SIGGROUP_GLIB_PROXY>: noninstantiatable
{
public:
	typedef T_signal signal_type;
	typedef typename signal_type::SlotType slot_type;
	typedef typename signal_type::VoidSlotType void_slot_type;

	/**	@note Executed by the server thread only.
	 */
	static void connect(const std::tr1::shared_ptr<sigc_connection_ptr>& _A_refconnptr, const std::tr1::shared_ptr<signal_source_base>& psigsource, const slot_type& _A_slot, bool after)
	{
		// must have a valid signal source
		g_return_if_fail(psigsource.get());

		// get the signal from the signal source ...
		typedef signal_type (*fp_sig_getter)(signal_source_ptr);
		const fp_sig_getter getsig = 
			reinterpret_cast<fp_sig_getter>(psigsource->getter());

		// ... and connect the slot
		const sigc::connection& c = getsig(psigsource.get()).connect(_A_slot, after);
		// ... store the resulting connection in the container of the thread's connections
		connection_handler::store(_A_refconnptr, c);
	}

	/**	@note Executed by the server thread only.
	 */
	static void connect_notify(const std::tr1::shared_ptr<sigc_connection_ptr>& _A_refconnptr, const std::tr1::shared_ptr<signal_source_base>& psigsource, const void_slot_type& _A_slot, bool after)
	{
		// must have a valid signal source
		g_return_if_fail(psigsource.get());

		// get the signal from the signal source ...
		typedef signal_type (*fp_sig_getter)(signal_source_ptr);
		const fp_sig_getter getsig = 
			reinterpret_cast<fp_sig_getter>(psigsource->getter());

		// ... and connect the slot
		const sigc::connection& c = getsig(psigsource.get()).connect_notify(_A_slot, after);
		// ... store the resulting connection in the container of the thread's connections
		connection_handler::store(_A_refconnptr, c);
	}
};

/**	@short Specialization for a Glib::SignalIdle
 */
template<>
class SIGX_API typed_connection_handler<Glib::SignalIdle, internal::SIGGROUP_IRRELEVANT>: noninstantiatable
{
public:
	typedef Glib::SignalIdle signal_type;
	typedef sigc::slot<bool> slot_type;

	/**	@note Executed by the server thread only.
	 */
	static void connect(const std::tr1::shared_ptr<sigc_connection_ptr>& _A_refconnptr, const std::tr1::shared_ptr<signal_source_base>& psigsource, const slot_type& _A_slot, int priority);
};


/**	@short Specialization for a Glib::SignalTimeout
 */
template<>
class SIGX_API typed_connection_handler<Glib::SignalTimeout, internal::SIGGROUP_IRRELEVANT>: noninstantiatable
{
public:
	typedef Glib::SignalTimeout signal_type;
	typedef sigc::slot<bool> slot_type;

	/**	@note Executed by the server thread only.
	 */
	static void connect(const std::tr1::shared_ptr<sigc_connection_ptr>& _A_refconnptr, const std::tr1::shared_ptr<signal_source_base>& psigsource, const slot_type& _A_slot, unsigned int interval, int priority);
};


/**	@short Specialization for a Glib::SignalIO
 */
template<>
class SIGX_API typed_connection_handler<Glib::SignalIO, internal::SIGGROUP_IRRELEVANT>: noninstantiatable
{
public:
	typedef Glib::SignalIO signal_type;
	typedef sigc::slot<bool, Glib::IOCondition> slot_type;

	/**	@note Executed by the server thread only.
	 */
	static void connect(const std::tr1::shared_ptr<sigc_connection_ptr>& _A_refconnptr, const std::tr1::shared_ptr<signal_source_base>& psigsource, const slot_type& _A_slot, int fd, Glib::IOCondition condition, int priority);
};


/**	@short Specialization for a Glib::SignalChildWatch
 */
template<>
class SIGX_API typed_connection_handler<Glib::SignalChildWatch, internal::SIGGROUP_IRRELEVANT>: noninstantiatable
{
public:
	typedef Glib::SignalChildWatch signal_type;
	typedef sigc::slot<void, GPid, int> slot_type;

	/**	@note Executed by the server thread only.
	 */
	static void connect(const std::tr1::shared_ptr<sigc_connection_ptr>& _A_refconnptr, const std::tr1::shared_ptr<signal_source_base>& psigsource, const slot_type& _A_slot, GPid pid, int priority);
};


} // namespace sigx


#endif // end file guard
