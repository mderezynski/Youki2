#ifndef MPX_COLOR_FADE_HH
#define MPX_COLOR_FADE_HH

#include "mpx/i-youki-theme-engine.hh"

namespace MPX
{
    class ColorFade
    {
	protected:
	ThemeColor color_a ;
	ThemeColor color_b ;

	public:
	ColorFade(const MPX::ThemeColor& c_a, const MPX::ThemeColor& c_b)
	: color_a(c_a)
	, color_b(c_b)
	{}

	ThemeColor operator() (double p) const
	{
	    MPX::ThemeColor r ;
	    r.set_red   (color_a.get_red()   + (color_b.get_red()   - color_a.get_red())   * p);
	    r.set_green (color_a.get_green() + (color_b.get_green() - color_a.get_green()) * p);
	    r.set_blue  (color_a.get_blue()  + (color_b.get_blue()  - color_a.get_blue())  * p);
	    return r ;
	}
    } ;
}

#endif // MPX_COLOR_FADE_HH
