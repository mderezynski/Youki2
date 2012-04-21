//  BMPx - The Dumb Music IPlayer
//  Copyright (C) 2005-2007 BMPx development team.
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
//  The BMPx project hereby grants permission for non-GPL compatible GStreamer
//  plugins to be used and distributed together with GStreamer and BMPx. This
//  permission is above and beyond the permissions granted by the GPL license
//  BMPx is covered by.

#ifndef _YOUKI_THEME_ENGINE__INTERFACE__HH
#define _YOUKI_THEME_ENGINE__INTERFACE__HH

#include "config.h"

#include <cairomm/cairomm.h>
#include <gdk/gdk.h>

#include <map>
#include <vector>
#include <string>

#include "mpx/widgets/cairo-extensions.hh"

namespace MPX
{
    enum ThemeColorID
    {
          THEME_COLOR_BACKGROUND
        , THEME_COLOR_BASE
        , THEME_COLOR_BASE_ALTERNATE
        , THEME_COLOR_TEXT
        , THEME_COLOR_TEXT_SELECTED
        , THEME_COLOR_SELECT
        , THEME_COLOR_DRAWER
        , THEME_COLOR_TITLEBAR_1 
        , THEME_COLOR_TITLEBAR_2 
        , THEME_COLOR_TITLEBAR_3 
        , THEME_COLOR_TITLEBAR_TOP 
        , THEME_COLOR_WINDOW_BORDER
        , THEME_COLOR_ENTRY_OUTLINE
        , THEME_COLOR_TREELINES
        , THEME_COLOR_INFO_AREA
        , THEME_COLOR_VOLUME
        , THEME_COLOR_RESIZE_GRIP
    } ;

    struct ThemeColor
    {
        double r ;
        double g ;
        double b ;
        double a ;

        ThemeColor(
        )
        {}

        ThemeColor(
              double r_
            , double g_
            , double b_
            , double a_
        )
            : r( r_ ) 
            , g( g_ )
            , b( b_ )
            , a( a_ )
        {}
    } ;

    typedef std::map<ThemeColorID, ThemeColor>  ThemeColorMap_t ;

    struct Theme
    {
            std::string         Name ;
            std::string         Authors ;
            std::string         Copyright ;
            std::string         Description ;
            std::string         WebLink ;
            ThemeColorMap_t     Colors ;

            Theme(
            )
            {}

            Theme(
                  const std::string&        name
                , const std::string&        authors
                , const std::string&        copyright
                , const std::string&        description
                , const std::string&        weblink 
                , const ThemeColorMap_t&    colors
            )
                : Name( name )
                , Authors( authors )
                , Copyright( copyright )
                , Description( description )
                , WebLink( weblink )
                , Colors( colors )
            {}
    } ;

    typedef std::map<std::string, Theme> Theme_map_t ;

    class IYoukiThemeEngine
    {
        public:

            IYoukiThemeEngine () {}
            virtual ~IYoukiThemeEngine () {}

            virtual void
            draw_selection_rectangle(
                  Cairo::RefPtr<Cairo::Context>&                /*cairo ctx*/
                , const GdkRectangle&                           /*rectangle*/
                , bool                                          /*sensitive*/
                , double = 4.
		, MPX::CairoCorners::CORNERS = MPX::CairoCorners::ALL
            ) = 0 ;

            virtual void
            draw_focus(
                  Cairo::RefPtr<Cairo::Context>&                /*cairo ctx*/
                , const GdkRectangle&                           /*rectangle*/
                , bool                                          /*sensitive*/
                , double = 4.
		, MPX::CairoCorners::CORNERS = MPX::CairoCorners::ALL
            ) = 0 ;

            virtual std::vector<std::string>
            list_themes() = 0 ;

            virtual void 
            load_theme(
                  const std::string&
            ) = 0 ;

            virtual const ThemeColor& 
            get_color(
                  ThemeColorID
            )  = 0 ;

            virtual void
            reload(
            ) = 0 ;
    } ;
}

#endif // _YOUKI_THEME_ENGINE__INTERFACE__HH

