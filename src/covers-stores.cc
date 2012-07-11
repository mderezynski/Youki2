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

#include <gtkmm.h>
#include <glibmm.h>
#include <giomm.h>

#include <string>
#include <future>

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

#include "mpx/mpx-covers-stores.hh"
#include "mpx/algorithm/ld.hh"
#include "mpx/mpx-uri.hh"
#include "mpx/util-string.hh"

#include "mpx/xml/xml.hh"

using namespace TagLib ;

namespace MPX
{
    // --------------------------------------------------
    //
    // Remote store (still an abstract class, but provides a callback)
    //
    // --------------------------------------------------
    void
    RemoteStore::load_artwork(const RequestQualifier& rql)
    {
        m_state = FETCH_STATE_NOT_FETCHED ;

        if( can_load_artwork( rql ) )
        {
            fetch_image( get_url( rql ), rql ) ;
        }
    }

    void
    RemoteStore::fetch_image(const std::string& url, const RequestQualifier& rql)
    {
        if( can_load_artwork( rql ) )
        {
	    g_message("RemoteStore:: URL['%s']", url.c_str()) ;

            auto request = Soup::RequestSync::create( url ) ; 

	    auto handle = std::async(
		  std::launch::async
		, [&request](){ request->run(); }
	    ) ;

	    while(!handle.wait_for(std::chrono::duration<int, std::milli>(40)))
	    {
		g_usleep(10) ;
	    }

	    save_image(
		  request->get_data_raw()
		, request->get_data_size()
		, rql
	    ) ;
        }
    }

    bool
    RemoteStore::can_load_artwork(const RequestQualifier& rql)
    {
        return true ;
    }

    void
    RemoteStore::save_image(
          char const*        data
        , guint              size
        , const RequestQualifier& rql
    )
    {
        Glib::RefPtr<Gdk::PixbufLoader> loader = Gdk::PixbufLoader::create ();

        loader->write( reinterpret_cast<const guint8*>( data ), size ) ;
        loader->close() ;

        Glib::RefPtr<Gdk::Pixbuf> cover = loader->get_pixbuf();

        if( cover->get_width() == 1 && cover->get_height() == 1 )
        {
	    g_message("RemoteStore: got 1x1px image; aborting") ;	
            request_failed() ;
        }
        else
        { 
            try{
		    g_message("RemoteStore: trying to save cover") ;
                    m_SIGNAL.emit( rql.mbid, cover );
                    m_state = FETCH_STATE_COVER_SAVED;

            } catch( Glib::FileError & cxe )
            {
                    g_message("RemoteStore: File Error while trying to save cover image: %s", cxe.what().c_str());
                    request_failed() ;
            }
        }
    }
    
    void
    RemoteStore::request_failed()
    {
	g_message("RemoteStore: Request failed.") ;
        m_state = FETCH_STATE_NOT_FETCHED ;
    }

    // --------------------------------------------------
    //
    // LastFM cover art urls
    //
    // --------------------------------------------------

    std::string
    LastFMCovers::get_url( const RequestQualifier& rql )
    {
    static boost::format lastfm_no_MBID_f ("http://ws.audioscrobbler.com/2.0/?method=album.getinfo&artist=%s&album=%s&api_key=ff56d530598d65c1a4088e57da7be2f9");
    static boost::format lastfm_MBID_f ("http://ws.audioscrobbler.com/2.0/?method=album.getinfo&mbid=%s&api_key=ff56d530598d65c1a4088e57da7be2f9");

	if( rql.mbid.substr(0,4) != "mpx-" )
        	return (lastfm_MBID_f % (URI::escape_string(rql.mbid))).str();
	else
        	return (lastfm_no_MBID_f % (URI::escape_string(rql.artist)) % (URI::escape_string(rql.album))).str();
    }
 
    void
    LastFMCovers::load_artwork(const RequestQualifier& rql)
    {
        m_state = FETCH_STATE_NOT_FETCHED ;

        request = Soup::RequestSync::create( get_url( rql ));

	g_message("LastFMCovers: running HTTP request...") ;

	int code = request->run() ;

        if( code == 200 )
        {
	    g_message("LastFMCovers: HTTP 200, parsing XML...") ;
    
	    try
            {
                std::string album ;

                album = xpath_get_text(
                    request->get_data_raw(),
                    request->get_data_size(),
		    "//album/name",
                    "" 
                ); 

		if( album.empty() )
		{
		    request_failed() ;
		    return ;
		}

		Glib::ustring utf8_album = Glib::ustring(album).lowercase() ;
		Glib::ustring utf8_album_rql = Glib::ustring(rql.album).lowercase() ;

		std::size_t ld = ld_distance<Glib::ustring>( utf8_album, utf8_album_rql ) ;

                if( ld > 0 )
                {
		    request_failed() ;
                    return;
                }
	    }
            catch( std::runtime_error& ) {}

	    std::vector<std::string> sizes { "extralarge", "large", "normal" } ;

	    for( auto& s : sizes ) 
            {
                try
                {
                    std::string image_url = xpath_get_text(
                        request->get_data_raw(),
                        request->get_data_size(),
                        (boost::format("//image[@size='%s']") % s).str().c_str(),
                        ""
                    ) ;

		    if(!image_url.empty())
		    {
			std::string::size_type i = image_url.rfind(".") ;
			std::string url_jpg = image_url.substr(0,i) + ".jpg" ; 
			RemoteStore::fetch_image( url_jpg, rql ) ;
			return ;
		    }
                } 
                catch( std::runtime_error& ) {}
            }
        }

	g_message("LastFMCovers: No Cover.") ;
	request_failed() ;
    }

