// (C) 2006 M. Derezynski

#ifndef TAGLIB_MODFILETYPERESOLVER_H
#define TAGLIB_MODFILETYPERESOLVER_H

#include <tfile.h>
#include <fileref.h>

class MODFileTypeResolver : public TagLib::FileRef::FileTypeResolver
{
    TagLib::File *createFile(const char                         *filename,
                             bool                               readAudioProperties,
                             TagLib::AudioProperties::ReadStyle audioPropertiesStyle) const;
    virtual ~MODFileTypeResolver () {}
};

#endif
