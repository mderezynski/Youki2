// -*- c++ -*-
/* Do not edit! -- generated file */


#ifndef _SIGXMACROS_SIGNAL_WRAPPER_H_
#define _SIGXMACROS_SIGNAL_WRAPPER_H_


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

#include <tr1/memory> // std::tr1::shared_ptr, std::auto_ptr
#include <sigc++/sigc++.h>
#include <glibmm/main.h>	// glibmm signals
#include <sigx/types.h>
#include <sigx/internal_types.h>
#include <sigx/signal_traits.h>
#include <sigx/signal_wrapper_base.h>
#include <sigx/signal_source_base.h>
#include <sigx/tunnel_functor.h>
#include <sigx/connection_handler.h>
#include <sigx/connection_wrapper.h>


/**	@defgroup signals Signals
 *	Threadsafe signals on top of the @ref Functors and @ref Dispatching
 *	facilities
 */


namespace sigx
{

	namespace internal
	{
	
/**	@short Counts a signal's arguments, default class
 */
template<typename T_signal>
struct count_signal_arguments
{
	static const int value = -1;
	static const int tspec = -1;
};


/**	@short Counts the arguments of an unnumbered sigc::signal
 */
template<typename T_return, typename T_arg1, typename T_arg2, typename T_arg3, typename T_arg4, typename T_arg5, typename T_arg6, typename T_arg7>
struct count_signal_arguments<sigc::signal<T_return, T_arg1, T_arg2, T_arg3, T_arg4, T_arg5, T_arg6, T_arg7> >
{
	// forward to count_arguments and not to count_signal_arguments, otherwise
	// we would get a false count if there is a another signal as the first 
	// argument of a signal, like: sigc::signal<void, sigc::signal<void> >
	static const int value = count_arguments<T_arg1, T_arg2, T_arg3, T_arg4, T_arg5, T_arg6, T_arg7>::value;
	static const int tspec = value;
};

/**	@short counts the arguments of a sigc::signal0
 */
template<typename T_return, typename T_accumulator>
struct count_signal_arguments<sigc::signal0<T_return, T_accumulator> >
{
	static const int value = 0;
	// template specialization for argument count needed
	static const int tspec = value;
};

/**	@short counts the arguments of a sigc::signal1
 */
template<typename T_return, typename T_arg1, typename T_accumulator>
struct count_signal_arguments<sigc::signal1<T_return, T_arg1, T_accumulator> >
{
	static const int value = 1;
	// template specialization for argument count needed
	static const int tspec = value;
};

/**	@short counts the arguments of a sigc::signal2
 */
template<typename T_return, typename T_arg1, typename T_arg2, typename T_accumulator>
struct count_signal_arguments<sigc::signal2<T_return, T_arg1, T_arg2, T_accumulator> >
{
	static const int value = 2;
	// template specialization for argument count needed
	static const int tspec = value;
};

/**	@short counts the arguments of a sigc::signal3
 */
template<typename T_return, typename T_arg1, typename T_arg2, typename T_arg3, typename T_accumulator>
struct count_signal_arguments<sigc::signal3<T_return, T_arg1, T_arg2, T_arg3, T_accumulator> >
{
	static const int value = 3;
	// template specialization for argument count needed
	static const int tspec = value;
};

/**	@short counts the arguments of a sigc::signal4
 */
template<typename T_return, typename T_arg1, typename T_arg2, typename T_arg3, typename T_arg4, typename T_accumulator>
struct count_signal_arguments<sigc::signal4<T_return, T_arg1, T_arg2, T_arg3, T_arg4, T_accumulator> >
{
	static const int value = 4;
	// template specialization for argument count needed
	static const int tspec = value;
};

/**	@short counts the arguments of a sigc::signal5
 */
template<typename T_return, typename T_arg1, typename T_arg2, typename T_arg3, typename T_arg4, typename T_arg5, typename T_accumulator>
struct count_signal_arguments<sigc::signal5<T_return, T_arg1, T_arg2, T_arg3, T_arg4, T_arg5, T_accumulator> >
{
	static const int value = 5;
	// template specialization for argument count needed
	static const int tspec = value;
};

/**	@short counts the arguments of a sigc::signal6
 */
template<typename T_return, typename T_arg1, typename T_arg2, typename T_arg3, typename T_arg4, typename T_arg5, typename T_arg6, typename T_accumulator>
struct count_signal_arguments<sigc::signal6<T_return, T_arg1, T_arg2, T_arg3, T_arg4, T_arg5, T_arg6, T_accumulator> >
{
	static const int value = 6;
	// template specialization for argument count needed
	static const int tspec = value;
};

/**	@short counts the arguments of a sigc::signal7
 */
template<typename T_return, typename T_arg1, typename T_arg2, typename T_arg3, typename T_arg4, typename T_arg5, typename T_arg6, typename T_arg7, typename T_accumulator>
struct count_signal_arguments<sigc::signal7<T_return, T_arg1, T_arg2, T_arg3, T_arg4, T_arg5, T_arg6, T_arg7, T_accumulator> >
{
	static const int value = 7;
	// template specialization for argument count needed
	static const int tspec = value;
};


/**	@short counts the arguments of a Glib::SignalProxy0
 */
template<typename T_return>
struct count_signal_arguments<Glib::SignalProxy0<T_return> >
{
	static const int value = 0;
	// template specialization for argument count not needed;
	// this allows us to group all SignProxyN signals together in one template
	// class
	static const int tspec = -1;
};

/**	@short counts the arguments of a Glib::SignalProxy1
 */
template<typename T_return, typename T_arg1>
struct count_signal_arguments<Glib::SignalProxy1<T_return, T_arg1> >
{
	static const int value = 1;
	// template specialization for argument count not needed;
	// this allows us to group all SignProxyN signals together in one template
	// class
	static const int tspec = -1;
};

/**	@short counts the arguments of a Glib::SignalProxy2
 */
template<typename T_return, typename T_arg1, typename T_arg2>
struct count_signal_arguments<Glib::SignalProxy2<T_return, T_arg1, T_arg2> >
{
	static const int value = 2;
	// template specialization for argument count not needed;
	// this allows us to group all SignProxyN signals together in one template
	// class
	static const int tspec = -1;
};

/**	@short counts the arguments of a Glib::SignalProxy3
 */
template<typename T_return, typename T_arg1, typename T_arg2, typename T_arg3>
struct count_signal_arguments<Glib::SignalProxy3<T_return, T_arg1, T_arg2, T_arg3> >
{
	static const int value = 3;
	// template specialization for argument count not needed;
	// this allows us to group all SignProxyN signals together in one template
	// class
	static const int tspec = -1;
};

/**	@short counts the arguments of a Glib::SignalProxy4
 */
template<typename T_return, typename T_arg1, typename T_arg2, typename T_arg3, typename T_arg4>
struct count_signal_arguments<Glib::SignalProxy4<T_return, T_arg1, T_arg2, T_arg3, T_arg4> >
{
	static const int value = 4;
	// template specialization for argument count not needed;
	// this allows us to group all SignProxyN signals together in one template
	// class
	static const int tspec = -1;
};

/**	@short counts the arguments of a Glib::SignalProxy5
 */
template<typename T_return, typename T_arg1, typename T_arg2, typename T_arg3, typename T_arg4, typename T_arg5>
struct count_signal_arguments<Glib::SignalProxy5<T_return, T_arg1, T_arg2, T_arg3, T_arg4, T_arg5> >
{
	static const int value = 5;
	// template specialization for argument count not needed;
	// this allows us to group all SignProxyN signals together in one template
	// class
	static const int tspec = -1;
};

/**	@short counts the arguments of a Glib::SignalProxy6
 */
template<typename T_return, typename T_arg1, typename T_arg2, typename T_arg3, typename T_arg4, typename T_arg5, typename T_arg6>
struct count_signal_arguments<Glib::SignalProxy6<T_return, T_arg1, T_arg2, T_arg3, T_arg4, T_arg5, T_arg6> >
{
	static const int value = 6;
	// template specialization for argument count not needed;
	// this allows us to group all SignProxyN signals together in one template
	// class
	static const int tspec = -1;
};



/**	@short Counts the arguments of a Glib::SignalIdle
 */
template<>
struct count_signal_arguments<Glib::SignalIdle>
{
	static const int value = 0;
	static const int tspec = value;
};

/**	@short Counts the arguments of a Glib::SignalTimeout
 */
template<>
struct count_signal_arguments<Glib::SignalTimeout>
{
	static const int value = 0;
	static const int tspec = value;
};

/**	@short Counts the arguments of a Glib::SignalIO
 */
template<>
struct count_signal_arguments<Glib::SignalIO>
{
	static const int value = 1;
	static const int tspec = value;
};

/**	@short Counts the arguments of a Glib::SignalChildWatch
 */
template<>
struct count_signal_arguments<Glib::SignalChildWatch>
{
	static const int value = 2;
	static const int tspec = value;
};


	} // namespace internal


/**	@short	A threadsafe wrapper for sigc signals, Glib
 *	signals or theoretically any other type of signal.
 *	
 *	sigx signals have a shared signal source that exists in the context of
 *	another thread. This signal source has access to the signal. Access (e.g. 
 *	connecting) is regulated by a dispatcher running in the context of the 
 *	thread owning that signal.
 */
template<typename T_signal, internal::signal_group I_oneof = internal::signal_type_trait<T_signal>::type, int I_arg_count = internal::count_signal_arguments<T_signal>::tspec>
class signal_wrapper;

/**	@short A threadsafe wrapper for any sigc signal with 0 argument(s).
 *	
 *	@ingroup signals
 */
template<typename T_signal>
class signal_wrapper<T_signal, internal::SIGGROUP_SIGC, 0>: public signal_wrapper_base
{
public:
	//BOOST_STATIC_ASSERT((internal::count_signal_arguments<T_signal>::value == 0));
	static const int argument_count = 0;
	static const internal::signal_group signal_group = internal::SIGGROUP_SIGC;
	typedef T_signal signal_type;
	typedef signal_wrapper<signal_type, signal_group, argument_count> this_type;
	typedef typename signal_type::slot_type slot_type;
	typedef typename signal_type::result_type result_type;
	
