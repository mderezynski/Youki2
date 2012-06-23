#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <cairo.h>
#include <gtk/gtk.h>
#include <gtkmm.h>
#include <cmath>
#include <pixman.h>

#include "exponential-blur.h"
#include "mpx/widgets/cairo-blur.hh"

namespace
{
/* G(x,y) = 1/(2 * PI * sigma^2) * exp(-(x^2 + y^2)/(2 * sigma^2))
 */
pixman_fixed_t *
create_gaussian_blur_kernel (int     radius,
                             double  sigma,
                             int    *length)
{
        const double scale2 = 2.0 * sigma * sigma;
        const double scale1 = 1.0 / (M_PI * scale2);

        const int size = 2 * radius + 1;
        const int n_params = size * size;

        pixman_fixed_t *params;
        double *tmp, sum;
        int x, y, i;

        tmp = g_newa (double, n_params);

        /* caluclate gaussian kernel in floating point format */
        for (i = 0, sum = 0, x = -radius; x <= radius; ++x) {
                for (y = -radius; y <= radius; ++y, ++i) {
                        const double u = x * x;
                        const double v = y * y;

                        tmp[i] = scale1 * exp (-(u+v)/scale2);

                        sum += tmp[i];
                }
        }

        /* normalize gaussian kernel and convert to fixed point format */
        params = g_new (pixman_fixed_t, n_params + 2);

        params[0] = pixman_int_to_fixed (size);
        params[1] = pixman_int_to_fixed (size);

        for (i = 0; i < n_params; ++i)
                params[2 + i] = pixman_double_to_fixed (tmp[i] / sum);

        if (length)
                *length = n_params + 2;

        return params;
}

cairo_surface_t *
blur_image_surface (cairo_surface_t *surface,
                    int              radius,
                    double           sigma)
{
        static cairo_user_data_key_t data_key;
        pixman_fixed_t *params = NULL;
        int n_params;

        pixman_image_t *src, *dst;
        int w, h, s;
        gpointer p;

        w = cairo_image_surface_get_width (surface);
        h = cairo_image_surface_get_height (surface);
        s = cairo_image_surface_get_stride (surface);

        /* create pixman image for cairo image surface */
        p = cairo_image_surface_get_data (surface);
        src = pixman_image_create_bits (PIXMAN_a8r8g8b8, w, h, (uint32_t*)p, s);

        /* attach gaussian kernel to pixman image */
        params = create_gaussian_blur_kernel (radius, sigma, &n_params);
        pixman_image_set_filter (src, PIXMAN_FILTER_CONVOLUTION, params, n_params);
        g_free (params);

        /* render blured image to new pixman image */
        p = g_malloc0 (s * h);
        dst = pixman_image_create_bits (PIXMAN_a8r8g8b8, w, h, (uint32_t*)p, s);
        pixman_image_composite (PIXMAN_OP_SRC, src, NULL, dst, 0, 0, 0, 0, 0, 0, w, h);
        pixman_image_unref (src);

        /* create new cairo image for blured pixman image */
        surface = cairo_image_surface_create_for_data((unsigned char*)p, CAIRO_FORMAT_ARGB32, w, h, s);
        cairo_surface_set_user_data (surface, &data_key, p, g_free);
        pixman_image_unref (dst);

        return surface;
}
}

////////////////////////////////////////////////////////////////////////////////
//3456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789
//      10        20        30        40        50        60        70        80
//
// raico-blur
//
// exponential-blur.c - implements exponential-blur function
//
// Copyright 2009 Canonical Ltd.
//
// Authors:
//    Mirco "MacSlow" Mueller <mirco.mueller@canonical.com>
//    Jason Smith <jason.smith@canonical.com>
//
// Notes:
//    based on exponential-blur algorithm by Jani Huhtanen
//
// This program is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License version 3, as published
// by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranties of
// MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
// PURPOSE.  See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program.  If not, see <http://www.gnu.org/licenses/>.
//
////////////////////////////////////////////////////////////////////////////////

// FIXME: not working yet, unfinished

