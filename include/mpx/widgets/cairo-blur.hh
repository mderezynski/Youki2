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

void
render_text_shadow(	
      Glib::RefPtr<Pango::Layout>	   layout
    , guint x
    , guint y
    , const Cairo::RefPtr<Cairo::Context>& cairo
    , double radius = 1.
    , double alpha = 0.45
) ;

}}

#endif
