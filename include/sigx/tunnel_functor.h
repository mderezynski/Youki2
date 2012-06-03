// -*- c++ -*-
/* Do not edit! -- generated file */


#ifndef _SIGXMACROS_TUNNEL_FUNCTOR_H_
#define _SIGXMACROS_TUNNEL_FUNCTOR_H_


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

/**	@defgroup Functors Adaptors
 *	@short Useful sigc++ adaptors.
 */

#include <sigc++/sigc++.h>
#include <sigx/types.h>
#include <sigx/internal_types.h>
#include <sigx/tunnel_base.h>
#include <sigx/tunnel_context.h>
#include <sigx/ref.h>


namespace sigx
{


template<sync_type I_sync, typename T_functor>
struct tunnel_functor;

/**	@short creates a tunnel on the given functor.
 *	@note expects the functor to be dispatchable. A functor is dispatchable if
 *	the class the functor operates on is derived from sigx::dispatchable or if the 
 *	functor is or contains a SIGX_DISPATCH_WITH_FUNCTOR.
 *	@ingroup Functors
 *	@code
 *	// if class MyThread is dispatchable, sigc::mem_fun creates a dispatchable functor.
 *	open_tunnel(sigc::mem_fun(destobj, &MyThread::dosomething));
 *	// otherwise, create a dispatchable functor explicitly with dispatch_with
 *	open_tunnel_with(sigc::mem_fun(destobj, &MyThread::dosomething), dispatchable);
 *	open_tunnel_with(sigc::ptr_fun(&MyThread::dosomething_static), dispatchable);
 *	@endcode
 *	@attention Never invoke an asynchronous functor with arguments passed by
 *	reference with sigc::ref() (or at least not if you don't know exactly what 
 *	you are doing)!
 *	@note You have to be careful that T_functor, arguments bound to it and 
 *	passed arguments are threadsafe.
 *	Asynchronous tunnels copy T_functor and passed arguments on invokation of 
 *	the tunnel functor and destroy them in the context of the server thread 
 *	(the thread receiving the message which is different from the sender thread!).
 *	e.g. never do this:
 *	@code
 *	struct MyThread
 *	{
 *		void do_something(const GLib::RefPtr<X>& p) {}
 *	};
 *	
 *	Glib::RefPtr<X> p;
 *	open_tunnel(mythread, &MyThread::do_something)(p);
 *	@endcode
 *	For safety reasons you can apply this rule also for synchronous tunnels, 
 *	although the invokation behaves differently: Still T_functor is copied but
 *	passed arguments are sent by reference to the server thread.
 *	The same rules apply for sigx::request_f
 */
template<typename T_functor>
struct tunnel_functor<ASYNC, T_functor>: public sigc::adapts<T_functor>, public tunnel_base
{
	typedef typename sigc::adapts<T_functor>::adaptor_type adaptor_type;
	typedef typename adaptor_type::result_type result_type;

	template<typename T_arg1 = void, typename T_arg2 = void, typename T_arg3 = void, typename T_arg4 = void, typename T_arg5 = void, typename T_arg6 = void, typename T_arg7 = void>
	struct deduce_result_type
	{
		// we could also use sigc::deduce_result_type but this saves another
		// level of indirection and does what sigc++ does internally
		typedef typename adaptor_type::template deduce_result_type<typename sigc::type_trait<T_arg1>::pass, typename sigc::type_trait<T_arg2>::pass, typename sigc::type_trait<T_arg3>::pass, typename sigc::type_trait<T_arg4>::pass, typename sigc::type_trait<T_arg5>::pass, typename sigc::type_trait<T_arg6>::pass, typename sigc::type_trait<T_arg7>::pass>::type type;
	};


	result_type operator()()
	{
		return make_new_tunnel_context<ASYNC, result_type>(m_disp, m_validity_tracker, this->functor_)->tunnel();
	}

	#ifndef SIGC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD
	result_type sun_forte_workaround()
	{
		return make_new_tunnel_context<I_sync, result_type>(m_disp, m_validity_tracker, this->functor_)->tunnel();
	}
	#endif

	template<typename T_arg1>
	typename deduce_result_type<T_arg1>::type
	operator()(T_arg1 _A_arg1)
	{
		typedef typename deduce_result_type<T_arg1>::type deduced_result_type;
		return make_new_tunnel_context<ASYNC, deduced_result_type>(m_disp, m_validity_tracker, 
			sigc::bind(this->functor_, _A_arg1))->tunnel();
	}

