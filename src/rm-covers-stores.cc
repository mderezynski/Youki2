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

#include <glibmm.h>

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

#include "mpx/algorithm/ld.hh"
#include "mpx/mpx-uri.hh"
#include "mpx/util-string.hh"
#include "mpx/xml/xml.hh"

#include "rm-covers-stores.hh"

using namespace TagLib ;

namespace MPX
{
namespace RM
{
    // --------------------------------------------------
    //
    // Remote store (still an abstract class, but provides a callback)
    //
    // --------------------------------------------------
    Glib::RefPtr<Gdk::Pixbuf> 
    RemoteStore::load_artwork(RequestQualifier const& rq)
    {
	return fetch_image (get_url(rq),rq) ;
    }

    Glib::RefPtr<Gdk::Pixbuf>
    RemoteStore::fetch_image(const std::string& url, const RequestQualifier& rq)
    {
	Glib::RefPtr<Gdk::Pixbuf> cover ;

        if( can_load_artwork( rq ) )
        {
	    auto session = soup_session_sync_new() ;
	    auto message = soup_message_new("GET", url.c_str()) ;

	    soup_session_send_message(SOUP_SESSION(session), SOUP_MESSAGE(message)) ;

	    if( message->status_code == 200 )
	    {
		cover = save_image(
		      message->response_body->data 
		    , message->response_body->length 
		    , rq
		) ;
	    }
        }

	return cover ;
    }

    bool
    RemoteStore::can_load_artwork(const RequestQualifier& rq)
    {
        return true ;
    }

    Glib::RefPtr<Gdk::Pixbuf> 
    RemoteStore::save_image(
          char const*        data
        , guint              size
        , const RequestQualifier& rq
    )
    {
        Glib::RefPtr<Gdk::PixbufLoader> loader = Gdk::PixbufLoader::create ();
        Glib::RefPtr<Gdk::Pixbuf> cover ; 

	try{
	    loader->write( reinterpret_cast<const guint8*>( data ), size ) ;
	    loader->close() ;
	    cover = loader->get_pixbuf();
	} catch( Glib::Error& cxe ) {
	    g_message("%s: Error: %s", G_STRLOC, cxe.what().c_str()) ;
	    cover.reset() ;
	}

        if( cover->get_width() == 1 && cover->get_height() == 1 )
        {
	    cover.reset() ;
        }

	return cover ;
    }
    
    // --------------------------------------------------
    //
    // LastFM cover art urls
    //
    // --------------------------------------------------

    std::string
    LastFMCovers::get_url( const RequestQualifier& rq )
    {
    static boost::format lastfm_no_MBID_f ("http://ws.audioscrobbler.com/2.0/?method=album.getinfo&artist=%s&album=%s&api_key=ff56d530598d65c1a4088e57da7be2f9");
    static boost::format lastfm_MBID_f ("http://ws.audioscrobbler.com/2.0/?method=album.getinfo&mbid=%s&api_key=ff56d530598d65c1a4088e57da7be2f9");

	if( rq.mbid.substr(0,4) != "mpx-" )
        	return (lastfm_MBID_f % (URI::escape_string(rq.mbid))).str();
	else
        	return (lastfm_no_MBID_f % (URI::escape_string(rq.artist)) % (URI::escape_string(rq.album))).str();
    }
 
    Glib::RefPtr<Gdk::Pixbuf> 
    LastFMCovers::load_artwork(const RequestQualifier& rq)
    {
	Glib::RefPtr<Gdk::Pixbuf> cover ;

        std::string url = get_url (rq) ;

	auto session = soup_session_sync_new() ;
	auto message = soup_message_new("GET", url.c_str()) ;

	soup_session_send_message(SOUP_SESSION(session), SOUP_MESSAGE(message)) ;

        if( message->status_code == 200 )
        {
	    try
            {
                std::string album ;

                album = xpath_get_text(
		      message->response_body->data 
                    , message->response_body->length 
		    , "//album/name"
                    , "" 
                ); 

		if(album.empty())
		    return cover ;

		Glib::ustring utf8_album = Glib::ustring(album).lowercase() ;
		Glib::ustring utf8_album_rq = Glib::ustring(rq.album).lowercase() ;

		std::size_t ld = ld_distance<Glib::ustring>( utf8_album, utf8_album_rq ) ;

                if( ld > 0 )
                {
                    return cover ;
                }
	    }
            catch( std::runtime_error& cxe )
	    {
		g_message("%s: Exception: %s", G_STRLOC, cxe.what()) ;
		return cover ;
	    }

	    std::vector<std::string> sizes { "extralarge", "large", "normal" } ;

	    for( auto& s : sizes ) 
            {
                try
                {
                    std::string image_url = xpath_get_text(
			  message->response_body->data 
                        , message->response_body->length 
                        , (boost::format("//image[@size='%s']") % s).str().c_str()
                        , ""
                    ) ;

		    g_message("%s: size {%s}, image_url: %s", G_STRLOC, s.c_str(), image_url.c_str()) ;

		    if(!image_url.empty())
		    {
			std::string::size_type i = image_url.rfind(".") ;
			std::string url_jpg = image_url.substr(0,i) + ".jpg" ; 
			cover = RemoteStore::fetch_image( url_jpg, rq ) ;

			return cover;
		    }
                } 
                catch( std::runtime_error& cxe)
		{
		    g_message("%s: Exception: %s", G_STRLOC, cxe.what()) ;
		    return cover;
		}
            }
        }

	return cover;
    }

