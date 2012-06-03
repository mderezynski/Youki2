#ifndef _SIGX_LOCKABLE_H_
#define _SIGX_LOCKABLE_H_

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

/*
 * Inspired by Andrei Alexandrescu's article "volatile - Multithreaded 
 * Programmer's Best Friend":
 * http://www.ddj.com/dept/cpp/184403766
*/

#include <tr1/type_traits>
#include <sigx/noncopyable.h>
#include <sigx/lockable_fwddecl.h>
#include <sigx/const_trait.h>
#include <sigx/volatile_trait.h>


namespace sigx
{

/**	@addtogroup threadsafety
 *	@{
 */

/**	@short The base for all lockables, template specialized for a specific 
 *	lock, e.g. a boost::mutex.
 *	
 *	Lockables are wrapper objects pairing a certain type together with a mutex type.
 *	Mutex objects of type T_mutex must be default constructible.
 *	
 */
template<typename T_mutex>
struct lockable_base: noncopyable
{
    typedef T_mutex mutex_type;


    mutex_type& mutex() const throw()
    {
        return m_mutex;
    }

protected:
    lockable_base(): 
        m_mutex()
    {}


    /**	@note mutable in case that lockable_base is const.
     */
    mutable mutex_type m_mutex;
};



/**	@short Makes @e T_type %lockable.
 *	
 *	The "safe" in safe_lockable means that access to the mutex and the locked type is denied, they are only accessible through a lock_acquirer.
 *	
 *	The following template arguments are used:
 *	- @e T_type The type to be protected, e.g. an int
 *	- @e T_mutex The mutex type to protect @e T_type, e.g. a boost::mutex
 *	
 *	@code
 *	typedef lockable<int, boost::mutex> mutex_lockable_int;
 *	@endcode
 *	
 *	@note	lockables are inseparably tied together and constness for lockables and the locked type is transitive, i.e
 *			no matter whether the type to protect (@e T_type) or the lockable itself is somewhere declared const you get only 
 *			const access to the variable to protect
*/
template<typename T_type, typename T_mutex>
struct safe_lockable: public lockable_base<T_mutex>
{
    // lock_acquirer can use interface methods
    template<locking_policy I_policy, typename T_type1, typename T_mutex1, typename T_islockable> friend class lock_acquirer;

    typedef lockable_base<T_mutex> parent_type;
    // acquired_type = type to protect, 1:1 from T_type
    typedef T_type acquired_type;
    // volatile_type = make T_type volatile, even if T_type is a reference
    // volatile T_type or volatile T_type&
    typedef typename volatile_trait<acquired_type>::add volatile_type;
    // reference_type = reference to volatile-stripped T_type
    // T_type&
    typedef typename std::tr1::add_reference<typename volatile_trait<acquired_type>::remove>::type reference_type;
    // volatile_reference_type = reference to volatile T_type, even if T_type is a reference
    // volatile T_type&
    typedef typename std::tr1::add_reference<volatile_type>::type volatile_reference_type;
    // reference_type = reference to volatile-stripped T_type, even if T_type is a reference
    // const T_type&
    typedef typename std::tr1::add_reference<typename const_trait<reference_type>::add>::type const_reference_type;
    // cv_reference_type = reference to cv-qualified T_type, even if T_type is a reference
    // const volatile T_type&
    typedef typename std::tr1::add_reference<typename const_trait<volatile_type>::add>::type cv_reference_type;
    // apply const qualifier and reference to toplevel type, unchanged if toplevel type is a reference
    typedef typename std::tr1::add_reference<typename std::tr1::add_const<acquired_type>::type>::type toplevel_const_reference_type;


    /**	@short Default constructor.
     *	
     *	@e T_type is initialized with its default ctor or its default value
     */
    safe_lockable(): 
        parent_type(), 
        m_obj()
    {}
    
    /**	@short Constructs a lockable initializing @e T_type with @e _a_value
     */
    safe_lockable(toplevel_const_reference_type _a_value): 
        parent_type(), 
        m_obj(_a_value)
    {}


protected:
    /**	@return reference to volatile @e T_type
     */
    volatile_reference_type access_volatile() throw()
    {
        return m_obj;
    }

    /**	@return reference to non-volatile @e T_type
     */
    reference_type access_nonvolatile() throw()
    {
        // volatile_cast m_obj
        return const_cast<reference_type>(m_obj);
    }

    /**	@return reference to volatile @e T_type
     */
    cv_reference_type access_volatile() const throw()
    {
        return m_obj;
    }