	#ifndef SIGC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD
	template<typename T_arg1>
	typename deduce_result_type<T_arg1>::type
	sun_forte_workaround(T_arg1 _A_arg1)
	{
		typedef typename deduce_result_type<T_arg1>::type deduced_result_type;
		return make_new_tunnel_context<ASYNC, deduced_result_type>(m_disp, m_validity_tracker, 
			sigc::bind(this->functor_, _A_arg1))->tunnel();
	}
	#endif

	template<typename T_arg1, typename T_arg2>
	typename deduce_result_type<T_arg1, T_arg2>::type
	operator()(T_arg1 _A_arg1, T_arg2 _A_arg2)
	{
		typedef typename deduce_result_type<T_arg1, T_arg2>::type deduced_result_type;
		return make_new_tunnel_context<ASYNC, deduced_result_type>(m_disp, m_validity_tracker, 
			sigc::bind(this->functor_, _A_arg1, _A_arg2))->tunnel();
	}

	#ifndef SIGC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD
	template<typename T_arg1, typename T_arg2>
	typename deduce_result_type<T_arg1, T_arg2>::type
	sun_forte_workaround(T_arg1 _A_arg1, T_arg2 _A_arg2)
	{
		typedef typename deduce_result_type<T_arg1, T_arg2>::type deduced_result_type;
		return make_new_tunnel_context<ASYNC, deduced_result_type>(m_disp, m_validity_tracker, 
			sigc::bind(this->functor_, _A_arg1, _A_arg2))->tunnel();
	}
	#endif

	template<typename T_arg1, typename T_arg2, typename T_arg3>
	typename deduce_result_type<T_arg1, T_arg2, T_arg3>::type
	operator()(T_arg1 _A_arg1, T_arg2 _A_arg2, T_arg3 _A_arg3)
	{
		typedef typename deduce_result_type<T_arg1, T_arg2, T_arg3>::type deduced_result_type;
		return make_new_tunnel_context<ASYNC, deduced_result_type>(m_disp, m_validity_tracker, 
			sigc::bind(this->functor_, _A_arg1, _A_arg2, _A_arg3))->tunnel();
	}

	#ifndef SIGC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD
	template<typename T_arg1, typename T_arg2, typename T_arg3>
	typename deduce_result_type<T_arg1, T_arg2, T_arg3>::type
	sun_forte_workaround(T_arg1 _A_arg1, T_arg2 _A_arg2, T_arg3 _A_arg3)
	{
		typedef typename deduce_result_type<T_arg1, T_arg2, T_arg3>::type deduced_result_type;
		return make_new_tunnel_context<ASYNC, deduced_result_type>(m_disp, m_validity_tracker, 
			sigc::bind(this->functor_, _A_arg1, _A_arg2, _A_arg3))->tunnel();
	}
	#endif

	template<typename T_arg1, typename T_arg2, typename T_arg3, typename T_arg4>
	typename deduce_result_type<T_arg1, T_arg2, T_arg3, T_arg4>::type
	operator()(T_arg1 _A_arg1, T_arg2 _A_arg2, T_arg3 _A_arg3, T_arg4 _A_arg4)
	{
		typedef typename deduce_result_type<T_arg1, T_arg2, T_arg3, T_arg4>::type deduced_result_type;
		return make_new_tunnel_context<ASYNC, deduced_result_type>(m_disp, m_validity_tracker, 
			sigc::bind(this->functor_, _A_arg1, _A_arg2, _A_arg3, _A_arg4))->tunnel();
	}

	#ifndef SIGC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD
	template<typename T_arg1, typename T_arg2, typename T_arg3, typename T_arg4>
	typename deduce_result_type<T_arg1, T_arg2, T_arg3, T_arg4>::type
	sun_forte_workaround(T_arg1 _A_arg1, T_arg2 _A_arg2, T_arg3 _A_arg3, T_arg4 _A_arg4)
	{
		typedef typename deduce_result_type<T_arg1, T_arg2, T_arg3, T_arg4>::type deduced_result_type;
		return make_new_tunnel_context<ASYNC, deduced_result_type>(m_disp, m_validity_tracker, 
			sigc::bind(this->functor_, _A_arg1, _A_arg2, _A_arg3, _A_arg4))->tunnel();
	}
	#endif

