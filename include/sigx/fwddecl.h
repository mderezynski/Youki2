#ifndef _SIGX_FWDDECL_HPP_
#define _SIGX_FWDDECL_HPP_

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


namespace sigc
{

struct trackable;
struct connection;


} // namespace sigc


namespace Glib
{

template<class T_CppObject> class RefPtr;
class MainContext;
class SignalIdle;


} // namespace Glib


namespace sigx
{

	namespace internal
	{

class tunnel_validity_tracker;
struct validity_trackable;


	} // namespace internal


// fwd decl
class bad_caller;
class bad_sync_call;
class shared_dispatchable;
class signal_source_base;
class dispatcher;
class dispatchable;
class shared_dispatchable;
class tunnel_base;
class connection_wrapper;

// typedefs
typedef dispatcher* dispatcher_ptr;
typedef signal_source_base* signal_source_ptr;
typedef sigc::connection* sigc_connection_ptr;


} // namespace sigx


#include <sigx/lockable_fwddecl.h>


#endif // end file guard
