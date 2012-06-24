//  MPX
//  Copyright (C) 2010 MPX development.
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
//  --
//
//  The MPX project hereby grants permission for non-GPL compatible GStreamer
//  plugins to be used and distributed together with GStreamer and MPX. This
//  permission is above and beyond the permissions granted by the GPL license
//  MPX is covered by.

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif //HAVE_CONFIG_H

#include <gtkmm.h>
#include <glibmm.h>
#include <giomm.h>
#include <glibmm/i18n.h>
#include <glib/gstdio.h>
#include <boost/ref.hpp>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <unistd.h>
#include <sys/stat.h>

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

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

#include "mpx/mpx-covers.hh"
#include "mpx/mpx-covers-stores.hh"
#include "mpx/mpx-main.hh"
#include "mpx/mpx-minisoup.hh"
#include "mpx/mpx-network.hh"
#include "mpx/mpx-uri.hh"
#include "mpx/util-file.hh"
#include "mpx/util-graphics.hh"
#include "mpx/util-string.hh"
#include "mpx/xml/xml.hh"
#include "mpx/algorithm/ld.hh"

#include "library.hh"

#undef PACKAGE
#define PACKAGE "youki"

using namespace Glib;
using namespace TagLib;

namespace MPX
{
    struct Covers::ThreadData
    {
        SignalGotCover_t GotCover ;
	SignalNoCover_t NoCover ;
    } ;

    Covers::Covers ()

        : Service::Base("mpx-service-covers")

        , sigx::glib_threadable()
        , cache(sigc::mem_fun(*this, &Covers::on_cache))
        , signal_got_cover(*this, m_ThreadData, &ThreadData::GotCover)
        , signal_no_cover(*this, m_ThreadData, &ThreadData::NoCover)
    {
        Glib::ScopedPtr<char> path (g_build_filename(g_get_user_cache_dir(), PACKAGE, "covers", NULL));
        g_mkdir(path.get(), 0700);

        m_stores_all.push_back(StorePtr(new InlineCovers()));
        m_stores_all.push_back(StorePtr(new LastFMCovers()));
        m_stores_all.push_back(StorePtr(new MusicBrainzCovers()));

	for( guint n = 0 ; n < m_stores_all.size(); ++n )
	{
	    m_stores_all[n]->signal_save_cover().connect( sigc::mem_fun( *this, &Covers::cache_artwork )) ;
	}

        m_default_cover = Gdk::Pixbuf::create_from_file( Glib::build_filename( DATA_DIR, "images" G_DIR_SEPARATOR_S "disc.png" )) ;
    }

    void
    Covers::on_startup()
    {
        m_ThreadData.set( new ThreadData ) ;
    }

    void
    Covers::on_cleanup()
    {
        m_ThreadData.set( new ThreadData ) ;
    }

    void
    Covers::on_cache(
          const RequestQualifier& qual
        , bool                    acquire
	, bool			  overwrite
    )
    {
/*
        maincontext()->signal_idle().connect(
            sigc::bind(
            sigc::mem_fun(
                  *this
                , &Covers::handle_cache
           )
            , qual
            , acquire
	    , overwrite
        )) ;
*/
	handle_cache( qual, acquire, overwrite ) ;
    }

    void
    Covers::cache_artwork(
          const std::string&		mbid
	, Glib::RefPtr<Gdk::Pixbuf> 	cover
    )
    {
	g_message("%s: Saving cover for %s", G_STRLOC, mbid.c_str()) ;
        cover->save( get_thumb_path( mbid ), "png" ) ;
        m_pixbuf_cache[mbid] = cover ;
    }

    std::string
    Covers::get_thumb_path(
        std::string mbid
    )
    {
        using boost::algorithm::replace_all ;

        replace_all(mbid, "/","_") ;
        std::string basename = (boost::format ("%s.png") % mbid).str () ;
        Glib::ScopedPtr<char> path (g_build_filename(g_get_user_cache_dir(), PACKAGE, "covers", basename.c_str(), NULL)) ;

        return std::string(path.get()) ;
    }