	template<typename T_arg1, typename T_arg2, typename T_arg3, typename T_arg4, typename T_arg5>
	typename deduce_result_type<T_arg1, T_arg2, T_arg3, T_arg4, T_arg5>::type
	operator()(T_arg1 _A_arg1, T_arg2 _A_arg2, T_arg3 _A_arg3, T_arg4 _A_arg4, T_arg5 _A_arg5)
	{
		typedef typename deduce_result_type<T_arg1, T_arg2, T_arg3, T_arg4, T_arg5>::type deduced_result_type;
		return make_new_tunnel_context<ASYNC, deduced_result_type>(m_disp, m_validity_tracker, 
			sigc::bind(this->functor_, _A_arg1, _A_arg2, _A_arg3, _A_arg4, _A_arg5))->tunnel();
	}

	#ifndef SIGC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD
	template<typename T_arg1, typename T_arg2, typename T_arg3, typename T_arg4, typename T_arg5>
	typename deduce_result_type<T_arg1, T_arg2, T_arg3, T_arg4, T_arg5>::type
	sun_forte_workaround(T_arg1 _A_arg1, T_arg2 _A_arg2, T_arg3 _A_arg3, T_arg4 _A_arg4, T_arg5 _A_arg5)
	{
		typedef typename deduce_result_type<T_arg1, T_arg2, T_arg3, T_arg4, T_arg5>::type deduced_result_type;
		return make_new_tunnel_context<ASYNC, deduced_result_type>(m_disp, m_validity_tracker, 
			sigc::bind(this->functor_, _A_arg1, _A_arg2, _A_arg3, _A_arg4, _A_arg5))->tunnel();
	}
	#endif

	template<typename T_arg1, typename T_arg2, typename T_arg3, typename T_arg4, typename T_arg5, typename T_arg6>
	typename deduce_result_type<T_arg1, T_arg2, T_arg3, T_arg4, T_arg5, T_arg6>::type
	operator()(T_arg1 _A_arg1, T_arg2 _A_arg2, T_arg3 _A_arg3, T_arg4 _A_arg4, T_arg5 _A_arg5, T_arg6 _A_arg6)
	{
		typedef typename deduce_result_type<T_arg1, T_arg2, T_arg3, T_arg4, T_arg5, T_arg6>::type deduced_result_type;
		return make_new_tunnel_context<ASYNC, deduced_result_type>(m_disp, m_validity_tracker, 
			sigc::bind(this->functor_, _A_arg1, _A_arg2, _A_arg3, _A_arg4, _A_arg5, _A_arg6))->tunnel();
	}

	#ifndef SIGC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD
	template<typename T_arg1, typename T_arg2, typename T_arg3, typename T_arg4, typename T_arg5, typename T_arg6>
	typename deduce_result_type<T_arg1, T_arg2, T_arg3, T_arg4, T_arg5, T_arg6>::type
	sun_forte_workaround(T_arg1 _A_arg1, T_arg2 _A_arg2, T_arg3 _A_arg3, T_arg4 _A_arg4, T_arg5 _A_arg5, T_arg6 _A_arg6)
	{
		typedef typename deduce_result_type<T_arg1, T_arg2, T_arg3, T_arg4, T_arg5, T_arg6>::type deduced_result_type;
		return make_new_tunnel_context<ASYNC, deduced_result_type>(m_disp, m_validity_tracker, 
			sigc::bind(this->functor_, _A_arg1, _A_arg2, _A_arg3, _A_arg4, _A_arg5, _A_arg6))->tunnel();
	}
	#endif

	template<typename T_arg1, typename T_arg2, typename T_arg3, typename T_arg4, typename T_arg5, typename T_arg6, typename T_arg7>
	typename deduce_result_type<T_arg1, T_arg2, T_arg3, T_arg4, T_arg5, T_arg6, T_arg7>::type
	operator()(T_arg1 _A_arg1, T_arg2 _A_arg2, T_arg3 _A_arg3, T_arg4 _A_arg4, T_arg5 _A_arg5, T_arg6 _A_arg6, T_arg7 _A_arg7)
	{
		typedef typename deduce_result_type<T_arg1, T_arg2, T_arg3, T_arg4, T_arg5, T_arg6, T_arg7>::type deduced_result_type;
		return make_new_tunnel_context<ASYNC, deduced_result_type>(m_disp, m_validity_tracker, 
			sigc::bind(this->functor_, _A_arg1, _A_arg2, _A_arg3, _A_arg4, _A_arg5, _A_arg6, _A_arg7))->tunnel();
	}

