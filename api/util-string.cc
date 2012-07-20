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
//  The MPX project hereby grants permission for non-GPL compatible GStreamer
//  plugins to be used and distributed together with GStreamer and MPX. This
//  permission is above and beyond the permissions granted by the GPL license
//  MPX is covered by.

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif //HAVE_CONFIG_H

#include <glibmm.h>

#include <boost/algorithm/string.hpp>
#define BOOST_REGEX_MATCH_EXTRA 1
#include <boost/regex.hpp>

#include "mpx/algorithm/md5.h"
#include "mpx/util-string.hh"

using namespace Glib;

namespace
{
  const struct {
      const char *exp;
      const char *fmt;
  } lastfm_regexes[] = {

     { "(\\[[^\\]]+\\])",
       "(?1)"},

     { "(\\[\\/[^\\]]+\\])",
       "(?1)"},
  };

  // This table of RFC822 timezones is from gmime-utils.c of the gmime API */
  const struct {
    char const* name;
    int   offset;
  } tz_offsets [] = {
    { "UT",      0 },
    { "GMT",     0 },
    { "EST", - 500 },        // These are all US timezones.  Bloody Yanks!!
    { "EDT", - 400 },
    { "CST", - 600 },
    { "CDT", - 500 },
    { "MST", - 700 },
    { "MDT", - 600 },
    { "PST", - 800 },
    { "PDT", - 700 },
    { "Z",       0 },
    { "A",   - 100 },
    { "M",   -1200 },
    { "N",     100 },
    { "Y",    1200 }
  };

  // Returns timezone offset in seconds
  // Code (C) Liferea Developers
  time_t common_parse_rfc822_tz (char *token)
  {
    int offset = 0;
    const char *inptr = token;

    if (*inptr == '+' || *inptr == '-')
    {
      offset = atoi (inptr);
    }
    else
    {
      int t;

      if (*inptr == '(') inptr++;

      for (t = 0; t < 15; t++)
      {
        if (!strncmp (inptr, tz_offsets[t].name, strlen (tz_offsets[t].name))) offset = tz_offsets[t].offset;
      }
    }
    return 60 * ((offset / 100) * 60 + (offset % 100));
  }
}

namespace MPX
{
namespace Util
{
    // Converts a RFC822 time string to a time_t value
    // Code (C) Liferea Developers
    time_t parseRFC822Date (const char * date)
    {
      struct tm	tm;
      time_t t, t2;
      char 	 *oldlocale;
      char	 *pos;
      gboolean	success = FALSE;

      memset (&tm, 0, sizeof(struct tm));

      // We expect at least something like "03 Dec 12 01:38:34"
      // and don't require a day of week or the timezone.
      //
      // The most specific format we expect:  "Fri, 03 Dec 12 01:38:34 CET"

      // Skip day of week
      if (NULL != (pos = g_utf8_strchr(date, -1, ','))) date = ++pos;

      // We expect English month names, so we set the locale
      oldlocale = g_strdup (setlocale (LC_TIME, NULL));
      setlocale (LC_TIME, "C");

      // Standard format with seconds and 4 digit year
      if (0 != (pos = strptime((const char *)date, " %d %b %Y %T", &tm)))
        success = TRUE;
      // Non-standard format without seconds and 4 digit year
      else if (0 != (pos = strptime((const char *)date, " %d %b %Y %H:%M", &tm)))
        success = TRUE;
      // Non-standard format with seconds and 2 digit year
      else if (0 != (pos = strptime((const char *)date, " %d %b %y %T", &tm)))
        success = TRUE;
      // Non-standard format without seconds 2 digit year
      else if (0 != (pos = strptime((const char *)date, " %d %b %y %H:%M", &tm)))
        success = TRUE;

      while (pos != 0 && *pos != '\0' && isspace((int)*pos)) pos++; // skip whitespaces before timezone

      if (0 != oldlocale)
      {
        setlocale (LC_TIME, oldlocale);	// Reset Locale
        g_free (oldlocale);
      }

      if (TRUE == success)
      {
        if((time_t)(-1) != (t = mktime(&tm)))
        {

          //
          // GMT time, with no daylight savings time
          // correction. (Usually, there is no daylight savings
          // time since the input is GMT.)
          //

          t = t - common_parse_rfc822_tz (pos);
          t2 = mktime (gmtime(&t));
          t = t - (t2 - t);
          return t;
        }
        else
        {
          g_warning ("Internal error! time conversion error! mktime failed!\n");
        }
      }

      return 0;
    }

    std::string
    hex_string (void const* data,
                std::size_t len)
    {
      static const char hexchars[] = "0123456789abcdef";

      guint8 const* byte_array = static_cast<guint8 const*> (data);

      std::string s;
      s.reserve (len * 2);

      for(unsigned int i = 0; i < len; i++)
        {
          s.push_back (hexchars[byte_array[i] >> 4]);
          s.push_back (hexchars[byte_array[i] & 0x0f]);
        }

      return s;
    }

