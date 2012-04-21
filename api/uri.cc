//  MPX::URI (C) 2006-2007 M. Derezynski 
//
//  Part of MPX (C) 2003-2007 MPX Development 
//
//  ----
//  Based on:
//
//  GNet - Networking library
//  Copyright (C) 2000-2003-2007  David Helder, David Bolcsfoldi, Eric Williams
//
//  libneon - HTTP networking library
//  (C) Joe Orton
//
//  ----
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
//  ----
//
//  The MPX project hereby grants permission for non-GPL compatible GStreamer
//  plugins to be used and distributed together with GStreamer and MPX. This
//  permission is above and beyond the permissions granted by the GPL license
//  MPX is covered by.

#include <glib.h>
#include <string>
#include <boost/algorithm/string.hpp>
#include <glibmm.h>
#include <glib/gprintf.h>
#include <iostream>
#include <cstdio>

#include "mpx/mpx-uri.hh"
#include "mpx/util-string.hh"

#define USERINFO_ESCAPE_MASK	0x01
#define PATH_ESCAPE_MASK	0x02
#define QUERY_ESCAPE_MASK	0x04
#define FRAGMENT_ESCAPE_MASK	0x08

namespace 
{
   char const* protocols[] =
   {
        "file",
        "cdda",
        "http", 
        "ftp",
        "mlq",
        "track",
        "mms",
        "mmsu",
        "mmst",
        "lastfm",
        "itpc"
  };

  char* field_unescape  (char * str);
  char* field_escape	  (char * str, guchar mask);

