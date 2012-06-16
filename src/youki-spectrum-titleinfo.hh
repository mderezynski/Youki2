#ifndef _YOUKI_SPECTRUM_TITLEINFO__HH
#define _YOUKI_SPECTRUM_TITLEINFO__HH

#include <gtkmm.h>
#include <cairomm/cairomm.h>
#include <vector>
#include <string>
#include <sigc++/sigc++.h>
#include <boost/optional.hpp>

#include "mpx/algorithm/modulo.hh"

#include "mpx/i-youki-play.hh"
#include "mpx/i-youki-theme-engine.hh"
#include "mpx/mpx-types.hh"

namespace MPX
{
    enum TapArea
    {
	  TAP_LEFT
	, TAP_CENTER
	, TAP_RIGHT
    };

    typedef boost::optional<Gdk::RGBA> Color_opt_t ;

    class YoukiSpectrumTitleinfo
    : public Gtk::DrawingArea 
    {
        private:

            sigc::signal<void, int>		  m_SIGNAL__area_tapped ;
            boost::shared_ptr<IYoukiThemeEngine>  m_theme ;
	    bool				  m_cursor_inside ;

	    Color_opt_t m_color ;

        public:

            sigc::signal<void, int>&
            signal_tapped()
            {
              return m_SIGNAL__area_tapped ;
            }

            YoukiSpectrumTitleinfo () ;
            virtual ~YoukiSpectrumTitleinfo () {}

            void
            reset(
            ) ;

	    void
	    set_color(
		  const Color_opt_t& v = Color_opt_t()
	    )
	    {
		m_color = v ;
		queue_draw() ;
	    }

        protected:

	    bool
	    on_enter_notify_event(
		  GdkEventCrossing*
	    ) 
	    {
		m_cursor_inside = true ;
		queue_draw() ;
		return true ;
	    }

	    bool
	    on_leave_notify_event(
		  GdkEventCrossing*
	    )
	    {
		m_cursor_inside = false ;
		queue_draw() ;
		return true ;
	    }

            bool
            on_button_press_event(
                  GdkEventButton*
            ) ;

            virtual bool
            on_draw(
                  const Cairo::RefPtr<Cairo::Context>&
            ) ;

            void
            draw_titleinfo(
                  const Cairo::RefPtr<Cairo::Context>&
            );

            void
            draw_background(
                  const Cairo::RefPtr<Cairo::Context>&
            );

            void
            draw_cover(
                  const Cairo::RefPtr<Cairo::Context>&
            );

      public:

          void
          set_info(
              const std::vector<std::string>&
          ) ;

          void
          set_cover(
              Glib::RefPtr<Gdk::Pixbuf>
          ) ;

          void
          clear() ;
    
      private:

          std::vector<std::string>  m_info ;
          Glib::RefPtr<Gdk::Pixbuf> m_cover ;

	  boost::optional<unsigned int> m_audio_bitrate ;
	  boost::optional<std::string>  m_audio_codec ;

	  boost::optional<guint>        m_total_time ;

      public:

	  void 
	  set_bitrate(
		boost::optional<guint> b
	    ) { m_audio_bitrate = b; queue_draw() ; }

	  void
	  set_codec(
		boost::optional<std::string> c
	    ) { m_audio_codec = c; queue_draw() ; }

	  void
	  set_playlist_total(
		guint total
	    ) {m_total_time = total; queue_draw(); }
   };
}


#endif // _YOUKI_SPECTRUM_TITLEINFO__HH
