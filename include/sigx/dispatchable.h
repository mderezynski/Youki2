#ifndef _SIGX_DISPATCHABLE_HPP_
#define _SIGX_DISPATCHABLE_HPP_

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

#include <utility> // std::pair
#include <sigc++/trackable.h> // need the trackable_callback_list
#include <sigxconfig.h>
#include <sigx/fwddecl.h>
#include <sigx/noncopyable.h>
#include <sigx/glib_lockables.h>


namespace sigx
{

	namespace internal
	{

typedef rw_lockable<dispatcher_ptr> rwlockable_dispatcher_ptr;


	} // namespace internal
	

	
/**	@short Derived classes designate their ability to dispatch messages
 *	over a sigx::dispatcher.
 *	
 *	This class holds just a thread safe pointer to a dispatcher.
 *	
 *	@ingroup Dispatching
*/
class SIGX_API dispatchable: noncopyable
{
	// must access members
	friend class shared_dispatchable;

protected:
	/**	
	 *	@throw	Might throw a std::bad_alloc exception
	 */
	dispatchable();
	// non-virtual by design
	~dispatchable() throw();
	
private:
	/**	@short	Private copy constructor, only accessible by shared_dispatchable.
	 */
	dispatchable(const dispatchable& other) throw();

	/**	@short	Private assignment operator, only accessible by shared_dispatchable.
	 */
	dispatchable& operator =(const dispatchable& other) throw();

	/**	@short	Release shared stuff.
	 */
	void release() throw();

protected:
	/**	@short	Invalidate those tunnels (and disconnect them from signals) that have 
	 *			registered themselves with add_dispatcher_change_notify_callback() 
	 *			when the validity tracking was activated.
	 */
	void invalidate_tunnels();

	/**	@short	non-volatile access to the dispatcher pointer in the current thread
	 */
	dispatcher_ptr dispatcher() const throw()
	{
		return m_disp_ptr->access_nonvolatile();
	}

public:
	typedef void (*func_dispatcher_change_notify)(void* /*handle to internal::validity_trackable*/);
	
	/**	Add a callback that is executed (notified) when the dispatcher is changed.
	 *	@param data Passed into func upon notification.
	 *	@param func Callback executed upon destruction of the object.
	 *	@attention You must not call dispatchable::remove_dispatcher_change_notify_callback() from 
	 *	within your callback!
	*/
	void add_dispatcher_change_notify_callback(void* data, func_dispatcher_change_notify func) const;
	
	/**	Remove a callback previously installed with add_dispatcher_change_notify_callback().
	 *	The callback is not executed.
	 *	@param data Parameter passed into previous call to add_dispatcher_change_notify_callback().
	*/
	void remove_dispatcher_change_notify_callback(void* data) const;


private:
	///	shared counter for m_disp_ptr and m_dispatcher_change_callback_list
	volatile int* m_shared_count;

	/**	@short A list of callbacks fired on dispatcher changes.
	 *	The callbacks are held in a list of type callback_list_type.
	 *	This list is allocated dynamically when the first callback is added.
	 */
	typedef std::list<std::pair<void*, func_dispatcher_change_notify> > callback_list_type;
	typedef callback_list_type* callback_list_ptr_type;
	callback_list_ptr_type* m_dispatcher_change_callback_list;

protected:
	internal::rwlockable_dispatcher_ptr* m_disp_ptr;
};


} // namespace sigx


#endif // end file guard
