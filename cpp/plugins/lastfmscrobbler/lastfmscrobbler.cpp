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

#include "lastfmscrobbler.h"

#include "scopedlock.h"
#include "log.h"

#include <boost/format.hpp>

#include <stdexcept>
#include <iostream>
#include <signal.h>
#include <algorithm>

#include "mpx/mpx-main.hh"
#include "mpx/i-youki-play.hh"

using namespace std;
using namespace utils;

static const time_t MIN_SECONDS_TO_SUBMIT       = 240;
static const time_t MIN_TRACK_LENGTH_TO_SUBMIT  = 30;
static const time_t MIN_SECS_BETWEEN_CONNECT    = 60;
static const time_t MAX_SECS_BETWEEN_CONNECT    = 7200;

LastFmScrobbler::LastFmScrobbler(bool synchronous, ILog & log )
: m_pLastFmClient(new LastFmClient())
, m_LastConnectionAttempt(0)
, m_AuthenticateThread(LastFmScrobbler::authenticateThread, this)
, m_SendInfoThread(LastFmScrobbler::sendInfoThread, this)
, m_FinishPlayingThread(LastFmScrobbler::finishPlayingThread, this)
, m_Authenticated(false)
, m_Disabled(true)
, m_HardConnectionFailureCount(0)
, m_Synchronous(synchronous)
, m_CommitOnly(false)
, m_Log(log)
{
    boost::shared_ptr<MPX::IPlay> p = MPX::services->get<MPX::IPlay>("mpx-service-play") ;

    p->signal_seek().connect(
        sigc::mem_fun(
              *this
            , &LastFmScrobbler::on_play_seek
    )) ;
}

LastFmScrobbler::~LastFmScrobbler()
{
    joinThreads();
    delete m_pLastFmClient;
}

void
LastFmScrobbler::set_enabled( bool enable )
{
    m_Disabled = !enable ;
}

void
LastFmScrobbler::set_credentials( const std::string& user, const std::string& pass)
{
    m_AuthenticateThread.cancel();
    
    m_Password = LastFmClient::generatePasswordHash(pass);
    m_Username = user ;
    m_Authenticated = false ;

    authenticateIfNecessary();
}

void LastFmScrobbler::setCommitOnlyMode(bool enabled)
{
    m_CommitOnly = enabled;
}

void LastFmScrobbler::on_play_seek( guint diff )
{
    m_SeekDiff += (-diff) ; 
}

void LastFmScrobbler::startedPlaying(const SubmissionInfo& info)
{
    m_CurrentTrackInfo = info;
    m_SeekDiff = 0. ;

    authenticateIfNecessary();

    if (m_Synchronous)
    {
        if (!m_CommitOnly)
        {
            setNowPlaying();
        }
    }
    else
    {
        m_SendInfoThread.start();
    }
}

void LastFmScrobbler::pausePlaying(bool paused)
{
}

void LastFmScrobbler::finishedPlaying()
{
    if( !m_CurrentTrackInfo )
        return ;

    boost::shared_ptr<MPX::IPlay> p = MPX::services->get<MPX::IPlay>("mpx-service-play") ;

    m_Elapsed = p->property_position().get_value() + m_SeekDiff ;
    m_SeekDiff = 0 ;

    authenticateIfNecessary();

    if (m_Synchronous)
    {
        submitTrack(m_CurrentTrackInfo.get());
    }
    else
    {
        m_FinishPlayingThread.start();
    }
}

bool LastFmScrobbler::trackCanBeCommitted(const SubmissionInfo& info)
{
    bool trackTooShort = info.getTrackLength() < MIN_TRACK_LENGTH_TO_SUBMIT;

    bool trackPlayedLongEnough =
            (m_Elapsed >= MIN_SECONDS_TO_SUBMIT)
        ||  (m_Elapsed >= (info.getTrackLength() / 2));

    if (trackTooShort)
    {
        logger::info( m_Log, "Track \"" + info.getTrack() + "\" can't be committed: length is too short");
    }
    else
    if (!trackPlayedLongEnough)
    {
        logger::info( m_Log, "Track \"" + info.getTrack() + "\" can't be committed: not played long enough");
    }
    else
   {
//        logger::info( m_Log, "Track \"" + info.getTrack() + "\" can be committed: conditions OK");
    }

    return (!trackTooShort) && trackPlayedLongEnough;
}

void LastFmScrobbler::authenticateIfNecessary()
{
    if (!m_Authenticated &&
        !m_AuthenticateThread.isRunning() &&
        canReconnect())
    {
        if (m_Synchronous)
        {
            authenticate();
        }
        else
        {
            m_AuthenticateThread.start();
        }
    }
}

void LastFmScrobbler::authenticate()
{
    try
    {
        m_pLastFmClient->handshake(m_Username, m_Password);
        logger::info( m_Log, "Authentication successful for user: \"" + m_Username + "\"" );
        m_HardConnectionFailureCount = 0;
        m_Authenticated = true;
    }
    catch (ConnectionError& e)
    {
        ++m_HardConnectionFailureCount;
        m_LastConnectionAttempt = time(NULL);
    }
    catch (logic_error& e)
    {
        logger::error( m_Log, e.what());
    }
}

