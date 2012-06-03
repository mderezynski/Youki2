#ifndef _SIGX_NONCOPYABLE_HPP_
#define _SIGX_NONCOPYABLE_HPP_

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

/*
 * Inspired by Beman Dawes' boost::noncopyable
*/

#include <sigxconfig.h>


namespace sigx
{

/**	@short Private copy constructor and copy assignment ensure derived classes 
 *	cannot be copied.
 *	@note Intended use as baseclass only.
 */
class SIGX_API noncopyable
{
protected:
	noncopyable() {}
	~noncopyable() {}

private:
	noncopyable(const noncopyable&);
	noncopyable& operator =(const noncopyable&);
};


} // namespace sigx


#endif  // #ifndef _SIGX_NONCOPYABLE_HPP_