	typedef signal_type (*fp_sig_getter_type)(signal_source_ptr);
	
protected:
	typedef sigc::bound_const_mem_functor0<signal_source_base::hook, signal_source_base> make_slot_f1;
	typedef sigc::retype_return_functor<fp_sig_getter_type, make_slot_f1> make_slot_f2;
	typedef sigc::const_mem_functor0<signal_source_ptr, std::tr1::shared_ptr<signal_source_base> > make_slot_f3;
	typedef sigc::bind_functor<-1, make_slot_f3, std::tr1::shared_ptr<signal_source_base> > make_slot_f4;
	typedef sigc::compose1_functor<make_slot_f2, make_slot_f4> make_slot_composed1_functor_type;
	typedef sigc::const_mem_functor0<result_type, signal_type> make_slot_emit_functor_type;
	typedef sigc::compose1_functor<make_slot_emit_functor_type, make_slot_composed1_functor_type> make_slot_composed2_functor_type;
	typedef SIGX_DISPATCH_WITH_FUNCTOR(make_slot_composed2_functor_type) make_slot_functor_type;


public:
	/**	@short Constructs an empty signal_wrapper.
	 *	@throw	Might throw a std::bad_alloc exception (from dispatchable's ctor)
	 */
	signal_wrapper(): signal_wrapper_base()
	{}

	/**	@short Creates a signal_wrapper from a signal source.
	 *	@param _A_disp The dispatchable to operate on
	 *	@param _A_sigsource A shared pointer to the server 
	 *	thread's signal source.
	 */
	signal_wrapper(const shared_dispatchable& _A_disp, const std::tr1::shared_ptr<signal_source_base>& _A_sigsource) throw(): 
		signal_wrapper_base(_A_disp, _A_sigsource)
	{}
	
	/**	@short Connects a functor, tunnels it automatically if not yet 
	 *	tunneled and activates validity tracking for sigc::trackableS.
	 *	
	 *	auto tunneling is successful only if the passed in functor is a
	 *	"dispatchable" functor, i.e. a functor on a dispatchable's method
	 *	or one explicitly created with "dispatch_with".
	 *	You will get compiler errors if the dispatchable can't be deduced from the 
	 *	passed in functor.
	 *	
	 *	@note At the moment it is only possible to pass in a non-tunneled functor or
	 *	a toplevel tunneled functor due to the fact that the visit_each mechanism
	 *	is turned off for the tunnel functor (otherwise there would be the problem
	 *	of not threadsafe access to the sigc::trackable base of the of a 
	 *	dispatchable object..
	 *	
	 *	@note passed in functor must not be a slot or adapt a slot;
	 *	we have to apply this restriction because slots might have bound
	 *	trackables that can cause non-threadsafe access to the passed in slot
	 *	which will live in the context of the server thread.
	 *	
	 *	@attention	All sigc::trackableS and the original dispatchable contained 
	 *	in the passed functor must belong to the context of the calling thread.
	 *	
	 *	@return sigx::connection_wrapper A threadsafe connection wrapper
	 *	@note asynchronous
	 */
	template<typename T_functor>
	connection_wrapper connect(const T_functor& _A_func) const
	{
		return	signal_wrapper_base::connect(
					_A_func, 
					sigc::ptr_fun(&typed_connection_handler<signal_type, signal_group>::connect)
				);
	}

	/**	@short emits the signal on the other side of the tunnel.
	 */
	template<sync_type I_sync>
	result_type emit() const
	{
		return open_tunnel_with<I_sync>(
			// calls T_signal::*emit
			sigc::compose(
				sigc::mem_fun(&signal_type::emit), 
				// getter for the T_signal
				sigc::compose(
					sigc::retype_return<fp_sig_getter_type>(
						sigc::mem_fun(m_sigsource.get(), &signal_source_base::getter)
					), 
					// this makes a copy of the shared signal source and thus 
					// shares it within the tunnel functor ensuring the lifetime 
					// of the shared signal source
					sigc::bind(
						sigc::mem_fun(&std::tr1::shared_ptr<signal_source_base>::operator *), 
						m_sigsource
					)
				)
			), 
			m_disp
		)();
	}

	/**	@short emits the signal asynchronously.
	 */
	result_type emit() const
	{
		return emit<ASYNC>();
	}

	/**	@short emits the signal synchronously.
	 */
	result_type emit_sync() const
	{
		return emit<SYNC>();
	}

	/**	see emit()
	 */
	result_type operator()() const
	{
		return emit<ASYNC>();
	}


	/**	@short creates a tunnel_functor that emits the signal when invoked
	 */
	template<sync_type I_sync>
	tunnel_functor<I_sync, make_slot_functor_type> make_slot() const
	{
		typedef tunnel_functor<I_sync, make_slot_functor_type> tunnel_funtor_type;
		return tunnel_funtor_type(
			dispatch_with(
				// calls T_signal::*emit
				sigc::compose(
					&signal_type::emit, 
					// getter for the T_signal
					sigc::compose(
						sigc::retype_return<fp_sig_getter_type>(
							sigc::mem_fun(m_sigsource.get(), &signal_source_base::getter)
						), 
						sigc::bind(
							sigc::mem_fun(&std::tr1::shared_ptr<signal_source_base>::operator *), 
							m_sigsource
						)
					)
				), 
				m_disp
			));
	}

	/**	@short creates an asynchronous tunnel_functor that emits the signal 
	 *	when invoked
	 */
	tunnel_functor<ASYNC, make_slot_functor_type> make_slot() const
	{
		return make_slot<ASYNC>();
	}

	/**	@short creates a synchronous tunnel_functor that emits the signal 
	 *	when invoked
	 */
	tunnel_functor<SYNC, make_slot_functor_type> make_slot_sync() const
	{
		return make_slot<SYNC>();
	}
};

/**	@short A threadsafe wrapper for any sigc signal with 1 argument(s).
 *	
 *	@ingroup signals
 */
template<typename T_signal>
class signal_wrapper<T_signal, internal::SIGGROUP_SIGC, 1>: public signal_wrapper_base
{
public:
	//BOOST_STATIC_ASSERT((internal::count_signal_arguments<T_signal>::value == 1));
	static const int argument_count = 1;
	static const internal::signal_group signal_group = internal::SIGGROUP_SIGC;
	typedef T_signal signal_type;
	typedef signal_wrapper<signal_type, signal_group, argument_count> this_type;
	typedef typename signal_type::slot_type slot_type;
	typedef typename signal_type::result_type result_type;
		typedef typename slot_type::arg1_type_ arg1_type_;

	typedef signal_type (*fp_sig_getter_type)(signal_source_ptr);
	
protected:
	typedef sigc::bound_const_mem_functor0<signal_source_base::hook, signal_source_base> make_slot_f1;
	typedef sigc::retype_return_functor<fp_sig_getter_type, make_slot_f1> make_slot_f2;
	typedef sigc::const_mem_functor0<signal_source_ptr, std::tr1::shared_ptr<signal_source_base> > make_slot_f3;
	typedef sigc::bind_functor<-1, make_slot_f3, std::tr1::shared_ptr<signal_source_base> > make_slot_f4;
	typedef sigc::compose1_functor<make_slot_f2, make_slot_f4> make_slot_composed1_functor_type;
	typedef sigc::const_mem_functor1<result_type, signal_type, arg1_type_> make_slot_emit_functor_type;
	typedef sigc::compose1_functor<make_slot_emit_functor_type, make_slot_composed1_functor_type> make_slot_composed2_functor_type;
	typedef SIGX_DISPATCH_WITH_FUNCTOR(make_slot_composed2_functor_type) make_slot_functor_type;


public:
	/**	@short Constructs an empty signal_wrapper.
	 *	@throw	Might throw a std::bad_alloc exception (from dispatchable's ctor)
	 */
	signal_wrapper(): signal_wrapper_base()
	{}

	/**	@short Creates a signal_wrapper from a signal source.
	 *	@param _A_disp The dispatchable to operate on
	 *	@param _A_sigsource A shared pointer to the server 
	 *	thread's signal source.
	 */
	signal_wrapper(const shared_dispatchable& _A_disp, const std::tr1::shared_ptr<signal_source_base>& _A_sigsource) throw(): 
		signal_wrapper_base(_A_disp, _A_sigsource)
	{}
	
	/**	@short Connects a functor, tunnels it automatically if not yet 
	 *	tunneled and activates validity tracking for sigc::trackableS.
	 *	
	 *	auto tunneling is successful only if the passed in functor is a
	 *	"dispatchable" functor, i.e. a functor on a dispatchable's method
	 *	or one explicitly created with "dispatch_with".
	 *	You will get compiler errors if the dispatchable can't be deduced from the 
	 *	passed in functor.
	 *	
	 *	@note At the moment it is only possible to pass in a non-tunneled functor or
	 *	a toplevel tunneled functor due to the fact that the visit_each mechanism
	 *	is turned off for the tunnel functor (otherwise there would be the problem
	 *	of not threadsafe access to the sigc::trackable base of the of a 
	 *	dispatchable object..
	 *	
	 *	@note passed in functor must not be a slot or adapt a slot;
	 *	we have to apply this restriction because slots might have bound
	 *	trackables that can cause non-threadsafe access to the passed in slot
	 *	which will live in the context of the server thread.
	 *	
	 *	@attention	All sigc::trackableS and the original dispatchable contained 
	 *	in the passed functor must belong to the context of the calling thread.
	 *	
	 *	@return sigx::connection_wrapper A threadsafe connection wrapper
	 *	@note asynchronous
	 */
	template<typename T_functor>
	connection_wrapper connect(const T_functor& _A_func) const
	{
		return	signal_wrapper_base::connect(
					_A_func, 
					sigc::ptr_fun(&typed_connection_handler<signal_type, signal_group>::connect)
				);
	}

	/**	@short emits the signal on the other side of the tunnel.
	 */
	template<sync_type I_sync>
	result_type emit(arg1_type_ _A_a1) const
	{
		return open_tunnel_with<I_sync>(
			// calls T_signal::*emit
			sigc::compose(
				sigc::mem_fun(&signal_type::emit), 
				// getter for the T_signal
				sigc::compose(
					sigc::retype_return<fp_sig_getter_type>(
						sigc::mem_fun(m_sigsource.get(), &signal_source_base::getter)
					), 
					// this makes a copy of the shared signal source and thus 
					// shares it within the tunnel functor ensuring the lifetime 
					// of the shared signal source
					sigc::bind(
						sigc::mem_fun(&std::tr1::shared_ptr<signal_source_base>::operator *), 
						m_sigsource
					)
				)
			), 
			m_disp
		)(_A_a1);
	}

	/**	@short emits the signal asynchronously.
	 */
	result_type emit(arg1_type_ _A_a1) const
	{
		return emit<ASYNC>(_A_a1);
	}

	/**	@short emits the signal synchronously.
	 */
	result_type emit_sync(arg1_type_ _A_a1) const
	{
		return emit<SYNC>(_A_a1);
	}

