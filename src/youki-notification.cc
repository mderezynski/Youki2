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

#include "config.h"

#include <gtkmm.h>
#include <glibmm/i18n.h>
#include <gdkmm.h>
#include <pangomm.h>
#include <cairomm/cairomm.h>
#include <boost/format.hpp>
#include <sigc++/sigc++.h>

#include <cmath>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <gdk/gdkx.h>

#include "mpx/util-graphics.hh"

#include "youki-notification.hh"

using namespace Glib ;
using namespace Gtk ;

namespace
{
    const int popup_width         = 480 ;
    const int popup_height        = 100 ;

    const int popup_animation_fps = 25 ;
    const int popup_animation_frame_period_ms = 1000 / popup_animation_fps ;

    const double popup_full_alpha    = 1.0 ;
    const double popup_fade_in_time  = 0.15 ;
    const double popup_fade_out_time = 0.30 ;
    const double popup_hold_time     = 5.0 ;

    const double popup_total_time_no_fade = popup_hold_time ;
    const double popup_total_time_fade    = popup_fade_in_time + popup_hold_time + popup_fade_out_time ;

    const int arrow_width  = 26 ;
    const int arrow_height = 8 ;

    inline double
        cos_smooth (double x)
    {
        return (1.0 - std::cos (x * G_PI)) / 2.0 ;
    }

    double
        get_popup_alpha_at_time(
              double    time
            , bool      fading = true
        )
    {
        if( fading )
        {
            if( time >= 0.0 && popup_total_time_fade )
            {
                if( time < popup_fade_in_time )
                {
                    return popup_full_alpha * cos_smooth (time / popup_fade_in_time) ;
                }
                else if( time < popup_fade_in_time + popup_hold_time )
                {
                    return popup_full_alpha ;
                }
                else
                {
                    time -= popup_fade_in_time + popup_hold_time ;
                    return popup_full_alpha * cos_smooth (1.0 - time / popup_fade_out_time) ;
                }
            }
        }
        else
        {
            if( time >= 0.0 && time < popup_total_time_no_fade )
                return popup_full_alpha ;
        }

        return 0.0 ;
    }

    inline double
        get_popup_end_time (bool fading = true)
    {
        return fading ? popup_total_time_fade : popup_total_time_no_fade ;
    }

    inline double
        get_popup_time_offset (bool fading = true)
    {
        return fading ? popup_fade_in_time : 0.0 ;
    }

    inline double
        get_popup_disappear_start_time (bool fading = true)
    {
        return fading ? popup_fade_in_time + popup_hold_time : get_popup_end_time (fading) ;
    }

    void
        window_set_opacity (Glib::RefPtr<Gdk::Window> const& window,
        double                           d)
    {
        const unsigned int opaque= 0xffffffff ;
        const char * opacity_prop = "_NET_WM_WINDOW_OPACITY" ;

        unsigned int opacity = (unsigned int) (d * opaque) ;

        ::Display * dpy = gdk_x11_display_get_xdisplay (gdk_display_get_default()) ;
        ::Window    win = GDK_WINDOW_XID (window->gobj()) ;

        if( opacity == opaque )
        {
            XDeleteProperty (dpy, win, XInternAtom (dpy, opacity_prop, False)) ;
        }
        else
        {
            XChangeProperty (dpy, win, XInternAtom (dpy, opacity_prop, False),
                XA_CARDINAL, 32, PropModeReplace,
                reinterpret_cast<unsigned char *> (&opacity), 1L) ;
            XSync (dpy, False) ;
        }
    }
}

