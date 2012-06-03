#ifndef _SIGX_TUNNEL_CONTEXT_H_
#define _SIGX_TUNNEL_CONTEXT_H_

/*
 * Copyright 2007 Klaus Triendl
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

#include <memory>	// std::auto_ptr
#include <sigc++/type_traits.h>
#include <sigc++/adaptors/bound_argument.h>
#include <sigx/fwddecl.h>
#include <sigx/types.h>
#include <sigx/internal_types.h>
#include <sigx/dispatcher.h>
#include <sigx/tunnel_context_base.h>


namespace sigx
{

/**	@short	Represents a tunnel message.
 *	
 *	Specializations of this template represent different types of tunnel messages.
 *	tunnel_contextS are typically created on the heap by tunnel_functorS and 
 *	manage their lifetime themselves.
 *	
 *	@ingroup Dispatching
 */
template<sync_type I_sync, typename T_return, typename T_unary_functor>
struct tunnel_context;


/**	@short	Exists solely to make the compiler deduce the meta argument T_adaptor.
 */
template<sync_type I_sync, typename T_return, typename T_adaptor>
tunnel_context<I_sync, T_return, T_adaptor>* make_new_tunnel_context(const shared_dispatchable& _A_disp, const tunnel_validity_tracker& _A_validity_tracker, const T_adaptor& _A_func)
{
	return new tunnel_context<I_sync, T_return, T_adaptor>(_A_disp, _A_validity_tracker, _A_func);
}


/**	@short	An asynchronous tunnel message.
 *	
 *	Asynchronous tunnels store a copy of the passed arguments by value, thus
 *	ensuring valid argument transmission.
 *	
 *	@note Asynchronous tunnels disregard the return value of
 *	the invoked functor and return the return type's default value
 *	@ingroup Dispatching
 */
template<typename T_return, typename T_unary_functor>
struct tunnel_context<ASYNC, T_return, T_unary_functor>: public tunnel_context_base
{
	typedef tunnel_context<ASYNC, T_return, T_unary_functor> this_type;
	typedef T_return result_type;

	tunnel_context(const shared_dispatchable& _A_disp, const tunnel_validity_tracker& _A_validity_tracker, typename sigc::type_trait<T_unary_functor>::take _A_func):	
		tunnel_context_base(_A_disp, _A_validity_tracker), 
		m_boundmessage(_A_func)
	{}

	/**	@short dispatches the tunnel_context (itself) over the referenced 
	 *	dispatcher.
	 */
	result_type tunnel()
	{
		tunnel_context_base::dispatch_me();
		return result_type();
	}
	
	void invoke()
	{
		// async tunnels must delete themselves after dispatching
		const std::auto_ptr<this_type> autodelete_this(this);

		// call functor in the context of the server thread, disregard return value
		m_boundmessage();
	}

private:
	T_unary_functor m_boundmessage;
};


/**	@short	A synchronous tunnel message
 *	
 *	Synchronous tunnels store reference wrappers to the passed arguments, thus
 *	optimizing argument transmission.
 *	
 *	@note Synchronous tunnels lock until the functor at the other side
 *	of the tunnel has completed except for when the dispatcher reference is not
 *	valid anymore (the owner thread of the dispatcher has destroyed its 
 *	dispatcher).
 *	@ingroup Dispatching
 */
template<typename T_return, typename T_unary_functor>
struct tunnel_context<SYNC, T_return, T_unary_functor>: public sync_tunnel_context_base
{
	typedef tunnel_context<SYNC, T_return, T_unary_functor> this_type;
	typedef T_return result_type;

	tunnel_context(const shared_dispatchable& _A_disp, const tunnel_validity_tracker& _A_validity_tracker, typename sigc::type_trait<T_unary_functor>::take _A_func):
		sync_tunnel_context_base(_A_disp, _A_validity_tracker), 
		m_boundmessage(_A_func), 
		m_bound_result(result_type())
	{}

	/**	@short dispatches the tunnel_context (itself) over the referenced 
	 *	dispatcher.
	 */
	T_return tunnel()
	{
		const std::auto_ptr<this_type> autodelete_this(this);
		
		Glib::Mutex::Lock lock(m_mutex);
		// rather call tunnel_context_base::dispatch_me() than 
		// sync_tunnel_context_base::dispatch_me() because we want to ensure
		// that the result is returned while we still hold the lock
		tunnel_context_base::dispatch_me();
		// synchronize with other end of the tunnel
		m_cond.wait(m_mutex);

		return m_bound_result.invoke();
	}

	void invoke()
	{
		Glib::Mutex::Lock lock(m_mutex);
		// save result
		m_bound_result = m_boundmessage();

		// tell the one end of the tunnel that we are done
		m_cond.signal();
	}

private:
	T_unary_functor m_boundmessage;
	sigc::bound_argument<typename sigc::type_trait<T_return>::type> m_bound_result;
};


/**	@short a synchronous tunnel with return type `void".
 *	
 *	Synchronous tunnels store reference wrappers to the passed arguments, thus
 *	optimizing argument transmission.
 *	
 *	@note Synchronous tunnels lock until the functor at the other side
 *	of the tunnel has completed except for when the dispatcher reference is not
 *	valid anymore (the owner thread of the dispatcher has destroyed its 
 *	dispatcher).
 *	@ingroup Dispatching
 */
template<typename T_unary_functor>
struct tunnel_context<SYNC, void, T_unary_functor>: public sync_tunnel_context_base
{
	typedef tunnel_context<SYNC, void, T_unary_functor> this_type;
	typedef void result_type;

	tunnel_context(const shared_dispatchable& _A_disp, const tunnel_validity_tracker& _A_validity_tracker, typename sigc::type_trait<T_unary_functor>::take _A_func):
		sync_tunnel_context_base(_A_disp, _A_validity_tracker), 
		m_boundmessage(_A_func)
	{}

	/**	@short dispatches the tunnel_context (itself) over the referenced 
	 *	dispatcher.
	 */
	void tunnel()
	{
		const std::auto_ptr<this_type> autodelete_this(this);

		sync_tunnel_context_base::dispatch_me();
	}
	
	void invoke()
	{
		Glib::Mutex::Lock lock(m_mutex);
		m_boundmessage();

		// tell the one end of the tunnel that we are done
		m_cond.signal();
	}

private:
	T_unary_functor m_boundmessage;
};


} // namespace sigx


#endif	//	file guard