	/**	see emit(arg1_type_)
	 */
	result_type operator()(arg1_type_ _A_a1) const
	{
		return emit<ASYNC>(_A_a1);
	}


	/**	@short creates a tunnel_functor that emits the signal when invoked
	 */
	template<sync_type I_sync>
	tunnel_functor<I_sync, make_slot_functor_type> make_slot() const
	{
		typedef tunnel_functor<I_sync, make_slot_functor_type> tunnel_funtor_type;
		return tunnel_funtor_type(
			dispatch_with(
				// calls T_signal::*emit
				sigc::compose(
					&signal_type::emit, 
					// getter for the T_signal
					sigc::compose(
						sigc::retype_return<fp_sig_getter_type>(
							sigc::mem_fun(m_sigsource.get(), &signal_source_base::getter)
						), 
						sigc::bind(
							sigc::mem_fun(&std::tr1::shared_ptr<signal_source_base>::operator *), 
							m_sigsource
						)
					)
				), 
				m_disp
			));
	}

	/**	@short creates an asynchronous tunnel_functor that emits the signal 
	 *	when invoked
	 */
	tunnel_functor<ASYNC, make_slot_functor_type> make_slot() const
	{
		return make_slot<ASYNC>();
	}

	/**	@short creates a synchronous tunnel_functor that emits the signal 
	 *	when invoked
	 */
	tunnel_functor<SYNC, make_slot_functor_type> make_slot_sync() const
	{
		return make_slot<SYNC>();
	}
};

/**	@short A threadsafe wrapper for any sigc signal with 2 argument(s).
 *	
 *	@ingroup signals
 */
template<typename T_signal>
class signal_wrapper<T_signal, internal::SIGGROUP_SIGC, 2>: public signal_wrapper_base
{
public:
	//BOOST_STATIC_ASSERT((internal::count_signal_arguments<T_signal>::value == 2));
	static const int argument_count = 2;
	static const internal::signal_group signal_group = internal::SIGGROUP_SIGC;
	typedef T_signal signal_type;
	typedef signal_wrapper<signal_type, signal_group, argument_count> this_type;
	typedef typename signal_type::slot_type slot_type;
	typedef typename signal_type::result_type result_type;
		typedef typename slot_type::arg1_type_ arg1_type_;
	typedef typename slot_type::arg2_type_ arg2_type_;

	typedef signal_type (*fp_sig_getter_type)(signal_source_ptr);
	
protected:
	typedef sigc::bound_const_mem_functor0<signal_source_base::hook, signal_source_base> make_slot_f1;
	typedef sigc::retype_return_functor<fp_sig_getter_type, make_slot_f1> make_slot_f2;
	typedef sigc::const_mem_functor0<signal_source_ptr, std::tr1::shared_ptr<signal_source_base> > make_slot_f3;
	typedef sigc::bind_functor<-1, make_slot_f3, std::tr1::shared_ptr<signal_source_base> > make_slot_f4;
	typedef sigc::compose1_functor<make_slot_f2, make_slot_f4> make_slot_composed1_functor_type;
	typedef sigc::const_mem_functor2<result_type, signal_type, arg1_type_, arg2_type_> make_slot_emit_functor_type;
	typedef sigc::compose1_functor<make_slot_emit_functor_type, make_slot_composed1_functor_type> make_slot_composed2_functor_type;
	typedef SIGX_DISPATCH_WITH_FUNCTOR(make_slot_composed2_functor_type) make_slot_functor_type;


public:
	/**	@short Constructs an empty signal_wrapper.
	 *	@throw	Might throw a std::bad_alloc exception (from dispatchable's ctor)
	 */
	signal_wrapper(): signal_wrapper_base()
	{}

	/**	@short Creates a signal_wrapper from a signal source.
	 *	@param _A_disp The dispatchable to operate on
	 *	@param _A_sigsource A shared pointer to the server 
	 *	thread's signal source.
	 */
	signal_wrapper(const shared_dispatchable& _A_disp, const std::tr1::shared_ptr<signal_source_base>& _A_sigsource) throw(): 
		signal_wrapper_base(_A_disp, _A_sigsource)
	{}
	
	/**	@short Connects a functor, tunnels it automatically if not yet 
	 *	tunneled and activates validity tracking for sigc::trackableS.
	 *	
	 *	auto tunneling is successful only if the passed in functor is a
	 *	"dispatchable" functor, i.e. a functor on a dispatchable's method
	 *	or one explicitly created with "dispatch_with".
	 *	You will get compiler errors if the dispatchable can't be deduced from the 
	 *	passed in functor.
	 *	
	 *	@note At the moment it is only possible to pass in a non-tunneled functor or
	 *	a toplevel tunneled functor due to the fact that the visit_each mechanism
	 *	is turned off for the tunnel functor (otherwise there would be the problem
	 *	of not threadsafe access to the sigc::trackable base of the of a 
	 *	dispatchable object..
	 *	
	 *	@note passed in functor must not be a slot or adapt a slot;
	 *	we have to apply this restriction because slots might have bound
	 *	trackables that can cause non-threadsafe access to the passed in slot
	 *	which will live in the context of the server thread.
	 *	
	 *	@attention	All sigc::trackableS and the original dispatchable contained 
	 *	in the passed functor must belong to the context of the calling thread.
	 *	
	 *	@return sigx::connection_wrapper A threadsafe connection wrapper
	 *	@note asynchronous
	 */
	template<typename T_functor>
	connection_wrapper connect(const T_functor& _A_func) const
	{
		return	signal_wrapper_base::connect(
					_A_func, 
					sigc::ptr_fun(&typed_connection_handler<signal_type, signal_group>::connect)
				);
	}

	/**	@short emits the signal on the other side of the tunnel.
	 */
	template<sync_type I_sync>
	result_type emit(arg1_type_ _A_a1, arg2_type_ _A_a2) const
	{
		return open_tunnel_with<I_sync>(
			// calls T_signal::*emit
			sigc::compose(
				sigc::mem_fun(&signal_type::emit), 
				// getter for the T_signal
				sigc::compose(
					sigc::retype_return<fp_sig_getter_type>(
						sigc::mem_fun(m_sigsource.get(), &signal_source_base::getter)
					), 
					// this makes a copy of the shared signal source and thus 
					// shares it within the tunnel functor ensuring the lifetime 
					// of the shared signal source
					sigc::bind(
						sigc::mem_fun(&std::tr1::shared_ptr<signal_source_base>::operator *), 
						m_sigsource
					)
				)
			), 
			m_disp
		)(_A_a1, _A_a2);
	}

	/**	@short emits the signal asynchronously.
	 */
	result_type emit(arg1_type_ _A_a1, arg2_type_ _A_a2) const
	{
		return emit<ASYNC>(_A_a1, _A_a2);
	}

	/**	@short emits the signal synchronously.
	 */
	result_type emit_sync(arg1_type_ _A_a1, arg2_type_ _A_a2) const
	{
		return emit<SYNC>(_A_a1, _A_a2);
	}

	/**	see emit(arg1_type_, arg2_type_)
	 */
	result_type operator()(arg1_type_ _A_a1, arg2_type_ _A_a2) const
	{
		return emit<ASYNC>(_A_a1, _A_a2);
	}


	/**	@short creates a tunnel_functor that emits the signal when invoked
	 */
	template<sync_type I_sync>
	tunnel_functor<I_sync, make_slot_functor_type> make_slot() const
	{
		typedef tunnel_functor<I_sync, make_slot_functor_type> tunnel_funtor_type;
		return tunnel_funtor_type(
			dispatch_with(
				// calls T_signal::*emit
				sigc::compose(
					&signal_type::emit, 
					// getter for the T_signal
					sigc::compose(
						sigc::retype_return<fp_sig_getter_type>(
							sigc::mem_fun(m_sigsource.get(), &signal_source_base::getter)
						), 
						sigc::bind(
							sigc::mem_fun(&std::tr1::shared_ptr<signal_source_base>::operator *), 
							m_sigsource
						)
					)
				), 
				m_disp
			));
	}

	/**	@short creates an asynchronous tunnel_functor that emits the signal 
	 *	when invoked
	 */
	tunnel_functor<ASYNC, make_slot_functor_type> make_slot() const
	{
		return make_slot<ASYNC>();
	}

	/**	@short creates a synchronous tunnel_functor that emits the signal 
	 *	when invoked
	 */
	tunnel_functor<SYNC, make_slot_functor_type> make_slot_sync() const
	{
		return make_slot<SYNC>();
	}
};

/**	@short A threadsafe wrapper for any sigc signal with 3 argument(s).
 *	
 *	@ingroup signals
 */
template<typename T_signal>
class signal_wrapper<T_signal, internal::SIGGROUP_SIGC, 3>: public signal_wrapper_base
{
public:
	//BOOST_STATIC_ASSERT((internal::count_signal_arguments<T_signal>::value == 3));
	static const int argument_count = 3;
	static const internal::signal_group signal_group = internal::SIGGROUP_SIGC;
	typedef T_signal signal_type;
	typedef signal_wrapper<signal_type, signal_group, argument_count> this_type;
	typedef typename signal_type::slot_type slot_type;
	typedef typename signal_type::result_type result_type;
		typedef typename slot_type::arg1_type_ arg1_type_;
	typedef typename slot_type::arg2_type_ arg2_type_;
	typedef typename slot_type::arg3_type_ arg3_type_;

	typedef signal_type (*fp_sig_getter_type)(signal_source_ptr);
	
protected:
	typedef sigc::bound_const_mem_functor0<signal_source_base::hook, signal_source_base> make_slot_f1;
	typedef sigc::retype_return_functor<fp_sig_getter_type, make_slot_f1> make_slot_f2;
	typedef sigc::const_mem_functor0<signal_source_ptr, std::tr1::shared_ptr<signal_source_base> > make_slot_f3;
	typedef sigc::bind_functor<-1, make_slot_f3, std::tr1::shared_ptr<signal_source_base> > make_slot_f4;
	typedef sigc::compose1_functor<make_slot_f2, make_slot_f4> make_slot_composed1_functor_type;
	typedef sigc::const_mem_functor3<result_type, signal_type, arg1_type_, arg2_type_, arg3_type_> make_slot_emit_functor_type;
	typedef sigc::compose1_functor<make_slot_emit_functor_type, make_slot_composed1_functor_type> make_slot_composed2_functor_type;
	typedef SIGX_DISPATCH_WITH_FUNCTOR(make_slot_composed2_functor_type) make_slot_functor_type;


public:
	/**	@short Constructs an empty signal_wrapper.
	 *	@throw	Might throw a std::bad_alloc exception (from dispatchable's ctor)
	 */
	signal_wrapper(): signal_wrapper_base()
	{}

