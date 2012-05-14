//  MPX
//  Copyright (C) 2010 MPX development.
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
#include <boost/algorithm/string.hpp>

#include "mpx/mpx-main.hh" 
#include "mpx/util-string.hh"

#include "plugin-loader-cpp.hh"

using namespace Glib;
using boost::is_any_of;
using boost::split;

namespace
{
  inline bool
  is_module( const std::string& basename)
  {
    return MPX::Util::str_has_suffix_nocase(basename.c_str (), G_MODULE_SUFFIX);
  }
}

namespace MPX
{
    PluginLoaderCPP::PluginLoaderCPP ()
    {
    }
    
    PluginLoaderCPP::~PluginLoaderCPP ()
    {
        for( CPPPluginKeeper_t::const_iterator i = m_Plugins.begin(); i != m_Plugins.end() ; ++i )
        {
            (*i)->del_instance( (*i)->instance ) ;
        }
    }

    void
    PluginLoaderCPP::load_plugins(
        guint& next_id
    ) 
    {
        const std::string plugin_path = build_filename( PLUGIN_DIR, "cppmod" );

        if(! file_test( plugin_path, FILE_TEST_EXISTS ))
            return;

        Glib::Dir dir (plugin_path);
        StrV v (dir.begin(), dir.end());
        dir.close();

        for( StrV::const_iterator i = v.begin(); i != v.end(); ++i )
        {
                const std::string& path = *i;

                enum
                {
                    LIB_BASENAME,
                    LIB_PLUGNAME,
                    LIB_SUFFIX
                };

                const std::string type = "cppmod";

                std::string basename (path_get_basename (path));
                std::string pathname (path_get_dirname  (path));

                if (!is_module (basename))
                    continue;

                StrV subs;
                split (subs, basename, is_any_of ("-."));
                std::string name  = type + std::string("-") + subs[LIB_PLUGNAME];
                std::string mpath = Module::build_path(build_filename(PLUGIN_DIR, "cppmod"), name);

                Module module (mpath, ModuleFlags (0));
                if( !module )
                {
                    g_message("CPP Plugin load FAILURE '%s': %s", mpath.c_str (), module.get_last_error().c_str());
                    continue;
                }

                g_message("LOADING CPP Plugin: %s", mpath.c_str ());
                module.make_resident();

                CPPPluginLoaderRefP_t plugin = CPPPluginLoaderRefP_t( new CPPPluginLoader );

                m_Plugins.insert( plugin ) ;

                if( !g_module_symbol( module.gobj(), "get_instance", (gpointer*)(&plugin->get_instance) ))
                {
                    g_message("CPP Plugin load FAILURE '%s': get_instance hook missing", mpath.c_str ());
                    continue;
                }

                if( !g_module_symbol (module.gobj(), "del_instance", (gpointer*)(&plugin->del_instance) ))
                {
                    g_message("CPP Plugin load FAILURE '%s': del_instance hook missing", mpath.c_str ());
                    continue;
                }

                plugin->instance = plugin->get_instance(next_id++);

                mcs->key_register("plugins", plugin->instance->get_name(), false ) ; 

                signal_plugin_loaded_.emit( PluginHolderRefP_t( plugin->instance ));
        }
    }
}
