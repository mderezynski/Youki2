//  MPX
//  Copyright (C) 2010 MPX development.
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
#include "config.h"
#include <gtkmm.h>
#include <string>

#include "mpx/mpx-stock.hh"

using namespace Glib ;
using namespace Gtk ;
using namespace Gdk ;

namespace MPX
{
    void
    register_stock_icons(
          const StockIconSpecV&     spec
        , int                       size
        , const std::string&        path
    )
    {
        RefPtr<Gtk::IconTheme> theme = IconTheme::get_default ();

        for( StockIconSpecV::const_iterator i = spec.begin(); i != spec.end(); ++i )
        {
            const StockIconSpec& icon = *i;

            theme->add_builtin_icon(
                  icon.stock_id
                , size
                , Pixbuf::create_from_file( build_filename( path, icon.filename ))
            ) ;

            g_message("Registered stock icon ['%s'] with file: '%s'", icon.stock_id.c_str(), icon.filename.c_str() ) ;
        }
    }

    std::string
    default_stock_path(
          const std::string& s
    )
    {
        return build_filename(
              DATA_DIR
            , build_filename("icons"
                    , build_filename("hicolor"
                            , build_filename( s, "stock"
        ))));
    }

    void
    register_default_stock_icons()
    {
        StockIconSpecV v; 

        v.push_back(StockIconSpec( "icon-plugins.png",             MPX_STOCK_PLUGIN            ));
        v.push_back(StockIconSpec( "icon-preferences.png",         "mpx-stock-preferences"     ));
        v.push_back(StockIconSpec( "icon-musiclibrary.png",        "mpx-stock-musiclibrary"    ));
        v.push_back(StockIconSpec( "lastfm.png",		   "mpx-stock-lastfm"	       ));
        v.push_back(StockIconSpec( "turntable.png",		   "mpx-stock-youkidj"	       ));
        register_stock_icons (v, 24, default_stock_path("24x24")); 

        v.clear();
        v.push_back(StockIconSpec( "clear.png",                    "mpx-stock-entry-clear"     ));
        v.push_back(StockIconSpec( "deadend.png",                  "mpx-stock-dead-end"        ));
        v.push_back(StockIconSpec( "icon-add.png",                 "mpx-stock-add"             ));
        v.push_back(StockIconSpec( "icon-error.png",               MPX_STOCK_ERROR             ));
        v.push_back(StockIconSpec( "heart-black.png",              "mpx-loved-no"              ));
        v.push_back(StockIconSpec( "heart-redblack.png",           "mpx-loved-none"            ));
        v.push_back(StockIconSpec( "heart-red.png",                "mpx-loved-yes"             ));
        v.push_back(StockIconSpec( "icon-plugins.png",             MPX_STOCK_PLUGIN            ));
        v.push_back(StockIconSpec( "icon-musiclibrary.png",        "mpx-stock-musiclibrary"    ));
        v.push_back(StockIconSpec( "icon-preferences.png",         "mpx-stock-preferences"     ));
        register_stock_icons (v, 16, default_stock_path("16x16")); 
    }
}