namespace MPX
{
	Notification::Notification(
        GtkWidget* widget
    )
    : Window          (WINDOW_POPUP)
    , m_widget        (widget)
    , m_outline       (Gdk::Color ("#949494"))
    , m_inlay         (Gdk::Color ("#ffffff"))
    , m_image         (Glib::RefPtr<Gdk::Pixbuf>(0))
    , m_time_offset   (0.)
    , m_width         (popup_width)
    , m_height        (popup_height)
    , m_ax            (40)
    , m_x             (0)
    , m_y             (0)
    , m_has_alpha     (Gdk::Screen::get_default()->is_composited())
    , m_location      (ARROW_BOTTOM)
    , m_position      (ARROW_POS_DEFAULT)
    , m_tooltip_mode  (false)
	{
		if( m_has_alpha )
        {
            set_colormap (Gdk::Screen::get_default()->get_rgba_colormap()) ;
        }

		m_layout_1 = Pango::Layout::create (get_pango_context ()) ;
		m_layout_2 = Pango::Layout::create (get_pango_context ()) ;

		add_events( Gdk::ALL_EVENTS_MASK ) ;

		set_app_paintable( true )  ;
		set_resizable( false )  ;
		set_decorated( false )  ;
		set_size_request( m_width, m_height )  ;

		add( m_fixed ) ;

        Gdk::Color background ;
        background.set_rgb_p( 1., 1., 1. ) ; 

        m_kobo_position = new KoboPosition() ;

        Gdk::Color cgdk ;
        cgdk.set_rgb_p( 1., 1., 1. ) ; 
        m_kobo_position->modify_bg( Gtk::STATE_NORMAL, cgdk ) ;
        m_kobo_position->modify_base( Gtk::STATE_NORMAL, cgdk ) ;

		m_fixed.set_size_request( m_width, m_height )  ;
		m_kobo_position->set_size_request( m_width - 102, 18 )  ;
		m_fixed.put( *m_kobo_position, 94, 78 )  ;
		m_fixed.set_has_window( false )  ;
		m_fixed.show_all()  ;

		m_fade = Gdk::Screen::get_default()->is_composited()  ;
		m_timer.stop()  ;
		m_timer.reset()  ;
	}

	void
		Notification::reposition ()
	{
		int x, y, width, height ;
		acquire_widget_info (x, y, width, height) ;

		int new_x = x + width/2 ;
		int new_y = y - m_height ;

		if( m_x != new_x || m_y != new_y )
		{
			m_x = new_x ;
			m_y = new_y ;

			m_location = ARROW_BOTTOM ;
			m_position = ARROW_POS_DEFAULT ;
			m_ax = 40 ;

			if( m_y < 0 )
			{
				m_location = ARROW_TOP ;
				m_y = y + height ;
			}

			int screen_width = Gdk::Screen::get_default ()->get_width () ;

			if( m_x + m_width > screen_width )
			{
				m_ax = m_width - 40 ;
				m_x = (screen_width - m_width) - (screen_width - m_x - 40) ;

				if( m_x + m_width > screen_width )
				{
					m_position = ARROW_POS_RIGHT ;
					m_x -= (m_width - m_ax) ;
					m_ax  = 0 ;
				}
			}
			else
			{
				m_x -= 40 ;
				if( m_x < 0 )
				{
					m_position = ARROW_POS_LEFT ;
					m_ax  = 0 ;
					m_x  = 40 + m_x ;
				}
			}

			move (m_x, m_y) ;
			update_mask () ;
		}
	}

	void
		Notification::update_mask ()
	{
		if( !m_has_alpha && is_realized() )
		{
			Glib::RefPtr<Gdk::Bitmap> mask = Glib::RefPtr<Gdk::Bitmap>::cast_static (Gdk::Pixmap::create (RefPtr<Gdk::Drawable> (0), m_width, m_height, 1)) ;

			Cairo::RefPtr<Cairo::Context> cr = mask->create_cairo_context () ;

			int width, height ;
			width = get_allocation ().get_width () - 2 ;
			height = get_allocation ().get_height () ;

			draw_arrow_mask (cr, width, height) ;

			get_window ()->shape_combine_mask (mask, 0, 0) ;
		}
	}

	Notification::~Notification ()
	{
		// empty
	}

	bool
		Notification::on_button_press_event(
            GdkEventButton* event
        )
	{
		if( event->button == 1 )
        {
            hide() ;
        }

		return false ;
	}

