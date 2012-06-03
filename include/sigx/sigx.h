#ifndef _SIGX_HPP_
#define _SIGX_HPP_

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
// don't impose tr1 onto the programmer
//#include <sigx/fwddecl.h>
#include <sigx/internal_types.h>
#include <sigx/types.h>
#include <sigx/volatile_trait.h>
#include <sigx/noninstantiatable.h>
#include <sigx/noncopyable.h>
#include <sigx/nonassignable.h>
#include <sigx/nonheapallocatable.h>
#include <sigx/operator_new.h>
// don't impose boost onto the programmer
//#include <sigx/choose_lock.h>
//#include <sigx/lockable.h>
//#include <sigx/lock_acquirer.h>
//#include <sigx/glib_lockables.h>
#include <sigx/dispatcher.h>
#include <sigx/glib_dispatcher.h>
#include <sigx/tunnel_context_base.h>
#include <sigx/tunnel_context.h>
#include <sigx/tunnel_functor.h>
#include <sigx/dispatcher.h>
#include <sigx/connection_wrapper.h>
#include <sigx/signal_wrapper.h>
#include <sigx/dispatchable.h>
#include <sigx/auto_dispatchable.h>
#include <sigx/glib_auto_dispatchable.h>
#include <sigx/manual_dispatchable.h>
#include <sigx/shared_dispatchable.h>
//only used internally
//#include <sigx/signal_source_base.h>
//#include <sigx/signal_source_obj_mem.h>
//#include <sigx/signal_source_slot.h>
//#include <sigx/signal_source_threadprivate.h>
//#include <sigx/connection_handler.h>
//#include <sigx/auto_tunneler.h>
#include <sigx/request_f.h>
#include <sigx/signal_wrapper.h>
#include <sigx/signal_f.h>
#include <sigx/glib_threadable.h>
#include <sigx/bad_dispatcher.h>
#include <sigx/bad_sync_call.h>
#include <sigx/bad_caller.h>


#endif // end file guard
