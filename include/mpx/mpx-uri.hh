#ifndef MPX_URI_HH
#define MPX_URI_HH

#include <map>
#include <string>
#include <glibmm/ustring.h>

namespace MPX
{
    class URI
    {
      public:

#include "mpx/exception.hh"
        EXCEPTION(ParseError)

        typedef std::pair <Glib::ustring, Glib::ustring> QElement;
        typedef std::map  <Glib::ustring, QElement>      Query;
        typedef std::pair <Glib::ustring, QElement>      QueryPair;

        enum Protocol
        {
          PROTOCOL_UNKNOWN = -1,
          PROTOCOL_FILE,
          PROTOCOL_CDDA,
          PROTOCOL_HTTP,
          PROTOCOL_FTP,
          PROTOCOL_QUERY,
          PROTOCOL_TRACK,
          PROTOCOL_MMS, 
          PROTOCOL_MMSU,
          PROTOCOL_MMST,
          PROTOCOL_LASTFM,
          PROTOCOL_ITPC
        };

        URI ();
        URI (Glib::ustring const& uri, bool escape = false);

        Protocol            get_protocol ();
        void                set_protocol (Protocol p);
        static std::string  get_protocol_scheme (Protocol p);
        static std::string  escape_string (std::string const& string);
        static std::string  unescape_string (std::string const& string);
        void                escape ();
        void                unescape ();
        std::string         fullpath () const;
        void                parse_query (Query & q);

        operator Glib::ustring () const;

        Glib::ustring scheme;
        Glib::ustring userinfo;
        Glib::ustring hostname;
        Glib::ustring path;
        Glib::ustring query;
        Glib::ustring fragment;
        guint16	      port;

      private:

        bool fragmentize (Glib::ustring const& uri);

        mutable Glib::ustring complete;

    };
}

#endif //!MPX_URI_HH
