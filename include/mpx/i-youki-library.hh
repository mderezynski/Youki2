//  MPX
//  Copyright (C) 2005-2007 MPX Project 
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License Version 2
//  as published by the Free Software Foundation.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
//  --
//
//  The MPX project hereby grants permission for non-GPL compatible GStreamer
//  plugins to be used and distributed together with GStreamer and MPX. This
//  permission is above and beyond the permissions granted by the GPL license
//  MPX is covered by.
#ifndef _YOUKI_LIBRARY__INTERFACE__HH
#define _YOUKI_LIBRARY__INTERFACE__HH

namespace MPX
{
    class ILibrary
    {
        public:

            ILibrary(
            ) {} ;

            virtual ~ILibrary () {} ;

            virtual void
            switch_mode(bool /*use_hal*/) = 0 ;
    };
} // namespace MPX

#endif // !_YOUKI_LIBRARY__INTERFACE__HH
