// -*- c++ -*-
/* Do not edit! -- generated file */


#ifndef _SIGXMACROS_INTERNAL_TYPES_H_
#define _SIGXMACROS_INTERNAL_TYPES_H_


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

#include <sigc++/type_traits.h>	// sigc::is_base_and_derived
#include <sigc++/functors/functor_trait.h>	// sigc::nil
#include <sigc++/adaptors/deduce_result_type.h>	// sigc::adaptor_base
#include <sigc++/adaptors/adaptor_trait.h>	// sigc::adaptor_functor
#include <sigc++/adaptors/bind.h>	// sigc::bind_functor
#include <sigc++/adaptors/hide.h>	// sigc::hide_functor
#include <sigc++/functors/slot_base.h>	// sigc::slot_base
#include <sigx/fwddecl.h>
#include <sigx/types.h>


// functor attaching a shared_dispatchable to another functor by binding
// the shared_dispatchable and hiding it
#define SIGX_DISPATCH_WITH_FUNCTOR(T_functor)\
	sigc::bind_functor<-1, sigc::hide_functor<-1, T_functor>, shared_dispatchable>


namespace sigx
{

	namespace internal
	{


/**	@short counts the provided template arguments. There are specializations
 *	for 1 to (7-1) template arguments that are not sigc::nil
 */
template <typename T_arg1 = sigc::nil, typename T_arg2 = sigc::nil, typename T_arg3 = sigc::nil, typename T_arg4 = sigc::nil, typename T_arg5 = sigc::nil, typename T_arg6 = sigc::nil, typename T_arg7 = sigc::nil>
struct count_arguments
  { static const int value = 7; };

template <>
struct count_arguments<>
  { static const int value = 0; };
template <typename T_arg1>
struct count_arguments<T_arg1>
  { static const int value = 1; };
template <typename T_arg1, typename T_arg2>
struct count_arguments<T_arg1, T_arg2>
  { static const int value = 2; };
template <typename T_arg1, typename T_arg2, typename T_arg3>
struct count_arguments<T_arg1, T_arg2, T_arg3>
  { static const int value = 3; };
template <typename T_arg1, typename T_arg2, typename T_arg3, typename T_arg4>
struct count_arguments<T_arg1, T_arg2, T_arg3, T_arg4>
  { static const int value = 4; };
template <typename T_arg1, typename T_arg2, typename T_arg3, typename T_arg4, typename T_arg5>
struct count_arguments<T_arg1, T_arg2, T_arg3, T_arg4, T_arg5>
  { static const int value = 5; };
template <typename T_arg1, typename T_arg2, typename T_arg3, typename T_arg4, typename T_arg5, typename T_arg6>
struct count_arguments<T_arg1, T_arg2, T_arg3, T_arg4, T_arg5, T_arg6>
  { static const int value = 6; };



/**	@short finds out whether @e T_functor is tunneled, i.e.
 *	whether the functor chain contains a functor derived from sigx::tunnel_base.
 *	
 *	To investigate the fact that there exists one functor derived from 
 *	sigx::tunnel_base, this template struct cascades down the functor chain.
 *	e.g. bind_functor - tunnel_functor - member_functor
 *	
 *	The template argument @e T_functor is the functor type.
 *	@e I_isadaptor indicates whether @e T_functor inherits from sigc::adaptor_base.
 *	@e I_istunnel indicates whether @e T_functor inherits from sigx::tunnel_base.
 */
template<typename T_functor, 
bool I_istunnel =
	sigc::is_base_and_derived<sigx::tunnel_base, T_functor>::value, 
bool I_isadaptor =
	sigc::is_base_and_derived<sigc::adaptor_base, T_functor>::value>
struct is_functor_tunneled;


/**	@short finds out whether the functor chain contains a tunnel functor.
 *	This specialization is used for tunneled adaptors.
 */
template<typename T_functor>
struct is_functor_tunneled<T_functor, true, true>
{
	static const bool value = true;
};

/**	@short finds out whether the functor chain contains a tunnel functor.
 *	This specialization is used for tunneled arbitrary functors.
 */
template<typename T_functor>
struct is_functor_tunneled<T_functor, true, false>
{
	static const bool value = true;
};

/**	@short finds out whether the functor chain contains a tunnel functor.
 *	This specialization is used for arbitrary functors, static and member
 *	function pointers.
 */
template<typename T_functor>
struct is_functor_tunneled<T_functor, false, false>
{
	// no chance to investigate further, probably a static or member function
	// pointer
	static const bool value = false;
};

/**	@short finds out whether the functor chain contains a tunnel functor.
 *	This specialization is used for adaptors
 */
template<typename T_functor>
struct is_functor_tunneled<T_functor, false, true>
{
	// investigate further by cascading the functor chain
	// functor must define adaptor_type;
	// T_functor is probably derived from sigc::adapts<T_functor>; defines
	// adaptor_type
	typedef typename T_functor::adaptor_type adaptor_type;
	static const bool value = is_functor_tunneled<adaptor_type>::value;
};

/**	@short finds out whether the functor chain contains a tunnel functor.
 *	This specialization is used for sigc::adaptor_functors wrapping
 *	static and arbitrary functors.
 *	This specialization is needed because sigc::adaptor_functor does not define
 *	its wrapped functor as adaptor_type
 */
template<typename T_functor>
struct is_functor_tunneled<sigc::adaptor_functor<T_functor>, false, true>
{
	static const bool value = is_functor_tunneled<T_functor>::value;
};




/**	@short finds out whether @e T_functor is tunneled, i.e.
 *	whether the functor chain contains a functor derived from sigx::tunnel_base.
 *	
 *	To investigate the fact that there exists one functor derived from 
 *	sigx::tunnel_base, this template struct cascades down the functor chain.
 *	e.g. bind_functor - tunnel_functor - member_functor
 *	
 *	The template argument @e T_functor is the functor type.
 *	@e I_isadaptor indicates whether @e T_functor inherits from sigc::adaptor_base.
 *	@e I_isslot indicates whether @e T_functor inherits from sigc::slot_base.
 */
template<
	typename T_functor, 
	bool I_isslot = sigc::is_base_and_derived<sigc::slot_base, T_functor>::value, 
	bool I_isadaptor = sigc::is_base_and_derived<sigc::adaptor_base, T_functor>::value>
struct is_or_adapts_slot;


/**	@short finds out whether the functor chain contains a tunnel functor.
 *	This specialization is used for tunneled adaptors.
 */
template<typename T_functor>
struct is_or_adapts_slot<T_functor, true, false>
{
	static const bool value = true;
};

/**	@short finds out whether the functor chain contains a tunnel functor.
 *	This specialization is used for arbitrary functors, static and member
 *	function pointers.
 */
template<typename T_functor>
struct is_or_adapts_slot<T_functor, false, false>
{
	// no chance to investigate further, probably a static or member function
	// pointer
	static const bool value = false;
};

/**	@short finds out whether the functor chain contains a tunnel functor.
 *	This specialization is used for adaptors
 */
template<typename T_functor>
struct is_or_adapts_slot<T_functor, false, true>
{
	// investigate further by cascading the functor chain
	// functor must define adaptor_type;
	// T_functor is probably derived from sigc::adapts<T_functor>; defines
	// adaptor_type
	typedef typename T_functor::adaptor_type adaptor_type;
	static const bool value = is_or_adapts_slot<adaptor_type>::value;
};

/**	@short finds out whether the functor chain contains a tunnel functor.
 *	This specialization is used for sigc::adaptor_functors wrapping
 *	static and arbitrary functors.
 *	This specialization is needed because sigc::adaptor_functor does not define
 *	its wrapped functor as adaptor_type
 */
template<typename T_functor>
struct is_or_adapts_slot<sigc::adaptor_functor<T_functor>, false, true>
{
	static const bool value = is_or_adapts_slot<T_functor>::value;
};




/**	@short Used to find a dispatchable out of the functor/adaptor chain,
 *	i.e. the dispatchable object of a mem_functor or the shared_dispatchable
 *	stored by dispatch_with().
 *	
 *	It additionally checks at compile time that T_functor is really a functor on a
 *	dispatchable's method by accessing the member variable obj_ of mem_functors
 *	and converting the object to a sigx::dispatchable;
 *	hence, the compiler will issue error messages if it is not a mem_functor or the object
 *	is not convertible to a sigx::dispatchable
 */
template<typename T_functor, bool I_isadapter = sigc::is_base_and_derived<sigc::adaptor_base, T_functor>::value>
struct dispatchable_constraint;


template<typename T_functor>
struct dispatchable_constraint<T_functor, false>
{
	static const dispatchable& find_dispatchable(typename sigc::type_trait<T_functor>::take _A_func)
	{
		// if the compiler reports an error there are only 2 possibilities:
		// 1) you connected a static function to a signal or opened a tunnel on a 
		//    static signal without specifying the dispatchable to operate on.
		// 2) you connected a member function to a signal or opened a tunnel on a 
		//    member function where the member function is not from a dispatchable
		// solution: call 'open_tunnel_with' or 'open_sync_tunnel_with' and specify the dispatachable!

		// access obj_ of the functor;
		// this issues a compile time error if _A_func is not a functor
		// on an object's member function (only sigc member functors have this)
		// obj_ is a (const_)limit_reference type; must call invoke() to 
		// ensure that we get the bound member type instead of a sigc::trackable
		return _A_func.obj_.invoke();
	}
};

// walks down the adaptor chain
template<typename T_functor>
struct dispatchable_constraint<T_functor, true>
{
	static const dispatchable& find_dispatchable(typename sigc::type_trait<T_functor>::take _A_func)
	{
		return is_adaptor_dispatchable(_A_func);
	}
	
private:
	// takes T_functor and matches it again to find
	// special types and walk down the adaptor chain
	template<typename T_adapted_functor>
	static const dispatchable& is_adaptor_dispatchable(const sigc::adaptor_functor<T_adapted_functor>& _A_func)
	{
		return dispatchable_constraint<T_adapted_functor>::find_dispatchable(_A_func.functor_);
	}

	// match the sigx special dispatchable adaptor chain
	template<typename T_adapted_functor>
	static const dispatchable& is_adaptor_dispatchable(const SIGX_DISPATCH_WITH_FUNCTOR(T_adapted_functor)& _A_func)
	{
		// no need to proceed as we found out that the functor is dispatchable
		return _A_func.bound1_.visit();
	}

	// match all other apaptors, must define adaptor_type
	template<typename T_adapted_functor>
	static const dispatchable& is_adaptor_dispatchable(const T_adapted_functor& _A_func)
	{
		// T_adapted_functor must define adaptor_type;
		typedef typename T_adapted_functor::adaptor_type adaptor_type;
		// _A_func.functor_ is always an adaptor_type
		return is_adaptor_dispatchable(_A_func.functor_);
	}
};


	} // namespace internal

} // namespace sigx
#endif /* _SIGXMACROS_INTERNAL_TYPES_H_ */
