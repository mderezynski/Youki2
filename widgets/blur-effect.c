#include <gtk/gtk.h>
#include <math.h>
#include <pixman.h>

static int        radius  = 3;
static double     offset  = 2.0;
static double     sigma   = 3.0;
static double     alpha   = 0.8;
static double     red     = 0.0;
static double     green   = 0.0;
static double     blue    = 0.0;

/* G(x,y) = 1/(2 * PI * sigma^2) * exp(-(x^2 + y^2)/(2 * sigma^2))
 */
static pixman_fixed_t *
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

static cairo_surface_t *
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

        g_return_val_if_fail
          (cairo_surface_get_type (surface) != CAIRO_SURFACE_TYPE_IMAGE,
           NULL);

        w = cairo_image_surface_get_width (surface);
        h = cairo_image_surface_get_height (surface);
        s = cairo_image_surface_get_stride (surface);

        /* create pixman image for cairo image surface */
        p = cairo_image_surface_get_data (surface);
        src = pixman_image_create_bits (PIXMAN_a8, w, h, p, s);

        /* attach gaussian kernel to pixman image */
        params = create_gaussian_blur_kernel (radius, sigma, &n_params);
        pixman_image_set_filter (src, PIXMAN_FILTER_CONVOLUTION, params, n_params);
        g_free (params);

        /* render blured image to new pixman image */
        p = g_malloc0 (s * h);
        dst = pixman_image_create_bits (PIXMAN_a8, w, h, p, s);
        pixman_image_composite (PIXMAN_OP_SRC, src, NULL, dst, 0, 0, 0, 0, 0, 0, w, h);
        pixman_image_unref (src);

        /* create new cairo image for blured pixman image */
        surface = cairo_image_surface_create_for_data (p, CAIRO_FORMAT_A8, w, h, s);
        cairo_surface_set_user_data (surface, &data_key, p, g_free);
        pixman_image_unref (dst);

        return surface;
}
