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
#include "config.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <list>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <grp.h>
#include <unistd.h>
#include <boost/format.hpp>

#include <glib/gstdio.h>
#include <glibmm.h>
#include <giomm.h>

#include "mpx/util-file.hh"
#include "mpx/util-string.hh"

#include "mpx/mpx-uri.hh"

using namespace Glib;

namespace MPX
{
  namespace Util
  {
    namespace
    {
        typedef std::list<std::string> DirEntries;

        bool
        always_true(
              const std::string& G_GNUC_UNUSED 
        )
        {
            return true;
        }

        bool
        is_audio_file(
              const std::string& uri 
        )
        {
          static char const* extensions[] =
          { ".mp3",
            ".aac",
            ".mp4",
            ".m4a",
            ".m4b",
            ".m4p",
            ".m4v",
            ".mp4v",
            ".flac",
            ".wma",
            ".asf",
            ".sid",
            ".psid",
            ".mod",
            ".oct",
            ".xm",
            ".669",
            ".sht",
            ".mpc",
            ".ogg",
            ".wav",
            0 };

            try{
                return Util::str_has_suffixes_nocase( uri, extensions ) ;
            }
            catch (...)
            {
                return false ;
            }
        }

    } // <anonymous> namespace

    std::string
    create_glob (const std::string &suffix)
    {
        static boost::format fmt ("[%c%c]");

        std::string result;

        for (std::string::const_iterator i = suffix.begin (); i != suffix.end (); ++i)
            result += (fmt % g_ascii_tolower (*i) % g_ascii_toupper (*i)).str ();

        return result;
    }

    void
    collect_dirs (std::string  const& uri,
                  FileCallback const& callback) 
    {
      callback( uri );

      Glib::RefPtr<Gio::File> file = Gio::File::create_for_uri(uri);
      Glib::RefPtr<Gio::FileEnumerator> enm = file->enumerate_children(G_FILE_ATTRIBUTE_STANDARD_NAME "," G_FILE_ATTRIBUTE_STANDARD_TYPE, Gio::FILE_QUERY_INFO_NONE);

      gboolean iterate = TRUE;
      while(iterate && enm)
      {
        Glib::RefPtr<Gio::FileInfo> f = enm->next_file();
        if(f)
        {
            std::string full_path = uri + G_DIR_SEPARATOR_S + URI::escape_string(f->get_name());
            Gio::FileType t = f->get_file_type();

            if (t == Gio::FILE_TYPE_DIRECTORY)
            {
                collect_dirs (full_path, callback);
            }
        }
        else
            iterate = FALSE;
      }
    }

    void
    collect_paths_recursive (std::string const& uri,
                   FileList&          collection,
                   FilePred           pred,
                   bool               clear)
    {
      if (clear)
        collection.clear ();

      Glib::RefPtr<Gio::File> file = Gio::File::create_for_uri(uri);
      Glib::RefPtr<Gio::FileEnumerator> enm = file->enumerate_children(G_FILE_ATTRIBUTE_STANDARD_NAME "," G_FILE_ATTRIBUTE_STANDARD_TYPE, Gio::FILE_QUERY_INFO_NONE);

      gboolean iterate = TRUE;
      while(iterate && enm)
      {
        Glib::RefPtr<Gio::FileInfo> f = enm->next_file();
        if(f)
        {
            std::string full_path = uri + G_DIR_SEPARATOR_S + URI::escape_string(f->get_name());
            Gio::FileType t = f->get_file_type();
            if (t == Gio::FILE_TYPE_REGULAR)
            {
                if (pred (full_path))
                {
                  collection.push_back (full_path);
                }
            }
            else if (t == Gio::FILE_TYPE_DIRECTORY)
            {
                // pred is getting repeatedly copied for no good reason!
                collect_paths_recursive (full_path, collection, pred, false);
            }
        }
        else
            iterate = FALSE;
      }
    }


