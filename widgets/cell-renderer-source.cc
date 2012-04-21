//  MPX
//  Copyright (C) 2005-2007 MPX Project 
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

#include <glibmm.h>
#include <gtkmm.h>

#include "mpx/widgets/cell-renderer-source.hh"
#include "mpx/util-graphics.hh"

using namespace Gtk;

namespace
{
    using namespace MPX;

    void
    draw_selection(Cairo::RefPtr<Cairo::Context> cr,
                   Gdk::Color const& selection_color,
                   double x,
                   double y,
                   double width,
                   double height
    )
    {
        Gdk::Color selection_stroke = Util::color_shade(selection_color, 0.85);
        Gdk::Color selection_fill_light = Util::color_shade (selection_color, 1.1);
        Gdk::Color selection_fill_dark = Util::color_shade (selection_color, 0.90);

        Cairo::RefPtr<Cairo::LinearGradient> grad = Cairo::LinearGradient::create(x, y, x, y + height);

        grad->add_color_stop_rgb(
            0,
            selection_fill_light.get_red()/65535.,
            selection_fill_light.get_green()/65535.,
            selection_fill_light.get_blue()/65535.
        );

        grad->add_color_stop_rgb(
            1,
            selection_fill_dark.get_red()/65535.,
            selection_fill_dark.get_green()/65535.,
            selection_fill_dark.get_blue()/65535.
        );

        cr->set_source(grad);
        Util::cairo_rounded_rect(cr, x, y, width, height, 4.); 
        cr->fill ();

        cr->set_line_width(1.0);
        cr->set_source_rgb(
            selection_stroke.get_red()/65535.,
            selection_stroke.get_green()/65535.,
            selection_stroke.get_blue()/65535.);
        Util::cairo_rounded_rect(cr, x + 0.5, y + 0.5, width - 1, height - 1, 4.); 
        cr->stroke();
    }
}

namespace MPX
{
    namespace Util
    {
        void color_to_hsb(
                Gdk::Color const& color,
                double & hue,
                double & saturation,
                double & brightness
        );
    }

    CellRendererSource::CellRendererSource ()
        : ObjectBase            (typeid(CellRendererSource))
        , property_icon_        (*this, "icon", RefSurface(0))
        , property_label_       (*this, "label", "")
        , property_active_      (*this, "active", false)
        , property_view_        (*this, "view", 0)
        , property_iter_        (*this, "iter", Gtk::TreeIter())
    {
    }

    CellRendererSource::~CellRendererSource ()
    {
    }

    void
    CellRendererSource::get_size_vfunc  (Gtk::Widget & widget,
                                               const Gdk::Rectangle * cell_area,
                                               int *   x_offset,
                                               int *   y_offset,
                                               int *   width,
                                               int *   height) const 
    {
      double xalign = Gtk::CellRenderer::property_xalign();
      double yalign = Gtk::CellRenderer::property_yalign();
      unsigned int xpad = Gtk::CellRenderer::property_xpad();
      unsigned int ypad = Gtk::CellRenderer::property_ypad();
      int calc_width = 0, calc_height = 0;

      RefSurface surface = property_icon_.get_value();

      if( surface )
      {
        calc_width += surface->get_width();
        calc_height += surface->get_height();
      } 
  
      calc_width += xpad * 2; 
      calc_height += ypad * 2 /*+ 6*/ + 2; 

      Glib::RefPtr<Pango::Layout> layout = property_view_.get_value()->create_pango_layout (property_label_.get_value());
      Pango::Rectangle Ink, Logical;
      layout->get_pixel_extents(Ink, Logical);

      calc_width += Logical.get_width() + 4;

      if (x_offset) *x_offset = 0;
      if (y_offset) *y_offset = 0;

      if (width)
        *width = calc_width;
  
      if (height)
        *height = calc_height;
    }

    void
    CellRendererSource::render_vfunc    (Glib::RefPtr<Gdk::Drawable> const& window,
                                               Gtk::Widget                      & widget,
                                               Gdk::Rectangle              const& background_area,
                                               Gdk::Rectangle              const& cell_area,
                                               Gdk::Rectangle              const& expose_area,
                                               Gtk::CellRendererState             flags)
    {
      double xalign = Gtk::CellRenderer::property_xalign();
      double yalign = Gtk::CellRenderer::property_yalign();
      unsigned int xpad = Gtk::CellRenderer::property_xpad();
      unsigned int ypad = Gtk::CellRenderer::property_ypad();

      Gtk::TreeIter iter = property_iter_.get_value();
      bool selected = property_view_.get_value()->get_selection()->is_selected(iter);
    
      RefSurface surface = property_icon_.get_value();
     
      if( !surface ) 
        return;

      /* FIXME: Account for INSENSITIVE/SELECTED/PRELIT */

      Cairo::RefPtr< ::Cairo::Context> cr = window->create_cairo_context(); 

#if 0
      if(selected)
      {
        cr->rectangle(
            background_area.get_x(),
            background_area.get_y(),
            background_area.get_width(),
            background_area.get_height()
        );
        Gdk::Cairo::set_source_color(cr, property_view_.get_value()->get_style()->get_base(Gtk::STATE_NORMAL));
        cr->fill();

        draw_selection(
            cr, 
            property_view_.get_value()->get_style()->get_base(Gtk::STATE_SELECTED),
            background_area.get_x()+2,  
            background_area.get_y()+2,
            background_area.get_width()-4,
            background_area.get_height()-4
        );
      }
#endif

      GdkRectangle r = *(cell_area.gobj());
      r.y += 2; // top padding, see above

      // Icon

      cr->set_operator (Cairo::OPERATOR_ATOP);
      cr->set_source (surface, r.x, r.y);
      cr->rectangle (r.x, r.y, surface->get_width(), surface->get_height());
      cr->fill ();

      Glib::RefPtr<Pango::Layout> layout = property_view_.get_value()->create_pango_layout (property_label_.get_value());
      if(selected)
        Gdk::Cairo::set_source_color(cr, property_view_.get_value()->get_style()->get_fg(Gtk::STATE_SELECTED));
      else
        Gdk::Cairo::set_source_color(cr, property_view_.get_value()->get_style()->get_fg(Gtk::STATE_NORMAL));

      // Label

      Pango::Rectangle Ink, Logical;
      layout->get_pixel_extents(Ink, Logical);
      r    = *(cell_area.gobj());
      r.x += surface->get_width() + 4;
      r.y += (r.height - Logical.get_height()) / 2;
      cr->move_to(r.x, r.y);
      pango_cairo_show_layout(cr->cobj(), layout->gobj());
    }

    ///////////////////////////////////////////////
    /// Object Properties
    ///////////////////////////////////////////////

    ProxyOf<PropSurface>::ReadWrite
    CellRendererSource::property_icon()
    {
      return ProxyOf<PropSurface>::ReadWrite
        (this, "icon");
    }

    ProxyOf<PropString>::ReadWrite
    CellRendererSource::property_label()
    {
      return ProxyOf<PropString>::ReadWrite
        (this, "label");
    }

    ProxyOf<PropBool>::ReadWrite
    CellRendererSource::property_active()
    {
      return ProxyOf<PropBool>::ReadWrite
        (this, "active");
    }

    ProxyOf<PropView>::ReadWrite
    CellRendererSource::property_view()
    {
      return ProxyOf<PropView>::ReadWrite
        (this, "view");
    }

    ProxyOf<PropIter>::ReadWrite
    CellRendererSource::property_iter()
    {
      return ProxyOf<PropIter>::ReadWrite
        (this, "iter");
    }
}
