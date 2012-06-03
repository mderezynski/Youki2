#ifndef _SIGX_CONST_TRAIT_HPP_
#define _SIGX_CONST_TRAIT_HPP_

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

/**	@short Traits for adding/removing the const qualifier from a type
 */
template<typename T_type>
struct const_trait
{
    typedef const T_type add;
    typedef T_type remove;
};

/**	@short Specialization for const types
 */
template<typename T_type>
struct const_trait<const T_type>
{
    typedef const T_type add;
    typedef T_type remove;
};

/**	@short Specialization for references to non-const types.
 *	
 *	Specializations for references do not consider a reference as a "top level"
 *	type qualifer unlike pointers; therefore, they add or remove the 
 *	volatileness to/from the referenced type:
 *	const_trait<int&>::add -> const int&
 *	const_trait<int&>::remove -> int&
 *	const_trait<const int&>::add -> const int&
 *	const_trait<const int&>::remove -> int&
 *	
 *	(
 *	a different placing of the qualifier, same type:
 *	const_trait<int&>::add -> int const&
 *	const_trait<int&>::remove -> int&
 *	const_trait<int const&>::add -> int const&
 *	const_trait<int const&>::remove -> int&
 *	)
 *	
 *	whereas pointers themselves are treated like "top level" type qualifiers.
 *	Hence, they add or remove the volatileness from the pointer type qualifier:
 *	const_trait<int*>::add -> const int*
 *	const_trait<int*>::remove -> int*
 *	const_trait<const int*>::add -> const int*
 *	const_trait<const int*>::remove -> int*
 *	const_trait<int* const>::add -> int* const
 *	const_trait<int* const>::remove -> int*
 *	
 *	(
 *	a different placing of the qualifier, same type:
 *	const_trait<int*>::add -> int const*
 *	const_trait<int*>::remove -> int*
 *	const_trait<int const*>::add -> int const*
 *	const_trait<int const*>::remove -> int*
 *	const_trait<int* const>::add -> int* const
 *	const_trait<int* const>::remove -> int*
 *	)
 *	
 *	This is a major difference to boost::type_traits that consider a reference
 *	as a top level type qualifier
 */
template<typename T_type>
struct const_trait<T_type&>
{
    typedef const T_type& add;
    typedef T_type& remove;
};

/**	@short Specialization for references to const types
 */
template<typename T_type>
struct const_trait<const T_type&>
{
    typedef const T_type& add;
    typedef T_type& remove;
};



// @addtogroup threadsafety
/**	@}
 */

} // namespace sigx


#endif // end file guard
