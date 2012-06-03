#ifndef _SIGX_AUTO_DISPATCHABLE_HPP_
#define _SIGX_AUTO_DISPATCHABLE_HPP_

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

#include <sigxconfig.h>
#include <sigx/dispatchable.h>


namespace sigx
{

/**	@short A dispatchable managing the pointer to the dispatcher.
 *	
 *	the dispatcher reference is initialized with a dispatcher automatically in the ctor,
 *	thus, an auto-dispatchable is especially useful for classes running in the
 *	default glib main context. if you need manual control over which dispatcher your
 *	derived class should use (like for threads), then use the manual_dispatchable
 *	@note non-copyable. If you want to pass around the dispatchable then use
 *	a sigx::shared_dispatchable instead
 *	
 *	@ingroup Dispatching
 */
class SIGX_API auto_dispatchable: public dispatchable
{
protected:
	auto_dispatchable(dispatcher_ptr disp);
	
	~auto_dispatchable();
};


} // namespace sigx


#endif // end file guard
