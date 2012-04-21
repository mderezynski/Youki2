#include "config.h"
#include "mpx/widgets/youki-clutter-widget.hh"

namespace MPX
{
    YoukiClutterWidget::YoukiClutterWidget(
    )
    {
        add_events( Gdk::EventMask( Gdk::EXPOSURE_MASK) ) ;
    }

    void
    YoukiClutterWidget::on_size_allocate(
          Gtk::Allocation& a
    )
    {
        Clutter::Gtk::Embed::on_size_allocate( a ) ;
        redraw() ;
    }

    void
    YoukiClutterWidget::on_show(
    )
    {
        Clutter::Gtk::Embed::on_show() ;
        redraw() ;
    }
}
