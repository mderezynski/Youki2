#ifndef _SIGX_NONASSIGNABLE_HPP_
#define _SIGX_NONASSIGNABLE_HPP_

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

#include <sigxconfig.h>


namespace sigx
{

/**	@short Private assignment operator (operator =) ensures that derived classes
 *	can't be copied by assignment.
 *	@note Intended use as a baseclass only.
 *	@author klaus triendl
 */
class SIGX_API nonassignable
{
protected:
	nonassignable() {}
	~nonassignable() {}


private:
	nonassignable& operator =(const nonassignable&);
};


} // namespace sigx


#endif  // #ifndef _SIGX_NONASSIGNABLE_HPP_
