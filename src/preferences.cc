//  MPX
//  Copyright (C) 2010 MPX Development
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif 

#include <utility>
#include <iostream>

#include <glibmm.h>
#include <glib/gi18n.h>
#include <gtk/gtkstock.h>
#include <gtkmm.h>
#include <libglademm.h>

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#ifdef HAVE_ALSA
#  define ALSA_PCM_NEW_HW_PARAMS_API
#  define ALSA_PCM_NEW_SW_PARAMS_API

#  include <alsa/global.h>
#  include <alsa/asoundlib.h>
#  include <alsa/pcm_plugin.h>
#  include <alsa/control.h>
#endif

#include "mpx/mpx-main.hh"
#include "mpx/mpx-stock.hh"
#include "mpx/util-string.hh"
#include "mpx/widgets/widgetloader.hh"

#include "mpx/i-youki-play.hh"

#include "library.hh"
#include "preferences.hh"

using namespace Glib ;
using namespace Gtk ;

namespace MPX
{
    //// Preferences
    Preferences*
        Preferences::create ()
    {
        return new Preferences (Gnome::Glade::Xml::create (build_filename(DATA_DIR, "glade" G_DIR_SEPARATOR_S "preferences.glade")));
    }

    Preferences::~Preferences ()
    {
        hide();
        mcs->key_set<int>("mpx","preferences-notebook-page", m_notebook_preferences->get_current_page());
    }

    bool
    Preferences::on_delete_event(GdkEventAny* G_GNUC_UNUSED)
    {
        hide();
        return true;
    }

    void
    Preferences::present ()
    {
        resize(
            mcs->key_get<int>("mpx","window-prefs-w"),
            mcs->key_get<int>("mpx","window-prefs-h")
            );

        move(
            mcs->key_get<int>("mpx","window-prefs-x"),
            mcs->key_get<int>("mpx","window-prefs-y")
            );

        Gtk::Window::show ();
        Gtk::Window::raise ();
    }

    void
    Preferences::hide ()
    {
        Gtk::Window::get_position( Mcs::Key::adaptor<int>(mcs->key("mpx", "window-prefs-x")), Mcs::Key::adaptor<int>(mcs->key("mpx", "window-prefs-y")));
        Gtk::Window::get_size( Mcs::Key::adaptor<int>(mcs->key("mpx", "window-prefs-w")), Mcs::Key::adaptor<int>(mcs->key("mpx", "window-prefs-h")));
        Gtk::Widget::hide();
    }

    void
    Preferences::add_page(
          Gtk::Widget*          widget
        , const std::string&    name
    )
    {
        g_object_ref(G_OBJECT(widget->gobj())) ;
        widget->unparent() ;
        m_notebook_preferences->append_page(
              *widget
            , name
        ) ;
        g_object_unref(G_OBJECT(widget->gobj())) ;
    }

    Preferences::Preferences(
        const Glib::RefPtr<Gnome::Glade::Xml>&  xml
    )
        : Gnome::Glade::WidgetLoader<Gtk::Window>(xml, "preferences")
        , MPX::Service::Base("mpx-service-preferences")
    {
        dynamic_cast<Button*>(m_Xml->get_widget ("close"))->signal_clicked().connect(
            sigc::mem_fun(
                *this
              , &Preferences::hide
        ));

        mcs->key_register("mpx","preferences-notebook-page", 0);

        m_Xml->get_widget ("notebook", m_notebook_preferences);
        m_notebook_preferences->set_current_page( mcs->key_get<int>("mpx","preferences-notebook-page") );
    }
}  // namespace MPX
