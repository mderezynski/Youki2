#ifndef MPX_CELL_RENDERER_ALBUM_INFO_HH
#define MPX_CELL_RENDERER_ALBUM_INFO_HH

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif //HAVE_CONFIG_H

#include <gtkmm.h>
#include <cairomm/cairomm.h>
#include <boost/shared_ptr.hpp>
#include "mpx/aux/glibaddons.hh"

namespace MPX
{
    struct AlbumInfo
    {
        std::string Name;
        std::string Artist;
        std::string Release;
        std::string Type;
        std::string Genre;
        gint64      Qual;
        gint64      Rating;
    };

    typedef boost::shared_ptr<AlbumInfo> AlbumInfo_pt;
    typedef Glib::Property<AlbumInfo_pt> PropAlbumInfo;

    class CellRendererAlbumData
    : public Gtk::CellRenderer
    {
      public:

        CellRendererAlbumData ();
        virtual ~CellRendererAlbumData ();

        ProxyOf<PropAlbumInfo>::ReadWrite
        property_info ();

        ProxyOf<PropAlbumInfo>::ReadOnly
        property_info () const;

      private:

        PropAlbumInfo property_info_;

        Glib::RefPtr<Gdk::Pixbuf> m_Quality[4];
        Glib::RefPtr<Gdk::Pixbuf> m_Ratings[6];

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
