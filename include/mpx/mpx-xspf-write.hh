#ifndef MPX_XSPF_HH
#define MPX_XSPF_HH

#include "mpx/mpx-types.hh"

namespace MPX
{
    std::string
    XSPF_write(
	  const Track_sp_v&	/*tracklist*/
	, const std::string&	/*uuid*/
    ) ;

    void 
    XSPF_read(
	  const std::string&	 
	, const std::string&	
        , std::vector<guint>&
    ) ;
}

#endif
