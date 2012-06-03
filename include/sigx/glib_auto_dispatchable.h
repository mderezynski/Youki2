#ifndef _SIGX_GLIB_AUTO_DISPATCHABLE_HPP_
#define _SIGX_GLIB_AUTO_DISPATCHABLE_HPP_

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
#include <sigx/auto_dispatchable.h>


namespace sigx
{

/**	
 *	
 *	@ingroup Dispatching
 */
class SIGX_API glib_auto_dispatchable: public auto_dispatchable
{
public:
	glib_auto_dispatchable();
	glib_auto_dispatchable(const Glib::RefPtr<Glib::MainContext>& context);
};


} // namespace sigx


#endif // end file guard