	void
		Notification::draw_arrow(
              Cairo::RefPtr<Cairo::Context> & cr
            , int                             w
            , int                             h
        )
	{
		cr->save () ;

		if( m_location == ARROW_TOP )
		{
			Cairo::Matrix matrix ;

            matrix.xx =  1 ;
            matrix.yx =  0 ;
            matrix.xy =  0 ;
            matrix.yy = -1 ; 
            matrix.x0 =  0 ;
            matrix.y0 =  h ;

			cr->set_matrix (matrix) ;
		}

		cr->move_to( 1, arrow_height ) ;

		if( m_position == ARROW_POS_LEFT )
		{
			cr->rel_line_to (+w, 0) ;
			cr->rel_line_to (0, +(h-(2*arrow_height))) ;
			cr->rel_line_to (-(w-(arrow_width/2)), 0) ;
			cr->rel_line_to (-(arrow_width/2), +arrow_height) ;
			cr->rel_line_to (0, -(h-arrow_height)) ;
		}
		else
        if( m_position == ARROW_POS_RIGHT )
		{
			cr->rel_line_to (+w, 0) ;
			cr->rel_line_to (0, +(h-arrow_height)) ;
			cr->rel_line_to (-(arrow_width/2), -arrow_height) ;
			cr->rel_line_to (-(w-(arrow_width/2)), 0) ;
			cr->rel_line_to (0, -(h-(2*arrow_height))) ;
		}
		else
		{
			cr->rel_line_to (+w, 0) ;
			cr->rel_line_to (0, +(h-(2*arrow_height))) ;
			cr->rel_line_to (-((w-m_ax)-(arrow_width/2)), 0) ;
			cr->rel_line_to (-(arrow_width/2), +arrow_height) ;
			cr->rel_line_to (-(arrow_width/2), -arrow_height) ;
			cr->rel_line_to (-(m_ax-(arrow_width/2)), 0) ;
			cr->rel_line_to (0, -(h-(2*arrow_height))) ;
		}

		cr->restore () ;
	}

	void
		Notification::draw_arrow_mask(
              Cairo::RefPtr<Cairo::Context> & cr
            , int                             w
            , int                             h
        )
	{
		cr->save () ;

		cr->set_operator (Cairo::OPERATOR_CLEAR) ;
		cr->paint () ;

		cr->set_operator (Cairo::OPERATOR_SOURCE) ;
		cr->set_source_rgba (.0, .0, .0, 1.0) ;

		draw_arrow (cr, w, h) ;

		cr->fill () ;

		cr->restore () ;
	}

	void
		Notification::draw_arrow_outline(
              Cairo::RefPtr<Cairo::Context> & cr
            , int                             w
            , int                             h
        )
	{
		cr->save () ;

		cr->set_line_width( 0.8 ) ;
		cr->set_line_cap( Cairo::LINE_CAP_ROUND ) ;
		cr->set_line_join( Cairo::LINE_JOIN_ROUND ) ;
		cr->set_operator( Cairo::OPERATOR_SOURCE ) ;

		Gdk::Cairo::set_source_color( cr, m_inlay ) ;
		draw_arrow (cr, w, h) ;
		cr->fill_preserve () ;

		Gdk::Cairo::set_source_color( cr, m_outline ) ;
		cr->stroke() ;
		cr->restore () ;
	}

	void
		Notification::set_position(
              guint position
            , guint duration
        )
	{
		m_kobo_position->set_position( position, duration ) ;
	}

	void
		Notification::set_playstatus (PlayStatus status)
	{
		if( (status == PLAYSTATUS_STOPPED ) || (status == PLAYSTATUS_WAITING) )
		{
			m_kobo_position->set_position( 0, 0 ) ;
		}
	}

