#ifndef MPX_CAIRO_BLUR_H
#define MPX_CAIRO_BLUR_H

#include <cairo.h>

namespace MPX { namespace Util {
void cairo_image_surface_blur( cairo_surface_t* surface, double radius ) ;
}}

#endif
