#ifndef _SIGX_CHOOSE_LOCK_TRAIT_H_
#define _SIGX_CHOOSE_LOCK_TRAIT_H_

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

/**	@file 
 *	lock traits are used to choose the best lock type for a given mutex.
 *	E.g. For a mutex type boost::mutex lock type boost::mutex::scoped_lock 
 *	is chosen
 */

#include <sigx/lockable_fwddecl.h>


namespace sigx
{

/**	@addtogroup threadsafety
 *	@{
 */

/*enum locking_policy
{
    readlock, 
    writelock
};*/

/**	@short	Metafunction that chooses an appropriate scoped lock for a mutex.
 *	
 *	The lock type should be a scoped lock because lock_acquirer is a scope-bound 
 *	type.
 *	
 *	@note	There is no default lock type choosing mechanism because there is no 
 *			such thing as a default or commonly used mutex.
 *			This means that the using programmer has to specialize this metafunction 
 *			for her mutexes and the locking policy
 */
template<typename T_mutex, locking_policy I_policy>
struct choose_lock;


#if 0
/**	@short r/w lock trait for a CRecMutex
 */
template<locking_policy I_policy>
struct choose_lock<boost::mutex, I_policy>
{
    typedef boost::mutex::scoped_lock type;
};
#endif


} // namespace sigx


#endif // end file guard
