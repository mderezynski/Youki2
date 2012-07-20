#include <string>
#include <glibmm.h>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include "mpx/xml/xmltoc++.hh"
#include "xmlcpp/xsd-artist-info-2.0.hxx"

#include "mpx/util-string.hh"
#include "mpx/util-graphics.hh"
#include "mpx/util-file.hh"

#include "mpx/mpx-uri.hh"

#include "rm-artist-images.hh"

namespace MPX
{
	ArtistImage
	ArtistImage::retrieve_cached(Key const& key)
	{
	    Glib::RefPtr<Gdk::Pixbuf> artist_image ;

	    std::string base_path = Glib::build_filename(Glib::build_filename(g_get_user_cache_dir(), PACKAGE), "artists");
	    std::string thumb = Glib::build_filename(base_path,key+".png") ;  

	    if( Glib::file_test( thumb, Glib::FILE_TEST_EXISTS ))
	    {
		try{
		    artist_image = Gdk::Pixbuf::create_from_file( thumb ) ;		    
		} catch( Glib::Error& cxe ) {
		    g_message("%s: Error loading artist thumbnail for ID '%s'", G_STRLOC, key.c_str()) ;
		}
	    }

	    return ArtistImage (key,artist_image);
	}

	ArtistImage
	ArtistImage::retrieve(Key const& key)
	{
	    Glib::RefPtr<Gdk::Pixbuf> artist_image ;

	    std::string base_path = Glib::build_filename(Glib::build_filename(g_get_user_cache_dir(), PACKAGE), "artists");
	    std::string thumb = Glib::build_filename(base_path,key+".png") ;  

	    try{    
		using boost::algorithm::split;
		using boost::algorithm::is_any_of;
		using boost::algorithm::find_first;

		std::string url ; 

		if( key.substr(0,4) == "mpx-" )
		    url = (boost::format("http://ws.audioscrobbler.com/2.0/?method=artist.getinfo&artist=%s&api_key=37cd50ae88b85b764b72bb4fe4041fe4") % URI::escape_string(key.substr(4, std::string::npos))).str() ;
		else
		    url = (boost::format("http://ws.audioscrobbler.com/2.0/?method=artist.getinfo&mbid=%s&api_key=37cd50ae88b85b764b72bb4fe4041fe4") % key).str() ;

		MPX::XmlInstance<LastFMArtistInfo::lfm> xml (url) ; 

		const std::string& image_url = *(xml.xml().artist().image().begin());

		StrV m;
		split( m, image_url, is_any_of("/") );

		std::string image_url_126s = (boost::format("http://userserve-ak.last.fm/serve/126s/%s") % m[m.size()-1]).str();

		artist_image = Util::get_image_from_uri( image_url_126s );
	
		if( artist_image )
		    artist_image->save( thumb, "png" ) ;

	    } catch(Glib::Error& cxe) {
		g_message("%s: exception (glib): %s", G_STRLOC, cxe.what().c_str()) ;
		return ArtistImage(key) ;
	    } catch(std::exception& cxe) {
		g_message("%s: exception: %s", G_STRLOC, cxe.what()) ;
		return ArtistImage(key) ;
	    }

	    return ArtistImage (key,artist_image); 
	}
}
