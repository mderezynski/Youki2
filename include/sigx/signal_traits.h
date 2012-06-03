#ifndef _SIGX_SIGNAL_TYPE_TRAIT_HPP_
#define _SIGX_SIGNAL_TYPE_TRAIT_HPP_

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

#include <sigc++/signal_base.h>
#include <glibmm/signalproxy.h>


namespace sigx
{

	namespace internal
	{

struct derivation_helper
{
	typedef char sm;
	struct middle {
		char memory[32];
	};
	struct big {
		char memory[64];
	};

	enum Type
	{
		NOBASE = sizeof(sm), 
		BASE1 = sizeof(middle), 
		BASE2 = sizeof(big)
	};
};

/**	@short Compile-time determination of base-class relationship in C++.
 *	
 *	Tests from what base class @e T_derived derives. T_derived must not derive
 *	from both classes.
 *	@note inspired by sigc::is_base_and_derived
 *	@author klaus triendl
 *	@date 2006-09-11, kj	created
 */
template<typename T_derived, typename T_base1, typename T_base2>
struct is_derived_from
{
private:
#ifndef SIGC_SELF_REFERENCE_IN_MEMBER_INITIALIZATION

	//Certain compilers, notably GCC 3.2, require these functions to be inside an inner class.
	struct internal_class
	{
		static derivation_helper::sm is_base_class_(...);
		static derivation_helper::middle is_base_class_(typename sigc::type_trait<T_base1>::pointer);
		static derivation_helper::big is_base_class_(typename sigc::type_trait<T_base2>::pointer);
	};

public:
	static const int value =
		sizeof(internal_class::is_base_class_(reinterpret_cast<typename sigc::type_trait<T_derived>::pointer>(0)));

#else //SIGC_SELF_REFERENCE_IN_MEMBER_INITIALIZATION

	//The AIX xlC compiler does not like these 2 functions being in the inner class.
	//It says "The incomplete type "test" must not be used as a qualifier.
	//It does not seem necessary anyway. murrayc.
	static derivation_helper::sm is_base_class_(...);
	static derivation_helper::middle is_base_class_(typename sigc::type_trait<T_base1>::pointer);
	static derivation_helper::big is_base_class_(typename sigc::type_trait<T_base2>::pointer);

public:
	static const int value =
		sizeof(is_base_class_(reinterpret_cast<typename sigc::type_trait<T_derived>::pointer>(0)));

#endif //SIGC_SELF_REFERENCE_IN_MEMBER_INITIALIZATION

 	void avoid_gcc3_warning_(); //Not implemented. g++ 3.3.5 (but not 3.3.4, and not 3.4) warn that there are no public methods, even though there is a public variable.
};


enum signal_group
{
	SIGGROUP_IRRELEVANT, 
	SIGGROUP_SIGC, 
	SIGGROUP_GLIB_PROXY
};


/**	@short Trait to group signal types.
 *	@note We could use specializations for all signals like 
 *	count_signal_arguments does. But by finding the baseclass we can reduce 
 *	the specializations.
 */
template<typename T_signal, int I_oneof = is_derived_from<T_signal, sigc::signal_base, Glib::SignalProxyNormal>::value>
struct signal_type_trait
{
	static const signal_group type = SIGGROUP_IRRELEVANT;
};

/**	@short Specialization for sigc::signal_base derived signals
 */
template<typename T_signal>
struct signal_type_trait<T_signal, derivation_helper::BASE1>
{
	static const signal_group type = SIGGROUP_SIGC;
};

/**	@short Specialization for Glib::SignalProxyNormal derived signals
 */
template<typename T_signal>
struct signal_type_trait<T_signal, derivation_helper::BASE2>
{
	static const signal_group type = SIGGROUP_GLIB_PROXY;
};


	} // namespace internal

} // namespace sigx


#endif // end file guard
