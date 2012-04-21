// (C) 2006 M. Derezynski

#ifndef TAGLIB_SIDFILETYPERESOLVER_H
#define TAGLIB_SIDFILETYPERESOLVER_H

#include <tfile.h>
#include <fileref.h>

class SIDFileTypeResolver : public TagLib::FileRef::FileTypeResolver
{
        TagLib::File *createFile(const char                         *filename,
                             bool                               readAudioProperties,
                             TagLib::AudioProperties::ReadStyle audioPropertiesStyle) const;
        virtual ~SIDFileTypeResolver () {}
};

#endif
