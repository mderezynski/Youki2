#ifndef _SIGX_THREADABLE_HPP_
#define _SIGX_THREADABLE_HPP_

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

/**	@defgroup Threading Threading
 *	@short A group of types helping to facilitate threads with glibmm and 
 *	interthread-communication
 */

#include <sigxconfig.h>
#include <sigx/manual_dispatchable.h>


namespace sigx
{

/**	@short	Derived classes denote that they are a thread wrapper.
 *	
 *	Additionally, since a threadable is a dispatchable, derived classes denote 
 *	their ability to participate automatically in threadsafe messaging.
 *	
 *	@ingroup Threading
 */
class SIGX_API threadable: public manual_dispatchable
{
protected:
	/**	@short Initialize thread specific stuff just before entering 
	 *	the thread's mainloop.
	 *	
	 *	This method gives derived classes the possibility to initialize
	 *	their things like thread private data or connecting to the
	 *	idle signal (via mainloop()->signal_idle() in case of a glib_threadable)
	 *	just before entering the mainloop.
	 *	
	 *	@note The sigx::dispatchable baseclass already has a valid dispatcher
	 *	and the thread's maincontext and mainloop are already valid, too.
	 */
	virtual void on_startup() {}
	
	/**	@short cleanup other stuff just after quitting the mainloop.
	 *	
	 *	This method gives derived classes the possibility to clean up
	 *	their things like thread private data right after quitting the mainloop.
	 *	
	 *	@note The sigx::dispatchable baseclass still has a valid dispatcher
	 *	reference and the thread's maincontext and mainloop are still valid, too.
	 */
	virtual void on_cleanup() {}
};


} // namespace sigx


#endif // end file guard
