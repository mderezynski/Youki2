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

#include "sigx/signal_f_base.h"
#include "sigx/shared_dispatchable.h"
#include "sigx/signal_source_base.h"
#include "__sigx_pchfence__.h"


namespace sigx
{

signal_f_base::signal_f_base(const shared_dispatchable& _A_disp, signal_source_ptr _A_psigsource): 
	m_disp(_A_disp), 
	m_sigsource(_A_psigsource)
{}



} // namespace sigx
