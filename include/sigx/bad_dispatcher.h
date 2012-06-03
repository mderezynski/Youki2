#ifndef _SIGX_BAD_DISPATCHER_HPP_
#define _SIGX_BAD_DISPATCHER_HPP_

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

#include <exception>


namespace sigx
{

class bad_dispatcher: public std::exception//, public operator_new
{
public:
	bad_dispatcher(): std::exception()
	{}

	// virtuals from std::exception
	virtual const char* what() const throw();
};


/**	@short Catcher for a sigc::exception_catch_functor ignoring exceptions
 *	of type sigx::bad_dispatcher.
 */
template<typename T_return>
struct bad_dispatcher_catcher
{
	T_return operator ()() const
	{
		try
		{ throw; }
		catch (const ::sigx::bad_dispatcher&)
		{}
		return T_return();
	}
};


} // namespace sigx


#endif // end file guard
