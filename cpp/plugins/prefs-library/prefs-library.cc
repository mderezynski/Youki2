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

#include <utility>
#include <iostream>

#include <glibmm.h>
#include <glib/gi18n.h>
#include <gtkmm.h>

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#include "mpx/mpx-main.hh"
#include "mpx/widgets/widgetloader.hh"

#include "mpx/i-youki-library.hh"
#include "mpx/i-youki-preferences.hh"

#include "prefs-library.hh"

using namespace Glib;
using namespace Gtk;

namespace MPX
{
    namespace
    {
        const char *ui_path = DATA_DIR G_DIR_SEPARATOR_S "ui" G_DIR_SEPARATOR_S "cppmod-prefs-library.ui";
    }

    PrefsLibrary*
        PrefsLibrary::create(gint64 id)
    {
        return new PrefsLibrary (Gtk::Builder::create_from_file (ui_path), id);
    }

    PrefsLibrary::~PrefsLibrary ()
    {
    }

    PrefsLibrary::PrefsLibrary(
          const Glib::RefPtr<Gtk::Builder>& builder
        , gint64                            id
    )
        : WidgetLoader<Gtk::VBox>(builder, "cppmod-prefs-library")
        , PluginHolderBase()
    {
        show() ;

        m_Name = "PreferencesModule LIBRARY" ;
        m_Description = "This plugin provides library preferences" ;
        m_Authors = "M. Derezynski" ;
        m_Copyright = "(C) 2009 MPX Project" ;
        m_IAge = 0 ;
        m_Website = "http://redmine.sivashs.org/projects/mpx" ;
        m_Active = false ;
        m_HasGUI = false ;
        m_CanActivate = false ;
        m_Hidden = true ;
        m_Id = id ;

        boost::shared_ptr<IPreferences> p = services->get<IPreferences>("mpx-service-preferences") ;

        p->add_page(
              this
            , _("Music Library")
        ) ;

        //// LIBRARY
        Gtk::FileChooser* file_chooser = 0;

        m_Builder->get_widget( "fc-music-import-path", file_chooser );
        mcs_bind->bind_filechooser(*file_chooser, "mpx", "music-import-path");

        m_Builder->get_widget( "fc-quarantine-path", file_chooser );
        mcs_bind->bind_filechooser(*file_chooser, "mpx", "music-quarantine-path");

        Glib::RefPtr<Gio::File> quarantine_path = Gio::File::create_for_path( mcs->key_get<std::string>("mpx","music-quarantine-path")) ;

        try{
            quarantine_path->make_directory() ;
        } catch( Glib::Error & cxe )
        {
            g_message( "Error creating quarantine path: %s", cxe.what().c_str() ) ;
        }


        m_Builder->get_widget( "rescan-at-startup", m_Library_RescanAtStartup ) ;
        m_Builder->get_widget( "rescan-in-intervals", m_Library_RescanInIntervals) ;
        m_Builder->get_widget( "rescan-interval", m_Library_RescanInterval ) ;
        m_Builder->get_widget( "quarantine-files", m_Library_QuarantineInvalid ) ;

        m_Builder->get_widget( "hbox-rescan-interval", m_Library_RescanIntervalBox ) ;
        m_Builder->get_widget( "hbox-quarantine", m_Library_QuarantineBox ) ;

        mcs_bind->bind_spin_button(
            *m_Library_RescanInterval
            , "library"
            , "rescan-interval"
            );

        mcs_bind->bind_toggle_button(
            *m_Library_RescanAtStartup
            , "library"
            , "rescan-at-startup"
            );

        mcs_bind->bind_toggle_button(
            *m_Library_RescanInIntervals
            , "library"
            , "rescan-in-intervals"
            );

        mcs_bind->bind_toggle_button(
            *m_Library_QuarantineInvalid
            , "library"
            , "quarantine-invalid"
            );

        m_Library_RescanInIntervals->signal_toggled().connect(
            sigc::compose(
            sigc::mem_fun(*m_Library_RescanIntervalBox, &Gtk::Widget::set_sensitive),
            sigc::mem_fun(*m_Library_RescanInIntervals, &Gtk::ToggleButton::get_active)
            ));

        m_Library_QuarantineInvalid->signal_toggled().connect(
            sigc::compose(
            sigc::mem_fun(*m_Library_QuarantineBox, &Gtk::Widget::set_sensitive),
            sigc::mem_fun(*m_Library_QuarantineInvalid, &Gtk::ToggleButton::get_active)
            ));
    }
}  // namespace MPX
