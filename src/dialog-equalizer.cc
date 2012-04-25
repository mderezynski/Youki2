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
#include <boost/format.hpp>
#include "mpx/mpx-main.hh"

#include "dialog-equalizer.hh"

namespace
{
  const std::string ui_path = DATA_DIR G_DIR_SEPARATOR_S "ui" G_DIR_SEPARATOR_S "equalizer.ui";

  static boost::format band_f ("band%d");
}

namespace MPX
{
  //-- Equalizer implementation ----------------------------------------------
  Equalizer *
  Equalizer::create ()
  {
      Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create_from_file(ui_path);

      Equalizer* dialog = 0;
      builder->get_widget_derived ("equalizer", dialog);

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

  Equalizer::Equalizer (BaseObjectType*                   obj,
                        Glib::RefPtr<Gtk::Builder> const& builder)
  : Gtk::Window   (obj)
  , m_ref_builder (builder)
  {
    Gtk::Button* close_button = 0;
    m_ref_builder->get_widget ("close", close_button);
    close_button->signal_clicked ().connect (sigc::mem_fun (this, &Equalizer::hide));

    Gtk::Button* reset_button = 0;
    m_ref_builder->get_widget ("reset", reset_button);
    reset_button->signal_clicked ().connect (sigc::mem_fun (this, &Equalizer::reset));

    for (int n = 0; n < 10; ++n)
    {
      std::string band_name = (band_f % n).str();

      Gtk::Range *range = 0;
      m_ref_builder->get_widget(band_name, range);

      mcs_bind->bind_range_float(*range, "audio", band_name);
    }
  }
} // namespace MPX