    /**	@return reference to non-volatile @e T_type
     */
    const_reference_type access_nonvolatile() const throw()
    {
        // volatile_cast m_obj
        return const_cast<const_reference_type>(m_obj);
    }

    
private:
    /**	@short store volatile @e T_type
     */
    volatile_type m_obj;
};


/**	@short	Refinement of safe_lockable, open access to mutex and locked type.
 */
template<typename T_type, typename T_mutex>
struct lockable: public safe_lockable<T_type, T_mutex>
{
    typedef safe_lockable<T_type, T_mutex> parent_type;
	typedef typename parent_type::toplevel_const_reference_type toplevel_const_reference_type;

public:
    /**	@short Default constructor.
     *	
     *	@e T_type is initialized with its default ctor or its default value
     */
    lockable(): 
        parent_type()
    {}
    
    /**	@short Constructs a lockable initializing @e T_type with @e _a_value
     */
    lockable(toplevel_const_reference_type _a_value): 
        parent_type(_a_value)
    {}

    // make safe_lockable's interface publicly available
    using parent_type::access_volatile;
    using parent_type::access_nonvolatile;
};



#if 0 // specializations for pointers
/**	@short Makes a void pointer %lockable.
 *	
 *	The following template arguments are used:
 *	- @e T_mutex The lock to protect @e void*, e.g. a boost::mutex
 *	
 *	@code
 *	typedef lockable<void*, boost::mutex> mutex_lockable_void_ptr;
 *	@endcode
 */
template<typename T_mutex>
struct lockable<void*, T_mutex>: public lockable_base<T_mutex>
{
    typedef void* acquired_type;
    typedef T_mutex mutex_type;
    typedef lockable_base<mutex_type> parent_type;
    typedef lockable<acquired_type, mutex_type> type;
    typedef typename volatile_trait<acquired_type>::add volatile_type; 
    typedef typename std::tr1::add_reference<typename volatile_trait<acquired_type>::remove>::type reference_type;
    typedef typename std::tr1::add_reference<typename volatile_trait<acquired_type>::add>::type volatile_reference_type;
    typedef typename std::tr1::add_reference<typename std::tr1::add_const<acquired_type>::type>::type take_type;

    
    /**	@short Default constructor.
     *	
     *	The void pointer is initialized with the provided pointer or to 0
     */
    lockable(take_type _a_value = 0): 
        parent_type(), 
        m_obj(_a_value)
    {}

    /**	@return reference to volatile void*
     */
    volatile_reference_type access_volatile()
    {
        return m_obj;
    }

    /**	@return reference to volatile @e T_type
     */
    reference_type access_nonvolatile()
    {
        // volatile_cast m_obj
        return const_cast<reference_type>(m_obj);
    }

    
private:
    /**	@short volatile void*
     */
    volatile_type m_obj;
};



/**	@short Makes any type of pointer %lockable.
 *	
 *	The following template arguments are used:
 *	- @e T_type The pointer type to be protected, e.g. an int*
 *	- @e T_mutex The lock to protect @e T_type, e.g. a boost::mutex
 *	
 *	@code
 *	typedef lockable<int*, boost::mutex> mutex_lockable_int_ptr;
 *	@endcode
 */
template<typename T_type, typename T_mutex>
struct lockable<T_type*, T_mutex>: public lockable<void*, T_mutex>
{
    typedef lockable<void*, T_mutex> parent_type;
    typedef T_type* acquired_type;
    typedef lockable<acquired_type, mutex_type> type;
    typedef typename volatile_trait<acquired_type>::add volatile_type; 
    typedef typename std::tr1::add_reference<typename volatile_trait<acquired_type>::remove>::type reference_type;
    typedef typename std::tr1::add_reference<typename volatile_trait<acquired_type>::add>::type volatile_reference_type;
    typedef typename std::tr1::add_reference<typename std::tr1::add_const<acquired_type>::type>::type take_type;

    
    /**	@short default ctor.
     *	
     *	@e T_type is initialized with the provided pointer or to 0
     */
    lockable(take_type _a_value = 0): 
        parent_type((void*&) _a_value)
    {}

    /**	@return reference to volatile @e T_type
        problem by now:
        void* volatile p = 0;
        const int* volatile& p1 = reinterpret_cast<const int* volatile&>(p);
     */
    volatile_reference_type access_volatile()
    {
        return (volatile_reference_type) parent_type::access_volatile();
    }

    /**	@return reference to volatile @e T_type
     */
    reference_type access_acquiree()
    {
        return (reference_type) parent_type::access_acquiree();
    }
};
#endif



// @addtogroup threadsafety
/**	@}
 */

} // namespace sigx


#endif // end file guard
