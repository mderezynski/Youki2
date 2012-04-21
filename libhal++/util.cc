#include <cstdlib>
#include <string>
#include <cstring>

#include "types.hh"

namespace Hal
{
  namespace Util
  {
    std::string
    wrap_string (const char * v)
    {
      return v ? std::string(v) : std::string();
    }

    std::string
    wrap_string (char * v)
    {
      std::string r (v ? std::string(v) : std::string());
      free (v);
      return r;
    }
  }

  StrV::StrV (char ** v)
  {
    char ** p = v;
    for ( ; *v ; ++v)
      push_back (std::string (*v));
    libhal_free_string_array (p);
  }

  StrV::StrV (const char ** v)
  {
    for ( ; *v ; ++v)
      push_back (std::string (*v));
  }
}
