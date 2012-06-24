//  MPX
//  Copyright( C ) 2005-2007 MPX development.
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License Version 2
//  as published by the Free Software Foundation.
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
//  The MPX project hereby grants permission for non-GPL compatible GStreamer
//  plugins to be used and distributed together with GStreamer and MPX. This
//  permission is above and beyond the permissions granted by the GPL license
//  MPX is covered by.

#include <cstdlib>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <gtkmm.h>
#include <glibmm/i18n.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include "mpx/util-graphics.hh"
#include "mpx/widgets/cairo-extensions.hh"
#include "mpx/mpx-minisoup.hh"
#include "mpx/mpx-uri.hh"

using namespace Glib ;

namespace
{
  gdouble
  mm_to_inch( gdouble mm )
  {
    return mm / 25.40 ;
  }

  inline guint8
  convert_color_channel (guint8 src,
                         guint8 alpha)
  {
    return alpha ?( (guint (src) << 8) - src ) / alpha : 0 ;
  }

  void
  convert_bgra_to_rgba (guint8 const* src,
                        guint8*       dst,
                        int           width,
                        int           height)
  {
    guint8 const* src_pixel = src ;
    guint8*       dst_pixel = dst ;

    for( int y = 0; y < height; y++ )
      for( int x = 0; x < width; x++ )
      {
        dst_pixel[0] = convert_color_channel (src_pixel[2],
                                              src_pixel[3]) ;
        dst_pixel[1] = convert_color_channel (src_pixel[1],
                                              src_pixel[3]) ;
        dst_pixel[2] = convert_color_channel (src_pixel[0],
                                              src_pixel[3]) ;
        dst_pixel[3] = src_pixel[3] ;

        dst_pixel += 4 ;
        src_pixel += 4 ;
      }
   }

    struct HSL
    {
	float h, s, l;
    };

    HSL make_hsl (float h, float s, float l)
    {
	HSL hsl = { h, s, l };

	return hsl;
    }

    HSL hsl_from_rgb_f (float r, float g, float b) ;

    HSL hsl_from_rgb (uint8_t r8, uint8_t g8, uint8_t b8)
    {
	float r = r8 / 255.0;
	float g = g8 / 255.0;
	float b = b8 / 255.0;

	return hsl_from_rgb_f( r, g, b ) ;
    }

    HSL hsl_from_rgb_f (float r, float g, float b)
    {
	float max = std::max(r, std::max (g, b));
	float min = std::min(r, std::min (g, b));

	float h;
	float s;
	float l = (max + min) / 2;

	if (max == min) {
	    h = s = 0; // achromatic
	} else {
	    float d = max - min;

	    s = l > 0.5 ? d / (2 - max - min) : d / (max + min);

	    if (max == r) {
		h = (g - b) / d + (g < b ? 6 : 0);
	    } else if (max == g) {
		h = (b - r) / d + 2;
	    } else if (max == b) {
		h = (r - g) / d + 4;
	    }

	    h /= 6;
	}

	return make_hsl (h, s, l);
    }

    Gdk::RGBA get_dominant_color (Glib::RefPtr<Gdk::Pixbuf> const& image)
    {
	using namespace MPX ;

	int bytes_per_pixel = image->get_has_alpha () ? 4 : 3;

	int width  = image->get_width ();
	int height = image->get_height ();
	int row_stride = image->get_rowstride ();

	int count[64][64];

	int lightness = 0 ;

	int* start = &count[0][0];
	int* end   = start + 64*64;

	std::fill (start, end, 0);

	uint8_t const *pixel_row = image->get_pixels ();

	for (int y = 0; y < height; y++) {
	    uint8_t const* p = pixel_row;

	    for (int x = 0; x < width; x++) {

		double h, s, b ;

		Util::color_to_hsb(Util::make_rgba(p[0]/255.,p[1]/255.,p[2]/255.), h, s, b ) ; 
		count[int (h / 6)][int (s * 63)]++;

		lightness += (b*255) ;

		p += bytes_per_pixel;
	    }

	    pixel_row += row_stride;
	}

	int* entry = std::max_element (start, end);

	std::ptrdiff_t index = entry - start;

	int h = index >> 6;
	int s = index & 0x3f;

	double hf, sf, lf ;

	hf = h * 6 ; 
	sf = s / 63. ;
	lf = (lightness / ( width * height )) / 255. ; 

	return Util::color_from_hsb( hf, sf, lf ) ; 
    }
}

