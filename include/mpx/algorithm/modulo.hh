#ifndef _MPX_ALGORITHM__MODULO__HH
#define _MPX_ALGORITHM__MODULO__HH

#include <cstdlib>
#include <cmath>

namespace MPX
{
    template <typename T>
    class Modulo
    {
        protected:

            const T& v ;
            T        m ;
        
        public:

            Modulo(
                  const T&  v_
                , const T&  m_ 
            )
            : v( v_ )
            , m( m_ )
            {
            }

            operator T (
            ) const
            {
                return ( v % m ) ;
            }
    } ;

    template <>
    class Modulo<double>
    {
        protected:

            const double& v ;
            double        m ;
        
        public:

            Modulo(
                  const double&  v_
                , const double&  m_ 
            )
            : v( v_ )
            , m( m_ )
            {
            }

            operator double (
            ) const
            {
                return std::fmod( v, m ) ; 
            }
    } ;

}
#endif // _MPX_ALGORITHM__MODULO__HH
