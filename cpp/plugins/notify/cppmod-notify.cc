//  MPX
//  Copyright (C) 2003-2007 MPX Development
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
//  The MPX project hereby grants permission for non GPL-compatible GStreamer
//  plugins to be used and distributed together with GStreamer and MPX. This
//  permission is above and beyond the permissions granted by the GPL license
//  MPX is covered by.

#include "config.h"

#include <glibmm.h>
#include <glibmm/i18n.h>
#include <gtkmm.h>
#include <boost/format.hpp>

#include "mpx/mpx-plugin.hh"
#include "mpx/i-youki-controller.hh"

#include "cppmod-notify.hh"

#include <libnotify/notify.h>
#include <libnotify/notification.h>

#include "mpx/mpx-covers.hh"

namespace MPX
{
    CPPModNotify*
        CPPModNotify::create(guint id)
    {
        return new CPPModNotify(id);
    }

    CPPModNotify::~CPPModNotify ()
    {
    }

    CPPModNotify::CPPModNotify(
          guint id
    )
        : PluginHolderBase()
    {
        m_Name = "Desktop Notification Plugin" ;
        m_Description = "" ;
        m_Authors = "M. Derezynski" ;
        m_Copyright = "(C) 2012 MPX Project" ;
        m_IAge = 0 ;
        m_Website = "" ;
        m_Active = false ;
        m_HasGUI = false ;
        m_CanActivate = true ;
        m_Hidden = false ;
        m_Id = id ;


        boost::shared_ptr<IYoukiController> p1 = services->get<IYoukiController>("mpx-service-controller") ;

        GObject * obj = G_OBJECT(p1->gobj()) ;

        g_signal_connect(
              obj
            , "track-new"
            , GCallback(on_controller_track_new)
            , this 
        ) ;

	if(!notify_is_initted())
	{
	    notify_init("Youki") ;
	}
    }
}

using namespace MPX ;

Gtk::Widget*
CPPModNotify::get_gui()
{
    return nullptr ;
}

bool
CPPModNotify::activate()
{
    m_Active = true ;
    return true ;
}

bool
CPPModNotify::deactivate()
{
    m_Active = false ;
    return true ;
}

bool
CPPModNotify::on_display_notification_idle(
)
{
    boost::shared_ptr<IYoukiController> p = services->get<IYoukiController>("mpx-service-controller") ;

    try{
	MPX::Track & t = p->get_metadata() ; 

	const std::string&  artist  =   boost::get<std::string>(t[ATTRIBUTE_ARTIST].get()) ;
	const std::string&  title   =   boost::get<std::string>(t[ATTRIBUTE_TITLE].get()) ;
	const std::string&  mbid    =   boost::get<std::string>(t[ATTRIBUTE_MB_ALBUM_ID].get()) ;

	boost::shared_ptr<Covers> p = services->get<Covers>("mpx-service-covers") ;

	Glib::RefPtr<Gdk::Pixbuf> pb ;

	p->fetch( mbid, pb, 128 ) ;

	NotifyNotification * n = notify_notification_new(
	      "Youki Now Playing"
	    , (boost::format("%s - %s") % artist % title).str().c_str()
	    , NULL
	) ;

	if(pb)
	{
	    notify_notification_set_image_from_pixbuf(n, GDK_PIXBUF(pb->gobj())) ;
	}

	GError* err = nullptr ;

	notify_notification_show(n, &err) ;	

    } catch( std::runtime_error )
    {
    }

    return false ;
}

void
CPPModNotify::on_controller_track_new(
      GObject *     controller
    , gpointer      data
)
{
    CPPModNotify & obj = *reinterpret_cast<CPPModNotify*>(data) ;

    if( !obj.m_Active )
    {
	return ;
    }

    Glib::signal_idle().connect( sigc::mem_fun( obj, &CPPModNotify::on_display_notification_idle )) ;
}
