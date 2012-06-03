#ifndef _SIGX_TYPES_HPP_
#define _SIGX_TYPES_HPP_

/*
 * Copyright 2005 Klaus Triendl
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free 
 * Software Foundation, 51 Franklin Street, Fifth Floor, 
 * Boston, MA 02110-1301, USA.
 */

namespace sigx
{

/**	@short specifies the synchronization mode of the tunnel, i.e.
 *	whether the message should be sent asynchronous or synchronous
 *	@note asynchronous tunnels are the default
 *	@code
 *	open_tunnel<ASYNC>(&callback)();
 *	// is the same as:
 *	open_tunnel(&callback)();
 *	@endcode
 *	@note other types are thinkable like a SYNC_TIMED
 */
enum sync_type
{
	ASYNC, 
	SYNC
};


}


#endif // end file guard
