/*
 * Copyright 2008 Klaus Triendl
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

#include <sigx/operator_new.h>
#include "__sigx_pchfence__.h"


namespace sigx
{

void* operator_new::operator new(std::size_t size)
{
	// forward to global operator new()
	return ::operator new(size);
}

void operator_new::operator delete(void* p)
{
	// forward to global operator delete()
	::operator delete(p);
}

void* operator_new::operator new[](std::size_t size)
{
	// forward to global operator new[]()
	return ::operator new[](size);
}

void operator_new::operator delete[](void* p)
{
	// forward to global operator delete[]()
	::operator delete[](p);
}


} // namespace sigx