  guchar neednt_escape_table[] = 
  {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x0f, 0x00, 0x00, 0x0f, 0x00, 0x0f, 0x0f, 
    0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0e, 
    0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 
    0x0f, 0x0f, 0x0f, 0x0f, 0x00, 0x0f, 0x00, 0x0c, 
    0x0e, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 
    0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 
    0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 
    0x0f, 0x0f, 0x0f, 0x00, 0x0f, 0x00, 0x00, 0x0f, 
    0x00, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 
    0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 
    0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 
    0x0f, 0x0f, 0x0f, 0x00, 0x00, 0x00, 0x0f, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  };

#define WITHIN_ESCAPE_TABLE_SIZE(_char) ((size_t)(_char) < (sizeof (neednt_escape_table)))
#define ISSPACE(C) (((C) >= 9 && (C) <= 13) || (C) == ' ')

/* URI ABNF from RFC 3986: */

#define PS (0x0001) /* "+" */
#define PC (0x0002) /* "%" */
#define DS (0x0004) /* "-" */
#define DT (0x0008) /* "." */
#define US (0x0010) /* "_" */
#define TD (0x0020) /* "~" */
#define FS (0x0040) /* "/" */
#define CL (0x0080) /* ":" */
#define AT (0x0100) /* "@" */
#define QU (0x0200) /* "?" */

#define DG (0x0400) /* DIGIT */
#define AL (0x0800) /* ALPHA */

#define GD (0x1000) /* gen-delims    = "#" / "[" / "]" 
                     * ... except ":", "/", "@", and "?" */

#define SD (0x2000) /* sub-delims    = "!" / "$" / "&" / "'" / "(" / ")"
                     *               / "*" / "+" / "," / ";" / "=" 
                     * ... except "+" which is PS */

#define OT (0x4000) /* others */

#define URI_ALPHA (AL)
#define URI_DIGIT (DG)

/* unreserved = ALPHA / DIGIT / "-" / "." / "_" / "~" */
#define URI_UNRESERVED (AL | DG | DS | DT | US | TD)
/* scheme = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." ) */
#define URI_SCHEME (AL | DG | PS | DS | DT)
/* real sub-delims definition, including "+" */
#define URI_SUBDELIM (PS | SD)
/* real gen-delims definition, including ":", "/", "@" and "?" */
#define URI_GENDELIM (GD | CL | FS | AT | QU)
/* userinfo = *( unreserved / pct-encoded / sub-delims / ":" ) */
#define URI_USERINFO (URI_UNRESERVED | PC | URI_SUBDELIM | CL)
/* pchar = unreserved / pct-encoded / sub-delims / ":" / "@" */
#define URI_PCHAR (URI_UNRESERVED | PC | URI_SUBDELIM | CL | AT)
/* invented: segchar = pchar / "/" */
#define URI_SEGCHAR (URI_PCHAR | FS)
/* query = fragment = *( pchar / "/" / "?" ) */
#define URI_QUERY (URI_PCHAR | FS | QU)

/* any characters which should be path-escaped: */
#define URI_ESCAPE ((URI_GENDELIM & ~(FS)) | URI_SUBDELIM | OT | PC)

static const unsigned int uri_chars[256] = {
/* 0xXX    x0      x2      x4      x6      x8      xA      xC      xE     */
/*   0x */ OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT,
/*   1x */ OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT,
/*   2x */ OT, SD, OT, GD, SD, PC, SD, SD, SD, SD, SD, PS, SD, DS, DT, FS,
/*   3x */ DG, DG, DG, DG, DG, DG, DG, DG, DG, DG, CL, SD, OT, SD, OT, QU,
/*   4x */ AT, AL, AL, AL, AL, AL, AL, AL, AL, AL, AL, AL, AL, AL, AL, AL,
/*   5x */ AL, AL, AL, AL, AL, AL, AL, AL, AL, AL, AL, GD, OT, GD, OT, US,
/*   6x */ OT, AL, AL, AL, AL, AL, AL, AL, AL, AL, AL, AL, AL, AL, AL, AL,
/*   7x */ AL, AL, AL, AL, AL, AL, AL, AL, AL, AL, AL, OT, OT, OT, TD, OT,
/*   8x */ OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, 
/*   9x */ OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, 
/*   Ax */ OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, 
/*   Bx */ OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, 
/*   Cx */ OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, 
/*   Dx */ OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, 
/*   Ex */ OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, 
/*   Fx */ OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT, OT
};

#define uri_lookup(ch) (uri_chars[(unsigned char)ch])

char *path_unescape(const char *uri) 
{
    const char *pnt;
    char *ret, *retpos, buf[6] = { "0x00\0" };
    retpos = ret = (char*)(g_malloc(strlen(uri) + 1));
    for (pnt = uri; *pnt != '\0'; pnt++) {
	if (*pnt == '%') {
	    if (!isxdigit((unsigned char) pnt[1]) || 
		!isxdigit((unsigned char) pnt[2])) {
		/* Invalid URI */
                g_free(ret);
		return NULL;
	    }
	    buf[2] = *++pnt; buf[3] = *++pnt; /* bit faster than memcpy */
	    *retpos++ = (char)strtol(buf, NULL, 16);
	} else {
	    *retpos++ = *pnt;
	}
    }
    *retpos = '\0';
    return ret;
}

/* CH must be an unsigned char; evaluates to 1 if CH should be
 * percent-encoded. */
#define path_escape_ch(ch) (uri_lookup(ch) & URI_ESCAPE)

char *path_escape(const char *path) 
{
    const unsigned char *pnt;
    char *ret, *p;
    size_t count = 0;

    for (pnt = (const unsigned char *)path; *pnt != '\0'; pnt++) {
        count += path_escape_ch(*pnt);
    }

    if (count == 0) {
    	return g_strdup (path);
    }

    p = ret = (char*)(g_malloc(strlen(path) + 2 * count + 1));
    for (pnt = (const unsigned char *)path; *pnt != '\0'; pnt++)
    {
    	if (path_escape_ch(*pnt))
      {
	    /* Escape it - %<hex><hex> */
	    g_sprintf(p, "%%%02x", (unsigned char) *pnt);
	    p += 3;
	    } else {
	      *p++ = *pnt;
	    }
    }
    *p = '\0';
    return ret;
  }

