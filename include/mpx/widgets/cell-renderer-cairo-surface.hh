//  MPXx - The Dumb Music CellRendererCairoSurfaceer
//  Copyright (C) 2005-2007 MPX development.
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
//  The MPXx project hereby grants permission for non-GPL compatible GStreamer
//  plugins to be used and distributed together with GStreamer and MPXx. This
//  permission is above and beyond the permissions granted by the GPL license
//  MPXx is covered by.

#ifndef MPX_CELL_RENDERER_CAIRO_SURFACE_HH
#define MPX_CELL_RENDERER_CAIRO_SURFACE_HH

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif //HAVE_CONFIG_H

#include <gtkmm.h>
#include <cairomm/cairomm.h>
#include "mpx/aux/glibaddons.hh"

namespace MPX
{
    typedef Cairo::RefPtr<Cairo::ImageSurface>  RefSurface;
    typedef Glib::Property<RefSurface>          PropSurface;

    class CellRendererCairoSurface
      : public Gtk::CellRenderer
    {
      public:

        CellRendererCairoSurface ();
        virtual ~CellRendererCairoSurface ();

        ProxyOf<PropSurface>::ReadWrite
        property_surface ();

      private:

        PropSurface property_surface_;

      protected:

        virtual void
        get_size_vfunc  (Gtk::Widget & widget,
                         const Gdk::Rectangle * cell_area,
                         int *   x_offset,
                         int *   y_offset,
                         int *   width,
                         int *   height) const; 

        virtual void
        render_vfunc    (Cairo::RefPtr<Cairo::Context> const& window,
                         Gtk::Widget                      & widget,
                         Gdk::Rectangle              const& background_area,
                         Gdk::Rectangle              const& cell_area,
                         Gtk::CellRendererState             flags);

    };
}

#endif // MPX_CELL_RENDERER_CAIRO_SURFACE_HH
