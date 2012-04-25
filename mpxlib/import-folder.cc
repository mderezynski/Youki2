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
#include <cstring>
#include <string>
#include "import-folder.hh"

namespace MPX
{
    namespace
    {
        const std::string ui_path = DATA_DIR G_DIR_SEPARATOR_S "ui" G_DIR_SEPARATOR_S "import-folder.ui";
    }

    DialogImportFolder*
    DialogImportFolder::create ()
    {
        DialogImportFolder *p = new DialogImportFolder(Gtk::Builder::create_from_file (ui_path));
        return p;
    }

    DialogImportFolder::DialogImportFolder(const Glib::RefPtr<Gtk::Builder>& builder)
        : WidgetLoader<Gtk::Dialog>(builder, "import-folder")
    {
    }

    void
    DialogImportFolder::get_folder_infos(Glib::ustring& uri)
    {
        Gtk::FileChooserButton* button = 0;
        m_Builder->get_widget("fcbutton", button);

        uri = button->get_current_folder_uri();
    }

    DialogImportFolder::~DialogImportFolder ()
    {
    }

} // namespace MPX
