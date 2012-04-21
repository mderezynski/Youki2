//  Youki
//  Copyright (C) 2008 Youki 
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
//  The Youki project hereby grants permission for non-GPL compatible GStreamer
//  plugins to be used and distributed together with GStreamer and Youki. This
//  permission is above and beyond the permissions granted by the GPL license
//  Youki is covered by.

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif //HAVE_CONFIG_H

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdint.h>

#include <locale.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <glib-object.h>
#include <glib/gprintf.h>
#include <glibmm/miscutils.h>
#include <gtk/gtk.h>

#include <boost/format.hpp>

#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <dbus/dbus.h>

namespace
{
    GMainLoop* mainloop = NULL;

    void
    print_version ()
    {
      g_print ("%s %s",
               PACKAGE,
               PACKAGE_VERSION);

      g_print (_("\nYouki Copyright (c) 2009 MPX Project <http://mpx.backtrace.info>\n\n"));
    }

    void
    print_configure ()
    {
      g_print ("%s\n",
               CONFIGURE_ARGS);
    }

    void
    setup_i18n ()
    {
      setlocale (LC_ALL, "");
      bindtextdomain (PACKAGE, LOCALE_DIR);
      bind_textdomain_codeset (PACKAGE, "UTF-8");
      textdomain (PACKAGE);
    }

    void
    app_startup_completed (DBusGProxy* proxy,
                           gpointer    data)
    {
        g_main_loop_quit( mainloop ) ;
    }

    void
    app_quit (DBusGProxy* proxy,
              gpointer    data)
    {
      g_main_loop_quit( mainloop ) ;
      g_main_loop_unref( mainloop ) ;

      std::exit( EXIT_SUCCESS ) ;
    }

    void
    display_startup_crash ()
    {
      // We assume Youki started up, but crashed again

      if (!g_getenv ("DISPLAY"))
      {
          char const* command = PREFIX "/libexec/youki-bin --no-log";

          char const* message =
            _("       Youki seems to have crashed.\n"
              "       Please try starting it from a terminal using '%s'\n"
              "       for further information on what could have caused the crash\n"
              "       and report it to our IRC channel, #youki on irc.freenode.net\n"
              "\n");
          g_print( message, command ) ;
      }
      else
      {
          gtk_init( 0, 0 ) ;
          GtkWidget *dialog = gtk_message_dialog_new_with_markup (NULL, GtkDialogFlags (0), GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                    _("Youki tried to start up, but crashed.\n\n"
                                      "Please file a bug at:\n\n"
                                      "http://mpx.backtrace.info/newticket"));

          gtk_window_set_title (GTK_WINDOW (dialog), _("Youki Crashed"));
          gtk_dialog_run (GTK_DIALOG (dialog));
          gtk_widget_destroy (dialog);
      }

      exit( EXIT_FAILURE ) ;
    }

    void
    name_owner_changed (DBusGProxy* proxy,
                        char*       name,
                        char*       old_owner,
                        char*       new_owner,
                        gpointer    data)
    {
        if (!name || (name && std::strcmp (name, "info.backtrace.Youki.App")))
            return;

        if (std::strlen (old_owner) && !std::strlen (new_owner))
        {
            display_startup_crash ();
        }
    }

    void
    print_error (DBusError* error)
    {
        g_log (G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "DBus Error: (%s): %s", error->name, error->message);
    }

    void
    print_g_error (GError** error)
    {
        g_log (G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "Error: %s", (*error)->message);
        g_error_free (*error);
        *error = NULL;
    }
} // anonymous namespace