    void
    collect_paths (std::string const& uri,
                   FileList&          collection,
                   FilePred           pred,
                   bool               clear)
    {
      if (clear)
        collection.clear ();

      Glib::RefPtr<Gio::File> file = Gio::File::create_for_uri(uri);
      Glib::RefPtr<Gio::FileEnumerator> enm = file->enumerate_children(G_FILE_ATTRIBUTE_STANDARD_NAME "," G_FILE_ATTRIBUTE_STANDARD_TYPE, Gio::FILE_QUERY_INFO_NONE);

      gboolean iterate = TRUE;
      while(iterate && enm)
      {
        Glib::RefPtr<Gio::FileInfo> f = enm->next_file();
        if(f)
        {
            std::string full_path = uri + G_DIR_SEPARATOR_S + URI::escape_string(f->get_name());
            Gio::FileType t = f->get_file_type();
            if (t == Gio::FILE_TYPE_REGULAR)
            {
                if (pred (full_path))
                {
                  collection.push_back (full_path);
                }
            }
            /*
            else if (t == Gio::FILE_TYPE_DIRECTORY)
            {
                // pred is getting repeatedly copied for no good reason!
                collect_paths (full_path, collection, pred, false);
            }
            */
        }
        else
            iterate = FALSE;
      }
    }

    void
    collect_paths (std::string const& dir_path,
                   FileList&          collection,
                   bool               clear)
    {
      collect_paths (dir_path, collection, sigc::ptr_fun (always_true));
    }

    void
    collect_audio_paths (std::string const& dir_path,
                         FileList&          collection,
                         bool               clear)
    {
      collect_paths (dir_path, collection, sigc::ptr_fun (&is_audio_file), clear);
    }

    void
    collect_audio_paths_recursive (std::string const& dir_path,
                                   FileList&          collection,
                                   bool               clear)
    {
      collect_paths_recursive (dir_path, collection, sigc::ptr_fun (&is_audio_file), clear);
    }

    void
    dir_for_each_entry (const std::string &path,
                        FilePred           pred)
    {
      Glib::Dir dir (path);
      DirEntries entries (dir.begin(), dir.end ());

      for (DirEntries::const_iterator iter = entries.begin (), end = entries.end ();
           iter != end;
           ++iter)
        {
          std::string full_path = Glib::build_filename (path, *iter);

          if (pred (full_path))
            break;
        }
    }

    void
    files_writable (FileList const& list,
                    FileList      & non_writable)
    {
      for (FileList::const_iterator i = list.begin () ; i != list.end () ; ++i)
      {
        if (!access ((*i).c_str(), W_OK))
          continue;
        else
          non_writable.push_back (*i);
      }
    }

    // compares files wrg to existence and filesize
    bool
    compare_files (const std::string &file1,
                   const std::string &file2)
    {
        if (!Glib::file_test (file1, Glib::FILE_TEST_EXISTS))
            return false;

        if (!Glib::file_test (file2, Glib::FILE_TEST_EXISTS))
            return false;

        struct stat stat_a;
        struct stat stat_b;

        stat (file1.c_str (), &stat_a);
        stat (file2.c_str (), &stat_b);

        return (stat_a.st_size == stat_b.st_size);
    }

    void
    copy_file (const std::string &s,
               const std::string &d)
    {
        using namespace std;

        if (s == d) return;

        std::ifstream i;
        std::ofstream o;

        i.exceptions (ifstream::badbit | ifstream::failbit);
        o.exceptions (ofstream::badbit | ofstream::failbit);

        i.open (s.c_str());
        o.open (d.c_str());

        if (i.is_open() && o.is_open())
          o << i.rdbuf();

        if (o.is_open())
          o.close ();

        if (i.is_open())
          i.close ();
    }

    guint
    get_file_ctime(
        const std::string& uri
    )
    {
        try{
                Glib::RefPtr<Gio::File> file = Gio::File::create_for_uri(uri); 
                Glib::RefPtr<Gio::FileInfo> info = file->query_info(G_FILE_ATTRIBUTE_TIME_CHANGED, Gio::FILE_QUERY_INFO_NONE);
                return info->get_attribute_uint64(G_FILE_ATTRIBUTE_TIME_CHANGED) ;
        } catch( Glib::Error )
        {
                return 0;
        }
    }

    guint
    get_file_mtime(
        const std::string& uri
    )
    {
        try{
                Glib::RefPtr<Gio::File> file = Gio::File::create_for_uri(uri); 
                Glib::RefPtr<Gio::FileInfo> info = file->query_info(G_FILE_ATTRIBUTE_TIME_MODIFIED, Gio::FILE_QUERY_INFO_NONE);
                return info->get_attribute_uint64(G_FILE_ATTRIBUTE_TIME_MODIFIED) ;
        } catch( Glib::Error )
        {
                return 0;
        }
    }

    std::string
    normalize_path(
        const std::string& path
    )
    {
        if( path.size() > 1 && path[path.size()-1] == '/' )
            return path.substr( 0, path.size() - 1 );
        else
            return path;
    }
  } // namespace Util
} // namespace MPX
