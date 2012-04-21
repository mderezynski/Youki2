// (c) 2007 Milosz Derezynski
// See COPYING file for licensing information

#define MPX_PLUGIN_BUILD 1

#include "resolver.h"
#include "asffile.h"

TagLib::File *ASFFileTypeResolver::createFile (const char                        *filename,
                                               bool                               read_properties,
                                               TagLib::AudioProperties::ReadStyle properties_style) const 
{
      TagLib::ASF::File * p = new TagLib::ASF::File (filename, read_properties, properties_style);

      if (p->isValid())
          return p;
      else
          delete p;

      return 0;
}
