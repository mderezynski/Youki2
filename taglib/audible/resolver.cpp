// (c) 2005 Martin Aumueller <aumuell@reserv.at>
// See COPYING file for licensing information

#include <glibmm.h>
#include <src/util_string.hh>

#include "resolver.h"
#include "taglib_audiblefile.h"

namespace 
{
  char* const suffixes[] =
  {
    "aa",
    0,
  };
}

TagLib::File *AudibleFileTypeResolver::createFile(const char *filename,
        bool readProperties,
        TagLib::AudioProperties::ReadStyle propertiesStyle) const
{
  if (Bmp::Util::str_has_suffixes_nocase (filename, suffixes))
    {
        FILE *fp = fopen(filename, "rb");
        if(!fp)
            return 0;

        return new TagLib::Audible::File(filename, readProperties, propertiesStyle, fp);
    }

    return 0;
}
