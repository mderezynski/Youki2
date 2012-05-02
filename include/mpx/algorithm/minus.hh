#ifndef _MPX_ALGORITHM__MINUS__HH
#define _MPX_ALGORITHM__MINUS__HH

#include <cstdlib>

namespace MPX
{
    template <typename T>
    class Minus
    {
            T neg ;
        
        public:

	    Minus()
	    {}

            Minus(
		T n
            ) : neg(n)
            {}

	    T operator()( T v )
	    {
		return v - neg ;	
	    }
    } ;
}
#endif // _MPX_ALGORITHM__MINUS__HH
