//  MPX
//  Copyright (C) 2010 MPX development.
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
//  --
//
//  The MPX project hereby grants permission for non GPL-compatible GStreamer
//  plugins to be used and distributed together with GStreamer and MPX. This
//  permission is above and beyond the permissions granted by the GPL license
//  MPX is covered by.

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif //HAVE_CONFIG_H

#include "kobo-titleinfo.hh"

#include <glib/gi18n.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gdkmm/general.h>
#include <cairomm/cairomm.h>
#include <boost/format.hpp>
#include <cmath>

#include "mpx/util-graphics.hh"
#include "mpx/util-ui.hh"
#include "mpx/widgets/cairo-extensions.hh"
#include "mpx/widgets/cairo-blur.hh"
#include "mpx/i-youki-theme-engine.hh"

#include "mpx/mpx-main.hh"

namespace
{
    // Animation settings

    int const animation_fps = 24;
    int const animation_frame_period_ms = 1000 / animation_fps;

    int const    text_size_px           = 18 ;
    double const text_fade_in_time      = 0.05 ;
    double const text_fade_out_time     = 0.15 ;
    double const text_hold_time         = 2 ; 
    double const text_time              = text_fade_in_time + text_fade_out_time + text_hold_time;
    double const text_full_alpha        = 1. ;
    double const initial_delay          = 0.0 ;

}

namespace MPX
{
    inline double
    KoboTitleInfo::cos_smooth (double x)
    {
        return (1.0 - std::cos (x * G_PI)) / 2.0;
    }

    guint
    KoboTitleInfo::get_text_at_time(std::string& text)
    {
        if( !m_info.empty() )
        {
            guint line = std::fmod( ( m_current_time / text_time ), m_info.size() ) ;

	    if( line == 0 )
		text = m_info[2];
	    else
		return 3 ;

	    return (m_current_time/text_time) ;
        }
        else
        {
            text.clear() ; 
	    return 0 ;
        }
    }

    double
    KoboTitleInfo::get_text_alpha_at_time()
    {
        {
            double offset = m_tmod ; 

            if (offset < text_fade_in_time)
            {
                return text_full_alpha * cos_smooth (offset / text_fade_in_time);
            }
            else if (offset < text_fade_in_time + text_hold_time)
            {
                return text_full_alpha;
            }
            else
            {
                return text_full_alpha * cos_smooth (1.0 - (offset - text_fade_in_time - text_hold_time) / text_fade_out_time);
            }
        }
    }

    KoboTitleInfo::KoboTitleInfo()
    : m_tmod( m_current_time, text_time )
    {
        m_theme = services->get<IYoukiThemeEngine>("mpx-service-theme") ;

	set_app_paintable (true);
        add_events( Gdk::EventMask(Gdk::BUTTON_PRESS_MASK | Gdk::ENTER_NOTIFY_MASK | Gdk::LEAVE_NOTIFY_MASK )) ;

        m_timer.stop();
        m_timer.reset();
        set_size_request( -1, 38 ) ;
    }

    void
    KoboTitleInfo::clear()
    {
        m_info.clear() ;
        m_timer.stop() ;
        m_timer.reset() ;
	m_cover.reset() ;
	m_color.reset() ;
        m_update_connection.disconnect();
        queue_draw() ;
    }

    void
    KoboTitleInfo::set_info(
        const std::vector<std::string>& i
    )
    {
        m_info = i ;

        total_animation_time    = m_info.size() * text_time;
        start_time              = initial_delay;
        end_time                = start_time + total_animation_time;

        m_timer.reset() ;

        if( !m_update_connection )
        {
          m_timer.start();
          m_update_connection = Glib::signal_timeout().connect (sigc::mem_fun (this, &KoboTitleInfo::update_frame),
                                                                 animation_frame_period_ms);
        }
    }

    bool
    KoboTitleInfo::on_draw(const Cairo::RefPtr<Cairo::Context>& cairo)
    {
        const Gtk::Allocation& a = get_allocation() ;

        const ThemeColor& c_base = m_theme->get_color( THEME_COLOR_BACKGROUND ) ; // all hail to the C-Base!

        ThemeColor c_info ;

	if( m_color )
	    c_info = m_color.get() ;
	else
	    c_info = m_theme->get_color( THEME_COLOR_INFO_AREA ) ; 

        GdkRectangle r ;
        r.x = 2 ;
        r.y = 1 ;
        r.width = a.get_width() - 6 ;
        r.height = a.get_height() - 2 ; 

        cairo->set_operator(Cairo::OPERATOR_SOURCE) ;
        cairo->set_source_rgba(
              c_base.get_red()
            , c_base.get_green()
            , c_base.get_blue()
            , 1 
        ) ;
        cairo->paint() ;

        cairo->set_operator( Cairo::OPERATOR_OVER ) ;
        cairo->set_source_rgba(
              c_info.get_red() 
            , c_info.get_green()
            , c_info.get_blue()
            , 1 
        ) ;
        RoundedRectangle(
              cairo
            , r.x 
            , r.y
            , r.width 
            , r.height 
            , 2. 
        ) ;
        cairo->fill() ;

        Gdk::RGBA cgdk ;

	if( m_color )
	    cgdk = m_color.get() ;
	else
	    cgdk = Util::make_rgba( 0.25, 0.25, 0.25, 1 ) ; 

        Cairo::RefPtr<Cairo::LinearGradient> gradient = Cairo::LinearGradient::create(
              r.x + r.width / 2
            , r.y  
            , r.x + r.width / 2
            , r.y + r.height
        ) ;

        double h, s, b ;
    
        double alpha = 1. ;
        
        Util::color_to_hsb( cgdk, h, s, b ) ;
        b *= 0.80 ; 
        s *= 0.90 ;
        Gdk::RGBA c1 = Util::color_from_hsb( h, s, b ) ;

        Util::color_to_hsb( cgdk, h, s, b ) ;
	b *= 0.75 ;
        s *= 0.85 ;
        Gdk::RGBA c2 = Util::color_from_hsb( h, s, b ) ;

        Util::color_to_hsb( cgdk, h, s, b ) ;
        b *= 0.71 ;
        s *= 0.82 ;
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
              1 
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
            , r.y 
            , r.width 
            , r.height 
            , 2. 
        ) ;
        cairo->fill(); 

