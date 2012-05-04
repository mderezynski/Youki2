
#include "kobo-position.hh"

#include <boost/format.hpp>
#include <cmath>

#include "mpx/mpx-main.hh"
#include "mpx/algorithm/limiter.hh"
#include "mpx/algorithm/interval.hh"
#include "mpx/widgets/cairo-extensions.hh"
#include "mpx/widgets/cairo-blur.hh"
#include "mpx/util-graphics.hh"

namespace
{
    MPX::ThemeColor
    fade_colors(const MPX::ThemeColor& c_a, const MPX::ThemeColor& c_b, double position)
    {
        MPX::ThemeColor r ;

        r.set_red   (c_a.get_red()   + (c_b.get_red()   - c_a.get_red())   * position);
        r.set_green (c_a.get_green() + (c_b.get_green() - c_a.get_green()) * position);
        r.set_blue  (c_a.get_blue()  + (c_b.get_blue()  - c_a.get_blue())  * position);

        return r ;
    }
}

namespace MPX
{
    inline double
    KoboPosition::cos_smooth (double x)
    {
        return (2.0 - std::cos (x * G_PI)) / 2.0;
    }

    double
    KoboPosition::get_position ()
    {
        return 0. ;
    }

    KoboPosition::KoboPosition(
    )

        : m_duration( 0 )
        , m_seek_position( 0 )
        , m_seek_factor( 0 )
        , m_clicked( false )
	, m_paused( false )

    {
        add_events(Gdk::EventMask(Gdk::LEAVE_NOTIFY_MASK | Gdk::ENTER_NOTIFY_MASK | Gdk::POINTER_MOTION_MASK | Gdk::POINTER_MOTION_HINT_MASK | Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK )) ;
        set_can_focus (false) ;

        m_theme = services->get<IYoukiThemeEngine>("mpx-service-theme").get() ;
    }

    KoboPosition::~KoboPosition () 
    {
    }

    void
    KoboPosition::stop()
    {
    }

    void
    KoboPosition::start()
    {
    }

    void
    KoboPosition::set_position(
          guint    duration
        , guint    position
    )
    {
        m_position = std::max<guint>( 0, position ) ;
        m_duration = std::max<guint>( 0, duration ) ;
        queue_draw () ;
    }

