#ifndef _YOUKI_ROUNDED_FRAME__HH
#define _YOUKI_ROUNDED_FRAME__HH

#include <gtkmm.h>
#include "mpx/widgets/cairo-extensions.hh"

namespace MPX
{
    class RoundedFrame
    : public Gtk::Bin 
    {
        protected:

                static bool signal_added ;

                virtual bool
                on_expose_event(
                      GdkEventExpose*
                ) ;

                virtual void
                on_size_allocate(
                      Gtk::Allocation&
                ) ;

                virtual void
                on_size_request(
                      Gtk::Requisition*
                ) ;

                static gboolean
                set_adjustments(
                      GtkWidget*
                    , GtkAdjustment*
                    , GtkAdjustment*
                    , gpointer
                ) ;

       public:

                RoundedFrame(
                ) ;

                virtual
                ~RoundedFrame() ;
    };
}
#endif // _YOUKI_ROUNDED_FRAME__HH