namespace MPX
{
  namespace Util
  {
    Glib::RefPtr<Gdk::Pixbuf>
    get_image_from_uri(
        const std::string& uri
    )
    {
      Glib::RefPtr<Gdk::Pixbuf> image = Glib::RefPtr<Gdk::Pixbuf>(0) ;

      try{
          URI u( uri ) ;
          if( u.get_protocol() == URI::PROTOCOL_HTTP )
          {
              Soup::RequestSyncRefP request = Soup::RequestSync::create( Glib::ustring (u) ) ;
              guint code = request->run(); 

              if( code == 200 )
              try{
                    Glib::RefPtr<Gdk::PixbufLoader> loader = Gdk::PixbufLoader::create() ;
                    loader->write( reinterpret_cast<const guint8*>(request->get_data_raw()), request->get_data_size() ) ;
                    loader->close() ;
                    image = loader->get_pixbuf() ;
              } catch( Gdk::PixbufError & cxe ) {
                    g_message("%s: Gdk::PixbufError: %s", G_STRLOC, cxe.what().c_str()) ;
              }
          }
          else if( u.get_protocol() == URI::PROTOCOL_FILE ) 
          {
            image = Gdk::Pixbuf::create_from_file( filename_from_uri (uri) ) ;
          }
      } catch( URI::ParseError & cxe ) {
          g_message("%s: URI Parse Error: %s", G_STRLOC, cxe.what()) ;
      }
      return image ;
    }

    void
    window_set_busy( GtkWindow* window )
    {
        static GdkCursor* cursor = 0 ;

        if( !cursor )
        {
            cursor = gdk_cursor_new_from_name( gdk_display_get_default(), "watch" ) ;
        }

        gdk_window_set_cursor( gtk_widget_get_window (GTK_WIDGET (window)), cursor ) ;
        gdk_flush() ;
    }

    void
    window_set_idle( GtkWindow* window )
    {
        gdk_window_set_cursor( gtk_widget_get_window (GTK_WIDGET (window)), NULL ) ;
        gdk_flush() ;
    }

    void
    window_set_busy( Gtk::Window & window )
    {
        static GdkCursor* cursor = 0 ;

        if( !cursor )
        {
            cursor = gdk_cursor_new_from_name( gdk_display_get_default(), "watch" ) ;
        }

        gdk_window_set_cursor( window.get_window()->gobj(), cursor ) ;
        gdk_flush() ;
    }

    void
    window_set_idle( Gtk::Window & window )
    {
        window.get_window()->set_cursor() ;
        gdk_flush() ;
    }

    void
    color_to_rgba(
          const Gdk::RGBA& color
        , double &          r
        , double &          g
        , double &          b
        , double &          a
    )
    {
      r = color.get_red_u()   / 65535.0 ;
      g = color.get_green_u() / 65535.0 ;
      b = color.get_blue_u()  / 65535.0 ;
      a = 1.0 ;
    }

    void color_to_hsb(
          Gdk::RGBA const& color
        , double & hue
        , double & saturation
        , double & brightness
    )
    {
        double min, max, delta ;

        double red = color.get_red_u()/65535. ;
        double green = color.get_green_u()/65535. ;
        double blue = color.get_blue_u()/65535. ;

        hue = 0 ;
        saturation = 0 ;
        brightness = 0 ;

        if(red > green) {
            max = fmax(red, blue) ;
            min = fmin(green, blue) ;
        } else {
            max = fmax(green, blue) ;
            min = fmin(red, blue) ;
        }

        brightness =( max + min ) / 2 ;

        if(fabs(max - min) < 0.0001) {
            hue = 0 ;
            saturation = 0 ;
        } else {
            saturation = brightness <= 0.5
                ?( max - min) / (max + min )
                :( max - min) / (2 - max - min ) ;

            delta = max - min ;

            if(red == max) {
                hue =( green - blue ) / delta ;
            } else if(green == max) {
                hue = 2 +( blue - red ) / delta ;
            } else if(blue == max) {
                hue = 4 +( red - green ) / delta ;
            }

            hue *= 60 ;
            if(hue < 0) {
                hue += 360 ;
            }
        }
    }