	#ifndef SIGC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD
	template<typename T_arg1, typename T_arg2, typename T_arg3, typename T_arg4, typename T_arg5, typename T_arg6, typename T_arg7>
	typename deduce_result_type<T_arg1, T_arg2, T_arg3, T_arg4, T_arg5, T_arg6, T_arg7>::type
	sun_forte_workaround(T_arg1 _A_arg1, T_arg2 _A_arg2, T_arg3 _A_arg3, T_arg4 _A_arg4, T_arg5 _A_arg5, T_arg6 _A_arg6, T_arg7 _A_arg7)
	{
		typedef typename deduce_result_type<T_arg1, T_arg2, T_arg3, T_arg4, T_arg5, T_arg6, T_arg7>::type deduced_result_type;
		return make_new_tunnel_context<ASYNC, deduced_result_type>(m_disp, m_validity_tracker, 
			sigc::bind(this->functor_, _A_arg1, _A_arg2, _A_arg3, _A_arg4, _A_arg5, _A_arg6, _A_arg7))->tunnel();
	}
	#endif


	
	/**	@short Constructs an adaptor that wraps the passed functor.
	 *	@param _A_func Functor to invoke at the other end of the tunnel 
	 *	from operator()().
	 *	@param dispatcher_change_is_cleanup Whether a dispatcher change should be
	 *	be treated as reason to destroy the tunnel
	 *	@note The passed in functor must be a "dispatchable functor", i.e.
	 *	a functor on a dispatchable's method or a functor created by 
	 *	sigx::dispatch_with.
	 */
	explicit tunnel_functor(typename sigc::type_trait<T_functor>::take _A_func): 
		sigc::adapts<T_functor>(_A_func), 
		// find the dispatchable object contained in the functor by 
		// stepping down the functor chain;
		// dispatchable_constraint finds the dispatchable and issues a compiler
		// error if the passed in functor is not a functor on a dispatchable's
		// method or does find a dispatchable in a SIGX_DISPATCH_WITH_FUNCTOR
		tunnel_base(internal::dispatchable_constraint<adaptor_type>::find_dispatchable(this->functor_))
	{}
	
	// implicit copy ctor is fine
	// implicit dtor is fine
	// implicit assignment operator is fine


	/**	@short	Activates validity tracking for sigc::trackableS and tracking of a dispatcher change 
	 *		(e.g. when a thread finishes its execution and resets its dispatcher)
	 *	@note	%activate_validity_tracking() assumes that the tunnel functor, all sigc::trackableS and the dispatcher/dispatchable 
	 *		are managed and accessed in the context of the calling thread.
	 */
	void activate_validity_tracking() const
	{
		validity_tracker().activate();

		// visit each trackable and bind the validity trackable to the sigc trackable and vice versa
		sigc::visit_each_type<sigc::trackable*>(
			sigc::mem_fun(validity_tracker(), &tunnel_validity_tracker::do_bind_to_trackable), 
			this->functor_
		);
	}
};

template<typename T_functor>
struct tunnel_functor<SYNC, T_functor>: public sigc::adapts<T_functor>, public tunnel_base
{
	typedef typename sigc::adapts<T_functor>::adaptor_type adaptor_type;
	typedef typename adaptor_type::result_type result_type;

	template<typename T_arg1 = void, typename T_arg2 = void, typename T_arg3 = void, typename T_arg4 = void, typename T_arg5 = void, typename T_arg6 = void, typename T_arg7 = void>
	struct deduce_result_type
	{
		// we could also use sigc::deduce_result_type but this saves another
		// level of indirection and does what sigc++ does internally
		typedef typename adaptor_type::template deduce_result_type<typename sigc::type_trait<T_arg1>::pass, typename sigc::type_trait<T_arg2>::pass, typename sigc::type_trait<T_arg3>::pass, typename sigc::type_trait<T_arg4>::pass, typename sigc::type_trait<T_arg5>::pass, typename sigc::type_trait<T_arg6>::pass, typename sigc::type_trait<T_arg7>::pass>::type type;
	};


	result_type operator()()
	{
		return make_new_tunnel_context<SYNC, result_type>(m_disp, m_validity_tracker, this->functor_)->tunnel();
	}

	#ifndef SIGC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD
	result_type sun_forte_workaround()
	{
		return make_new_tunnel_context<I_sync, result_type>(m_disp, m_validity_tracker, this->functor_)->tunnel();
	}
	#endif

