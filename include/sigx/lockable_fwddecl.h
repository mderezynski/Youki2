#ifndef _SIGX_LOCKABLE_FWDDECL_HPP_
#define _SIGX_LOCKABLE_FWDDECL_HPP_

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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free 
 * Software Foundation, 51 Franklin Street, Fifth Floor, 
 * Boston, MA 02110-1301, USA.
*/

#include <tr1/type_traits>


namespace sigx
{

//enum locking_policy;
enum locking_policy
{
    readlock, 
    writelock
};

template<typename T_mutex, locking_policy I_policy>
struct choose_lock;

template<typename T_mutex> 
struct lockable_base;
template<typename T_type, typename T_mutex> 
struct lockable;
template<typename T_type, typename T_mutex> 
struct safe_lockable;

template<locking_policy I_policy, typename T_type, typename T_mutex = typename T_type::mutex_type, typename T_islockable = typename std::tr1::is_base_of<lockable_base<T_mutex>, T_type>::type>
class lock_acquirer;
template<typename T_type, typename T_mutex = typename T_type::mutex_type, typename T_islockable = typename std::tr1::is_base_of<lockable_base<T_mutex>, T_type>::type>
class writelock_acquirer;
template<typename T_type, typename T_mutex = typename T_type::mutex_type, typename T_islockable = typename std::tr1::is_base_of<lockable_base<T_mutex>, T_type>::type>
class readlock_acquirer;

}


#endif	// end file guard
