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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif //HAVE_CONFIG_H

#include "mb-utils.hh"
#include <boost/algorithm/string.hpp>

namespace MPX
{
  namespace MusicBrainzUtil
  {
    bool 
    reverse_sortname (std::string & a)
    {
      boost::iterator_range <std::string::iterator> match = boost::find_nth (a, ", ", 0);
      if (!match.empty())
      {
        a = std::string (match.end(), a.end()) + " " + std::string (a.begin(), match.begin());
        return true;
      }

      return false;
    }
  }
}