	/**	@short Creates a signal_wrapper from a signal source.
	 *	@param _A_disp The dispatchable to operate on
	 *	@param _A_sigsource A shared pointer to the server 
	 *	thread's signal source.
	 */
	signal_wrapper(const shared_dispatchable& _A_disp, const std::tr1::shared_ptr<signal_source_base>& _A_sigsource) throw(): 
		signal_wrapper_base(_A_disp, _A_sigsource)
	{}
	
	/**	@short Connects a functor, tunnels it automatically if not yet 
	 *	tunneled and activates validity tracking for sigc::trackableS.
	 *	
	 *	auto tunneling is successful only if the passed in functor is a
	 *	"dispatchable" functor, i.e. a functor on a dispatchable's method
	 *	or one explicitly created with "dispatch_with".
	 *	You will get compiler errors if the dispatchable can't be deduced from the 
	 *	passed in functor.
	 *	
	 *	@note At the moment it is only possible to pass in a non-tunneled functor or
	 *	a toplevel tunneled functor due to the fact that the visit_each mechanism
	 *	is turned off for the tunnel functor (otherwise there would be the problem
	 *	of not threadsafe access to the sigc::trackable base of the of a 
	 *	dispatchable object..
	 *	
	 *	@note passed in functor must not be a slot or adapt a slot;
	 *	we have to apply this restriction because slots might have bound
	 *	trackables that can cause non-threadsafe access to the passed in slot
	 *	which will live in the context of the server thread.
	 *	
	 *	@attention	All sigc::trackableS and the original dispatchable contained 
	 *	in the passed functor must belong to the context of the calling thread.
	 *	
	 *	@return sigx::connection_wrapper A threadsafe connection wrapper
	 *	@note asynchronous
	 */
	template<typename T_functor>
	connection_wrapper connect(const T_functor& _A_func) const
	{
		return	signal_wrapper_base::connect(
					_A_func, 
					sigc::ptr_fun(&typed_connection_handler<signal_type, signal_group>::connect)
				);
	}

	/**	@short emits the signal on the other side of the tunnel.
	 */
	template<sync_type I_sync>
	result_type emit(arg1_type_ _A_a1, arg2_type_ _A_a2, arg3_type_ _A_a3) const
	{
		return open_tunnel_with<I_sync>(
			// calls T_signal::*emit
			sigc::compose(
				sigc::mem_fun(&signal_type::emit), 
				// getter for the T_signal
				sigc::compose(
					sigc::retype_return<fp_sig_getter_type>(
						sigc::mem_fun(m_sigsource.get(), &signal_source_base::getter)
					), 
					// this makes a copy of the shared signal source and thus 
					// shares it within the tunnel functor ensuring the lifetime 
					// of the shared signal source
					sigc::bind(
						sigc::mem_fun(&std::tr1::shared_ptr<signal_source_base>::operator *), 
						m_sigsource
					)
				)
			), 
			m_disp
		)(_A_a1, _A_a2, _A_a3);
	}

	/**	@short emits the signal asynchronously.
	 */
	result_type emit(arg1_type_ _A_a1, arg2_type_ _A_a2, arg3_type_ _A_a3) const
	{
		return emit<ASYNC>(_A_a1, _A_a2, _A_a3);
	}

	/**	@short emits the signal synchronously.
	 */
	result_type emit_sync(arg1_type_ _A_a1, arg2_type_ _A_a2, arg3_type_ _A_a3) const
	{
		return emit<SYNC>(_A_a1, _A_a2, _A_a3);
	}

	/**	see emit(arg1_type_, arg2_type_, arg3_type_)
	 */
	result_type operator()(arg1_type_ _A_a1, arg2_type_ _A_a2, arg3_type_ _A_a3) const
	{
		return emit<ASYNC>(_A_a1, _A_a2, _A_a3);
	}


	/**	@short creates a tunnel_functor that emits the signal when invoked
	 */
	template<sync_type I_sync>
	tunnel_functor<I_sync, make_slot_functor_type> make_slot() const
	{
		typedef tunnel_functor<I_sync, make_slot_functor_type> tunnel_funtor_type;
		return tunnel_funtor_type(
			dispatch_with(
				// calls T_signal::*emit
				sigc::compose(
					&signal_type::emit, 
					// getter for the T_signal
					sigc::compose(
						sigc::retype_return<fp_sig_getter_type>(
							sigc::mem_fun(m_sigsource.get(), &signal_source_base::getter)
						), 
						sigc::bind(
							sigc::mem_fun(&std::tr1::shared_ptr<signal_source_base>::operator *), 
							m_sigsource
						)
					)
				), 
				m_disp
			));
	}

	/**	@short creates an asynchronous tunnel_functor that emits the signal 
	 *	when invoked
	 */
	tunnel_functor<ASYNC, make_slot_functor_type> make_slot() const
	{
		return make_slot<ASYNC>();
	}

	/**	@short creates a synchronous tunnel_functor that emits the signal 
	 *	when invoked
	 */
	tunnel_functor<SYNC, make_slot_functor_type> make_slot_sync() const
	{
		return make_slot<SYNC>();
	}
};

/**	@short A threadsafe wrapper for any sigc signal with 4 argument(s).
 *	
 *	@ingroup signals
 */
template<typename T_signal>
class signal_wrapper<T_signal, internal::SIGGROUP_SIGC, 4>: public signal_wrapper_base
{
public:
	//BOOST_STATIC_ASSERT((internal::count_signal_arguments<T_signal>::value == 4));
	static const int argument_count = 4;
	static const internal::signal_group signal_group = internal::SIGGROUP_SIGC;
	typedef T_signal signal_type;
	typedef signal_wrapper<signal_type, signal_group, argument_count> this_type;
	typedef typename signal_type::slot_type slot_type;
	typedef typename signal_type::result_type result_type;
		typedef typename slot_type::arg1_type_ arg1_type_;
	typedef typename slot_type::arg2_type_ arg2_type_;
	typedef typename slot_type::arg3_type_ arg3_type_;
	typedef typename slot_type::arg4_type_ arg4_type_;

	typedef signal_type (*fp_sig_getter_type)(signal_source_ptr);
	
protected:
	typedef sigc::bound_const_mem_functor0<signal_source_base::hook, signal_source_base> make_slot_f1;
	typedef sigc::retype_return_functor<fp_sig_getter_type, make_slot_f1> make_slot_f2;
	typedef sigc::const_mem_functor0<signal_source_ptr, std::tr1::shared_ptr<signal_source_base> > make_slot_f3;
	typedef sigc::bind_functor<-1, make_slot_f3, std::tr1::shared_ptr<signal_source_base> > make_slot_f4;
	typedef sigc::compose1_functor<make_slot_f2, make_slot_f4> make_slot_composed1_functor_type;
	typedef sigc::const_mem_functor4<result_type, signal_type, arg1_type_, arg2_type_, arg3_type_, arg4_type_> make_slot_emit_functor_type;
	typedef sigc::compose1_functor<make_slot_emit_functor_type, make_slot_composed1_functor_type> make_slot_composed2_functor_type;
	typedef SIGX_DISPATCH_WITH_FUNCTOR(make_slot_composed2_functor_type) make_slot_functor_type;


public:
	/**	@short Constructs an empty signal_wrapper.
	 *	@throw	Might throw a std::bad_alloc exception (from dispatchable's ctor)
	 */
	signal_wrapper(): signal_wrapper_base()
	{}

	/**	@short Creates a signal_wrapper from a signal source.
	 *	@param _A_disp The dispatchable to operate on
	 *	@param _A_sigsource A shared pointer to the server 
	 *	thread's signal source.
	 */
	signal_wrapper(const shared_dispatchable& _A_disp, const std::tr1::shared_ptr<signal_source_base>& _A_sigsource) throw(): 
		signal_wrapper_base(_A_disp, _A_sigsource)
	{}
	
	/**	@short Connects a functor, tunnels it automatically if not yet 
	 *	tunneled and activates validity tracking for sigc::trackableS.
	 *	
	 *	auto tunneling is successful only if the passed in functor is a
	 *	"dispatchable" functor, i.e. a functor on a dispatchable's method
	 *	or one explicitly created with "dispatch_with".
	 *	You will get compiler errors if the dispatchable can't be deduced from the 
	 *	passed in functor.
	 *	
	 *	@note At the moment it is only possible to pass in a non-tunneled functor or
	 *	a toplevel tunneled functor due to the fact that the visit_each mechanism
	 *	is turned off for the tunnel functor (otherwise there would be the problem
	 *	of not threadsafe access to the sigc::trackable base of the of a 
	 *	dispatchable object..
	 *	
	 *	@note passed in functor must not be a slot or adapt a slot;
	 *	we have to apply this restriction because slots might have bound
	 *	trackables that can cause non-threadsafe access to the passed in slot
	 *	which will live in the context of the server thread.
	 *	
	 *	@attention	All sigc::trackableS and the original dispatchable contained 
	 *	in the passed functor must belong to the context of the calling thread.
	 *	
	 *	@return sigx::connection_wrapper A threadsafe connection wrapper
	 *	@note asynchronous
	 */
	template<typename T_functor>
	connection_wrapper connect(const T_functor& _A_func) const
	{
		return	signal_wrapper_base::connect(
					_A_func, 
					sigc::ptr_fun(&typed_connection_handler<signal_type, signal_group>::connect)
				);
	}

	/**	@short emits the signal on the other side of the tunnel.
	 */
	template<sync_type I_sync>
	result_type emit(arg1_type_ _A_a1, arg2_type_ _A_a2, arg3_type_ _A_a3, arg4_type_ _A_a4) const
	{
		return open_tunnel_with<I_sync>(
			// calls T_signal::*emit
			sigc::compose(
				sigc::mem_fun(&signal_type::emit), 
				// getter for the T_signal
				sigc::compose(
					sigc::retype_return<fp_sig_getter_type>(
						sigc::mem_fun(m_sigsource.get(), &signal_source_base::getter)
					), 
					// this makes a copy of the shared signal source and thus 
					// shares it within the tunnel functor ensuring the lifetime 
					// of the shared signal source
					sigc::bind(
						sigc::mem_fun(&std::tr1::shared_ptr<signal_source_base>::operator *), 
						m_sigsource
					)
				)
			), 
			m_disp
		)(_A_a1, _A_a2, _A_a3, _A_a4);
	}

	/**	@short emits the signal asynchronously.
	 */
	result_type emit(arg1_type_ _A_a1, arg2_type_ _A_a2, arg3_type_ _A_a3, arg4_type_ _A_a4) const
	{
		return emit<ASYNC>(_A_a1, _A_a2, _A_a3, _A_a4);
	}

