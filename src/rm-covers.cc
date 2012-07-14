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

#include "rm-covers.hh"

namespace
{
    std::string
    get_thumb_path(
	  std::string mbid
    )
    {
	using boost::algorithm::replace_all ;

        replace_all(mbid, "/","_") ;
        std::string basename = (boost::format ("%s.png") % mbid).str () ;
        Glib::ScopedPtr<char> path (g_build_filename(g_get_user_cache_dir(), PACKAGE, "covers", basename.c_str(), NULL)) ;

        return std::string(path.get()) ;
    }
}

namespace MPX
{
namespace RM
{
	AlbumImage
	AlbumImage::retrieve_cached(Key const& key)
	{
	    Glib::RefPtr<Gdk::Pixbuf> img;

	    std::string thumb_path = get_thumb_path (key.mbid);

	    if( Glib::file_test( thumb_path, Glib::FILE_TEST_EXISTS))
	    {
		img = Gdk::Pixbuf::create_from_file (thumb_path);
	    }

	    return AlbumImage (key,img);
	}

	AlbumImage
	AlbumImage::retrieve(Key const& key)
	{
	    typedef boost::shared_ptr<CoverStore>   CS_sh_p_t ;
	    typedef std::vector<CS_sh_p_t>	    CS_v_t ;

	    CS_v_t stores ;

	    stores.push_back(CS_sh_p_t(new InlineCovers));
	    stores.push_back(CS_sh_p_t(new MusicBrainzCovers));
	    stores.push_back(CS_sh_p_t(new LastFMCovers));

	    Glib::RefPtr<Gdk::Pixbuf> cover ;

	    for( auto store : stores )
	    {
		Glib::RefPtr<Gdk::Pixbuf> cover;
		cover = store->load_artwork(key) ;

		if(cover)
		{
		    cover->save (get_thumb_path (key.mbid), "png") ;
		    return AlbumImage(key, cover);
		}
	    }

	    return AlbumImage(key) ;
	}
}}
