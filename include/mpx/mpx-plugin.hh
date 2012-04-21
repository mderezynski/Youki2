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

#ifndef MPX_PLUGIN_HH
#define MPX_PLUGIN_HH

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

#include "mpx/mpx-services.hh"
#include "mpx/plugin-types.hh"

namespace MPX
{
    typedef std::map<guint, PluginHolderRefP_t> PluginHoldMap_t ; 

    class Traceback
	{
		std::string name;
		std::string method;
		std::string traceback;

		public:
			Traceback(const std::string& /*name*/, const std::string& /*method*/, const std::string& /*traceback*/);
			~Traceback();

			std::string
			get_name() const;

			std::string
			get_method() const;

			std::string
			get_traceback() const;
	};

    typedef sigc::signal<void, guint> SignalPlugin;
    typedef sigc::signal<void>         Signal;

    class PluginLoaderPython ;
    class PluginLoaderCPP ;

    class PluginManager
    : public Service::Base
    {
            SignalPlugin    signal_activated_ ;
            SignalPlugin    signal_deactivated_ ;
            SignalPlugin    signal_plugin_show_gui_ ;
            Signal          signal_traceback_ ;

		public:
	
			PluginManager ();
			~PluginManager ();

            SignalPlugin&	
            signal_plugin_show_gui()
            {
                return signal_plugin_show_gui_;
            }

            SignalPlugin&	
            signal_plugin_activated()
            {
                return signal_activated_;
            }

            SignalPlugin&	
            signal_plugin_deactivated()
            {
                return signal_deactivated_;
            }

            Signal&	
            signal_traceback()
            {
                return signal_traceback_;
            }

            void
            shutdown();

			const PluginHoldMap_t&
			get_map ()
            const;

			bool	
			activate(
                guint /*id*/
            );
	
			bool
			deactivate(
                guint /*id*/
            );

            void
            show(
                guint /*id*/
            );

            Gtk::Widget *
            get_gui(
                guint /*id*/
            );

	    void
	    push_traceback(
                  guint                /*id*/
                , const std::string&    /*method*/
                , const std::string&    /*error*/
            );

			unsigned int
			get_traceback_count()
            const;

			Traceback
			get_last_traceback()
            const;

			Traceback
			pull_last_traceback();

		private:

            void
            on_plugin_loaded(
                PluginHolderRefP_t
            );

            PluginLoaderPython  * m_PluginLoader_Python;
            PluginLoaderCPP     * m_PluginLoader_CPP;

	    PluginHoldMap_t	  m_Map ;	
	    guint	          m_Id ;
	    Glib::Mutex		  m_StateChangeLock ;
	    std::list<Traceback>  m_TracebackList ;
    };
}

#endif