	/**	@short emits the signal synchronously.
	 */
	result_type emit_sync(arg1_type_ _A_a1, arg2_type_ _A_a2, arg3_type_ _A_a3, arg4_type_ _A_a4) const
	{
		return emit<SYNC>(_A_a1, _A_a2, _A_a3, _A_a4);
	}

	/**	see emit(arg1_type_, arg2_type_, arg3_type_, arg4_type_)
	 */
	result_type operator()(arg1_type_ _A_a1, arg2_type_ _A_a2, arg3_type_ _A_a3, arg4_type_ _A_a4) const
	{
		return emit<ASYNC>(_A_a1, _A_a2, _A_a3, _A_a4);
	}


	/**	@short creates a tunnel_functor that emits the signal when invoked
	 */
	template<sync_type I_sync>
	tunnel_functor<I_sync, make_slot_functor_type> make_slot() const
	{
		typedef tunnel_functor<I_sync, make_slot_functor_type> tunnel_funtor_type;
		return tunnel_funtor_type(
			dispatch_with(
				// calls T_signal::*emit
				sigc::compose(
					&signal_type::emit, 
					// getter for the T_signal
					sigc::compose(
						sigc::retype_return<fp_sig_getter_type>(
							sigc::mem_fun(m_sigsource.get(), &signal_source_base::getter)
						), 
						sigc::bind(
							sigc::mem_fun(&std::tr1::shared_ptr<signal_source_base>::operator *), 
							m_sigsource
						)
					)
				), 
				m_disp
			));
	}

	/**	@short creates an asynchronous tunnel_functor that emits the signal 
	 *	when invoked
	 */
	tunnel_functor<ASYNC, make_slot_functor_type> make_slot() const
	{
		return make_slot<ASYNC>();
	}

	/**	@short creates a synchronous tunnel_functor that emits the signal 
	 *	when invoked
	 */
	tunnel_functor<SYNC, make_slot_functor_type> make_slot_sync() const
	{
		return make_slot<SYNC>();
	}
};

/**	@short A threadsafe wrapper for any sigc signal with 5 argument(s).
 *	
 *	@ingroup signals
 */
template<typename T_signal>
class signal_wrapper<T_signal, internal::SIGGROUP_SIGC, 5>: public signal_wrapper_base
{
public:
	//BOOST_STATIC_ASSERT((internal::count_signal_arguments<T_signal>::value == 5));
	static const int argument_count = 5;
	static const internal::signal_group signal_group = internal::SIGGROUP_SIGC;
	typedef T_signal signal_type;
	typedef signal_wrapper<signal_type, signal_group, argument_count> this_type;
	typedef typename signal_type::slot_type slot_type;
	typedef typename signal_type::result_type result_type;
		typedef typename slot_type::arg1_type_ arg1_type_;
	typedef typename slot_type::arg2_type_ arg2_type_;
	typedef typename slot_type::arg3_type_ arg3_type_;
	typedef typename slot_type::arg4_type_ arg4_type_;
	typedef typename slot_type::arg5_type_ arg5_type_;

	typedef signal_type (*fp_sig_getter_type)(signal_source_ptr);
	
protected:
	typedef sigc::bound_const_mem_functor0<signal_source_base::hook, signal_source_base> make_slot_f1;
	typedef sigc::retype_return_functor<fp_sig_getter_type, make_slot_f1> make_slot_f2;
	typedef sigc::const_mem_functor0<signal_source_ptr, std::tr1::shared_ptr<signal_source_base> > make_slot_f3;
	typedef sigc::bind_functor<-1, make_slot_f3, std::tr1::shared_ptr<signal_source_base> > make_slot_f4;
	typedef sigc::compose1_functor<make_slot_f2, make_slot_f4> make_slot_composed1_functor_type;
	typedef sigc::const_mem_functor5<result_type, signal_type, arg1_type_, arg2_type_, arg3_type_, arg4_type_, arg5_type_> make_slot_emit_functor_type;
	typedef sigc::compose1_functor<make_slot_emit_functor_type, make_slot_composed1_functor_type> make_slot_composed2_functor_type;
	typedef SIGX_DISPATCH_WITH_FUNCTOR(make_slot_composed2_functor_type) make_slot_functor_type;


public:
	/**	@short Constructs an empty signal_wrapper.
	 *	@throw	Might throw a std::bad_alloc exception (from dispatchable's ctor)
	 */
	signal_wrapper(): signal_wrapper_base()
	{}

	/**	@short Creates a signal_wrapper from a signal source.
	 *	@param _A_disp The dispatchable to operate on
	 *	@param _A_sigsource A shared pointer to the server 
	 *	thread's signal source.
	 */
	signal_wrapper(const shared_dispatchable& _A_disp, const std::tr1::shared_ptr<signal_source_base>& _A_sigsource) throw(): 
		signal_wrapper_base(_A_disp, _A_sigsource)
	{}
	
	/**	@short Connects a functor, tunnels it automatically if not yet 
	 *	tunneled and activates validity tracking for sigc::trackableS.
	 *	
	 *	auto tunneling is successful only if the passed in functor is a
	 *	"dispatchable" functor, i.e. a functor on a dispatchable's method
	 *	or one explicitly created with "dispatch_with".
	 *	You will get compiler errors if the dispatchable can't be deduced from the 
	 *	passed in functor.
	 *	
	 *	@note At the moment it is only possible to pass in a non-tunneled functor or
	 *	a toplevel tunneled functor due to the fact that the visit_each mechanism
	 *	is turned off for the tunnel functor (otherwise there would be the problem
	 *	of not threadsafe access to the sigc::trackable base of the of a 
	 *	dispatchable object..
	 *	
	 *	@note passed in functor must not be a slot or adapt a slot;
	 *	we have to apply this restriction because slots might have bound
	 *	trackables that can cause non-threadsafe access to the passed in slot
	 *	which will live in the context of the server thread.
	 *	
	 *	@attention	All sigc::trackableS and the original dispatchable contained 
	 *	in the passed functor must belong to the context of the calling thread.
	 *	
	 *	@return sigx::connection_wrapper A threadsafe connection wrapper
	 *	@note asynchronous
	 */
	template<typename T_functor>
	connection_wrapper connect(const T_functor& _A_func) const
	{
		return	signal_wrapper_base::connect(
					_A_func, 
					sigc::ptr_fun(&typed_connection_handler<signal_type, signal_group>::connect)
				);
	}

	/**	@short emits the signal on the other side of the tunnel.
	 */
	template<sync_type I_sync>
	result_type emit(arg1_type_ _A_a1, arg2_type_ _A_a2, arg3_type_ _A_a3, arg4_type_ _A_a4, arg5_type_ _A_a5) const
	{
		return open_tunnel_with<I_sync>(
			// calls T_signal::*emit
			sigc::compose(
				sigc::mem_fun(&signal_type::emit), 
				// getter for the T_signal
				sigc::compose(
					sigc::retype_return<fp_sig_getter_type>(
						sigc::mem_fun(m_sigsource.get(), &signal_source_base::getter)
					), 
					// this makes a copy of the shared signal source and thus 
					// shares it within the tunnel functor ensuring the lifetime 
					// of the shared signal source
					sigc::bind(
						sigc::mem_fun(&std::tr1::shared_ptr<signal_source_base>::operator *), 
						m_sigsource
					)
				)
			), 
			m_disp
		)(_A_a1, _A_a2, _A_a3, _A_a4, _A_a5);
	}

	/**	@short emits the signal asynchronously.
	 */
	result_type emit(arg1_type_ _A_a1, arg2_type_ _A_a2, arg3_type_ _A_a3, arg4_type_ _A_a4, arg5_type_ _A_a5) const
	{
		return emit<ASYNC>(_A_a1, _A_a2, _A_a3, _A_a4, _A_a5);
	}

	/**	@short emits the signal synchronously.
	 */
	result_type emit_sync(arg1_type_ _A_a1, arg2_type_ _A_a2, arg3_type_ _A_a3, arg4_type_ _A_a4, arg5_type_ _A_a5) const
	{
		return emit<SYNC>(_A_a1, _A_a2, _A_a3, _A_a4, _A_a5);
	}

	/**	see emit(arg1_type_, arg2_type_, arg3_type_, arg4_type_, arg5_type_)
	 */
	result_type operator()(arg1_type_ _A_a1, arg2_type_ _A_a2, arg3_type_ _A_a3, arg4_type_ _A_a4, arg5_type_ _A_a5) const
	{
		return emit<ASYNC>(_A_a1, _A_a2, _A_a3, _A_a4, _A_a5);
	}


	/**	@short creates a tunnel_functor that emits the signal when invoked
	 */
	template<sync_type I_sync>
	tunnel_functor<I_sync, make_slot_functor_type> make_slot() const
	{
		typedef tunnel_functor<I_sync, make_slot_functor_type> tunnel_funtor_type;
		return tunnel_funtor_type(
			dispatch_with(
				// calls T_signal::*emit
				sigc::compose(
					&signal_type::emit, 
					// getter for the T_signal
					sigc::compose(
						sigc::retype_return<fp_sig_getter_type>(
							sigc::mem_fun(m_sigsource.get(), &signal_source_base::getter)
						), 
						sigc::bind(
							sigc::mem_fun(&std::tr1::shared_ptr<signal_source_base>::operator *), 
							m_sigsource
						)
					)
				), 
				m_disp
			));
	}

	/**	@short creates an asynchronous tunnel_functor that emits the signal 
	 *	when invoked
	 */
	tunnel_functor<ASYNC, make_slot_functor_type> make_slot() const
	{
		return make_slot<ASYNC>();
	}

	/**	@short creates a synchronous tunnel_functor that emits the signal 
	 *	when invoked
	 */
	tunnel_functor<SYNC, make_slot_functor_type> make_slot_sync() const
	{
		return make_slot<SYNC>();
	}
};

/**	@short A threadsafe wrapper for any sigc signal with 6 argument(s).
 *	
 *	@ingroup signals
 */
template<typename T_signal>
class signal_wrapper<T_signal, internal::SIGGROUP_SIGC, 6>: public signal_wrapper_base
{
public:
	//BOOST_STATIC_ASSERT((internal::count_signal_arguments<T_signal>::value == 6));
	static const int argument_count = 6;
	static const internal::signal_group signal_group = internal::SIGGROUP_SIGC;
	typedef T_signal signal_type;
	typedef signal_wrapper<signal_type, signal_group, argument_count> this_type;
	typedef typename signal_type::slot_type slot_type;
	typedef typename signal_type::result_type result_type;
		typedef typename slot_type::arg1_type_ arg1_type_;
	typedef typename slot_type::arg2_type_ arg2_type_;
	typedef typename slot_type::arg3_type_ arg3_type_;
	typedef typename slot_type::arg4_type_ arg4_type_;
	typedef typename slot_type::arg5_type_ arg5_type_;
	typedef typename slot_type::arg6_type_ arg6_type_;

