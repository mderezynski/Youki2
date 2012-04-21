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

#ifndef MPX_MBXML_TYPES_V2
#define MPX_MBXML_TYPES_V2

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif //HAVE_CONFIG_H

#include <glibmm/ustring.h>
#include <vector>
#include <map>

namespace MPX
{
  namespace MusicBrainzXml
  {
    ////////////////////////// TYPES FOR PARSING RELEASE-LIST

    struct MusicBrainzReleaseEvent
    {
      std::string           releaseEventDate;
      std::string           releaseEventCountry;

      MusicBrainzReleaseEvent (std::string const& p_releaseEventDate, std::string const& p_releaseEventCountry)
      : releaseEventDate (p_releaseEventDate)
      , releaseEventCountry (p_releaseEventCountry)
      {}

    };
  
    struct MusicBrainzDisc
    {
      guint64               discSectors;
      std::string           discId;

      MusicBrainzDisc (guint64 p_discSectors, std::string const& p_discId)
      : discSectors (p_discSectors)
      , discId (p_discId)
      {}
    };

    struct MusicBrainzTrack
    {
      std::string           trackId;
      std::string           trackTitle;
      guint64               trackDuration;
      guint64               trackTrackNumber; // potentially synthesized

      std::string           artistId;
      std::string           artistName;
      std::string           artistSortName;
    };

    struct MusicBrainzArtist
    {
      std::string           artistId;
      std::string           artistType;
      std::string           artistName;
      std::string           artistSortName;
      std::string           artistDisambiguation;
      std::string           artistLifeSpanBegin;
      std::string           artistLifeSpanEnd;
    };


    typedef std::vector <MusicBrainzReleaseEvent>
      MusicBrainzReleaseEventV;

    typedef std::vector <MusicBrainzDisc>
      MusicBrainzDiscV;

    typedef std::vector <MusicBrainzTrack>
      MusicBrainzTrackV;

    struct MusicBrainzRelease
    {
      std::string             releaseId;
      std::string             releaseTitle;
      std::string             releaseType;
      std::string             releaseASIN;
     
      MusicBrainzArtist         mArtist; 
      MusicBrainzDiscV          mDiscV;
      MusicBrainzTrackV         mTrackV;
      MusicBrainzReleaseEventV  mReleaseEventV;

      guint64                  mDiscListCount;
      guint64                  mTrackListCount;
      guint64                  mTrackListOffset;

      // CTOR
      MusicBrainzRelease ()
      : mDiscListCount    (0)
      , mTrackListCount   (0)
      , mTrackListOffset  (0)
      {}

    };

    typedef std::vector <MusicBrainzRelease>
      MusicBrainzReleaseV;

    ////////////////////////// TYPES FOR PARSING TRACK-LIST

    struct MusicBrainzTracklistTrack
    {
      std::string           trackId;
      std::string           trackTitle;
      guint64               trackDuration;

      std::string           artistId;
      std::string           artistName;
      std::string           artistSortName;

      MusicBrainzReleaseV   mReleaseV;

      MusicBrainzTracklistTrack ()
      : trackDuration (0)
      {}
    };

    typedef std::vector <MusicBrainzTracklistTrack>
      MusicBrainzTracklistTrackV;
  }
}

#endif //MPX_MBXML_V2
