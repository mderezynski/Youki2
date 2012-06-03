#ifndef _SIGX_GLIB_DISPATCHER_HPP
#define _SIGX_GLIB_DISPATCHER_HPP

/*
 * Copyright 2005 Tim Mayberry and Klaus Triendl
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

#include <glibmm/dispatcher.h>
#include <glibmm/main.h>
#include <sigxconfig.h>
#include <sigx/fwddecl.h>
#include <sigx/dispatcher.h>


namespace sigx
{
    
/**	@short a dispatcher on top of Glib::dispatcher.
 *	
 *	This will work with gtkmm but only glibmm is required.
 *	@ingroup Dispatching
 */
class glib_dispatcher: public dispatcher
{
public:
	glib_dispatcher(const Glib::RefPtr<Glib::MainContext>& context = Glib::MainContext::get_default());

	/**	
	 *	@throw bad_caller
	 */
	~glib_dispatcher();
	
	// virtuals from dispatcher
	virtual void send(tunnel_context_base* tcb);

private:
	void do_work();

	///< the dispatcher doing the interthread communication
	Glib::Dispatcher m_disp;
};


} // namespace sigx


#endif // _SIGX_GLIB_DISPATCHER_HPP
