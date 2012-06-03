#ifndef _SIGX_AUTO_TUNNELER_HPP_
#define _SIGX_AUTO_TUNNELER_HPP_

/*
 * Copyright 2005 Tim Mayberry
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

#include <sigxconfig.h>
#include <sigx/internal_types.h>
#include <sigx/tunnel_functor.h>


namespace sigx
{

	namespace internal
	{

/**	@short Automatic creation of a tunnel functor if @e T_functor
 *	is not yet tunneled.
 *	
 *	This general version is used for non-tunneled functors.
 */
template<typename T_functor, 
	bool I_istunneled = is_functor_tunneled<T_functor>::value>
struct auto_tunneler
{
	static const bool is_tunneled = false;
	typedef tunnel_functor<ASYNC, T_functor> functor_type;
	static functor_type auto_open_tunnel(const T_functor& _A_func)
	{
		return open_tunnel(_A_func);
	}
};

/**	@short This specialization is used for tunneled functors.
 *	
 *	Just returns the functor passed to auto_open_tunnel.
 */
template<typename T_functor>
struct auto_tunneler<T_functor, true>
{
	static const bool is_tunneled = true;
	typedef T_functor functor_type;
	static const functor_type& auto_open_tunnel(const T_functor& _A_func)
	{
		return _A_func;
	}
};


	} // namespace internal

} // namespace sigx


#endif // end file guard
