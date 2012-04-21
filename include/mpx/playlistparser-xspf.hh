// (c) 2007 M. Derezynski

#ifndef MPX_I_PLAYLIST_PARSER_XSPF_HH
#define MPX_I_PLAYLIST_PARSER_XSPF_HH

#include <string>
#include "mpx/mpx-types.hh"
#include "mpx/i-playlistparser.hh"
#include "mpx/util-file.hh"

namespace MPX
{
namespace PlaylistParser
{
    class XSPF
		:	public PlaylistParser::Base
    {
        public:

          XSPF ();
          virtual ~XSPF (); 

          virtual bool
          read( 
            const std::string&,
            Track_v&,
            bool
          );

          virtual bool
          write(
            const std::string&,
            const Track_v&
          );
    };
}
}

#endif // MPX_I_PLAYLIST_PARSER_HH
