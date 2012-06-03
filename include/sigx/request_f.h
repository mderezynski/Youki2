// -*- c++ -*-
/* Do not edit! -- generated file */


#ifndef _SIGXMACROS_REQUEST_F_H_
#define _SIGXMACROS_REQUEST_F_H_


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
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <sigc++/slot.h>
#include <sigx/noncopyable.h>
#include <sigx/nonheapallocatable.h>
#include <sigx/nonpointeraliasing.h>
#include <sigx/internal_types.h>


namespace sigx
{

/**	@short Asynchronous request functor for a sigx::threadable.
 *	
 *	It saves you writing request methods that have to call a handler method
 *	for the request through a tunnel,like:
 *	
 *	@code
 *	class IPResolverThread: public sigx::threadable
 *	{
 *	public:
 *		void resolve(in_addr_t nIP);
 *		void stop_resolving();
 *	};
 *	
 *	void IPResolverThread::resolve(in_addr_t nIP)
 *	{
 *		sigx::open_tunnel(sigc::mem_fun(this, &IPResolverThread::on_resolve))(nIP);
 *	}
 *	
 *	void IPResolverThread::stop_resolving()
 *	{
 *		sigx::open_tunnel(sigc::mem_fun(this, &IPResolverThread::on_stop_resolving))();
 *	}
 *	@endcode
 *	
 *	Instead,delegate it to the request functor:
 *	
 *	@code
 *	class IPResolverThread: public sigx::threadable
 *	{
 *	public:
 *		sigx::request_f<in_addr_t> resolve;
 *		sigx::request_f<> stop_resolving;
 *	};
 *	
 *	IPResolverThread::IPResolverThread(): 
 *		resolve(sigc::mem_fun(this, &IPResolverThread::on_resolve)),*		stop_resolving(sigc::mem_fun(this, &IPResolverThread::on_stop_resolving))
 *	{}
 *	@endcode
 *	
 *	It is derived from %sigc::slot because a slot provides already all the
 *	necessary functionalities: takes a functor and creates a untyped slot
 *	representation,has function invokation operator ().
 *	
 *	@attention Do not specify a return type as the first template parameter.
 *	As asynchronous tunnels actually do not have a return type,@e request_f 
 *	omits it,thus the return type is always `void".
 *	
 *	@note non-copyable,not constructible on the heap (with new) and can't be 
 *	pointer aliased (with operator &) to ensure that it is de-facto bound to  
 *	a wrapping object.
 *	
 *	@ingroup signals
 */
template<typename T_arg1 = sigc::nil, typename T_arg2 = sigc::nil, typename T_arg3 = sigc::nil, typename T_arg4 = sigc::nil, typename T_arg5 = sigc::nil, typename T_arg6 = sigc::nil, typename T_arg7 = sigc::nil>
class request_f: noncopyable, nonheapallocatable, nonpointeraliasing, protected sigc::slot<void, T_arg1, T_arg2, T_arg3, T_arg4, T_arg5, T_arg6, T_arg7>
{
public:
	typedef sigc::slot<void,T_arg1, T_arg2, T_arg3, T_arg4, T_arg5, T_arg6, T_arg7> parent_type;
	
	// allow function operator to be used
	using parent_type::operator ();

	/**	@short Constructs the request functor.
	 *	
	 *	@note The passed in functor must not be a sigc::slot and must not be 
	 *	a tunnel functor.
	 *	The passed in functor gets tunneled automatically.
	 *	
	 *	@param _A_func A dispatchable functor,i.e. a functor on a dispatchable's
	 *	method or one explicitly created with dispatch_with().
	 */
	template<typename T_functor>
	explicit request_f(const T_functor& _A_func): 
		parent_type(tunnel_functor<ASYNC, T_functor>(_A_func))
	{
		// passed in functor must not be tunneled
		static_assert((internal::is_functor_tunneled<T_functor>::value == false),"");

		// passed in functor must not be a slot or adapt a slot;
		// we have to apply this restriction because slots might have bound
		// trackables that can cause non-threadsafe access to the passed in slot
		// which will live in the context of the server thread
		static_assert((sigx::internal::is_or_adapts_slot<T_functor>::value == false),"");
	}
};


} // namespace sigx
#endif /* _SIGXMACROS_REQUEST_F_H_ */
