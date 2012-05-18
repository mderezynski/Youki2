#include "config.h"

#include <gtkmm.h>
#include "mpx/mpx-main.hh"
#include "mpx/widgets/cairo-extensions.hh"

#include "mpx/i-youki-theme-engine.hh"

#include "kobo-main.hh"

namespace MPX
{
    MainWindow::~MainWindow ()
    {
        get_position( Mcs::Key::adaptor<int>(mcs->key("mpx", "window-x")), Mcs::Key::adaptor<int>(mcs->key("mpx", "window-y")));
        get_size( Mcs::Key::adaptor<int>(mcs->key("mpx", "window-w")), Mcs::Key::adaptor<int>(mcs->key("mpx", "window-h")) );

	mcs->key_set<bool>("main-window","maximized", (get_window()->get_state()&Gdk::WINDOW_STATE_MAXIMIZED)) ;
    }

    MainWindow::MainWindow ()

        : m_bottom_pad( 0 )
        , m_composited( Gdk::Screen::get_default()->is_composited() )
	, m_do_disconnect( false )

    {
        set_title( "Youki" ) ;

        a1 = Gtk::manage( new Gtk::Alignment  ) ;
	add( *a1 ) ;

        a1->property_top_padding() = 0 ;
	a1->property_left_padding() = 0 ;
	a1->property_right_padding() = 0 ;
        a1->property_bottom_padding() = 7 ;

        a1->set_border_width( 0 ) ;

	set_border_width( 0 ) ;

        gtk_widget_realize( GTK_WIDGET( gobj() )) ;

        while (gtk_events_pending())
          gtk_main_iteration();

        resize(
              mcs->key_get<int>("mpx","window-w"),
              mcs->key_get<int>("mpx","window-h")
        );

        move(
              mcs->key_get<int>("mpx","window-x"),
              mcs->key_get<int>("mpx","window-y")
        );

	if( mcs->key_get<bool>("main-window","maximized"))
	    maximize() ;

        while (gtk_events_pending())
          gtk_main_iteration();
    }

    void
    MainWindow::set_widget_top( Gtk::Widget & w )
                    {
                        a1->add( w ) ;
                    }

    void
    MainWindow::clear_widget_top()
                    {
                        a1->remove() ;
                    }
}