    bool
    KoboPosition::on_draw(
	const Cairo::RefPtr<Cairo::Context>& cairo
    )
    {
        const Gdk::Rectangle& a = get_allocation() ;
        const guint w = a.get_width() - 2 ;

        const guint position = m_clicked ? m_seek_position : m_position ;

        const ThemeColor& c = m_theme->get_color( THEME_COLOR_SELECT ) ;
        const ThemeColor& c_bg = m_theme->get_color( THEME_COLOR_BACKGROUND ) ; 
        const ThemeColor& ct = m_theme->get_color( THEME_COLOR_TEXT_SELECTED ) ;

        cairo->set_operator(Cairo::OPERATOR_SOURCE) ;
        Gdk::Cairo::set_source_rgba(cairo, c_bg);
        cairo->paint() ;

        cairo->set_operator( Cairo::OPERATOR_OVER ) ;

        Gdk::RGBA cgdk ;
        Gdk::RGBA c_text_dark, c1, c2, c3 ; 

        cgdk.set_rgba( c.get_red(), c.get_green(), c.get_blue(), 1.0);

	double h, s, b ;
	
	Util::color_to_hsb( cgdk, h, s, b ) ;
	b *= 0.85 ; 
	s *= 0.90 ;
	c_text_dark = Util::color_from_hsb( h, s, b ) ;

	Util::color_to_hsb( cgdk, h, s, b ) ;
	b *= 0.85 ; 
	c1 = Util::color_from_hsb( h, s, b ) ;

	Util::color_to_hsb( cgdk, h, s, b ) ;
	b *= 0.55 ; 
	c2 = Util::color_from_hsb( h, s, b ) ;

	Util::color_to_hsb( cgdk, h, s, b ) ;
	b *= 0.35 ; 
	c3 = Util::color_from_hsb( h, s, b ) ;

        // BAR BACKGROUND
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
            , 0.35
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
            , 0.35 
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
        cairo->fill_preserve() ;

        cairo->save() ;
        cairo->set_source_rgba( c.get_red() , c.get_green(), c.get_blue(), 0.5 ) ; 
        cairo->set_line_width( 0.75 ) ;
        cairo->stroke() ;
        cairo->restore() ;

        RoundedRectangle(
              cairo
            , 1 
            , 1 
            , w 
            , 17
            , 2.
        ) ;
	cairo->clip() ;

        // BAR
        double factor  = 1. ;
        double percent = double(position) / double(m_duration) ; 

	GdkRectangle r ;

        if( percent >= 0.90 )
        {
            factor = (1. - percent) * 10. ; 
        }

        if( percent > 0. )
	{
            r.x         = 1 ; 
            r.y         = 1 ; 
            r.width     = w * percent ; 
            r.height    = 17 ; 

            cairo->save () ;

            Cairo::RefPtr<Cairo::LinearGradient> position_bar_gradient = Cairo::LinearGradient::create(
                  r.x + r.width / 2
                , r.y  
                , r.x + r.width / 2
                , r.y + r.height
            ) ;

            position_bar_gradient->add_color_stop_rgba(
                  0. 
                , c1.get_red()
                , c1.get_green()
                , c1.get_blue()
                , 1 // factor 
            ) ;

            position_bar_gradient->add_color_stop_rgba(
                  .60
                , c2.get_red()
                , c2.get_green()
                , c2.get_blue()
                , 1 // factor
            ) ;
            
            position_bar_gradient->add_color_stop_rgba(
                  1. 
                , c3.get_red()
                , c3.get_green()
                , c3.get_blue()
                , 1 // factor
            ) ;

            cairo->set_source( position_bar_gradient ) ;
            cairo->rectangle(
                  r.x 
                , r.y
                , r.width 
                , r.height
            ) ;
            cairo->fill(); 
            cairo->restore() ;
        }

	Pango::Rectangle rl, ri ;

	Pango::FontDescription font_desc = get_style_context()->get_font() ;
	Glib::RefPtr<Pango::Layout> layout = Glib::wrap( pango_cairo_create_layout(cairo->cobj()) ) ;

	const int text_size_px = 14 ;
	const int text_size_pt = static_cast<int>((text_size_px * 72) / Util::screen_get_y_resolution(Gdk::Screen::get_default())) ;

	font_desc.set_size( text_size_pt * PANGO_SCALE ) ;
	layout->set_font_description( font_desc ) ;

        if( m_duration && m_position >= 0 ) // position 
        {
	    // Set layout
            layout->set_markup(
                (boost::format("<b>%02d</b>:<b>%02d</b>") % ( position / 60 ) % ( position % 60 )).str()
            ) ;
            layout->get_extents( rl, ri ) ; 

	    // Render text shadow
	    Cairo::RefPtr<Cairo::ImageSurface> s = Cairo::ImageSurface::create( Cairo::FORMAT_ARGB32, 100, 48 ) ; // FIXME: Our blur implementation doesn't like small surfaces
	    Cairo::RefPtr<Cairo::Context> c2 = Cairo::Context::create( s ) ;

	    c2->set_operator( Cairo::OPERATOR_CLEAR ) ;
	    c2->paint() ;

	    c2->set_operator( Cairo::OPERATOR_OVER ) ;
	    c2->set_source_rgba(
		      0. 
		    , 0. 
		    , 0.
		    , 0.40
	    ) ;
	    c2->move_to(
		      .5
		    , .5
	    ) ;
	    pango_cairo_show_layout(
		  c2->cobj()
		, layout->gobj()
	    ) ;

	    Util::cairo_image_surface_blur( s, 1.5 ) ;

	    double alpha = get_alpha_at_time() ;

	    // Render text, split in 2 parts if necessary
	    if( r.width-6 < (ri.get_width() / PANGO_SCALE)) 
	    {
		GdkRectangle r1 ;

		r1.width  = ri.get_width() / PANGO_SCALE ;
		r1.height = ri.get_height() / PANGO_SCALE ;
		r1.y      = (a.get_height() - r1.height) / 2 ; 
		r1.x      = 3 ; 

		cairo->rectangle( 1, 1, w * percent, 17 ) ; 
		cairo->clip() ;

		if( alpha ) 
		{
		    cairo->set_source( s, r1.x, r1.y ) ;
		    cairo->rectangle( r1.x, r1.y, ri.get_width()+1, 17 ) ;
		    cairo->set_operator( Cairo::OPERATOR_OVER ) ;
		    cairo->fill() ;
		}

		cairo->move_to(
		      r1.x                  
		    , r1.y 
		) ;
		cairo->set_source_rgba(
		      ct.get_red()
		    , ct.get_green()
		    , ct.get_blue()
		    , 0.95 * alpha 
		) ;
		pango_cairo_show_layout( cairo->cobj(), layout->gobj() ) ;

		cairo->reset_clip() ;
		cairo->rectangle( 1+w*percent, 1, 1+(2*(ri.get_width()/PANGO_SCALE)), 17 ) ; 
		cairo->clip() ;

		cairo->move_to(
		      r1.x                  
		    , r1.y 
		) ;
		cairo->set_source_rgba(
		      c_text_dark.get_red()
		    , c_text_dark.get_green()
		    , c_text_dark.get_blue()
		    , 0.95 * get_alpha_at_time()
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

		if( alpha )
		{
		    cairo->set_source( s, r1.x, r1.y ) ;
		    cairo->rectangle( r1.x, r1.y, ri.get_width()+1, 17 ) ;
		    cairo->set_operator( Cairo::OPERATOR_OVER ) ;
		    cairo->fill() ;
		}

		cairo->move_to(
		      r1.x                  
		    , r1.y 
		) ;
		cairo->set_source_rgba(
		      ct.get_red()
		    , ct.get_green()
		    , ct.get_blue()
		    , 0.95 * alpha
		) ;
		pango_cairo_show_layout( cairo->cobj(), layout->gobj() ) ;
	    }
	}

	if( m_duration ) // duration
	{
	    layout->set_markup(
		(boost::format("<b>%02d</b>:<b>%02d</b>") % ( m_duration / 60 ) % ( m_duration % 60 )).str()
	    ) ;

	    layout->get_extents( rl, ri ) ; 

	    r.width  = ri.get_width() / PANGO_SCALE ;
	    r.height = ri.get_height() / PANGO_SCALE ;
	    r.x      = 3 + double(a.get_width()) - r.width - 7 ;
	    r.y      = (a.get_height() - r.height) / 2 ; 

	    cairo->move_to(
		  r.x 
		, r.y
	    ) ;

	    cairo->set_source_rgba(
		  c_text_dark.get_red()
		, c_text_dark.get_green()
		, c_text_dark.get_blue()
		, 1. * factor
	    ) ;

	    pango_cairo_show_layout( cairo->cobj(), layout->gobj() ) ;
	}

        if( has_focus() )
        {
            r.x      = 1 ;
            r.y      = 1 ;
            r.width  = a.get_width() - 2 ;
            r.height = 17 ; 

            m_theme->draw_focus(
                  cairo
                , r 
                , is_sensitive()
                , 2.
            ) ;
        }

        return true ;
    }

    bool
    KoboPosition::on_leave_notify_event(
        GdkEventCrossing* G_GNUC_UNUSED
    )
    {
        return true ;
    }

    bool
    KoboPosition::on_enter_notify_event(
        GdkEventCrossing*
    )
    {
        return true ;
    }

    bool
    KoboPosition::on_button_press_event(
        GdkEventButton* event
    ) 
    {
        if( event->button == 1 )
        {
            grab_focus() ;
        
            const Gtk::Allocation& a = get_allocation() ;

            Interval<std::size_t> i (
                  Interval<std::size_t>::IN_IN
                , 0 
                , a.get_width()
            ) ;

            if( i.in( event->x ) ) 
            {
                m_clicked       = true ;
                m_seek_factor   = double(a.get_width()-2) / double(m_duration) ;

                Limiter<guint> p (
                      Limiter<guint>::ABS_ABS
                    , 0
                    , m_duration 
                    , event->x / m_seek_factor
                ) ;

                m_seek_position = p ; 

                queue_draw () ;
            }
        }

        return true ;
    }

    bool
    KoboPosition::on_motion_notify_event(
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

            const Gtk::Allocation& a = get_allocation() ;

            Interval<std::size_t> i (
                  Interval<std::size_t>::IN_IN
                , 0 
                , a.get_width()
            ) ;

            if( i.in( x_orig ))
            {
                Limiter<guint> p (
                      Limiter<guint>::ABS_ABS
                    , 0
                    , m_duration
                    , x_orig / m_seek_factor
                ) ;

                m_seek_position = p ; 
            }

            queue_draw () ;
        }

        return true ;
    }