    Gdk::RGBA
    color_from_hsb(
          double hue
        , double saturation
        , double brightness
    )
    {
        int i  ;
        double hue_shift[] = { 0, 0, 0 }  ;
        double color_shift[] = { 0, 0, 0 }  ;
        double m1, m2, m3  ;

        m2 = brightness <= 0.5
            ? brightness *( 1 + saturation )
            : brightness + saturation - brightness * saturation ;

        m1 = 2 * brightness - m2 ;

        hue_shift[0] = hue + 120 ;
        hue_shift[1] = hue ;
        hue_shift[2] = hue - 120 ;

        color_shift[0] = color_shift[1] = color_shift[2] = brightness;

        i = saturation == 0 ? 3 : 0 ;

        for(; i < 3; i++) {
            m3 = hue_shift[i] ;

            if(m3 > 360) {
                m3 = fmod(m3, 360) ;
            } else if(m3 < 0) {
                m3 = 360 - fmod(fabs(m3), 360) ;
            }

            if(m3 < 60) {
                color_shift[i] = m1 +( m2 - m1 ) * m3 / 60 ;
            } else if(m3 < 180) {
                color_shift[i] = m2 ;
            } else if(m3 < 240) {
                color_shift[i] = m1 +( m2 - m1) * (240 - m3 ) / 60 ;
            } else {
                color_shift[i] = m1 ;
            }
        }

        Gdk::RGBA color ;

        color.set_red_u(color_shift[0]*65535) ;
        color.set_green_u(color_shift[1]*65535) ;
        color.set_blue_u(color_shift[2]*65535) ;

        return color ;
    }

    Gdk::RGBA
    color_shade(
          const Gdk::RGBA& base
        , double            ratio
    )
    {
        double h, s, b ;

        color_to_hsb(base, h, s, b) ;

        b = fmax(fmin(b * ratio, 1), 0) ;
        s = fmax(fmin(s * ratio, 1), 0) ;

        return color_from_hsb(h, s, b) ;
    }

    Gdk::RGBA
    color_adjust_brightness(
          const Gdk::RGBA& base
        , double           brightness
    )
    {
        double h, s, b ;
        color_to_hsb(base, h, s, b) ;
        b = fmax(fmin(brightness, 1), 0) ;
        return color_from_hsb(h, s, b) ;
    }

    double
    screen_get_resolution(
          Glib::RefPtr<Gdk::Screen> screen
    )
    {
        // NOTE: this assumes the width and height resolutions are
        // equal. This is only true for video modes with aspect ratios that
        // match the physical screen area ratio e.g. 1024x768 for 4:3
        // screens - descender

        return std::ceil( screen_get_y_resolution (screen) ) ;
    }

    double
    screen_get_x_resolution(
          Glib::RefPtr<Gdk::Screen> screen
    )
    {
        return static_cast<double>( screen->get_width()) / mm_to_inch (screen->get_height_mm() ) ;
    }

    double
    screen_get_y_resolution(
          Glib::RefPtr<Gdk::Screen> screen
    )
    {
      return static_cast<double>( screen->get_height()) / mm_to_inch (screen->get_height_mm() ) ;
    }

    Cairo::RefPtr<Cairo::ImageSurface>
    cairo_image_surface_from_pixbuf(
          Glib::RefPtr<Gdk::Pixbuf> pixbuf
    )
    {
        Cairo::RefPtr< ::Cairo::ImageSurface> surface = Cairo::ImageSurface::create( Cairo::FORMAT_ARGB32, pixbuf->get_width(), pixbuf->get_height() ) ;
        Cairo::RefPtr< ::Cairo::Context> cairo = Cairo::Context::create( surface ) ;

        cairo->set_operator( Cairo::OPERATOR_SOURCE ) ;
        Gdk::Cairo::set_source_pixbuf( cairo, pixbuf, 0, 0 ) ;
        cairo->rectangle( 0, 0, pixbuf->get_width(), pixbuf->get_height() ) ;
        cairo->fill() ;
        return surface ;
    }

