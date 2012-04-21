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


#include <config.h>
#include <glib/gi18n.h>
#include <glibmm/miscutils.h>
#include <giomm.h>
#include <gst/gst.h>
#include <gtkmm/main.h>
#include <cstdlib>
#include <string>

#include "mpx/mpx-paths.hh"
#include "library-mlibman.hh"

#include "mpx/mpx-main.hh"
#include "mpx/mpx-network.hh"
#include "mpx/mpx-services.hh"
#include "mpx/mpx-types.hh"
#include "mpx/util-file.hh"
#ifdef HAVE_HAL
#include "mpx/mpx-hal.hh"
#endif // HAVE_HAL

#include "mlibmanager.hh"
#include "mpx/metadatareader-taglib.hh"
#include <dbus-c++/glib-integration.h>

#include "mpx/mpx-signals.hh"

using namespace MPX;
using namespace Glib;

namespace MPX
{
  Mcs::Config         * mcs      = 0;
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

        mcs->domain_register ("mpx");
        mcs->key_register ("mpx", "file-chooser-close-on-open", true);
        mcs->key_register ("mpx", "file-chooser-close-on-add", false);
        mcs->key_register ("mpx", "file-chooser-path", Glib::get_home_dir ());
        mcs->key_register ("mpx", "window-mlib-w", 600);
        mcs->key_register ("mpx", "window-mlib-h", 400);
        mcs->key_register ("mpx", "window-mlib-x", 100);
        mcs->key_register ("mpx", "window-mlib-y", 100);
        mcs->key_register ("mpx", "music-import-path", Glib::build_filename(Glib::get_home_dir (),"Music"));
        mcs->key_register ("mpx", "music-quarantine-path", Glib::build_filename(Glib::get_home_dir (),"Music Youki-Quarantined"));

        mcs->domain_register ("musicbrainz");
        mcs->key_register ("musicbrainz", "username", std::string ());
        mcs->key_register ("musicbrainz", "password", std::string ());

        mcs->domain_register ("library");
        mcs->key_register ("library", "rootpath", std::string (Glib::build_filename (Glib::get_home_dir (), "Music")));
        mcs->key_register ("library", "rescan-at-startup", true);
        mcs->key_register ("library", "rescan-in-intervals", true);
        mcs->key_register ("library", "rescan-interval", 30); // in minutes
        mcs->key_register ("library", "quarantine-invalid", false);
#ifdef HAVE_HAL
        mcs->key_register ("library", "use-hal", true);
#endif // HAVE_HAL

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

    DBus::Glib::BusDispatcher dispatcher ;
    DBus::default_dispatcher = &dispatcher ;
    dispatcher.attach( g_main_context_default() ) ;

    Gio::init();

    signal_handlers_install ();
    setup_mcs ();

    try{
        gtk = new Gtk::Main( argc, argv );
    } catch( Glib::OptionError & cxe )
    {
        g_warning(G_STRLOC ": %s", cxe.what().c_str());
        std::exit(EXIT_FAILURE);
    }

    gst_init(&argc, &argv);

    services = new Service::Manager;

    DBus::Connection conn = DBus::Connection::SessionBus () ;
    conn.request_name( "info.backtrace.Youki.MLibMan" ) ;

#ifdef HAVE_HAL
    try{
        services->add(boost::shared_ptr<HAL>(new MPX::HAL));
#endif //HAVE_HAL
        services->add(boost::shared_ptr<MetadataReaderTagLib>(new MPX::MetadataReaderTagLib));
        services->add(boost::shared_ptr<Library_MLibMan>(new MPX::Library_MLibMan));
#ifdef HAVE_HAL
        services->add(boost::shared_ptr<MLibManager>(MPX::MLibManager::create( conn )));
#endif // HAVE_HAL

        gtk->run() ;

#ifdef HAVE_HAL
    }
    catch( HAL::NotInitializedError& cxe )
    {
        g_message("%s: Critical! HAL Error: %s", G_STRLOC, cxe.what());
    }
#endif

    delete services ;
    delete gtk ;
//    delete mcs ;

    return EXIT_SUCCESS;
}

