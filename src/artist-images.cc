#include <queue>
#include <boost/ref.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <giomm.h>
#include <glibmm.h>
#include <glibmm/i18n.h>

#include "mpx/mpx-artist-images.hh"
#include "mpx/mpx-artist-images.hh"
#include "mpx/xml/xmltoc++.hh"
#include "mpx/mpx-services.hh"
#include "mpx/util-string.hh"
#include "mpx/util-graphics.hh"
#include "mpx/util-file.hh"
#include "mpx/mpx-uri.hh"
#include "xmlcpp/xsd-artist-info-2.0.hxx"

#include "library.hh"

using boost::get;
using namespace MPX;
using namespace MPX::SQL;
using namespace Glib;

struct MPX::ArtistImages::ThreadData
{
   SignalGotArtistImage_t    GotArtistImage ;
};

MPX::ArtistImages::ArtistImages(
)

: Base("mpx-service-artist-images")

, sigx::glib_threadable()

, signal_got_artist_image(
              *this
            , m_ThreadData
            , &ThreadData::GotArtistImage
  )

, recache_images(
            sigc::mem_fun(
                *this,
                &ArtistImages::on_recache_images
            )
  )

, m_SQL(new SQL::SQLDB(*(services->get<Library>("mpx-service-library")->get_sql_db())))

{
    m_base_path = build_filename( build_filename(g_get_user_cache_dir(), PACKAGE), "artists");
}

void
MPX::ArtistImages::on_startup ()
{
    m_ThreadData.set(new ThreadData);
}

void
MPX::ArtistImages::on_cleanup ()
{
}

Glib::RefPtr<Gdk::Pixbuf>
MPX::ArtistImages::get_image_by_mbid(
      const std::string& mbid
    , const std::string& name
)
{
    Glib::RefPtr<Gdk::Pixbuf> artist_image ;

    try{    

        using boost::algorithm::split;
        using boost::algorithm::is_any_of;
        using boost::algorithm::find_first;

	std::string url ;

	if( mbid.substr(0,4) == "mpx-" )
	{
	    url = (boost::format("http://ws.audioscrobbler.com/2.0/?method=artist.getinfo&artist=%s&api_key=37cd50ae88b85b764b72bb4fe4041fe4") % URI::escape_string(name)).str() ;
	}
	else
	{
	    url = (boost::format("http://ws.audioscrobbler.com/2.0/?method=artist.getinfo&mbid=%s&api_key=37cd50ae88b85b764b72bb4fe4041fe4") % mbid).str() ;
	}

        MPX::XmlInstance<LastFMArtistInfo::lfm> xml (url) ; 

        const std::string& image_url = *(xml.xml().artist().image().begin());

        StrV m;

        split( m, image_url, is_any_of("/") );
        std::string image_url_126s = (boost::format("http://userserve-ak.last.fm/serve/126s/%s") % m[m.size()-1]).str();

        artist_image = Util::get_image_from_uri( image_url_126s );

    } catch(...) {
    }

    return artist_image ;
}

void
MPX::ArtistImages::on_recache_images()
{
    ThreadData * pthreaddata = m_ThreadData.get();

    SQL::RowV v;
    
    m_SQL->get(
              v
            , "SELECT album_artist, mb_album_artist_id FROM album_artist"
    );

    for( SQL::RowV::iterator i = v.begin(); i != v.end(); ++i )
    {
        using boost::get;

        const std::string& mbid = get<std::string>((*i)["mb_album_artist_id"]);
	const std::string& name = get<std::string>((*i)["album_artist"]);

        const std::string& thumb_path = build_filename( m_base_path, mbid + std::string(".png" ));

        if( !file_test( thumb_path, FILE_TEST_EXISTS )) 
        {
                Glib::RefPtr<Gdk::Pixbuf> artist_image = get_image_by_mbid( mbid, name );

                if( artist_image )
                {
		    try{
			artist_image->save( thumb_path, "png" );
		    }catch(Glib::FileError&){}
                    m_pixbuf_cache.insert( std::make_pair( mbid, artist_image ));
                    pthreaddata->GotArtistImage.emit( mbid, artist_image );
                }
        }
        else
        {
            const std::string& thumb_path = build_filename( m_base_path, mbid + std::string(".png" ));
            Glib::RefPtr<Gdk::Pixbuf> artist_image = Gdk::Pixbuf::create_from_file( thumb_path ); 
            if( artist_image )
            {
                m_pixbuf_cache.insert( std::make_pair( mbid, artist_image ));
                pthreaddata->GotArtistImage.emit( mbid, artist_image );
            }
        }

	g_message("%s: Processed %u", G_STRLOC, std::distance( v.begin(), i)) ;
    }

    g_message("%s: Done recaching artists.", G_STRLOC) ;
}

Glib::RefPtr<Gdk::Pixbuf>
MPX::ArtistImages::get_image(
    const std::string& mbid
)
{
    return sigx::open_sync_tunnel(
		sigc::bind(
		sigc::mem_fun(
			*this
		      , &ArtistImages::on_get_image
		)
		, mbid
    ))() ;
}

Glib::RefPtr<Gdk::Pixbuf>
MPX::ArtistImages::on_get_image (
    const std::string& mbid
)
{
    ThreadData * pthreaddata = m_ThreadData.get();

    if( m_pixbuf_cache.find( mbid ) == m_pixbuf_cache.end() )
    {
        const std::string& thumb_path = build_filename( m_base_path, mbid + std::string(".png" ));
	try{
	    Glib::RefPtr<Gdk::Pixbuf> artist_image = Gdk::Pixbuf::create_from_file( thumb_path ); 
	    if( artist_image )
	    {
		m_pixbuf_cache.insert( std::make_pair( mbid, artist_image ));
		return artist_image ;
	    }
	} catch( Glib::FileError ) {}
    }
    else
    {
        return m_pixbuf_cache.find( mbid )->second ;
    }

    return Glib::RefPtr<Gdk::Pixbuf>(0) ;
}