    void
    cairo_rounded_rect(
          Cairo::RefPtr<Cairo::Context> cairo
        , double                        x
        , double                        y
        , double                        width
        , double                        height
        , double                        radius
    )
    {
        g_return_if_fail( width > 0 && height > 0 && radius >= 0 ) ;
        RoundedRectangle( cairo, x + 1, y + 1, width - 2, height - 2, radius ) ;
    }

    Cairo::RefPtr<Cairo::ImageSurface>
    cairo_image_surface_scale(
          Cairo::RefPtr<Cairo::ImageSurface>    source
        , double                                width
        , double                                height
    )
    {
        Cairo::RefPtr< ::Cairo::ImageSurface> dest = Cairo::ImageSurface::create( source->get_format(), int (width), int (height) ) ;
        Cairo::RefPtr< ::Cairo::Context> cairo = Cairo::Context::create( dest ) ;

        cairo->set_operator( Cairo::OPERATOR_SOURCE ); 
        cairo->scale( width / source->get_width(), height / source->get_height() ) ;
        cairo->set_source( source, 0., 0. ) ;
        cairo->rectangle( 0., 0., source->get_width(), source->get_height() ) ;
        cairo->fill() ;
        return dest ;
    }

    Cairo::RefPtr<Cairo::ImageSurface>
    cairo_image_surface_round(
          Cairo::RefPtr<Cairo::ImageSurface>    source
        , double                                radius
    )
    {
        Cairo::RefPtr< ::Cairo::ImageSurface> dest = Cairo::ImageSurface::create( source->get_format(), source->get_width(), source->get_height() ); 
        Cairo::RefPtr< ::Cairo::Context> cairo = Cairo::Context::create( dest ) ;

        cairo->set_operator( Cairo::OPERATOR_SOURCE ); 
        cairo_rounded_rect( cairo, 0, 0, source->get_width(), source->get_height(), radius ) ;

        cairo->set_source( source, 0., 0. ) ;
        cairo->fill_preserve() ;

        return dest ;
    }

    Cairo::RefPtr<Cairo::ImageSurface>
    cairo_image_surface_overlay(
          Cairo::RefPtr<Cairo::ImageSurface>    source
        , Cairo::RefPtr<Cairo::ImageSurface>    overlay
        , double                                x
        , double                                y
        , double                                alpha
    )
    {
      Cairo::RefPtr< ::Cairo::ImageSurface> dest = Cairo::ImageSurface::create( source->get_format(), source->get_width(), source->get_height() ); 
      Cairo::RefPtr< ::Cairo::Context> cairo = Cairo::Context::create( dest ) ;

      cairo->set_operator( Cairo::OPERATOR_SOURCE ); 
      cairo->rectangle( 0., 0., double(source->get_width()), double(source->get_height()) ) ;
      cairo->set_source( source, 0., 0. ) ;
      cairo->fill() ;

      cairo->set_operator( Cairo::OPERATOR_ATOP ); 
      cairo->rectangle( x, y, overlay->get_width(), overlay->get_height() ) ;
      cairo->set_source( overlay, x, y ) ;
      cairo->paint_with_alpha( alpha ) ;

      return dest ;
    }


    void 
    cairo_image_surface_border(
          Cairo::RefPtr<Cairo::ImageSurface>      source
        , double                                  width
        , double                                  r
        , double                                  g
        , double                                  b
        , double                                  a
    )
    {
      Cairo::RefPtr< ::Cairo::Context> cairo = Cairo::Context::create( source ) ;
      cairo->set_operator( Cairo::OPERATOR_SOURCE ); 
      cairo->rectangle( 0, 0, source->get_width(), source->get_height() ) ;
      cairo->set_source_rgba(r, g, b, a) ;
      cairo->set_line_width( width ) ;
      cairo->stroke() ;
    }

