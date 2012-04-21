//  MPX
//  Copyright (C) 2005-2007 MPX development.
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
//  The MPXx project hereby grants permission for non GPL-compatible GStreamer
//  plugins to be used and distributed together with GStreamer and MPXx. This
//  permission is above and beyond the permissions granted by the GPL license
//  MPXx is covered by.

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif //HAVE_CONFIG_H

#include <glibmm/ustring.h>

using namespace std;
using namespace Glib;

#include "mpx/parser/xpath.hh"
#include "mpx/parser/libxml2-sax-base.hh"
using MPX::XPath;

#include "mpx/xml/xspf-libxml2-sax.hh"

namespace
{
  struct XSPFParserContext
    : public ParseContextBase
  {
    MPX::XSPF::Playlist & m_playlist;
    MPX::XSPF::Item       m_item;
    XSPFParserContext (MPX::XSPF::Playlist & playlist): m_playlist (playlist) {}
  };

#define DEFAULT_REFS \
  XSPFParserContext & context (static_cast<XSPFParserContext&>(_context));  \
  MPX::XSPF::Playlist &playlist G_GNUC_UNUSED (context.m_playlist);         \
  MPX::XSPF::Item &item G_GNUC_UNUSED (context.m_item);  

  //////////////////////////////////////////////////////////////////////////////

  namespace Handlers
  {
    namespace Track
    {
      HANDLER(trackList_track)
      {
        DEFAULT_REFS
        context.m_item = MPX::XSPF::Item();
      }
    }
  }

  namespace HandlersEnd
  {
    namespace Track
    {
      HANDLER_END(trackList_track)
      {
        DEFAULT_REFS
        playlist.Items.push_back (item);
      }
    }
  }
  
  namespace HandlersText
  {
    namespace Playlist
    {
      HANDLER_Text(title)
      {
        DEFAULT_REFS
        playlist.Title += text;
      }
    }

    HANDLER_Text(location)
    {
      DEFAULT_REFS
      item.location += text;
    }

    HANDLER_Text(title)
    {
      DEFAULT_REFS
      item.title += text;
    }

    HANDLER_Text(id)
    {
      DEFAULT_REFS
      item.id = g_ascii_strtoull (text.c_str(), NULL, 10); 
    }

    HANDLER_Text(album)
    {
      DEFAULT_REFS
      item.album += text;
    }

    HANDLER_Text(creator)
    {
      DEFAULT_REFS
      item.creator += text;
    }

    HANDLER_Text(duration)
    {
      DEFAULT_REFS
      item.duration = g_ascii_strtoull (text.c_str(), NULL, 10); 
    }

    HANDLER_Text(image)
    {
      DEFAULT_REFS
      item.image += text; 
    }

    HANDLER_Text(trackauth)
    {
      DEFAULT_REFS
      item.trackauth += text; 
    }

    HANDLER_Text(albumId)
    {
      DEFAULT_REFS
      item.albumId = g_ascii_strtoull (text.c_str(), NULL, 10); 
    }

    HANDLER_Text(artistId)
    {
      DEFAULT_REFS
      item.artistId = g_ascii_strtoull (text.c_str(), NULL, 10); 
    }
  }

  ///////////////////////////////////////////////////////////////////////////////////////

  static XPath PathTrackBase (XPath("playlist/trackList/track"));

  HandlerPair
  handlers_start[] = 
  {
    HandlerPair( XPath("playlist/trackList/track"), 
                  sigc::ptr_fun( &Handlers::Track::trackList_track))
  };
  
  HandlerEndPair
  handlers_end[] =
  {
    HandlerEndPair( XPath("playlist/trackList/track"), 
                  sigc::ptr_fun( &HandlersEnd::Track::trackList_track))
  };
  
  HandlerTextPair
  handlers_text[] = 
  {
      HandlerTextPair( XPath("playlist/title"),
        sigc::ptr_fun( &HandlersText::Playlist::title)),

      HandlerTextPair( PathTrackBase / XPath("location"),
        sigc::ptr_fun( &HandlersText::location)),

      HandlerTextPair( PathTrackBase / XPath("title"),
        sigc::ptr_fun( &HandlersText::title)),

      HandlerTextPair( PathTrackBase / XPath("id"),
        sigc::ptr_fun( &HandlersText::id)),

      HandlerTextPair( PathTrackBase / XPath("album"),
        sigc::ptr_fun( &HandlersText::album)),

      HandlerTextPair( PathTrackBase / XPath("creator"),
        sigc::ptr_fun( &HandlersText::creator)),

      HandlerTextPair( PathTrackBase / XPath("duration"),
        sigc::ptr_fun( &HandlersText::duration)),

      HandlerTextPair( PathTrackBase / XPath("image"),
        sigc::ptr_fun( &HandlersText::image)),

      HandlerTextPair( PathTrackBase / XPath("trackauth"),
        sigc::ptr_fun( &HandlersText::trackauth)),

      HandlerTextPair( PathTrackBase / XPath("albumId"),
        sigc::ptr_fun( &HandlersText::albumId)),

      HandlerTextPair( PathTrackBase / XPath("artistId"),
        sigc::ptr_fun( &HandlersText::artistId)),
  };
}

namespace MPX
{
  namespace XSPF
  {
    int XSPF_parse (MPX::XSPF::Playlist & playlist, char const* data, ssize_t size) 
    {
      XSPFParserContext context (playlist);

      for (unsigned int n = 0; n < G_N_ELEMENTS(handlers_start); context << handlers_start[n++]); 
      for (unsigned int n = 0; n < G_N_ELEMENTS(handlers_end); context << handlers_end[n++]); 
      for (unsigned int n = 0; n < G_N_ELEMENTS(handlers_text); context << handlers_text[n++]); 

      return SaxParserBase::xml_base_parse(data, size, context);
    }

    int XSPF_parse (MPX::XSPF::Playlist & playlist, std::string const& data)
    {
      XSPFParserContext context (playlist);

      for (unsigned int n = 0; n < G_N_ELEMENTS(handlers_start); context << handlers_start[n++]); 
      for (unsigned int n = 0; n < G_N_ELEMENTS(handlers_end); context << handlers_end[n++]); 
      for (unsigned int n = 0; n < G_N_ELEMENTS(handlers_text); context << handlers_text[n++]); 

      return SaxParserBase::xml_base_parse(data, context);
    }
  }
}
