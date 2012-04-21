//
// (C) 2007 DEREZYNSKI Milosz
//

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
#include <id3v2tag.h>
#include <mpegfile.h>
#include <id3v2framefactory.h>
#include <textidentificationframe.h>
#include <uniquefileidentifierframe.h>
#include <attachedpictureframe.h>

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <vector>
#include <string>

#include "mpx/mpx-types.hh"

using namespace Glib;
using namespace TagLib;
using namespace std;

namespace
{
      using namespace MPX;

      TagLib::ID3v2::UserTextIdentificationFrame*
      find_utif (TagLib::ID3v2::Tag *tag, TagLib::String const& description)
      {
        TagLib::ID3v2::FrameList l = tag->frameList("TXXX");
        for(TagLib::ID3v2::FrameList::Iterator i = l.begin(); i != l.end(); ++i)
        {
          TagLib::ID3v2::UserTextIdentificationFrame * f (dynamic_cast<TagLib::ID3v2::UserTextIdentificationFrame *> (*i));
          if (f && f->description() == description)
            return f;
        }
        return 0;
      }
}

// we redeclare it locally here so we don't need to include BMP stuff

namespace MPX
{
    void
    metadata_get_id3v2 (TagLib::ID3v2::Tag * tag, Track & track)
    {
        using boost::algorithm::split;
        using boost::algorithm::find_nth;
        using boost::iterator_range;
        using TagLib::ID3v2::FrameList;

        struct {
            int         datum;
            char const* id;
        } mb_metadata_id3v2[] = {
          { ATTRIBUTE_MB_ALBUM_ARTIST_ID,             "MusicBrainz Album Artist Id"         },
          { ATTRIBUTE_MB_ALBUM_ID,                    "MusicBrainz Album Id"                },
          { ATTRIBUTE_MB_ARTIST_ID,                   "MusicBrainz Artist Id"               },
          { ATTRIBUTE_ASIN,                           "ASIN"                                },
          { ATTRIBUTE_ALBUM_ARTIST_SORTNAME,          "ALBUMARTISTSORT"                     },
          { ATTRIBUTE_MUSICIP_PUID,                   "MusicIP PUID"                        },
          { ATTRIBUTE_MB_RELEASE_COUNTRY,             "MusicBrainz Album Release Country"   },
          { ATTRIBUTE_MB_RELEASE_TYPE,                "MusicBrainz Album Type"              }
        };

        ID3v2::UserTextIdentificationFrame * frame = 0;
        for (unsigned int n = 0; n < G_N_ELEMENTS (mb_metadata_id3v2); ++n)
        {
            frame = find_utif (tag, String (mb_metadata_id3v2[n].id, String::UTF8));
            if(frame)
            {
                std::string s = frame->toString().toCString( true ) ;
                iterator_range <std::string::iterator> match = find_nth (s, mb_metadata_id3v2[n].id + std::string(" "), 0);
                if(!match.empty())
                {
                    ustring substr (match.end(), s.end());
                    if(!substr.empty())
                    {
                        track[mb_metadata_id3v2[n].datum] = substr;
                    }
                }
            }
        }

        // MB UFID
        const FrameList& map = tag->frameListMap()["UFID"];
        if( !map.isEmpty() )
        {
            for (FrameList::ConstIterator iter = map.begin(); iter != map.end(); ++iter)
            {
                ID3v2::UniqueFileIdentifierFrame *ufid = reinterpret_cast<ID3v2::UniqueFileIdentifierFrame*> (*iter);

                if (ufid->owner() == "http://musicbrainz.org")
                {
                    ByteVector vec (ufid->identifier());
                    vec.append ('\0');

                    track[ATTRIBUTE_MB_TRACK_ID] = string( vec.data());

                    break;
                }
            }
        }

        // TDRC 
        {
            const FrameList& map = tag->frameListMap()["TDRC"];
            if( !map.isEmpty() )
            {
                track[ATTRIBUTE_MB_RELEASE_DATE] = string( map.front()->toString().toCString( true ) );
            }
        }

        // TPE2 (Album Artist) 
        {
            const FrameList& map = tag->frameListMap()["TPE2"];
            if( !map.isEmpty() )
            {
                track[ATTRIBUTE_ALBUM_ARTIST] = string( map.front()->toString().toCString( true ) );
            }
        }

        // TPUB (Label) 
        {
            const FrameList& map = tag->frameListMap()["TPUB"];
            if( !map.isEmpty() )
            {
                track[ATTRIBUTE_LABEL] = string( map.front()->toString().toCString( true ) );
            }
        }

        // Totaltracks
        {
            const FrameList& map = tag->frameListMap()["TRCK"];
            if( !map.isEmpty() )
            {
                std::string trck_str = string( map.front()->toString().toCString( true ) );
		std::vector<string> m ;
		split( m, trck_str, boost::algorithm::is_any_of("/")) ;
		
		if( m.size() == 2 )
		{
			try{
       				track[ATTRIBUTE_DISCTOTAL] = boost::lexical_cast<guint>( m[1] ) ; 
			}
			catch(...) {}
		}
            }
        }

        // Compilation
        {
            const FrameList& map = tag->frameListMap()["TCMP"];

            if( !map.isEmpty() )
            {
                std::string tcmp_str = string( map.front()->toString().toCString( true ) );

		try{	
			track[ATTRIBUTE_IS_COMPILATION] = boost::lexical_cast<guint>( tcmp_str ) ;

			if( !track.has( ATTRIBUTE_MB_RELEASE_TYPE) )
			{
				track[ATTRIBUTE_MB_RELEASE_TYPE] = "compilation";
			}

		} catch(...) {}
	    }
        }
	
        // Disc Nr. 
        {
            const FrameList& map = tag->frameListMap()["TPOS"];
            if( !map.isEmpty() )
            {
                std::string tpos_str = string( map.front()->toString().toCString( true ) );
		std::vector<string> m ;
		split( m, tpos_str, boost::algorithm::is_any_of("/")) ;

		try{
			if( m.size() >= 1 )
       				track[ATTRIBUTE_DISCNR] = boost::lexical_cast<guint>( m[0] ) ; 
		
			if( m.size() == 2 )
       				track[ATTRIBUTE_DISCS] = boost::lexical_cast<guint>( m[1] ) ; 
		}
		catch(...) {}
            }
        }


        // TSOP/XSOP (Artist Sort Name)
        {
            const char *id3v2frame = 0;

            if( !tag->frameListMap()["XSOP"].isEmpty() )
              id3v2frame = "XSOP" ;
            else
            if( !tag->frameListMap()["TSOP"].isEmpty() )
              id3v2frame = "TSOP" ;

            if( id3v2frame )
            {
                const FrameList& map = tag->frameListMap()[id3v2frame];

                if( !map.isEmpty() )
                {
                    track[ATTRIBUTE_ARTIST_SORTNAME] = string( map.front()->toString().toCString( true ) );
                }
            }
        }
    }

} // end namespace MPX
