#ifndef _SIGX_VALIDITY_TRACKABLE_HPP_
#define _SIGX_VALIDITY_TRACKABLE_HPP_

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

#include <list>
#include <vector>
#include <glib/gtypes.h> // gint
#include <sigx/fwddecl.h>
#include <sigx/operator_new.h>
#include <sigx/shared_dispatchable.h>
#include <sigx/connection_wrapper.h>


namespace sigx
{

	namespace internal
	{

/**	@short	Lynchpin to track the validity of a tunnel functor and storing
 *			information about who needs to be notified about dying dispatchers 
 *			and tunnel functors.
 */
struct validity_trackable: public operator_new
{
	validity_trackable(const shared_dispatchable& _A_disp);


	gint m_refcount;
	// track the reference count from tunnel functors
	// (functionality wrapped in tunnel_validity_tracker)
	// in a separate variable to be able to track those
	// validity_trackables that don't have a purpose anymore
	gint m_tunnel_refcount;
	bool m_valid;
	/// A connection_wrapper to a signal
	std::list<connection_wrapper> m_connections;
	///	The dispatchable guarding the tunnel callback.
	///	It has us registered
	shared_dispatchable m_disp;
	std::vector<const sigc::trackable*> m_trackables;
	/// this variable holds the address of the dispatcher
	/// with which the tunnel functor was originally created;
	/// this allows to track a change of the dispatcher;
	/// a change might happen if a thread ends and the threadable 
	/// resets its dispatcher, 
	/// but the threadable starts again and sets up another dispatcher.
	/// This situation is fine for request functors because they
	/// only need a valid dispatcher; however, this situation is a problem
	/// for tunnel functors connected in a server thread because for them
	/// resetting the dispatcher means that they will get disconnected
	/// from the server thread's signal.
	//const dispatcher_ptr m_original_dispatcher;
	gint m_dispatcher_change_is_cleanup;
	void* m_creator_thread;
};


	} // namespace internal

} // namespace sigx


#endif // _SIGX_VALIDITY_TRACKABLE_HPP_