    // --------------------------------------------------
    //
    // MusicBrainz cover art urls
    //
    // --------------------------------------------------

    std::string
    MusicBrainzCovers::get_url(const RequestQualifier& rql)
    {

	static boost::format mbxml_f ("http://www.musicbrainz.org/ws/1/release/%s?type=xml&inc=url-rels");

        return (mbxml_f % rql.mbid).str();
    }

    void
    MusicBrainzCovers::load_artwork(const RequestQualifier& rql)
    {
        m_state = FETCH_STATE_NOT_FETCHED ;

	std::string url = get_url( rql ) ;

	g_message("%s: MusicBrainzCovers:URL [%s]",G_STRLOC,url.c_str()) ;

        request = Soup::RequestSync::create(url) ;

	int code = request->run() ;

        if( code == 200 )
        {
	    std::string image_url;

	    try
            {
		image_url = xpath_get_text(
		    request->get_data_raw(),
		    request->get_data_size(),
		    "/mb:metadata/mb:release/mb:relation-list//mb:relation[@type='CoverArtLink']/@target",
		    "mb=http://musicbrainz.org/ns/mmd-1.0# ext=http://musicbrainz.org/ns/ext-1.0#"
		); 
	    }
            catch( std::runtime_error& ) {}

	    if( !image_url.empty() )
	    {
		RemoteStore::fetch_image( image_url, rql ) ;
		return ;
	    }
	    // try ASIN

	    std::string asin ;

	    try{
		asin = xpath_get_text(
		    request->get_data_raw(),
		    request->get_data_size(),
		    "/mb:metadata/mb:release/mb:asin",
		    "mb=http://musicbrainz.org/ns/mmd-1.0# ext=http://musicbrainz.org/ns/ext-1.0#"
		); 
	    }catch( std::runtime_error& ) {}

	    if( !asin.empty() )
	    {
		image_url = (boost::format("http://images.amazon.com/images/P/%s.01.LZZZZZZZ.jpg") % asin).str() ;
		RemoteStore::fetch_image( image_url, rql ) ;
		return ;
	    }
        }

	request_failed() ;
    }

    // --------------------------------------------------
    //
    //  cover.jpg Store 
    //
    // --------------------------------------------------
    void
    LocalCovers::load_artwork(const RequestQualifier& rql)
    {
        m_state = FETCH_STATE_NOT_FETCHED ;

        Glib::RefPtr<Gio::File> directory = Gio::File::create_for_uri( rql.uri )->get_parent() ;
        Glib::RefPtr<Gio::FileEnumerator> files = directory->enumerate_children() ;
        Glib::RefPtr<Gio::FileInfo> file;

        std::string coverart_filename ;

        while (( file = files->next_file()) != 0 )
        {
            if( file->get_content_type().find("image") != std::string::npos &&
              ( file->get_name().find("folder") ||
                file->get_name().find("cover")  ||
                file->get_name().find("front")
              ))
            {
                coverart_filename = directory->get_child(file->get_name())->get_path();
                break;
            }
        }

        if(!coverart_filename.empty())
        {
            try{
                Glib::RefPtr<Gdk::Pixbuf> cover = Gdk::Pixbuf::create_from_file( coverart_filename );
		g_message("%s: LocalCovers: saving", G_STRLOC) ;
                m_SIGNAL.emit( rql.mbid, cover );
                m_state = FETCH_STATE_COVER_SAVED ;
            }
            catch (Gdk::PixbufError)
            {
            }
            catch (Glib::FileError)
            {
            }
        }
    }

    // --------------------------------------------------
    //
    // Inline covers 
    //
    // --------------------------------------------------
    void
    InlineCovers::load_artwork(const RequestQualifier& rql)
    {
        m_state = FETCH_STATE_NOT_FETCHED ;

        Glib::RefPtr<Gdk::Pixbuf> cover;

        if(!rql.uri.empty())
	{
	    try{
		g_message("InlineCovers: Trying id3v2 cover...") ;

		MPEG::File opfile (rql.uri.c_str());

		if(opfile.isOpen() && opfile.isValid())
		{
		    using TagLib::ID3v2::FrameList;

		    ID3v2::Tag * tag = opfile.ID3v2Tag(false) ;

		    g_message("InlineCovers: Looking for AttachedPictureFrame...") ; 

		    if( tag )
		    {
			FrameList const& list = tag->frameList();

			for(FrameList::ConstIterator i = list.begin(); i != list.end(); ++i)
			{
			    TagLib::ID3v2::Frame const* frame = *i;

			    if( frame )
			    {
				TagLib::ID3v2::AttachedPictureFrame const* picture =
				    dynamic_cast<TagLib::ID3v2::AttachedPictureFrame const*>(frame);

				if( picture && picture->type() == TagLib::ID3v2::AttachedPictureFrame::FrontCover )
				{
				    Glib::RefPtr<Gdk::PixbufLoader> loader = Gdk::PixbufLoader::create();
				    ByteVector picdata = picture->picture() ;
				    loader->write (reinterpret_cast<const guint8*>(picdata.data()), picdata.size());
				    loader->close ();
				    cover = loader->get_pixbuf();
				    g_message("InlineCovers: Saved cover for MBID ['%s']", rql.mbid.c_str()) ;
				    m_SIGNAL.emit( rql.mbid, cover );
				    m_state = FETCH_STATE_COVER_SAVED ; 
				    return ;
				}
			    }
			}
		    }
		}
	    } catch(...)
	    {
	    }
	}
    }
}
