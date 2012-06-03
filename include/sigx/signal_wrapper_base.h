#ifndef _SIGX_SIGNAL_BASE_HPP_
#define _SIGX_SIGNAL_BASE_HPP_

/*
 * Copyright 2006 Klaus Triendl
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

#include <tr1/memory>
#include <utility> // std::pair
#include <sigxconfig.h>
#include <sigx/fwddecl.h>
#include <sigx/nonheapallocatable.h>
#include <sigx/shared_dispatchable.h>
#include <sigx/connection_wrapper.h>
#include <sigx/auto_tunneler.h>


namespace sigx
{

/**	@short The base for a sigx::signal_wrapper.
 *	
 *	It holds a shared_dispatchable and a shared signal source.
 *	
 *	@ingroup signals
 */
class SIGX_API signal_wrapper_base: nonheapallocatable
{

protected:
	/**	
	 *	@throw	Might throw a std::bad_alloc exception (from dispatchable's ctor)
	 */
	signal_wrapper_base();
	// non-virtual by design
	~signal_wrapper_base() throw();
	signal_wrapper_base(const shared_dispatchable& _A_disp, const std::tr1::shared_ptr<signal_source_base>& _A_sigsource) throw();

	// implicit copy ctor is fine
	// implicit assignment operator is fine



	/**	@short	Prepare a shared sigc::connection pointer and a connection_wrapper.
	 *	
	 *	The sigc::connection itself will be created in the context of the server thread
	 */
	std::pair<connection_wrapper, std::tr1::shared_ptr<sigc_connection_ptr> >
	prepare_connection(const tunnel_base& _A_tunnel) const;

	/**	@short	Call _A_func_conn_handler in the context of the server thread, _A_func_conn_handler 
	 *			resolves to typed_connection_handler<>::connect[_notify] connecting _A_func to the server 
	 *			thread's signal
	 */
	template<typename T_functor, typename T_functor_conn_handler>
	connection_wrapper connect(const T_functor& _A_func, const T_functor_conn_handler& _A_func_conn_handler) const;

protected:
	///	The server thread's shared dispatchable to operate on for making connections or 
	///	emitting the signal
	shared_dispatchable m_disp;
	///	The source for the server thread's signal
	std::tr1::shared_ptr<signal_source_base> m_sigsource;
};


template<typename T_functor, typename T_functor_conn_handler>
connection_wrapper signal_wrapper_base::connect(const T_functor& _A_func, const T_functor_conn_handler& _A_func_conn_handler) const
{
	typedef internal::auto_tunneler<T_functor> auto_tunneler_t;
	
	// passed in functor must not be a slot or adapt a slot;
	// we have to apply this restriction because slots might have bound
	// trackables that can cause non-threadsafe access to the passed in slot
	// which will live in the context of the server thread
	static_assert((sigx::internal::is_or_adapts_slot<T_functor>::value == false),"");
	
	// toplevel functor must be a tunnel functor
	static_assert((sigc::is_base_and_derived<tunnel_base, typename auto_tunneler_t::functor_type>::value == true),"");

	typename auto_tunneler_t::functor_type functor2connect = 
		auto_tunneler_t::auto_open_tunnel(_A_func);
	// the validity of tunnel functors doesn't get tracked by the sigc++ default visit_each mechanism, 
	// we activate sigx' own validity tracking, which is threadsafe
	functor2connect.activate_validity_tracking();

	const std::pair<connection_wrapper, std::tr1::shared_ptr<sigc_connection_ptr> >& ret = 
		signal_wrapper_base::prepare_connection(functor2connect);

	try
	{
		// now send a message to the server thread (holding the signal the client thread wants 
		// to connect to);
		// the message gets handled by a special function handling the connection
		open_tunnel_with(
			_A_func_conn_handler, 
			m_disp
		)
		// transfer:
		// - the prepared connection pointer
		// - the signal source
		// - the functor to connect
		// 
		// The functor to connect is the tunnel functor wrapped in an exception catcher
		// that catches a bad_dispatcher error.
		// This is necessary because the dispatcher of the tunnel functor (that is 
		// probably the dispatcher running in the context of the calling thread)
		// could go out of scope (e.g. because the calling thread finishes), but 
		// the tunnel functor is still connected to the server thread's signal.
		// Before the server thread gets the disconnect message (which is 
		// triggered by the dispatcher or by trackable objects of the calling 
		// thread going out of scope) it might emit the signal 
		// on this tunnel functor and gets a bad_dispatcher error thrown.
		// Because the programmer can't really influence this situation, sigx
		// catches the exception.
		(	ret.second, m_sigsource, 
			sigc::exception_catch(functor2connect, 
				// use a catcher here because the signal might get emitted 
				// while the dispatcher the tunnel functor operates on dies
				// before the tunnel functor is disconnected from that signal
				// (btw: this is done internally by the validity trackable
				bad_dispatcher_catcher<typename auto_tunneler_t::functor_type::result_type>()
			)
		);
	}
	catch (...)
	{
		// message dispatching failed at the call site;
		// reset pointer to the sigc connection to make the connection invalid
		*ret.second = 0;
		throw;
	}

	return ret.first;
}


} // namespace sigx


#endif // end file guard
