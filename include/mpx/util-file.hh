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
#ifndef MPX_FILE_UTILS_HH
#define MPX_FILE_UTILS_HH

#include "config.h"

#include <vector>
#include <string>
#include <sigc++/slot.h>

namespace MPX
{
  namespace Util
  {
    typedef std::vector<std::string>      FileList ;
    typedef sigc::slot<bool, std::string> FilePred ;
    typedef sigc::slot<void, std::string> FileCallback ;

    void
    dir_for_each_entry (const std::string &path,
                        FilePred           slot) ;

    void
    collect_dirs (std::string  const& dir_path,
                  FileCallback const& callback) ;

    void
    collect_paths (std::string const& dir_path,
                   FileList&          collection,
                   bool               clear = true) ;

    void
    collect_paths (std::string const& dir_path,
                   FileList&          collection,
                   FilePred           pred,
                   bool               clear = true) ;

    void
    collect_paths_recursive (std::string const& dir_path,
                   FileList&          collection,
                   FilePred           pred,
                   bool               clear = true) ;

    void
    collect_audio_paths (std::string const &dir_path,
                         FileList          &collection,
                         bool               clear = true) ;

    void
    collect_audio_paths_recursive (std::string const &dir_path,
                         FileList          &collection,
                         bool               clear = true) ;

    std::string
    create_glob (const std::string &suffix) ;

    bool
    compare_files (const std::string &file1,
                   const std::string &file2) ;

    void
    copy_file (const std::string &source,
               const std::string &dest) ;

    void
    files_writable (const FileList &list,
                    FileList       &non_writable) ;

    std::string
    normalize_path (const std::string&) ;


    guint
    get_file_ctime(
        const std::string& uri
    ) ;

    guint
    get_file_mtime(
        const std::string& uri
    ) ;
 
  } // namespace Util
} // namespace MPX

#endif // !MPX_FILE_UTILS_HH