	template<typename T_arg1>
	typename deduce_result_type<T_arg1>::type
	operator()(T_arg1 _A_arg1)
	{
		typedef typename deduce_result_type<T_arg1>::type deduced_result_type;
		return make_new_tunnel_context<SYNC, deduced_result_type>(m_disp, m_validity_tracker, 
			sigc::bind(this->functor_, sigx::ref(_A_arg1)))->tunnel();
	}

	#ifndef SIGC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD
	template<typename T_arg1>
	typename deduce_result_type<T_arg1>::type
	sun_forte_workaround(T_arg1 _A_arg1)
	{
		typedef typename deduce_result_type<T_arg1>::type deduced_result_type;
		return make_new_tunnel_context<SYNC, deduced_result_type>(m_disp, m_validity_tracker, 
			sigc::bind(this->functor_, sigx::ref(_A_arg1)))->tunnel();
	}
	#endif

	template<typename T_arg1, typename T_arg2>
	typename deduce_result_type<T_arg1, T_arg2>::type
	operator()(T_arg1 _A_arg1, T_arg2 _A_arg2)
	{
		typedef typename deduce_result_type<T_arg1, T_arg2>::type deduced_result_type;
		return make_new_tunnel_context<SYNC, deduced_result_type>(m_disp, m_validity_tracker, 
			sigc::bind(this->functor_, sigx::ref(_A_arg1), sigx::ref(_A_arg2)))->tunnel();
	}

	#ifndef SIGC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD
	template<typename T_arg1, typename T_arg2>
	typename deduce_result_type<T_arg1, T_arg2>::type
	sun_forte_workaround(T_arg1 _A_arg1, T_arg2 _A_arg2)
	{
		typedef typename deduce_result_type<T_arg1, T_arg2>::type deduced_result_type;
		return make_new_tunnel_context<SYNC, deduced_result_type>(m_disp, m_validity_tracker, 
			sigc::bind(this->functor_, sigx::ref(_A_arg1), sigx::ref(_A_arg2)))->tunnel();
	}
	#endif

	template<typename T_arg1, typename T_arg2, typename T_arg3>
	typename deduce_result_type<T_arg1, T_arg2, T_arg3>::type
	operator()(T_arg1 _A_arg1, T_arg2 _A_arg2, T_arg3 _A_arg3)
	{
		typedef typename deduce_result_type<T_arg1, T_arg2, T_arg3>::type deduced_result_type;
		return make_new_tunnel_context<SYNC, deduced_result_type>(m_disp, m_validity_tracker, 
			sigc::bind(this->functor_, sigx::ref(_A_arg1), sigx::ref(_A_arg2), sigx::ref(_A_arg3)))->tunnel();
	}

	#ifndef SIGC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD
	template<typename T_arg1, typename T_arg2, typename T_arg3>
	typename deduce_result_type<T_arg1, T_arg2, T_arg3>::type
	sun_forte_workaround(T_arg1 _A_arg1, T_arg2 _A_arg2, T_arg3 _A_arg3)
	{
		typedef typename deduce_result_type<T_arg1, T_arg2, T_arg3>::type deduced_result_type;
		return make_new_tunnel_context<SYNC, deduced_result_type>(m_disp, m_validity_tracker, 
			sigc::bind(this->functor_, sigx::ref(_A_arg1), sigx::ref(_A_arg2), sigx::ref(_A_arg3)))->tunnel();
	}
	#endif

	template<typename T_arg1, typename T_arg2, typename T_arg3, typename T_arg4>
	typename deduce_result_type<T_arg1, T_arg2, T_arg3, T_arg4>::type
	operator()(T_arg1 _A_arg1, T_arg2 _A_arg2, T_arg3 _A_arg3, T_arg4 _A_arg4)
	{
		typedef typename deduce_result_type<T_arg1, T_arg2, T_arg3, T_arg4>::type deduced_result_type;
		return make_new_tunnel_context<SYNC, deduced_result_type>(m_disp, m_validity_tracker, 
			sigc::bind(this->functor_, sigx::ref(_A_arg1), sigx::ref(_A_arg2), sigx::ref(_A_arg3), sigx::ref(_A_arg4)))->tunnel();
	}

