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

#include "config.h"

#include "youki-simpleinfo.hh"

#include <glib/gi18n.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gdkmm/general.h>
#include <cairomm/cairomm.h>
#include <cmath>
#include "mpx/util-graphics.hh"
#include "mpx/widgets/cairo-extensions.hh"

namespace MPX
{
    YoukiSimpleInfo::YoukiSimpleInfo ()
    {
        set_app_paintable (true);
        set_size_request( -1, 24 ) ;
    }

    void
    YoukiSimpleInfo::clear ()
    {
        m_info.clear() ;
        queue_draw () ;
    }

    void
    YoukiSimpleInfo::set_info(
        const std::string& i
    )
    {
        m_info = i ;
        queue_draw() ;
    }

    bool
    YoukiSimpleInfo::on_expose_event (GdkEventExpose *event)
    {
        draw_frame ();
        return false;
    }

    void
    YoukiSimpleInfo::draw_frame ()
    {
        Cairo::RefPtr<Cairo::Context> cairo = get_window ()->create_cairo_context () ;

        const Gtk::Allocation& a = get_allocation() ;

        cairo->set_operator(Cairo::OPERATOR_SOURCE) ;
        cairo->set_source_rgba( 0.10, 0.10, 0.10, 1. ) ;
        cairo->paint () ;

        cairo->set_operator( Cairo::OPERATOR_OVER ) ;
        cairo->set_source_rgba(
              .8
            , .8
            , .8 
            , .08
        ) ;
        RoundedRectangle(
              cairo
            , 1 
            , 1 
            , (a.get_width() - 2)
            , double((a.get_height() - 2))
            , 4. 
        ) ;
        cairo->fill () ;

        Pango::FontDescription font_desc ("sans") ;
        int text_size_pt = static_cast<int> ((10 * 72) / Util::screen_get_y_resolution (Gdk::Screen::get_default ())) ;
        font_desc.set_size (text_size_pt * PANGO_SCALE) ;
        font_desc.set_weight (Pango::WEIGHT_BOLD) ;

        Glib::RefPtr<Pango::Layout> layout = Glib::wrap (pango_cairo_create_layout (cairo->cobj ())) ;
        layout->set_font_description (font_desc) ;
        layout->set_text (m_info) ;

        int width, height;
        layout->get_pixel_size (width, height) ;

        cairo->move_to(
              (a.get_width () - width) / 2
            , (a.get_height () - height) / 2
        ) ;

        cairo->set_source_rgba( 1., 1., 1., 1. ) ;
        cairo->set_operator (Cairo::OPERATOR_OVER) ;
        pango_cairo_show_layout (cairo->cobj (), layout->gobj ()) ;
    }
} // namespace MPX