	bool
		Notification::on_expose_event(
            GdkEventExpose* event
        )
	{
		Cairo::RefPtr<Cairo::Context> cr = get_window ()->create_cairo_context () ;

		int w, h ;

		w = get_allocation().get_width() ; 
		h = get_allocation().get_height() ;

		cr->set_operator( Cairo::OPERATOR_CLEAR ) ;
		cr->paint() ;

		draw_arrow_outline (cr, w, h) ;

        int offset = 0 ;

		if( m_image )
        {
			get_window ()->draw_pixbuf(
                  RefPtr<Gdk::GC>(0)
                , m_image
                , 0
                , 0
                , 3
                , 10
                , -1
                , -1
                , Gdk::RGB_DITHER_MAX
                , 0
                , 0
            ) ;
        }
        else
        {
            offset = 82 ;
        }

		w = get_allocation().get_width() ; 
		h = get_allocation().get_height() ;

        cr->set_source_rgba(
              0.
            , 0.
            , 0.
            , .6
        ) ;

        cr->set_operator(
            Cairo::OPERATOR_OVER
        ) ;

        Pango::Rectangle ink, logical ;

        m_layout_1->get_pixel_extents( ink, logical ) ;
        int w1 = logical.get_width() ;
        int h1 = logical.get_height() ; 

        m_layout_2->get_pixel_extents( ink, logical ) ;
        int w2 = logical.get_width() ; 
        int h2 = logical.get_height() ;

        int y2 = (h - (h1 + h2 + 2) - 16) / 2 ;
    
        int x_off = (m_image?82:0) ;

        cr->move_to(
              ((w - w2) / 2.) + x_off/2
            , y2 
        ) ;
        pango_cairo_show_layout(
              cr->cobj()
            , m_layout_2->gobj()
        ) ;

        int x1 = ((w - w1) / 2.) + x_off/2 ;
        int y1 = y2 + h2 + 2 ;

        int w1_max = m_width - (m_image?102:8) ; 

        if( w1 > w1_max ) 
        {
            double scale = w1_max / double(w1) ;

            cr->scale(
                  scale 
                , scale 
            ) ;

            x1 = (m_image?94:8)*(1/scale) ;
            y1 = (((h - ((h1*scale) + h2 + 2) - 16) / 2) + h2 + 2) * (1/scale) ;
        }

        cr->move_to(
              x1 
            , y1 
        ) ;

        pango_cairo_show_layout(
              cr->cobj()
            , m_layout_1->gobj()
        ) ;

		return false ;
	}

	void
		Notification::enable(
              bool tooltip
        )
	{
		if( G_UNLIKELY(!is_realized( )) )
        {
            realize() ;
        }

        if( is_visible() )
        {
            hide() ;
        }

	    m_tooltip_mode = tooltip ;

        if( m_tooltip_mode )
        {
	        window_set_opacity( get_window(), 1.0 ) ;
        }

	    reposition () ;

	    Window::show () ;
    }

	void
		Notification::disable(
        )
	{
		if( G_UNLIKELY(!is_realized( )) )
        {
            realize() ;
        }

        if( !is_mapped() )
        {
            return ;
        }

        if( m_tooltip_mode )
	    {
	        m_tooltip_mode = false ;
        }

	    hide() ;
        window_set_opacity( get_window(), 1.0 ) ;
	}

	void
		Notification::on_realize ()
	{
		Window::on_realize() ;

		window_set_opacity( get_window(), 0.0 ) ;
		update_mask() ;

        const int text_size_px_1 = 26 ;
        const int text_size_px_2 = 18 ;

        int text_size_pt = 0 ;

		std::string family = get_pango_context()->get_font_description().get_family() ;

		PangoFontDescription * desc = pango_font_description_new() ;
		pango_font_description_set_family( desc, family.c_str()) ;

        text_size_pt = static_cast<int>((text_size_px_1 * 72) / Util::screen_get_y_resolution (Gdk::Screen::get_default ())) ;
		pango_font_description_set_absolute_size( desc, text_size_pt * PANGO_SCALE ) ;
		pango_font_description_set_weight( desc, PANGO_WEIGHT_BOLD ) ; 
		pango_layout_set_font_description( m_layout_1->gobj(), desc ) ;

        text_size_pt = static_cast<int>((text_size_px_2 * 72) / Util::screen_get_y_resolution (Gdk::Screen::get_default ())) ;
		pango_font_description_set_absolute_size( desc, text_size_pt * PANGO_SCALE ) ;
		pango_font_description_set_weight( desc, PANGO_WEIGHT_NORMAL ) ; 
		pango_layout_set_font_description( m_layout_2->gobj(), desc ) ;

		pango_font_description_free( desc ) ; 
	}

