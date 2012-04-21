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
  struct MbTracksParseContext : public ParseContextBase
  {
    MusicBrainzTracklistTrackV &  mTrackV;
    MbTracksParseContext (MusicBrainzTracklistTrackV & v) : mTrackV (v) {}
  };

#define DEFAULT_REFS                                                                            \
  MbTracksParseContext & context G_GNUC_UNUSED (static_cast<MbTracksParseContext&>(_context));  \
  MusicBrainzTracklistTrack & t G_GNUC_UNUSED (*context.mTrackV.rbegin());                      \
  MusicBrainzReleaseV & rr G_GNUC_UNUSED (t.mReleaseV);

#define RELEASE \
  MusicBrainzRelease & r G_GNUC_UNUSED (*t.mReleaseV.rbegin());

  namespace Handlers
  {
    HANDLER(release)
    {
      DEFAULT_REFS
      rr.push_back(MusicBrainzRelease());

      RELEASE
      r.releaseId = props["id"];
      return;
    }

    HANDLER(artist)
    {
      DEFAULT_REFS
      t.artistId = props["id"];
    }

    HANDLER(track_list)
    {
      DEFAULT_REFS
      RELEASE

#if 0
      if (props.count ("count") != 0)
      {
        r.mTrackListCount = g_ascii_strtoull (props["count"].c_str(), NULL, 10);
        return;
      }

      if (props.count ("offset") != 0)
      {
        r.mTrackListOffset = g_ascii_strtoull (props["offset"].c_str(), NULL, 10);
        return;
      }
#endif
    }

    HANDLER(track)
    {
      static_cast<MbTracksParseContext&>(_context).mTrackV.push_back (MusicBrainzTracklistTrack());

      // NOTE we get DEFAULT_REFS only after adding the track
      DEFAULT_REFS
      t.trackId = props["id"];
    }
  }

  namespace HandlersText
  {
    HANDLER_Text(title)
    {
      DEFAULT_REFS
      RELEASE
  
      t.trackTitle += text;
    }

    HANDLER_Text(release_title)
    {
      DEFAULT_REFS
      RELEASE
  
      r.releaseTitle += text;
    }

    HANDLER_Text(duration)
    {
      DEFAULT_REFS

      t.trackDuration = guint64 (g_ascii_strtoull (text.c_str(), NULL, 10));
    }

    HANDLER_Text(name)
    {
      DEFAULT_REFS

      t.artistName += text;
    }

    HANDLER_Text(sortname)
    {
      DEFAULT_REFS

      t.artistSortName += text;
    }
  }

  HandlerPair
  handlers_start[] = 
  {
    HandlerPair( XPath("metadata/track-list/track"), 
      sigc::ptr_fun(&Handlers::track)), 

    HandlerPair( XPath("metadata/track-list/track/artist"), 
      sigc::ptr_fun(&Handlers::artist)),

    HandlerPair( XPath("metadata/track-list/track/release-list/release"), 
      sigc::ptr_fun(&Handlers::release)),

    HandlerPair( XPath("metadata/track-list/track/release-list/release/track"), 
      sigc::ptr_fun(&Handlers::track_list))
  };
  
  HandlerTextPair
  handlers_text[] = 
  {
    HandlerTextPair( XPath("metadata/track-list/track/title"), 
      sigc::ptr_fun( &HandlersText::title)), 

    HandlerTextPair( XPath("metadata/track-list/track/duration"), 
      sigc::ptr_fun( &HandlersText::duration)),

    HandlerTextPair( XPath("metadata/track-list/track/artist/name"), 
      sigc::ptr_fun( &HandlersText::name)),

    HandlerTextPair( XPath("metadata/track-list/track-artist/sort-name"), 
      sigc::ptr_fun( &HandlersText::sortname)), 

    HandlerTextPair( XPath("metadata/track-list/release-list/release/track/title"), 
      sigc::ptr_fun( &HandlersText::release_title)) 
  };
}

namespace MPX
{
  namespace MusicBrainzXml
  {
    int mb_xml_tracks_parse (std::string const &data, MusicBrainzTracklistTrackV & tracks)
    {
      MbTracksParseContext context (tracks); 
    
      for (unsigned int n = 0; n < G_N_ELEMENTS(handlers_start); context << handlers_start[n++]); 
      for (unsigned int n = 0; n < G_N_ELEMENTS(handlers_text); context << handlers_text[n++]); 
       
      return SaxParserBase::xml_base_parse (data, context);
    }
  }
}
