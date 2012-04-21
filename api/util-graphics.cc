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

        gdk_window_set_cursor( GTK_WIDGET (window)->window, cursor ) ;
        gdk_flush() ;
    }

    void
    window_set_idle( GtkWindow* window )
    {
        gdk_window_set_cursor( GTK_WIDGET (window)->window, NULL ) ;
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

        gdk_window_set_cursor( GTK_WIDGET(window.gobj())->window, cursor ) ;
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
          const Gdk::Color& color
        , double &          r
        , double &          g
        , double &          b
        , double &          a
    )
    {
      r = color.get_red()   / 65535.0 ;
      g = color.get_green() / 65535.0 ;
      b = color.get_blue()  / 65535.0 ;
      a = 1.0 ;
    }

    void color_to_hsb(
          Gdk::Color const& color
        , double & hue
        , double & saturation
        , double & brightness
    )
    {
        double min, max, delta ;
        double red = color.get_red()/65535. ;
        double green = color.get_green()/65535. ;
        double blue = color.get_blue()/65535. ;

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

    Gdk::Color
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

        Gdk::Color color ;
        color.set_red(color_shift[0]*65535) ;
        color.set_green(color_shift[1]*65535) ;
        color.set_blue(color_shift[2]*65535) ;

        return color ;
    }

    Gdk::Color
    color_shade(
          const Gdk::Color& base
        , double            ratio
    )
    {
        double h, s, b ;

        color_to_hsb(base, h, s, b) ;

        b = fmax(fmin(b * ratio, 1), 0) ;
        s = fmax(fmin(s * ratio, 1), 0) ;

        return color_from_hsb(h, s, b) ;
    }

    Gdk::Color
    color_adjust_brightness(
          const Gdk::Color& base
        , double            brightness
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
    )
    {
        cairo->save() ;

        cairo->translate( x, y ) ;

        cairo->set_operator( Cairo::OPERATOR_ATOP ) ;
        cairo->set_source( image, 0.0, 0.0 ) ;
        cairo->rectangle( 0.0, 0.0, image->get_width(), image->get_height() ) ;
        cairo->clip() ;
        cairo->paint_with_alpha( alpha ) ;

        cairo->restore() ;
    }

    Glib::RefPtr<Gdk::Pixbuf>
    cairo_image_surface_to_pixbuf(
          Cairo::RefPtr<Cairo::ImageSurface>    image
    )
    {
        Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, 8, image->get_width(), image->get_height()) ;
        convert_bgra_to_rgba(((guint8 const*)image->get_data()), pixbuf->get_pixels(), image->get_width(), image->get_height()) ;
        return pixbuf ;
    }

    Gdk::Color
    get_mean_color_for_pixbuf(
          Glib::RefPtr<Gdk::Pixbuf>             image
    )
    {
        Gdk::Color c ;

        Glib::RefPtr<Gdk::Pixbuf> tiny = image->scale_simple( 1, 1 , Gdk::INTERP_NEAREST ) ;
        guchar * pixels = gdk_pixbuf_get_pixels( GDK_PIXBUF(tiny->gobj()) ) ;
        c.set_rgb_p( double(pixels[0])/255., double(pixels[1])/255., double(pixels[2])/255. ) ;

        return c ;
    }
  } // namespace Util
} // namespace MPX