    std::string
    md5_hex_string (void const* data,
                    std::size_t len)
    {
        md5_state_t md5state;
        md5_byte_t  md5pword[16];

        md5_init (&md5state);
        md5_append (&md5state, static_cast<md5_byte_t const*> (data), static_cast<int> (len));
        md5_finish (&md5state, md5pword);

        return hex_string (md5pword, sizeof (md5pword));
    }

    bool
    str_has_prefix_nocase (std::string const& str,
                           std::string const& prefix)
    {
      if (str.empty () || prefix.empty ())
        return false;

      return (g_ascii_strncasecmp (str.c_str (), prefix.c_str (), prefix.length ()) == 0);
    }
    

    bool
    str_has_suffix_nocase (std::string const& str,
                           std::string const& suffix)
    {
      if (str.empty () || suffix.empty ())
        return false;

      return (g_ascii_strcasecmp (str.c_str () + str.length () - suffix.length (), suffix.c_str ()) == 0);
    }

    bool
    str_has_suffixes_nocase (std::string const& str,
                             char const**       suffixes)
    {
      if (str.empty () || !suffixes)
        return false;

      for (char const** suffix = suffixes; *suffix; ++suffix)
        {
          if (str_has_suffix_nocase (str, std::string (*suffix)))
            return true;
        }

      return false;
    }

    bool
    str_has_suffixes_nocase (std::string const& str,
                             StrV const&        strv)
    {
      if (str.empty () || strv.empty ())
        return false;

      for (StrV::const_iterator i = strv.begin () ; i != strv.end (); ++i)
        {
          if (str_has_suffix_nocase (str, *i))
            return true;
        }

      return false;
    }

    std::string
    stdstrjoin (StrV const& strings, std::string const& delimiter)
    {
      std::string result;
      StrV::const_iterator e = strings.end(); 
      --e;
      for (StrV::const_iterator i = strings.begin(); i != strings.end(); ++i) 
      {
        result += *i; 
      	if (i != e)
      	{
	        result += delimiter;
        }
      }
      return result;
    }

    std::string
    row_get_album_artist_name(
          const MPX::SQL::Row& r
    )
    {
        std::string name ;

        if( r.count("album_artist") ) 
        {
            Glib::ustring in_utf8 = boost::get<std::string>(r.find("album_artist")->second) ; 
            gunichar c = in_utf8[0] ;

            if( g_unichar_get_script( c ) != G_UNICODE_SCRIPT_LATIN && r.count("album_artist_sortname") ) 
            {
                    std::string in = boost::get<std::string>( r.find("album_artist_sortname")->second ) ; 

                    boost::iterator_range <std::string::iterator> match1 = boost::find_nth( in, ", ", 0 ) ;
                    boost::iterator_range <std::string::iterator> match2 = boost::find_nth( in, ", ", 1 ) ;

                    if( !match1.empty() && match2.empty() ) 
                    {
                        name = std::string (match1.end(), in.end()) + " " + std::string (in.begin(), match1.begin());
                    }
                    else
                    {
                        name = in ;
                    }

                    return name ;
            }

            name = in_utf8 ;
        }

        return name ;
    }

    std::string
    row_get_artist_name(
          const MPX::SQL::Row& r
    )
    {
        std::string name ;

        if( r.count("artist") ) 
        {
            Glib::ustring in_utf8 = boost::get<std::string>(r.find("artist")->second) ; 
            gunichar c = in_utf8[0] ;

            if( g_unichar_get_script( c ) != G_UNICODE_SCRIPT_LATIN && r.count("artist_sortname") ) 
            {
                    std::string in = boost::get<std::string>( r.find("artist_sortname")->second ) ; 

                    boost::iterator_range <std::string::iterator> match1 = boost::find_nth( in, ", ", 0 ) ;
                    boost::iterator_range <std::string::iterator> match2 = boost::find_nth( in, ", ", 1 ) ;

                    if( !match1.empty() && match2.empty() ) 
                    {
                        name = std::string (match1.end(), in.end()) + " " + std::string (in.begin(), match1.begin());
                    }
                    else
                    {
                        name = in ;
                    }

                    return name ;
            }

            name = in_utf8 ;
        }

        return name ;
    }

    Glib::ustring
    utf8_string_normalize (Glib::ustring const& in)
    {
      StrV uppercased;

      if (in.empty())
        return Glib::ustring();

      char ** splitted = g_strsplit_set (in.c_str(), " -", -1);
      int n = 0;

      while (splitted[n])
      {
        if (strlen (splitted[n]))
        {
          ustring o  = ustring (splitted[n]).normalize().lowercase();
          ustring ol = o.substr (1);
          ustring of = ustring (1, o[0]).uppercase();
          ustring compose = (of + ol);
          uppercased.push_back (compose);
        }
        ++n;
      }

      g_strfreev (splitted);
      ustring norm = stdstrjoin (uppercased, " ");
      return norm; 
    }

