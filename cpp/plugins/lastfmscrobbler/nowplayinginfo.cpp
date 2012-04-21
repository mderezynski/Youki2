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

#include "nowplayinginfo.h"

#include "stringoperations.h"

using namespace std;

NowPlayingInfo::NowPlayingInfo()
: m_TrackLengthInSecs(-1)
, m_TrackNr(-1)
{
}

NowPlayingInfo::NowPlayingInfo(const std::string& artist, const std::string& track)
: m_Artist(artist)
, m_Track(track)
, m_TrackLengthInSecs(-1)
, m_TrackNr(-1)
{
}

NowPlayingInfo::NowPlayingInfo(const std::wstring& artist, const std::wstring& track)
: m_TrackLengthInSecs(-1)
, m_TrackNr(-1)
{
    StringOperations::wideCharToUtf8(artist, m_Artist);
    StringOperations::wideCharToUtf8(track, m_Track);
}

string NowPlayingInfo::getPostData() const
{
    stringstream ss;
    ss  << "&a=" << StringOperations::urlEncode(m_Artist)
        << "&t=" << StringOperations::urlEncode(m_Track)
        << "&b=" << StringOperations::urlEncode(m_Album)
        << "&l=" << (m_TrackLengthInSecs > 0 ? StringOperations::getPostData(m_TrackLengthInSecs) : "")
        << "&n=" << (m_TrackNr > 0 ? StringOperations::getPostData(m_TrackNr) : "")
        << "&m=" << StringOperations::urlEncode(m_MusicBrainzId);

    return ss.str();
}

void NowPlayingInfo::setArtist(const std::string& artist)
{
    m_Artist = artist;
}

void NowPlayingInfo::setArtist(const std::wstring& artist)
{
    StringOperations::wideCharToUtf8(artist, m_Artist);
}

void NowPlayingInfo::setTrack(const std::string& track)
{
    m_Track = track;
}

void NowPlayingInfo::setTrack(const std::wstring& track)
{
    StringOperations::wideCharToUtf8(track, m_Track);
}

void NowPlayingInfo::setAlbum(const std::string& album)
{
    m_Album = album;
}

void NowPlayingInfo::setAlbum(const std::wstring& album)
{
    StringOperations::wideCharToUtf8(album, m_Album);
}

void NowPlayingInfo::setTrackLength(int lengthInSecs)
{
    m_TrackLengthInSecs = lengthInSecs;
}

void NowPlayingInfo::setTrackNr(int trackNr)
{
    m_TrackNr = trackNr;
}

void NowPlayingInfo::setMusicBrainzId(const std::string& musicBrainzId)
{
    m_MusicBrainzId = musicBrainzId;
}

void NowPlayingInfo::setMusicBrainzId(const std::wstring& musicBrainzId)
{
    StringOperations::wideCharToUtf8(musicBrainzId, m_MusicBrainzId);
}

const std::string& NowPlayingInfo::getArtist() const
{
    return m_Artist;
}

const std::string& NowPlayingInfo::getTrack() const
{
    return m_Track;
}

const std::string& NowPlayingInfo::getAlbum() const
{
    return m_Album;
}

int NowPlayingInfo::getTrackLength() const
{
    return m_TrackLengthInSecs;
}

int NowPlayingInfo::getTrackNr() const
{
    return m_TrackNr;
}

const std::string& NowPlayingInfo::getMusicBrainzId()
{
    return m_MusicBrainzId;
}
