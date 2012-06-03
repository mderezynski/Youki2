#ifndef _SIGX_SIGNAL_SOURCE_THREADPRIVATE_HPP_
#define _SIGX_SIGNAL_SOURCE_THREADPRIVATE_HPP_

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

#include <glibmm/thread.h> // Glib::Private
#include <sigxconfig.h>
#include <sigx/signal_source_base.h>


namespace sigx
{

/**	@short signal source is a signal of type T_signal from a thread 
 *	private data object's member.
 */
template<typename T_threadpriv, typename T_signal>
struct signal_source_threadprivate: public signal_source_base
{
	typedef signal_source_threadprivate<T_threadpriv, T_signal> self_type;
	typedef T_signal T_threadpriv::*typed_signal_ptr;

	signal_source_threadprivate(Glib::Private<T_threadpriv>& _A_priv, typed_signal_ptr _A_sig): 
		signal_source_base(reinterpret_cast<hook>(&self_type::get_signal)), 
		m_threadpriv(_A_priv), 
		m_sig(_A_sig)
	{}
	
	static T_signal get_signal(signal_source_ptr base)
	{
		self_type* this_ = static_cast<self_type*>(base);
		const typed_signal_ptr sig = this_->m_sig;
		return this_->m_threadpriv.get()->*sig;
	}
	
	Glib::Private<T_threadpriv>& m_threadpriv;
	typed_signal_ptr m_sig;
};


} // namespace sigx


#endif // _SIGX_SIGNAL_SOURCE_THREADPRIVATE_HPP_
