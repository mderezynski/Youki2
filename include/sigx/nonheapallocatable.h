#ifndef _SIGX_NONHEAPALLOCATABLE_HPP_
#define _SIGX_NONHEAPALLOCATABLE_HPP_

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

#include <new>
#include <sigxconfig.h>


namespace sigx
{

/**	@short Private operator new and delete ensure derived classes cannot be 
 *	created with new on the heap.
 *	@note Intended use as baseclass only.
 *	@author klaus triendl
 */
class SIGX_API nonheapallocatable
{
protected:
	nonheapallocatable() {}
	~nonheapallocatable() {}


private:
	void* operator new(std::size_t);
	void operator delete(void*);
};


} // namespace sigx


#endif  // #ifndef _SIGX_NONHEAPALLOCATABLE_HPP_
