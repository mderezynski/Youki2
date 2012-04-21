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

#ifndef MPX_NETWORK_HH
#define MPX_NETWORK_HH

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#include <dbus/dbus.h>
#include <glibmm.h>

#ifdef HAVE_NM
#include <NetworkManager.h>
#endif // HAVE_NM

namespace MPX
{
  class NM
  {
    public:
        NM ();
        ~NM ();

        bool
        Check_Status (bool ForcedCheck = false);

        void
        Disable ();

    private:

        bool m_Attempted;
        bool m_CachedState;

        bool
        Check_Host (std::string const& hostname,
                    guint16     port,
                    bool        write = false,
                    guint       n_tries = 4,
                    gdouble     initial_timeout = 2.0,
                    gdouble     timeout_step = 0.5);

        bool
        Check_Manually ();

#ifdef HAVE_NM
        bool
        Check_NM ();

        DBusConnection* m_DbusConnection;
#endif // HAVE_NM
  };
}

#endif // MPX_NETWORK_HH
