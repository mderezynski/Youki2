#ifndef _SIGX_NONPOINTERALIASING_HPP_
#define _SIGX_NONPOINTERALIASING_HPP_

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

/**	@short Private address operator (operator &) ensures that the address of 
 *	derived objects can't be taken.
 *	@note Intended use as a baseclass only.
 *	@author klaus triendl
 */
class SIGX_API nonpointeraliasing
{
protected:
	nonpointeraliasing() {}
	~nonpointeraliasing() {}


private:
	nonpointeraliasing* operator &();
	nonpointeraliasing* operator &() const;
	nonpointeraliasing* operator &() volatile;
	nonpointeraliasing* operator &() const volatile;
};


} // namespace sigx


#endif  // #ifndef _SIGX_NONPOINTERALIASING_HPP_
