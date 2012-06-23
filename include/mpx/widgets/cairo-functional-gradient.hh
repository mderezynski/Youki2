#ifndef MPX_CAIRO_FUNCTIONAL_GRADIENT_HH
#define MPX_CAIRO_FUNCTIONAL_GRADIENT_HH

#include <cairomm/cairomm.h>
#include <gdkmm.h>
#include <stdexcept>
#include <cmath>

namespace MPX
{
    typedef sigc::slot<Gdk::RGBA, double> SlotRGBA ;

    class CairoFunctionalGradient
    {
	public:

	    CairoFunctionalGradient( SlotRGBA& slot )
	    : slot_(slot)
	    , m_sampleStepping( 0.05 )
	    {}

	    virtual ~CairoFunctionalGradient()
	    {}

	    Cairo::RefPtr<Cairo::LinearGradient>
	    apply()
	    {
		Cairo::RefPtr<Cairo::LinearGradient> source = Cairo::LinearGradient::create( m_cStart.x, m_cStart.y, m_cStop.x, m_cStop.y ) ;

		for( double n = 0 ; n <= 1 ; n += m_sampleStepping )
		{
		    Gdk::RGBA c = slot_( n ) ;

		    source->add_color_stop_rgba(
			  n
			, c.get_red()
			, c.get_green()
			, c.get_blue()			
			, c.get_alpha()
		    ) ;
		}

		return source ;
	    }

	    void 
	    set_target_surface(
		  Cairo::RefPtr<Cairo::Surface>	surface
	    )
	    {
		m_targetSurface = surface ;
	    }

	    void
	    set_start(
		  int x
		, int y
	    )
	    {
		m_cStart.x = x ;
		m_cStart.y = y ;
	    }

	    void
	    set_stop(
		  int x
		, int y
	    )
	    {
		m_cStop.x = x ;
		m_cStop.y = y ;
	    }

	    void
	    set_sample_stepping(
	        double n
	    )
	    {
		m_sampleStepping = n ;
	    }

	private:

	    struct Coords
	    {
		int x ;
		int y ;
	    } :

	    Coords m_cStart ;
	    Coords m_cStop ;

	    Cairo::RefPtr<Cairo::Surface> m_targetSurface ;	

	    SlotRGBA slot_ ;

	    double m_sampleStepping ;
    };
}

#endif
