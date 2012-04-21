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
 * @file submissioninfo.h
 * @brief Contains the SubmissionInfo class
 * @author Dirk Vanden Boer
 */

#ifndef SUBMISSION_INFO_H
#define SUBMISSION_INFO_H

#include <ctime>
#include <vector>
#include <string>

#include "nowplayinginfo.h"
#include "lastfmtypes.h"

/** The NowPlayingInfo class contains all the necessary information to
 *  submit a played track to Last.Fm. Artist, Track and StartTime are
 *  required fields that must be set. The class inherits from
 *  NowPlayingInfo and contains some additional information that could
 *  be needed to submit a track.
 */
class SubmissionInfo : public NowPlayingInfo
{
public:
    /** \brief Default constructor */
    SubmissionInfo();
    /** \brief Constructor that sets artist ,track and optionally the time the track started playing */
    SubmissionInfo(const std::string& artist, const std::string& title, time_t timeStarted = -1);
    /** \brief Constructor that sets artist ,track (unicode) and optionally the time the track started playing */
    SubmissionInfo(const std::wstring& artist, const std::wstring& title, time_t timeStarted = -1);

    /** \brief returns the postdata needed to submit the info to Last.fm, used by LastFmClient */
    std::string getPostData(int index = 0) const;
    /** \brief returns the time track started playing */
    time_t getTimeStarted() const;

    /** Set the source of the track
     * \param source the source type
     * \param recommendationKey The 5-digit Last.fm recommendation key.
     * Only needs to be supplied when the source is set to Lastfm.
     */
    void setSource(TrackSource source, const std::string& recommendationKey = "");
    /** Set the rating of the track
     * \param rating the track rating
     */
    void setRating(TrackRating rating);
    /** Set the time when the track started playing
     * \param timeStarted The time in UNIX timestamp format
     * (integer number of seconds since 00:00:00, January 1st 1970 UTC).
     * This must be in the UTC time zone.
     */
    void setTimeStarted(time_t timeStarted);

private:
    time_t          m_TimeStarted;
    TrackSource     m_Source;
    TrackRating     m_Rating;
    std::string     m_RecommendationKey;
};

#endif
