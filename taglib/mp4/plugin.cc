#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif //HAVE_CONFIG_H

#include <glibmm.h>

#include <fileref.h>

#include "resolver.h"
#include "mp4file.h"
#include "mp4tag.h"

#include "common/common.hh"
#include "mpx/mpx-types.hh"

#include <boost/lexical_cast.hpp>

extern "C" int  _plugin_has_accessors;
int  _plugin_has_accessors = 1;

extern "C" int  _plugin_version;
int  _plugin_version = PLUGIN_VERSION;

extern "C" const char ** _mimetypes ()
{
  static const char * types[] =
  {
    "audio/x-m4a",
    "audio/mp4",
    "video/quicktime",
     NULL 
  };
  return types;
}

using namespace MPX;
using namespace TagLib;
using namespace TagLib::MP4;
using namespace Glib;
using boost::get;

extern "C" bool _set (std::string const& filename, Track & track)
{
  MP4::File opfile (filename.c_str()) ;
  if (!metadata_check_file (&opfile))
    return false;
  
  MP4::Tag* tag;
  if (!(tag = dynamic_cast<MP4::Tag*> (opfile.tag())))
    return false;

  metadata_set_common (&opfile, track) ;

  MP4::ItemListMap & ItemMap = tag->itemListMap() ;

  if (track[ATTRIBUTE_ALBUM_ARTIST])
    ItemMap["aART"] =  StringList( String( get<std::string>(track[ATTRIBUTE_ALBUM_ARTIST].get())
					 , String::UTF8)) ;
                       
  if (track[ATTRIBUTE_MB_TRACK_ID])
    ItemMap["----:com.apple.iTunes:MusicBrainz Track Id"] = 
                       StringList( String( get<std::string>(track[ATTRIBUTE_MB_TRACK_ID].get())
					 , String::UTF8)) ;

  if (track[ATTRIBUTE_MB_ALBUM_ID])
    ItemMap["----:com.apple.iTunes:MusicBrainz Album Id"] =
                       StringList( String(get<std::string>(track[ATTRIBUTE_MB_ALBUM_ID].get())
					 , String::UTF8)) ;

  if (track[ATTRIBUTE_MB_ARTIST_ID])
    ItemMap["----:com.apple.iTunes:MusicBrainz Artist Id"] = 
                       StringList( String( get<std::string>(track[ATTRIBUTE_MB_ARTIST_ID].get())
					 , String::UTF8)) ;

  if (track[ATTRIBUTE_MB_ALBUM_ARTIST_ID])
    ItemMap["----:com.apple.iTunes:MusicBrainz Album Artist Id"] =
                       StringList( String( get<std::string>(track[ATTRIBUTE_MB_ALBUM_ARTIST_ID].get())
					 , String::UTF8)) ;

  if (track[ATTRIBUTE_ALBUM_ARTIST_SORTNAME])
    ItemMap["soaa"] =  StringList( String( get<std::string>(track[ATTRIBUTE_ALBUM_ARTIST_SORTNAME].get())
					 , String::UTF8)) ;
                       
  if (track[ATTRIBUTE_ARTIST_SORTNAME])
    ItemMap["soar"] =  StringList( String( get<std::string>(track[ATTRIBUTE_ARTIST_SORTNAME].get())
					 , String::UTF8)) ;

  if (track[ATTRIBUTE_MB_RELEASE_DATE])
    ItemMap["\251day"] =  
                       StringList( String( get<std::string>(track[ATTRIBUTE_MB_RELEASE_DATE].get())
					 , String::UTF8)) ;

  if (track[ATTRIBUTE_ASIN])
    ItemMap["----:com.apple.iTunes:ASIN"] = 
                       StringList( String( get<std::string>(track[ATTRIBUTE_ASIN].get())
					 , String::UTF8)) ;

  if (track[ATTRIBUTE_LABEL])
    ItemMap["----:com.apple.iTunes:LABEL"] = 
                       StringList( String( get<std::string>(track[ATTRIBUTE_LABEL].get())
					 , String::UTF8)) ;

  if (track[ATTRIBUTE_MUSICIP_PUID])
    ItemMap["----:com.apple.ITunes:MusicIP PUID"] = 
                       StringList( String( get<std::string>(track[ATTRIBUTE_MUSICIP_PUID].get())
					 , String::UTF8)) ;

  opfile.save () ;
  return true;
}