        cairo->set_source_rgba( 0.15, 0.15, 0.15, 1. ) ; 
        cairo->set_line_width( 0.75 ) ;
        RoundedRectangle(
              cairo
            , r.x 
            , r.y 
            , r.width 
            , r.height 
            , 2. 
        ) ;
        cairo->stroke_preserve() ;
	cairo->clip() ;

	if( m_cover )
	{
	    auto gradient = Cairo::LinearGradient::create(
		  (r.x + r.width - 300)
		, r.y + (r.height/2)
		, (r.x + r.width)
		, r.y + (r.height/2)
	    ) ;

	    gradient->add_color_stop_rgba(
		  0
		, 0 
		, 0 
		, 0 
		, 0 
	    ) ;
	    gradient->add_color_stop_rgba(
		  .23 
		, 0 
		, 0 
		, 0 
		, 1 
	    ) ;
	    gradient->add_color_stop_rgba(
		  1 
		, 0 
		, 0 
		, 0 
		, 1 
	    ) ;
	    RoundedRectangle( cairo, r.x+r.width-300, r.y, 300, 36, 2. ) ;
	    Gdk::Cairo::set_source_pixbuf( cairo, m_cover, r.x+r.width-300, r.y-124 ) ;
	    cairo->set_operator( Cairo::OPERATOR_OVER ) ;
	    cairo->mask(gradient) ;
	}

	if( !m_info.empty() && (m_info.size() == 3))
        {
            int text_size_pt = static_cast<int>( (text_size_px * 72) / Util::screen_get_y_resolution( Gdk::Screen::get_default() )) ;
            const ThemeColor& cgdk = m_theme->get_color( THEME_COLOR_TEXT_SELECTED ) ; 
            Pango::FontDescription font_desc = get_style_context()->get_font() ;

            m_current_time = m_timer.elapsed() ;

            std::string text ; 

	    guint   line = get_text_at_time( text ) ;
            double alpha = get_text_alpha_at_time() ;

            Glib::RefPtr<Pango::Layout> layout = Glib::wrap( pango_cairo_create_layout( cairo->cobj() )) ;

            font_desc.set_size( text_size_pt * PANGO_SCALE ) ;
            layout->set_font_description( font_desc ) ;

	    if( line > 2 )
	    {
		m_update_connection.disconnect() ;
		layout->set_markup((boost::format("<b>%s</b>  <small>•</small>  <b>%s</b>  <small>•</small>  %s")
		    % Glib::Markup::escape_text(m_info[2])
		    % Glib::Markup::escape_text(m_info[0])
		    % Glib::Markup::escape_text(m_info[1])
	    ).str()) ;
	    }
	    else
	    {
		layout->set_markup((boost::format("<b>%s</b>") % Glib::Markup::escape_text(text)).str()) ;
	    }

            int width, height;
            layout->get_pixel_size( width, height ) ;

	    cairo->save() ;
            cairo->set_operator( Cairo::OPERATOR_OVER ) ;
            cairo->translate(
                  (a.get_width()-width)/2.
                , (a.get_height()-height)/2. 
            ) ;

	    Util::render_text_shadow( layout, 1, 1, cairo, 2, ((line>2)?0.3:alpha/3.)) ;

	    cairo->move_to(0,0) ;
            cairo->set_source_rgba(
                  cgdk.get_red()
                , cgdk.get_green()
                , cgdk.get_blue() 
                , ((line>2)?1:alpha)
            ) ; 
            pango_cairo_show_layout(cairo->cobj(), layout->gobj()) ;

	    cairo->restore() ;
        }

	return true ;
    }

    bool
    KoboTitleInfo::update_frame()
    {
        queue_draw() ;
        return true;
    }

    bool
    KoboTitleInfo::on_button_press_event(
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

    void
    KoboTitleInfo::set_cover(
          Glib::RefPtr<Gdk::Pixbuf> cover
    )
    {
	if( cover )
        {
	    m_cover = cover->scale_simple( 300, 300, Gdk::INTERP_BILINEAR ) ;
	}
	else
	{
	    m_cover = cover ; 
	}

	queue_draw() ;
    }


} // namespace MPX
