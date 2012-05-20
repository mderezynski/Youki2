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
#include "mpx/mpx-network.hh"

namespace MPX
{
    NM::NM()
    : m_NM_Client(nullptr)
    {
	m_NM_Client = nm_client_new() ;
    }

    NM::~NM()
    {
	g_object_unref(m_NM_Client) ;
    }

    bool
    NM::is_connected()
    {
	if(!m_NM_Client)
	    return true ; // we assume all network functions will just fail gracefully

	NMState state = nm_client_get_state(m_NM_Client) ;
	return( state == NM_STATE_CONNECTED_GLOBAL ) ;
    }
}

