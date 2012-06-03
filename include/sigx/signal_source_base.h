#ifndef _SIGX_SIGNAL_SOURCE_BASE_HPP_
#define _SIGX_SIGNAL_SOURCE_BASE_HPP_

/*
 * Copyright 2005 Klaus Triendl
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free 
 * Software Foundation, 51 Franklin Street, Fifth Floor, 
 * Boston, MA 02110-1301, USA.
 */

#include <sigxconfig.h>
#include <sigx/operator_new.h>


namespace sigx
{

/**	@short Represents a source for any type of signal.
 *	
 *	Signal sources are proxies for a concrete signal and offer 
 *	different ways of accessing a signal: 
 *	- from thread private data
 *	- through an object's method
 *	- through a functor
 */
class signal_source_base: public operator_new
{
public:
	/// An untyped function pointer
	typedef void (*hook)();

	
protected:
	hook m_getter;


	signal_source_base(hook _A_getter): 
		m_getter(_A_getter)
	{}
		
public:
	virtual ~signal_source_base()
	{
		m_getter = 0;
	}
	
	hook getter() const	{	return m_getter;	}
};


typedef signal_source_base* signal_source_ptr;


} // namespace sigx


#endif // _SIGX_SIGNAL_SOURCE_BASE_HPP_
