#ifndef _SIGX_SIGNAL_SOURCE_OBJ_MEM_HPP_
#define _SIGX_SIGNAL_SOURCE_OBJ_MEM_HPP_

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

/**	@short signal source is a object's member of type T_signal.
 */
template<typename T_obj, typename T_signal>
struct signal_source_obj_mem: public signal_source_base
{
	typedef signal_source_obj_mem<T_obj, T_signal> self_type;
	typedef T_signal T_obj::*typed_signal_ptr;

	signal_source_obj_mem(T_obj* _A_obj, typed_signal_ptr _A_sig): 
		signal_source_base(reinterpret_cast<hook>(&self_type::get_signal)), 
		m_obj(_A_obj), 
		m_sig(_A_sig)
	{}
	
	static T_signal get_signal(signal_source_ptr base)
	{
		self_type* const this_ = static_cast<self_type*>(base);
		const typed_signal_ptr sig = this_->m_sig;
		return this_->m_obj->*sig;
	}
	
	T_obj* m_obj;
	typed_signal_ptr m_sig;
};

/**	@short signal source is a object's member of type T_signal.
 *	Object instance is late bound.
 */
template<typename T_obj, typename T_signal>
struct signal_source_pobj_mem: public signal_source_base
{
	typedef signal_source_pobj_mem<T_obj, T_signal> self_type;
	typedef T_signal T_obj::*typed_signal_ptr;

	signal_source_pobj_mem(T_obj* const& _A_obj, typed_signal_ptr _A_sig): 
		signal_source_base(reinterpret_cast<hook>(&self_type::get_signal)), 
		m_obj(_A_obj), 
		m_sig(_A_sig)
	{}
	
	static T_signal get_signal(signal_source_ptr base)
	{
		self_type* const this_ = static_cast<self_type*>(base);
		const typed_signal_ptr sig = this_->m_sig;
		return this_->m_obj->*sig;
	}
	
	T_obj* const& m_obj;
	typed_signal_ptr m_sig;
};

/**	@short signal source is a object's member function returning a signal of 
 *	type T_signal.
 *	Object instance is late bound.
 */
template<typename T_obj, typename T_functor, typename T_signal>
struct signal_source_pobj_mem_fun: public signal_source_base
{
	typedef signal_source_pobj_mem_fun<T_obj, T_functor, T_signal> self_type;
	typedef typename sigc::functor_trait<T_functor>::functor_type functor_type;

	signal_source_pobj_mem_fun(T_obj* const& _A_obj, const T_functor& _A_mem_func): 
		signal_source_base(reinterpret_cast<hook>(&self_type::get_signal)), 
		m_obj(_A_obj), 
		m_mem_func(_A_mem_func)
	{}
	
	static T_signal get_signal(signal_source_ptr base)
	{
		self_type* this_ = static_cast<self_type*>(base);
		return this_->m_mem_func(this_->m_obj);
	}
	
	T_obj* const& m_obj;
	functor_type m_mem_func;
};


} // namespace sigx


#endif // _SIGX_SIGNAL_SOURCE_OBJ_MEM_HPP_
