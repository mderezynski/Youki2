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
 * @file lastfmscrobbler.h
 * @brief Contains the LastFmScrobbler class
 * @author Dirk Vanden Boer
 */

#ifndef LAST_FM_SCROBBLER_H
#define LAST_FM_SCROBBLER_H

#include <glibmm/timer.h>

#include <string>
#include <ctime>

#include <boost/optional.hpp>
#include <sigx/sigx.h>

#include "lastfmclient.h"
#include "submissioninfo.h"
#include "submissioninfocollection.h"
#include "condition.h"
#include "mutex.h"
#include "thread.h"
#include "i-log.hh"


class LastFmScrobbler
: public sigx::glib_threadable
{
protected:
	struct ThreadData ;
	Glib::Private<ThreadData> m_ThreadData ;

	virtual void
	on_startup() {} 

	virtual void
	on_cleanup() {} 

public:
    /** Constructor
     * \param user Last.fm user name
     * \param pass Last.fm password for user
     * \param hashedPass true if the password is hashed, false otherwise
     * \param synchronous if false all public methods will be executed in
     * a thread and return immediately (prevents long blocking methods in
     * case of network problems)
     */
    LastFmScrobbler(bool synchronous, ILog&);
    /** Destructor */
    virtual ~LastFmScrobbler();

    /** When commit only mode is set, now playinginfo is not updated on
     * Last.fm, tracks will only be commited once they finished playing
     * \param enabled set commitOnlyMode to true or false
     */
    void setCommitOnlyMode(bool enabled);

    /** Indicate that a new track has started playing, the previous track
     * will be submitted (if available) and the new track will be set as
     * Now Playing
     * \param info SubmissionInfo object containing information about
     * the new song
     */

    sigx::request_f<const SubmissionInfo&> startedPlaying ;
    void on_startedPlaying(const SubmissionInfo& info);
    /** Indicate that the current track has stopped playing. The current
     * track will be submitted to Last.fm
     */

    sigx::request_f<> finishedPlaying ;
    void on_finishedPlaying();
    /** Indicate that playback of the current track has been (un)paused
     * \param paused true if track is being paused, false if being unpaused
     */

    sigx::request_f<bool> pausePlaying ;
    void on_pausePlaying(bool paused);

    /** Sets new credentials and reauthenticates
     *
     */
    sigx::request_f<const std::string&, const std::string&> set_credentials ;
    void on_set_credentials(const std::string& user, const std::string& pass);

    /** Set authentication to false 
     *
     */
    sigx::request_f<bool> set_enabled ;
    void on_set_enabled( bool ) ;

    sigx::request_f<int> play_seek ;
    void on_play_seek(int) ;

protected:
    LastFmClient*   m_pLastFmClient;
    /** \brief Last time a connection attempt was made */
    time_t          m_LastConnectionAttempt;
    /** \brief The time that the current track has been played, is set on pause */
    //time_t          m_TrackPlayTime;
    /** \brief The time that the current track was resumed after a pause */
    //time_t          m_TrackResumeTime;
    // Glib::Timer     m_TrackPlayTimer ;
    double          m_Elapsed ;
    double          m_SeekDiff ;
    /** \brief Thread handle of authentication thread (protected for testing) */
    utils::Thread   m_AuthenticateThread;
    /** \brief Thread handle of sendinfo thread (protected for testing) */
    utils::Thread   m_SendInfoThread;
    /** \brief Thread handle of finishPlaying thread (protected for testing) */
    utils::Thread   m_FinishPlayingThread;

private:

    void authenticateIfNecessary();
    void authenticate();
    bool trackCanBeCommitted(const SubmissionInfo& info);
    bool canReconnect();
    void submitTrack(const SubmissionInfo& info);
    void setNowPlaying();

    void startThread(pthread_t* pThread, utils::ThreadFunction pfnThreadFunction);
    void joinThreads();

    static void* authenticateThread(void* pInstance);
    static void* sendInfoThread(void* pInstance);
    static void* finishPlayingThread(void* pInstance);

    boost::optional<SubmissionInfo> m_PreviousTrackInfo ;
    boost::optional<SubmissionInfo> m_CurrentTrackInfo ;

    SubmissionInfoCollection    m_BufferedTrackInfos ;

    bool                        m_Authenticated ;
    bool                        m_Disabled ;
    int                         m_HardConnectionFailureCount ;
    utils::Condition            m_AuthenticatedCondition ;
    utils::Mutex                m_AuthenticatedMutex ;
    utils::Mutex                m_TrackInfosMutex ;
    utils::Mutex                m_TimerMutex ;

    std::string                 m_Username ;
    std::string                 m_Password ;

    bool                        m_Synchronous ;
    bool                        m_CommitOnly ;

    ILog                       & m_Log ;
};

#endif
