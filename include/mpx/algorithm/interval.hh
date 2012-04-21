#ifndef _MPX_ALGORITHM__INTERVAL__HH
#define _MPX_ALGORITHM__INTERVAL__HH

#include <cstdlib>

namespace MPX
{
    template <typename T>
    class Interval
    {
        public:

            enum Type
            {
                  IN_IN
                , EX_EX
                , IN_EX
                , EX_IN
                , UNSET
            } ;

        protected:

            Type    t ;
            T       a ;
            T       b ;
        
        public:

            Interval(
            )
                : t( UNSET )
            {}

            Interval(
                  Type      t_
                , const T&  a_
                , const T&  b_
            )
                : t( t_ )
                , a( a_ )
                , b( b_ )
            {}

            inline bool
            in(
                  const T&  i
            )
            {
                switch( t )
                {
                    case IN_IN:
                        return i >= a && i <= b ;
                    case EX_EX:
                        return i >  a && i <  b ;
                    case IN_EX:
                        return i >= a && i <  b ;
                    case EX_IN:
                        return i >  a && i <= b ;

                    default:
                        return false ;
                } ;
            }
    } ;
}
#endif // _MPX_ALGORITHM__INTERVAL__HH
