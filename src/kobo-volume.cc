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

    Gdk::RGBA
    get_color_at_pos(
          const Gdk::RGBA&     c1
        , const Gdk::RGBA&     c2
        , const double          ratio
    )
    {
        Gdk::RGBA c ;

        double r = ( c1.get_red()   * ( 1 - ratio ) + c2.get_red()   * ratio ) ;
        double g = ( c1.get_green() * ( 1 - ratio ) + c2.get_green() * ratio ) ;
        double b = ( c1.get_blue()  * ( 1 - ratio ) + c2.get_blue()  * ratio ) ;

        c.set_rgba( r, g, b ) ;

        return c ;
    }
}

namespace MPX
{
    KoboVolume::KoboVolume()
        : m_clicked( false )
        , m_volume( 0 )
    {
	m_posv.push_back( m_volume ); // ha, let's play!

        add_events(Gdk::EventMask(Gdk::LEAVE_NOTIFY_MASK | Gdk::ENTER_NOTIFY_MASK | Gdk::POINTER_MOTION_MASK | Gdk::POINTER_MOTION_HINT_MASK | Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK )) ;

        set_can_focus(false) ;

        m_theme = services->get<IYoukiThemeEngine>("mpx-service-theme").get() ;

        const ThemeColor& c = m_theme->get_color( THEME_COLOR_BASE ) ;

        Gdk::RGBA cgdk ;
        cgdk.set_rgba( c.get_red(), c.get_green(), c.get_blue() ) ;
        override_background_color( cgdk, Gtk::STATE_FLAG_NORMAL ) ;

        m_image_mute = Util::cairo_image_surface_from_pixbuf( Gdk::Pixbuf::create_from_file( Glib::build_filename( DATA_DIR, "images" G_DIR_SEPARATOR_S "mute-small-14x14px.png" ))) ;
    }

