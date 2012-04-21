#ifndef KOBO_MAIN_WINDOW_HH
#define KOBO_MAIN_WINDOW_HH

#include <gtkmm.h>
#include <boost/ref.hpp>

namespace MPX
{
        class MainWindow
        : public Gtk::Window
        {
            protected:

                    int                         m_bottom_pad ;
                    bool                        m_composited ;

                    sigc::signal<void>          SignalPause ;

		    Gtk::Alignment*		a1 ;

		    Glib::Timer			m_button_timeout ;
		    sigc::connection		m_button_timeout_conn ;
		    bool			m_do_disconnect ;
		    Gtk::EventBox*		m_evbox ;

		    bool
		    timer_check_func()
		    {
			if( m_button_timeout.elapsed() > 2 )
			{
				m_button_timeout_conn.disconnect() ;			    
				SignalPause.emit() ;
				return false ;
			}

			if( m_do_disconnect )
			{
				return false ;
			}

			return true ;
		    }

		    bool
		    evbox_on_button_press_event( GdkEventButton* ) 
	            {
			g_message( G_STRFUNC ) ;
			
			m_button_timeout.stop() ;
			m_button_timeout.reset() ;

			m_button_timeout_conn = Glib::signal_timeout().connect( sigc::mem_fun( *this, &MainWindow::timer_check_func), 100 ) ;
			m_button_timeout.start() ;

			return false;
		    }

		    bool 
		    evbox_on_button_release_event( GdkEventButton* )
		    {
			g_message( G_STRFUNC ) ;

			m_do_disconnect = true ;
			m_button_timeout_conn.disconnect() ;

			return false ;
		    }

            public:

                    MainWindow () ;
                    virtual ~MainWindow () ;

		    sigc::signal<void>&
                    signal_pause()
                    {
                        return SignalPause ;
                    }

		    Gtk::EventBox&
		    w()
		    {
			return *m_evbox ;
		    }

                    void
                    set_widget_top( Gtk::Widget & w ) ;

                    void
                    clear_widget_top() ;
        } ;
}

#endif // KOBO_MAIN_WINDOW_HH
