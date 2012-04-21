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

#include <tstring.h>
#include "sidfile.h"
#include "sidtag.h"
#include "sidproperties.h"

using namespace TagLib;

namespace
{
    TagLib::SID::Tag *sidtag = 0;
    TagLib::SID::Properties *properties = 0;
}

SID::File::File(const char *file, bool readProperties, Properties::ReadStyle propertiesStyle) : TagLib::File (file)
{
  read (readProperties, propertiesStyle);
}

SID::File::~File()
{
  if (tune)
    {
      delete tune;   
    }

  if (properties)
    {
      delete properties;
    }

  if (sidtag)
    {
      delete sidtag;
    }
}

TagLib::Tag *SID::File::tag() const
{
  return sidtag;
} 

SID::Tag *SID::File::SIDTag() const
{
  return sidtag;
} 

SID::Properties *SID::File::audioProperties() const
{
  return properties;
} 

void SID::File::read (bool readProperties, Properties::ReadStyle /*propertiesStyle*/)
{
  int length = 0;
  int bitrate = 0;
  int sampleRate = 0;
  int channels = 0;
  
  sidtag = new SID::Tag();
  if(!sidtag) return;

  tune = new sidTune (this->name());
  tune->getInfo (this->info);

  sidtag->setTitle (this->info.nameString);
  sidtag->setArtist (this->info.authorString);

  if (this->info.numberOfInfoStrings)
    {
      sidtag->setAlbum (this->info.infoString[0]);
    }
    
  if (readProperties)
    {
      properties = new SID::Properties();
      if (properties) 
        {
          properties->set (length, bitrate, sampleRate, channels);
        }
    }
}
