#ifndef KOBO_MAIN_WINDOW_HH
#define KOBO_MAIN_WINDOW_HH

#include <gtkmm.h>
#include <gtk/gtk.h>
#include <boost/ref.hpp>
#include "mpx/widgets/cairo-extensions.hh"

namespace MPX
{
        class MainWindow
        : public Gtk::Window
        {
            protected:

		    Gtk::Alignment  *a1 ;

            public:

                    MainWindow () ;
                    virtual ~MainWindow () ;

                    void
                    set_widget_top( Gtk::Widget & w ) ;

                    void
                    clear_widget_top() ;
        } ;
}

#endif // KOBO_MAIN_WINDOW_HH
