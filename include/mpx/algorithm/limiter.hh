#ifndef _MPX_ALGORITHM__LIMITER__HH
#define _MPX_ALGORITHM__LIMITER__HH

#include <cstdlib>

namespace MPX
{
    template <typename T>
    class Limiter
    {
        public:

            enum Type
            {
                  ABS_ABS
                , REL_REL
                , ABS_REL
                , REL_ABS
            } ;

        protected:

            T v ;
        
        public:

            Limiter(
            )
            {}

            Limiter(
                  Type      type
                , const T&  min
                , const T&  max
                , const T&  v_
            )
            {
                switch( type )
                { 
                    case ABS_ABS:
                        v = std::min( std::max( min, v_ ), max ) ;
                        break; 

                    case REL_REL:
                        v = std::min( std::max( v_ + min, v_ ), v_ + max ) ;
                        break; 

                    case ABS_REL:
                        v = std::min( std::max( min, v_ ), v_ + max ) ;
                        break; 

                    case REL_ABS:
                        v = std::min( std::max( v_ + min, v_ ), max ) ;
                        break; 

                    default: v = T() ; 
                }
            }

            operator const T& (
            ) const
            {
                return v ;
            }
    } ;
}
#endif // _MPX_ALGORITHM__LIMITER__HH