	#ifndef SIGC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD
	template<typename T_arg1, typename T_arg2, typename T_arg3, typename T_arg4>
	typename deduce_result_type<T_arg1, T_arg2, T_arg3, T_arg4>::type
	sun_forte_workaround(T_arg1 _A_arg1, T_arg2 _A_arg2, T_arg3 _A_arg3, T_arg4 _A_arg4)
	{
		typedef typename deduce_result_type<T_arg1, T_arg2, T_arg3, T_arg4>::type deduced_result_type;
		return make_new_tunnel_context<SYNC, deduced_result_type>(m_disp, m_validity_tracker, 
			sigc::bind(this->functor_, sigx::ref(_A_arg1), sigx::ref(_A_arg2), sigx::ref(_A_arg3), sigx::ref(_A_arg4)))->tunnel();
	}
	#endif

	template<typename T_arg1, typename T_arg2, typename T_arg3, typename T_arg4, typename T_arg5>
	typename deduce_result_type<T_arg1, T_arg2, T_arg3, T_arg4, T_arg5>::type
	operator()(T_arg1 _A_arg1, T_arg2 _A_arg2, T_arg3 _A_arg3, T_arg4 _A_arg4, T_arg5 _A_arg5)
	{
		typedef typename deduce_result_type<T_arg1, T_arg2, T_arg3, T_arg4, T_arg5>::type deduced_result_type;
		return make_new_tunnel_context<SYNC, deduced_result_type>(m_disp, m_validity_tracker, 
			sigc::bind(this->functor_, sigx::ref(_A_arg1), sigx::ref(_A_arg2), sigx::ref(_A_arg3), sigx::ref(_A_arg4), sigx::ref(_A_arg5)))->tunnel();
	}

	#ifndef SIGC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD
	template<typename T_arg1, typename T_arg2, typename T_arg3, typename T_arg4, typename T_arg5>
	typename deduce_result_type<T_arg1, T_arg2, T_arg3, T_arg4, T_arg5>::type
	sun_forte_workaround(T_arg1 _A_arg1, T_arg2 _A_arg2, T_arg3 _A_arg3, T_arg4 _A_arg4, T_arg5 _A_arg5)
	{
		typedef typename deduce_result_type<T_arg1, T_arg2, T_arg3, T_arg4, T_arg5>::type deduced_result_type;
		return make_new_tunnel_context<SYNC, deduced_result_type>(m_disp, m_validity_tracker, 
			sigc::bind(this->functor_, sigx::ref(_A_arg1), sigx::ref(_A_arg2), sigx::ref(_A_arg3), sigx::ref(_A_arg4), sigx::ref(_A_arg5)))->tunnel();
	}
	#endif

	template<typename T_arg1, typename T_arg2, typename T_arg3, typename T_arg4, typename T_arg5, typename T_arg6>
	typename deduce_result_type<T_arg1, T_arg2, T_arg3, T_arg4, T_arg5, T_arg6>::type
	operator()(T_arg1 _A_arg1, T_arg2 _A_arg2, T_arg3 _A_arg3, T_arg4 _A_arg4, T_arg5 _A_arg5, T_arg6 _A_arg6)
	{
		typedef typename deduce_result_type<T_arg1, T_arg2, T_arg3, T_arg4, T_arg5, T_arg6>::type deduced_result_type;
		return make_new_tunnel_context<SYNC, deduced_result_type>(m_disp, m_validity_tracker, 
			sigc::bind(this->functor_, sigx::ref(_A_arg1), sigx::ref(_A_arg2), sigx::ref(_A_arg3), sigx::ref(_A_arg4), sigx::ref(_A_arg5), sigx::ref(_A_arg6)))->tunnel();
	}

	#ifndef SIGC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD
	template<typename T_arg1, typename T_arg2, typename T_arg3, typename T_arg4, typename T_arg5, typename T_arg6>
	typename deduce_result_type<T_arg1, T_arg2, T_arg3, T_arg4, T_arg5, T_arg6>::type
	sun_forte_workaround(T_arg1 _A_arg1, T_arg2 _A_arg2, T_arg3 _A_arg3, T_arg4 _A_arg4, T_arg5 _A_arg5, T_arg6 _A_arg6)
	{
		typedef typename deduce_result_type<T_arg1, T_arg2, T_arg3, T_arg4, T_arg5, T_arg6>::type deduced_result_type;
		return make_new_tunnel_context<SYNC, deduced_result_type>(m_disp, m_validity_tracker, 
			sigc::bind(this->functor_, sigx::ref(_A_arg1), sigx::ref(_A_arg2), sigx::ref(_A_arg3), sigx::ref(_A_arg4), sigx::ref(_A_arg5), sigx::ref(_A_arg6)))->tunnel();
	}
	#endif

