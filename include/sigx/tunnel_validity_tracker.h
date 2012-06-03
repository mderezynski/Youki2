#ifndef _SIGX_TUNNEL_VALIDITY_TRACKER_HPP_
#define _SIGX_TUNNEL_VALIDITY_TRACKER_HPP_

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

#include <sigxconfig.h>
#include <sigx/fwddecl.h>


namespace sigx
{


/**	@short Interface for tracking the validity of a tunnel.
 *	
 */
class SIGX_API tunnel_validity_tracker
{
public:
	tunnel_validity_tracker(const shared_dispatchable& _A_disp);
	tunnel_validity_tracker(const tunnel_validity_tracker& other);
	tunnel_validity_tracker& operator =(const tunnel_validity_tracker& other);
	~tunnel_validity_tracker();


	/**	@short Adds a dependency to @p t.
	 *	
	 *	Increases the reference count of the validity info object.
	 *	
	 *	@param t The trackable object to add a callback to.
	 *	Must be const because functors might have stored const trackables.
	 */
	void do_bind_to_trackable(const sigc::trackable* t) const;

	void add_connection(const connection_wrapper& c);
	
	/**	@short Whether the callback is still valid (all trackables are still 
	 *	alive and dispatcher didn't change).
	 *	@return true = valid, false = invalid (callback must not be executed)
	 */
	bool is_valid() const;

	/**	@short	Activate the validity tracking.
	 *	@note	Called from tunnel_functor<>::activate_validity_tracking()
	 */
	void activate();



	/**	@short This function will be called by the destructor of sigc::trackableS.
	 *	
	 *	It invalidates the callback associated in a tunnel context.
	 *	If it was the last object referencing the validity object that notified 
	 *	then %notify() destroys the validity info object.
	 *	It also disconnects eventually the tunnel functor from a signal and 
	 *	notifies the dispatchable associated with the tunnel functor that the 
	 *	tunnel functor is not valid anymore.
	 *	@param data Handle to the validity info object.
	 */
	static void* notify_from_trackable(void* data);

	/**	@short This function will be called by the dispatchable whenever the dispatcher changes.
	 *	
	 *	It invalidates the callback associated in a tunnel context.
	 *	If it was the last object referencing the validity object that notified 
	 *	then %notify() destroys the validity info object.
	 *	It also disconnects eventually the tunnel functor from a signal and 
	 *	notifies the dispatchable associated with the tunnel functor that the 
	 *	tunnel functor is not valid anymore.
	 *	@param data Handle to the validity info object.
	 */
	static void notify_dispatcher_change(void* data);

private:
	static void clear_connections(internal::validity_trackable* t);
	static void cleanup(void* data, bool cleanup_dispatcher);	
	static void on_last_functor(tunnel_validity_tracker& data);


private:
	internal::validity_trackable* m_info;
};


} // namespace sigx


#endif // end file guard
