#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif //HAVE_CONFIG_H

#include <glibmm.h>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

// Plugin-specific include
#include <taglib-gio.h>
#include <fileref.h>
#include <tfile.h>
#include <tag.h>
#include <oggfile.h>
#include <vorbisfile.h>
#include <vorbisproperties.h>
#include <xiphcomment.h>

#include <flacfile.h>
#include <mpegfile.h>
#include <id3v2tag.h>

#include "id3v2/reader.hh"
#include "xiph/reader.hh"
#include "common/common.hh"

using namespace TagLib;
using namespace MPX;
using boost::get;

extern "C" int  _plugin_has_accessors;
int  _plugin_has_accessors = 1;

extern "C" int  _plugin_version;
int _plugin_version = PLUGIN_VERSION;

extern "C" const char ** _mimetypes ()
{
  static const char * _types[] =
  {
    "audio/x-flac",
    NULL
  };
  return _types;
}

extern "C"
bool _set (std::string const& filename, Track & track)
{
    FLAC::File opfile( filename.c_str() ) ;

    if( !metadata_check_file( &opfile ))
    {
        return false ;
    }

    Ogg::XiphComment* comment = opfile.xiphComment() ;

    if( comment )
    {
        metadata_set_common( &opfile, track ) ;

        if (track[ATTRIBUTE_ALBUM_ARTIST])
          comment->addField (  String ("ALBUMARTIST", String::UTF8)
                             , String (get<std::string>(track[ATTRIBUTE_ALBUM_ARTIST].get())
                             , String::UTF8)) ;

        if (track[ATTRIBUTE_ALBUM_ARTIST_SORTNAME])
          comment->addField( String ("ALBUMARTISTSORT", String::UTF8)
                           , String (get<std::string>(track[ATTRIBUTE_ALBUM_ARTIST_SORTNAME].get())
                           , String::UTF8)) ;

        if (track[ATTRIBUTE_MB_ALBUM_ARTIST_ID])
          comment->addField( String ("MUSICBRAINZ_ALBUMARTISTID", String::UTF8)
                           , String (get<std::string>(track[ATTRIBUTE_MB_ALBUM_ARTIST_ID].get())
                           , String::UTF8)) ;

        if (track[ATTRIBUTE_MB_TRACK_ID])
          comment->addField( String ("MUSICBRAINZ_TRACKID", String::UTF8)
                           , String (get<std::string>(track[ATTRIBUTE_MB_TRACK_ID].get())
                           , String::UTF8)) ;

        if (track[ATTRIBUTE_MB_ALBUM_ID])
          comment->addField( String ("MUSICBRAINZ_ALBUMID", String::UTF8)
                           , String(get<std::string>(track[ATTRIBUTE_MB_ALBUM_ID].get())
                           , String::UTF8)) ;

        if (track[ATTRIBUTE_MB_ARTIST_ID])
          comment->addField( String ("MUSICBRAINZ_ARTISTID", String::UTF8)
                           , String (get<std::string>(track[ATTRIBUTE_MB_ARTIST_ID].get())
                           , String::UTF8)) ;

        if (track[ATTRIBUTE_ARTIST_SORTNAME])
          comment->addField( String ("ARTISTSORT", String::UTF8)
                           , String (get<std::string>(track[ATTRIBUTE_ARTIST_SORTNAME].get())
                           , String::UTF8)) ;

        if (track[ATTRIBUTE_MB_RELEASE_DATE])
          comment->addField( String ("DATE", String::UTF8)
                           , String (get<std::string>(track[ATTRIBUTE_MB_RELEASE_DATE].get())
                           , String::UTF8)) ;

        if (track[ATTRIBUTE_ASIN])
          comment->addField( String ("ASIN", String::UTF8)
                           , String (get<std::string>(track[ATTRIBUTE_ASIN].get())
                           , String::UTF8)) ;

        if (track[ATTRIBUTE_LABEL])
          comment->addField( String ("LABEL", String::UTF8)
                           , String (get<std::string>(track[ATTRIBUTE_LABEL].get())
                           , String::UTF8)) ;

        if (track[ATTRIBUTE_MUSICIP_PUID])
          comment->addField( String ("MUSICIP_PUID", String::UTF8)
                           , String (get<std::string>(track[ATTRIBUTE_MUSICIP_PUID].get())
                           , String::UTF8)) ;

        opfile.save() ;
        return true;
    }
    else
    {
      return false;
    }
}

extern "C"
bool _get(
      const std::string&    filename
    , Track&                track
) 
{
    FLAC::File opfile( filename.c_str() ) ;

    if( !metadata_check_file( &opfile ))
    {
        return false;
    }

    Ogg::XiphComment* comment = opfile.xiphComment() ;

    ID3v2::Tag* tag = opfile.ID3v2Tag( false ) ;

    if( comment )
    {
        metadata_get_xiph (comment, track) ;
        metadata_get_common (&opfile, track) ;
        return true;
    }

    if( tag )
    {
        metadata_get_id3v2 (tag, track) ;
        metadata_get_common (&opfile, track) ;
        return true;
    }

    return false;
}
