#include "config.h"

#include <glibmm/i18n.h>
#include <cmath>
#include <boost/format.hpp>

#include "mpx/util-graphics.hh"
#include "mpx/widgets/cairo-extensions.hh"
#include "mpx/i-youki-play.hh"
#include "mpx/i-youki-theme-engine.hh"
#include "mpx/mpx-main.hh"

#include "youki-spectrum-titleinfo.hh"

namespace
{
    const double rounding = 3 ; 
}

namespace MPX
{
    void
    YoukiSpectrumTitleinfo::clear()
    {
        m_info.clear() ;
	m_audio_bitrate.reset() ;
	m_audio_codec.reset() ;
        m_cover.reset() ;
        queue_draw() ;
    }

    void
    YoukiSpectrumTitleinfo::set_info(
          const std::vector<std::string>& i
    )
    {
        m_info = i ;
	queue_draw() ;
    }

    void
    YoukiSpectrumTitleinfo::set_cover(
          Glib::RefPtr<Gdk::Pixbuf> cover
    )
    {
	if( cover )
        {
		m_cover = cover->scale_simple( 105, 105, Gdk::INTERP_HYPER ) ;
	}
	else
	{
		m_cover = cover ; 
	}

	queue_draw() ;
    }

    ///////////////////////////////////

    YoukiSpectrumTitleinfo::YoukiSpectrumTitleinfo(
    )
    : m_cursor_inside( false )
    {
        add_events( Gdk::EventMask(Gdk::BUTTON_PRESS_MASK | Gdk::ENTER_NOTIFY_MASK | Gdk::LEAVE_NOTIFY_MASK )) ;
        set_size_request( -1, 111 ) ;
        m_theme = services->get<IYoukiThemeEngine>("mpx-service-theme") ;
        const ThemeColor& c_bg   = m_theme->get_color( THEME_COLOR_BACKGROUND ) ; // all hail to the C-Base!
	override_background_color(c_bg) ;
    }

    bool
    YoukiSpectrumTitleinfo::on_button_press_event(
        GdkEventButton* event 
    )
    {
	if( event->button == 1 )
	{
		guint w = get_allocation().get_width() ;
		guint l = 0 ; 
		guint c = w/3 ; 
		guint r = 2*w/3 ;

		TapArea area ;

		guint x = event->x ;

		if( x >= l && x < c ) 
		{
		    area = TAP_LEFT ;
		}
		else
		if( x >= c && x < r )
		{
		    area = TAP_CENTER ;
		}
		else
		{
		    area = TAP_RIGHT ;
		}

		m_SIGNAL__area_tapped.emit( area ) ;
	} 

        return true ;
    }

    bool
    YoukiSpectrumTitleinfo::on_draw(
	const Cairo::RefPtr<Cairo::Context>& cairo
    )
    {
	const Gtk::Allocation& a = get_allocation() ;

        draw_background( cairo ) ;
        draw_titleinfo( cairo ) ;
        draw_cover( cairo ) ;

#if 0
        GdkRectangle r ;
        r.x = 2 ;
        r.y = 5 ;
        r.width = a.get_width() - 6 ; 
        r.height = a.get_height() - 8 ; 

	cairo->save() ;
	cairo->rectangle(
	      2
	    , 4 
	    , 103 
	    , 105 
	) ;
	cairo->clip() ;
	RoundedRectangle(
	      cairo
	    , r.x
	    , r.y
	    , r.width
	    , r.height
	    , rounding
	) ;
	cairo->set_line_width(1) ;
	cairo->set_source_rgba(.45,.45,.45, 1) ;
	cairo->stroke() ;
	cairo->restore() ;
#endif

	if( m_audio_bitrate || m_audio_codec )
	{
		std::string audioinfo ;

		cairo->save() ;
		cairo->set_operator( Cairo::OPERATOR_OVER ) ;

		int text_size_pt = static_cast<int>((13 * 72) / Util::screen_get_y_resolution( Gdk::Screen::get_default())) ;

		Pango::FontDescription font_desc = get_style_context()->get_font() ;
		font_desc.set_size( text_size_pt * PANGO_SCALE ) ;

		Glib::RefPtr<Pango::Layout> layout = Glib::wrap( pango_cairo_create_layout( cairo->cobj() )) ;
		layout->set_font_description( font_desc ) ;

		int width, height;

		if( m_audio_bitrate ) 
		{
		    audioinfo += (boost::format("Bitrate <b>%u kbps</b>") % m_audio_bitrate.get()).str() ;
		}

		if( m_audio_codec )
		{
		    std::string transformed ;

		    if( m_audio_codec.get() == "MPEG 1 Audio, Layer 3 (MP3)" )
			    transformed = "MP3" ;
		    else if( m_audio_codec.get() == "MPEG-4 AAC" )
			    transformed = "AAC" ;
		    else
			    transformed = m_audio_codec.get() ;

		    audioinfo += (boost::format(" Codec <b>%s</b>") % transformed).str() ;
		}

		layout->set_markup(audioinfo) ;
		layout->get_pixel_size( width, height ) ;
		cairo->set_source_rgba(
		      0.85 
		    , 0.85 
		    , 0.85 
		    , 1.
		) ; 

		cairo->move_to(
		      a.get_width() - width - 14 - (m_cover ? 100 : 0)
		    , 10 
		) ;

		pango_cairo_show_layout( cairo->cobj(), layout->gobj() ) ;
		cairo->restore() ;
	}

	return true ;
    }