	typedef signal_type (*fp_sig_getter_type)(signal_source_ptr);
	
protected:
	typedef sigc::bound_const_mem_functor0<signal_source_base::hook, signal_source_base> make_slot_f1;
	typedef sigc::retype_return_functor<fp_sig_getter_type, make_slot_f1> make_slot_f2;
	typedef sigc::const_mem_functor0<signal_source_ptr, std::tr1::shared_ptr<signal_source_base> > make_slot_f3;
	typedef sigc::bind_functor<-1, make_slot_f3, std::tr1::shared_ptr<signal_source_base> > make_slot_f4;
	typedef sigc::compose1_functor<make_slot_f2, make_slot_f4> make_slot_composed1_functor_type;
	typedef sigc::const_mem_functor6<result_type, signal_type, arg1_type_, arg2_type_, arg3_type_, arg4_type_, arg5_type_, arg6_type_> make_slot_emit_functor_type;
	typedef sigc::compose1_functor<make_slot_emit_functor_type, make_slot_composed1_functor_type> make_slot_composed2_functor_type;
	typedef SIGX_DISPATCH_WITH_FUNCTOR(make_slot_composed2_functor_type) make_slot_functor_type;


public:
	/**	@short Constructs an empty signal_wrapper.
	 *	@throw	Might throw a std::bad_alloc exception (from dispatchable's ctor)
	 */
	signal_wrapper(): signal_wrapper_base()
	{}

	/**	@short Creates a signal_wrapper from a signal source.
	 *	@param _A_disp The dispatchable to operate on
	 *	@param _A_sigsource A shared pointer to the server 
	 *	thread's signal source.
	 */
	signal_wrapper(const shared_dispatchable& _A_disp, const std::tr1::shared_ptr<signal_source_base>& _A_sigsource) throw(): 
		signal_wrapper_base(_A_disp, _A_sigsource)
	{}
	
	/**	@short Connects a functor, tunnels it automatically if not yet 
	 *	tunneled and activates validity tracking for sigc::trackableS.
	 *	
	 *	auto tunneling is successful only if the passed in functor is a
	 *	"dispatchable" functor, i.e. a functor on a dispatchable's method
	 *	or one explicitly created with "dispatch_with".
	 *	You will get compiler errors if the dispatchable can't be deduced from the 
	 *	passed in functor.
	 *	
	 *	@note At the moment it is only possible to pass in a non-tunneled functor or
	 *	a toplevel tunneled functor due to the fact that the visit_each mechanism
	 *	is turned off for the tunnel functor (otherwise there would be the problem
	 *	of not threadsafe access to the sigc::trackable base of the of a 
	 *	dispatchable object..
	 *	
	 *	@note passed in functor must not be a slot or adapt a slot;
	 *	we have to apply this restriction because slots might have bound
	 *	trackables that can cause non-threadsafe access to the passed in slot
	 *	which will live in the context of the server thread.
	 *	
	 *	@attention	All sigc::trackableS and the original dispatchable contained 
	 *	in the passed functor must belong to the context of the calling thread.
	 *	
	 *	@return sigx::connection_wrapper A threadsafe connection wrapper
	 *	@note asynchronous
	 */
	template<typename T_functor>
	connection_wrapper connect(const T_functor& _A_func) const
	{
		return	signal_wrapper_base::connect(
					_A_func, 
					sigc::ptr_fun(&typed_connection_handler<signal_type, signal_group>::connect)
				);
	}

	/**	@short emits the signal on the other side of the tunnel.
	 */
	template<sync_type I_sync>
	result_type emit(arg1_type_ _A_a1, arg2_type_ _A_a2, arg3_type_ _A_a3, arg4_type_ _A_a4, arg5_type_ _A_a5, arg6_type_ _A_a6) const
	{
		return open_tunnel_with<I_sync>(
			// calls T_signal::*emit
			sigc::compose(
				sigc::mem_fun(&signal_type::emit), 
				// getter for the T_signal
				sigc::compose(
					sigc::retype_return<fp_sig_getter_type>(
						sigc::mem_fun(m_sigsource.get(), &signal_source_base::getter)
					), 
					// this makes a copy of the shared signal source and thus 
					// shares it within the tunnel functor ensuring the lifetime 
					// of the shared signal source
					sigc::bind(
						sigc::mem_fun(&std::tr1::shared_ptr<signal_source_base>::operator *), 
						m_sigsource
					)
				)
			), 
			m_disp
		)(_A_a1, _A_a2, _A_a3, _A_a4, _A_a5, _A_a6);
	}

	/**	@short emits the signal asynchronously.
	 */
	result_type emit(arg1_type_ _A_a1, arg2_type_ _A_a2, arg3_type_ _A_a3, arg4_type_ _A_a4, arg5_type_ _A_a5, arg6_type_ _A_a6) const
	{
		return emit<ASYNC>(_A_a1, _A_a2, _A_a3, _A_a4, _A_a5, _A_a6);
	}

	/**	@short emits the signal synchronously.
	 */
	result_type emit_sync(arg1_type_ _A_a1, arg2_type_ _A_a2, arg3_type_ _A_a3, arg4_type_ _A_a4, arg5_type_ _A_a5, arg6_type_ _A_a6) const
	{
		return emit<SYNC>(_A_a1, _A_a2, _A_a3, _A_a4, _A_a5, _A_a6);
	}

	/**	see emit(arg1_type_, arg2_type_, arg3_type_, arg4_type_, arg5_type_, arg6_type_)
	 */
	result_type operator()(arg1_type_ _A_a1, arg2_type_ _A_a2, arg3_type_ _A_a3, arg4_type_ _A_a4, arg5_type_ _A_a5, arg6_type_ _A_a6) const
	{
		return emit<ASYNC>(_A_a1, _A_a2, _A_a3, _A_a4, _A_a5, _A_a6);
	}


	/**	@short creates a tunnel_functor that emits the signal when invoked
	 */
	template<sync_type I_sync>
	tunnel_functor<I_sync, make_slot_functor_type> make_slot() const
	{
		typedef tunnel_functor<I_sync, make_slot_functor_type> tunnel_funtor_type;
		return tunnel_funtor_type(
			dispatch_with(
				// calls T_signal::*emit
				sigc::compose(
					&signal_type::emit, 
					// getter for the T_signal
					sigc::compose(
						sigc::retype_return<fp_sig_getter_type>(
							sigc::mem_fun(m_sigsource.get(), &signal_source_base::getter)
						), 
						sigc::bind(
							sigc::mem_fun(&std::tr1::shared_ptr<signal_source_base>::operator *), 
							m_sigsource
						)
					)
				), 
				m_disp
			));
	}

	/**	@short creates an asynchronous tunnel_functor that emits the signal 
	 *	when invoked
	 */
	tunnel_functor<ASYNC, make_slot_functor_type> make_slot() const
	{
		return make_slot<ASYNC>();
	}

	/**	@short creates a synchronous tunnel_functor that emits the signal 
	 *	when invoked
	 */
	tunnel_functor<SYNC, make_slot_functor_type> make_slot_sync() const
	{
		return make_slot<SYNC>();
	}
};

/**	@short A threadsafe wrapper for any sigc signal with 7 argument(s).
 *	
 *	@ingroup signals
 */
template<typename T_signal>
class signal_wrapper<T_signal, internal::SIGGROUP_SIGC, 7>: public signal_wrapper_base
{
public:
	//BOOST_STATIC_ASSERT((internal::count_signal_arguments<T_signal>::value == 7));
	static const int argument_count = 7;
	static const internal::signal_group signal_group = internal::SIGGROUP_SIGC;
	typedef T_signal signal_type;
	typedef signal_wrapper<signal_type, signal_group, argument_count> this_type;
	typedef typename signal_type::slot_type slot_type;
	typedef typename signal_type::result_type result_type;
		typedef typename slot_type::arg1_type_ arg1_type_;
	typedef typename slot_type::arg2_type_ arg2_type_;
	typedef typename slot_type::arg3_type_ arg3_type_;
	typedef typename slot_type::arg4_type_ arg4_type_;
	typedef typename slot_type::arg5_type_ arg5_type_;
	typedef typename slot_type::arg6_type_ arg6_type_;
	typedef typename slot_type::arg7_type_ arg7_type_;

	typedef signal_type (*fp_sig_getter_type)(signal_source_ptr);
	
protected:
	typedef sigc::bound_const_mem_functor0<signal_source_base::hook, signal_source_base> make_slot_f1;
	typedef sigc::retype_return_functor<fp_sig_getter_type, make_slot_f1> make_slot_f2;
	typedef sigc::const_mem_functor0<signal_source_ptr, std::tr1::shared_ptr<signal_source_base> > make_slot_f3;
	typedef sigc::bind_functor<-1, make_slot_f3, std::tr1::shared_ptr<signal_source_base> > make_slot_f4;
	typedef sigc::compose1_functor<make_slot_f2, make_slot_f4> make_slot_composed1_functor_type;
	typedef sigc::const_mem_functor7<result_type, signal_type, arg1_type_, arg2_type_, arg3_type_, arg4_type_, arg5_type_, arg6_type_, arg7_type_> make_slot_emit_functor_type;
	typedef sigc::compose1_functor<make_slot_emit_functor_type, make_slot_composed1_functor_type> make_slot_composed2_functor_type;
	typedef SIGX_DISPATCH_WITH_FUNCTOR(make_slot_composed2_functor_type) make_slot_functor_type;


public:
	/**	@short Constructs an empty signal_wrapper.
	 *	@throw	Might throw a std::bad_alloc exception (from dispatchable's ctor)
	 */
	signal_wrapper(): signal_wrapper_base()
	{}

	/**	@short Creates a signal_wrapper from a signal source.
	 *	@param _A_disp The dispatchable to operate on
	 *	@param _A_sigsource A shared pointer to the server 
	 *	thread's signal source.
	 */
	signal_wrapper(const shared_dispatchable& _A_disp, const std::tr1::shared_ptr<signal_source_base>& _A_sigsource) throw(): 
		signal_wrapper_base(_A_disp, _A_sigsource)
	{}
	
