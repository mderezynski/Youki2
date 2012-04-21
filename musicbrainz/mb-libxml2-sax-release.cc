//  MPXx - The Dumb Music Player
//  Copyright (C) 2005-2007 MPXx development team.
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

#include <glibmm.h>

#include "mb-libxml2-sax-release.hh"

using namespace Glib;
using namespace std;
using namespace MPX::MusicBrainzXml;

#include "mpx/parser/libxml2-sax-base.hh"
using MPX::XPath;

namespace
{
  struct MbReleaseParseContext : public ParseContextBase
  {
    MusicBrainzReleaseV &         mReleaseV;
    unsigned int                  tracknum;
    
    MbReleaseParseContext (MusicBrainzReleaseV & v) : ParseContextBase(), mReleaseV (v), tracknum (1)  {}
  };

#define DEFAULT_REFS                                                                \
                                                                                    \
  MbReleaseParseContext &context (static_cast<MbReleaseParseContext&>(_context));   \
                                                                                    \
  MusicBrainzRelease &r G_GNUC_UNUSED (*context.mReleaseV.rbegin());                \
  MusicBrainzArtist &a G_GNUC_UNUSED (r.mArtist);

  namespace Handlers
  {
    HANDLER(release)
    {
      static_cast<MbReleaseParseContext&>(_context).mReleaseV.push_back(MusicBrainzRelease());

      // NOTE we get DEFAULT_REFS only after adding the release
      DEFAULT_REFS

      r.releaseId = props["id"];
    }

    HANDLER(artist)
    {
      DEFAULT_REFS

      a.artistId = props["id"];
    }

    HANDLER(track_artist)
    {
      DEFAULT_REFS

      MusicBrainzTrack & t (*r.mTrackV.rbegin());
      t.artistId = props["id"];
    }

    HANDLER(track_list)
    {
      DEFAULT_REFS

      if (props.count ("count") != 0)
      {
        r.mTrackListCount = g_ascii_strtoull (props["count"].c_str(), NULL, 10);
      }

      if (props.count ("offset") != 0)
      {
        r.mTrackListOffset = g_ascii_strtoull (props["offset"].c_str(), NULL, 10);
      }
    }

    HANDLER(track)
    {
      DEFAULT_REFS

      r.mTrackV.push_back (MusicBrainzTrack());
      MusicBrainzTrack & t (*r.mTrackV.rbegin());
      t.trackId = props["id"];
    }

    HANDLER(disc_list)
    {
      DEFAULT_REFS

      if (props.count ("count") != 0)
      {
        r.mDiscListCount = g_ascii_strtoull (props["count"].c_str(), NULL, 10);
      }
    }

    HANDLER(disc)
    {
      DEFAULT_REFS

      r.mDiscV.push_back (MusicBrainzDisc (guint64 (g_ascii_strtoull (props["sectors"].c_str(), NULL, 10)), props["id"]));
    }

    HANDLER(event)
    {
      DEFAULT_REFS

      r.mReleaseEventV.push_back (MusicBrainzReleaseEvent (props["date"], props["country"]));
    }
  }

  namespace HandlersEnd
  {
    HANDLER_END(track)
    {
      DEFAULT_REFS

      MusicBrainzTrack & t (*r.mTrackV.rbegin());

      if (t.artistId.empty())
        t.artistId = r.mArtist.artistId;

      if (t.artistName.empty())
        t.artistName = r.mArtist.artistName;

      if (t.artistSortName.empty())
        t.artistSortName = r.mArtist.artistSortName;

      t.trackTrackNumber = context.tracknum + r.mTrackListOffset;
      context.tracknum++;
    }
  };
  
  namespace HandlersText
  {
    HANDLER_Text(title)
    {
      DEFAULT_REFS

      r.releaseTitle += text;
    }

    HANDLER_Text(track_title)
    {
      DEFAULT_REFS

      MusicBrainzTrack & t (*r.mTrackV.rbegin());
      t.trackTitle += text;
    }

