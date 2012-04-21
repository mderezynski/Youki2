#include <config.h>
#include <glib/gstdio.h>
#include <glibmm/miscutils.h>

#include "mpx/mpx-paths.hh"

#undef PACKAGE
#define PACKAGE "youki"

namespace MPX
{
  Path app_paths[N_PATHS];

  namespace
  {
    void make_user_dir (std::string const& path)
    {
        g_mkdir (path.c_str(), 0700);
    }

  } // anonymous

  void create_user_dirs ()
  {
      app_paths[PATH_APP_DATA_DIR]   = Glib::build_filename (g_get_user_data_dir   (), PACKAGE);
      app_paths[PATH_APP_CACHE_DIR]  = Glib::build_filename (g_get_user_cache_dir  (), PACKAGE);
      app_paths[PATH_APP_CONFIG_DIR] = Glib::build_filename (g_get_user_config_dir (), PACKAGE);

      make_user_dir (Glib::build_filename( get_app_cache_dir(), "artists" ));
      make_user_dir (get_app_data_dir   ());
      make_user_dir (get_app_cache_dir  ());
      make_user_dir (get_app_config_dir ());
  }

} // MPX