	/**	@short Connects a functor, tunnels it automatically if not yet 
	 *	tunneled and activates validity tracking for sigc::trackableS.
	 *	
	 *	auto tunneling is successful only if the passed in functor is a
	 *	"dispatchable" functor, i.e. a functor on a dispatchable's method
	 *	or one explicitly created with "dispatch_with".
	 *	You will get compiler errors if the dispatchable can't be deduced from the 
	 *	passed in functor.
	 *	
	 *	@note At the moment it is only possible to pass in a non-tunneled functor or
	 *	a toplevel tunneled functor due to the fact that the visit_each mechanism
	 *	is turned off for the tunnel functor (otherwise there would be the problem
	 *	of not threadsafe access to the sigc::trackable base of the of a 
	 *	dispatchable object..
	 *	
	 *	@note passed in functor must not be a slot or adapt a slot;
	 *	we have to apply this restriction because slots might have bound
	 *	trackables that can cause non-threadsafe access to the passed in slot
	 *	which will live in the context of the server thread.
	 *	
	 *	@attention	All sigc::trackableS and the original dispatchable contained 
	 *	in the passed functor must belong to the context of the calling thread.
	 *	
	 *	@return sigx::connection_wrapper A threadsafe connection wrapper
	 *	@note asynchronous
	 */
	template<typename T_functor>
	connection_wrapper connect(const T_functor& _A_func) const
	{
		return	signal_wrapper_base::connect(
					_A_func, 
					sigc::ptr_fun(&typed_connection_handler<signal_type, signal_group>::connect)
				);
	}

	/**	@short emits the signal on the other side of the tunnel.
	 */
	template<sync_type I_sync>
	result_type emit(arg1_type_ _A_a1, arg2_type_ _A_a2, arg3_type_ _A_a3, arg4_type_ _A_a4, arg5_type_ _A_a5, arg6_type_ _A_a6, arg7_type_ _A_a7) const
	{
		return open_tunnel_with<I_sync>(
			// calls T_signal::*emit
			sigc::compose(
				sigc::mem_fun(&signal_type::emit), 
				// getter for the T_signal
				sigc::compose(
					sigc::retype_return<fp_sig_getter_type>(
						sigc::mem_fun(m_sigsource.get(), &signal_source_base::getter)
					), 
					// this makes a copy of the shared signal source and thus 
					// shares it within the tunnel functor ensuring the lifetime 
					// of the shared signal source
					sigc::bind(
						sigc::mem_fun(&std::tr1::shared_ptr<signal_source_base>::operator *), 
						m_sigsource
					)
				)
			), 
			m_disp
		)(_A_a1, _A_a2, _A_a3, _A_a4, _A_a5, _A_a6, _A_a7);
	}

	/**	@short emits the signal asynchronously.
	 */
	result_type emit(arg1_type_ _A_a1, arg2_type_ _A_a2, arg3_type_ _A_a3, arg4_type_ _A_a4, arg5_type_ _A_a5, arg6_type_ _A_a6, arg7_type_ _A_a7) const
	{
		return emit<ASYNC>(_A_a1, _A_a2, _A_a3, _A_a4, _A_a5, _A_a6, _A_a7);
	}

	/**	@short emits the signal synchronously.
	 */
	result_type emit_sync(arg1_type_ _A_a1, arg2_type_ _A_a2, arg3_type_ _A_a3, arg4_type_ _A_a4, arg5_type_ _A_a5, arg6_type_ _A_a6, arg7_type_ _A_a7) const
	{
		return emit<SYNC>(_A_a1, _A_a2, _A_a3, _A_a4, _A_a5, _A_a6, _A_a7);
	}

	/**	see emit(arg1_type_, arg2_type_, arg3_type_, arg4_type_, arg5_type_, arg6_type_, arg7_type_)
	 */
	result_type operator()(arg1_type_ _A_a1, arg2_type_ _A_a2, arg3_type_ _A_a3, arg4_type_ _A_a4, arg5_type_ _A_a5, arg6_type_ _A_a6, arg7_type_ _A_a7) const
	{
		return emit<ASYNC>(_A_a1, _A_a2, _A_a3, _A_a4, _A_a5, _A_a6, _A_a7);
	}


	/**	@short creates a tunnel_functor that emits the signal when invoked
	 */
	template<sync_type I_sync>
	tunnel_functor<I_sync, make_slot_functor_type> make_slot() const
	{
		typedef tunnel_functor<I_sync, make_slot_functor_type> tunnel_funtor_type;
		return tunnel_funtor_type(
			dispatch_with(
				// calls T_signal::*emit
				sigc::compose(
					&signal_type::emit, 
					// getter for the T_signal
					sigc::compose(
						sigc::retype_return<fp_sig_getter_type>(
							sigc::mem_fun(m_sigsource.get(), &signal_source_base::getter)
						), 
						sigc::bind(
							sigc::mem_fun(&std::tr1::shared_ptr<signal_source_base>::operator *), 
							m_sigsource
						)
					)
				), 
				m_disp
			));
	}

	/**	@short creates an asynchronous tunnel_functor that emits the signal 
	 *	when invoked
	 */
	tunnel_functor<ASYNC, make_slot_functor_type> make_slot() const
	{
		return make_slot<ASYNC>();
	}

	/**	@short creates a synchronous tunnel_functor that emits the signal 
	 *	when invoked
	 */
	tunnel_functor<SYNC, make_slot_functor_type> make_slot_sync() const
	{
		return make_slot<SYNC>();
	}
};




/**	@short A threadsafe wrapper for a Glib::SignalNormalProxy derived signal.
 *	@ingroup signals
 */
template<typename T_signal>
// have to specialize the argument count explicitly because template arguments
// can't be involved as template parameters in further template arguments
class signal_wrapper<T_signal, internal::SIGGROUP_GLIB_PROXY, -1>: public signal_wrapper_base
{
public:
	static const int argument_count = internal::count_signal_arguments<T_signal>::value;
	static const internal::signal_group signal_group = internal::SIGGROUP_GLIB_PROXY;
	typedef T_signal signal_type;
	typedef signal_wrapper<signal_type, signal_group, argument_count> this_type;
	typedef typename signal_type::SlotType slot_type;
	typedef typename signal_type::VoidSlotType void_slot_type;
	typedef typename slot_type::result_type result_type;
	typedef signal_type (*fp_sig_getter_type)(signal_source_ptr);


	/**	@short Constructs an empty signal_wrapper.
	 *	@throw	Might throw a std::bad_alloc exception (from dispatchable's ctor)
	 */
	signal_wrapper(): signal_wrapper_base()
	{}

	/**	@short Creates a signal_wrapper from a signal source.
	 *	@param _A_disp The dispatchable to operate on
	 *	@param _A_sigsource A double pointer to the server 
	 *	thread's signal source.
	 */
	signal_wrapper(const shared_dispatchable& _A_disp, const std::tr1::shared_ptr<signal_source_base>& _A_sigsource): 
		signal_wrapper_base(_A_disp, _A_sigsource)
	{}
	
	/**	@short Connects a functor, tunnels it automatically if not yet 
	 *	tunneled and activates validity tracking for sigc::trackableS.
	 *	
	 *	auto tunneling is successful only if the passed in functor is a
	 *	"dispatchable" functor, i.e. a functor on a dispatchable's method
	 *	or one explicitly created with "dispatch_with".
	 *	You will get compiler errors if the dispatchable can't be deduced from the 
	 *	passed in functor.
	 *	
	 *	@note At the moment it is only possible to pass in a non-tunneled functor or
	 *	a toplevel tunneled functor due to the fact that the visit_each mechanism
	 *	is turned off for the tunnel functor (otherwise there would be the problem
	 *	of not threadsafe access to the sigc::trackable base of the of a 
	 *	dispatchable object..
	 *	
	 *	@note passed in functor must not be a slot or adapt a slot;
	 *	we have to apply this restriction because slots might have bound
	 *	trackables that can cause non-threadsafe access to the passed in slot
	 *	which will live in the context of the server thread
	 *	
	 *	@attention	All sigc::trackableS and the original dispatchable contained 
	 *	in the passed functor must belong to the context of the calling thread.
	 *	
	 *	@return sigx::connection_wrapper A threadsafe connection wrapper
	 *	@note asynchronous
	 */
	template<typename T_functor>
	connection_wrapper connect(const T_functor& _A_func, bool after = true) const
	{
		return	signal_wrapper_base::connect(
					_A_func, 
					sigc::bind(
						sigc::ptr_fun(&typed_connection_handler<signal_type, signal_group>::connect), 
						after
					)
				);
	}

	/**	
	 *	@return sigx::connection_wrapper A threadsafe connection wrapper
	 *	@note asynchronous
	 */
	template<typename T_functor>
	connection_wrapper connect_notify(const T_functor& _A_func, bool after = false) const
	{
		return	signal_wrapper_base::connect(
					_A_func, 
					sigc::bind(
						sigc::ptr_fun(&typed_connection_handler<signal_type, signal_group>::connect_notify), 
						after
					)
				);
	}
};


/**	@short A threadsafe wrapper for a Glib::SignalIdle.
 *	@ingroup signals
 */
template<>
class signal_wrapper<Glib::SignalIdle>: public signal_wrapper_base
{
public:
	static const int argument_count = internal::count_signal_arguments<Glib::SignalIdle>::value;
	static const internal::signal_group signal_group = internal::SIGGROUP_IRRELEVANT;
	typedef Glib::SignalIdle signal_type;
	typedef signal_wrapper<signal_type, signal_group, argument_count> this_type;
	typedef bool result_type;
	typedef sigc::slot<bool> slot_type;
	typedef signal_type (*fp_sig_getter_type)(signal_source_ptr);


	/**	@short Constructs an empty signal_wrapper.
	 *	@throw	Might throw a std::bad_alloc exception (from dispatchable's ctor)
	 */
	signal_wrapper(): signal_wrapper_base()
	{}

	/**	@short Creates a signal_wrapper from a signal source.
	 *	@param _A_disp The dispatchable to operate on
	 *	@param _A_sigsource A double pointer to the server 
	 *	thread's signal source.
	 */
	signal_wrapper(const shared_dispatchable& _A_disp, const std::tr1::shared_ptr<signal_source_base>& _A_sigsource): 
		signal_wrapper_base(_A_disp, _A_sigsource)
	{}
	
