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

#include "request-value.hh"

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
    RequestValue*
    RequestValue::create ()
    {
      const std::string path = DATA_DIR G_DIR_SEPARATOR_S "glade" G_DIR_SEPARATOR_S "request-value.glade";
      RequestValue *p = new RequestValue(Gnome::Glade::Xml::create (path));
      return p;
    }

    RequestValue::RequestValue(const Glib::RefPtr<Gnome::Glade::Xml>& xml)
    : Gnome::Glade::WidgetLoader<Gtk::Dialog>(xml, "request-value")
    , m_ref_xml(xml)
    {
    }

    void
    RequestValue::get_request_infos(Glib::ustring& value)
    {
        value = dynamic_cast<Entry*>(m_ref_xml->get_widget("field"))->get_text();
    }

    void
    RequestValue::set_question(const Glib::ustring& question)
    {
        dynamic_cast<Label*>(m_ref_xml->get_widget("question"))->set_text(question);
    }

    RequestValue::~RequestValue ()
    {
    }
} // namespace MPX
