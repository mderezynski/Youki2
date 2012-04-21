#include "mpx/widgets/rounded-layout.hh"

#include <gtkmm.h>
#include "mpx/widgets/cairo-extensions.hh"

namespace MPX
{
    RoundedLayout::RoundedLayout(
          const Glib::RefPtr<Gnome::Glade::Xml>&  xml
        , const std::string&                      name
        , CairoCorners::CORNERS                   corners
        , double                                  radius
    )
    : Gnome::Glade::WidgetLoader<Gtk::DrawingArea>(xml, name)
    , m_corners( corners )
    , m_radius( radius )
    {
        add_events(Gdk::EXPOSURE_MASK);
    }

    RoundedLayout::~RoundedLayout()
    {
    }

    void
    RoundedLayout::set_text(
        const Glib::ustring& text
    )
    {
        m_text = text;
        queue_draw();
    }

    void
    RoundedLayout::on_size_request (Gtk::Requisition* r)
    {
        Glib::RefPtr<Pango::Layout> Layout = create_pango_layout(m_text);
        Pango::Rectangle Ink, Logical;
        Layout->get_pixel_extents(Ink, Logical);
        r->height = Logical.get_height() + 4;
    }

    bool
    RoundedLayout::on_expose_event(GdkEventExpose* G_GNUC_UNUSED)
    {
        Cairo::RefPtr<Cairo::Context> cairo = get_window()->create_cairo_context();

        Gdk::Rectangle const& a = get_allocation();

        cairo->rectangle(
              0
            , 0
            , a.get_width()
            , a.get_height()
        ) ;

        Gdk::Cairo::set_source_color(
              cairo
            , get_style()->get_bg(Gtk::STATE_NORMAL)
        ) ;

        cairo->set_operator(Cairo::OPERATOR_SOURCE);
        cairo->fill();

        RoundedRectangle(
              cairo
            , 0 
            , 0
            , a.get_width() 
            , a.get_height()
            , m_radius
            , m_corners 
        ) ;

        cairo->set_source_rgba(0., 0., 0., 1.);
        cairo->set_operator(Cairo::OPERATOR_ATOP) ;
        cairo->fill() ;
        
        Glib::RefPtr<Pango::Layout> Layout = create_pango_layout(m_text);

        Pango::Rectangle Ink, Logical;
        Layout->get_pixel_extents(Ink, Logical);

        cairo->move_to(
              (a.get_width() - Logical.get_width())/2.
            , (a.get_height() - Logical.get_height())/2.
        ) ;
    
        cairo->set_source_rgba(1., 1., 1., 1.);
        cairo->set_operator(Cairo::OPERATOR_SOURCE) ;
        pango_cairo_show_layout(
              cairo->cobj()
            , Layout->gobj()
        ) ;

        return true;
    }
}


