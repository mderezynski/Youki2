//  MPX - The Dumb Music CellRendererSourceer
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
//  The MPX project hereby grants permission for non-GPL compatible GStreamer
//  plugins to be used and distributed together with GStreamer and MPX. This
//  permission is above and beyond the permissions granted by the GPL license
//  MPX is covered by.

#ifndef MPX_CELL_RENDERER_SOURCE_HH
#define MPX_CELL_RENDERER_SOURCE_HH

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
    typedef Glib::Property<Gtk::TreeView*>      PropView;
    typedef Glib::Property<Gtk::TreeIter>       PropIter;

    class CellRendererSource
      : public Gtk::CellRenderer
    {
      public:

        CellRendererSource ();
        virtual ~CellRendererSource ();

        ProxyOf<PropSurface>::ReadWrite
        property_icon ();

        ProxyOf<PropString>::ReadWrite
        property_label ();

        ProxyOf<PropBool>::ReadWrite
        property_active ();

        ProxyOf<PropView>::ReadWrite
        property_view ();

        ProxyOf<PropIter>::ReadWrite
        property_iter ();

      private:

        PropSurface property_icon_;
        PropString  property_label_;
        PropBool    property_active_;
        PropView    property_view_;
        PropIter    property_iter_;

      protected:

        virtual void
        get_size_vfunc  (Gtk::Widget & widget,
                         const Gdk::Rectangle * cell_area,
                         int *   x_offset,
                         int *   y_offset,
                         int *   width,
                         int *   height) const; 

        virtual void
        render_vfunc    (Glib::RefPtr<Gdk::Drawable> const& window,
                         Gtk::Widget                      & widget,
                         Gdk::Rectangle              const& background_area,
                         Gdk::Rectangle              const& cell_area,
                         Gdk::Rectangle              const& expose_area,
                         Gtk::CellRendererState             flags);

    };
}

#endif // MPX_CELL_RENDERER_SOURCE_HH
