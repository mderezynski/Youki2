#ifndef KOBO_CONTROLLER_STATUS_ICON_HH
#define KOBO_CONTROLLER_STATUS_ICON_HH

#include "mpx/mpx-types.hh"
#include "widgets/youki-status-icon.h"
#include "youki-notification.hh"

namespace MPX
{
    class IPlay ;
    class YoukiControllerStatusIcon
    {
        protected:

            YoukiStatusIcon                 * m_main_status_icon ;
            Notification                    * m_notification ;
            Glib::RefPtr<Gdk::Pixbuf>         m_icon ;
            IPlay                           * m_play ;

        public: 

            YoukiControllerStatusIcon() ;
            virtual ~YoukiControllerStatusIcon() ;

            void
            set_position(
                  gint64 position
                , gint64 duration
            )
            {
                if( m_notification )
                {
                    m_notification->set_position( position, duration ) ;
                }
            }

            void
            set_playstatus(
                PlayStatus status
            )
            {
                if( m_notification )
                {
                    m_notification->set_playstatus( status ) ;
                }
            }

            void
            set_metadata(
                  const Glib::RefPtr<Gdk::Pixbuf>&      image
                , const MPX::Track&                     t 
            )
            {
                if( m_notification )
                {
                    m_notification->set_metadata( image, t ) ;
                }
            }

            void
            clear(
            )
            {
                if( m_notification )
                {
                    m_notification->clear() ;
                }
            }

        protected:

            typedef sigc::signal<void> Signal_t ;

            Signal_t        m_SIGNAL_clicked ;
            Signal_t        m_SIGNAL_scroll_up ;
            Signal_t        m_SIGNAL_scroll_down ;

        public:

            Signal_t&
            signal_clicked ()
            {
                return m_SIGNAL_clicked ;
            }

            Signal_t&
            signal_scroll_down ()
            {
                return m_SIGNAL_scroll_up ;
            }

            Signal_t&
            signal_scroll_up ()
            {
                return m_SIGNAL_scroll_down ;
            }

        protected:

            static void
            on_status_icon_click(
                  YoukiStatusIcon*
                , gpointer
            ) ;
    
            static void
            on_status_icon_scroll_up(
                  YoukiStatusIcon*
                , gpointer
            ) ; 

            static void
            on_status_icon_scroll_down(
                  YoukiStatusIcon*
                , gpointer
            ) ;

            static void
            on_status_icon_enter(
                  YoukiStatusIcon*
                , GdkEventCrossing*
                , gpointer
            ) ;

            static void
            on_status_icon_leave(
                  YoukiStatusIcon*
                , GdkEventCrossing*
                , gpointer
            ) ;

            static void
            on_tray_icon_embedded(
                  YoukiTrayIcon*
                , gpointer
            ) ;

            static void
            on_tray_icon_destroyed(
                  YoukiTrayIcon*
                , gpointer
            ) ;
    } ;
}

#endif // KOBO_CONTROLLER_STATUS_ICON_HH
