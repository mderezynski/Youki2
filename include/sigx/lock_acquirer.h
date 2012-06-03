#ifndef _SIGX_LOCK_ACQUIRER_H_
#define _SIGX_LOCK_ACQUIRER_H_

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

#include <tr1/type_traits>
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/identity.hpp>
#include <sigx/noncopyable.h>
//#include <sigx/nonheapallocatable.h>
#include <sigx/nonpointeraliasing.h>
#include <sigx/lockable_fwddecl.h>
#include <sigx/choose_lock.h>


namespace sigx
{

/**	@addtogroup threadsafety
 *	@{
 */


/**	@short Locks the given mutex and ensures threadsafe write 
 *	access to the given locked type.
 *	
 *	Collects acquisition of a mutex lock and a volatile_cast from a 
 *	volatile object.
 *	A lock_acquirer object is initialized with a lock from an associated mutex 
 *	and a volatile object.
 *	The appropriate lock is chosen by the metafunction choose_lock according to the 
 *	mutex and the locking policy (read/write). Note that because the lock_acquirer is 
 *	scope bound choose_lock must only choose scoped lock types.
 *	
 *	During its lifetime, a lock_acquirer keeps the lock acquired. Also, lock_acquirer 
 *	offers read or write access (according to the locking policy) to the volatile-stripped object. 
 *	Access is granted by a protected friend template function access_acquiree(). 
 *	The volatile_cast is performed by access_acquiree(). 
 *	The cast is semantically valid because lock_acquirer keeps the lock acquired 
 *	for its lifetime.
 *	
 *	If the locking policy is readlock then the lock_acquirer grants only const access to the 
 *	protected variable.
 *	
 *	The following template arguments are used:
 *	- @e T_type The type to be protected by the lock, e.g. an int.
 *	- @e T_mutex The lock, e.g. a Glib::Mutex.
 *	- @e I_islockable Whether T_type derives from lockable_base
 *	
 *	@note The locked type can only be accessed with access_acquiree()
 *	@code
 *	// somewhere
 *	boost::mutex mtx;
 *	int x;
 *	
 *	// a scope somewhere else
 *	{
 *		lock_acquirer<writelock, int, boost::mutex> l(x, mtx);
 *		int& i = access_acquiree(l);
 *		i = 42;
 *	}
 *	@endcode
 */
template<locking_policy I_policy, typename T_type, typename T_mutex, typename T_islockable>
class lock_acquirer: noncopyable/*, nonheapallocatable*/, nonpointeraliasing
{
protected:
    typedef T_type acquired_type;
    typedef T_mutex mutex_type;
    // value_type = acquired_type with top-level reference stripped off
    typedef typename std::tr1::remove_reference<acquired_type>::type value_type;
    // const_or_value_type = unchanged value_type if policy is writelock, const value_type if readlock
    typedef typename boost::mpl::eval_if_c<
        I_policy == readlock, 
        std::tr1::add_const<value_type>, 
        boost::mpl::identity<value_type> 
    >::type const_or_value_type;
    typedef typename std::tr1::add_reference<typename std::tr1::add_volatile<value_type>::type>::type volatile_reference_type;
    typedef typename std::tr1::add_reference<typename std::tr1::remove_volatile<const_or_value_type>::type>::type reference_type;

    /**	@short Gives non-volatile access to the locked type
     *	
     *	Forces the programmer to pass a previously named lock_acquirer object thus
     *	ensuring that the lock is active throughout the usage of the locked object.
     */
    friend reference_type 
    access_acquiree(lock_acquirer& l) throw()
    {
        return l.access_acquiree();
    }


public:
    /**	@short Constructs a lock_acquirer from a volatile type to protect and a lock.
     *	@note Acquires the lock immediately, unlocks when it goes out of scope.
     *	@attention	We rely here on the fact that members are initialized according
     *				to the order in which they are declared in a class, such that
     *				the lock is acquired before _a_value is accessed non-volatile.
     */
    lock_acquirer(volatile_reference_type _a_value, mutex_type& _a_mutex): 
        m_lock(_a_mutex), 
        // volatile_cast
        m_acquiree(const_cast<reference_type>(_a_value))
    {}
    /**	@short	Constructs a lock_acquirer from a volatile type to protect, a lock and 
     *			an additional argument forwarded to the lock constructor.
     *	@note Acquires the lock immediately, unlocks when it goes out of scope
     */
    template<typename T_lockfwd_arg1>
    lock_acquirer(volatile_reference_type _a_value, mutex_type& _a_mutex, T_lockfwd_arg1 lockfwd_arg1): 
        m_lock(_a_mutex, lockfwd_arg1), 
        // volatile_cast
        m_acquiree(const_cast<reference_type>(_a_value))
    {}


protected:		
    /**	@return The locked type with the `volatile" qualifier removed
     */
    reference_type access_acquiree() throw()
    {
        return m_acquiree;
    }
        
        
protected:
    /**	@short lock manager appropriate for the lock type
     */
    typename choose_lock<mutex_type, I_policy>::type m_lock;

