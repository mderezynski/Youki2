//  BMPx - The Dumb Music Player
//  Copyright (C) 2005-2007 BMPx development team.
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
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
//  The BMPx project hereby grants permission for non GPL-compatible GStreamer
//  plugins to be used and distributed together with GStreamer and BMPx. This
//  permission is above and beyond the permissions granted by the GPL license
//  BMPx is covered by.

#ifndef _BMP_GLIB_ADDONS_HH
#define _BMP_GLIB_ADDONS_HH

#include <glibmm.h>

template <typename Property>
struct ProxyOf
{
    typedef typename Property::PropertyType                      PropertyType;
    typedef typename Glib::PropertyProxy_ReadOnly<PropertyType>  ReadOnly;
    typedef typename Glib::PropertyProxy_WriteOnly<PropertyType> WriteOnly;
    typedef typename Glib::PropertyProxy<PropertyType>           ReadWrite;
};

typedef Glib::Property<std::string> PropString;
typedef Glib::Property<bool>        PropBool;
typedef Glib::Property<gint64>      PropInt;
typedef Glib::Property<double>      PropFloat;

#endif //!_BMP_GLIB_ADDONS_HH
