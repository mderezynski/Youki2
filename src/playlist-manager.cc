#ifdef HAVE_CONFIG_H
#include "config.h" 
#endif

#include "playlist-manager.hh"

#include <vector>
#include <set>
#include <glibmm.h>

#include <boost/algorithm/string.hpp>
#include "mpx/mpx-xspf-write.hh"
#include "mpx/mpx-types.hh"
#include "mpx/mpx-main.hh"
#include "mpx/mpx-covers.hh"
#include "mpx/util-graphics.hh"

#include "library.hh"

namespace MPX
{
    PlaylistManager::PlaylistManager()
    {
	mcs->domain_register("PlaylistManager") ;
	mcs->key_register("PlaylistManager","basedir",std::string(Glib::build_filename(Glib::build_filename(g_get_user_data_dir(),PACKAGE),"playlists"))) ;
	m_Playlists_Base_Dir = mcs->key_get<std::string>("PlaylistManager","basedir") ;

	load_playlists() ;
    }

    void
    PlaylistManager::generate_artist_excerpt(
	  Playlist_t& p
    )
    {
	boost::shared_ptr<MPX::Library> lib = services->get<Library>("mpx-service-library");
	
	guint c = 0 ;
	guint mod = p.Playlist.size() / 3 ;
	mod = (p.Playlist.size() < 4) ? 1 : ((mod < 2) ? 2 : mod) ;

	for( auto id : p.Playlist )
	{	
	    if(!(c%mod))
	    {
		SQL::RowV rv ;
		lib->getSQL( rv, (boost::format("SELECT artist FROM track_view WHERE id ='%u'") % id).str()) ;

		if( !rv.empty())
		{
		    const std::string& artist = boost::get<std::string>(rv[0]["artist"]) ;
		    p.Excerpt.Artists.insert(artist) ;
		}
	    }
	    ++c ; 
	}

	p.Excerpt.update() ;
    }

    void
    PlaylistManager::generate_icon(
	  Playlist_t& p
    )
    {
	boost::shared_ptr<MPX::Library> lib = services->get<Library>("mpx-service-library");
	boost::shared_ptr<MPX::Covers> covers = services->get<Covers>("mpx-service-covers");

	std::set<std::string> s ;

	for( auto id : p.Playlist )
	{	
	    SQL::RowV rv ;
	    lib->getSQL( rv, (boost::format("SELECT mb_album_id FROM track_view WHERE id ='%u'") % id).str()) ;
	    s.insert(boost::get<std::string>(rv[0]["mb_album_id"])) ;
	}

	auto icon = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, 144, 48) ;
	auto cairo = Cairo::Context::create(icon) ;

	cairo->set_operator(Cairo::OPERATOR_CLEAR) ;
	cairo->paint() ;

	cairo->set_operator(Cairo::OPERATOR_OVER) ;
	cairo->set_source_rgba(1,1,1,1) ;
	cairo->paint() ;

	std::set<std::string>::iterator ii = s.begin() ;

	if(s.size() > 0)
	{
	    Glib::RefPtr<Gdk::Pixbuf> pb ; 
	    if(covers->fetch( *ii, pb, 48 ))
	    {
		Gdk::Cairo::set_source_pixbuf( cairo, pb, 0, 0 ) ;
		cairo->rectangle(0,0,48,48) ;
		cairo->fill() ;
	    }
	}

	if(s.size() > 1)
	{
	    ++ii ;

	    Glib::RefPtr<Gdk::Pixbuf> pb ; 
	    if(covers->fetch( *ii, pb, 48 ))
	    {
		Gdk::Cairo::set_source_pixbuf( cairo, pb, 48, 0 ) ;
		cairo->rectangle(48,0,48,48) ;
		cairo->fill() ;
	    }
	}

	if(s.size() > 2)
	{
	    ++ii ;

	    Glib::RefPtr<Gdk::Pixbuf> pb ; 
	    if(covers->fetch( *ii, pb, 48 ))
	    {
		Gdk::Cairo::set_source_pixbuf( cairo, pb, 96, 0 ) ;
		cairo->rectangle(96,0,48,48) ;
		cairo->fill() ;
	    }
	}

#if 0
	if(s.size() > 3)
	{
	    ++ii ;

	    Glib::RefPtr<Gdk::Pixbuf> pb ; 
	    if(covers->fetch( *ii, pb, 48 ))
	    {
		Gdk::Cairo::set_source_pixbuf( cairo, pb, 0, 48 ) ;
		cairo->rectangle(0,48,48,48) ;
		cairo->fill() ;
	    }
	}
#endif

	Util::cairo_image_surface_rounded_border( icon, 3, 4) ;
	icon = Util::cairo_image_surface_round( icon, 4 ) ;

	p.Icon = Util::cairo_image_surface_to_pixbuf(icon) ;
    }

    void
    PlaylistManager::load_playlists()
    { 
	boost::shared_ptr<MPX::Library> lib = services->get<Library>("mpx-service-library");

	m_Playlists.clear() ;

        Glib::Dir d ( m_Playlists_Base_Dir ) ;
        std::vector<std::string> s (d.begin(), d.end()) ;
        d.close() ;

        for( auto& path : s ) 
        {
            std::vector<std::string> subs;
            boost::split( subs, path, boost::is_any_of (".") ) ;

            if( !subs.empty() )
            {
                std::string suffix = *subs.rbegin() ; 
    
                if( suffix == "xspf" )
                {
		    std::string XML = Glib::file_get_contents(Glib::build_filename(m_Playlists_Base_Dir,path)) ;

		    Playlist_sp p_sp ( new Playlist_t ) ;
		    Playlist_t& p = *p_sp ; 

		    p.Name = subs[0] ; 

		    XSPF_read(
			  XML
			, lib->get_uuid()
			, p.Playlist
		    ) ;

		    generate_artist_excerpt(p);
		    generate_icon(p);

		    m_Playlists.push_back(p_sp);

		    g_message("Loaded Playlist ['%s']", p.Name.c_str()) ;
		}
	    }
	}
    }
}


