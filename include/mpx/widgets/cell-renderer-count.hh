#ifndef MPX_CELL_RENDERER_COUNT_HH
#define MPX_CELL_RENDERER_COUNT_HH

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif //HAVE_CONFIG_H

#include <gtkmm.h>
#include <cairomm/cairomm.h>
#include "mpx/aux/glibaddons.hh"

namespace MPX
{
    typedef Cairo::RefPtr<Cairo::ImageSurface>  RefSurface;
    typedef Glib::Property<Glib::ustring>       PropText;

    enum BoxState
    {
      BOX_NOBOX,
      BOX_NORMAL,
      BOX_DASHED,
    };

    class CellRendererCount
      : public Gtk::CellRenderer
    {
      public:

        CellRendererCount ();
        virtual ~CellRendererCount ();

        ProxyOf<PropText>::ReadOnly
        property_text () const;

        ProxyOf<PropText>::ReadWrite
        property_text ();

        ProxyOf<PropInt>::ReadWrite
        property_box ();

      private:

        PropText  property_text_;
        PropInt   property_box_;

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

#endif
