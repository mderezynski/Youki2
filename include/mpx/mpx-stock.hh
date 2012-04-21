//  MPX
//  Copyright (C) 2005-2007 MPX development.
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

#ifndef MPX_STOCK_HH
#define MPX_STOCK_HH

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif //HAVE_CONFIG_H
#include <vector>

// Default stock icons
#  define MPX_STOCK_PLUGIN          "mpx-stock-plugin"
#  define MPX_STOCK_LASTFM          "mpx-stock-lastfm"
#  define MPX_STOCK_PLUGIN_DISABLED "mpx-stock-plugin-disabled"
#  define MPX_STOCK_ERROR           "mpx-stock-error"
#  define MPX_STOCK_EQUALIZER       "mpx-stock-equalizer"

namespace MPX
{
    struct StockIconSpec
    {
        std::string     filename;
        std::string     stock_id;

        StockIconSpec(std::string const& filename_, std::string const& stock_id_)
        : filename(filename_)
        , stock_id(stock_id_)
        {
        }
    };
    typedef std::vector<StockIconSpec> StockIconSpecV;

    void
    register_stock_icons (StockIconSpecV const& icons, std::string const& base_path);

    void
    register_default_stock_icons ();

    std::string
    default_stock_path ();
}

#endif //MPX_STOCK_HH
