#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <cairo.h>
#include <gtk/gtk.h>
#include <math.h>
#include <pixman.h>

#include "mpx/widgets/cairo-blur.hh"

namespace
{
int        radius  = 1;
double     offset  = 1.0;
double     sigma   = 5.0;
double     alpha   = 1.0; 
double     red     = 0.0;
double     green   = 0.0;
double     blue    = 0.0;

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
        src = pixman_image_create_bits (PIXMAN_a8, w, h, (uint32_t*)p, s);

        /* attach gaussian kernel to pixman image */
        params = create_gaussian_blur_kernel (radius, sigma, &n_params);
        pixman_image_set_filter (src, PIXMAN_FILTER_CONVOLUTION, params, n_params);
        g_free (params);

        /* render blured image to new pixman image */
        p = g_malloc0 (s * h);
        dst = pixman_image_create_bits (PIXMAN_a8, w, h, (uint32_t*)p, s);
        pixman_image_composite (PIXMAN_OP_SRC, src, NULL, dst, 0, 0, 0, 0, 0, 0, w, h);
        pixman_image_unref (src);

        /* create new cairo image for blured pixman image */
        surface = cairo_image_surface_create_for_data((unsigned char*)p, CAIRO_FORMAT_A8, w, h, s);
        cairo_surface_set_user_data (surface, &data_key, p, g_free);
        pixman_image_unref (dst);

        return surface;
}
}

/*
namespace MPX
{
namespace Util
{
void
cairo_image_surface_blur( Cairo::RefPtr<Cairo::ImageSurface>& s, double radius )
{
    radius = 2 ;
    cairo_surface_t* blurred = blur_image_surface( static_cast<cairo_surface_t*>(s->cobj()), radius, sigma ) ;
    s = Cairo::RefPtr<Cairo::ImageSurface>(new Cairo::ImageSurface( blurred, true )) ;
}
}}
*/

namespace MPX
{
namespace Util
{
void cairo_image_surface_blur( Cairo::RefPtr<Cairo::ImageSurface>& s, double radius )
{
    // Steve Hanov, 2009
    // Released into the public domain.

    cairo_surface_t* surface = s->cobj() ;
    
    // get width, height
    int width = cairo_image_surface_get_width( surface );
    int height = cairo_image_surface_get_height( surface );
    unsigned char* dst = (unsigned char*)malloc(width*height*4);
    unsigned* precalc = 
        (unsigned*)malloc(width*height*sizeof(unsigned));
    unsigned char* src = cairo_image_surface_get_data( surface );
    double mul=1.f/((radius*2)*(radius*2));
    int channel;
    
    // The number of times to perform the averaging. According to wikipedia,
    // three iterations is good enough to pass for a gaussian.
    const int MAX_ITERATIONS = 3; 
    int iteration;

    memcpy( dst, src, width*height*4 );

    for ( iteration = 0; iteration < MAX_ITERATIONS; iteration++ ) {
        for( channel = 0; channel < 4; channel++ ) {
            int x,y;

            // precomputation step.
            unsigned char* pix = src;
            unsigned* pre = precalc;

            pix += channel;
            for (y=0;y<height;y++) {
                for (x=0;x<width;x++) {
                    int tot=pix[0];
                    if (x>0) tot+=pre[-1];
                    if (y>0) tot+=pre[-width];
                    if (x>0 && y>0) tot-=pre[-width-1];
                    *pre++=tot;
                    pix += 4;
                }
            }

            // blur step.
            pix = dst + (int)radius * width * 4 + (int)radius * 4 + channel;
            for (y=radius;y<height-radius;y++) {
                for (x=radius;x<width-radius;x++) {
                    int l = x < radius ? 0 : x - radius;
                    int t = y < radius ? 0 : y - radius;
                    int r = x + radius >= width ? width - 1 : x + radius;
                    int b = y + radius >= height ? height - 1 : y + radius;
                    int tot = precalc[r+b*width] + precalc[l+t*width] - 
                        precalc[l+b*width] - precalc[r+t*width];
                    *pix=(unsigned char)(tot*mul);
                    pix += 4;
                }
                pix += (int)radius * 2 * 4;
            }
        }
        memcpy( src, dst, width*height*4 );
    }

    free( dst );
    free( precalc );
}}}
