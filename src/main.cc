//  main.cc
//
//  Authors:
//      Milosz Derezynski <milosz@backtrace.info>
//
//  (C) 2008 MPX Project
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


#include <glib/gi18n.h>
#include <glibmm/miscutils.h>
#include <giomm.h>
#include <gst/gst.h>
#include <gtkmm/main.h>
#include <cstdlib>
#include <string>
#include <locale.h>

#include "mpx/mpx-covers.hh"
#include "mpx/mpx-main.hh"
#include "mpx/mpx-network.hh"
#include "mpx/mpx-paths.hh"
#include "mpx/mpx-plugin.hh"
#include "mpx/mpx-python.hh"
#include "mpx/mpx-services.hh"
#include "mpx/mpx-signals.hh"
#include "mpx/mpx-stock.hh"
#include "mpx/mpx-types.hh"

#include "mpx/util-file.hh"

#ifdef HAVE_HAL
#include "mpx/mpx-hal.hh"
#endif // HAVE_HAL

#include "library.hh"
#include "play.hh"
#include "plugin-manager-gui.hh"
#include "preferences.hh"
#include "splash-screen.hh"

#include "youki-controller.hh"
#include "youki-theme-engine.hh"

#undef PACKAGE
#define PACKAGE "youki"

using namespace MPX;
using namespace Glib;

namespace MPX
{
  Mcs::Config      * mcs      = 0;
  Mcs::Bind        * mcs_bind = 0;
  Service::Manager * services = 0;

  namespace
  {
    Gtk::Main * gtk = 0;

    const int signal_check_interval = 1000;

    int terminate = 0;

    void
    empty_handler (int signal_number)
    {
        // empty
    }

    void
    sigsegv_handler (int signal_number)
    {
        delete mcs; // try to save config

#ifdef HANDLE_SIGSEGV
        std::exit (EXIT_FAILURE);
#else
        std::abort ();
#endif
    }

    void
    sigterm_handler (int signal_number)
    {
        g_atomic_int_set( &terminate, 1 );
    }

    bool
    process_signals ()
    {
        if( g_atomic_int_get(&terminate) )
        {
            g_message (_("Got SIGTERM: Exiting (It's all right!)"));

            bool gtk_running = Gtk::Main::level();

            if (gtk_running)
                Gtk::Main::quit ();
            else
                std::exit (EXIT_SUCCESS);

            return false;
        }

        return true;
    }

    void
    signal_handlers_install ()
    {
        install_handler (SIGPIPE, empty_handler);
        install_handler (SIGSEGV, sigsegv_handler);
        install_handler (SIGINT, sigterm_handler);
        install_handler (SIGTERM, sigterm_handler);

        Glib::signal_timeout ().connect( sigc::ptr_fun (&process_signals), signal_check_interval );
    }