extern "C" bool _get (std::string const& filename, Track & track)  
{
    MP4::File opfile( filename.c_str() ) ;

    if( !metadata_check_file( &opfile ))
    {
        return false;
    }

    MP4::Tag* tag;

    if( !(tag = dynamic_cast<MP4::Tag*>( opfile.tag() )))
    {
        return false;
    }

    const MP4::ItemListMap& ItemMap = tag->itemListMap() ;

    if (ItemMap.contains ("\251day"))
    {
        track[ATTRIBUTE_MB_RELEASE_DATE] =
                    ItemMap["\251day"].toStringList()[0].toCString (true); 
    }

    if (ItemMap.contains ("----:com.apple.iTunes:MusicBrainz Album Release Country"))
    {
        track[ATTRIBUTE_MB_RELEASE_COUNTRY] =
                    ItemMap[ "----:com.apple.iTunes:MusicBrainz Album Release Country"].toStringList()[0].toCString (true); 
    }

    if (ItemMap.contains ("----:com.apple.iTunes:MusicBrainz Album Type"))
    {
        track[ATTRIBUTE_MB_RELEASE_TYPE] =
                    ItemMap[ "----:com.apple.iTunes:MusicBrainz Album Type"].toStringList()[0].toCString (true); 
    }

    if (ItemMap.contains ("cpil"))
    {
	bool is_cpil= ItemMap["cpil"].toBool() ;

	if( is_cpil )
	try{
        	track[ATTRIBUTE_IS_COMPILATION] =
                    boost::lexical_cast<guint>( is_cpil ) ;

		track[ATTRIBUTE_MB_RELEASE_TYPE] = "compilation" ;

	} catch(...) {}
    }

    if (ItemMap.contains ("aART"))
    {
        track[ATTRIBUTE_ALBUM_ARTIST] =
                    ItemMap["aART"].toStringList()[0].toCString (true); 
    }

    if (ItemMap.contains ("soaa"))
    {
        track[ATTRIBUTE_ALBUM_ARTIST_SORTNAME] =
                    ItemMap["soaa"].toStringList()[0].toCString (true); 
    }

    if (ItemMap.contains ("soar"))
    {
        track[ATTRIBUTE_ARTIST_SORTNAME] =
                    ItemMap["soar"].toStringList()[0].toCString (true); 
    }

    if (ItemMap.contains ("----:com.apple.iTunes:MusicBrainz Track Id"))
    {
        track[ATTRIBUTE_MB_TRACK_ID] =
                    ItemMap["----:com.apple.iTunes:MusicBrainz Track Id"].toStringList()[0].toCString (true); 
    }

    if (ItemMap.contains ("----:com.apple.iTunes:MusicBrainz Album Id"))
    {
        track[ATTRIBUTE_MB_ALBUM_ID] =
                    ItemMap["----:com.apple.iTunes:MusicBrainz Album Id"].toStringList()[0].toCString (true); 
    }

    if (ItemMap.contains ("----:com.apple.iTunes:MusicBrainz Artist Id"))
    {
        track[ATTRIBUTE_MB_ARTIST_ID] =
                    ItemMap["----:com.apple.iTunes:MusicBrainz Artist Id"].toStringList()[0].toCString (true); 
    }

    if (ItemMap.contains ("----:com.apple.iTunes:MusicBrainz Album Artist Id"))
    {
        track[ATTRIBUTE_MB_ALBUM_ARTIST_ID] =
                    ItemMap["----:com.apple.iTunes:MusicBrainz Album Artist Id"].toStringList()[0].toCString (true); 
    }

    if (ItemMap.contains ("----:com.apple.iTunes:ASIN"))
    {
        track[ATTRIBUTE_ASIN] =
                    ItemMap["----:com.apple.iTunes:ASIN"].toStringList()[0].toCString (true); 
    }

    if (ItemMap.contains ("----:com.apple.iTunes:MusicIP PUID"))
    {
        track[ATTRIBUTE_MUSICIP_PUID] =
                    ItemMap["----:com.apple.iTunes:MusicIP PUID"].toStringList()[0].toCString (true); 
    }

    if (ItemMap.contains ("----:com.apple.iTunes:MusicBrainz Album Release Country"))
    {
        track[ATTRIBUTE_MB_RELEASE_COUNTRY] =
                    ItemMap["----:com.apple.iTunes:MusicBrainz Album Release Country"].toStringList()[0].toCString (true); 
    }

    if (ItemMap.contains ("----:com.apple.iTunes:MusicBrainz Album Release Type"))
    {
        track[ATTRIBUTE_MB_RELEASE_TYPE] =
                    ItemMap["----:com.apple.iTunes:MusicBrainz Album Release Type"].toStringList()[0].toCString (true); 
    }

/* NOT CURRENTLY SUPPORTED
    if (ItemMap.contains ("----:com.apple.iTunes:MusicBrainz Album Status"))
    {
        track[ATTRIBUTE_MB_RELEASE_STATUS] =
                    ItemMap["----:com.apple.iTunes:MusicBrainz Album Status"].toStringList()[0].toCString (true); 
    }
*/

    metadata_get_common( &opfile, track ) ;

    return true;
}

static void _plugin_init (void) __attribute__((constructor)) ;
static void _plugin_init (void)
{
  TagLib::FileRef::addFileTypeResolver(new MP4FileTypeResolver) ;
}
