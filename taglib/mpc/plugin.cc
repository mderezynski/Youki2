#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif //HAVE_CONFIG_H

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <glibmm.h>

#include <taglib-gio.h>
#include <fileref.h>
#include <tfile.h>
#include <tag.h>
#include <mpcfile.h>
#include <apetag.h>
#include <apeitem.h>
#include <apefooter.h>

#include "common/common.hh"
#include "mpx/mpx-types.hh"

using namespace MPX;
using namespace TagLib;
using namespace Glib;
using boost::get;

extern "C" int  _plugin_has_accessors;
int  _plugin_has_accessors = 1;

extern "C" int  _plugin_version;
int _plugin_version = PLUGIN_VERSION;

extern "C" const char ** _mimetypes ()
{
  static const char * _types[] =
  {
    "application/x-apetag",
    NULL
  };
  return _types;
}

extern "C" bool _set (std::string const& filename, Track & track)
{
  MPC::File opfile (filename.c_str());
  if (!metadata_check_file (&opfile))
    return false;

  APE::Tag * tag = opfile.APETag(true);
  if (tag)
  {
      metadata_set_common (&opfile, track);

      if (track[ATTRIBUTE_ALBUM_ARTIST])
        tag->addValue ("Album Artist",
                        String (get<std::string>(track[ATTRIBUTE_ALBUM_ARTIST].get()),           String::UTF8));

      if (track[ATTRIBUTE_MB_ALBUM_ARTIST_ID])
        tag->addValue ("MUSICBRAINZ_ALBUMARTISTID",
                        String (get<std::string>(track[ATTRIBUTE_MB_ALBUM_ARTIST_ID].get()),        String::UTF8));

      if (track[ATTRIBUTE_ALBUM_ARTIST_SORTNAME])
        tag->addValue ("ALBUMARTISTSORT",
                        String (get<std::string>(track[ATTRIBUTE_ALBUM_ARTIST_SORTNAME].get()),  String::UTF8));

      if (track[ATTRIBUTE_MB_TRACK_ID])
        tag->addValue ("MUSICBRAINZ_TRACKID",
                        String (get<std::string>(track[ATTRIBUTE_MB_TRACK_ID].get()),               String::UTF8));

      if (track[ATTRIBUTE_MB_ALBUM_ID])
        tag->addValue ("MUSICBRAINZ_ALBUMID",
                        String(get<std::string>(track[ATTRIBUTE_MB_ALBUM_ID].get()),                String::UTF8));

      if (track[ATTRIBUTE_MB_ARTIST_ID])
        tag->addValue ("MUSICBRAINZ_ARTISTID",
                        String (get<std::string>(track[ATTRIBUTE_MB_ARTIST_ID].get()),              String::UTF8));

      if (track[ATTRIBUTE_ARTIST_SORTNAME])
        tag->addValue ("ARTISTSORT",
                        String (get<std::string>(track[ATTRIBUTE_ARTIST_SORTNAME].get()),        String::UTF8));

      if (track[ATTRIBUTE_MB_RELEASE_DATE])
        tag->addValue ("Year",
                        String (get<std::string>(track[ATTRIBUTE_MB_RELEASE_DATE].get()),           String::UTF8));

      if (track[ATTRIBUTE_ASIN])
        tag->addValue ("ASIN",
                        String (get<std::string>(track[ATTRIBUTE_ASIN].get()),                      String::UTF8));

      if (track[ATTRIBUTE_MUSICIP_PUID])
        tag->addValue ("MUSICIP_PUID",
                        String (get<std::string>(track[ATTRIBUTE_MUSICIP_PUID].get()),              String::UTF8));

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
  struct {
      int         datum;
      char const* id;
  } mb_metadata_ape[] = {
    { ATTRIBUTE_ALBUM_ARTIST,           "Album Artist"                  },
    { ATTRIBUTE_ALBUM_ARTIST_SORTNAME,  "ALBUMARTISTSORT"               }, 
    { ATTRIBUTE_MB_ALBUM_ARTIST_ID,     "MUSICBRAINZ_ALBUMARTISTID"     },
    { ATTRIBUTE_MB_TRACK_ID,            "MUSICBRAINZ_TRACKID"           },
    { ATTRIBUTE_MB_ALBUM_ID,            "MUSICBRAINZ_ALBUMID"           },
    { ATTRIBUTE_ARTIST_SORTNAME,        "ARTISTSORT"                    },
    { ATTRIBUTE_MB_ARTIST_ID,           "MUSICBRAINZ_ARTISTID"          },
    { ATTRIBUTE_MB_RELEASE_DATE,        "Year"                          },
    { ATTRIBUTE_ASIN,                   "ASIN"                          },
    { ATTRIBUTE_MUSICIP_PUID,           "MUSICIP_PUID"                  },
    { ATTRIBUTE_MB_RELEASE_COUNTRY,     "RELEASECOUNTRY"                },
    { ATTRIBUTE_MB_RELEASE_TYPE,        "MUSICBRAINZ_ALBUMTYPE"         }
  };


  MPC::File opfile (filename.c_str());
  if (!metadata_check_file (&opfile))
    return false;

  APE::Tag * tag = opfile.APETag();
  if (tag)
  {
      metadata_get_common (&opfile, track);
      TagLib::APE::ItemListMap const& items (tag->itemListMap());
      for (unsigned int n = 0; n < G_N_ELEMENTS (mb_metadata_ape); ++n)
      {
        if (!items[mb_metadata_ape[n].id].isEmpty())
        {
          track[mb_metadata_ape[n].datum] = items[mb_metadata_ape[n].id].toString().toCString(true);
        }
      }
      return true;
  }
  else
  {
      return false;
  }
}
