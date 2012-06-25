/* Copyright (C) 2005 Tim Mayberry
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

#include <sigc++/functors/mem_fun.h>
#include <glib.h> // g_return_if_fail()
#include "sigx/glib_dispatcher.h"
#include "__sigx_pchfence__.h"


namespace sigx
{
    
glib_dispatcher::glib_dispatcher(const Glib::RefPtr<Glib::MainContext>& context): 
	dispatcher(), 
	m_disp(context)
{
	m_disp.connect(sigc::mem_fun(this, &glib_dispatcher::do_work));
}

glib_dispatcher::~glib_dispatcher()
{
	// must be executed in the context of the owner thread, i.e. the thread that
	// created this glib_dispatcher instance;
	// note that the Glib::dispatcher requires this, too;
	test_calling_thread();

	// flush queue.
	// note that this calls the message handlers!
	while (process_next()) /*++clouds*/;
}

void glib_dispatcher::send(tunnel_context_base* context)
{
	dispatcher::send(context);
	m_disp.emit();
}

void glib_dispatcher::do_work()
{
	// 2006-08-12, kj
	// do not check anymore whether m_exiting==true because do_work() and the 
	// dtor are executed in the context of the same thread anyway
	while (process_next()) /*++clouds*/;
}


} // namespace sigx
