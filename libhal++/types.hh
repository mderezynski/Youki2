//
// libhal++ (C) GPL 2006 M. Derezynski
//

#ifndef _HAL_CC_TYPES_HH_
#define _HAL_CC_TYPES_HH_

#include <libhal.h>
#include <vector>
#include <string>

namespace Hal
{
  struct StrV : public std::vector<std::string>
  {
    explicit StrV () {}

    /** Wraps a C array of C char arrays (strings)
      * NOTE: The input data will be free'd after wrapping
      * 
      * @param v The char string array to wrap
      */
    explicit StrV (char ** v);

    /** Wraps a C array of C char arrays (strings)
      *
      * @param v The char string array to wrap
      */
    explicit StrV (const char ** v);
  };

  enum PropertyType
  {
    // Used to report error condition
    PROPERTY_TYPE_INVALID   =  LIBHAL_PROPERTY_TYPE_INVALID,

    // Type for 32-bit signed integer property
    PROPERTY_TYPE_INT32     =  LIBHAL_PROPERTY_TYPE_INT32,

    // Type for 64-bit unsigned integer property
    PROPERTY_TYPE_UINT64    =  LIBHAL_PROPERTY_TYPE_UINT64,

    // Type for double precision floating point property
    PROPERTY_TYPE_DOUBLE    =  LIBHAL_PROPERTY_TYPE_DOUBLE,

    // Type for boolean property
    PROPERTY_TYPE_BOOLEAN   =  LIBHAL_PROPERTY_TYPE_BOOLEAN,

    // Type for UTF-8 string property
    PROPERTY_TYPE_STRING    =  LIBHAL_PROPERTY_TYPE_STRING,

    // Type for list of UTF-8 strings property
    PROPERTY_TYPE_STRLIST   =  LIBHAL_PROPERTY_TYPE_STRLIST
  };
}
#endif //!_HAL_CC_TYPES_HH_
