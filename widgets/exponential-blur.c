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

#include <math.h>

#include "exponential-blur.h"

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

