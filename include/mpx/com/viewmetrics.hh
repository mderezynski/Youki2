#pragma once

#ifndef MPX_VIEW_METRICS_HH
#define MPX_VIEW_METRICS_HH

#include <glib.h>
#include "mpx/algorithm/range.hh"

namespace MPX
{
	class ViewMetrics_type
	{
	    public:

		guint		RowHeight ;
		guint		Height ;	
		guint		Excess ;
		Range<guint>	ViewPortPx ;
		Range<guint>	ViewPort ;

	    public:

		ViewMetrics_type()
		    : RowHeight(0)
		    , Height(0)
		    , Excess(0)
		    , ViewPortPx(0,0)
		    , ViewPort(0,0)
		{}

		virtual ~ViewMetrics_type()
		{}

		void
		set_base__row_height(
		      guint v_
		)	
		{
		    RowHeight = v_ ;
		}

		void
		set(
		      guint height
		    , guint top
		)
		{
		    Height = height ;

		    ViewPortPx = Range<guint>( top, top+height ) ;

		    if( RowHeight )
		    {
			ViewPort = Range<guint>( ViewPortPx.upper()/RowHeight, ViewPortPx.upper()/RowHeight + Height/RowHeight ) ;
			Excess = ViewPortPx.size() - (ViewPortPx.size()/RowHeight)*RowHeight ;
		    }	
		}
	} ;
}

#endif
