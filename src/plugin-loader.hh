//  MPX
//  Copyright (C) 2005-2007 MPX development.
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
//  --
//
//  The MPX project hereby grants permission for non-GPL compatible GStreamer
//  plugins to be used and distributed together with GStreamer and MPX. This
//  permission is above and beyond the permissions granted by the GPL license
//  MPX is covered by.

#ifndef MPX_PLUGIN_LOADER_HH
#define MPX_PLUGIN_LOADER_HH

#include <config.h>

#include <map>
#include <list>
#include <string>
#include <vector>
#include <set>

#include <glib/gtypes.h>
#include <glibmm/thread.h>
#include <gtkmm/widget.h>
#include <boost/shared_ptr.hpp>
#include <mcs/mcs.h>

#include "mpx/plugin-types.hh"

namespace MPX
{
    class PluginLoaderBase
    {
        public:
        
            typedef sigc::signal<void, PluginHolderRefP_t> SignalPluginLoaded;

            PluginLoaderBase ()
            {}

            virtual ~PluginLoaderBase ()
            {}

            virtual void
            load_plugins(
                guint& /* first available next id*/
            ) = 0;

            virtual SignalPluginLoaded&
            signal_plugin_loaded(
            )
            {
                return signal_plugin_loaded_;
            }

        protected:

            SignalPluginLoaded signal_plugin_loaded_;
    };
}

#endif
