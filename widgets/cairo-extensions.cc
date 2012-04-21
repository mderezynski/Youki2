#include <cairomm/cairomm.h>
#include <cmath>
#include <glibmm.h>
#include "mpx/widgets/cairo-extensions.hh"

namespace MPX        
{
    void RoundedRectangle (RefContext cr, double x, double y, double w, double h,
        double r, CairoCorners::CORNERS corners)
    {
        if(r < 0.0001 || corners == CairoCorners::NONE) {
            cr->rectangle(x, y, w, h);
            return;
        }

        if((corners & CairoCorners::TOPLEFT) != 0) {
            cr->move_to(x + r, y);
        } else {
            cr->move_to(x, y);
        }

        if((corners & CairoCorners::TOPRIGHT) != 0) {
            cr->arc(x + w - r, y + r, r, M_PI * 1.5, M_PI * 2);
        } else {
            cr->line_to(x + w, y);
        }

        if((corners & CairoCorners::BOTTOMRIGHT) != 0) {
            cr->arc(x + w - r, y + h - r, r, 0, M_PI * 0.5);
        } else {
            cr->line_to(x + w, y + h);
        }

        if((corners & CairoCorners::BOTTOMLEFT) != 0) {
            cr->arc(x + r, y + h - r, r, M_PI * 0.5, M_PI);
        } else {
            cr->line_to(x, y + h);
        }

        if((corners & CairoCorners::TOPLEFT) != 0) {
            cr->arc(x + r, y + r, r, M_PI, M_PI * 1.5);
        } else {
            cr->line_to(x, y);
        }
    }
}
