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
//  The MPXx project hereby grants permission for non-GPL compatible GStreamer
//  plugins to be used and distributed together with GStreamer and MPXx. This
//  permission is above and beyond the permissions granted by the GPL license
//  MPXx is covered by.

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif //!HAVE_CONFIG_H

#include "mpx/util-ui.hh"
#include "mpx/widgets/dialog-simple-entry.hh"

#include <glibmm.h>
#include <glibmm/i18n.h>
#include <gtkmm.h>
#include <cstring>
#include <string>

namespace MPX
{
  namespace
  {
    const std::string ui_path = DATA_DIR G_DIR_SEPARATOR_S "ui" G_DIR_SEPARATOR_S "dialog-simple-entry.ui";
  }

  DialogSimpleEntry::~DialogSimpleEntry ()
  {
  }

  DialogSimpleEntry*
  DialogSimpleEntry::create ()
  {
    Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create_from_file (ui_path);
    DialogSimpleEntry * p = 0;
    builder->get_widget_derived ("dialog", p);
    return p;
  }

  DialogSimpleEntry::DialogSimpleEntry (BaseObjectType                  * cobj,
                                        Glib::RefPtr<Gtk::Builder> const& builder)
  : Dialog        (cobj)
  , m_ref_builder (builder)
  {
  }

  int
  DialogSimpleEntry::run (Glib::ustring & text)
  {
    int response = Dialog::run ();

    Gtk::Entry* entry = 0;
    m_ref_builder->get_widget("entry", entry);

    text = entry->get_text();
    return response;
  }

  void
  DialogSimpleEntry::set_heading (Glib::ustring const& heading)
  {
    Gtk::Label* label = 0;
    m_ref_builder->get_widget("heading", label);

    label->set_text (heading);
  }

} // namespace MPX
