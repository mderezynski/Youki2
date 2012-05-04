#ifndef MPX_CAIRO_BLUR_H
#define MPX_CAIRO_BLUR_H

#include <cairo.h>
#include <cairomm/cairomm.h>

namespace MPX
{
namespace Util
{
void
cairo_image_surface_blur( Cairo::RefPtr<Cairo::ImageSurface>& s, double radius ) ;
}}

#endif
