// (c) 2005 Martin Aumueller <aumuell@reserv.at>
// See COPYING file for licensing information

#define MPX_PLUGIN_BUILD 1

#include "resolver.h"
#include "mp4file.h"

TagLib::File *MP4FileTypeResolver::createFile (const char*                        filename,
                                               bool                               read_properties,
                                               TagLib::AudioProperties::ReadStyle properties_style) const
{
    return new(std::nothrow)TagLib::MP4::File (filename, read_properties, properties_style);
    return 0;
}
