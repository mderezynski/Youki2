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

using namespace Gtk;
using namespace Glib;

namespace MPX
{

  DialogSimpleEntry::~DialogSimpleEntry ()
  {
  }

  DialogSimpleEntry*
  DialogSimpleEntry::create ()
  {
    const std::string path = DATA_DIR G_DIR_SEPARATOR_S "glade" G_DIR_SEPARATOR_S "dialog-simple-entry.glade";
    Glib::RefPtr<Gnome::Glade::Xml> glade_xml = Gnome::Glade::Xml::create (path);
    DialogSimpleEntry * p = 0;
    glade_xml->get_widget_derived ("dialog", p);
    return p;
  }

  DialogSimpleEntry::DialogSimpleEntry (BaseObjectType                 * cobj,
                                        RefPtr<Gnome::Glade::Xml> const& xml)
  : Dialog    (cobj)
  , m_ref_xml (xml)
  {
  }

  int
  DialogSimpleEntry::run (Glib::ustring & text)
  {
    int response = Dialog::run ();
    text = dynamic_cast<Entry *>(m_ref_xml->get_widget("entry"))->get_text();
    return response;
  }

  void
  DialogSimpleEntry::set_heading (Glib::ustring const& heading)
  {
    dynamic_cast<Label *>(m_ref_xml->get_widget("heading"))->set_text (heading);
  }

} // namespace MPX