int
main (int    argc,
      char **argv)
{
    GError *error = NULL;

    setup_i18n ();
    g_type_init ();
    dbus_g_type_specialized_init ();

    mainloop = g_main_loop_new( NULL, FALSE ) ;

    if ((argc > 1) && !strcmp(argv[1], "--version"))
    {
        print_version ();
        print_configure ();
        return EXIT_SUCCESS;
    }

    DBusGConnection* session = dbus_g_bus_get( DBUS_BUS_SESSION, &error ) ;

    if(! session )
    {
        if (error)
        {
            g_log(
                G_LOG_DOMAIN
              , G_LOG_LEVEL_CRITICAL
              , "Youki-Startup: DBus error: %s"
              , error->message
            ) ;
            g_error_free( error ) ;
            error = NULL ;
        }

        if( !g_getenv ("DISPLAY") )
        {
            g_printerr (_("Youki-Startup: Could not connect to session bus: %s\n\n"), error->message);
            return EXIT_FAILURE;
        }

        gtk_init (&argc, &argv);

        boost::format error_fmt (_("<big><b>Youki-Startup DBus Error</b></big>\n\nYouki can not be started trough DBus activation.\n"
                                   "The following error occured trying to start up Youki:\n\n'<b>%s</b>'\n\n"
                                   "DBus might not be running at all or have even crashed, please consult further "
                                   "help with this problem from DBus or Youki"));

        std::string message;

        if (error)
        {
            message = (error_fmt % error->message).str ();
        }
        else
        {
            message = (error_fmt % _("(Unknown error. Perhaps DBus is not running at all?)")).str ();
        }

        GtkWidget * dialog = gtk_message_dialog_new_with_markup(
                                    NULL
                                  , GtkDialogFlags (0)
                                  , GTK_MESSAGE_ERROR
                                  , GTK_BUTTONS_OK
                                  , message.c_str()
        ) ;

        if( error )
        {
            g_error_free( error ) ;
            error = NULL ;
        }

        gtk_window_set_title( GTK_WINDOW (dialog), _("Youki-Startup: DBus Error") ) ;
        gtk_dialog_run( GTK_DIALOG (dialog) ) ;
        gtk_widget_destroy( dialog ) ;

        return EXIT_FAILURE;
    }

    DBusConnection* sessraw = dbus_g_connection_get_connection (session);
    dbus_connection_setup_with_g_main (sessraw, g_main_context_default());

    DBusGProxy* fdobus = dbus_g_proxy_new_for_name (session, "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus");

    unsigned int request_name_result = 0 ;

    if(!dbus_g_proxy_call(
              fdobus

            , "RequestName"
            , &error

            , G_TYPE_STRING
            , "info.backtrace.Youki.Startup"

            , G_TYPE_UINT
            , 0

            , G_TYPE_INVALID

            , G_TYPE_UINT
            , &request_name_result

            , G_TYPE_INVALID
    ))
    {
        g_error("Youki-Startup: failed DBus RequestName request: %s", error->message);
        g_error_free( error ) ;
        return EXIT_FAILURE;
    }

    switch( request_name_result )
    {
        case DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER:
            break;

        case DBUS_REQUEST_NAME_REPLY_IN_QUEUE:
        case DBUS_REQUEST_NAME_REPLY_EXISTS:
        case DBUS_REQUEST_NAME_REPLY_ALREADY_OWNER:

            if( fdobus )
                g_object_unref( fdobus ) ;

            g_message( "Youki-Startup: Name info.backtrace.Youki.Startup exists" ) ;

            return EXIT_SUCCESS;
    }

    dbus_g_proxy_add_signal(
          fdobus
        , "NameOwnerChanged"
        , G_TYPE_STRING
        , G_TYPE_STRING
        , G_TYPE_STRING
        , G_TYPE_INVALID
    ) ;

    dbus_g_proxy_connect_signal(
          fdobus
        , "NameOwnerChanged"
        , G_CALLBACK( name_owner_changed )
        , NULL
        , NULL
    ) ;

    gboolean youki_running = TRUE;

    DBusGProxy* youki = dbus_g_proxy_new_for_name_owner(
                                session
                              , "info.backtrace.Youki.App"
                              , "/info/backtrace/Youki/App"
                              , "info.backtrace.Youki.App"
                              , &error
    ) ;

    if( !youki )
    {
        youki_running = FALSE;
    }

    if( error )
    {
        g_error_free( error ) ;
        error = NULL ;
        // we don't exit here since it could just mean it's not running
    }

    if( !youki_running )
    {
            youki = dbus_g_proxy_new_for_name(
                      session
                    , "info.backtrace.Youki.App"
                    , "/info/backtrace/Youki/App"
                    , "info.backtrace.Youki.App"
            ) ;

            dbus_g_proxy_call(
                      youki
                    , "Startup"
                    , &error
                    , G_TYPE_INVALID
                    , G_TYPE_INVALID
            ) ;

            if( error )
            {
                g_error_free( error ) ;
                error = NULL ;
            }

            dbus_g_proxy_add_signal(
                  youki
                , "StartupComplete"
                , G_TYPE_INVALID
            ) ;

            dbus_g_proxy_add_signal(
                  youki
                , "Quit"
                , G_TYPE_INVALID
            ) ;

            dbus_g_proxy_connect_signal(
                  youki
                , "StartupComplete"
                , G_CALLBACK( app_startup_completed )
                , NULL
                , NULL
            ) ;

            dbus_g_proxy_connect_signal(
                  youki
                , "Quit"
                , G_CALLBACK( app_quit )
                , NULL
                , NULL
            ) ;

            g_main_loop_run (mainloop);
    }
    else
    {
            g_message( "Youki-Startup: Youki already running, raising window." ) ;

            youki = dbus_g_proxy_new_for_name(
                      session
                    , "info.backtrace.Youki.App"
                    , "/info/backtrace/Youki/App"
                    , "info.backtrace.Youki.App"
            ) ;

            dbus_g_proxy_call(
                      youki
                    , "Present"
                    , &error
                    , G_TYPE_INVALID
                    , G_TYPE_INVALID
            ) ;
    }

    g_main_loop_unref (mainloop);

    if( youki )
        g_object_unref( youki ) ;

    if( fdobus )
        g_object_unref( fdobus ) ;

    return EXIT_SUCCESS;
}
