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

#include <iostream>
#include <glibmm.h>
#include <glibmm/markup.h>
#include <boost/format.hpp>

#include "mpx/mpx-minisoup.hh"
#include "mpx/mpx-uri.hh"
#include "mpx/mpx-types.hh"
#include "mb-libxml2-sax-release-list.hh"
#include "mb-libxml2-sax-release.hh"
#include "mb-libxml2-sax-tracks.hh"
#include "mbxml-types-v2.hh"

using namespace Glib;

namespace
{
  inline void opt_assign_string(
        MPX::OVariant&                  var,
        const std::string&              str
  )
  {
    if( !str.empty() )
      var = str;
    else
      var.reset ();
  }
}


namespace MPX
{
  namespace MusicBrainzXml
  {
    void
    imbue_track(
        const MusicBrainzTrack& m,
        Track&                  x
    )
    {
      x[ATTRIBUTE_TRACK] = (unsigned int)(m.trackTrackNumber);
      x[ATTRIBUTE_TIME] = (unsigned int)(m.trackDuration/1000);

      opt_assign_string(
          x[ATTRIBUTE_MB_TRACK_ID],
          m.trackId
      );

      opt_assign_string(
          x[ATTRIBUTE_TITLE],
          m.trackTitle
      );

      opt_assign_string(
          x[ATTRIBUTE_MB_ARTIST_ID],
          m.artistId
      );

      opt_assign_string(
          x[ATTRIBUTE_ARTIST],
          m.artistName
      );

      opt_assign_string(
          x[ATTRIBUTE_ARTIST_SORTNAME],
          m.artistSortName
      );
    }

    void
    imbue_album(
        const MusicBrainzRelease& m,
        Track&                    x,
        MusicBrainzReleaseV::size_type release_event_index)
    {
      opt_assign_string(
            x[ATTRIBUTE_MB_ALBUM_ID],
            m.releaseId
      );

      opt_assign_string(
            x[ATTRIBUTE_ALBUM],
            m.releaseTitle
      );

      opt_assign_string(
            x[ATTRIBUTE_MB_ALBUM_ARTIST_ID],
            m.mArtist.artistId
      );

      opt_assign_string(
            x[ATTRIBUTE_ALBUM_ARTIST],
            m.mArtist.artistName
      );

      opt_assign_string(
            x[ATTRIBUTE_ALBUM_ARTIST_SORTNAME],
            m.mArtist.artistSortName
      );

      opt_assign_string(
            x[ATTRIBUTE_ASIN],
            m.releaseASIN
      );

      x[ATTRIBUTE_DATE].reset();
      x[ATTRIBUTE_MB_RELEASE_DATE].reset();

      if( m.mReleaseEventV.size() ) 
      {
        std::string release_date = m.mReleaseEventV[release_event_index].releaseEventDate;

        if( release_date.length() == 10 )
        {
          x[ATTRIBUTE_MB_RELEASE_DATE] = release_date; 
        }
        else if( release_date.length() == 7 )
        {
          static boost::format date_f ("%s-01");
          x[ATTRIBUTE_MB_RELEASE_DATE] = (date_f % release_date.c_str()).str();
          x[ATTRIBUTE_DATE] = (unsigned int)(g_ascii_strtoull (release_date.substr (0, 4).c_str(), NULL, 10));
        }
        else if( release_date.length() == 4 )
        {
          static boost::format date_f ("%s-01-01");
          x[ATTRIBUTE_MB_RELEASE_DATE] = (date_f % release_date.c_str()).str();
          x[ATTRIBUTE_DATE] = (unsigned int)(g_ascii_strtoull (release_date.substr (0, 4).c_str(), NULL, 10));
        }
      }
    }

    void
    mb_releases_query (Glib::ustring const& artist,
                       Glib::ustring const& album,
                       MusicBrainzReleaseV & releases)
    {
      if( artist.empty() && album.empty() )
        return;

      static boost::format 
          query_album_artist_f ("ws/1/release/?type=xml&title=%s&artist=%s&releasetypes=Official&inc=release-events"),
          query_album_f ("ws/1/release/?type=xml&title=%s&releasetypes=Official&inc=release-events"),
          query_artist_f ("ws/1/release/?type=xml&artist=%s&releasetypes=Official&inc=release-events");

      std::string path;

      if( !artist.empty() && !album.empty() )
        path = (query_album_artist_f % MPX::URI::escape_string (album) % MPX::URI::escape_string (artist)).str();
      else if( !artist.empty() && album.empty() )
        path = (query_artist_f % MPX::URI::escape_string (artist)).str();
      else if( artist.empty() && !album.empty() )
        path = (query_album_f % MPX::URI::escape_string (album)).str();
     
      URI u ((boost::format ("http://musicbrainz.org/%s") % path).str());

      Soup::RequestSyncRefP request = Soup::RequestSync::create (ustring (u));
      guint code = request->run ();

      if (code == 200)
      {
        std::string data;
        request->get_data (data);
        mb_xml_release_list_parse (data, releases);
      }
    }

    void
    mb_tracks_by_puid (Glib::ustring const& puid, MusicBrainzTracklistTrackV & tracks)
    {
      if( puid.empty() )
        return;

      URI u ((boost::format ("http://musicbrainz.org/ws/1/track/?puid=%s&type=xml&inc=releases") % puid).str());
      Soup::RequestSyncRefP request = Soup::RequestSync::create (ustring (u));
      guint code = request->run ();

      if (code == 200)
      {
        std::string data;
        request->get_data (data);
        mb_xml_tracks_parse (data, tracks);
      }
    }

    void
    mb_releases_by_id (Glib::ustring const& mb_release_id, MusicBrainzReleaseV & releases)
    {
      if( mb_release_id.empty() )
        return;

      MPX::URI u ((boost::format
        ("http://musicbrainz.org/ws/1/release/%s?type=xml&inc=artist+counts+release-events+discs+"
         "tracks+artist-rels+label-rels+release-rels+track-rels+url-rels+track-level-rels+labels") % mb_release_id).str());

      Soup::RequestSyncRefP request = Soup::RequestSync::create (ustring (u));
      guint code = request->run ();

      if (code == 200)
      {
        std::string data;
        request->get_data (data);
        mb_xml_release_parse (data, releases);
      }
    }

    void
    mb_releases_by_disc_id (std::string const& disc_id, MusicBrainzReleaseV & releases)
    {
      if( disc_id.empty() )
        return;

      URI u ((boost::format ("http://musicbrainz.org/ws/1/release/?discid=%s&type=xml&"
                              "inc=artist+tracks+counts+release-events") % disc_id).str());
      Soup::RequestSyncRefP request = Soup::RequestSync::create (ustring (u));
      guint code = request->run ();

      if (code == 200)
      {
        std::string data;
        request->get_data (data);
        MusicBrainzReleaseV r;
        mb_xml_release_list_parse (data, r);
        for (MusicBrainzReleaseV::const_iterator i = r.begin(); i != r.end(); ++i)
        {
          mb_releases_by_id (i->releaseId, releases);
        }
      }
    }
  }
}
