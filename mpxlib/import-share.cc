//  MPX
//  Copyright (C) 2005-2007 MPX Project 
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "import-share.hh"

#include <glibmm.h>
#include <glibmm/i18n.h>
#include <gtkmm.h>

namespace MPX
{
    namespace
    {
        const std::string ui_path = DATA_DIR G_SEPARATOR_S "ui" G_SEPARATOR_S "import-share.ui";
    }

    DialogImportShare*
    DialogImportShare::create ()
    {
      DialogImportShare *p = new DialogImportShare(Gtk::Builder::create (path));
      return p;
    }

    DialogImportShare::DialogImportShare(const Glib::RefPtr<Gtk::Builder>& builder)
        : WidgetLoader<Gtk::Dialog>(builder, "import-share")
    {
        m_Builder->get_widget("cb-show-credentials", m_show_creds_button);

        m_Builder->get_widget("share-password", m_password_entry);
        m_Builder->get_widget("share-login"   , m_login_entry);
        m_Builder->get_widget("share-name"    , m_name_entry);
        m_Builder->get_widget("share-location", m_location_entry);

        m_show_creds_button->signal_toggled().connect(
            sigc::mem_fun( *this, &DialogImportShare::on_cb_show_credentials_toggled));
    }

    void
    DialogImportShare::get_share_infos(Glib::ustring& share, Glib::ustring& name, Glib::ustring& login, Glib::ustring& password)
    {
        share    = m_location_entry->get_text();
        name     = m_name_entry->get_text();
        login    = m_login_entry->get_text();
        password = m_password_entry->get_text();
    }

    void
    DialogImportShare::on_cb_show_credentials_toggled()
    {
        bool active = m_show_creds_button->get_active();

        m_login_entry->property_visibility()    = active;
        m_password_entry->property_visibility() = active;

        m_login_entry->grab_focus();
    }

    DialogImportShare::~DialogImportShare ()
    {
    }

} // namespace MPX
