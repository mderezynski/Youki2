//  MPX
//  Copyright (C) 2005-2007 MPX Project
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
//  The MPXx project hereby grants permission for non-GPL compatible GStreamer
//  plugins to be used and distributed together with GStreamer and MPXx. This
//  permission is above and beyond the permissions granted by the GPL license
//  MPXx is covered by.

#ifndef MPX_PATHS_HH
#define MPX_PATHS_HH

#include <string>

namespace MPX
{
  typedef std::string Path;

  enum
  {
      PATH_APP_DATA_DIR,
      PATH_APP_CACHE_DIR,
      PATH_APP_CONFIG_DIR,

      N_PATHS
  };

  extern Path app_paths[N_PATHS];

  void create_user_dirs ();

  inline Path
  get_app_data_dir ()
  {
      return app_paths[PATH_APP_DATA_DIR];
  }

  inline Path
  get_app_cache_dir ()
  {
      return app_paths[PATH_APP_CACHE_DIR];
  }

  inline Path
  get_app_config_dir ()
  {
      return app_paths[PATH_APP_CONFIG_DIR];
  }

} // MPX

#endif // MPX_PATHS_HH
