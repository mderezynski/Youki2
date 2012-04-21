#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif //HAVE_CONFIG_H

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <glibmm.h>

// Plugin-specific include
#include <taglib-gio.h>
#include <fileref.h>
#include <tfile.h>
#include <tag.h>

#include <id3v2tag.h>
#include <mpegfile.h>
#include <id3v2framefactory.h>
#include <textidentificationframe.h>
#include <uniquefileidentifierframe.h>

#include "id3v2/reader.hh"
#include "common/common.hh"

using namespace MPX;
using namespace TagLib;
using namespace Glib;
using boost::get;

extern "C" int  _plugin_has_accessors;
int _plugin_has_accessors = 1;

extern "C" int  _plugin_version;
int _plugin_version = PLUGIN_VERSION;

extern "C" const char ** _mimetypes ()
{
  static const char * _types[] =
  {
    "application/x-id3",
    "audio/mpeg",
    NULL
  };

  return _types;
}

namespace
{
  // Based on SoundJuicer code, (C) Ross Burton <ross@burtonini.com> 
  void
  add_txxx_tag (TagLib::ID3v2::Tag    * id3v2tag,
                ustring const&    spec_id,
                ustring const&    idstring) 
  {
    ID3v2::UserTextIdentificationFrame * frame;

    if (idstring.empty())
      return;

    if (spec_id.empty())
      return;

    id3v2tag->removeFrames (spec_id.c_str());
    frame = new ID3v2::UserTextIdentificationFrame (String::UTF8);
    id3v2tag->addFrame (frame);
    frame->setDescription (String (spec_id.c_str(), String::UTF8));
    frame->setText (String (idstring.c_str(), String::UTF8));
  }
}

extern "C" bool _set (std::string const& filename, Track & track)
{
  ID3v2::FrameFactory *factory = ID3v2::FrameFactory::instance();
  factory->setDefaultTextEncoding (String::UTF8);

  MPEG::File opfile (filename.c_str(), factory);
  if (!metadata_check_file (&opfile))
    return false;

  opfile.strip ();

  ID3v2::Tag * tag = opfile.ID3v2Tag (true);
  if (!tag)
    return false;

  metadata_set_common (&opfile, track);
   
  ID3v2::TextIdentificationFrame *frame = 0;

  if (track[ATTRIBUTE_ALBUM_ARTIST])
  {
    frame = new ID3v2::TextIdentificationFrame ("TPE2", String::UTF8);
    frame->setText (String (get<std::string>(track[ATTRIBUTE_ALBUM_ARTIST].get()).c_str(), String::UTF8));
    tag->addFrame (frame);
  }

  if (track[ATTRIBUTE_MB_ALBUM_ARTIST_ID])
  {
    add_txxx_tag (tag,
                  "MusicBrainz Album Artist Id", 
                  get<std::string>(track[ATTRIBUTE_MB_ALBUM_ARTIST_ID].get()));
  }

  if (track[ATTRIBUTE_MB_ALBUM_ID])
  {
    add_txxx_tag (tag,
                  "MusicBrainz Album Id", 
                  get<std::string>(track[ATTRIBUTE_MB_ALBUM_ID].get()));
  }

  if (track[ATTRIBUTE_MB_ARTIST_ID])
  {
    add_txxx_tag (tag,
                  "MusicBrainz Artist Id", 
                  get<std::string>(track[ATTRIBUTE_MB_ARTIST_ID].get()));
  }

  if (track[ATTRIBUTE_ALBUM_ARTIST_SORTNAME])
  {
    add_txxx_tag (tag,
                  "ALBUMARTISTSORT", 
                  get<std::string>(track[ATTRIBUTE_ALBUM_ARTIST_SORTNAME].get()));
  }

  if (track[ATTRIBUTE_ASIN])
  {
    add_txxx_tag (tag,
                  "ASIN", 
                  get<std::string>(track[ATTRIBUTE_ASIN].get()));
  }

  if (track[ATTRIBUTE_MUSICIP_PUID])
  {
    add_txxx_tag (tag,
                  "MusicIP PUID", 
                  get<std::string>(track[ATTRIBUTE_MUSICIP_PUID].get()));
  }

  if (track[ATTRIBUTE_MB_TRACK_ID])
  {
    ID3v2::UniqueFileIdentifierFrame *frame = new ID3v2::UniqueFileIdentifierFrame ("http://musicbrainz.org", get<std::string>(track[ATTRIBUTE_MB_TRACK_ID].get()).c_str());
    tag->addFrame (frame);
  }

  if (track[ATTRIBUTE_MB_RELEASE_DATE])
  {
    frame = new ID3v2::TextIdentificationFrame ("TDRL", String::UTF8);
    frame->setText (String (get<std::string>(track[ATTRIBUTE_MB_RELEASE_DATE].get()).c_str(), String::UTF8));
    tag->addFrame (frame);

    frame = new ID3v2::TextIdentificationFrame ("TDRC", String::UTF8);
    frame->setText (String (get<std::string>(track[ATTRIBUTE_MB_RELEASE_DATE].get()).c_str(), String::UTF8));
    tag->addFrame (frame);
  }

  if (track[ATTRIBUTE_ARTIST_SORTNAME])
  {
    frame = new ID3v2::TextIdentificationFrame ("TSOP", String::UTF8);
    frame->setText (String (get<std::string>(track[ATTRIBUTE_ARTIST_SORTNAME].get()).c_str(), String::UTF8));
    tag->addFrame (frame);
  }

  opfile.save (TagLib::MPEG::File::ID3v2);
  return true;
}

extern "C" bool _get (std::string const& filename, Track & track)  
{
  try{
        MPEG::File opfile ( filename.c_str() ) ;

        if( !metadata_check_file( &opfile ))
        {
            return false;
        }

        ID3v2::Tag * tag = opfile.ID3v2Tag (false);

        if( tag )
        {
              metadata_get_id3v2 (tag, track);
        }

        track [ ATTRIBUTE_TRACK ] = guint (opfile.tag()->track());
        track [ ATTRIBUTE_DATE ] = guint (opfile.tag()->year());

        std::string value; 

        value = opfile.tag()->artist().to8Bit(true);
        if (!value.empty())
        {
            track [ ATTRIBUTE_ARTIST ] = value;
        }

        value = opfile.tag()->album().to8Bit(true);
        if (!value.empty())
        {
            track [ ATTRIBUTE_ALBUM ] = value;
        }

        value = opfile.tag()->genre().to8Bit(true);
        if (!value.empty())
        {
            track [ ATTRIBUTE_GENRE ] = value;
        }

        value = opfile.tag()->comment().to8Bit(true);
        if (!value.empty())
        {
            track [ ATTRIBUTE_COMMENT ] = value;
        }

        if (opfile.audioProperties())
        {
            track [ ATTRIBUTE_BITRATE ] = guint (opfile.audioProperties()->bitrate());
            track [ ATTRIBUTE_SAMPLERATE ] = guint (opfile.audioProperties()->sampleRate());
            track [ ATTRIBUTE_TIME ] = guint (opfile.audioProperties()->length());
        }
        else
        {
            track [ ATTRIBUTE_BITRATE ] = guint (0);
            track [ ATTRIBUTE_SAMPLERATE ] = guint (0);
            track [ ATTRIBUTE_TIME ] = guint (0);
        }

        track [ ATTRIBUTE_TITLE ] = opfile.tag()->title().to8Bit(true);

    } catch( std::bad_alloc )
    {
        return false;
    }

  return true;
}