static inline void
_blurinner (unsigned char* pixel,
	    int   *zR,
	    int   *zG,
	    int   *zB,
	    int   *zA,
	    int    alpha,
	    int    aprec,
	    int    zprec);

static inline void
_blurrow (unsigned char* pixels,
	  int    width,
	  int    height,
	  int    channels,
	  int    line,
	  int    alpha,
	  int    aprec,
	  int    zprec);

static inline void
_blurcol (unsigned char* pixels,
	  int    width,
	  int    height,
	  int    channels,
	  int    col,
	  int    alpha,
	  int    aprec,
	  int    zprec);

void
_expblur (unsigned char* pixels,
	  int    width,
	  int    height,
	  int    channels,
	  int    radius,
	  int    aprec,
	  int    zprec);

void
surface_exponential_blur (cairo_surface_t* surface,
			  unsigned int     radius)
{
	unsigned char*        pixels;
	unsigned int          width;
	unsigned int          height;
	cairo_format_t format;

	// sanity checks are done in raico-blur.c

	// before we mess with the surface execute any pending drawing
	cairo_surface_flush (surface);

	pixels = cairo_image_surface_get_data (surface);
	width  = cairo_image_surface_get_width (surface);
	height = cairo_image_surface_get_height (surface);
	format = cairo_image_surface_get_format (surface);

	switch (format)
	{
		case CAIRO_FORMAT_ARGB32:
			_expblur (pixels, width, height, 4, radius, 16, 7);
		break;

		case CAIRO_FORMAT_RGB24:
			_expblur (pixels, width, height, 3, radius, 16, 7);
		break;

		case CAIRO_FORMAT_A8:
			_expblur (pixels, width, height, 1, radius, 16, 7);
		break;

		default :
			// do nothing
		break;
	}

	// inform cairo we altered the surfaces contents
	cairo_surface_mark_dirty (surface);	
}

//
// pixels   image-data
// width    image-width
// height   image-height
// channels image-channels
//
// in-place blur of image 'img' with kernel of approximate radius 'radius'
//
// blurs with two sided exponential impulse response
//
// aprec = precision of alpha parameter in fixed-point format 0.aprec
//
// zprec = precision of state parameters zR,zG,zB and zA in fp format 8.zprec
//
void
_expblur (unsigned char* pixels,
	  int    width,
	  int    height,
	  int    channels,
	  int    radius,
	  int    aprec,
	  int    zprec)
{
	int alpha;
	int row = 0;
	int col = 0;

	if (radius < 1)
		return;

	// calculate the alpha such that 90% of 
	// the kernel is within the radius.
	// (Kernel extends to infinity)
	alpha = (int) ((1 << aprec) * (1.0f - expf (-2.3f / (radius + 1.f))));

	for (; row < height; row++)
		_blurrow (pixels,
			  width,
			  height,
			  channels,
			  row,
			  alpha,
			  aprec,
			  zprec);

	for(; col < width; col++)
		_blurcol (pixels,
			  width,
			  height,
			  channels,
			  col,
			  alpha,
			  aprec,
			  zprec);

	return;
}

static inline void
_blurinner (unsigned char* pixel,
	    int   *zR,
	    int   *zG,
	    int   *zB,
	    int   *zA,
	    int    alpha,
	    int    aprec,
	    int    zprec)
{
	int R;
	int G;
	int B;
	unsigned char A;

	R = *pixel;
	G = *(pixel + 1);
	B = *(pixel + 2);
	A = *(pixel + 3);

	*zR += (alpha * ((R << zprec) - *zR)) >> aprec;
	*zG += (alpha * ((G << zprec) - *zG)) >> aprec;
	*zB += (alpha * ((B << zprec) - *zB)) >> aprec;
	*zA += (alpha * ((A << zprec) - *zA)) >> aprec;

	*pixel       = *zR >> zprec;
	*(pixel + 1) = *zG >> zprec;
	*(pixel + 2) = *zB >> zprec;
	*(pixel + 3) = *zA >> zprec;
} 

