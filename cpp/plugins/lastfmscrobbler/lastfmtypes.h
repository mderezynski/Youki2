//    Copyright (C) 2008 Dirk Vanden Boer <dirk.vdb@gmail.com>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

/**
 * @file lastfmstypes.h
 * @brief Contains some enum definitions for Last.fm
 * @author Dirk Vanden Boer
 */

#ifndef LASTFM_TYPES_H
#define LASTFM_TYPES_H

/** The TrackSource enum contains possible sources fo submitted tracks.
 * Note: Currently Last.fm only supports UserChosen and Lastfm
 */
enum TrackSource
{
    UserChosen,                 /**< Chosen by the user */
    NonPersonalizedBroadCast,   /**< Non-personalised broadcast (e.g. Shoutcast, BBC Radio 1) */
    PersonalizedRecommendation, /**< Personalised recommendation except Last.fm (e.g. Pandora, Launchcast) */
    Lastfm,                     /**< Last.fm */
    Unknown                     /**< Unknown source */
};

/** The TrackRating enum contains possible ratings for a track.
 */
enum TrackRating
{
    Love,   /**< On any mode if the user has manually loved the track. This implies a listen */
    Ban,    /**< Only if source = Lastfm. This implies a skip, and the client should skip to the next track when a ban happens */
    Skip,   /**< Only if source = Lastfm */
    None    /**< If no rating is applicable */
};

#endif
