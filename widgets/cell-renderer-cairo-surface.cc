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

#include "mpx/widgets/cell-renderer-cairo-surface.hh"

using namespace Gtk;

namespace MPX
{
    CellRendererCairoSurface::CellRendererCairoSurface ()
        : ObjectBase              (typeid(CellRendererCairoSurface))
        , property_surface_       (*this, "surface", RefSurface(0))
        , property_alpha_         (*this, "alpha",   1.)
    {
    }

    CellRendererCairoSurface::~CellRendererCairoSurface ()
    {
    }

    void
    CellRendererCairoSurface::get_size_vfunc  (Gtk::Widget & widget,
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
    
      RefSurface surface = property_surface_.get_value();

      if( surface )
      {
        calc_width += surface->get_width();
        calc_height += surface->get_height();
      } 
  
      calc_width += xpad * 2; 
      calc_height += ypad * 2; 

      if (x_offset) *x_offset = 0;
      if (y_offset) *y_offset = 0;

      if (surface && cell_area && surface->get_width() > 0 && surface->get_height() > 0)
      {
        if (x_offset)
        {
          *x_offset = int ((((widget.get_direction () == Gtk::TEXT_DIR_RTL) ? (1.0 - xalign) : (xalign)) *
                        (cell_area->get_width() - calc_width - (2 * xpad))));
          *x_offset = int (std::max (*x_offset, 0) + xpad); 
        }

        if (y_offset)
        {
          *y_offset = (yalign * (cell_area->get_height() - calc_height - (2 * ypad)));
          *y_offset = std::max (*y_offset, 0) + ypad; 
        }
      }

      if (width)
        *width = calc_width;
  
      if (height)
        *height = calc_height;
    }

    void
    CellRendererCairoSurface::render_vfunc    (Glib::RefPtr<Gdk::Drawable> const& window,
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

      RefSurface surface = property_surface_.get_value();
     
      if( !surface ) 
        return;

      /* FIXME: Account for INSENSITIVE/SELECTED/PRELIT */

      Cairo::RefPtr< ::Cairo::Context> cr = window->create_cairo_context(); 
      GdkRectangle r = *(cell_area.gobj());

      r.x += int ((cell_area.get_width() - surface->get_width()) * xalign);
      r.y += int ((cell_area.get_height() - surface->get_height()) * yalign);
      r.x += xpad;
      r.y += ypad;

      cr->set_operator (Cairo::OPERATOR_ATOP);
      cr->set_source (surface, r.x, r.y);
      cr->rectangle (r.x, r.y, surface->get_width(), surface->get_height());
      cr->clip();
      cr->paint_with_alpha (property_alpha_.get_value());
    }

    ///////////////////////////////////////////////
    /// Object Properties
    ///////////////////////////////////////////////

    ProxyOf<PropSurface>::ReadWrite
    CellRendererCairoSurface::property_surface()
    {
      return ProxyOf<PropSurface>::ReadWrite (this, "surface");
    }

    ProxyOf<PropFloat>::ReadWrite
    CellRendererCairoSurface::property_alpha()
    {
      return ProxyOf<PropFloat>::ReadWrite (this, "alpha");
    }

};