    bool
    KoboPosition::on_button_release_event(
        GdkEventButton* G_GNUC_UNUSED
    )
    {
        m_position = m_seek_position ;
        m_clicked  = false ;

        m_SIGNAL_seek_event.emit( m_seek_position ) ;

        queue_draw () ;

        return true ;
    }

    bool
    KoboPosition::on_focus_in_event(
        GdkEventFocus* event
    )
    {
        queue_draw() ;
        return false ;
    }

    bool
    KoboPosition::on_key_press_event(
        GdkEventKey* event
    )
    {
        Limiter<guint> p ;

        switch( event->keyval )
        {
            case GDK_KEY_Down:
            case GDK_KEY_KP_Down:

                p = Limiter<guint> (
                      Limiter<guint>::ABS_ABS
                    , 0
                    , m_duration
                    , m_position - 5 
                ) ;

                m_seek_position = p ; 
                m_SIGNAL_seek_event.emit( m_seek_position ) ;

                return true ;

            case GDK_KEY_Up:
            case GDK_KEY_KP_Up:

                p = Limiter<guint> (
                      Limiter<guint>::ABS_ABS
                    , 0
                    , m_duration
                    , m_position + 5 
                ) ;

                m_seek_position = p ; 
                m_SIGNAL_seek_event.emit( m_seek_position ) ;

                return true ;

            default: break;
        }

        return false ;
    }
}
