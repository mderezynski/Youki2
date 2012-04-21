//  BMP
//  Copyright (C) 2005 BMP development.
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License Version 2 as 
//  published by the Free Software Foundation.
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
//  The BMPx project hereby grants permission for non-GPL compatible GStreamer
//  plugins to be used and distributed together with GStreamer and BMPx. This
//  permission is above and beyond the permissions granted by the GPL license
//  BMPx is covered by.

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include <cstdlib>
#include <cstring>

#include <boost/format.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>

#include <glibmm.h>
#include <glibmm/i18n.h>

#include "src/util-string.hh"
#include "src/util-file.hh"
#include "src/mpx-uri.hh"
#include "src/vfs.hh"

using namespace Glib;
using namespace Bmp;
using namespace std;

namespace
{
  typedef map < string, string > StringMap;

  void
  parse_to_map (StringMap& map, string const& buffer)
  {
    using boost::algorithm::split;
    using boost::algorithm::split_regex;
    using boost::algorithm::is_any_of;

    vector<string> lines;
    split_regex (lines, buffer, boost::regex ("\\\r?\\\n"));

    for (unsigned int n = 0; n < lines.size(); ++n)
    {
      char **line = g_strsplit (lines[n].c_str(), "=", 2);
      if (line[0] && line[1] && strlen(line[0]) && strlen(line[1]))
      {
        ustring key (line[0]);
        map[std::string (key.lowercase())] = line[1];
      }
      g_strfreev (line);
    }
  }
}

namespace Bmp
{
  namespace VFS
  {
      class PluginContainerPLS
        : public Bmp::VFS::PluginContainerBase
      {
        public:

          virtual bool
          can_process (ustring const& uri)
          {
            URI u (uri);
            return Util::str_has_suffix_nocase (u.path, ".pls");
          }

          virtual void 
          handle_read	(Handle & handle,
                       VUri & list) throw (ProcessingError)
          {

            if (!handle.get_buffer()) 
                throw ProcessingError ("Empty Buffer"); 

            string buffer = (const char*)(handle.get_buffer());

            StringMap map;
            parse_to_map (map, buffer);

            if (map.empty()) 
              throw ProcessingError (_("Malformed PLS: No key/value pairs (Invalid Playlist?)")); 

            if (map.find("numberofentries") == map.end())
              throw ProcessingError (_("Malformed PLS: No 'numberofentries' key")); 

            unsigned int n = strtol (map.find("numberofentries")->second.c_str(), NULL, 10);
            static boost::format File ("file%d");
            for (unsigned int a = 1; a <= n ; ++a)
            {
              StringMap::iterator const& i = map.find ((File % a).str());
              if (i != map.end())
              {
                list.push_back (i->second);
              }
            }

            if (list.empty()) 
              throw ProcessingError (_("Malformed PLS: No 'File' keys found")); 

            if (list.size() != n)
              throw ProcessingError (_("Malformed PLS: Number of Files doesn't match specified number"));
          }

          virtual bool
          can_write ()
          {
            return true;
          }

          virtual void
          handle_write (Handle & handle, TrackV const& list) throw (ProcessingError)
          {
            stringstream out;
            int n = 1;

            out << "[playlist]\n";
            out << "numberofentries=" << list.size () << "\n";

            for (TrackV::const_iterator i = list.begin (); i != list.end (); ++i)
            {
              Track const& track (i.operator*());

              if (!track.location)
                continue;

              out << "File"   
                  << n 
                  << "=" 
                  << track.location.get() 
                  << endl; 

              out << "Title"  
                  << n 
                  << "=" 
                  << (track.title ? track.title.get() : string())
                  << endl;

              out << "Length" 
                  << n 
                  << "=" 
                  << (track.duration ? (boost::format ("%llu") % track.duration.get()).str() : "-1")
                  << endl;
              ++n; 
            }

            out << "Version=2" << endl;
            handle.set_buffer((const unsigned char *)strdup(out.str().c_str()), strlen(out.str().c_str())+1);
          }

          virtual Bmp::VFS::ExportData const&
          get_export_data ()
          {
            static Bmp::VFS::ExportData export_data ("PLS Playlist", "pls"); 
            return export_data;
          }
      };
  }
}
  
extern "C" Bmp::VFS::PluginContainerBase* plugin_create (void)
{
  return new Bmp::VFS::PluginContainerPLS;
}

extern "C" void plugin_delete (Bmp::VFS::PluginContainerPLS* p)
{
  delete p;
}
