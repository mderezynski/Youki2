#ifndef _SIGX_THREADMANAGED_DISPATCHABLE_HPP_
#define _SIGX_THREADMANAGED_DISPATCHABLE_HPP_

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
#include <sigx/dispatchable.h>


namespace sigx
{

/**	@short A dispatchable whose dispatcher pointer is managed by derived classes.
 *	
 *	The dispatcher must be set manually with set_dispatcher()
 *	in the context of the running thread, the best point is after creating the 
 *	main context for the thread.
 *	
 *	@note non-copyable. If you want to pass around the dispatchable then use
 *	a sigx::shared_dispatchable instead
 */
class SIGX_API manual_dispatchable: public dispatchable
{
public:
	manual_dispatchable();
	
	virtual ~manual_dispatchable();

	
protected:
	void set_dispatcher(dispatcher_ptr disp);
};


} // namespace sigx


#endif // file guard
