#include "youki-controller-status-icon.hh"
#include "mpx/mpx-main.hh"

namespace MPX
{
    YoukiControllerStatusIcon::YoukiControllerStatusIcon(
    )
    : m_notification( 0 )
    {

        m_play = services->get<IPlay>("mpx-service-play").get() ;
        m_icon = Gdk::Pixbuf::create_from_file( Glib::build_filename( DATA_DIR, "images" G_DIR_SEPARATOR_S "youki32x32.png" )) ;

        m_main_status_icon  = youki_status_icon_new_from_pixbuf( m_icon->gobj() ) ;
        youki_status_icon_set_visible( m_main_status_icon, TRUE ) ;

        g_object_connect(

            G_OBJECT( m_main_status_icon )

            , "signal::click"
            , G_CALLBACK(YoukiControllerStatusIcon::on_status_icon_click)
            , this

            , "signal::scroll-up"
            , G_CALLBACK(YoukiControllerStatusIcon::on_status_icon_scroll_up)
            , this

            , "signal::scroll-down"
            , G_CALLBACK(YoukiControllerStatusIcon::on_status_icon_scroll_down)
            , this

            , NULL
        ) ;

        GtkWidget* tray_icon = youki_status_icon_get_tray_icon( m_main_status_icon ) ;
        gtk_widget_realize( GTK_WIDGET( tray_icon ) );

        g_object_connect(

              G_OBJECT( tray_icon )

            , "signal::embedded"
            , G_CALLBACK(YoukiControllerStatusIcon::on_tray_icon_embedded)
            , this

            , "signal::destroyed"
            , G_CALLBACK(YoukiControllerStatusIcon::on_tray_icon_destroyed)
            , this

            , NULL
        ) ;


    }

    YoukiControllerStatusIcon::~YoukiControllerStatusIcon ()
    {
        g_object_unref(G_OBJECT( m_main_status_icon )) ;
    }

    void
    YoukiControllerStatusIcon::on_status_icon_click(
          YoukiStatusIcon*      icon
        , gpointer              data
    )
    {
        YoukiControllerStatusIcon & controller = *(reinterpret_cast<YoukiControllerStatusIcon*>(data)) ;
        controller.m_SIGNAL_clicked.emit() ;
    }

    void
    YoukiControllerStatusIcon::on_status_icon_scroll_up(
          YoukiStatusIcon*      icon
        , gpointer              data
    )
    {
        YoukiControllerStatusIcon & controller = *(reinterpret_cast<YoukiControllerStatusIcon*>(data)) ;
        controller.m_SIGNAL_scroll_up.emit() ;
    }

    void
    YoukiControllerStatusIcon::on_status_icon_scroll_down(
          YoukiStatusIcon*      icon
        , gpointer              data
    )
    {
        YoukiControllerStatusIcon & controller = *(reinterpret_cast<YoukiControllerStatusIcon*>(data)) ;
        controller.m_SIGNAL_scroll_down.emit() ;
    }

    void
    YoukiControllerStatusIcon::on_tray_icon_embedded(
          YoukiTrayIcon*        icon
        , gpointer              data
    )
    {
        YoukiControllerStatusIcon & controller = *(reinterpret_cast<YoukiControllerStatusIcon*>(data)) ;

        GtkWidget * widget = GTK_WIDGET( icon ) ;

/*
        Notification * p = new Notification( widget ) ;
        g_atomic_pointer_set((gpointer*)&controller.m_notification, p) ;
*/

        g_object_connect(
              G_OBJECT( icon )

            , "signal::enter-notify-event"
            , G_CALLBACK(YoukiControllerStatusIcon::on_status_icon_enter)
            , data 

            , "signal::leave-notify-event"
            , G_CALLBACK(YoukiControllerStatusIcon::on_status_icon_leave)
            , data 

            , NULL
        ) ;
    }

    void
    YoukiControllerStatusIcon::on_tray_icon_destroyed(
          YoukiTrayIcon*        icon
        , gpointer              data
    )
    {
        YoukiControllerStatusIcon & controller = *(reinterpret_cast<YoukiControllerStatusIcon*>(data)) ;

        if( controller.m_notification )
        {
            g_signal_handlers_disconnect_by_func(
                  gpointer(icon)
                , gpointer(YoukiControllerStatusIcon::on_status_icon_enter)
                , data 
            ) ;

            g_signal_handlers_disconnect_by_func(
                  gpointer(icon)
                , gpointer(YoukiControllerStatusIcon::on_status_icon_leave)
                , data 
            ) ;

            g_atomic_pointer_set( (gpointer*)&controller.m_notification, 0 ) ;
        }
    }

    void
    YoukiControllerStatusIcon::on_status_icon_enter(
          YoukiStatusIcon*      icon
        , GdkEventCrossing*     event
        , gpointer              data
    )
    {
/*
        YoukiControllerStatusIcon & controller = *(reinterpret_cast<YoukiControllerStatusIcon*>(data)) ;
    
        if( controller.m_play->property_status().get_value() == PLAYSTATUS_PLAYING ||
            controller.m_play->property_status().get_value() == PLAYSTATUS_PAUSED
        )
        {
            controller.m_notification->enable( true ) ;
        }
*/
    }

    void
    YoukiControllerStatusIcon::on_status_icon_leave(
          YoukiStatusIcon*      icon
        , GdkEventCrossing*     event
        , gpointer              data
    )
    {
/*
        YoukiControllerStatusIcon & controller = *(reinterpret_cast<YoukiControllerStatusIcon*>(data)) ;
        controller.m_notification->disable() ;
*/
    }
}
