#ifndef _SIGX_TUNNEL_BASE_HPP_
#define _SIGX_TUNNEL_BASE_HPP_

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

#include <sigx/shared_dispatchable.h>
#include <sigx/tunnel_validity_tracker.h>


namespace sigx
{

/**	@short A hint to the compiler that the functor is a tunnel_functor.
 *	@note We could derive it from sigc::adaptor_base but then we would end
 *	up with two instances of sigc::adaptor_base and sigc::functor_base because
 *	tunnel functors derive additionally from sigc::adapts<T_functor> that 
 *	derives from sigc::adaptor_base, too.
 *	We could define our own intermediate type like 
 *	sigx::tunnels<T_functor> in the same way sigc::adapts<T_functor> is 
 *	defined but we don't want to break sigc++ functionality.
 */
class SIGX_API tunnel_base
{
public:
	tunnel_base(const shared_dispatchable& _A_disp);

	// implicit copy ctor is fine
	// implicit dtor is fine
	// implicit assignment operator is fine


public:
	tunnel_validity_tracker& validity_tracker() const
	{	return m_validity_tracker;	}


protected:
	shared_dispatchable m_disp;
	mutable tunnel_validity_tracker m_validity_tracker;
};


} // namespace sigx


#endif // _SIGX_TUNNEL_BASE_HPP_
