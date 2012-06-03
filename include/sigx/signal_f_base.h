#ifndef _SIGX_SIGNAL_F_BASE_HPP_
#define _SIGX_SIGNAL_F_BASE_HPP_

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

#include <tr1/memory>	// std::tr1::shared_ptr
#include <sigxconfig.h>
#include <sigx/fwddecl.h>
#include <sigx/nonassignable.h>
#include <sigx/nonheapallocatable.h>
#include <sigx/nonpointeraliasing.h>
#include <sigx/shared_dispatchable.h>


namespace sigx
{

/**	@short Base class for signal functors, see signal_f.
 *	@note non-copyable, not constructible on the heap (with new) and can't be 
 *	pointer aliased (with operator &) to ensure that it is de-facto bound to  
 *	a wrapping object.
 *	
 *	@ingroup signals
 *	@author klaus triendl
 */
class SIGX_API signal_f_base: nonassignable, nonheapallocatable, nonpointeraliasing
{
protected:
	signal_f_base(const shared_dispatchable& _A_disp, signal_source_ptr _A_psigsource);
	// implicit dtor is fine; non-virtual by design


protected:
	shared_dispatchable m_disp;
	/**	@short Shared signal source
	 */
	std::tr1::shared_ptr<signal_source_base> m_sigsource;
};


} // namespace sigx


#endif  // end file guard
