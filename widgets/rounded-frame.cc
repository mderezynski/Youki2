#include "config.h"

#include "mpx/widgets/rounded-frame.hh"

#include <gtkmm.h>
#include "mpx/widgets/cairo-extensions.hh"
#include "mpx/i-youki-theme-engine.hh"
#include "mpx/mpx-main.hh"

#include "widget-marshalers.h"

namespace MPX
{
    bool RoundedFrame::signal_added = false ;

    RoundedFrame::RoundedFrame(
    )
    {
        if( !signal_added )
        {
            GTK_WIDGET_GET_CLASS(gobj())->set_scroll_adjustments_signal =
                    g_signal_new ("set-scroll-adjustments",
                              G_OBJECT_CLASS_TYPE (G_OBJECT_CLASS (G_OBJECT_GET_CLASS(G_OBJECT(gobj())))),
                              GSignalFlags (G_SIGNAL_RUN_FIRST),
                              0,
                              NULL, NULL,
                              g_cclosure_user_marshal_VOID__OBJECT_OBJECT, G_TYPE_NONE, 2, GTK_TYPE_ADJUSTMENT, GTK_TYPE_ADJUSTMENT);

            signal_added = true ;
        }

        g_signal_connect(G_OBJECT(gobj()), "set-scroll-adjustments", G_CALLBACK(set_adjustments), this) ;
    }

    RoundedFrame::~RoundedFrame()
    {
    }

    gboolean
    RoundedFrame::set_adjustments(
          GtkWidget*
        , GtkAdjustment*    hadj
        , GtkAdjustment*    vadj
        , gpointer          data
    )
    {
        RoundedFrame & frame = *reinterpret_cast<RoundedFrame*>(data) ;

        if( frame.get_child() )
        {
            GtkWidget * child = GTK_WIDGET(frame.get_child()->gobj()) ;

            guint signal_id = GTK_WIDGET_GET_CLASS( child )->set_scroll_adjustments_signal ;

            g_signal_emit( child, signal_id, 0, hadj, vadj ) ;
        }

        return TRUE ;
    }

    void
    RoundedFrame::on_size_request(
          Gtk::Requisition * req
    )
    {
        if( get_child() && get_child()->get_visible() )
        {
            Gtk::Requisition child_r = get_child()->size_request() ;
            req->width = std::max( 0, child_r.width ) ;
            req->height = child_r.height ;
        }
        else
        {
            req->width = 0 ;
            req->height = 0 ;
        }
    
        req->width += 7 ;        
        req->height += 5 ;
    }

    void
    RoundedFrame::on_size_allocate(
          Gtk::Allocation&  alloc
    ) 
    {
        if( !get_child() || !get_child()->get_visible() )
        {
            return ;
        }

        Gtk::Bin::on_size_allocate( alloc ) ;
        Gdk::Rectangle child_alloc ;

        child_alloc.set_x( 3 + alloc.get_x() ) ;
        child_alloc.set_y( 4 + alloc.get_y() ) ;
        child_alloc.set_width( std::max( 1, alloc.get_width() - 3 * 2 - 3 )) ;
        child_alloc.set_height( std::max( 1, alloc.get_height() - 4 * 2 )) ;

        get_child()->size_allocate( child_alloc ) ;
    } 

    bool
    RoundedFrame::on_expose_event(
            GdkEventExpose* event
    )
    {
        Cairo::RefPtr<Cairo::Context> cairo = get_window()->create_cairo_context() ;

        boost::shared_ptr<IYoukiThemeEngine> theme = services->get<IYoukiThemeEngine>("mpx-service-theme") ;

        const Gtk::Allocation& a = get_allocation() ;

        const ThemeColor& c_bg		= theme->get_color( THEME_COLOR_BACKGROUND ) ;
//        const ThemeColor& c_base	= theme->get_color( THEME_COLOR_BASE ) ;
//        const ThemeColor& c_outline	= theme->get_color( THEME_COLOR_ENTRY_OUTLINE ) ;

        cairo->set_source_rgba(
              c_bg.r
            , c_bg.g
            , c_bg.b
            , c_bg.a
        ) ;
        cairo->rectangle(
              a.get_x() 
            , a.get_y() 
            , a.get_width() 
            , a.get_height() 
        ) ;
        cairo->fill() ;

/*
        cairo->set_source_rgba(
              c_base.r
            , c_base.g
            , c_base.b
            , c_base.a
        ) ;
        cairo->rectangle(
              a.get_x() 
            , a.get_y() 
            , a.get_width() - 3
            , a.get_height() 
        ) ;
        cairo->fill() ;
 
	// Gtk::Bin::on_expose_event( event ) ;

        RoundedRectangle(
              cairo
            , get_allocation().get_x()
            , get_allocation().get_y() 
            , a.get_width() - 3 
            , a.get_height()
            , 4.
        ) ;

       cairo->set_source_rgba(
              c_outline.r
            , c_outline.g
            , c_outline.b
            , c_outline.a
        ) ;

        cairo->set_line_width( 1.25 ) ;
        cairo->stroke() ;
*/

        return true ;
    }
}


