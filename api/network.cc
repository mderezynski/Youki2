//  MPX
//  Copyright (C) 2005-2007 MPX Project 
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

#include <config.h>

#ifdef HAVE_SUN
# include <fcntl.h>
#endif // !HAVE_SUN

#include <cstdio>
#include <cstring>

#include <dbus/dbus-glib-lowlevel.h>
#include <dbus/dbus-glib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include "mpx/mpx-uri.hh"
#include "mpx/mpx-minisoup.hh"
#include "mpx/mpx-network.hh"


namespace MPX
{
    NM::NM ()
        : m_Attempted (false)
        , m_CachedState (false)
    {
        DBusError error;
        dbus_error_init (&error);

#ifdef HAVE_NM
        m_DbusConnection = dbus_bus_get (DBUS_BUS_SYSTEM, &error);
        if( dbus_error_is_set (&error) )
        {
            m_DbusConnection = 0;
        }

        dbus_connection_setup_with_g_main (m_DbusConnection, NULL);
        dbus_connection_set_exit_on_disconnect (m_DbusConnection, FALSE);
#endif // HAVE_NM
    }

    NM::~NM ()
    {
    }

    void
    NM::Disable ()
    {
        m_Attempted = true;
        m_CachedState = false;
    }

    bool
    NM::Check_Host (std::string const& hostname,
                                guint16     port,
                                bool        write,
                                guint       n_tries,
                                gdouble     initial_timeout,
                                gdouble     timeout_step)
    {
      struct hostent* host = gethostbyname (hostname.c_str ());

      if (!host)
      {
          return false;
      }

      struct sockaddr_in serv_addr;

      serv_addr.sin_family = host->h_addrtype;
      std::memcpy (&serv_addr.sin_addr.s_addr, host->h_addr_list[0], host->h_length);
      serv_addr.sin_port = htons (port);

      // Create socket
      int sd = socket (AF_INET, SOCK_STREAM, 0);
      if (sd < 0)
      {
        return false;
      }

      struct sockaddr_in local_addr;

      // Bind Any Port Number
      local_addr.sin_family = AF_INET;
      local_addr.sin_addr.s_addr = htonl (INADDR_ANY);
      local_addr.sin_port = htons(0);

      int rc = bind (sd, reinterpret_cast<struct sockaddr const*> (&local_addr), sizeof (local_addr));
      if ( rc < 0 )
      {
        return false;
      }

      int flags = fcntl (rc, F_GETFL, 0);
      fcntl (rc, F_SETFL, flags | O_NONBLOCK);

      // Connect To Server
      rc = connect (sd, reinterpret_cast<struct sockaddr const*> (&serv_addr), sizeof (serv_addr));

      int errsv = errno;
      if (rc < 0)
      {
        if (!(rc == -1 && errsv == EINPROGRESS))
        {
          return false;
        }
      }

      gdouble timeout = initial_timeout;
      struct timeval tv;
      fd_set rfds;
      int retval;

      for (guint i = 0; i < n_tries; ++i)
      {
          FD_ZERO (&rfds);
          FD_SET (rc, &rfds);

          tv.tv_sec  = time_t (timeout);
          tv.tv_usec = (long)((timeout - tv.tv_sec) * 1000000);

          if (write)
            retval = select (rc+1, &rfds, NULL, NULL, &tv);
          else
            retval = select (rc+1, NULL, &rfds, NULL, &tv);

          if (retval >= 0)
          {
              connect (sd, reinterpret_cast<struct sockaddr const*> (&serv_addr), sizeof (serv_addr));
              shutdown (sd, SHUT_RDWR);
              return true;
          }
          timeout += timeout_step;
      }

      close (sd);
      shutdown (sd, SHUT_RDWR);
      return false;
    }

    bool
    NM::Check_Manually ()
    {
      return Check_Host ("google.com", 80);
    }

#ifdef HAVE_NM
    bool
    NM::Check_NM ()
    {
      DBusMessage *message, *reply;
      DBusError error;
      dbus_uint32_t state;
      
      message = dbus_message_new_method_call (NM_DBUS_SERVICE, NM_DBUS_PATH,
                          NM_DBUS_INTERFACE, "state");
      if (!message)
          return false;

      dbus_error_init (&error);
      reply = dbus_connection_send_with_reply_and_block (m_DbusConnection, message,
                                 -1, &error);
      dbus_message_unref (message);
      if (!reply)  // FIXME: Maybe we should have policies configured by user whether to fall back on manual check but that's getting techy
      {
          return Check_Manually();
      }

      if (!dbus_message_get_args (reply, NULL, DBUS_TYPE_UINT32, &state,
                      DBUS_TYPE_INVALID))
      {
        return Check_Manually();
      }

      if (state != NM_STATE_CONNECTED)
          return false;

      return true;
    }
#endif // HAVE_NM

    bool
    NM::Check_Status (bool ForcedCheck)
    {
      if( m_Attempted && !ForcedCheck )
        return m_CachedState;

      m_Attempted = true;

#ifdef HAVE_NM
      if( !m_DbusConnection )
      {
        m_CachedState = Check_Manually ();
      }
      else
      {
        m_CachedState = Check_NM ();
      }
#else
      m_CachedState = Check_Manually ();
#endif // HAVE_NM

      return m_CachedState;
    }
}

