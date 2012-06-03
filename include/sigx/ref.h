#ifndef _SIGX_AUTO_REF_H_
#define _SIGX_AUTO_REF_H_

/*
 * Copyright 2008 Klaus Triendl
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

/**	@file	Template function "ref" exactly the same as sigc::ref but 
 *			overloaded for types already being wrapped in a 
 *			sigc::reference_wrapper such that ref() returns a new 
 *			reference_wrapper for the type to be wrapped instead of 
 *			wrapping the reference_wrapper itself.
 */

#include <sigc++/reference_wrapper.h>


namespace sigx
{


template <class T_type>
sigc::reference_wrapper<T_type> ref(T_type& v)
{
	return sigc::reference_wrapper<T_type>(v);
}

template <class T_type>
sigc::const_reference_wrapper<T_type> ref(const T_type& v)
{
	return sigc::const_reference_wrapper<T_type>(v);
}


template <class T_type>
sigc::reference_wrapper<T_type> ref(const sigc::reference_wrapper<T_type>& v)
{
	return sigc::reference_wrapper<T_type>(v);
}

template <class T_type>
sigc::const_reference_wrapper<T_type> ref(const sigc::const_reference_wrapper<T_type>& v)
{
	return sigc::const_reference_wrapper<T_type>(v);
}


}	// namespace sigx


#endif	// file guard