    void 
    cairo_image_surface_rounded_border(
          Cairo::RefPtr<Cairo::ImageSurface>      source
        , double                                  width
        , double                                  radius
        , double                                  r
        , double                                  g
        , double                                  b
        , double                                  a
    )
    {
      Cairo::RefPtr< ::Cairo::Context> cairo = Cairo::Context::create( source ) ;
      cairo_rounded_rect(cairo, 0, 0, source->get_width(), source->get_height(), radius) ;
      cairo->set_source_rgba(r, g, b, a) ;
      cairo->set_line_width( width ) ;
      cairo->stroke() ;
    }

    void
    draw_cairo_image(
          Cairo::RefPtr<Cairo::Context>         cairo
        , Cairo::RefPtr<Cairo::ImageSurface>    image
        , double                                x
        , double                                y
        , double                                alpha
	, double				r
    )
    {
        cairo->save() ;

        cairo->translate( x, y ) ;

        cairo->set_operator( Cairo::OPERATOR_ATOP ) ;
        cairo->set_source( image, 0.0, 0.0 ) ;
        RoundedRectangle( cairo, 0.0, 0.0, image->get_width(), image->get_height(), r ) ;
        cairo->clip() ;
        cairo->paint_with_alpha( alpha ) ;

        cairo->restore() ;
    }

    Glib::RefPtr<Gdk::Pixbuf>
    cairo_image_surface_to_pixbuf(
          Cairo::RefPtr<Cairo::ImageSurface>    image
    )
    {
        Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create(image, 0, 0, image->get_width(), image->get_height()) ; 
        return pixbuf ;
    }


    Gdk::RGBA
    get_dominant_color_for_pixbuf(
          Glib::RefPtr<Gdk::Pixbuf>             pb0
    )
    {
	return get_dominant_color( pb0->scale_simple(64,64,Gdk::INTERP_HYPER)) ;
    }

    Gdk::RGBA
    pick_color_for_pixbuf(
          Glib::RefPtr<Gdk::Pixbuf>             pb0
    )
    {
	Gdk::RGBA mean = get_dominant_color_for_pixbuf(pb0) ;

	double    r = mean.get_red()
		, g = mean.get_green()
		, b = mean.get_blue()
	;

	if( r < 0.3 && g < 0.3 && b < 0.3 )
	{
	    mean = Util::get_mean_color_for_pixbuf(pb0) ; 
	}

	return mean ;
    }

    Gdk::RGBA
    get_mean_color_for_pixbuf(
          Glib::RefPtr<Gdk::Pixbuf>             pb0
    )
    {
	GdkPixbuf * pixbuf = pb0->gobj() ;

	guint64 a_total, r_total, g_total, b_total;
	guint row, column;
	int row_stride;
	const guchar *pixels, *p;
	int r, g, b, a;
	guint64 sub ;
	guint64 dividend;
	guint width, height ;
	gdouble dd;

	width = gdk_pixbuf_get_width (pixbuf);
	height = gdk_pixbuf_get_height (pixbuf)/*/2*/;
	row_stride = gdk_pixbuf_get_rowstride (pixbuf);
	pixels = gdk_pixbuf_get_pixels (pixbuf);

	/* iterate through the pixbuf, counting up each component */
	a_total = 0;
	r_total = 0;
	g_total = 0;
	b_total = 0;

	sub = 0 ;

	if (gdk_pixbuf_get_has_alpha (pixbuf)) {
		for (row = 0; row < height; row++) {
			p = pixels + (row * row_stride);
			for (column = 0; column < width; column++) {
				r = *p++;
				g = *p++;
				b = *p++;
				a = *p++;

				a_total += a;
				r_total += r * a;
				g_total += g * a;
				b_total += b * a;
			}
		}
		dividend = height * width * 0xFF - sub ;
		a_total *= 0xFF;
	} else {
		for (row = 0; row < height; row++) {
			p = pixels + (row * row_stride);
			for (column = 0; column < width; column++) {
				r = *p++;
				g = *p++;
				b = *p++;

				r_total += r;
				g_total += g;
				b_total += b;
			}
		}
		dividend = height * width - sub ;
		a_total = dividend * 0xFF;
	}

	dd = dividend * 0xFF ;
	return Util::make_rgba((r_total/dd) , (g_total/dd) , (b_total/dd)) ;
    }
  } // namespace Util
} // namespace MPX
