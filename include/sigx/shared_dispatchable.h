#ifndef _SIGX_SHARED_DISPATCHABLE_HPP_
#define _SIGX_SHARED_DISPATCHABLE_HPP_

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

#include <sigc++/reference_wrapper.h>
#include <sigxconfig.h>
#include <sigx/fwddecl.h>
#include <sigx/dispatchable.h>


namespace sigx
{
	
/**	@short thread safe dispatcher reference that can be passed around.
 *	
 *	A shared dispatchable holds either a copy of another dispatchable
 *	or 
 *	the dispatcher itself is only accessible by constructing a 
 *	shared_dispatchable::DispatcherPtr.
 *	
 *	@note Multiple reads of the same object are threadsafe, multiple writes not:
 *	@code
 *	// threadsafe
 *	shared_dispatchable::DispatcherPtr dispatcher(shareddisp);
 *	
 *	// not threadsafe
 *	shareddisp = mydisp;
 *	@endcode
 *	
 *	@ingroup Dispatching
 */
class SIGX_API shared_dispatchable: public dispatchable
{
	friend struct DispatcherPtr;
	// must get the dispatcher ptr over dispatcher() for assertions
	friend class tunnel_validity_tracker;


public:
	///**	@short behaves like a dispatcher pointer, ensures threadsafe access to the dispatcher reference.
	// *	
	// *	A DispatcherPtr is constructed from a dispatchable and references the 
	// *	dispatcher of the dispatchable. A read lock ensures threadsafe access to
	// *	the reference (the owner thread sets a read/write lock when it sets
	// *	the reference to 0). 
	// *	After you have constructed a DispatcherPtr, you *must* test for null.
	// */
	//class DispatcherPtr
	//{
	//public:
	//	explicit DispatcherPtr(shared_dispatchable& _A_disp): 
	//		m_locker(*_A_disp.m_disp_ptr)
	//	{}
	//	
	//	/**	@short Returns a plain dispatcher*
	//	 */
	//	dispatcher_ptr operator ->()
	//		{	return access_acquiree(m_locker); }
	//	operator bool()
	//		{	return access_acquiree(m_locker); }
	//	bool operator !()
	//		{	return !access_acquiree(m_locker); }
	//	bool operator !=(dispatcher_ptr other)
	//		{	return other != access_acquiree(m_locker); }
	//	
	//private:
	//	readlock_acquirer<internal::rwlockable_dispatcher_ptr> m_locker;
	//};
	class DispatcherPtr;


public:
	/**	
	 *	@throw	Might throw a std::bad_alloc exception (from dispatchable's ctor)
	 */
	shared_dispatchable();

	/**	@short	Copy construct from any dispatchable.
	 */
	shared_dispatchable(const dispatchable& d) throw();
	/**	@short	Assign from any dispatchable.
	 */
	shared_dispatchable& operator =(const dispatchable& d) throw();
};


} // namespace sigx


#endif // end file guard
