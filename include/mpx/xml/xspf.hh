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

#ifndef MPX_XSPF_TYPES_HH
#define MPX_XSPF_TYPES_HH

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif //HAVE_CONFIG_H

#include <string>
#include <vector>
#include <boost/cstdint.hpp>

namespace MPX
{
  namespace XSPF
  {
    struct Item
    {
      std::string   location;
      std::string   title;
      guint64      id;
      std::string   album;
      std::string   creator;
      guint64      duration;
      std::string   image;

      std::string   trackauth;
      guint64      albumId;
      guint64      artistId;

      std::string   rating;

      Item () : duration (0), rating ("") {}
    };

    typedef std::vector<XSPF::Item> ItemsT;

    struct Playlist
    {
      XSPF::ItemsT  Items;
      std::string   Title;
    };

    typedef ItemsT::iterator ItemIter;
  }
}

#endif //!MPX_XSPF_TYPES_HH
