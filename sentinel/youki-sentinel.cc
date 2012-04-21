//  Youki
//  Copyright (C) 2005-2009 MPX development.
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

#include "config.h"

#include <cstdlib>
#include <cstring>
#include <string>

#include <locale.h>
#include <glib.h>

#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <dbus/dbus.h>

#include <boost/format.hpp>

#define SERVER_NAME "stats.backtrace.info"

namespace
{
    GMainLoop*       mainloop   = NULL;
    DBusGConnection* session    = NULL;
    DBusConnection*  sessraw    = NULL;
    DBusGProxy*      fdobus     = NULL;
    DBusGProxy*      youki      = NULL;

    void
    send_crash ()
    {
        /*
        static boost::format uri_f ("http://stats.backtrace.info/stats.php?version=%s&svnrev=%s&system=%s&action=%s");
        std::string uri = (uri_f % VERSION % RV_REVISION % BUILD_ARCH % "crash").str ();

        SoupSession * session = soup_session_sync_new ();
        SoupMessage * message = soup_message_new ("GET", uri.c_str());
      
        soup_session_send_message (session, message);
        g_object_unref (session);
        */
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
            std::exit (EXIT_SUCCESS);
        }
    }

    void
    on_youki_shutdown_complete (DBusGProxy* proxy,
                              gpointer    data)
    {
        dbus_g_proxy_disconnect_signal(
              fdobus
            , "NameOwnerChanged"
            , G_CALLBACK (name_owner_changed)
            , NULL
        ) ;
        g_main_loop_quit( mainloop ) ;
    }
} 

int
main (int    argc,
      char** argv)
{
    GError *error = NULL;

    setup_i18n();
    g_type_init();
    dbus_g_type_specialized_init();

    mainloop = g_main_loop_new( NULL, FALSE ) ;
    session = dbus_g_bus_get( DBUS_BUS_SESSION, &error ) ;

    if( error )
    {
        g_log(
            G_LOG_DOMAIN
          , G_LOG_LEVEL_ERROR
          , "DBus Error: %s"
          , error->message
        ) ;

        g_error_free (error);
        return EXIT_FAILURE;
    }

    if(! session ) // We've been activated by it but can't access it now -> Something's very wrong
    {
        return EXIT_FAILURE;
    }

    sessraw = dbus_g_connection_get_connection( session ) ;

    dbus_connection_setup_with_g_main( sessraw, g_main_context_default() ) ;

    fdobus = dbus_g_proxy_new_for_name(
          session
        , "org.freedesktop.DBus"
        , "/org/freedesktop/DBus"
        , "org.freedesktop.DBus"
    ) ;

    unsigned int request_name_result = 0 ;

    if(!dbus_g_proxy_call(

          fdobus

        , "RequestName"
        , &error

        , G_TYPE_STRING 
        , "info.backtrace.Youki.Sentinel"

        , G_TYPE_UINT
        , 0

        , G_TYPE_INVALID

        , G_TYPE_UINT
        , &request_name_result

        , G_TYPE_INVALID)
    )
    {
        g_error ("%s: Youki-Sentinel: failed RequestName request: %s", G_STRFUNC, error->message);
        g_error_free( error ) ;
        error = 0 ;
    }

    switch( request_name_result )
    {
        case DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER:
            break;

        case DBUS_REQUEST_NAME_REPLY_IN_QUEUE:
        case DBUS_REQUEST_NAME_REPLY_EXISTS:
        case DBUS_REQUEST_NAME_REPLY_ALREADY_OWNER:
            g_object_unref( fdobus ) ;
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

    youki = dbus_g_proxy_new_for_name_owner(
          session
        , "info.backtrace.Youki.App" 
        , "/info/backtrace/Youki/App" 
        , "info.backtrace.Youki.App" 
        , &error
    ) ;

    if( !youki )
    {
        g_message( "Youki-Remote: Youki-App service not available, exiting." ) ;
        return EXIT_FAILURE ;
    }

    if( error )
    {
        g_log(
              G_LOG_DOMAIN
            , G_LOG_LEVEL_ERROR
            , "Youki-Sentinel: DBus error during acquisition of proxy for info.backtrace.Youki.App: %s"
            , error->message
        ) ;
        g_error_free (error);
        return EXIT_FAILURE;
    }

    dbus_g_proxy_add_signal(
          youki
        , "ShutdownComplete"
        , G_TYPE_INVALID
    ) ;

    dbus_g_proxy_connect_signal(
          youki
        , "ShutdownComplete"
        , G_CALLBACK( on_youki_shutdown_complete )
        , NULL
        , NULL
    ) ;

    g_main_loop_run( mainloop ) ;
    g_main_loop_unref( mainloop ) ;

    g_object_unref( youki ) ;
    g_object_unref( fdobus ) ;
    dbus_connection_unref( sessraw ) ;
    g_object_unref( session ) ;

    return EXIT_SUCCESS;
}
