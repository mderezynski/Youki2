//  MPX
//  Copyright (C) 2010 MPX development.
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

#include "dialog-about.hh"

#include <glib/gi18n.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gdkmm/general.h>
#include <cairomm/cairomm.h>
#include <cmath>
#include "mpx/util-graphics.hh"

namespace
{
  int const mask_alpha_threshold = 128;

  // Animation settings

  int const animation_fps = 24;
  int const animation_frame_period_ms = 1000 / animation_fps;

  char const*  text_font          = "Sans";
  int const    text_size_px       = 10;
  double const text_colour[3]     = { 1.0, 1.0, 1.0 };
  double const text_bottom_margin = 24;
  double const text_fade_in_time  = 0.2;
  double const text_fade_out_time = 0.05;
  double const text_hold_time     = 0.8;
  double const text_time          = text_fade_in_time + text_fade_out_time + text_hold_time;
  double const text_full_alpha    = 0.90;

  double const initial_delay      = 2.0;

  // Credits text
  // Each list of names is sorted in dictionary order, first according
  // to last name, then according to first name.

  char const* credit_text[] =
  {
      N_("<big>Youki is brought to you by</big>"),
      "Milosz Derezynski",
      "David Le Brun",
      "Chong Kai Xiong",
      "Siavash Safi",
  };

  unsigned int const n_lines = G_N_ELEMENTS (credit_text);

  double const total_animation_time = n_lines * text_time;
  double const start_time = initial_delay;
  double const end_time   = start_time + total_animation_time;

  inline double
  cos_smooth (double x)
  {
      return (1.0 - std::cos (x * G_PI)) / 2.0;
  }

  char const*
  get_text_at_time (double time)
  {
      if (time >= start_time && time <= end_time)
      {
          unsigned int line = static_cast<unsigned int> ((time - start_time) / text_time);
          return credit_text[line];
      }
      else
      {
          return 0;
      }
  }

  double
  get_text_alpha_at_time (double time)
  {
      if (time >= start_time && time <= end_time)
      {
          double offset = std::fmod ((time - start_time), text_time);

          if (offset < text_fade_in_time)
          {
              return text_full_alpha * cos_smooth (offset / text_fade_in_time);
          }
          else if (offset < text_fade_in_time + text_hold_time)
          {
              return text_full_alpha;
          }
          else
          {
              return text_full_alpha * cos_smooth (1.0 - (offset - text_fade_in_time - text_hold_time) / text_fade_out_time);
          }
      }
      else
      {
          return 0;
      }
  }
}

namespace MPX
{
  AboutDialog::AboutDialog ()
  {
      set_title (_("About MPX"));

      set_position (Gtk::WIN_POS_CENTER);
      set_resizable (false);

      set_type_hint (Gdk::WINDOW_TYPE_HINT_DIALOG);
      set_decorated (false);

      set_app_paintable (true);
      add_events (Gdk::ALL_EVENTS_MASK);

      char const* filename = DATA_DIR G_DIR_SEPARATOR_S "images" G_DIR_SEPARATOR_S "about.png";
      m_background = Gdk::Pixbuf::create_from_file (filename);

      set_size_request (m_background->get_width (), m_background->get_height ());

      if(!Gdk::Screen::get_default()->is_composited())
      {
          Glib::RefPtr<Gdk::Pixmap> pixmap;
          Glib::RefPtr<Gdk::Bitmap> mask;
          m_background->render_pixmap_and_mask( pixmap, mask, mask_alpha_threshold ) ;
          shape_combine_mask( mask, 0, 0 ) ;
      }
      else
      {
          set_colormap (Gdk::Screen::get_default()->get_rgba_colormap());
      }

      m_timer.stop ();
      m_timer.reset ();
  }

  bool
  AboutDialog::on_key_press_event (GdkEventKey *event)
  {
      if (event->keyval == GDK_Escape)
          hide ();

      return false;
  }

  bool
  AboutDialog::on_button_press_event (GdkEventButton *event)
  {
      if (event->button == 1)
          hide ();

      return false;
  }

  bool
  AboutDialog::on_expose_event (GdkEventExpose *event)
  {
      draw_frame ();
      return false;
  }

  bool
  AboutDialog::on_delete_event (GdkEventAny *event)
  {
      hide ();
      return true;
  }

  void
  AboutDialog::on_map ()
  {
      Gtk::Window::on_map ();

      if (m_timer.elapsed () <= 0.0)
      {
          m_timer.start ();

          m_update_connection = Glib::signal_timeout ().connect (sigc::mem_fun (this, &AboutDialog::update_frame),
                                                                 animation_frame_period_ms);
      }
  }

  void
  AboutDialog::on_unmap ()
  {
      if (m_timer.elapsed () > 0.0)
      {
          m_update_connection.disconnect ();

          m_timer.stop ();
          m_timer.reset ();
      }

      Gtk::Window::on_unmap ();
  }

  void
  AboutDialog::draw_frame ()
  {
      Cairo::RefPtr<Cairo::Context> cr = get_window ()->create_cairo_context ();

      cr->set_operator(Cairo::OPERATOR_CLEAR);
      cr->paint ();

      Gdk::Cairo::set_source_pixbuf (cr, m_background, 0, 0);
      cr->set_operator (Cairo::OPERATOR_SOURCE);
      cr->paint ();

      double current_time = m_timer.elapsed ();

      if (current_time >= start_time && current_time <= end_time)
      {
          Pango::FontDescription font_desc (text_font);
          int text_size_pt = static_cast<int> ((text_size_px * 72) / Util::screen_get_y_resolution (Gdk::Screen::get_default ()));
          font_desc.set_size (text_size_pt * PANGO_SCALE);

          char const* text  = get_text_at_time (current_time);
          double      alpha = get_text_alpha_at_time (current_time);

          Glib::RefPtr<Pango::Layout> layout = Glib::wrap (pango_cairo_create_layout (cr->cobj ()));
          layout->set_font_description (font_desc);
          layout->set_markup (_(text));

          int width, height;
          layout->get_pixel_size (width, height);

          cr->move_to ((m_background->get_width () - width) / 2,
                        m_background->get_height () - height - text_bottom_margin);
          cr->set_source_rgba (text_colour[0], text_colour[1], text_colour[2], alpha);
          cr->set_operator (Cairo::OPERATOR_OVER);

          pango_cairo_show_layout (cr->cobj (), layout->gobj ());
      }
  }

  bool
  AboutDialog::update_frame ()
  {
      queue_draw ();

      return true;
  }

} // namespace MPX
