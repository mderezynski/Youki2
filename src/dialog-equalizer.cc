//  MPX
//  Copyright (C) 2007-2009 MPX development.
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
#endif //HAVE_CONFIG_H

#include <glibmm.h>
#include <glib/gi18n.h>
#include <gtkmm.h>
#include <libglademm.h>
#include <boost/format.hpp>
#include "mpx/mpx-main.hh"

#include "dialog-equalizer.hh"

namespace
{
  static boost::format band_f ("band%d");
}

namespace MPX
{
  //-- Equalizer implementation ----------------------------------------------
  Equalizer *
  Equalizer::create ()
  {
      const std::string path = DATA_DIR G_DIR_SEPARATOR_S "glade" G_DIR_SEPARATOR_S "equalizer.glade";

      Glib::RefPtr<Gnome::Glade::Xml> glade_xml = Gnome::Glade::Xml::create(path);

      Equalizer* dialog = 0;
      glade_xml->get_widget_derived ("equalizer", dialog);

      return dialog;
  }

  void
  Equalizer::reset ()
  {
    for (int n = 0; n < 10; ++n)
    {
      mcs->key_set ("audio", ((band_f % n).str()), double (0.0));
    }
  }

  Equalizer::Equalizer (BaseObjectType*                        obj,
                        Glib::RefPtr<Gnome::Glade::Xml> const& xml)
  : Gtk::Window (obj)
  , m_ref_xml   (xml)
  {
    dynamic_cast<Gtk::Button*>(m_ref_xml->get_widget ("close"))->signal_clicked().connect
      (sigc::mem_fun (this, &Equalizer::hide));

    dynamic_cast<Gtk::Button*>(m_ref_xml->get_widget ("reset"))->signal_clicked().connect
      (sigc::mem_fun (this, &Equalizer::reset));

    for (int n = 0; n < 10; ++n)
    {
      mcs_bind->bind_range_float(*dynamic_cast<Gtk::Range*> (m_ref_xml->get_widget ((band_f % n).str ())) , "audio", (band_f % n).str());
    }
  }
} // namespace MPX
