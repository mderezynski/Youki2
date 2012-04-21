// (c) 2007 Milosz Derezynski
// See COPYING file for licensing information

#ifndef TAGLIB_ASFFILETYPERESOLVER_H
#define TAGLIB_ASFFILETYPERESOLVER_H

#include <tfile.h>
#include <fileref.h>

class ASFFileTypeResolver : public TagLib::FileRef::FileTypeResolver
{
    TagLib::File *createFile( const char *fileName,
                              bool readAudioProperties,
                              TagLib::AudioProperties::ReadStyle audioPropertiesStyle) const;

    virtual ~ASFFileTypeResolver () {}
};

#endif