    KoboVolume::~KoboVolume ()
    {
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
    KoboVolume::on_draw(
	const Cairo::RefPtr<Cairo::Context>& cairo
    )
    {
        const Gdk::Rectangle& a = get_allocation() ;
	const guint w = a.get_width() - 2 ;

        const ThemeColor& c_base /* :) */ = m_theme->get_color( THEME_COLOR_BACKGROUND ) ; 
        const ThemeColor& ct = m_theme->get_color( THEME_COLOR_TEXT_SELECTED ) ;

        double percent = m_volume / 100. ; 

	GdkRectangle r ;
	double h, s, b ;

        Gdk::RGBA cgdk, c1, c2 ;

	cgdk.set_rgba( 0.55, 0.55, 0.55 ) ;
	
	Util::color_to_hsb( cgdk, h, s, b ) ;
	cgdk = Util::color_from_hsb( h, s, b ) ;

	Util::color_to_hsb( cgdk, h, s, b ) ;
	b *= 0.95 ;
	c1 = Util::color_from_hsb( h, s, b ) ;

	Util::color_to_hsb( cgdk, h, s, b ) ;
	b *= 0.45 ;
	c2 = Util::color_from_hsb( h, s, b ) ;

	Gdk::RGBA c_gradient = get_color_at_pos( c1, c2, percent ) ;

        cairo->set_operator(Cairo::OPERATOR_SOURCE) ;
        Gdk::Cairo::set_source_rgba(cairo, c_base);
        cairo->paint () ;

        cairo->set_operator( Cairo::OPERATOR_OVER ) ;
        RoundedRectangle(
              cairo
            , 1 
            , 1 
            , w 
            , 17
            , 2.
        ) ;

        Cairo::RefPtr<Cairo::LinearGradient> position_bar_back_gradient = Cairo::LinearGradient::create(
              a.get_width() / 2 
            , 1 
            , a.get_width() / 2 
            , 17
        ) ;

        position_bar_back_gradient->add_color_stop_rgba(
              0. 
            , cgdk.get_red()
            , cgdk.get_green()
            , cgdk.get_blue()
            , 0.3
        ) ;

        position_bar_back_gradient->add_color_stop_rgba(
              .4
            , cgdk.get_red()
            , cgdk.get_green()
            , cgdk.get_blue()
            , 0.22
        ) ;

        position_bar_back_gradient->add_color_stop_rgba(
              .6
            , cgdk.get_red()
            , cgdk.get_green()
            , cgdk.get_blue()
            , 0.22
        ) ;
        
        position_bar_back_gradient->add_color_stop_rgba(
              1. 
            , cgdk.get_red()
            , cgdk.get_green()
            , cgdk.get_blue()
            , 0.3
        ) ;

        cairo->set_source( position_bar_back_gradient ) ;
        RoundedRectangle(
              cairo
            , 1 
            , 1 
            , w
            , 17
            , 2.
        ) ;
        cairo->fill_preserve () ;
	cairo->set_source_rgba(
	      c2.get_red()
	    , c2.get_green()
	    , c2.get_blue()
	    , 0.3 
	) ;
        cairo->set_line_width( 0.75 ) ; 
        cairo->stroke_preserve() ;
        cairo->clip() ;

	/// VOLUME
	r.x         = 1 ; 
	r.y         = 1 ; 
	r.width     = fmax( 0, (a.get_width()-2) * double(percent)) ;
	r.height    = 17 ; 

	cairo->save() ;
	cairo->set_source_rgba(
	      c_gradient.get_red()
	    , c_gradient.get_green()
	    , c_gradient.get_blue()
	    , 1. 
	) ;
	cairo->set_operator(
	      Cairo::OPERATOR_ATOP
	) ;
	cairo->rectangle(
	      r.x 
	    , r.y
	    , r.width 
	    , r.height
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
	    cairo->paint_with_alpha(0.75) ;
	}
	else
	{
	    const int text_size_px = 14 ;
	    const int text_size_pt = static_cast<int> ((text_size_px * 72) / Util::screen_get_y_resolution (Gdk::Screen::get_default ())) ;
	    Pango::FontDescription font_desc = get_style_context()->get_font() ; 
	    font_desc.set_size(text_size_pt * PANGO_SCALE) ;
	    font_desc.set_weight(Pango::WEIGHT_BOLD) ;
	    Glib::RefPtr<Pango::Layout> layout = Glib::wrap (pango_cairo_create_layout (cairo->cobj ())) ;
	    layout->set_font_description(font_desc) ;

	    layout->set_text(
		(boost::format("%d") % m_volume).str()
	    ) ;

	    Pango::Rectangle rl, ri ;

	    layout->get_extents( rl, ri ) ;

	    if( r.width < ((ri.get_width() / PANGO_SCALE)+6)) 
	    {
		GdkRectangle r1 ;

		r1.width  = ri.get_width() / PANGO_SCALE ;
		r1.height = ri.get_height() / PANGO_SCALE ;
		r1.y      = (a.get_height() - r1.height) / 2 ; 
		r1.x      = 3 ; 

		cairo->rectangle( 1, 1, w * percent, 17 ) ; 
		cairo->clip() ;

		cairo->set_source_rgba(
		      ct.get_red()
		    , ct.get_green()
		    , ct.get_blue()
		    , 0.95
		) ;

		cairo->move_to(
		      r1.x                  
		    , r1.y 
		) ;

		pango_cairo_show_layout( cairo->cobj(), layout->gobj() ) ;

		cairo->reset_clip() ;

		cairo->rectangle( 1+w*percent, 1, 1+(2*(ri.get_width()/PANGO_SCALE)), 17 ) ; 
		cairo->clip() ;

		cairo->set_source_rgba(
		      c1.get_red()
		    , c1.get_green()
		    , c1.get_blue()
		    , 0.95
		) ;

		cairo->move_to(
		      r1.x                  
		    , r1.y 
		) ;

		pango_cairo_show_layout( cairo->cobj(), layout->gobj() ) ;

		cairo->reset_clip() ;
	    }
	    else
	    {
		GdkRectangle r1 ;

		r1.width  = ri.get_width() / PANGO_SCALE ;
		r1.height = ri.get_height() / PANGO_SCALE ;
		r1.y      = (a.get_height() - r1.height) / 2 ; 
		r1.x      = r.width - r1.width - 2 ;

		cairo->move_to(
		      r1.x                  
		    , r1.y 
		) ;

		cairo->set_source_rgba(
		      ct.get_red()
		    , ct.get_green()
		    , ct.get_blue()
		    , 0.95
		) ;
		pango_cairo_show_layout( cairo->cobj(), layout->gobj() ) ;
	    }
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

	    std::size_t new_volume = std::max<std::size_t>( 0, std::min<std::size_t>( 100, (event->x) / 2 )) ;

	    if( new_volume != m_volume )
	    {
		m_volume = new_volume ; 

/*
		m_posv.push_back( m_volume ) ;
		m_timer.stop() ;
		m_timer.reset() ;
		m_timer.start() ;
*/

		m_SIGNAL_set_volume.emit( m_volume ) ;
		queue_draw () ;
	    }
        }

        return true ;
    }

    bool
    KoboVolume::on_button_release_event(
        GdkEventButton* event
    )
    {
        m_clicked = false ;

/*
	m_timer.stop() ;

	guint snap = m_volume ;

	if( m_timer.elapsed() > 0.6 ) 
	{
	    guint tenth = m_volume / guint(10) ;
	    guint   mod = m_volume % 10 ;

	    if( mod < 5 ) 
		snap = std::min<guint>( tenth * 10, 100 ) ;
	    else if( mod == 5 )
		snap = m_volume ;
	    else
		snap = std::min<guint>( tenth * 10 + 10,  100 ) ;

	    if( snap != m_volume )
	    {
		m_volume = snap ;
		m_SIGNAL_set_volume.emit( m_volume ) ;
		queue_draw() ;
	    }
	}
*/

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

            if( event->is_hint )
            {
                gdk_window_get_pointer( event->window, &x_orig, &y_orig, &state ) ;
            }
            else
            {
                x_orig = int(event->x);
                y_orig = int(event->y);

                state = GdkModifierType(event->state);
            } 

	    std::size_t new_volume = std::max( 0, std::min( 100, (x_orig) / 2 )) ;

	    if( new_volume != m_volume )
	    {
		m_volume = new_volume ; 
//		m_timer.reset() ;
		m_SIGNAL_set_volume.emit( m_volume ) ;
		queue_draw () ;
	    }
        }

        return true ;
    }

    void
    KoboVolume::vol_down()
    {
        m_volume -= 5 ; 
        m_volume = std::max<std::size_t>( std::min<std::size_t>( m_volume, 100), 0 ) ;
        m_SIGNAL_set_volume.emit( m_volume ) ;
        queue_draw () ;
    }

    void
    KoboVolume::vol_up()
    {
        m_volume += 5 ; 
        m_volume = std::max<std::size_t>( std::min<std::size_t>( m_volume, 100), 0 ) ;
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
            case GDK_KEY_Left:
            case GDK_KEY_KP_Left:
            case GDK_KEY_Down:
            case GDK_KEY_KP_Down:

                vol_down () ;

                return true ;

            case GDK_KEY_Up:
            case GDK_KEY_KP_Up:
            case GDK_KEY_Right:
            case GDK_KEY_KP_Right:

                vol_up () ;

                return true ;

            default: break;
        }

        return false ;
    }

}
