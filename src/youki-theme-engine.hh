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

#ifndef _YOUKI_THEME_ENGINE__HH
#define _YOUKI_THEME_ENGINE__HH

#include "config.h"

#include <map>
#include <vector>
#include <string>

#include "mpx/mpx-services.hh"
#include "mpx/i-youki-theme-engine.hh"
#include "mpx/widgets/cairo-extensions.hh"

namespace MPX
{
    class YoukiThemeEngine
    : public IYoukiThemeEngine
    , public Service::Base
    {
        protected:

            Theme_map_t             m_Themes ;
            Theme_map_t::iterator   m_CurrentTheme ;

        public:

            YoukiThemeEngine () ; 
            virtual ~YoukiThemeEngine () ; 

	    void
	    set_select_color() ;

	    void
	    set_select_color(Gdk::RGBA) ;

            //// DRAWING FUNCTIONS

            void
            draw_selection_rectangle(
                  const Cairo::RefPtr<Cairo::Context>&          /*cairo ctx*/
                , const GdkRectangle&                           /*rectangle*/
                , bool                                          /*sensitive*/
                , double = 4.                                   /*rounding*/
		, MPX::CairoCorners::CORNERS = MPX::CairoCorners::ALL
            ) ;

            void
            draw_focus(
                  const Cairo::RefPtr<Cairo::Context>&          /*cairo ctx*/
                , const GdkRectangle&                           /*rectangle*/
                , bool                                          /*sensitive*/
                , double = 4.                                   /*rounding*/
		, MPX::CairoCorners::CORNERS = MPX::CairoCorners::ALL
            ) ;

            //// PUBLIC

            std::vector<std::string>
            list_themes() ;

            void
            load_theme(
                  const std::string&
            ) ;

            const ThemeColor& 
            get_color(
                  ThemeColorID
            ) ; 

            void
            reload(
            ) ;

            void
            load_stored_themes(
            ) ;

            void
            load_theme(
                  const std::string&    /*name*/
                , const std::string&    /*document*/
            ) ;
    } ;
}

#endif // _YOUKI_THEME_ENGINE__INTERFACE__HH

