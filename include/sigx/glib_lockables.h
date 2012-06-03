#ifndef _SIGX_GLIB_LOCKABLES_HPP_
#define _SIGX_GLIB_LOCKABLES_HPP_

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

#include <sigx/lockable.h>
#include <sigx/choose_lock.h>
#include <glibmm/thread.h>


namespace sigx
{

/**	@addtogroup threadsafety
 *	@{
 */

/**	@short Makes @e T_type read/write %lockable with a Glib::RWLock
 */
template<typename T_type>
struct rw_lockable: public lockable<T_type, Glib::RWLock>
{
	typedef lockable<T_type, Glib::RWLock> parent_type;

public:
	rw_lockable(): 
		parent_type()
	{}
	rw_lockable(typename parent_type::const_reference_type v): 
		parent_type(v)
	{}
};

/**	@short Makes @e T_type %lockable with a Glib::Mutex
 */
template<typename T_type>
class mutex_lockable: public lockable<T_type, Glib::Mutex>
{
	typedef lockable<T_type, Glib::Mutex> parent_type;

public:
	mutex_lockable(): 
		parent_type()
	{}
	mutex_lockable(typename parent_type::const_reference_type v): 
		parent_type(v)
	{}
};

/**	@short Makes @e T_type %lockable with a Glib::StaticMutex
 */
template<typename T_type>
class static_mutex_lockable: public lockable<T_type, Glib::StaticMutex>
{
	typedef lockable<T_type, Glib::StaticMutex> parent_type;

public:
	static_mutex_lockable(): 
		parent_type()
	{
		g_static_mutex_init(this->m_mutex.gobj());
	}
	
	static_mutex_lockable(typename parent_type::const_reference_type v): 
		parent_type(v)
	{
		g_static_mutex_init(this->m_mutex.gobj());
	}
};

/**	@short Makes @e T_type %lockable with a Glib::RecMutex
 */
template<typename T_type>
class recmutex_lockable: public lockable<T_type, Glib::RecMutex>
{
	typedef lockable<T_type, Glib::RecMutex> parent_type;

public:
	recmutex_lockable(): 
		parent_type()
	{}
	recmutex_lockable(typename parent_type::const_reference_type v): 
		parent_type(v)
	{}
};

/**	@short Makes @e T_type %lockable with a Glib::StaticRecMutex
 */
template<typename T_type>
class static_recmutex_lockable: public lockable<T_type, Glib::StaticRecMutex>
{
	typedef lockable<T_type, Glib::StaticRecMutex> parent_type;

public:
	static_recmutex_lockable(): 
		parent_type()
	{
		g_static_rec_mutex_init(this->m_mutex.gobj());
	}

	static_recmutex_lockable(typename parent_type::const_reference_type v): 
		parent_type(v)
	{
		g_static_rec_mutex_init(this->m_mutex.gobj());
	}
};


template<>
struct choose_lock<Glib::RWLock, readlock>
{
	typedef Glib::RWLock::ReaderLock type;
};

template<>
struct choose_lock<Glib::RWLock, writelock>
{
	typedef Glib::RWLock::WriterLock type;
};

template<locking_policy I_policy>
struct choose_lock<Glib::Mutex, I_policy>
{
	typedef Glib::Mutex::Lock type;
};

template<locking_policy I_policy>
struct choose_lock<Glib::RecMutex, I_policy>
{
	typedef Glib::RecMutex::Lock type;
};

template<locking_policy I_policy>
struct choose_lock<Glib::StaticMutex, I_policy>
{
	typedef Glib::/*Static*/Mutex::Lock type;
};

template<locking_policy I_policy>
struct choose_lock<Glib::StaticRecMutex, I_policy>
{
	typedef Glib::/*Static*/RecMutex::Lock type;
};




// @addtogroup threadsafety
/**	@}
 */

} // namespace sigx


#endif // end file guard
