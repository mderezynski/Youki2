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

#include "config.h"

#include <glibmm/i18n.h>

#include "mpx/mpx-plugin.hh"
#include "mpx/mpx-main.hh"

#include "plugin-types-python.hh"
#include "plugin-types-cpp.hh"
#include "plugin-loader-python.hh"
#include "plugin-loader-cpp.hh"

using namespace Glib;

namespace MPX
{
	Traceback::Traceback(const std::string& n, const std::string& m, const std::string& t)
	:	name (n)
	,	method (m)
	,	traceback (t)
	{
	}

	Traceback::~Traceback()
	{
	}

	std::string
	Traceback::get_name() const
	{
		return name;
	}

	std::string
	Traceback::get_method() const
	{
		return method;
	}

	std::string
	Traceback::get_traceback() const
	{
		return traceback;
	}

	PluginManager::PluginManager ()
    : Service::Base("mpx-service-plugins")
	, m_Id(1)
	{
        mcs->domain_register("plugins");

        m_PluginLoader_Python = new PluginLoaderPython;
        m_PluginLoader_Python->signal_plugin_loaded().connect(
            sigc::mem_fun(
                *this,
                &PluginManager::on_plugin_loaded
        ));

        m_PluginLoader_CPP = new PluginLoaderCPP;
        m_PluginLoader_CPP->signal_plugin_loaded().connect(
            sigc::mem_fun(
                *this,
                &PluginManager::on_plugin_loaded
        ));

        m_PluginLoader_Python->load_plugins( m_Id ); 
        m_PluginLoader_CPP->load_plugins( m_Id ); 
	}

    void
    PluginManager::on_plugin_loaded(
        PluginHolderRefP_t ptr
    )
    {
        m_Map.insert( std::make_pair( ptr->m_Id, ptr ));

        bool active = mcs->key_get<bool>("plugins", ptr->get_name());

        g_message("Name: [%s], Active: %d", ptr->get_name().c_str(), int(active));

        if( active )
        {
            ptr->m_Active = true;
            try{
                ptr->activate();
            } catch( MethodInvocationError )
            {
                g_warning("%s: Could not activate plugin: %s", G_STRLOC, ptr->get_name().c_str()) ;
            }
        }
    }

    void
    PluginManager::shutdown ()
    {
        for( PluginHoldMap_t::iterator i = m_Map.begin(); i != m_Map.end(); ++i )
        {
                if(!i->second->m_Active)
                {
                    continue;
                }

                i->second->deactivate();
        }
    }

	PluginManager::~PluginManager ()
	{
        shutdown();

        delete m_PluginLoader_Python ;
        delete m_PluginLoader_CPP ;
	}

	const PluginHoldMap_t&
	PluginManager::get_map () const
	{
		return m_Map;
	}

	Gtk::Widget *
	PluginManager::get_gui(guint id)
	{
		Glib::Mutex::Lock L (m_StateChangeLock);

		PluginHoldMap_t::iterator i = m_Map.find(id);
		g_return_val_if_fail(i != m_Map.end(), false);
        g_return_val_if_fail(m_Map.find(id)->second->get_has_gui(), false);

        Gtk::Widget * gui = 0;

        try{
            gui = i->second->get_gui();

        } catch( MethodInvocationError & cxe )
        {
			push_traceback( id, "get_gui", cxe.what() );

            return 0;
        }

		return gui; 
	}

	bool
	PluginManager::activate(guint id)
	{
		Glib::Mutex::Lock L (m_StateChangeLock);

		PluginHoldMap_t::iterator i = m_Map.find(id);
		g_return_val_if_fail(i != m_Map.end(), false);

		if(i->second->m_Active)
		{
			g_message("%s: Requested activate from plugin %u, but is active.", G_STRLOC, id);	
			g_return_val_if_reached(false);
		}

        bool success = false;

        try{
                success = i->second->activate();

                if ( success ) 
                {
                    i->second->m_Active = true;
                    signal_activated_.emit(id);
                }

        } catch( MethodInvocationError & cxe )
        {
			push_traceback (id, "activate", cxe.what() );

            return false;
        }

        try{
            mcs->key_set<bool>("plugins", i->second->get_name(), i->second->m_Active);
        } catch(...) {
            g_message("%s: Failed saving plugin state for '%s'", G_STRLOC, i->second->get_name().c_str());
        }

		return success;
	}

    bool
	PluginManager::deactivate(guint id)
	{
		Glib::Mutex::Lock L (m_StateChangeLock);

		PluginHoldMap_t::iterator i = m_Map.find(id);
		g_return_val_if_fail(i != m_Map.end(), false);

		if(!i->second->m_Active)
		{
			g_message("%s: Deactivate requested for plugin %u, but is already deactivated.", G_STRLOC, id);	
			g_return_val_if_reached(false);
		}
        
        bool success = false;

        try{
                success = i->second->deactivate();

                if ( success ) 
                {
                    i->second->m_Active = false;
                    signal_deactivated_.emit(id);
                }

        } catch( MethodInvocationError & cxe )
        {
			push_traceback( id, "deactivate", cxe.what() );

            return false;
        }

        try{
            mcs->key_set<bool>("plugins", i->second->get_name(), i->second->m_Active);
        } catch(...) {
            g_message("%s: Failed saving plugin state for '%s'", G_STRLOC, i->second->get_name().c_str());
        }

        return success;
	}

    void
	PluginManager::show(guint id)
	{
        signal_plugin_show_gui_.emit(id);
	}

	void
	PluginManager::push_traceback(
          guint                id
        , const std::string&    method
        , const std::string&    traceback
    )
	{
        g_return_if_fail(m_Map.count(id) != 0);
        m_TracebackList.push_front( Traceback( m_Map.find(id)->second->get_name(), method, traceback ));
        signal_traceback_.emit();
    }

	unsigned int
	PluginManager::get_traceback_count() const
	{
		return m_TracebackList.size();
	}

	Traceback
	PluginManager::get_last_traceback() const
	{
		return *(m_TracebackList.begin());
	}

	Traceback
	PluginManager::pull_last_traceback()
	{
		std::list<Traceback>::iterator i = m_TracebackList.begin();
		Traceback t = *i;
		m_TracebackList.pop_front();
		return t;
	}
}
