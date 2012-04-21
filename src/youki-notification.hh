#ifndef _YOUKI_NOTIIFCATION_HH
#define _YOUKI_NOTIIFCATION_HH

#include "config.h" 

#include <boost/optional.hpp>
#include <glibmm/ustring.h>
#include <gdkmm/color.h>
#include <gdkmm/pixbuf.h>
#include <gtkmm/fixed.h>
#include <gtkmm/window.h>
#include <sigc++/connection.h>

#include <cairomm/cairomm.h>
#include <pangomm/layout.h>

#include "mpx/mpx-types.hh"

#include <string>

#include "kobo-position.hh"

#include "mpx/i-youki-play.hh"

namespace MPX
{
    class Notification
    : public Gtk::Window
    {
        public:

            Notification(
              GtkWidget* widget
            ) ;

            virtual ~Notification ();

            void
            enable(
                  bool /*tooltip*/ = true
            ) ;

            void
            disable(
            ) ;

            void
            set_position(
                  gint64
                , gint64
            ) ;

            void
            set_coords(
                  int x
                , int y
            ) ;

            void
            set_playstatus(
                  PlayStatus
            ) ;

            void
            set_metadata(
                  const Glib::RefPtr<Gdk::Pixbuf>&
                , const MPX::Track&
            ) ;

            void
            clear(
            ) ;

        protected:

            virtual bool
            on_expose_event(
                GdkEventExpose*
            ) ;

            virtual bool
            on_button_press_event(
                GdkEventButton*
            ) ;

            virtual void
            on_realize() ;

            virtual void
            on_map() ;

            virtual void
            on_unmap() ;

            virtual void
            on_show() ;
      
        private:

            Gtk::Fixed          m_fixed ;
            KoboPosition      * m_kobo_position ;

            enum ArrowLocation
            {
              ARROW_TOP,
              ARROW_BOTTOM,
            };

            enum ArrowPosition
            {
              ARROW_POS_DEFAULT,
              ARROW_POS_LEFT,
              ARROW_POS_RIGHT,
            };

            void  reposition ();
            void  update_mask ();
            void  disappear ();

            void  acquire_widget_info(
                  int & x
                , int & y
                , int & width
                , int & height
            ) ;
                                      

            void
            draw_arrow(
                  Cairo::RefPtr<Cairo::Context> & cr
                , int width
                , int height
            ) ;

            void
            draw_arrow_outline(
                  Cairo::RefPtr<Cairo::Context> & cr
                , int width
                , int height
            ) ;

            void
            draw_arrow_mask(
                  Cairo::RefPtr<Cairo::Context> & cr
                , int width
                , int height
            ) ;

            void
            queue_update() ;

            bool
            update_frame() ;

            GtkWidget                     * m_widget ;

            Gdk::Color                      m_outline
                                          , m_inlay ;

            Glib::RefPtr<Gdk::Pixbuf>       m_image ;
            std::string                     m_text ;
            bool                            m_fade ;

            Glib::Timer                     m_timer ;
            sigc::connection                m_update_connection ;
            double                          m_time_offset ;

            Glib::RefPtr<Pango::Layout>     m_layout_1 ;
            Glib::RefPtr<Pango::Layout>     m_layout_2 ;

            int                             m_width ;
            int                             m_height ;
            int                             m_ax ;
            int                             m_x, m_y ;

            bool                            m_has_alpha ;

            ArrowLocation                   m_location ;
            ArrowPosition                   m_position ;

            bool                            m_tooltip_mode ;

            Track                           m_metadata ;
    };
}

#endif // _YOUKI_NOTIFICATION_HH
