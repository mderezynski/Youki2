// (c) 2007 M. Derezynski

#include <glib/gmacros.h>
#include <glib/gmessages.h>
#include <string>
#include "mpx/playlistparser-pls.hh"
#include "xmlcpp/xspf.hxx"
#include "mpx/xml/xmltoc++.hh"

#include <boost/format.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/lexical_cast.hpp>

#include "mpx/util-string.hh"
#include "mpx/mpx-uri.hh"

using namespace Glib;
using namespace MPX;
using namespace std;

namespace
{
  typedef map < string, string > StringMap;

    void
    parse_to_map (StringMap& map, string const& data)
    {
        using boost::algorithm::split;
        using boost::algorithm::split_regex;
        using boost::algorithm::is_any_of;

        char **lines = g_strsplit(data.c_str(), "\n", -1);
        for (int n = 0; lines[n] != NULL; ++n)
        {
            char **line = g_strsplit (lines[n], "=", 2);
            if (line[0] && line[1] && strlen(line[0]) && strlen(line[1]))
            {
                ustring key (line[0]);
                map[std::string (key.lowercase())] = line[1];
            }
            g_strfreev (line);
        }
        g_strfreev (lines);
    } 
}


namespace MPX
{
namespace PlaylistParser
{
	PLS::PLS ()
	{
	}

	PLS::~PLS ()
	{
	}

	bool
	PLS::read (std::string const& data, Track_v & v, bool download)
	{
        try{
            StringMap map;

            if(download)
            {
                parse_to_map (map, file_get_contents(filename_from_uri(data)));
            }
            else
            {
                parse_to_map (map, data);
            } 
                
            int n = boost::lexical_cast<int>(map.find("numberofentries")->second);

            for (int i = 1; i <= n ; ++i)
            {
                MPX::Track t;
                t[ATTRIBUTE_LOCATION] = map[(boost::format("file%d") % i).str()]; 
                t[ATTRIBUTE_TITLE] = map[(boost::format("title%d") % i).str()]; 
                t[ATTRIBUTE_TIME] = gint64(boost::lexical_cast<int>(  map[(boost::format("length%d") % i).str()]));
                v.push_back(t);
            }
		} catch(std::exception & cxe) {
            g_message("%s: error: %s", G_STRLOC, cxe.what());
			return false;
		}
		return true;
	}

	bool
	PLS::write (std::string const& uri, Track_v const& list)
	{
		return false;
	}
}
}