  std::string destroy (char* s)
  {
    if (!s) return std::string();
    std::string v (s);
    g_free (s);
    return v;
  }

  char*
  field_escape (char * str, guchar mask)
  {
      int len = 0;
      unsigned int i = 0;
      bool must_escape = false;
      char* dst;
      gint j;

      if (!str)
        return 0;

      // Roughly calculate buffer size 
      len = 0;
      for (i = 0; str[i]; ++i)
      {
        if (WITHIN_ESCAPE_TABLE_SIZE (str[i]) && neednt_escape_table[(guint) str[i]] & mask)
        {
          len++;
        }
        else
        {
          len += 3;
          must_escape = true;
        }
      }

      // Don't escape if unnecessary
      if (must_escape == false)
          return str; 

      // Allocate buffer
      dst = static_cast<char*> (g_malloc (len + 1));

      // Copy 
      for (i = j = 0; str[i]; i++, j++)
      {
        // Unescaped character
        if (WITHIN_ESCAPE_TABLE_SIZE (str[i]) && neednt_escape_table[(guint) str[i]] & mask)
          {
            dst[j] = str[i];
          }

        // Escaped character
        else
          {
            dst[j] = '%';

              if (((str[i] & 0xf0) >> 4) < 10)
                  dst[j+1] = ((str[i] & 0xf0) >> 4) + '0';
              else
                  dst[j+1] = ((str[i] & 0xf0) >> 4) + 'a' - 10;

              if ((str[i] & 0x0f) < 10)
                  dst[j+2] = (str[i] & 0x0f) + '0';
              else
                  dst[j+2] = (str[i] & 0x0f) + 'a' - 10;

              j += 2;  // and j is incremented in loop too
          }
      }

      dst[j] = '\0';
      g_free (str);
      return dst;
    }

    char *
    field_unescape (char * _s)
    {
      char* s = strdup (_s);
      char* src;
      char* dst;

      for (src = dst = s; *src; ++src, ++dst)
        {
          if (src[0] == '%' && src[1] != '\0' && src[2] != '\0')
            {
              int high, low;

              if ('a' <= src[1] && src[1] <= 'f')
                  high = src[1] - 'a' + 10;
              else if ('A' <= src[1] && src[1] <= 'F')
                  high = src[1] - 'A' + 10;
              else if ('0' <= src[1] && src[1] <= '9')
                  high = src[1] - '0';
              else  // malformed
                  goto regular_copy;

              if ('a' <= src[2] && src[2] <= 'f')
                  low = src[2] - 'a' + 10;
              else if ('A' <= src[2] && src[2] <= 'F')
                  low = src[2] - 'A' + 10;
              else if ('0' <= src[2] && src[2] <= '9')
                  low = src[2] - '0';
              else  // malformed
                  goto regular_copy;

              *dst = (char)((high << 4) + low);
              src += 2;
            }
          else
            {
              regular_copy:
                *dst = *src;
            }
        }
      *dst = '\0';
      free (_s);
      return s;
    }
}

