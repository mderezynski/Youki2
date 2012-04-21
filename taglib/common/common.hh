//
// (C) 2007 M. DEREZYNSKI
//

#ifndef MPX_METADATA_COMMON_HH
#define MPX_METADATA_COMMON_HH

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif //HAVE_CONFIG_H

#include <tfile.h>
#include "mpx/mpx-types.hh"

namespace MPX
{
    void  metadata_get_common (TagLib::File*, Track &);
    void  metadata_set_common (TagLib::File*, Track &);
    bool  metadata_check_file (TagLib::File*);
}

#endif // ! MPX_METADATA_COMMON_HH
