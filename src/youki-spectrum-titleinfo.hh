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
    class YoukiSpectrumTitleinfo
    : public Gtk::DrawingArea 
    {
        private:

            sigc::signal<void> 			  m_signal ;
            boost::shared_ptr<IYoukiThemeEngine>  m_theme ;

        public:

            sigc::signal<void>&
            signal_clicked ()
            {
              return m_signal ;
            }

            YoukiSpectrumTitleinfo () ;
            virtual ~YoukiSpectrumTitleinfo () {}

            void
            reset(
            ) ;

        protected:

            bool
            on_button_press_event(
                  GdkEventButton*
            ) ;

            virtual bool
            on_expose_event(
                  GdkEventExpose*
            ) ;

            void
            draw_titleinfo(
                  Cairo::RefPtr<Cairo::Context>&
            );

            void
            draw_background(
                  Cairo::RefPtr<Cairo::Context>&
            );

            void
            draw_cover(
                  Cairo::RefPtr<Cairo::Context>&
            );

      public:

          void
          set_info(
              const std::vector<std::string>&
            , Glib::RefPtr<Gdk::Pixbuf>
          ) ;

          void
          clear () ;
    
      private:

          std::vector<std::string>  m_info ;
          Glib::RefPtr<Gdk::Pixbuf> m_cover ;

	  boost::optional<unsigned int> m_audio_bitrate ;
	  boost::optional<std::string>  m_audio_codec ;

      public:

	  void 
	  set_bitrate( boost::optional<unsigned int> b) { m_audio_bitrate = b; }

	  void
	  set_codec( boost::optional<std::string> c) { m_audio_codec = c; }
   };
}


#endif // _YOUKI_SPECTRUM_TITLEINFO__HH