    bool
    match_keys (ustring const& _h,
                ustring const& _n)
    {
        using boost::algorithm::split;
        using boost::algorithm::is_any_of;
        using boost::algorithm::find_first;

        StrV m;

        std::string n (_n.lowercase());
        std::string h (_h.lowercase());

        split( m, n, is_any_of(" ") );

        for (StrV::const_iterator i = m.begin (); i != m.end (); ++i)
        {
			if (i->length() < 1)
				continue;

	        if (!find_first (h, (*i)))
				return false;
        }
        return true;
    }

    bool
    match_vec(
          const std::string&              n
        , const std::vector<std::string>& h
    )
    {
        Glib::ustring&& nl = Glib::ustring(n).casefold() ;

        for( std::size_t i = 0 ; i < h.size(); ++i )
        {
	    Glib::ustring&& hl = Glib::ustring(h[i]).casefold() ;

    	    if(hl.find(nl) != Glib::ustring::npos)
            {
                return true ;
            }
        }

        return false ;
    }

    std::string
    sanitize_lastfm (std::string const& in)
    {
      std::string out = in;
      try {
        boost::regex e1;
        for (unsigned int n = 0; n < G_N_ELEMENTS(lastfm_regexes); ++n)
        {
          e1.assign (lastfm_regexes[n].exp);
          out = boost::regex_replace (out, e1, lastfm_regexes[n].fmt, boost::match_default | boost::format_all | boost::match_extra);
        }
       }
      catch (boost::regex_error & cxe)
      {
        g_warning ("%s: Error during Last.FM Markup sanitize: %s", G_STRLOC, cxe.what());
      }
      return out; 
    }

    std::string
    gprintf (const char *format, ...)
    {
      va_list args;
      va_start (args, format);
      Glib::ScopedPtr<char> s (g_strdup_vprintf (format, args));
      va_end (args);
      return std::string(s.get());
    }

    std::string
    text_match_highlight(
            const std::string& text_,
            const std::string& matches_,
            const std::string& color
    )  
    {
	    const std::string localcolor = "#303030" ;
	
            std::string text ( text_ );
            std::string matches ( matches_ );

            using namespace boost::algorithm;

            typedef boost::iterator_range<std::string::iterator>    Range;
            typedef std::vector< std::string >                      SplitVectorType;
            typedef std::map<std::string::size_type, int>           IndexSet; 
            typedef std::list<Range>                                Results;

            SplitVectorType split_vec; // #2: Search for tokens
            boost::algorithm::split( split_vec, matches, boost::algorithm::is_any_of(" ") );

            std::sort( split_vec.begin(), split_vec.end() );
            std::reverse( split_vec.begin(), split_vec.end());

            IndexSet i_begin;
            IndexSet i_end;

            for( SplitVectorType::const_iterator i = split_vec.begin(); i != split_vec.end(); ++i )
            {
                Results x; 
                ifind_all(x, text, *i);

                for(Results::const_iterator i = x.begin(); i != x.end(); ++i )
                {
                    i_begin[std::distance(text.begin(), (*i).begin())] ++;
                    i_end[std::distance(text.begin(), (*i).end())] ++;
                }
            }

            if( ! i_begin.empty() )
            {
                    std::string output;
                    output.reserve(1024);

                    if( i_begin.size() == 1 && (*(i_begin.begin())).first == 0 && (*(i_end.begin())).first == text.size() )    
                    {
                        output += "<span weight='bold'><i>" + Glib::Markup::escape_text(text).raw() + "</i></span>";
                    }
                    else
                    {
                            std::string chunk;
                            chunk.reserve(1024);
                            int c_open = 0;
                            int c_close = 0;

                            for(std::string::iterator i = text.begin(); i != text.end(); ++i)
                            {
                                std::string::size_type idx = std::distance(text.begin(), i);

                                if( i_begin.count(idx) && i_end.count(idx) )
                                {
                                    /* do nothing */
                                }
                                else
                                if( i_begin.count(idx) )
                                {
                                    int c_open_prev = c_open;
                                    c_open += (*(i_begin.find(idx))).second;
                                    if( !c_open_prev && c_open >= 1 )
                                    {
                                        output += Glib::Markup::escape_text( chunk ).raw();
                                        chunk.clear();
                                        output += "<span weight='bold'><i>";
                                    }
                                }
                                else
                                if( i_end.count(idx) )
                                {
                                    c_close += (*(i_end.find(idx))).second;
                                    if( c_close == c_open )
                                    {
                                        output += Glib::Markup::escape_text( chunk ).raw();
                                        chunk.clear();
                                        output += "</i></span>"; 
                                        c_close = 0;
                                        c_open  = 0;
                                    }
                                }

                                chunk += *i;
                            }

                            output += Glib::Markup::escape_text( chunk ).raw();

                            if( c_open )
                            {
                                output += "</i></span>";
                            }
                    }

                    return output;
            }
            else
            {
                return Glib::Markup::escape_text( text ) ;
            }
    }
  }
}

