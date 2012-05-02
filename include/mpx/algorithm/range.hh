#ifndef _MPX_ALGORITHM__RANGE__HH
#define _MPX_ALGORITHM__RANGE__HH

#include <cstdlib>
#include <cmath>

namespace MPX
{
    template <typename T>
    class Range
    {
        protected:

            T a ;
            T b ;
        
        public:

            Range(
            )
            {}

            Range(
                  const T&  a_
                , const T&  b_
            )
            : a( a_ )
            , b( b_ )
            {}

            inline bool
            operator()(
                  const T&  i
            ) const
            {
		return i >= a && i <= b ;
            }

	    inline T
	    size(
	    ) const
	    {
		return abs( a - b ) ;
	    }

	    inline T
	    upper(
	    ) const
	    {
		return a ;
	    }

	    inline T
	    lower(
	    ) const
	    {
		return b ;
	    }
    } ;

    inline bool operator<( guint a, const Range<guint>& b )
    {
	return a < b.upper() ;
    }

    inline bool operator>( guint a, const Range<guint>& b )
    {
	return a > b.lower() ;
    }

    inline bool operator<=( guint a, const Range<guint>& b )
    {
	return a <= b.upper() ;
    }

    inline bool operator>=( guint a, const Range<guint>& b )
    {
	return a >= b.lower() ;
    }
}
#endif // _MPX_ALGORITHM__RANGE__HH
