//  MPX
//  Copyright (C) 2005-2007 MPX development.
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License Version 2
//  as published by the Free Software Foundation.
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
//  The MPX project hereby grants permission for non-GPL compatible GStreamer
//  plugins to be used and distributed together with GStreamer and MPX. This
//  permission is above and beyond the permissions granted by the GPL license
//  MPX is covered by.

#ifndef MPX_UTIL_STRING_HH
#define MPX_UTIL_STRING_HH

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif //HAVE_CONFIG_H

#include <string>
#include <vector>
#include <glibmm/ustring.h>
#include "mpx/mpx-types.hh"

namespace MPX
{
  typedef std::vector<std::string> StrV;

  namespace Util
  {
    time_t
    parseRFC822Date(
        const char*
    ) ;

    std::string
    hex_string(
          void const*
        , std::size_t
    ) ;

    std::string
    md5_hex_string(
          void const*
        , std::size_t
    ) ;

    bool
    str_has_prefix_nocase(
          const std::string&
        , const std::string&
    ) ;

    bool
    str_has_suffix_nocase(
          const std::string&
        , const std::string&
    ) ;

    bool
    str_has_suffixes_nocase(
          const std::string&
        , char const**     
    ) ;

    bool
    str_has_suffixes_nocase(
          const std::string& 
        , const StrV&       
    ) ;

    bool
    match_keys(
          const Glib::ustring&
        , const Glib::ustring& 
    ) ;

    bool
    match_vec(
          const std::string&             
        , const std::vector<std::string>&
    ) ;

    std::string
    stdstrjoin(
          const StrV&
        , const std::string&
    ) ;

	std::string
    sanitize_lastfm(
          const std::string&
    ) ;

    std::string
    row_get_artist_name(
          const ::MPX::SQL::Row&
    ) ;

    std::string
    row_get_album_artist_name(
          const ::MPX::SQL::Row&
    ) ;


    Glib::ustring
    utf8_string_normalize(
          const Glib::ustring&
    ) ;

    std::string
    gprintf(
          const char *format
        , ...
    ) ;

    std::string
    text_match_highlight(
          const std::string&
        , const std::string&
        , const std::string&
    ) ;
  } // Util namespace
} // MPX namespace

#endif //!MPX_UTIL_STRING_HH
