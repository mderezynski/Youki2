//  MPX
//  Copyright (C) 2005-2007 MPX development.
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

#ifndef MPX_LYRICWIKI_HH
#define MPX_LYRICWIKI_HH

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif //HAVE_CONFIG_H
#include <exception>

#include "mpx/mpx-minisoup.hh"

namespace MPX
{

  class LyricsReturnNotOK : public std::runtime_error 
  { 
	public: 
        LyricsReturnNotOK (std::string const& msg = std::string()) : std::runtime_error(msg) {} 
  };

  namespace LyricWiki
  {
    class TextRequest
      : public Glib::Object
    {
      public:
  
        TextRequest (Glib::ustring const& artist, Glib::ustring const& title);
        std::string run ();
        virtual ~TextRequest ();

      private:

        Soup::RequestSyncRefP m_soup_request;
		Glib::ustring m_artist, m_title;
    };
  }
}

#endif //!MPX_LYRICWIKI_HH