    void
    YoukiSpectrumTitleinfo::draw_background(
          const Cairo::RefPtr<Cairo::Context>& cairo
    )
    {
        cairo->save() ;

        const Gtk::Allocation& a = get_allocation ();

        GdkRectangle r ;
        r.x = 1 ;
        r.y = 5 ;
        r.width = a.get_width() - 5 ; 
        r.height = a.get_height() - 8 ; 

        cairo->set_operator( Cairo::OPERATOR_OVER ) ;

        Gdk::RGBA cgdk ;
        double h, s, b ;

	cgdk.set_rgba( 0.25, 0.25, 0.25, 1.0 ) ;

        Cairo::RefPtr<Cairo::LinearGradient> gradient = Cairo::LinearGradient::create(
              r.x + r.width / 2
            , r.y  
            , r.x + r.width / 2
            , r.y + r.height
        ) ;

        double alpha = 1. ; 
        
        Util::color_to_hsb( cgdk, h, s, b ) ;
        b *= 1.05 ; 
        s *= 0.55 ; 
        Gdk::RGBA c1 = Util::color_from_hsb( h, s, b ) ;

        Util::color_to_hsb( cgdk, h, s, b ) ;
        s *= 0.55 ; 
        Gdk::RGBA c2 = Util::color_from_hsb( h, s, b ) ;

        Util::color_to_hsb( cgdk, h, s, b ) ;
        b *= 0.9 ; 
        s *= 0.60 ;
        Gdk::RGBA c3 = Util::color_from_hsb( h, s, b ) ;

        gradient->add_color_stop_rgba(
              0
            , c1.get_red()
            , c1.get_green()
            , c1.get_blue()
            , alpha / 1.05
        ) ;
        gradient->add_color_stop_rgba(
              .20
            , c2.get_red()
            , c2.get_green()
            , c2.get_blue()
            , alpha / 1.05
        ) ;
        gradient->add_color_stop_rgba(
              1. 
            , c3.get_red()
            , c3.get_green()
            , c3.get_blue()
            , alpha
        ) ;
        cairo->set_source( gradient ) ;
        cairo->set_operator( Cairo::OPERATOR_OVER ) ;
        RoundedRectangle(
              cairo
            , r.x 
            , r.y - 1 
            , r.width 
            , r.height + 2 
            , rounding 
        ) ;

	cairo->fill(); 
        cairo->restore() ;
    }

