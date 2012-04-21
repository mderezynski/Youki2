//
// (C) 2007 M.Derezynski
//

#ifndef MPX_ID3V2_METADATA_READER_HH
#define MPX_ID3V2_METADATA_READER_HH

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif //HAVE_CONFIG_H

#include <id3v2tag.h>
#include "mpx/mpx-types.hh"

namespace MPX
{
    void  metadata_get_id3v2 (TagLib::ID3v2::Tag*, MPX::Track&);
}

#endif // ! MPX_ID3V2_METADATA_READER_HH
