/***************************************************************************
 *   Copyright (C) 2005 Paul Cifarelli                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *                                                                         *
 * portions (C) 2005 Martin Aumueller <aumuell@reserv.at>                  *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 51 Franklin St, 5th fl, Boston, MA 02110-1301,      *
 *   USA, or check http://www.fsf.org/about/contact.html                   *
 *                                                                         *
 ***************************************************************************/

#include <tfile.h>
#include <audioproperties.h>
#include <id3v1tag.h>

#include "resolver.h"
#include "taglib_realmediafile.h"
#include "rmff.h"

#include <src/util_string.hh>

namespace
{
  char* const suffixes[] =
  {
      "ra",
      "rv",
      "rm",
      "rmj",
      "rmvb",
      0,
  };
}

TagLib::File * RealMediaFileTypeResolver::createFile (const char *  filename,
                                                      bool          read_properties,
                                                      TagLib::AudioProperties::ReadStyle properties_style) const
{
  if (Bmp::Util::str_has_suffixes_nocase (filename, suffixes))
    {
      TagLib::RealMedia::File * p = new TagLib::RealMedia::File(filename, read_properties, properties_style);

      if (p->isValid())
         return p;
      else
         delete p;
   }

   return 0;
}
