/***************************************************************************
    copyright            : (C) 2005 by Lukas Lalinsky
    email                : lalinsky@gmail.com
 ***************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it  under the terms of the GNU Lesser General Public License version  *
 *   2.1 as published by the Free Software Foundation.                     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  *
 *   USA                                                                   *
 ***************************************************************************/

#include "sidtag.h"

using namespace TagLib;

SID::Tag::Tag() : TagLib::Tag()
{
}

SID::Tag::~Tag()
{
}

String
SID::Tag::title() const
{
  return title_;  
}

String
SID::Tag::artist() const
{
  return artist_;  
}

String
SID::Tag::album() const
{
  return album_;
}

String
SID::Tag::copyright() const
{
  return copyright_;
}

String
SID::Tag::comment() const
{
  return comment_;
}

String
SID::Tag::rating() const
{
  return rating_;
}

unsigned
SID::Tag::year() const
{
  return 0;
}

unsigned
SID::Tag::track() const
{
  return 0;
}

String
SID::Tag::genre() const
{
  return String ("C64 SidTune"); 
}

void 
SID::Tag::setTitle(const String &value)
{
  title_ = value;
}

void 
SID::Tag::setArtist(const String &value)
{
  artist_ = value;  
}

void 
SID::Tag::setCopyright(const String &value)
{
  copyright_ = value;  
}

void 
SID::Tag::setComment(const String &value)
{
  comment_ = value;
}

void 
SID::Tag::setRating(const String &value)
{
  rating_ = value;
}

void 
SID::Tag::setAlbum(const String &value)
{
}

void 
SID::Tag::setGenre(const String &value)
{
}

void 
SID::Tag::setYear(unsigned value)
{
}

void 
SID::Tag::setTrack(unsigned value)
{
}

bool SID::Tag::isEmpty() const
{
  return TagLib::Tag::isEmpty() &&
         artist().isEmpty() &&
         album().isEmpty();
}