	void
		Notification::on_show ()
	{
		reposition () ;

		Window::on_show () ;
	}

	void
		Notification::on_map ()
	{
		Window::on_map () ;

		if( !m_tooltip_mode )
		{
			m_timer.start () ;

			m_update_connection = signal_timeout ().connect(
                  sigc::mem_fun(
                        *this
                      , &MPX::Notification::update_frame
                  )
                , popup_animation_frame_period_ms
            ) ;
		}
	}

	void
		Notification::on_unmap ()
	{
		Window::on_unmap () ;

		if( m_update_connection ) 
		{
			m_update_connection.disconnect() ;
			m_timer.stop() ;
			m_timer.reset() ;
		}
	}


	void
		Notification::set_metadata(
              const Glib::RefPtr<Gdk::Pixbuf>&    image
            , const MPX::Track&                   t
        )
	{
        if( image )
        {
            m_kobo_position->set_size_request( m_width - 102, -1 ) ;
            m_fixed.move( *m_kobo_position, 94, 72 ) ;

            m_image = image->scale_simple( 80, 80, Gdk::INTERP_HYPER ) ;
        }
        else
        {
            m_kobo_position->set_size_request( m_width - 16, -1 ) ;
            m_fixed.move( *m_kobo_position, 8, 72 ) ;

            m_image.reset() ;
        }

        const std::string& artist = t.has(ATTRIBUTE_ARTIST) ? boost::get<std::string>(t[ATTRIBUTE_ARTIST].get()) : "" ;
        const std::string& title  = t.has(ATTRIBUTE_TITLE)  ? boost::get<std::string>(t[ATTRIBUTE_TITLE].get())  : "" ;

        m_layout_1->set_text( title ) ;
        m_layout_2->set_text( artist ) ;

        enable( false ) ;
	}

	void
		Notification::clear(
        )
	{
        m_kobo_position->set_size_request( m_width - 16, -1  ) ;
        m_fixed.move( *m_kobo_position, 8, 72 ) ;
        m_image = Glib::RefPtr<Gdk::Pixbuf>(0);

        m_layout_1->set_text( "" ) ;
        m_layout_2->set_text( "" ) ;

        enable( false ) ;
	}

	void
		Notification::queue_update(
        )
	{
		m_timer.reset() ;

		if( is_mapped() )
		{
			reposition() ;
			queue_draw() ;
		}
	}

	void
		Notification::disappear()
	{
		double time = m_timer.elapsed () + m_time_offset ;
		double disappear_start_time = get_popup_disappear_start_time( m_fade ) ;

		if( time < disappear_start_time )
		{
			m_timer.reset() ;
			m_time_offset = disappear_start_time ;
		}
	}

	void
		Notification::acquire_widget_info(
              int & x
    		, int & y
	    	, int & width
		    , int & height
        )
	{
		gdk_flush() ;
		while( gtk_events_pending() )
			gtk_main_iteration() ;

		gdk_window_get_origin( m_widget->window, &x, &y ) ;

		gdk_flush() ;
		while( gtk_events_pending() )
			gtk_main_iteration() ;

		gdk_window_get_geometry( m_widget->window, NULL, NULL, &width, &height, NULL ) ;
	}

	bool
		Notification::update_frame ()
	{
		double time ;

		time = m_timer.elapsed () ; 

		if( time < get_popup_end_time( m_fade ))
		{
			double alpha = get_popup_alpha_at_time( time, m_fade ) ;
			window_set_opacity( get_window(), alpha ) ;
			return true ;
		}
		else
		{
			if( !m_tooltip_mode )
            {
                hide() ;
            }

			return false ;
		}
	}
} // namespace MPX