static inline void
_blurrow (unsigned char* pixels,
	  int    width,
	  int    height,
	  int    channels,
	  int    line,
	  int    alpha,
	  int    aprec,
	  int    zprec)
{
	int    zR;
	int    zG;
	int    zB;
	int    zA;
	int    index;
	unsigned char* scanline;

	scanline = &(pixels[line * width * channels]);

	zR = *scanline << zprec;
	zG = *(scanline + 1) << zprec;
	zB = *(scanline + 2) << zprec;
	zA = *(scanline + 3) << zprec;

	for (index = 0; index < width; index ++)
		_blurinner (&scanline[index * channels],
			    &zR,
			    &zG,
			    &zB,
			    &zA,
			    alpha,
			    aprec,
			    zprec);

	for (index = width - 2; index >= 0; index--)
		_blurinner (&scanline[index * channels],
			    &zR,
			    &zG,
			    &zB,
			    &zA,
			    alpha,
			    aprec,
			    zprec);
}

static inline void
_blurcol (unsigned char* pixels,
	  int    width,
	  int    height,
	  int    channels,
	  int    x,
	  int    alpha,
	  int    aprec,
	  int    zprec)
{
	int zR;
	int zG;
	int zB;
	int zA;
	int index;
	unsigned char* ptr;

	ptr = pixels;
	
	ptr += x * channels;

	zR = *((unsigned char*) ptr    ) << zprec;
	zG = *((unsigned char*) ptr + 1) << zprec;
	zB = *((unsigned char*) ptr + 2) << zprec;
	zA = *((unsigned char*) ptr + 3) << zprec;

	for (index = width; index < (height - 1) * width; index += width)
		_blurinner ((unsigned char*) &ptr[index * channels],
			    &zR,
			    &zG,
			    &zB,
			    &zA,
			    alpha,
			    aprec,
			    zprec);

	for (index = (height - 2) * width; index >= 0; index -= width)
		_blurinner ((unsigned char*) &ptr[index * channels],
			    &zR,
			    &zG,
			    &zB,
			    &zA,
			    alpha,
			    aprec,
			    zprec);
}


namespace MPX
{
namespace Util
{
void
render_text_shadow(	
      Glib::RefPtr<Pango::Layout>	   layout
    , guint x
    , guint y
    , const Cairo::RefPtr<Cairo::Context>& cairo
    , double radius
    , double alpha
)
{
    Pango::Rectangle ink, logical ;
    layout->get_extents( ink, logical ) ;

    auto s = Cairo::ImageSurface::create(
		      Cairo::FORMAT_ARGB32
		    , logical.get_width() / PANGO_SCALE
		    , logical.get_height() / PANGO_SCALE
    ) ; 

    auto c2 = Cairo::Context::create( s ) ;

    c2->set_operator( Cairo::OPERATOR_CLEAR ) ;
    c2->paint() ;

    c2->set_operator( Cairo::OPERATOR_OVER ) ;
    c2->set_source_rgba(
	      0. 
	    , 0. 
	    , 0.
	    , alpha 
    ) ;
    c2->move_to(
	       0 
	     , 0 
    ) ;
    pango_cairo_show_layout(
	  c2->cobj()
	, layout->gobj()
    ) ;

    Util::cairo_image_surface_blur( s, radius ) ; 

    cairo->move_to(
	  x 
	, y 
    ) ;
    cairo->set_source( s, x+1, y+1 ) ;
    cairo->rectangle( x+1, y+1, logical.get_width()/PANGO_SCALE, logical.get_height()/PANGO_SCALE) ;
    cairo->fill() ;
}

void
cairo_image_surface_blur( Cairo::RefPtr<Cairo::ImageSurface>& s, double radius )
{
#if 0
    cairo_surface_t* blurred = blur_image_surface( static_cast<cairo_surface_t*>(s->cobj()), radius, 8. ) ;
    s = Cairo::RefPtr<Cairo::ImageSurface>(new Cairo::ImageSurface( blurred, false )) ;
#endif
    surface_exponential_blur(static_cast<cairo_surface_t*>(s->cobj()), (unsigned int)radius) ;
}

}}
