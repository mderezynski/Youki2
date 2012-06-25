/*
 * Copyright 2006 Klaus Triendl
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free 
 * Software Foundation, 51 Franklin Street, Fifth Floor, 
 * Boston, MA 02110-1301, USA.
*/

#include "sigx/signal_wrapper_base.h"
#include "sigx/signal_source_base.h"
#include "sigx/tunnel_base.h"
#include "__sigx_pchfence__.h"


namespace sigx
{

signal_wrapper_base::signal_wrapper_base(): 
	m_disp(), 
	m_sigsource()
{}

signal_wrapper_base::signal_wrapper_base(const shared_dispatchable& _A_disp, const std::tr1::shared_ptr<signal_source_base>& _A_sigsource) throw(): 
	m_disp(_A_disp), 
	m_sigsource(_A_sigsource)
{}

signal_wrapper_base::~signal_wrapper_base() throw()
{}



std::pair<connection_wrapper, std::tr1::shared_ptr<sigc_connection_ptr> >
signal_wrapper_base::prepare_connection(const tunnel_base& _A_tunnel) const
{
	// prepare a connection pointer;
	// note that the value '0x01' denotes a pending connection request
	std::tr1::shared_ptr<sigc_connection_ptr> refconnptr(new sigc_connection_ptr((sigc_connection_ptr) 0x01));
	const connection_wrapper conn(m_disp, refconnptr);
	// add the connection_wrapper to the validity tracker of the tunnel functor.
	// This enables the validity tracker to disconnect the future slot connected
	// to this signal.
	_A_tunnel.validity_tracker().add_connection(conn);

	return std::make_pair(conn, refconnptr);
}


} // namespace sigx
