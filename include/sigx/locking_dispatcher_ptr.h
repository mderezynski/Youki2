#ifndef _SIGX_LOCKING_DISPATCHER_PTR_H_
#define _SIGX_LOCKING_DISPATCHER_PTR_H_

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

#include <sigx/fwddecl.h>
#include <sigx/shared_dispatchable.h>
#include <sigx/lock_acquirer.h>


namespace sigx
{

/**	@short behaves like a dispatcher pointer, ensures threadsafe access to the dispatcher reference.
 *	
 *	A DispatcherPtr is constructed from a dispatchable and references the 
 *	dispatcher of the dispatchable. A read lock ensures threadsafe access to
 *	the reference (the owner thread sets a read/write lock when it sets
 *	the reference to 0). 
 *	After you have constructed a DispatcherPtr, you *must* test for null.
 */
class shared_dispatchable::DispatcherPtr
{
public:
	explicit DispatcherPtr(shared_dispatchable& _A_disp): 
		m_locker(*_A_disp.m_disp_ptr)
	{}
	
	/**	@short Returns a plain dispatcher*
	 */
	dispatcher_ptr operator ->()
		{	return access_acquiree(m_locker); }
	operator bool()
		{	return access_acquiree(m_locker); }
	bool operator !()
		{	return !access_acquiree(m_locker); }
	bool operator !=(dispatcher_ptr other)
		{	return other != access_acquiree(m_locker); }
	
private:
	readlock_acquirer<internal::rwlockable_dispatcher_ptr> m_locker;
};


}	// namespace sigx


#endif	// file guard