namespace MPX
{
    bool
    URI::fragmentize (Glib::ustring const& _uri)
    {
      const char  * p;
      const char  * temp;
      char	      * uri;

      uri = strdup (_uri.c_str()); 

      //////// Skip initial whitespace
      p = uri;
      while (*p && ISSPACE ((int)*p)) ++p;
      if (!*p)	// Error if it's just a string of space
        return false;

      //////// Scheme
      temp = p;

      while (*p && *p != ':' && *p != '/' && *p != '?' && *p != '#') ++p;

      if (*p == ':')
      {
        scheme = destroy (g_strndup (temp, p - temp));
        ++p;
      }
      else
      /////// This char is NUL, /, ?, or # */
        p = temp;

      /////// Authority
      if (*p == '/' && p[1] == '/')
      {
        p += 2;

    		/////// Userinfo 
		    temp = p;

    		while (*p && *p != '@' && *p != '/' ) /* Look for @ or / */
		      ++p;

        if (*p == '@') /* Found userinfo */
		    {
          userinfo = destroy (g_strndup (temp, p - temp));
          ++p;
		    }
        else
		      p = temp;

        /////// Hostname
        // Check for no hostname at all (e.g. file://)  -- mderezynski
        if (*p == '/')
            goto path;

		    // Check for IPv6 canonical hostname in brackets
		    if (*p == '[')
		      {
            p++;  /* Skip [ */
			      temp = p;
			      while (*p && *p != ']') ++p;
			      if ((p - temp) == 0)
			        {
			          goto error;
			        }
			      hostname = destroy (g_strndup (temp, p - temp));
			      if (*p)
			        p++;	// Skip ] (if there)
		      }
		    else
		      {
			      temp = p;
			      while (*p && *p != '/' && *p != '?' && *p != '#' && *p != ':') ++p;
			      if ((p - temp) == 0) 
			        {
			          goto error;
			        }
			      hostname = destroy (g_strndup (temp, p - temp));
		      }

        /////// Port
		    if (*p == ':')
		      {
			      for (++p; isdigit ((int)*p); ++p)
			      port = port * 10 + (*p - '0');
		      }

	      }

        path:

	      // Path (we are liberal and won't check if it starts with /)
	      temp = p;
  
	      while (*p && *p != '?')
	        ++p;
	      if (p != temp)
	        path = destroy (g_strndup (temp, p - temp));

	      // Query
	      if (*p == '?')
	        {
		        temp = p + 1;
        		while (*p && *p != '#')
		         ++p;
        		query = destroy (g_strndup (temp, p - temp));
	        }

        // Fragment
      	if (*p == '#')
	        {
        		++p;
        		fragment = destroy (g_strdup (p));
          }

      	g_free (uri);
      	return true;

        error:
      	  return false;
    }

    URI::URI ()
    : port (0)
    {}

    URI::URI (Glib::ustring const& u, bool esc)
    : port (0)
    {
      if (u.empty())
        throw ParseError(); 

      if (!fragmentize (u))
        throw ParseError(); 

      if (esc)
        escape ();

      if ((port == 0) && ((get_protocol() == PROTOCOL_HTTP) || (get_protocol() == PROTOCOL_ITPC)))
        port = 80;
    }

    void
    URI::escape ()
    {
      userinfo = destroy (field_escape (strdup (userinfo.c_str()), USERINFO_ESCAPE_MASK));
      path = destroy (field_escape (strdup (path.c_str()), PATH_ESCAPE_MASK));
      query = destroy (field_escape (strdup (query.c_str()), QUERY_ESCAPE_MASK));
      fragment = destroy (field_escape (strdup (fragment.c_str()), FRAGMENT_ESCAPE_MASK));
    }
  
    void
    URI::parse_query (Query& q)
    {
      using namespace boost;
      using namespace std;

      StrV fragments;
      split (fragments, query, is_any_of("&"));
    
      for (StrV::const_iterator frag = fragments.begin (); frag != fragments.end(); ++frag)
      {
        StrV sub;
        split (sub, *frag, is_any_of("="));
        QElement element (std::make_pair (sub[0], sub[1]));
        q.insert (std::make_pair (sub[0], element)); 
      }
    }
  
    void
    URI::unescape ()
    {
      if (!userinfo.empty())
        userinfo = destroy (field_unescape (strdup (userinfo.c_str())));

      if (!path.empty())
        path = destroy (field_unescape (strdup (path.c_str())));

      if (!query.empty())
        query = destroy (field_unescape (strdup (query.c_str())));

      if (!userinfo.empty())
        fragment = destroy (field_unescape (strdup (fragment.c_str())));
    }

    std::string 
    URI::get_protocol_scheme (MPX::URI::Protocol p)
    {
      return protocols[p];
    }

