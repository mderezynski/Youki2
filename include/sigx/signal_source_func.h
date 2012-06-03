#ifndef _SIGX_SIGNAL_SOURCE_FUNC_HPP_
#define _SIGX_SIGNAL_SOURCE_FUNC_HPP_

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

#include <sigc++/functors/functor_trait.h>
#include <sigxconfig.h>
#include <sigx/signal_source_base.h>


namespace sigx
{

/**	@short signal source is a signal of type T_signal returned by a 
 *	functor.
 *	
 *	@note First I had the idea to use a slot to store the functor instead of
 *	storing the functor itself. But a slot has one caveat: It expects 
 *	@e T_signal to be default constructable (T_signal()), 
 *	see sigc::slotN::operator (). Not all signals have a default constructor, 
 *	e.g. Glib signals like SignalIdle.
 */
template<typename T_functor, typename T_signal>
struct signal_source_func: public signal_source_base
{
	typedef signal_source_func<T_functor, T_signal> self_type;
	typedef typename sigc::functor_trait<T_functor>::functor_type functor_type;

	signal_source_func(const T_functor& _A_func): 
		signal_source_base(reinterpret_cast<hook>(&self_type::get_signal)), 
		m_func(_A_func)
	{}
	
	static T_signal get_signal(signal_source_ptr base)
	{
		self_type* const this_ = static_cast<self_type*>(base);
		return this_->m_func();
	}


	functor_type m_func;
};


} // namespace sigx


#endif // _SIGX_SIGNAL_SOURCE_FUNC_HPP_