    void
    YoukiSpectrumTitleinfo::draw_titleinfo(
          const Cairo::RefPtr<Cairo::Context>& cairo
    )
    {
        if( m_info.empty())
            return ;

        cairo->save() ;
        cairo->set_operator( Cairo::OPERATOR_OVER ) ;
        const ThemeColor& c_text = m_theme->get_color( THEME_COLOR_TEXT_SELECTED ) ; 

        int text_size_pt = static_cast<int>((15 * 72)/Util::screen_get_y_resolution( Gdk::Screen::get_default() )) ;
        int width, height;

        Pango::FontDescription font_desc = get_style_context()->get_font() ;
        font_desc.set_size( text_size_pt * PANGO_SCALE ) ;
        font_desc.set_weight( Pango::WEIGHT_BOLD ) ;

        Glib::RefPtr<Pango::Layout> layout = Glib::wrap( pango_cairo_create_layout( cairo->cobj() )) ;
        layout->set_font_description( font_desc ) ;
        layout->set_text( m_info[0] ) ;
        layout->get_pixel_size( width, height ) ;

        cairo->move_to(
              8
            , 9
        ) ;

        Gdk::Cairo::set_source_rgba(cairo, c_text);
        pango_cairo_show_layout( cairo->cobj(), layout->gobj() ) ;

        cairo->move_to(
              8
            , 9 + height + 1
        ) ;

        layout->set_text( m_info[1] ) ;
        layout->get_pixel_size( width, height ) ;

        cairo->set_source_rgba(c_text.get_red(), c_text.get_green(), c_text.get_blue(), 0.60);
        pango_cairo_show_layout( cairo->cobj(), layout->gobj() ) ;

        text_size_pt = static_cast<int>( (20 * 72) / Util::screen_get_y_resolution( Gdk::Screen::get_default() )) ;

        font_desc = get_style_context()->get_font() ;
        font_desc.set_size( text_size_pt * PANGO_SCALE ) ;
        font_desc.set_weight( Pango::WEIGHT_BOLD ) ;

        layout->set_font_description( font_desc ) ;
        layout->set_text( m_info[2] ) ;
        layout->get_pixel_size( width, height ) ;

        cairo->move_to(
              8
            , 112 - height - 9 
        ) ;

        Gdk::Cairo::set_source_rgba(cairo, c_text);
        pango_cairo_show_layout( cairo->cobj(), layout->gobj() ) ;

        cairo->restore() ;
    }

    void
    YoukiSpectrumTitleinfo::draw_cover(
          const Cairo::RefPtr<Cairo::Context>& cairo
    )
    {
	if( !m_cover )
	    return ;

        cairo->save() ;

	cairo->set_operator( Cairo::OPERATOR_OVER ) ;

	GdkRectangle r ;

	r.x = get_allocated_width() - 108 ; 
	r.y = 5 ;
	r.width = 103 ; 
	r.height = 103 ; 

	Gdk::Cairo::set_source_pixbuf(
	      cairo
	    , m_cover
	    , r.x
	    , r.y
	) ;
	RoundedRectangle(
	      cairo
	    , r.x 
	    , r.y 
	    , r.width 
	    , r.height 
	    , rounding 
	    , MPX::CairoCorners::CORNERS(10)
	) ;
	cairo->fill() ;

	RoundedRectangle(
	      cairo
	    , r.x 
	    , r.y
	    , r.width
	    , r.height 
	    , rounding
	    , MPX::CairoCorners::CORNERS(5)
	) ;
	Cairo::RefPtr<Cairo::LinearGradient> gradient = Cairo::LinearGradient::create(
	      r.x + r.width/2.
	    , r.y 
	    , r.x + r.width/2. 
	    , r.y + r.height 
	) ;
	gradient->add_color_stop_rgba(
	      0
	    , 1. 
	    , 1. 
	    , 1. 
	    , 0.2
	) ;
	gradient->add_color_stop_rgba(
	      .6 
	    , 1. 
	    , 1. 
	    , 1. 
	    , 0.1 
	) ;
	gradient->add_color_stop_rgba(
	      .88 
	    , 1. 
	    , 1. 
	    , 1. 
	    , 0.1 
	) ;
	gradient->add_color_stop_rgba(
	      1. 
	    , 1. 
	    , 1. 
	    , 1. 
	    , 0.2 
	) ;
	cairo->set_source(gradient) ;
	cairo->fill() ;

        cairo->restore() ;
    }
}
