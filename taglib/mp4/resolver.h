// (c) 2005 Martin Aumueller <aumuell@reserv.at>
// See COPYING file for licensing information

#ifndef TAGLIB_MP4FILETYPERESOLVER_H
#define TAGLIB_MP4FILETYPERESOLVER_H

#include <tfile.h>
#include <fileref.h>

class MP4FileTypeResolver : public TagLib::FileRef::FileTypeResolver
{
        TagLib::File *createFile(char const * /* filename */,
                                 bool /* read audio props? */,
                                 TagLib::AudioProperties::ReadStyle /* read audio props style*/) const;
        virtual ~MP4FileTypeResolver () {}
};

#endif