    // --------------------------------------------------
    //
    // MusicBrainz cover art urls
    //
    // --------------------------------------------------

    std::string
    MusicBrainzCovers::get_url(const RequestQualifier& rq)
    {

	static boost::format mbxml_f ("http://www.musicbrainz.org/ws/1/release/%s?type=xml&inc=url-rels");

        return (mbxml_f % rq.mbid).str();
    }

    Glib::RefPtr<Gdk::Pixbuf> 
    MusicBrainzCovers::load_artwork(const RequestQualifier& rq)
    {
	Glib::RefPtr<Gdk::Pixbuf> cover;

	std::string url = get_url (rq);

	auto session = soup_session_sync_new() ;
	auto message = soup_message_new("GET", url.c_str()) ;

	soup_session_send_message(SOUP_SESSION(session), SOUP_MESSAGE(message)) ;

        if( message->status_code == 200 )
        {
	    std::string image_url;

	    try
            {
		image_url = xpath_get_text(
		      message->response_body->data 
		    , message->response_body->length 
		    , "/mb:metadata/mb:release/mb:relation-list//mb:relation[@type='CoverArtLink']/@target"
		    , "mb=http://musicbrainz.org/ns/mmd-1.0# ext=http://musicbrainz.org/ns/ext-1.0#"
		); 
	    }
            catch( std::runtime_error& cxe )
	    {
		g_message("%s: Exception: %s", G_STRLOC, cxe.what()) ;
		return cover;
	    }

	    if( !image_url.empty() )
	    {
		cover = RemoteStore::fetch_image( image_url, rq ) ;
		return cover;
	    }

	    // Try ASIN
	    std::string asin ;

	    try{
		asin = xpath_get_text(
		      message->response_body->data 
		    , message->response_body->length 
		    , "/mb:metadata/mb:release/mb:asin"
		    , "mb=http://musicbrainz.org/ns/mmd-1.0# ext=http://musicbrainz.org/ns/ext-1.0#"
		); 
            } catch( std::runtime_error& cxe )
	    {
		g_message("%s: Exception: %s", G_STRLOC, cxe.what()) ;
		return cover;
	    }

	    if( !asin.empty() )
	    {
		image_url = (boost::format("http://images.amazon.com/images/P/%s.01.LZZZZZZZ.jpg") % asin).str() ;
		cover = RemoteStore::fetch_image( image_url, rq ) ;
		return cover;
	    }
        }

	return cover;
    }

    // --------------------------------------------------
    //
    // Inline covers 
    //
    // --------------------------------------------------
    Glib::RefPtr<Gdk::Pixbuf> 
    InlineCovers::load_artwork(RequestQualifier const& rq)
    {
        Glib::RefPtr<Gdk::Pixbuf> cover;

        if(!rq.uri.empty())
	{
	    try{
		g_message("InlineCovers: Trying id3v2 cover...") ;

		MPEG::File opfile (rq.uri.c_str());

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
				    g_message("InlineCovers: Got cover for MBID ['%s']", rq.mbid.c_str()) ;
				}
			    }
			}
		    }
		}
	    } catch(std::exception& cxe) { 
		g_message("%s: Error: %s", G_STRLOC, cxe.what()) ;
	    } catch(Glib::Error& cxe) {
		g_message("%s: Error: %s", G_STRLOC, cxe.what().c_str()) ;
	    }
	}

	return cover ;
    }
}
}
