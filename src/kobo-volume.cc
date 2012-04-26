#include <gtkmm.h>
#include <glibmm/i18n.h>
#include <boost/format.hpp>
#include <cmath>
#include "kobo-volume.hh"
#include "mpx/widgets/cairo-extensions.hh"
#include "mpx/util-graphics.hh"

#include "mpx/i-youki-theme-engine.hh"
#include "mpx/mpx-main.hh"

namespace
{
    const int pad = 1 ;

    Gdk::Color
    get_color_at_pos(
          const Gdk::Color&     c1
        , const Gdk::Color&     c2
        , const double          ratio
    )
    {
        Gdk::Color c ;

        double r = ( c1.get_red_p() * ( 1 - ratio ) + c2.get_red_p() * ratio ) ;
        double g = ( c1.get_green_p() * ( 1 - ratio ) + c2.get_green_p() * ratio ) ;
        double b = ( c1.get_blue_p() * ( 1 - ratio ) + c2.get_blue_p() * ratio ) ; 

        c.set_rgb_p( r, g, b ) ;

        return c ;
    }
}

namespace MPX
{
    KoboVolume::KoboVolume ()

        : m_volume( 0 )
        , m_clicked( false )

    {
        add_events(Gdk::EventMask(Gdk::LEAVE_NOTIFY_MASK | Gdk::ENTER_NOTIFY_MASK | Gdk::POINTER_MOTION_MASK | Gdk::POINTER_MOTION_HINT_MASK | Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK )) ;
        unset_flags(Gtk::CAN_FOCUS) ;

        m_theme = services->get<IYoukiThemeEngine>("mpx-service-theme").get() ;

        const ThemeColor& c = m_theme->get_color( THEME_COLOR_BASE ) ;

        Gdk::Color cgdk ;
        cgdk.set_rgb_p( c.r, c.g, c.b ) ; 
        modify_bg( Gtk::STATE_NORMAL, cgdk ) ;
        modify_base( Gtk::STATE_NORMAL, cgdk ) ;

	m_image_mute = Util::cairo_image_surface_from_pixbuf( Gdk::Pixbuf::create_from_file( Glib::build_filename( DATA_DIR, "images" G_DIR_SEPARATOR_S "mute-small-14x14px.png" ))) ;
 
    }

    KoboVolume::~KoboVolume () 
    {
    }
    
    void
    KoboVolume::on_size_request( Gtk::Requisition * req )
    {
        req->width  = 100 + 2*pad ;
        req->height = 18 ;
    }

    void
    KoboVolume::set_volume(
          int volume
    )
    {
        m_volume = std::max( 0, std::min( 100,volume)) ;
        queue_draw () ;
    }