    /**	@short non-const reference to the locked object
     */
    reference_type m_acquiree;
};



template<typename T_type, typename T_mutex, typename T_islockable>
class writelock_acquirer: public lock_acquirer<writelock, T_type, T_mutex, T_islockable>
{
    typedef lock_acquirer<writelock, T_type, T_mutex, T_islockable> parent_type;
	typedef typename parent_type::mutex_type mutex_type;
	typedef typename parent_type::volatile_reference_type volatile_reference_type;

public:
    /**	@short Constructs a lock_acquirer from a volatile type to lock and a lock.
     *	@note Acquires the lock immediately, unlocks when it goes out of scope
     */
    writelock_acquirer(volatile_reference_type _a_value, mutex_type& _a_mutex): 
        parent_type(_a_value, _a_mutex)
    {}
    /**	@short	Constructs a lock_acquirer from a volatile type to protect, a lock and 
     *			an additional argument forwarded to the lock constructor.
     *	@note Acquires the lock immediately, unlocks when it goes out of scope
     */
    template<typename T_lockfwd_arg1>
    writelock_acquirer(volatile_reference_type _a_value, mutex_type& _a_mutex, T_lockfwd_arg1 lockfwd_arg1): 
        parent_type(_a_value, _a_mutex, lockfwd_arg1)
    {}
};

template<typename T_type, typename T_mutex, typename T_islockable>
class readlock_acquirer: public lock_acquirer<readlock, T_type, T_mutex, T_islockable>
{
    typedef lock_acquirer<readlock, T_type, T_mutex, T_islockable> parent_type;
	typedef typename parent_type::mutex_type mutex_type;
	typedef typename parent_type::volatile_reference_type volatile_reference_type;

public:
    /**	@short Constructs a lock_acquirer from a volatile type to lock and a lock.
     *	@note Acquires the lock immediately, unlocks when it goes out of scope
     */
    readlock_acquirer(volatile_reference_type _a_value, mutex_type& _a_mutex): 
        parent_type(_a_value, _a_mutex)
    {}
    /**	@short	Constructs a lock_acquirer from a volatile type to protect, a lock and 
     *			an additional argument forwarded to the lock constructor.
     *	@note Acquires the lock immediately, unlocks when it goes out of scope
     */
    template<typename T_lockfwd_arg1>
    readlock_acquirer(volatile_reference_type _a_value, mutex_type& _a_mutex, T_lockfwd_arg1 lockfwd_arg1): 
        parent_type(_a_value, _a_mutex, lockfwd_arg1)
    {}
};

/**	@short Specialization for a lockable_base derived object; locks the given 
 *	lockable object (e.g. a mutex_lockable) and ensures 
 *	threadsafe write access to the locked type.
 *	
 *	Collects acquisition of a mutex lock and a volatile_cast from the volatile 
 *	object contained in the lockable.
 *	A lock_acquirer object is initialized with a lockable object.
 *	During its lifetime, a lock_acquirer keeps the lock acquired. Also, lock_acquirer 
 *	offers write access to the volatile-stripped object. The access is offered 
 *	with a related access_acquiree() function. The volatile_cast is performed by access_acquiree(). 
 *	The cast is semantically valid because lock_acquirer keeps the lock acquired 
 *	for its lifetime.
 *	
 *	The following template arguments are used:
 *	- @e T_lockable A lockable_base derived type, e.g. a mutex_lockable<int>.
 *	
 *	The lock_acquirer chooses the appropriate lock manager for the lock automatically 
 *	by applying the choose_lock.
 *	
 *	@note The locked type can only be accessed with access_acquiree()
 *	@code
 *	// somewhere
 *	mutex_lockable<int> lockable_int;
 *	
 *	// a scope somewhere else
 *	{
 *		lock_acquirer<writelock, mutex_lockable<int> > l(lockable_int);
 *		int& i = access_acquiree(l);
 *		i = 42;
 *	}
 *	@endcode
 */
template<locking_policy I_policy, typename T_type, typename T_mutex>
class lock_acquirer<I_policy, T_type, T_mutex, std::tr1::true_type>: 
    // derive from lock_acquirer for the locked type (which is lockable::acquired_type);
    public lock_acquirer<
        I_policy, 
        // if the lockable is const ...
        typename boost::mpl::eval_if<
            std::tr1::is_const<T_type>, 
            // ... then transfer constness to the type to protect (constness for lockables and the locked type is transitive)
            std::tr1::add_const<typename T_type::acquired_type>, 
            // ... otherwise keep it as specified
            boost::mpl::identity<typename T_type::acquired_type>
        >::type, 
        T_mutex
        // let compiler deduce whether acquired_type is again a lockable
        /*, std::tr1::false_type*/
    >
{
    typedef lock_acquirer<
        I_policy, 
        typename boost::mpl::eval_if<
            std::tr1::is_const<T_type>, 
            std::tr1::add_const<typename T_type::acquired_type>, 
            boost::mpl::identity<typename T_type::acquired_type>
        >::type, 
        T_mutex
        /*, std::tr1::false_type*/
    > parent_type;
    typedef T_type lockable_type;


public:
    /**	@short Constructs a lock_acquirer from a lockable.
     *	@note Acquires the lock immediately, unlocks when it goes out of scope
     */
    explicit lock_acquirer(lockable_type& _a_lockable): 
        parent_type(_a_lockable.access_volatile(), _a_lockable.mutex())
    {}
    /**	@short	Constructs a lock_acquirer from a volatile type to protect, a lock and 
     *			an additional argument forwarded to the lock constructor.
     *	@note Acquires the lock immediately, unlocks when it goes out of scope
     */
    template<typename T_lockfwd_arg1>
    lock_acquirer(lockable_type& _a_lockable, T_lockfwd_arg1 lockfwd_arg1): 
        parent_type(_a_lockable.access_volatile(), _a_lockable.mutex(), lockfwd_arg1)
    {}
};


/**	@short	writelock_acquirer specialization for lockable's.
 */
template<typename T_type, typename T_mutex>
class writelock_acquirer<T_type, T_mutex, std::tr1::true_type>: public lock_acquirer<writelock, T_type, T_mutex, std::tr1::true_type>
{
    typedef lock_acquirer<writelock, T_type, T_mutex, std::tr1::true_type> parent_type;
    typedef T_type lockable_type;


public:
    /**	@short Constructs a lock_acquirer from a lockable.
     *	@note Acquires the lock immediately, unlocks when it goes out of scope
     */
    explicit writelock_acquirer(lockable_type& _a_lockable): 
        parent_type(_a_lockable)
    {}
    /**	@short	Constructs a lock_acquirer from a volatile type to protect, a lock and 
     *			an additional argument forwarded to the lock constructor.
     *	@note Acquires the lock immediately, unlocks when it goes out of scope
     */
    template<typename T_lockfwd_arg1>
    writelock_acquirer(lockable_type& _a_lockable, T_lockfwd_arg1 lockfwd_arg1): 
        parent_type(_a_lockable, lockfwd_arg1)
    {}
};


/**	@short	readlock_acquirer specialization for lockable's.
 */
template<typename T_type, typename T_mutex>
class readlock_acquirer<T_type, T_mutex, std::tr1::true_type>: public lock_acquirer<readlock, T_type, T_mutex, std::tr1::true_type>
{
    typedef lock_acquirer<readlock, T_type, T_mutex, std::tr1::true_type> parent_type;
    typedef T_type lockable_type;


public:
    /**	@short Constructs a lock_acquirer from a lockable.
     *	@note Acquires the lock immediately, unlocks when it goes out of scope
     */
    explicit readlock_acquirer(lockable_type& _a_lockable): 
        parent_type(_a_lockable)
    {}
    /**	@short	Constructs a lock_acquirer from a volatile type to protect, a lock and 
     *			an additional argument forwarded to the lock constructor.
     *	@note Acquires the lock immediately, unlocks when it goes out of scope
     */
    template<typename T_lockfwd_arg1>
    readlock_acquirer(lockable_type& _a_lockable, T_lockfwd_arg1 lockfwd_arg1): 
        parent_type(_a_lockable, lockfwd_arg1)
    {}
};


// @addtogroup threadsafety
/**	@}
 */

} // namespace mbox


#endif // end file guard