    void
    setup_mcs ()
    {
        mcs = new Mcs::Config (Glib::build_filename (get_app_config_dir (), "config.xml"), "mpx", 0.01);
        mcs_bind = new Mcs::Bind(mcs);
        mcs->load (Mcs::Config::VERSION_IGNORE);

        mcs->domain_register ("main-window");
        mcs->key_register ("main-window", "width", 0); //FIXME:
        mcs->key_register ("main-window", "height", 0); //FIXME:
        mcs->key_register ("main-window", "pos_x", 0);
        mcs->key_register ("main-window", "pos_y", 0);
        mcs->key_register ("main-window", "paned1", 0);
        mcs->key_register ("main-window", "paned2", 0);

        mcs->domain_register ("ui");
        mcs->key_register ("ui", "show-statusbar", true);

        mcs->domain_register ("mpx");
        mcs->key_register ("mpx", "display-notifications", true);
        mcs->key_register ("mpx", "follow-current-track", false);
        mcs->key_register ("mpx", "window-x", 20);
        mcs->key_register ("mpx", "window-y", 20);
        mcs->key_register ("mpx", "window-w", 400);
        mcs->key_register ("mpx", "window-h", 500);
        mcs->key_register ("mpx", "window-prefs-w", 700);
        mcs->key_register ("mpx", "window-prefs-h", 600);
        mcs->key_register ("mpx", "window-prefs-x", 120);
        mcs->key_register ("mpx", "window-prefs-y", 120);
        mcs->key_register ("mpx", "window-plugins-w", 500);
        mcs->key_register ("mpx", "window-plugins-h", 400);
        mcs->key_register ("mpx", "window-plugins-x", 140);
        mcs->key_register ("mpx", "window-plugins-y", 140);
        mcs->key_register ("mpx", "music-import-path", Glib::build_filename(Glib::get_home_dir (),"Music"));
        mcs->key_register ("mpx", "music-quarantine-path", Glib::build_filename(Glib::get_home_dir (),"Music Youki-Quarantined"));
        mcs->key_register ("mpx", "completion-enabled", true);
        mcs->key_register ("mpx", "theme", std::string("default"));
        mcs->key_register ("mpx", "volume", 50);
        mcs->key_register ("mpx", "minimize-on-pause", bool(false)) ; 

        mcs->domain_register ("audio");
        mcs->key_register ("audio", "sink", std::string (DEFAULT_SINK));
        mcs->key_register ("audio", "accurate-seek", bool( false ));

#ifdef HAVE_HAL
        mcs->key_register ("audio", "hal-udi", std::string (""));
#endif // HAVE_HAL

#ifdef HAVE_ALSA
        mcs->key_register ("audio", "alsa-buffer-time", 200000);
        mcs->key_register ("audio", "device-alsa", std::string (DEFAULT_DEVICE_ALSA));
#endif // HAVE_ALSA

#ifdef HAVE_SUN
        mcs->key_register ("audio", "sun-buffer-time", 200000);
        mcs->key_register ("audio", "device-sun", std::string (DEFAULT_DEVICE_SUN));
#endif // HAVE_SUN

        mcs->key_register ("audio", "oss-buffer-time", 200000);
        mcs->key_register ("audio", "device-oss", std::string (DEFAULT_DEVICE_OSS));

        mcs->key_register ("audio", "esd-buffer-time", 200000);
        mcs->key_register ("audio", "device-esd", std::string (DEFAULT_DEVICE_ESD));

        mcs->key_register ("audio", "pulse-buffer-time", 200000);
        mcs->key_register ("audio", "pulse-device", std::string ());
        mcs->key_register ("audio", "pulse-server", std::string ());

        mcs->key_register ("audio", "jack-buffer-time", 200000);
        mcs->key_register ("audio", "jack-server", std::string ());

        mcs->domain_register ("library");
        mcs->key_register ("library", "rootpath", std::string (Glib::build_filename (Glib::get_home_dir (), "Music")));
        mcs->key_register ("library", "rescan-at-startup", false);
        mcs->key_register ("library", "rescan-in-intervals", false);
        mcs->key_register ("library", "rescan-interval", 30); // in minutes
        mcs->key_register ("library", "quarantine-invalid", true);
#ifdef HAVE_HAL
        mcs->key_register ("library", "use-hal", true);
#endif // HAVE_HAL

        mcs->domain_register ("pyplugs");

        mcs->domain_register("Preferences-FileFormatPriorities");
        mcs->key_register("Preferences-FileFormatPriorities", "Format0", std::string("audio/x-flac")); 
        mcs->key_register("Preferences-FileFormatPriorities", "Format1", std::string("audio/x-ape"));
        mcs->key_register("Preferences-FileFormatPriorities", "Format2", std::string("audio/x-vorbis+ogg"));
        mcs->key_register("Preferences-FileFormatPriorities", "Format3", std::string("audio/x-musepack"));
        mcs->key_register("Preferences-FileFormatPriorities", "Format4", std::string("audio/mp4"));
        mcs->key_register("Preferences-FileFormatPriorities", "Format5", std::string("audio/mpeg"));
        mcs->key_register("Preferences-FileFormatPriorities", "Format6", std::string("audio/x-ms-wma"));
        mcs->key_register("Preferences-FileFormatPriorities", "prioritize-by-filetype", false); 
        mcs->key_register("Preferences-FileFormatPriorities", "prioritize-by-bitrate", false); 
    }