    HANDLER_Text(name)
    {
      DEFAULT_REFS

      a.artistName += text;
    }

    HANDLER_Text(track_name)
    {
      DEFAULT_REFS

      MusicBrainzTrack & t (*r.mTrackV.rbegin());
      t.artistName += text;
    }

    HANDLER_Text(sortname)
    {
      DEFAULT_REFS

      a.artistSortName += text;
    }

    HANDLER_Text(track_sortname)
    {
      DEFAULT_REFS

      MusicBrainzTrack & t (*r.mTrackV.rbegin());
      t.artistSortName += text;
    }

    HANDLER_Text(asin)
    {
      DEFAULT_REFS

      r.releaseASIN += text;
    }

    HANDLER_Text(duration)
    {
      DEFAULT_REFS

      MusicBrainzTrack & t (*r.mTrackV.rbegin());
      t.trackDuration = guint64 (g_ascii_strtoull (text.c_str(), NULL, 10));
    }

  }

  HandlerPair
  handlers_start[] = 
  {
    HandlerPair( "metadata/release", 
      sigc::ptr_fun( &Handlers::release)), 
    HandlerPair( "metadata/release/artist", 
      sigc::ptr_fun( &Handlers::artist)),
    
    HandlerPair( "metadata/release/disc-list", 
      sigc::ptr_fun( &Handlers::disc_list)),
    HandlerPair( "metadata/release/dist-list/disc", 
      sigc::ptr_fun( &Handlers::disc)),
    
    HandlerPair( "metadata/release/track-list", 
      sigc::ptr_fun( &Handlers::track_list)),
    HandlerPair( "metadata/release/track-list/track", 
      sigc::ptr_fun( &Handlers::track)), 
    HandlerPair( "metadata/release/track-list/track/artist", 
      sigc::ptr_fun( &Handlers::track_artist)), 

    HandlerPair( "metadata/release/release-event-list/event", 
      sigc::ptr_fun( &Handlers::event)), 
  };
  
  HandlerEndPair
  handlers_end[] =
  {
    HandlerEndPair( "metadata/release/track-list/track", 
      sigc::ptr_fun( &HandlersEnd::track))
  };


  HandlerTextPair
  handlers_text[] = 
  {
    HandlerTextPair( "metadata/release/title", 
      sigc::ptr_fun( &HandlersText::title)),
    HandlerTextPair( "metadata/release/artist/name", 
      sigc::ptr_fun( &HandlersText::name)), 
    HandlerTextPair( "metadata/release/artist/sort-name", 
      sigc::ptr_fun( &HandlersText::sortname)), 
    HandlerTextPair( "metadata/release/asin", 
      sigc::ptr_fun( &HandlersText::asin)), 
    
    HandlerTextPair( "metadata/release/track-list/track/title", 
      sigc::ptr_fun( &HandlersText::track_title)),
    HandlerTextPair( "metadata/release/track-list/track/artist/name", 
      sigc::ptr_fun( &HandlersText::track_name)), 
    HandlerTextPair( "metadata/release/track-list/track/artist/sort-name", 
      sigc::ptr_fun( &HandlersText::track_sortname)),
    HandlerTextPair( "metadata/release/track-list/track/duration",
      sigc::ptr_fun( &HandlersText::duration)),
  };
}

namespace MPX
{
  namespace MusicBrainzXml
  {
    int mb_xml_release_parse (std::string const &data, MusicBrainzReleaseV & releases)
    {
      MbReleaseParseContext context (releases); 

      for (unsigned int n = 0; n < G_N_ELEMENTS(handlers_start); context << handlers_start[n++]);
      for (unsigned int n = 0; n < G_N_ELEMENTS(handlers_end); context << handlers_end[n++]);
      for (unsigned int n = 0; n < G_N_ELEMENTS(handlers_text); context << handlers_text[n++]); 
       
      return SaxParserBase::xml_base_parse (data, context);
    }
  }
}
