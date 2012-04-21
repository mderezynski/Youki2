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
#include <libglademm.h>
#include <cstring>
#include <string>

using namespace Glib;
using namespace Gtk;

namespace MPX
{
    DialogImportShare*
    DialogImportShare::create ()
    {
      const std::string path (build_filename(DATA_DIR, build_filename("glade","import-share.glade")));
      DialogImportShare *p = new DialogImportShare(Gnome::Glade::Xml::create (path));
      return p;
    }

    DialogImportShare::DialogImportShare(const Glib::RefPtr<Gnome::Glade::Xml>& xml)
   : Gnome::Glade::WidgetLoader<Gtk::Dialog>(xml, "import-share")
    , m_ref_xml(xml)
    {
        dynamic_cast<ToggleButton*>(m_ref_xml->get_widget("cb-show-credentials"))->signal_toggled().connect(
            sigc::mem_fun( *this, &DialogImportShare::on_cb_show_credentials_toggled));
    }

    void
    DialogImportShare::get_share_infos(Glib::ustring& share, Glib::ustring& name, Glib::ustring& login, Glib::ustring& password)
    {
        share = dynamic_cast<Entry*>(m_ref_xml->get_widget("share-location"))->get_text();
        name = dynamic_cast<Entry*>(m_ref_xml->get_widget("share-name"))->get_text();
        login = dynamic_cast<Entry*>(m_ref_xml->get_widget("share-login"))->get_text();
        password = dynamic_cast<Entry*>(m_ref_xml->get_widget("share-password"))->get_text();
    }

    void
    DialogImportShare::on_cb_show_credentials_toggled()
    {
        bool active = dynamic_cast<ToggleButton*>(m_ref_xml->get_widget("cb-show-credentials"))->get_active();
        dynamic_cast<Entry*>(m_ref_xml->get_widget("share-login"))->property_visibility() = active;
        dynamic_cast<Entry*>(m_ref_xml->get_widget("share-password"))->property_visibility() = active;
        m_ref_xml->get_widget("share-login")->grab_focus();
    }

    DialogImportShare::~DialogImportShare ()
    {
    }
} // namespace MPX
