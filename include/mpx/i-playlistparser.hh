// (c) 2007 M. Derezynski

#ifndef MPX_I_PLAYLIST_PARSER_HH
#define MPX_I_PLAYLIST_PARSER_HH

#include <string>
#include "mpx/mpx-types.hh"
#include "mpx/util-file.hh"

namespace MPX
{
    typedef std::vector<MPX::Track> Track_v;

namespace PlaylistParser
{
    class Base
    {
        public:

          Base () {}
          virtual ~Base () {}

          virtual bool
          read(
            std::string const&,
            Track_v&,
            bool = true
          ) = 0;

          virtual bool
          write(
            std::string const&,
            Track_v const&
          ) = 0;
    };
}
}

#endif // MPX_I_PLAYLIST_PARSER_HH
