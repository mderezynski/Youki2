#ifndef _SIGX_VOLATILE_TRAIT_H_
#define _SIGX_VOLATILE_TRAIT_H_

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

/*
 * Inspired by Andrei Alexandrescu's article "volatile - Multithreaded 
 * Programmer's Best Friend":
 * http://www.ddj.com/dept/cpp/184403766
*/


namespace sigx
{

/**	@addtogroup threadsafety
 *	@{
 */

/**	@short Traits for adding/removing the volatile qualifier from a type
 */
template<typename T_type>
struct volatile_trait
{
    typedef volatile T_type add;
    typedef T_type remove;
};

/**	@short Specialization for volatile types
 */
template<typename T_type>
struct volatile_trait<volatile T_type>
{
    typedef volatile T_type add;
    typedef T_type remove;
};

/**	@short Specialization for references to non-volatile types.
 *	
 *	Specializations for references do not consider a reference as a "top level"
 *	type qualifer unlike pointers; therefore, they add or remove the 
 *	volatileness to/from the referenced type:
 *	volatile_trait<int&>::add -> volatile int&
 *	volatile_trait<int&>::remove -> int&
 *	volatile_trait<volatile int&>::add -> volatile int&
 *	volatile_trait<volatile int&>::remove -> int&
 *	
 *	(
 *	a different placing of the qualifier, same type:
 *	volatile_trait<int&>::add -> int volatile&
 *	volatile_trait<int&>::remove -> int&
 *	volatile_trait<int volatile&>::add -> int volatile&
 *	volatile_trait<int volatile&>::remove -> int&
 *	)
 *	
 *	whereas pointers themselves are treated like "top level" type qualifiers.
 *	Hence, they add or remove the volatileness from the pointer type qualifier:
 *	volatile_trait<int*>::add -> volatile int*
 *	volatile_trait<int*>::remove -> int*
 *	volatile_trait<volatile int*>::add -> volatile int*
 *	volatile_trait<volatile int*>::remove -> int*
 *	volatile_trait<int* volatile>::add -> int* volatile
 *	volatile_trait<int* volatile>::remove -> int*
 *	
 *	(
 *	a different placing of the qualifier, same type:
 *	volatile_trait<int*>::add -> int volatile*
 *	volatile_trait<int*>::remove -> int*
 *	volatile_trait<int volatile*>::add -> int volatile*
 *	volatile_trait<int volatile*>::remove -> int*
 *	volatile_trait<int* volatile>::add -> int* volatile
 *	volatile_trait<int* volatile>::remove -> int*
 *	)
 *	
 *	This is a major difference to boost::type_traits that consider a reference
 *	as a top level type qualifier
 */
template<typename T_type>
struct volatile_trait<T_type&>
{
    typedef volatile T_type& add;
    typedef T_type& remove;
};

/**	@short Specialization for references to volatile types
 */
template<typename T_type>
struct volatile_trait<volatile T_type&>
{
    typedef volatile T_type& add;
    typedef T_type& remove;
};



template<typename T, typename T_src>
T volatile_cast(T_src& tsrc)
{
	return const_cast<T>(tsrc);
}



// @addtogroup threadsafety
/**	@}
 */

} // namespace sigx


#endif // end file guard
