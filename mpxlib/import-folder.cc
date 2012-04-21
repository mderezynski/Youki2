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

#include <glibmm.h>
#include <glibmm/i18n.h>
#include <gtkmm.h>
#include <libglademm.h>
#include <cstring>
#include <string>
#include "import-folder.hh"

using namespace Glib;
using namespace Gtk;

namespace MPX
{
    DialogImportFolder*
    DialogImportFolder::create ()
    {
      const std::string path (build_filename(DATA_DIR, build_filename("glade","import-folder.glade")));
      DialogImportFolder *p = new DialogImportFolder(Gnome::Glade::Xml::create (path));
      return p;
    }

    DialogImportFolder::DialogImportFolder(const Glib::RefPtr<Gnome::Glade::Xml>& xml)
    : Gnome::Glade::WidgetLoader<Gtk::Dialog>(xml, "import-folder")
    , m_ref_xml(xml)
    {
    }

    void
    DialogImportFolder::get_folder_infos(Glib::ustring& uri)
    {
        uri = dynamic_cast<FileChooser*>(m_ref_xml->get_widget("fcbutton"))->get_current_folder_uri();
    }

    DialogImportFolder::~DialogImportFolder ()
    {
    }
} // namespace MPX