	template<typename T_arg1, typename T_arg2, typename T_arg3, typename T_arg4, typename T_arg5, typename T_arg6, typename T_arg7>
	typename deduce_result_type<T_arg1, T_arg2, T_arg3, T_arg4, T_arg5, T_arg6, T_arg7>::type
	operator()(T_arg1 _A_arg1, T_arg2 _A_arg2, T_arg3 _A_arg3, T_arg4 _A_arg4, T_arg5 _A_arg5, T_arg6 _A_arg6, T_arg7 _A_arg7)
	{
		typedef typename deduce_result_type<T_arg1, T_arg2, T_arg3, T_arg4, T_arg5, T_arg6, T_arg7>::type deduced_result_type;
		return make_new_tunnel_context<SYNC, deduced_result_type>(m_disp, m_validity_tracker, 
			sigc::bind(this->functor_, sigx::ref(_A_arg1), sigx::ref(_A_arg2), sigx::ref(_A_arg3), sigx::ref(_A_arg4), sigx::ref(_A_arg5), sigx::ref(_A_arg6), sigx::ref(_A_arg7)))->tunnel();
	}

	#ifndef SIGC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD
	template<typename T_arg1, typename T_arg2, typename T_arg3, typename T_arg4, typename T_arg5, typename T_arg6, typename T_arg7>
	typename deduce_result_type<T_arg1, T_arg2, T_arg3, T_arg4, T_arg5, T_arg6, T_arg7>::type
	sun_forte_workaround(T_arg1 _A_arg1, T_arg2 _A_arg2, T_arg3 _A_arg3, T_arg4 _A_arg4, T_arg5 _A_arg5, T_arg6 _A_arg6, T_arg7 _A_arg7)
	{
		typedef typename deduce_result_type<T_arg1, T_arg2, T_arg3, T_arg4, T_arg5, T_arg6, T_arg7>::type deduced_result_type;
		return make_new_tunnel_context<SYNC, deduced_result_type>(m_disp, m_validity_tracker, 
			sigc::bind(this->functor_, sigx::ref(_A_arg1), sigx::ref(_A_arg2), sigx::ref(_A_arg3), sigx::ref(_A_arg4), sigx::ref(_A_arg5), sigx::ref(_A_arg6), sigx::ref(_A_arg7)))->tunnel();
	}
	#endif


	
	/**	@short Constructs an adaptor that wraps the passed functor.
	 *	@param _A_func Functor to invoke at the other end of the tunnel 
	 *	from operator()().
	 *	@param dispatcher_change_is_cleanup Whether a dispatcher change should be
	 *	be treated as reason to destroy the tunnel
	 *	@note The passed in functor must be a "dispatchable functor", i.e.
	 *	a functor on a dispatchable's method or a functor created by 
	 *	sigx::dispatch_with.
	 */
	explicit tunnel_functor(typename sigc::type_trait<T_functor>::take _A_func): 
		sigc::adapts<T_functor>(_A_func), 
		// find the dispatchable object contained in the functor by 
		// stepping down the functor chain;
		// dispatchable_constraint finds the dispatchable and issues a compiler
		// error if the passed in functor is not a functor on a dispatchable's
		// method or does find a dispatchable in a SIGX_DISPATCH_WITH_FUNCTOR
		tunnel_base(internal::dispatchable_constraint<adaptor_type>::find_dispatchable(this->functor_))
	{}
	
	// implicit copy ctor is fine
	// implicit dtor is fine
	// implicit assignment operator is fine