    bool
    Covers::handle_cache(
          const RequestQualifier& qual
        , bool                    acquire
        , bool			  overwrite
    )
    {
        ThreadData * pthreaddata = m_ThreadData.get() ;

        std::string thumb_path = get_thumb_path( qual.mbid ) ;

        if( file_test( thumb_path, FILE_TEST_EXISTS ) && !overwrite )
        {
            pthreaddata->GotCover.emit( qual.id ) ;
            return false ; 
        }

	if( overwrite )
	{
	    m_pixbuf_cache.erase( qual.mbid ) ;
	    //FIXME: Erase from on-disk cache
	}

        if( acquire ) 
        {
            for( auto& Store : m_stores_all ) 
            {
		Store->load_artwork( qual ) ;

		if( Store->get_state() == FETCH_STATE_COVER_SAVED )
		{
		    pthreaddata->GotCover.emit( qual.id ) ;
		    return false ;	
		}
            }
        }

	pthreaddata->NoCover.emit( qual.id ) ;

        return false ;
    }

    bool
    Covers::fetch(
          RefPtr<Gdk::Pixbuf>&      cover
        , int                       size
    )
    {
        if( size == -1 )
            cover = m_default_cover ;
        else
            cover = m_default_cover->scale_simple( size, size, Gdk::INTERP_BILINEAR ) ;

        return true ;
    }

    bool
    Covers::fetch(
          const std::string&        mbid
        , RefPtr<Gdk::Pixbuf>&      cover
        , int                       size
    )
    {
        return sigx::open_sync_tunnel(
                    sigc::bind(
                    sigc::mem_fun(
                            *this
                          , &Covers::fetch_back1
                    )
                    , mbid
                    , boost::ref(cover)
                    , size
        ))() ;
    }

    void
    Covers::purge()
    {
        Glib::ScopedPtr<char> path (g_build_filename(g_get_user_cache_dir(), PACKAGE, "covers", NULL));

        Glib::Dir dir (path.get());
        StrV v (dir.begin(), dir.end());
        dir.close ();

        for(StrV::const_iterator i = v.begin(); i != v.end(); ++i)
        {
            std::string fullpath = build_filename(path.get(), *i);
            g_unlink(fullpath.c_str()) ; //FIXME: Use GIO
        }
    }

    ////

    bool
    Covers::fetch_back1(
          const std::string&      mbid_
        , RefPtr<Gdk::Pixbuf>&    cover
        , int                     size
    )
    {
        std::string mbid ;

        if( size != -1 )
        {
            mbid = (boost::format("%s-SIZE%d") % mbid_ % size ).str() ;
        }
        else
        {
            mbid = mbid_ ;
        }

        PixbufCache::const_iterator i = m_pixbuf_cache.find( mbid );

        if (i != m_pixbuf_cache.end())
        {
            cover = (*i).second; 
            return true;
        }

        std::string thumb_path = get_thumb_path( mbid ) ;

        if( file_test( thumb_path, FILE_TEST_EXISTS ))
        {
            try{
              cover = Gdk::Pixbuf::create_from_file( thumb_path ) ; 
              m_pixbuf_cache.insert( std::make_pair( mbid, cover )) ;
              return true;
            }
            catch( Gdk::PixbufError )
            {
            }
            catch( Glib::FileError )
            {
            }
        }
        else 
        if( size != -1 )
        {
            std::string thumb_path = get_thumb_path( mbid_ ) ;

            if( file_test( thumb_path, FILE_TEST_EXISTS ))
            {
                    try{
                      cover = Gdk::Pixbuf::create_from_file( thumb_path )->scale_simple( size, size, Gdk::INTERP_BILINEAR ) ; 
		      g_message("%s: Saving cover for %s", G_STRLOC, mbid.c_str()) ;
                      cover->save( get_thumb_path( mbid ), "png" );
                      m_pixbuf_cache.insert( std::make_pair( mbid, cover )) ;
                      return true;
                    }
                    catch( Gdk::PixbufError )
                    {
                    }
                    catch( Glib::FileError )
                    {
                    }
            }
        }

        return false;
    }
}