	/**	@short Connects a functor, tunnels it automatically if not yet 
	 *	tunneled and activates validity tracking for sigc::trackableS.
	 *	
	 *	auto tunneling is successful only if the passed in functor is a
	 *	"dispatchable" functor, i.e. a functor on a dispatchable's method
	 *	or one explicitly created with "dispatch_with".
	 *	You will get compiler errors if the dispatchable can't be deduced from the 
	 *	passed in functor.
	 *	
	 *	@note At the moment it is only possible to pass in a non-tunneled functor or
	 *	a toplevel tunneled functor due to the fact that the visit_each mechanism
	 *	is turned off for the tunnel functor (otherwise there would be the problem
	 *	of not threadsafe access to the sigc::trackable base of the of a 
	 *	dispatchable object..
	 *	
	 *	@note passed in functor must not be a slot or adapt a slot;
	 *	we have to apply this restriction because slots might have bound
	 *	trackables that can cause non-threadsafe access to the passed in slot
	 *	which will live in the context of the server thread
	 *	
	 *	@attention	All sigc::trackableS and the original dispatchable contained 
	 *	in the passed functor must belong to the context of the calling thread.
	 *	
	 *	@return sigx::connection_wrapper A threadsafe connection wrapper
	 *	@note asynchronous
	 */
	template<typename T_functor>
	connection_wrapper connect(const T_functor& _A_func, int priority = Glib::PRIORITY_DEFAULT_IDLE) const
	{
		return	signal_wrapper_base::connect(
					_A_func, 
					sigc::bind(
						sigc::ptr_fun(&typed_connection_handler<signal_type, signal_group>::connect), 
						priority
					)
				);
	}
};


/**	@short A threadsafe wrapper for a Glib::SignalTimeout.
 *	@ingroup signals
 */
template<>
class signal_wrapper<Glib::SignalTimeout>: public signal_wrapper_base
{
public:
	static const int argument_count = internal::count_signal_arguments<Glib::SignalTimeout>::value;
	static const internal::signal_group signal_group = internal::SIGGROUP_IRRELEVANT;
	typedef Glib::SignalTimeout signal_type;
	typedef signal_wrapper<signal_type, signal_group, argument_count> this_type;
	typedef bool result_type;
	typedef sigc::slot<bool> slot_type;
	typedef signal_type (*fp_sig_getter_type)(signal_source_ptr);


	/**	@short Constructs an empty signal_wrapper.
	 *	@throw	Might throw a std::bad_alloc exception (from dispatchable's ctor)
	 */
	signal_wrapper(): signal_wrapper_base()
	{}

	/**	@short Creates a signal_wrapper from a signal source.
	 *	@param _A_disp The dispatchable to operate on
	 *	@param _A_sigsource A double pointer to the server 
	 *	thread's signal source.
	 */
	signal_wrapper(const shared_dispatchable& _A_disp, const std::tr1::shared_ptr<signal_source_base>& _A_sigsource): 
		signal_wrapper_base(_A_disp, _A_sigsource)
	{}
	
	/**	@short Connects a functor, tunnels it automatically if not yet 
	 *	tunneled and activates validity tracking for sigc::trackableS.
	 *	
	 *	auto tunneling is successful only if the passed in functor is a
	 *	"dispatchable" functor, i.e. a functor on a dispatchable's method
	 *	or one explicitly created with "dispatch_with".
	 *	You will get compiler errors if the dispatchable can't be deduced from the 
	 *	passed in functor.
	 *	
	 *	@note At the moment it is only possible to pass in a non-tunneled functor or
	 *	a toplevel tunneled functor due to the fact that the visit_each mechanism
	 *	is turned off for the tunnel functor (otherwise there would be the problem
	 *	of not threadsafe access to the sigc::trackable base of the of a 
	 *	dispatchable object..
	 *	
	 *	@note passed in functor must not be a slot or adapt a slot;
	 *	we have to apply this restriction because slots might have bound
	 *	trackables that can cause non-threadsafe access to the passed in slot
	 *	which will live in the context of the server thread
	 *	
	 *	@attention	All sigc::trackableS and the original dispatchable contained 
	 *	in the passed functor must belong to the context of the calling thread.
	 *	
	 *	@return sigx::connection_wrapper A threadsafe connection wrapper
	 *	@note asynchronous
	 */
	template<typename T_functor>
	connection_wrapper connect(const T_functor& _A_func, unsigned int interval,
						int priority = Glib::PRIORITY_DEFAULT) const
	{
		return	signal_wrapper_base::connect(
					_A_func, 
					sigc::bind(
						sigc::ptr_fun(&typed_connection_handler<signal_type, signal_group>::connect), 
						interval, priority
					)
				);
	}
};


/**	@short A threadsafe wrapper for a Glib::SignalIO.
 *	@ingroup signals
 */
template<>
class signal_wrapper<Glib::SignalIO>: public signal_wrapper_base
{
public:
	static const int argument_count = internal::count_signal_arguments<Glib::SignalIO>::value;
	static const internal::signal_group signal_group = internal::SIGGROUP_IRRELEVANT;
	typedef Glib::SignalIO signal_type;
	typedef signal_wrapper<signal_type, signal_group, argument_count> this_type;
	typedef bool result_type;
	typedef sigc::slot<bool, Glib::IOCondition> slot_type;
	typedef signal_type (*fp_sig_getter_type)(signal_source_ptr);


	/**	@short Constructs an empty signal_wrapper.
	 *	@throw	Might throw a std::bad_alloc exception (from dispatchable's ctor)
	 */
	signal_wrapper(): signal_wrapper_base()
	{}

	/**	@short Creates a signal_wrapper from a signal source.
	 *	@param _A_disp The dispatchable to operate on
	 *	@param _A_sigsource A double pointer to the source of the server 
	 *	thread's signal.
	 */
	signal_wrapper(const shared_dispatchable& _A_disp, const std::tr1::shared_ptr<signal_source_base>& _A_sigsource): 
		signal_wrapper_base(_A_disp, _A_sigsource)
	{}
	
	/**	@short Connects a functor, tunnels it automatically if not yet 
	 *	tunneled and activates validity tracking for sigc::trackableS.
	 *	
	 *	auto tunneling is successful only if the passed in functor is a
	 *	"dispatchable" functor, i.e. a functor on a dispatchable's method
	 *	or one explicitly created with "dispatch_with".
	 *	You will get compiler errors if the dispatchable can't be deduced from the 
	 *	passed in functor.
	 *	
	 *	@note At the moment it is only possible to pass in a non-tunneled functor or
	 *	a toplevel tunneled functor due to the fact that the visit_each mechanism
	 *	is turned off for the tunnel functor (otherwise there would be the problem
	 *	of not threadsafe access to the sigc::trackable base of the of a 
	 *	dispatchable object..
	 *	
	 *	@note passed in functor must not be a slot or adapt a slot;
	 *	we have to apply this restriction because slots might have bound
	 *	trackables that can cause non-threadsafe access to the passed in slot
	 *	which will live in the context of the server thread
	 *	
	 *	@attention	All sigc::trackableS and the original dispatchable contained 
	 *	in the passed functor must belong to the context of the calling thread.
	 *	
	 *	@return sigx::connection_wrapper A threadsafe connection wrapper
	 *	@note asynchronous
	 */
	template<typename T_functor>
	connection_wrapper connect(const T_functor& _A_func, Glib::IOCondition condition,
						int priority = Glib::PRIORITY_DEFAULT) const
	{
		return	signal_wrapper_base::connect(
					_A_func, 
					sigc::bind(
						sigc::ptr_fun(&typed_connection_handler<signal_type, signal_group>::connect), 
						condition, priority
					)
				);
	}
};


/**	@short A threadsafe wrapper for a Glib::SignalChildWatch.
 *	@ingroup signals
 */
template<>
class signal_wrapper<Glib::SignalChildWatch>: public signal_wrapper_base
{
public:
	static const int argument_count = internal::count_signal_arguments<Glib::SignalChildWatch>::value;
	static const internal::signal_group signal_group = internal::SIGGROUP_IRRELEVANT;
	typedef Glib::SignalChildWatch signal_type;
	typedef signal_wrapper<signal_type, signal_group, argument_count> this_type;
	typedef bool result_type;
	typedef sigc::slot<bool, GPid, int> slot_type;
	typedef signal_type (*fp_sig_getter_type)(signal_source_ptr);


	/**	@short Constructs an empty signal_wrapper.
	 *	@throw	Might throw a std::bad_alloc exception (from dispatchable's ctor)
	 */
	signal_wrapper(): signal_wrapper_base()
	{}

	/**	@short Creates a signal_wrapper from a signal source.
	 *	@param _A_disp The dispatchable to operate on
	 *	@param _A_sigsource A double pointer to the server 
	 *	thread's signal source.
	 */
	signal_wrapper(const shared_dispatchable& _A_disp, const std::tr1::shared_ptr<signal_source_base>& _A_sigsource): 
		signal_wrapper_base(_A_disp, _A_sigsource)
	{}
	
	/**	@short Connects a functor, tunnels it automatically if not yet 
	 *	tunneled and activates validity tracking for sigc::trackableS.
	 *	
	 *	auto tunneling is successful only if the passed in functor is a
	 *	"dispatchable" functor, i.e. a functor on a dispatchable's method
	 *	or one explicitly created with "dispatch_with".
	 *	You will get compiler errors if the dispatchable can't be deduced from the 
	 *	passed in functor.
	 *	
	 *	@note At the moment it is only possible to pass in a non-tunneled functor or
	 *	a toplevel tunneled functor due to the fact that the visit_each mechanism
	 *	is turned off for the tunnel functor (otherwise there would be the problem
	 *	of not threadsafe access to the sigc::trackable base of the of a 
	 *	dispatchable object..
	 *	
	 *	@note passed in functor must not be a slot or adapt a slot;
	 *	we have to apply this restriction because slots might have bound
	 *	trackables that can cause non-threadsafe access to the passed in slot
	 *	which will live in the context of the server thread
	 *	
	 *	@attention	All sigc::trackableS and the original dispatchable contained 
	 *	in the passed functor must belong to the context of the calling thread.
	 *	
	 *	@return sigx::connection_wrapper A threadsafe connection wrapper
	 *	@note asynchronous
	 */
	template<typename T_functor>
	connection_wrapper connect(const T_functor& _A_func, GPid pid,
						int priority = Glib::PRIORITY_DEFAULT) const
	{
		return	signal_wrapper_base::connect(
					_A_func, 
					sigc::bind(
						sigc::ptr_fun(&typed_connection_handler<signal_type, signal_group>::connect), 
						pid, priority
					)
				);
	}
};


typedef signal_wrapper<Glib::SignalIdle> glib_signal_idle;
typedef signal_wrapper<Glib::SignalTimeout> glib_signal_timeout;
typedef signal_wrapper<Glib::SignalIO> glib_signal_io;
typedef signal_wrapper<Glib::SignalChildWatch> glib_ignal_childwatch;


} // namespace sigx


#endif /* _SIGXMACROS_SIGNAL_WRAPPER_H_ */