    bool
    KoboVolume::on_expose_event(
        GdkEventExpose* G_GNUC_UNUSED
    )
    {
        double percent = double(m_volume) / 100. ; 

        Cairo::RefPtr<Cairo::Context> cairo = get_window()->create_cairo_context() ;

        const Gdk::Rectangle& a = get_allocation() ;
	const guint w = a.get_width() - 2 ;

        const ThemeColor& c_base /* :) */ = m_theme->get_color( THEME_COLOR_BACKGROUND ) ; 
        const ThemeColor& c = m_theme->get_color( THEME_COLOR_SELECT ) ;
        const ThemeColor& ct = m_theme->get_color( THEME_COLOR_TEXT_SELECTED ) ;

	GdkRectangle r ;
	double h, s, b ;

        Gdk::Color cgdk, c1, c2 ;

	cgdk.set_rgb_p( 0.35, 0.35, 0.35 ) ;
	
	Util::color_to_hsb( cgdk, h, s, b ) ;
	// b *= 0.85 ;
	// s *= 0.50 ;
	cgdk = Util::color_from_hsb( h, s, b ) ;

	Util::color_to_hsb( cgdk, h, s, b ) ;
	b *= 0.95 ;
	// s *= 0.50 ;
	c1 = Util::color_from_hsb( h, s, b ) ;

	Util::color_to_hsb( cgdk, h, s, b ) ;
	b *= 0.70 ;
	//s *= 0.75 ;
	c2 = Util::color_from_hsb( h, s, b ) ;

	Gdk::Color c_gradient = get_color_at_pos( c1, c2, percent ) ;

        cairo->set_operator(Cairo::OPERATOR_SOURCE) ;
        cairo->set_source_rgba(
              c_base.r
            , c_base.g
            , c_base.b
            , c_base.a
        ) ;
        cairo->paint () ;

        cairo->set_operator( Cairo::OPERATOR_OVER ) ;
        RoundedRectangle(
              cairo
            , 1 
            , 1 
            , w 
            , 16
            , 2.
        ) ;

        Cairo::RefPtr<Cairo::LinearGradient> position_bar_back_gradient = Cairo::LinearGradient::create(
              a.get_width() / 2 
            , 1 
            , a.get_width() / 2 
            , 16
        ) ;

        position_bar_back_gradient->add_color_stop_rgba(
              0. 
            , cgdk.get_red_p()
            , cgdk.get_green_p()
            , cgdk.get_blue_p()
            , 0.2 
        ) ;

        position_bar_back_gradient->add_color_stop_rgba(
              .2
            , cgdk.get_red_p()
            , cgdk.get_green_p()
            , cgdk.get_blue_p()
            , 0.195 
        ) ;

        position_bar_back_gradient->add_color_stop_rgba(
              .4
            , cgdk.get_red_p()
            , cgdk.get_green_p()
            , cgdk.get_blue_p()
            , 0.185
        ) ;

        position_bar_back_gradient->add_color_stop_rgba(
              .6
            , cgdk.get_red_p()
            , cgdk.get_green_p()
            , cgdk.get_blue_p()
            , 0.185
        ) ;
        
        position_bar_back_gradient->add_color_stop_rgba(
              .9
            , cgdk.get_red_p()
            , cgdk.get_green_p()
            , cgdk.get_blue_p()
            , 0.195
        ) ;

        position_bar_back_gradient->add_color_stop_rgba(
              1. 
            , cgdk.get_red_p()
            , cgdk.get_green_p()
            , cgdk.get_blue_p()
            , 0.2 
        ) ;

        cairo->set_source( position_bar_back_gradient ) ;

        RoundedRectangle(
              cairo
            , 1 
            , 1 
            , w
            , 16
            , 2.
        ) ;

        cairo->fill_preserve () ;

	cairo->set_source_rgba(
	      c_gradient.get_red_p()
	    , c_gradient.get_green_p()
	    , c_gradient.get_blue_p()
	    , 1. 
	) ;

        cairo->set_line_width( 0.75 ) ;
        cairo->stroke_preserve() ;
        cairo->clip() ;

	cairo->set_line_width( 1. ) ;

	/// VOLUME
	r.x         = 1 ; 
	r.y         = 1 ; 
	r.width     = fmax( 0, (a.get_width()-2) * double(percent)) ;
	r.height    = 16 ; 

	cairo->save () ;

	cairo->set_source_rgba(
	      c_gradient.get_red_p()
	    , c_gradient.get_green_p()
	    , c_gradient.get_blue_p()
	    , 1. 
	) ;

	cairo->set_operator(
	      Cairo::OPERATOR_OVER
	) ;

	RoundedRectangle(
	      cairo
	    , r.x 
	    , r.y
	    , r.width 
	    , r.height
	    , 2.
	) ;

	cairo->fill (); 
	cairo->restore () ;

	if( m_volume == 0 )
	{
	    cairo->set_operator( Cairo::OPERATOR_OVER ) ;

	    cairo->set_source(
		  m_image_mute 
		, a.get_width() - 4 - m_image_mute->get_width()
		, 2
	    ) ;

	    cairo->rectangle(
		  a.get_width() - 4 - m_image_mute->get_width()
		, 2
		, m_image_mute->get_width()
		, m_image_mute->get_height()
	    ) ;
	    cairo->fill() ;
	}
	else
	{
	    const int text_size_px = 13 ;
	    const int text_size_pt = static_cast<int> ((text_size_px * 72) / Util::screen_get_y_resolution (Gdk::Screen::get_default ())) ;
	    Pango::FontDescription font_desc = get_style()->get_font() ; 
	    font_desc.set_size(text_size_pt * PANGO_SCALE) ;
	    font_desc.set_weight(Pango::WEIGHT_BOLD) ;
	    Glib::RefPtr<Pango::Layout> layout = Glib::wrap (pango_cairo_create_layout (cairo->cobj ())) ;
	    layout->set_font_description(font_desc) ;

	    layout->set_text(
		(boost::format("%d") % m_volume).str()
	    ) ;

	    Pango::Rectangle rl, ri ;
	    layout->get_extents( rl, ri ) ;

	    int xoff = 0 ;

	    if( r.width-6 < (ri.get_width()/PANGO_SCALE))
	    {
		xoff = r.width ;

		cairo->set_source_rgba(
		      c1.get_red_p()
		    , c1.get_green_p()
		    , c1.get_blue_p()
		    , 1. 
		) ;
	    }
	    else
	    {
		cairo->set_source_rgba(
		      ct.r
		    , ct.g
		    , ct.b
		    , ct.a
		) ;
	    } 

	    int x = 3 ;
	    int y = (get_height() - (ri.get_height()/PANGO_SCALE))/2. ;

	    if( xoff == 0 )
	    {
		x = r.width - (ri.get_width() / PANGO_SCALE) - 3 ; 
	    }
	    else
	    {
		x = xoff + 4 ;
	    }

	    cairo->move_to(
		  x 
		, y
	    ) ;

	    cairo->set_operator( Cairo::OPERATOR_OVER ) ;
	    pango_cairo_show_layout (cairo->cobj (), layout->gobj ()) ;
	}

        return true ;
    }

