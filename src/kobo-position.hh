#ifndef KOBO_POSITION_HH
#define KOBO_POSITION_HH

#include <gtkmm.h>
#include <gst/gst.h>
#include <cmath>
#include <boost/optional.hpp>
#include "mpx/i-youki-theme-engine.hh"

namespace MPX
{
    typedef boost::optional<Gdk::RGBA> Color_opt_t ;

    class KoboPosition

        : public Gtk::DrawingArea

    {
        protected:
        
            guint      m_duration ;        
            guint      m_position ;
            guint      m_seek_position ;
            double     m_seek_factor ;
            bool       m_clicked ;

            sigc::connection m_update_conn ;
	    Glib::Timer t; 
	    bool m_paused ;

            IYoukiThemeEngine * m_theme ;

	    Color_opt_t	m_color ;

       protected:
    
            inline double
            cos_smooth(
                  double
            ) ;

            double
            get_position(
            ) ;

	    bool
	    draw()
	    {
		queue_draw() ;
		return true ;
	    }
	
	public:

	    void
	    pause()
            {
		m_paused = true ;
		t.reset() ;
		m_update_conn = Glib::signal_timeout().connect( sigc::mem_fun( *this, &MPX::KoboPosition::draw ), 100 ) ;
	    }

	    void
	    unpause()
	    {
		m_paused = false ;
		m_update_conn.disconnect() ;
	    }

	    void
	    set_color(
		  const Color_opt_t& v = Color_opt_t()
	    )
	    {
		m_color = v ;
		queue_draw() ;
	    }

        public:

            typedef sigc::signal<void, guint> SignalSeekEvent ;

        protected:

	    double
	    get_alpha_at_time()
	    {
		if( !m_paused) return 1 ;

		double t_mod = std::fmod( t.elapsed(), 1.5 ) ;

		if( t_mod < 0.5 )
			return 0 ;

		return 1 ;
	    }
    
            SignalSeekEvent m_SIGNAL_seek_event ;

        public:

            SignalSeekEvent&
            signal_seek_event()
            {
                return m_SIGNAL_seek_event ;
            }

            KoboPosition () ;
            virtual ~KoboPosition () ;

            void
            set_position(
                  gint
                , gint
            ) ;

            void
            start(
            ) ;

            void
            stop(
            ) ;

        protected:

            virtual bool
            on_draw(
		const Cairo::RefPtr<Cairo::Context>&
            ) ;

            virtual bool
            on_leave_notify_event(
                  GdkEventCrossing*
            ) ;

            virtual bool
            on_enter_notify_event(
                  GdkEventCrossing*
            ) ;

            virtual bool
            on_button_press_event(
                  GdkEventButton*
            ) ;

            virtual bool
            on_button_release_event(
                  GdkEventButton*
            ) ;

            virtual bool
            on_motion_notify_event(
                  GdkEventMotion*
            ) ;

            virtual bool
            on_focus_in_event(
                  GdkEventFocus*
            ) ;

            virtual bool
            on_key_press_event(
                  GdkEventKey*
            ) ;
    } ; 
}

#endif // KOBO_POSITION_HH
