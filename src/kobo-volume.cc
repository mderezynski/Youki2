#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "kobo-volume.hh"
#include "mpx/i-youki-theme-engine.hh"

#include <gtkmm.h>
#include <glibmm/i18n.h>
#include <cmath>

#include <boost/format.hpp>

#include "mpx/algorithm/colorfade.hh"

#include "mpx/widgets/cairo-extensions.hh"
#include "mpx/widgets/cairo-blur.hh"

#include "mpx/util-graphics.hh"

#include "mpx/mpx-main.hh"

namespace
{
    const double rounding = 2 ;

    template <typename T>
    struct PangoScaleAdaptor
    {
	T operator()( const T& v )
	{
	    return v / PANGO_SCALE ;
	}
    } ;

    template <typename T>
    struct Center
    {
	T operator()( const T& a, const T& b )
	{
	    return (a-b) / 2 ;
	}
    } ;
}

namespace MPX
{
    KoboVolume::KoboVolume()
        : m_clicked( false )
        , m_volume( 0 )
    {
	m_posv.push_back( m_volume ); // ha, let's play!

        add_events(Gdk::EventMask(Gdk::LEAVE_NOTIFY_MASK | Gdk::ENTER_NOTIFY_MASK | Gdk::POINTER_MOTION_MASK | Gdk::POINTER_MOTION_HINT_MASK | Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::SCROLL_MASK )) ;

        set_can_focus(false) ;

        m_theme = services->get<IYoukiThemeEngine>("mpx-service-theme").get() ;

        const ThemeColor& c = m_theme->get_color( THEME_COLOR_BACKGROUND ) ;

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
        const Gdk::Rectangle&	a	    = get_allocation() ;
	const guint		w	    = a.get_width() - 2 ;
        const ThemeColor&	ct	    = m_theme->get_color( THEME_COLOR_TEXT_SELECTED ) ;
        const double		percent	    = m_volume / 100. ; 

	GdkRectangle r ;
	double h, s, b ;

        Gdk::RGBA cgdk ;

	cgdk.set_rgba( 0.25, 0.25, 0.25 ) ;
	
        Util::color_to_hsb( cgdk, h, s, b ) ;
        b *= 0.84 ; 
        s *= 0.92 ;
        Gdk::RGBA c1 = Util::color_from_hsb( h, s, b ) ;

        Util::color_to_hsb( cgdk, h, s, b ) ;
	b *= 0.9 ;
        s *= 0.92 ; 
        Gdk::RGBA c2 = Util::color_from_hsb( h, s, b ) ;

        Util::color_to_hsb( cgdk, h, s, b ) ;
        b *= 1 ;
        s *= 0.92 ; 
        Gdk::RGBA c3 = Util::color_from_hsb( h, s, b ) ;

	ColorFade F( c1, c2 ) ;

	Gdk::RGBA c_gradient = F( percent ) ; 

        Cairo::RefPtr<Cairo::LinearGradient> volume_bar_back_gradient = Cairo::LinearGradient::create(
              a.get_width() / 2 
            , 1 
            , a.get_width() / 2 
            , 21
        ) ;

	Cairo::RefPtr<Cairo::LinearGradient> volume_bar_gradient = Cairo::LinearGradient::create(
                  r.x + r.width / 2
                , r.y  
                , r.x + r.width / 2
                , r.y + r.height
	) ;

        volume_bar_back_gradient->add_color_stop_rgba(
              0. 
            , cgdk.get_red()
            , cgdk.get_green()
            , cgdk.get_blue()
            , 0.22
        ) ;

        volume_bar_back_gradient->add_color_stop_rgba(
              .5
            , cgdk.get_red()
            , cgdk.get_green()
            , cgdk.get_blue()
            , 0.3
        ) ;

        volume_bar_back_gradient->add_color_stop_rgba(
              1. 
            , cgdk.get_red()
            , cgdk.get_green()
            , cgdk.get_blue()
            , 0.3
        ) ;

        cairo->set_source( volume_bar_back_gradient ) ;
        RoundedRectangle(
              cairo
            , 1 
            , 1 
            , w
            , 21
            , rounding 
        ) ;
        cairo->fill() ;

	r.x      = 1 ; 
	r.y      = 1 ; 
	r.width  = fmax( 0, (a.get_width()-2) * double(percent)) ;
	r.height = 21 ; 

	cairo->save() ;
	volume_bar_gradient->add_color_stop_rgba(
	      0. 
	    , c3.get_red()
	    , c3.get_green()
	    , c3.get_blue()
	    , 1 // factor 
	) ;

	volume_bar_gradient->add_color_stop_rgba(
	      .6
	    , c2.get_red()
	    , c2.get_green()
	    , c2.get_blue()
	    , 1 // factor
	) ;
	
	volume_bar_gradient->add_color_stop_rgba(
	      1. 
	    , c1.get_red()
	    , c1.get_green()
	    , c1.get_blue()
	    , 1 // factor
	) ;

        RoundedRectangle(
              cairo
            , r.x 
            , r.y 
            , r.width
            , r.height
            , rounding 
        ) ;

	cairo->fill (); 
	cairo->restore () ;

	if( m_volume == 0 )
	{
	    cairo->set_operator( Cairo::OPERATOR_OVER ) ;

	    cairo->set_source(
		  m_image_mute 
		, a.get_width() - 4 - m_image_mute->get_width()
		, 4
	    ) ;

	    cairo->rectangle(
		  a.get_width() - 4 - m_image_mute->get_width()
		, 4
		, m_image_mute->get_width()
		, m_image_mute->get_height()
	    ) ;
	    cairo->paint_with_alpha(0.75) ;
	}
	else
	{
	    Pango::Rectangle rl, ri ;
	    const int text_size_px = 14 ;
	    const int text_size_pt = static_cast<int> ((text_size_px * 72) / Util::screen_get_y_resolution (Gdk::Screen::get_default ())) ;

	    PangoScaleAdaptor<double> PSA ;
	    Center<double> C ;

	    Glib::RefPtr<Pango::Layout> layout = Glib::wrap(pango_cairo_create_layout(cairo->cobj())) ;
	    Pango::FontDescription font_desc = get_style_context()->get_font() ; 

	    font_desc.set_size(text_size_pt * PANGO_SCALE) ;
	    font_desc.set_weight(Pango::WEIGHT_BOLD) ;

	    layout->set_font_description(font_desc) ;
	    layout->set_text((boost::format("%d") % m_volume).str()) ;
	    layout->get_extents( rl, ri ) ;

            // Render Text Shadow
            Cairo::RefPtr<Cairo::ImageSurface> s = Cairo::ImageSurface::create( Cairo::FORMAT_ARGB32, PSA(ri.get_width()), PSA(ri.get_height())) ;
            Cairo::RefPtr<Cairo::Context> c2 = Cairo::Context::create( s ) ;

            c2->set_operator( Cairo::OPERATOR_CLEAR ) ;
            c2->paint() ;

            c2->set_operator( Cairo::OPERATOR_OVER ) ;
            c2->set_source_rgba(
                      0.
                    , 0.
                    , 0.
                    , 0.65
            ) ;
            c2->move_to(
                      0
                    , 0
            ) ;
            pango_cairo_show_layout(
                  c2->cobj()
                , layout->gobj()
            ) ;

            Util::cairo_image_surface_blur( s, 1. ) ;

	    if( r.width < (PSA(ri.get_width())+6)) 
	    {
		GdkRectangle r1 ;

		r1.width  = PSA(ri.get_width()) ;
		r1.height = PSA(ri.get_height()) ;
		r1.y      = C(a.get_height(), r1.height) ; 
		r1.x      = 3 ; 

		cairo->rectangle( 1, 1, w * percent, 21 ) ; 
		cairo->clip() ;

		cairo->set_source( s, r1.x+1, r1.y+1 ) ;
		cairo->rectangle( r1.x+1, r1.y+1, r1.width, 21 ) ;
		cairo->set_operator( Cairo::OPERATOR_OVER ) ;
		cairo->fill() ;

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

		cairo->rectangle( 1+w*percent, 1, 1+(2*(r1.width)), 21 ) ; 
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
		pango_cairo_show_layout(cairo->cobj(), layout->gobj()) ;

		cairo->reset_clip() ;
	    }
	    else
	    {
		GdkRectangle r1 ;

		r1.width  = PSA(ri.get_width()) ; 
		r1.height = PSA(ri.get_height()) ; 
		r1.y      = C(a.get_height(), r1.height) ; 
		r1.x      = r.width - r1.width - 2 ;

		cairo->set_source( s, r1.x+1, r1.y+1 ) ;
		cairo->rectangle( r1.x+1, r1.y+1, r1.width, 21 ) ;
		cairo->set_operator( Cairo::OPERATOR_OVER ) ;
		cairo->fill() ;

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
		pango_cairo_show_layout(cairo->cobj(),layout->gobj()) ;
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

	    guint new_volume = std::max<guint>( 0, std::min<guint>( 100, (event->x) / 2 )) ;

	    if( new_volume != m_volume )
	    {
		m_volume = new_volume ; 
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
                gdk_window_get_device_position( event->window, event->device, &x_orig, &y_orig, &state ) ;
            }
            else
            {
                x_orig = int(event->x);
                y_orig = int(event->y);

                state = GdkModifierType(event->state);
            } 

	    guint new_volume = std::max( 0, std::min( 100, (x_orig) / 2 )) ;

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
        m_volume = std::max<int>( std::min<int>( m_volume-5, 100), 0 ) ;
        m_SIGNAL_set_volume.emit( m_volume ) ;
        queue_draw () ;
    }

    void
    KoboVolume::vol_up()
    {
        m_volume = std::max<int>( std::min<int>( m_volume+5, 100), 0 ) ;
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
