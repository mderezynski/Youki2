//  MPX
//  Copyright (C) 2010 MPX development.
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License Version 2
//  as published by the Free Software Foundation.
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
#  include <config.h>
#endif //HAVE_CONFIG_H

#include <glib/gi18n.h>
#include <gtkmm.h>
#include <gdkmm.h>
#include "mpx/util-graphics.hh"
#include "splash-screen.hh"
using namespace Glib;

namespace MPX
{
    Splashscreen::Splashscreen(
    )
            : Gtk::Window (Gtk::WINDOW_TOPLEVEL)
            , m_logo (Gdk::Pixbuf::create_from_file (build_filename(DATA_DIR, "images" G_DIR_SEPARATOR_S "splash.png")))
            , m_logo_w (m_logo->get_width())
            , m_logo_h (m_logo->get_height())
            , m_bar_w (223)
            , m_bar_h (2)
            , m_bar_x (24)
            , m_bar_y (64)
            , m_percent (0.0)
    {
        set_size_request (m_logo_w, m_logo_h);
        set_title (_("Youki is starting!"));
        set_skip_taskbar_hint (true);
        set_skip_pager_hint (true);
        set_keep_above (true);
        set_resizable (false);
        set_decorated (false);
        set_app_paintable (true);

        set_position (Gtk::WIN_POS_CENTER);
        set_type_hint (Gdk::WINDOW_TYPE_HINT_SPLASHSCREEN);

        Glib::RefPtr<Gdk::Screen> screen = Gdk::Screen::get_default();

	Gdk::RGBA rgba ;
	rgba.set_rgba( 0., 0., 0., 0. ) ;
	override_background_color( rgba ) ;

        // FIXME: Port this to use Cairo
        // if( m_has_alpha )
        // {
        //     set_colormap (screen->get_rgba_colormap());
        // }
        // else
        // {
        //     Glib::RefPtr<Gdk::Pixmap> mask_pixmap_window1, mask_pixmap_window2;
        //     Glib::RefPtr<Gdk::Bitmap> mask_bitmap_window1, mask_bitmap_window2;

        //     m_logo->render_pixmap_and_mask(
        //           mask_pixmap_window1
        //         , mask_bitmap_window1
        //         , 0
        //     ) ;

        //     m_logo->render_pixmap_and_mask(
        //           mask_pixmap_window2
        //         , mask_bitmap_window2
        //         , 128
        //     ) ;

        //     shape_combine_mask(
        //           mask_bitmap_window2
        //         , 0
        //         , 0
        //     ) ;
        // }

        show();
    }

    void
    Splashscreen::set_message(
          const std::string&    message
        , double                percent
    )
    {
        m_percent = percent;
        m_message = message;

        queue_draw ();

        while (gtk_events_pending())
            gtk_main_iteration();
    }

    bool
    Splashscreen::on_draw(
	const Cairo::RefPtr<Cairo::Context>& cairo
    )
    {
        cairo->set_operator( Cairo::OPERATOR_SOURCE ) ;

        if( m_has_alpha )
        {
            cairo->set_source_rgba( .0, .0, .0, .0 ) ;
            cairo->paint ();
        }

        Gdk::Cairo::set_source_pixbuf(
              cairo
            , m_logo
            , 0
            , 0
        ) ;
        cairo->paint () ;

/*
        cairo->set_operator( Cairo::OPERATOR_OVER ) ;
        cairo->set_source_rgba(
              1.
            , 1.
            , 1.
            , 1.
        ) ; 
        cairo->rectangle(
              m_bar_x - 2
            , m_bar_y - 2
            , m_bar_w + 4
            , m_bar_h + 4
        ) ;
        cairo->set_line_width(
              0.5
        ) ;
        cairo->stroke () ;
*/

        cairo->set_operator( Cairo::OPERATOR_OVER ) ;
        cairo->set_source_rgba( 1., 1., 1., .55 ) ; 
        cairo->rectangle(
              m_bar_x - 2
            , m_bar_y - 2
            , (m_bar_w+4) * m_percent
            , m_bar_h+4
        ) ;
        cairo->fill ();

        int lw, lh;

        Pango::FontDescription font_desc = get_style_context()->get_font ();

        int text_size_px = 8 ;
        int text_size_pt = static_cast<int> ((text_size_px * 72) / Util::screen_get_y_resolution (Gdk::Screen::get_default ())) ;

        font_desc.set_size(
              text_size_pt * PANGO_SCALE
        ) ;

        Glib::RefPtr<Pango::Layout> layout = create_pango_layout("") ;

        layout->set_font_description(
              font_desc
        ) ;

        layout->set_markup(
              m_message
        ) ; 

        layout->get_pixel_size(
              lw
            , lh
        ) ;

        cairo->set_operator( Cairo::OPERATOR_OVER );
        cairo->move_to(
                556 - lw 
              , m_bar_y - 4
        ) ; 
        cairo->set_source_rgba( 1., 1., 1., 1.);
        pango_cairo_show_layout(
              cairo->cobj()
            , layout->gobj()
        ) ;

        return true ;
    }
} // namespace MPX
