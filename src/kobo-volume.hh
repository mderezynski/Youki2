#ifndef KOBO_VOLUME_HH
#define KOBO_VOLUME_HH

#include <gtkmm.h>
#include "mpx/i-youki-theme-engine.hh"

namespace MPX
{
    class KoboVolume : public Gtk::DrawingArea
    {
        protected:
        
	    typedef std::vector<std::size_t> PosV_t ;

            bool	m_clicked ;
            std::size_t	m_volume ;        
	    std::size_t m_last_position ;

	    PosV_t m_posv ;

            IYoukiThemeEngine * m_theme ;            

	    Cairo::RefPtr<Cairo::ImageSurface> m_image_mute ;
    
        public:

            typedef sigc::signal<void, int> SignalSetVolume ;

        protected:
    
            SignalSetVolume m_SIGNAL_set_volume ;

        public:

            SignalSetVolume&
            signal_set_volume()
            {
                return m_SIGNAL_set_volume ;
            }

            KoboVolume () ;
            virtual ~KoboVolume () ;

            void
            set_volume(
                  int
            ) ;

        protected:

            virtual void
            vol_up () ;

            virtual void
            vol_down () ;

            virtual void
            on_size_request(
                Gtk::Requisition*
            ) ;

            virtual bool
            on_expose_event(
                GdkEventExpose*
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
            on_scroll_event(
                GdkEventScroll*
            ) ;

            virtual bool
            on_key_press_event(
                GdkEventKey*
            ) ;
    } ; 
}

#endif // KOBO_POSITION_HH
