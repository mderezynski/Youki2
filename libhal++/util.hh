#ifndef _HAL_CC_UTIL_HH_
#define _HAL_CC_UTIL_HH_

#include <string>
#include <cstring>

namespace Hal
{
  namespace Util
  {
    /** \brief Wraps a C const string
      *
      * @param v C char array to wrap
      * @returns An std::string with the contents of the C char array
      */
    std::string wrap_string (const char * v);

    /** \brief Wraps a C string.
      *
      * NOTE: The string will be free'd as it's not const.
      * Normally the compiler will decide which variant 
      * to call but with parts of the libhal API
      * it is neccessary to use a const_cast<>
      *
      * @param v C char array to wrap
      * @returns An std::string with the contents of the C char array
      */
    std::string wrap_string (char * v);
  }
}

#endif //!_HAL_CC_UTIL_HH_