    bool
    KoboVolume::on_leave_notify_event(
        GdkEventCrossing* G_GNUC_UNUSED
    )
    {
        return true ;
    }

    bool
    KoboVolume::on_enter_notify_event(
        GdkEventCrossing*
    )
    {
        return true ;
    }

    bool
    KoboVolume::on_button_press_event(
        GdkEventButton* event
    ) 
    {
        if( event->button == 1 ) 
        {
            grab_focus() ;
            m_clicked = true ;
            m_volume = event->x ; 
            m_volume = std::max( m_volume, 0 ) ;
            m_volume = std::min( m_volume, 100 ) ;
            m_SIGNAL_set_volume.emit( m_volume ) ;
            queue_draw () ;
        }

        return true ;
    }

    bool
    KoboVolume::on_button_release_event(
        GdkEventButton* event
    )
    {
        m_clicked = false ;

        m_volume = event->x ; 
        m_volume = std::max( m_volume, 0 ) ;
        m_volume = std::min( m_volume, 100 ) ;

        m_SIGNAL_set_volume.emit( m_volume ) ;

        queue_draw () ;

        return true ;
    }

    bool
    KoboVolume::on_motion_notify_event(
        GdkEventMotion* event
    )
    {
        if( m_clicked )
        {
            int x_orig, y_orig;
            GdkModifierType state;

            if (event->is_hint)
            {
                gdk_window_get_pointer (event->window, &x_orig, &y_orig, &state);
            }
            else
            {
                x_orig = int (event->x);
                y_orig = int (event->y);
                state = GdkModifierType (event->state);
            }

            m_volume = x_orig ;
            m_volume = std::max( m_volume, 0 ) ;
            m_volume = std::min( m_volume, 100 ) ;

            m_SIGNAL_set_volume.emit( m_volume ) ;

            queue_draw () ;
        }

        return true ;
    }

    void
    KoboVolume::vol_down()
    {
        m_volume -= 5 ; 
        m_volume = std::max( std::min( m_volume, 100), 0 ) ;
        m_SIGNAL_set_volume.emit( m_volume ) ;
        queue_draw () ;
    }

    void
    KoboVolume::vol_up()
    {
        m_volume += 5 ; 
        m_volume = std::max( std::min( m_volume, 100), 0 ) ;
        m_SIGNAL_set_volume.emit( m_volume ) ;
        queue_draw () ;
    }

    bool
    KoboVolume::on_scroll_event(
        GdkEventScroll* event
    ) 
    {
        if( event->direction == GDK_SCROLL_DOWN )
        {
            vol_down () ;
        }
        else
        if( event->direction == GDK_SCROLL_UP )
        {
            vol_up () ;
        }

        return true ;
    }

    bool
    KoboVolume::on_key_press_event(
        GdkEventKey* event
    )
    {
        switch( event->keyval )
        {
            case GDK_Left:
            case GDK_KP_Left:
            case GDK_Down:
            case GDK_KP_Down:

                vol_down () ;

                return true ;

            case GDK_Up:
            case GDK_KP_Up:
            case GDK_Right:
            case GDK_KP_Right:

                vol_up () ;

                return true ;

            default: break;
        }

        return false ;
    }

}