    void
    setup_i18n ()
    {
        setlocale (LC_ALL, "");
        bindtextdomain (PACKAGE, LOCALE_DIR);
        bind_textdomain_codeset (PACKAGE, "UTF-8");
        textdomain (PACKAGE);
    }

  } // anonymous namespace
} // MPX

int
main (int argc, char ** argv)
{
    setup_i18n();

    Glib::thread_init(0);
    Glib::init();
    Gio::init();

    GOptionContext * context_c = g_option_context_new (_(" - run Youki"));
    g_option_context_add_group (context_c, gst_init_get_option_group ());
    Glib::OptionContext context (context_c, true);

    try{
        gtk = new Gtk::Main (argc, argv, context);
    } catch( Glib::OptionError & cxe )
    {
        g_warning(G_STRLOC ": %s", cxe.what().c_str());
        std::exit(EXIT_FAILURE);
    }

    register_default_stock_icons();

    signal_handlers_install ();
    create_user_dirs ();
    setup_mcs ();
    gst_init(&argc, &argv);
    mpx_py_init() ;

    services = new Service::Manager;

    Splashscreen* splash = new Splashscreen;

#ifdef HAVE_HAL
    try{
        splash->set_message(_("Starting HAL..."),1/10.);
        services->add(boost::shared_ptr<HAL>(new MPX::HAL));
#endif //HAVE_HAL
        splash->set_message(_("Starting Covers"),2/10.);
        services->add(boost::shared_ptr<Covers>(new MPX::Covers));
        services->get<Covers>("mpx-service-covers")->run() ;

/*
        splash->set_message(_("Starting Library Manager..."), 2.5/10.);
        info::backtrace::Youki::MLibMan_proxy_actual * p = new info::backtrace::Youki::MLibMan_proxy_actual() ;
        p->Start() ;
        services->add(boost::shared_ptr<info::backtrace::Youki::MLibMan_proxy_actual>( p ));
*/

        splash->set_message(_("Starting Library"),3/10.);
        services->add(boost::shared_ptr<Library>(new MPX::Library));

#if 0
        splash->set_message(_("Precaching Covers..."),4/10.);
        services->get<Library>("mpx-service-library")->recacheCovers() ;
#endif

        splash->set_message(_("Starting Playback engine..."),5/10.);
        services->add(boost::shared_ptr<Play>(new MPX::Play));

        splash->set_message(_("Starting Preferences..."),6/10.);
        services->add(boost::shared_ptr<Preferences>(MPX::Preferences::create()));

        splash->set_message(_("Starting Theme Engine..."),6.5/10.);
        services->add(boost::shared_ptr<YoukiThemeEngine>( new YoukiThemeEngine ));

        splash->set_message(_("Starting Youki..."),7.5/10.);
        services->add(boost::shared_ptr<YoukiController>(new YoukiController ));

        splash->set_message(_("Starting Plugins..."),8/10.);
        services->add(boost::shared_ptr<PluginManager>(new MPX::PluginManager));

        splash->set_message(_("Starting Plugin Manager..."),9/10.);
        services->add(boost::shared_ptr<PluginManagerGUI>(MPX::PluginManagerGUI::create()));

        splash->set_message(_(""),10/10.);

        services->get<YoukiController>("mpx-service-controller")->get_widget()->show_all() ;
        services->get<YoukiController>("mpx-service-controller")->get_widget()->present() ;
        services->get<YoukiController>("mpx-service-controller")->get_widget()->raise() ;

        delete splash;
        gtk->run() ;

        // p->Exit () ;

#ifdef HAVE_HAL
    }
    catch( HAL::NotInitializedError& cxe )
    {
        g_message("%s: Critical! HAL Error: %s", G_STRLOC, cxe.what());
    }
#endif

    services->get<Covers>("mpx-service-covers")->finish() ;

    delete services ;
    delete gtk ;
    delete mcs ;

    return EXIT_SUCCESS;
}