    void
    URI::set_protocol (MPX::URI::Protocol p)
    {
      switch (p)
      {
        case PROTOCOL_FILE:
          scheme = "file";
          break;
        case PROTOCOL_CDDA:
          scheme = "cdda";
          break;
        case PROTOCOL_HTTP:
          scheme = "http";
          break;
        case PROTOCOL_FTP:
          scheme = "ftp";
          break;
        case PROTOCOL_QUERY:
          scheme = "query";
          break;
        case PROTOCOL_TRACK:
          scheme = "track";
          break;
        case PROTOCOL_MMS:
          scheme = "mms";
          break;
        case PROTOCOL_MMSU:
          scheme = "mmsu";
          break;
        case PROTOCOL_MMST:
          scheme = "mmst";
          break;
        case PROTOCOL_LASTFM:
          scheme = "lastfm";
          break;
        case PROTOCOL_ITPC:
          scheme = "itpc";
          break;

        default:
          g_warning ("%s: Request setting of protocol ID '%d' which is not supported/out of range",
            G_STRLOC, int (p));
          break;
      }
    }

    MPX::URI::Protocol
    URI::get_protocol ()
    {
      MPX::URI::Protocol protocol = URI::PROTOCOL_UNKNOWN;

      for (unsigned int n = 0; n < G_N_ELEMENTS (protocols); ++n)
      {
        if (scheme == protocols[n])
        {
          protocol = MPX::URI::Protocol(n);
          break;
        }
      }
      return protocol;
    }

    std::string
    URI::escape_string (std::string const& in)
    {
      Glib::ScopedPtr<char> field (path_escape (in.c_str()));
      return field.get();
    }
  
    std::string
    URI::unescape_string (std::string const& in)
    {
      Glib::ScopedPtr<char> field (path_unescape (in.c_str()));
      return field.get();
    }

    std::string
    URI::fullpath () const
    {
      std::string p (path);

      if (query.size())
      {
        p.append ("?");
        p.append (query);
      }

      if (fragment.size())
      {
        p.append ("#");
        p.append (fragment);
      }

      return p;
    }

    URI::operator Glib::ustring() const
    {
      GString* buffer = NULL;
      buffer = g_string_sized_new (16);

      if (!scheme.empty())
      {
        g_string_sprintfa (buffer, "%s:", scheme.c_str());
      }

      if ((scheme == protocols[PROTOCOL_FILE])   || 
          (scheme == protocols[PROTOCOL_CDDA])   || 
          (scheme == protocols[PROTOCOL_HTTP])   || 
          (scheme == protocols[PROTOCOL_ITPC])   || 
          (scheme == protocols[PROTOCOL_LASTFM]) || 
          (scheme == protocols[PROTOCOL_QUERY]))
      {
          g_string_append (buffer, "//");
      }

      if (!userinfo.empty())
      {
        buffer = g_string_append   (buffer,  userinfo.c_str());
        buffer = g_string_append_c (buffer, '@');
      }

      // Add brackets around the hostname if it's IPv6
      if (!hostname.empty())
      {
        if (strchr (hostname.c_str(), ':') == NULL) 
          buffer = g_string_append (buffer, hostname.c_str()); 
        else
          g_string_sprintfa (buffer, "[%s]", hostname.c_str());
      }

#if 0
      if (port)
      {
        g_string_sprintfa (buffer, ":%d", port);
      }
#endif

      if (!path.empty())
      {
        if (*(path.c_str()) == '/' || !(!userinfo.empty() || !hostname.empty() || port))
          g_string_append (buffer, path.c_str());
        else
          g_string_sprintfa (buffer, "/%s", path.c_str());
      }

      if (!query.empty())
        g_string_sprintfa (buffer, "?%s", query.c_str());

      if (!fragment.empty())
        g_string_sprintfa (buffer, "#%s", fragment.c_str());

      // Free only GString not data contained, return the data instead
      complete = buffer->str;
      g_string_free (buffer, TRUE); 
      return complete;
    }
}