bool LastFmScrobbler::canReconnect()
{
    if( m_Disabled )
        return false ;

    time_t curTime = time(NULL);
    time_t timeSinceLastConnectionAttempt = curTime - m_LastConnectionAttempt;
    time_t connectionDelay = max(MAX_SECS_BETWEEN_CONNECT, m_HardConnectionFailureCount * MIN_SECS_BETWEEN_CONNECT);

    return timeSinceLastConnectionAttempt > connectionDelay;
}

void* LastFmScrobbler::authenticateThread(void* pInstance)
{
    LastFmScrobbler* pScrobbler = reinterpret_cast<LastFmScrobbler*>(pInstance);
    logger::info( pScrobbler->m_Log, "Authenticating...");

    pScrobbler->authenticate();

    {
        ScopedLock lock(pScrobbler->m_AuthenticatedMutex);
        pScrobbler->m_AuthenticatedCondition.broadcast();
    }

    return NULL;
}

void* LastFmScrobbler::sendInfoThread(void* pInstance)
{
    LastFmScrobbler* pScrobbler = reinterpret_cast<LastFmScrobbler*>(pInstance);

    ScopedLock lock(pScrobbler->m_TrackInfosMutex);

    if (pScrobbler->m_Authenticated && pScrobbler->m_CurrentTrackInfo && !pScrobbler->m_Disabled)
    {
        if (!pScrobbler->m_CommitOnly)
        {
	    std::cerr << "LastFM: Setting NOW PLAYING" << std::endl ;
            pScrobbler->setNowPlaying();
	    return NULL ;
        }
    }

    std::cerr << "LastFM: No dice!!" << std::endl ;

    return NULL;
}

void* LastFmScrobbler::finishPlayingThread(void* pInstance)
{
    LastFmScrobbler* pScrobbler = reinterpret_cast<LastFmScrobbler*>(pInstance);

    ScopedLock lock(pScrobbler->m_TrackInfosMutex);

    {
        ScopedLock lock(pScrobbler->m_AuthenticatedMutex);
        if (!pScrobbler->m_Authenticated)
        {
            //Program is probalby cleaning up, dont't try to start authentication
            return NULL;
        }
    }

    if (pScrobbler->m_Authenticated && pScrobbler->m_CurrentTrackInfo && !pScrobbler->m_Disabled)
    {
        pScrobbler->submitTrack(pScrobbler->m_CurrentTrackInfo.get());
        pScrobbler->m_CurrentTrackInfo.reset() ;
    }

    return NULL;
}

void LastFmScrobbler::setNowPlaying()
{
    if (!m_Authenticated)
    {
        logger::info( m_Log, "Can't set Now Playing status: not authenticated");
        return;
    }

    try
    {
        if( m_CurrentTrackInfo )
        {
            m_pLastFmClient->nowPlaying(m_CurrentTrackInfo.get());
            logger::info( m_Log, "Now playing info submitted: \"" + m_CurrentTrackInfo.get().getArtist() + "\" - \"" + m_CurrentTrackInfo.get().getTrack() + "\"" );
        }
    }
    catch (BadSessionError& e)
    {
        logger::info( m_Log, "Session has become invalid: starting new handshake");
        authenticate();
        setNowPlaying();
    }
    catch (ConnectionError& e)
    {
        m_Authenticated = false;
    }
    catch (logic_error& e)
    {
        logger::error( m_Log, e.what());
    }
}

void LastFmScrobbler::submitTrack(const SubmissionInfo& info)
{
    if (info.getTrackLength() < 0 || !trackCanBeCommitted(info))
    {
        logger::debug( m_Log, "Track can not be committed");
        return;
    }
    else
    {
        //ScopedLock lock(m_TrackInfosMutex);
        m_BufferedTrackInfos.addInfo(info);
    }

    SubmissionInfoCollection tracksToSubmit;
    {
        //ScopedLock lock(m_TrackInfosMutex);
        tracksToSubmit = m_BufferedTrackInfos;
    }

    try
    {
        if (m_Authenticated)
        {
            m_pLastFmClient->submit(tracksToSubmit);
            logger::info( m_Log, "Tracks submitted...");
            m_BufferedTrackInfos.clear();
        }
        else
        {
            logger::warn( m_Log, "Track info queued: not currently connected");
        }
    }
    catch (BadSessionError& e)
    {
        logger::error( m_Log, "Session has become invalid: starting new handshake");
        authenticate();
        submitTrack(info);
    }
    catch (ConnectionError& e)
    {
        logger::error( m_Log, "Connection error ocurred");
        m_Authenticated = false;
    }
    catch (logic_error& e)
    {
        logger::error( m_Log, e.what());
    }
}

void LastFmScrobbler::joinThreads()
{
    if (m_Synchronous)
    {
        return;
    }

    {
        ScopedLock lock(m_AuthenticatedMutex);
        m_AuthenticatedCondition.broadcast();
    }

    m_AuthenticateThread.join();
    m_SendInfoThread.join();
    m_FinishPlayingThread.join();
}
