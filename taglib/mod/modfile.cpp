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
#include "modfile.h"
#include "modtag.h"
#include "modproperties.h"

#include <iostream>
#include <fstream>

using namespace TagLib;

namespace
{
    TagLib::MOD::Tag        * modtag     = 0;
    TagLib::MOD::Properties * properties = 0;
}

MOD::File::File (const char *file, bool readProperties, Properties::ReadStyle propertiesStyle) : TagLib::File (file)
{
  read (file, readProperties, propertiesStyle);
}

MOD::File::~File()
{
  if (properties)
    {
      delete properties;
    }

  if (modtag)
    {
      delete modtag;
    }
}

TagLib::Tag *MOD::File::tag() const
{
  return modtag;
} 

MOD::Tag *MOD::File::MODTag() const
{
  return modtag;
} 

MOD::Properties *MOD::File::audioProperties() const
{
  return properties;
} 

void MOD::File::read (const char * f, bool readProperties, Properties::ReadStyle /*propertiesStyle*/)
{
  int length = 0;
  
  modtag = new MOD::Tag();
  if(!modtag)
    return;

  std::ifstream file (f, std::ios::in|std::ios::binary|std::ios::ate);
  if( file.is_open())
  {
    std::ifstream::pos_type size;
    file.seekg (0, std::ios::end);
    size = file.tellg();
    file.seekg (0, std::ios::beg);
    char * memblock = new char [size];
    file.read (memblock, size);
    file.close ();

    ModPlugFile * mpfile = ModPlug_Load ((const void*)memblock, int(size));
    if (mpfile)
    {
      modtag->title_ = ModPlug_GetName (mpfile);
    }


    if (readProperties)
    {
      properties = new MOD::Properties();
      if (properties) 
        {
          length = ModPlug_GetLength (mpfile)/1000;
          properties->set (length);
        }
    }

    ModPlug_Unload (mpfile);
    delete memblock;
  }
}
