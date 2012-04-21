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
//  The MPX project hereby grants permission for non GPL-compatible GStreamer
//  plugins to be used and distributed together with GStreamer and MPX. This
//  permission is above and beyond the permissions granted by the GPL license
//  MPX is covered by.

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif //HAVE_CONFIG_H

#include <iostream>
#include <fstream>

#include <boost/format.hpp>
#include <glibmm.h>

#include "mpx/mpx-lyrics.hh"

#include "mpx/mpx-uri.hh"
#include "mpx/util-string.hh"
#include "mpx/xml/xml.hh"

using namespace Glib;
using boost::format;
using std::string;

namespace MPX
{
  namespace LyricWiki
  {
    TextRequest::TextRequest (const ustring &artist,
                              const ustring &title)
    : m_soup_request  (Soup::RequestSyncRefP (0))
    , m_artist        (artist)
    , m_title         (title)
    {
      string collation = artist.lowercase() + "_" + title.lowercase();
      string collation_md5_hex = Util::md5_hex_string (collation.data (), collation.size ());
    }
    
	std::string
    TextRequest::run ()
    {
        boost::format lyrics_url_f ("http://lyricwiki.org/api.php?fmt=text&artist=%s&song=%s");
        URI request_url ((lyrics_url_f % m_artist.c_str() % m_title.c_str()).str(), true);

        m_soup_request = Soup::RequestSync::create ((ustring(request_url)));
        m_soup_request->add_header("User-Agent", "MPX-1.0");
        m_soup_request->add_header("Content-Type", "text/xml"); 
        m_soup_request->add_header("Accept-Charset", "utf-8");
        m_soup_request->add_header("Connection", "close");

        g_message("%s: LyricWiki request: '%s'", G_STRLOC, (ustring(request_url)).c_str());

        int code = m_soup_request->run ();
		if( code != 200)
        {
			throw LyricsReturnNotOK();
        }

		std::string lyrics (m_soup_request->get_data_raw(), m_soup_request->get_data_size());

#if 0
        try{
                lyrics = xpath_get_text(
                    m_soup_request->get_data_raw(),
                    m_soup_request->get_data_size(),
                    "//lyrics",
                    NULL
                );
        } catch( ... )
        {
            throw LyricsReturnNotOK();
        }
#endif

		return lyrics;
    }

    TextRequest::~TextRequest ()
    {
    }

  }
}
