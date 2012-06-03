#ifndef _SIGX_OPERATOR_NEW_HPP_
#define _SIGX_OPERATOR_NEW_HPP_

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

/**	@short Ensures allocation of derived objects in the sigx module
 *	@note Intended use as baseclass only.
 */
class SIGX_API operator_new
{
protected:
	operator_new() {}
	~operator_new() {}


public:
	void* operator new(std::size_t size);
	void operator delete(void* p);
	void* operator new[](std::size_t size);
	void operator delete[](void* p);
};


} // namespace sigx


#endif // end file guard
