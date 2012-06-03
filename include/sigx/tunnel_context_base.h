#ifndef _SIGX_TUNNEL_CONTEXT_BASE_HPP_
#define _SIGX_TUNNEL_CONTEXT_BASE_HPP_

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

#include <glibmm/thread.h>
#include <sigx/fwddecl.h>
#include <sigx/bad_dispatcher.h>
#include <sigx/operator_new.h>
#include <sigx/shared_dispatchable.h>
#include <sigx/tunnel_validity_tracker.h>


namespace sigx
{

/**	@short the base class for all tunnel_context classes.
 *	
 *	A tunnel context represents the tunnel and all facilities involved:
 *	the dispatcher, the message, the functor to invoke, whether the message
 *	is dispatched asynchronously or synchronously.
 *	
 *	@ingroup Dispatching
 */
class SIGX_API tunnel_context_base: public operator_new
{
public:
	tunnel_context_base(const shared_dispatchable& _A_disp, const tunnel_validity_tracker& _A_validity_tracker, bool sync = false);
	virtual ~tunnel_context_base();

	/**	@short invokes the intended functor at the other end of the tunnel
	 */
	virtual void invoke() = 0;
		
	bool is_sync() const	{	return m_sync;	}
	bool is_valid() const	{	return m_validity_tracker.is_valid();	}
	const void* creator_thread() const	{	return m_creator_thread;	}
		
protected:
	/**	@short sends the tunnel context over the dispatcher.
	 *	@throw bad_dispatcher If dispatcher is invalid.
	 */
	void dispatch_me();
	
protected:
	tunnel_validity_tracker m_validity_tracker;
	shared_dispatchable m_disp;
	bool m_sync;
	const void* const m_creator_thread;
};


/**	@short Specialities for synchronous tunnel context.
 *	@ingroup Dispatching
 */
class SIGX_API sync_tunnel_context_base: public tunnel_context_base
{
public:
	sync_tunnel_context_base(const shared_dispatchable& _A_disp, const tunnel_validity_tracker& _A_validity_tracker);


protected:
	/**	@short sends the tunnel context over the dispatcher.
	 *	
	 *	Locks until invoke() has completed.
	 *	
	 *	@return whether the tunnel context is dispatchable, i.e. the dispatcher
	 *	reference was still valid.
	 *	@throw bad_dispatcher If dispatcher is invalid.
	 */
	void dispatch_me();


protected:
	Glib::Cond m_cond;
	Glib::Mutex m_mutex;
};


} // namespace sigx


#endif // end file guard