	/**	@short	Activates validity tracking for sigc::trackableS and tracking of a dispatcher change 
	 *		(e.g. when a thread finishes its execution and resets its dispatcher)
	 *	@note	%activate_validity_tracking() assumes that the tunnel functor, all sigc::trackableS and the dispatcher/dispatchable 
	 *		are managed and accessed in the context of the calling thread.
	 */
	void activate_validity_tracking() const
	{
		validity_tracker().activate();

		// visit each trackable and bind the validity trackable to the sigc trackable and vice versa
		sigc::visit_each_type<sigc::trackable*>(
			sigc::mem_fun(validity_tracker(), &tunnel_validity_tracker::do_bind_to_trackable), 
			this->functor_
		);
	}
};





/**	@short Binds a dispatchable explicitly to a functor.
 *	@note Use only with non-dispatchable functors (functors on functions or 
 *	methods of classes that do not derive from sigx::dispatchable)
 *	@ingroup Functors
 */
template<typename T_functor>
SIGX_DISPATCH_WITH_FUNCTOR(T_functor)
dispatch_with(const T_functor& _A_func, const shared_dispatchable& d)
{
	return sigc::bind(sigc::hide(_A_func), d);
}


/**	@short Opens an asynchronous tunnel on the specified functor.
 *	@ingroup Functors
 *	@param _A_func the functor on which the tunnel should be created
 *	@note @p _A_func must be a dispatchable functor, i.e. a member function
 *	of a class derived from sigx::dispatchable or a dispatchable functor explicitly created with 
 *	dispatch_with()
 *	@return Functor that executes @e _A_func on invokation in the context of the server thread.
 *	@ingroup Functors
 */
template<typename T_functor>
tunnel_functor<ASYNC, T_functor> 
open_tunnel(const T_functor& _A_func)
{
	return tunnel_functor<ASYNC, T_functor>(_A_func);
}

/**	@short Opens a synchronous tunnel on the specified functor.
 *	@ingroup Functors
 *	@param _A_func the functor on which the tunnel should be created
 *	@note @p _A_func must be a dispatchable functor, i.e. a member function
 *	of a class derived from sigx::dispatchable or a dispatchable functor explicitly created with 
 *	dispatch_with()
 *	@return Functor that executes @e _A_func on invokation in the context of the server thread.
 *	@ingroup Functors
 */
template<typename T_functor>
tunnel_functor<SYNC, T_functor> 
open_sync_tunnel(const T_functor& _A_func)
{
	return tunnel_functor<SYNC, T_functor>(_A_func);
}

/**	@short Opens an asynchronous tunnel on the specified functor with the dispatcher of the specified dispatchable.
 *	@ingroup Functors
 *	@param _A_func the functor on which the tunnel should be created
 *	@param d the dispatchable to operate on
 *	@note @p _A_func must be a dispatchable functor, i.e. a member function
 *	of a class derived from sigx::dispatchable or a dispatchable functor explicitly created with 
 *	dispatch_with()
 *	@return Functor that executes @e _A_func on invokation in the context of the server thread.
 *	@ingroup Functors
 */
template<typename T_functor>
tunnel_functor<ASYNC, SIGX_DISPATCH_WITH_FUNCTOR(T_functor)> 
open_tunnel_with(const T_functor& _A_func, const shared_dispatchable& d)
{
	return tunnel_functor<ASYNC, SIGX_DISPATCH_WITH_FUNCTOR(T_functor)>(dispatch_with(_A_func, d));
}

/**	@short Opens a synchronous tunnel on the specified functor with the dispatcher of the specified dispatchable.
 *	@ingroup Functors
 *	@param _A_func the functor on which the tunnel should be created
 *	@param d the dispatchable to operate on
 *	@note @p _A_func must be a dispatchable functor, i.e. a member function
 *	of a class derived from sigx::dispatchable or a dispatchable functor explicitly created with 
 *	dispatch_with()
 *	@return Functor that executes @e _A_func on invokation in the context of the server thread.
 *	@ingroup Functors
 */
template<typename T_functor>
tunnel_functor<SYNC, SIGX_DISPATCH_WITH_FUNCTOR(T_functor)> 
open_sync_tunnel_with(const T_functor& _A_func, const shared_dispatchable& d)
{
	return tunnel_functor<SYNC, SIGX_DISPATCH_WITH_FUNCTOR(T_functor)>(dispatch_with(_A_func, d));
}


} // namespace sigx


namespace sigc
{

/**	@short visit_each overload for tunnel functors, completely turning off the visit_each mechanism and thus turning off the trackable mechanism.
 *	
 *	This is necessary because binding a tunnel functor to a slot would access a trackable in a non-threadsafe manner.
 *	sigx++ activates validity tracking for trackables at the call site when the client thread connects to a signal through signal_wrapper<>::connect()
 */
template<typename T_action, typename T_functor, sigx::sync_type I_sync>
void visit_each(const T_action& /*_A_action*/, const sigx::tunnel_functor<I_sync, T_functor>& /*_A_func*/)
{
	// do nothing
}


} // namespace sigc
#endif /* _SIGXMACROS_TUNNEL_FUNCTOR_H_ */
