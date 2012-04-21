#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif //HAVE_CONFIG_H

#include <glibmm.h>
#include <fileref.h>

// Plugin-specific include
#include "resolver.h"
#include "asffile.h"
#include "asftag.h"

#include "mpx/mpx-types.hh"
#include "common/common.hh"

extern "C" int  _plugin_has_accessors;
int  _plugin_has_accessors = 1;

extern "C" int  _plugin_version;
int _plugin_version = PLUGIN_VERSION;

extern "C" const char ** _mimetypes ()
{
  static const char * _types[] =
  {
    "video/x-ms-asf",
    "audio/x-ms-wma",
     NULL
  };
  return _types;
}

using namespace TagLib;
using namespace MPX;
using boost::get;

extern "C" bool _set (std::string const& filename, Track & track)
{
  TagLib::ASF::File opfile (filename.c_str());

  if (!metadata_check_file (&opfile))
    return false;

  TagLib::ASF::Tag * tag = dynamic_cast<TagLib::ASF::Tag*>(opfile.tag());

  if (tag)
  {
      metadata_set_common (&opfile, track);
      if (track[ATTRIBUTE_MB_TRACK_ID])
        tag->setAttribute("MusicBrainz/Track Id",
                           ASF::Attribute (TagLib::String(get<std::string>(track[ATTRIBUTE_MB_TRACK_ID].get()).c_str(),
                           TagLib::String::UTF8)));

      if (track[ATTRIBUTE_MB_ARTIST_ID])
        tag->setAttribute("MusicBrainz/Artist Id",
                           ASF::Attribute (TagLib::String(get<std::string>(track[ATTRIBUTE_MB_ARTIST_ID].get()).c_str(),
                           TagLib::String::UTF8)));

      if (track[ATTRIBUTE_MB_ALBUM_ID])
        tag->setAttribute("MusicBrainz/Album Id",
                           ASF::Attribute (TagLib::String(get<std::string>(track[ATTRIBUTE_MB_ALBUM_ID].get()).c_str(),
                           TagLib::String::UTF8)));

      if (track[ATTRIBUTE_ARTIST_SORTNAME])
        tag->setAttribute("WM/ArtistSortOrder",
                           ASF::Attribute (TagLib::String(get<std::string>(track[ATTRIBUTE_ARTIST_SORTNAME].get()).c_str(),
                           TagLib::String::UTF8)));

      if (track[ATTRIBUTE_ALBUM_ARTIST])
        tag->setAttribute("WM/AlbumArtist",
                           ASF::Attribute (TagLib::String(get<std::string>(track[ATTRIBUTE_MB_ALBUM_ARTIST_ID].get()).c_str(),
                           TagLib::String::UTF8)));

      if (track[ATTRIBUTE_MB_ALBUM_ARTIST_ID])
        tag->setAttribute("MusicBrainz/Album Artist Id",
                           ASF::Attribute (TagLib::String(get<std::string>(track[ATTRIBUTE_MB_ALBUM_ARTIST_ID].get()).c_str(),
                           TagLib::String::UTF8)));

      if (track[ATTRIBUTE_ALBUM_ARTIST_SORTNAME])
        tag->setAttribute("WM/AlbumArtistSortOrder",
                           ASF::Attribute (TagLib::String(get<std::string>(track[ATTRIBUTE_ALBUM_ARTIST_SORTNAME].get()).c_str(),
                           TagLib::String::UTF8)));

      if (track[ATTRIBUTE_MB_RELEASE_DATE])
        tag->setAttribute("WM/Year",
                           ASF::Attribute (TagLib::String(get<std::string>(track[ATTRIBUTE_MB_RELEASE_DATE].get()).c_str(),
                           TagLib::String::UTF8)));

      if (track[ATTRIBUTE_ASIN])
        tag->setAttribute("Amazon/ASIN",
                           ASF::Attribute (TagLib::String(get<std::string>(track[ATTRIBUTE_ASIN].get()).c_str(),
                           TagLib::String::UTF8)));

      if (track[ATTRIBUTE_MUSICIP_PUID])
        tag->setAttribute("MusicIP/PUID",
                           ASF::Attribute (TagLib::String(get<std::string>(track[ATTRIBUTE_MUSICIP_PUID].get()).c_str(),
                           TagLib::String::UTF8)));

      opfile.save();
      return true;
    }
  else
    {
      return false;
    }
}

extern "C" bool _get (std::string const& filename, Track & track)  
{
  TagLib::ASF::File opfile (filename.c_str());

  if (!metadata_check_file (&opfile))
    return false;

  metadata_get_common(&opfile, track);

  ASF::Tag* tag = dynamic_cast<ASF::Tag*>(opfile.tag());
  if (!tag)
    return false;

  TagLib::ASF::AttributeListMap const& attrMap = tag->attributeListMap();
  struct { 
      int         datum;
      char const* id;
  } mb_metadata_wma[] = {
      { ATTRIBUTE_MB_TRACK_ID,              "MusicBrainz/Track Id"              },
      { ATTRIBUTE_MB_ARTIST_ID,             "MusicBrainz/Artist Id"             },
      { ATTRIBUTE_MB_ALBUM_ID,              "MusicBrainz/Album Id"              },
      { ATTRIBUTE_ALBUM_ARTIST,             "WM/AlbumArtist"                    },
      { ATTRIBUTE_ALBUM_ARTIST_SORTNAME,    "WM/AlbumArtistSortOrder"           },
      { ATTRIBUTE_MB_ALBUM_ARTIST_ID,       "MusicBrainz/Album Artist Id"       },
      { ATTRIBUTE_MB_RELEASE_TYPE,          "MusicBrainz/Album Type"            },
      { ATTRIBUTE_MB_RELEASE_COUNTRY,       "MusicBrainz/Album Release Country" },
      { ATTRIBUTE_MB_RELEASE_DATE,          "WM/Year"                           },
      { ATTRIBUTE_ARTIST_SORTNAME,          "WM/ArtistSortOrder"                },
      { ATTRIBUTE_ASIN,                     "Amazon/ASIN"                       },
      { ATTRIBUTE_MUSICIP_PUID,             "MusicIP/PUID"                      },
  };

  //metadata_get_common (&opfile, row);

  for (unsigned int n = 0; n < G_N_ELEMENTS (mb_metadata_wma); ++n)
  {
        if (attrMap.contains(mb_metadata_wma[n].id))
          track[mb_metadata_wma[n].datum] = attrMap[mb_metadata_wma[n].id][0].toString().toCString(true);
  }

  return true;
}

static void _plugin_init (void) __attribute__((constructor));
static void _plugin_init (void)
{
  TagLib::FileRef::addFileTypeResolver(new ASFFileTypeResolver);
}


