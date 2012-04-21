//  MPX
//  Copyright (C) 2010 MPX development team.
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
#include <boost/algorithm/string.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "json/json.h"

#include "mpx/util-graphics.hh"
#include "mpx/widgets/cairo-extensions.hh"

#include "mpx/mpx-main.hh"

#include "youki-theme-engine.hh"

namespace
{
    int int_from_hexbyte( const std::string& hex )
    {
        int value = 0;
        
        int a = 0;
        int b = hex.length() - 1;

        for (; b >= 0; a++, b--)
        {
            if (hex[b] >= '0' && hex[b] <= '9')
            {
                value += (hex[b] - '0') * (1 << (a * 4));
            }
            else
            {
                switch (hex[b])
                {
                    case 'A':
                    case 'a':
                        value += 10 * (1 << (a * 4));
                        break;
                        
                    case 'B':
                    case 'b':
                        value += 11 * (1 << (a * 4));
                        break;
                        
                    case 'C':
                    case 'c':
                        value += 12 * (1 << (a * 4));
                        break;
                        
                    case 'D':
                    case 'd':
                        value += 13 * (1 << (a * 4));
                        break;
                        
                    case 'E':
                    case 'e':
                        value += 14 * (1 << (a * 4));
                        break;
                        
                    case 'F':
                    case 'f':
                        value += 15 * (1 << (a * 4));
                        break;
                        
                    default:
                        g_message("%s: Error, invalid character '%c' in hex number", G_STRLOC, hex[a] ) ; 
                        break;
                }
            }
        }
        
        return value;
    }
     
    void hex_to_color(
          const std::string&  hex
        , MPX::ThemeColor&    color 
    )
    {
        std::string s_r = hex.substr(1, 2);
        std::string s_g = hex.substr(3, 2);
        std::string s_b = hex.substr(5, 2);
        std::string s_a = hex.substr(7, 2);

        color.r = int_from_hexbyte(s_r) / 255. ;
        color.g = int_from_hexbyte(s_g) / 255. ;
        color.b = int_from_hexbyte(s_b) / 255. ;
        color.a = int_from_hexbyte(s_a) / 255. ;
    }

    using namespace MPX ;

    ThemeColor
    theme_color_from_gdk( const Gdk::Color& gdk )
    {
        ThemeColor c ;
    
        c.r = gdk.get_red_p() ;
        c.g = gdk.get_green_p() ;
        c.b = gdk.get_blue_p() ;
        c.a = 1 ;

        return c ;
    }
}

namespace MPX
{
    YoukiThemeEngine::YoukiThemeEngine(
    )
    : Service::Base( "mpx-service-theme" )
    {
        reload() ;
    }

    void
    YoukiThemeEngine::load_theme(
          const std::string&    name
        , const std::string&    document
    )
    {
        Json::Value root ;
        Json::Reader reader ;

        reader.parse(
              document
            , root
            , false
        ) ;

        if( !root.isArray() )
        {
            g_warning("%s: Theme root node is not array! ('%s')", G_STRLOC, name.c_str() ) ;
            return ;
        }
    
        struct ColorNameToID
        {
            std::string     Color ;
            ThemeColorID    ID ;
        } ;

        ColorNameToID color_to_id[] =
        {
              { "background", THEME_COLOR_BACKGROUND }
            , { "base", THEME_COLOR_BASE }
            , { "base-alternate", THEME_COLOR_BASE_ALTERNATE }
            , { "text", THEME_COLOR_TEXT }
            , { "text-selected", THEME_COLOR_TEXT_SELECTED }
            , { "select", THEME_COLOR_SELECT }
            , { "drawer", THEME_COLOR_DRAWER }
            , { "titlebar-1", THEME_COLOR_TITLEBAR_1 }
            , { "titlebar-2", THEME_COLOR_TITLEBAR_2 }
            , { "titlebar-3", THEME_COLOR_TITLEBAR_3 }
            , { "titlebar-top", THEME_COLOR_TITLEBAR_TOP } 
            , { "window-border", THEME_COLOR_WINDOW_BORDER }
            , { "entry-outline", THEME_COLOR_ENTRY_OUTLINE }
            , { "treelines", THEME_COLOR_TREELINES }
            , { "info-area", THEME_COLOR_INFO_AREA }
            , { "volume", THEME_COLOR_VOLUME }
            , { "resize-grip", THEME_COLOR_RESIZE_GRIP }
        } ;

        typedef std::map<std::string, ThemeColorID> NameToIDMap ;

        NameToIDMap n_to_id ;
        for( unsigned int n = 0 ; n < G_N_ELEMENTS(color_to_id); ++n ) 
        {
            n_to_id.insert( std::make_pair( color_to_id[n].Color, color_to_id[n].ID )) ;
        }

        ThemeColorMap_t colors ;

        for( Json::Value::iterator i = root.begin() ; i != root.end() ; ++i )
        {
            Json::Value::Members m = (*i).getMemberNames() ;

            Json::Value   val_default ;

            Json::Value   val_name
                        , val_colr ;

            val_name = (*i).get( "name"  , val_default ) ;
            val_colr = (*i).get( "value" , val_default ) ;

            std::string name = val_name.asString() ;

            if( n_to_id.count( name ))
            {
                ThemeColor color ;

                hex_to_color(
                      val_colr.asString()
                    , color
                ) ;

                ThemeColorID id = n_to_id[name] ;

                colors[id] = color ;
            }
        }

        Theme theme (
              name 
            , ""
            , "" 
            , "" 
            , "" 
            , colors 
        ) ;

        m_Themes[name] = theme ;
    }

    void
    YoukiThemeEngine::load_stored_themes()
    {
        std::string path = Glib::build_filename( DATA_DIR, "themes" ) ;
        Glib::Dir d ( path ) ;
        std::vector<std::string> s (d.begin(), d.end()) ;
        d.close() ;

        for( std::vector<std::string>::const_iterator i = s.begin(); i != s.end(); ++i )
        {
            std::vector<std::string> subs;
            boost::split( subs, *i, boost::is_any_of (".") ) ;

            if( !subs.empty() )
            {
                std::string suffix = subs[subs.size()-1] ;
    
                if( suffix == "mpxtheme" )
                {
                    std::string document = Glib::file_get_contents( Glib::build_filename( path, *i )) ;
                    std::string name     = subs[0] ;

                    load_theme( name, document ) ;
                }
            }
        }
    }

    void
    YoukiThemeEngine::reload(
    )
    {
        // FIXME for now, we have one default theme engine
        double h, s, b ;

        Gtk::Window tv ;
        gtk_widget_realize(GTK_WIDGET(tv.gobj())) ;

        Gdk::Color csel = tv.get_style()->get_base( Gtk::STATE_SELECTED ) ;
	// csel.set_rgb_p( 0x53/255., 0x7f/255., 0xe9/255. ) ; 

        ThemeColorMap_t colors ;
        colors[THEME_COLOR_SELECT] = ThemeColor( csel.get_red_p(), csel.get_green_p(), csel.get_blue_p(), 1. ) ;

	Gdk::Color ctit ;
	ctit.set_rgb_p( 0x6d/255., 0x9c/255., 0xe9/255. ) ;

        Util::color_to_hsb( ctit, h, s, b ) ;
        b = std::max( 0.20, b-0.01 ) ;
        s = std::max( 0.20, s-0.02 ) ;
        Gdk::Color c0 = Util::color_from_hsb( h, s, b ) ;
        colors[THEME_COLOR_TITLEBAR_1] = ThemeColor( c0.get_red_p(), c0.get_green_p(), c0.get_blue_p(), 0.93 ) ;

        Util::color_to_hsb( ctit, h, s, b ) ;
        b = std::max( 0.12, b-0.02 ) ;
        s = std::max( 0.13, s-0.04 ) ;
        Gdk::Color c1 = Util::color_from_hsb( h, s, b ) ;
        colors[THEME_COLOR_TITLEBAR_2] = ThemeColor( c1.get_red_p(), c1.get_green_p(), c1.get_blue_p(), 0.93 ) ;

        Util::color_to_hsb( ctit, h, s, b ) ;
        b = std::max( 0.05, b-0.04 ) ;
        s = std::max( 0.09, s-0.06 ) ;
        Gdk::Color c2 = Util::color_from_hsb( h, s, b ) ;
        colors[THEME_COLOR_TITLEBAR_3] = ThemeColor( c2.get_red_p(), c2.get_green_p(), c2.get_blue_p(), 0.93 ) ;

        Util::color_to_hsb( ctit, h, s, b ) ;
        b = std::max( 0.21, b-0.02 ) ;
        s = std::max( 0.22, s-0.10 ) ;
        Gdk::Color c3 = Util::color_from_hsb( h, s, b ) ;
        colors[THEME_COLOR_TITLEBAR_TOP] = ThemeColor( c3.get_red_p(), c3.get_green_p(), c3.get_blue_p(), 0.90 ) ; 

        colors[THEME_COLOR_BACKGROUND] = theme_color_from_gdk(  tv.get_style()->get_bg( Gtk::STATE_NORMAL )) ;
        colors[THEME_COLOR_BASE] = theme_color_from_gdk( tv.get_style()->get_base( Gtk::STATE_NORMAL )) ;

        Util::color_to_hsb( tv.get_style()->get_base( Gtk::STATE_NORMAL ), h, s, b ) ;
	b *= 0.95 ;
        colors[THEME_COLOR_BASE_ALTERNATE] = theme_color_from_gdk( Util::color_from_hsb ( h, s, b )) ; 

        colors[THEME_COLOR_TEXT] = theme_color_from_gdk(  tv.get_style()->get_text( Gtk::STATE_NORMAL )) ;
        colors[THEME_COLOR_TEXT_SELECTED] = theme_color_from_gdk(  tv.get_style()->get_fg( Gtk::STATE_SELECTED )) ;
        colors[THEME_COLOR_DRAWER] = ThemeColor( 0.65, 0.65, 0.65, .4 ) ;
        colors[THEME_COLOR_WINDOW_BORDER] = ThemeColor( 0.25, 0.25, 0.25, 1. ) ; 
        colors[THEME_COLOR_ENTRY_OUTLINE] = ThemeColor( 0.2, 0.2, 0.2, 1. ) ; 

        colors[THEME_COLOR_TREELINES] = ThemeColor( .5, .5, .5, 1. ) ; 
        colors[THEME_COLOR_INFO_AREA] = theme_color_from_gdk( tv.get_style()->get_base( Gtk::STATE_NORMAL )) ;
        colors[THEME_COLOR_VOLUME] = ThemeColor( .7, .7, .7, 1. ) ;
        colors[THEME_COLOR_RESIZE_GRIP] = ThemeColor( 1., 1., 1., .10 ) ; 

        Theme theme (
              "Youki-Default"
            , "Milosz Derezynski"
            , "(C) 2009 Youki Project"
            , "This is Youki's default theme"
            , "http://youki.mp"
            , colors 
        ) ;

        m_Themes.erase("default") ;
        m_Themes["default"] = theme ;

        load_stored_themes() ;

        m_CurrentTheme = m_Themes.find( "default" ) ;
    }
    
    YoukiThemeEngine::~YoukiThemeEngine(
    )
    {
    }

    std::vector<std::string>
    YoukiThemeEngine::list_themes(
    )
    {
        return std::vector<std::string>( 1, "default" ) ;
    }

    void
    YoukiThemeEngine::load_theme(
          const std::string&    theme
    )
    {
    }

    const ThemeColor& 
    YoukiThemeEngine::get_color(
          ThemeColorID          color
    ) 
    {
        return m_CurrentTheme->second.Colors[color] ;
    }

    //// DRAWING FUNCTIONS

    void
    YoukiThemeEngine::draw_selection_rectangle(
          Cairo::RefPtr<Cairo::Context>&    cairo
        , const GdkRectangle&               r
        , bool                              sensitive
        , double                            rounding
	, MPX::CairoCorners::CORNERS	    corners
    )
    {
        const ThemeColor& c = get_color( THEME_COLOR_SELECT ) ;

        Gdk::Color cgdk ;
        cgdk.set_rgb_p( c.r, c.g, c.b ) ;

        cairo->set_operator( Cairo::OPERATOR_OVER ) ;

        Cairo::RefPtr<Cairo::LinearGradient> gradient = Cairo::LinearGradient::create(
              r.x + r.width / 2
            , r.y  
            , r.x + r.width / 2
            , r.y + r.height
        ) ;

        double alpha = sensitive ? 1. : .8 ;
        
        double h, s, b ;
        
        Util::color_to_hsb( cgdk, h, s, b ) ;
        b *= 0.90 ; 
        Gdk::Color c1 = Util::color_from_hsb( h, s, b ) ;

        Util::color_to_hsb( cgdk, h, s, b ) ;
        b *= 0.50 ; 
	s *= 0.95 ;
        Gdk::Color c2 = Util::color_from_hsb( h, s, b ) ;

        gradient->add_color_stop_rgba(
              0
            , c1.get_red_p()
            , c1.get_green_p()
            , c1.get_blue_p()
            , alpha
        ) ;
       
        gradient->add_color_stop_rgba(
              1. 
            , c2.get_red_p()
            , c2.get_green_p()
            , c2.get_blue_p()
            , alpha
        ) ;

        RoundedRectangle(
              cairo
            , r.x 
            , r.y 
            , r.width 
            , r.height 
            , rounding 
	    , corners
        ) ;
        cairo->set_source( gradient ) ;
        cairo->fill(); 

/*
        cairo->set_source_rgba(
              c.r
            , c.g
            , c.b
            , .95
        ) ;
        cairo->set_line_width( 1.25 ) ; 
        cairo->stroke () ;
*/
    }

    void
    YoukiThemeEngine::draw_focus(
          Cairo::RefPtr<Cairo::Context>&    cairo
        , const GdkRectangle&               r
        , bool                              sensitive
        , double                            rounding
	, MPX::CairoCorners::CORNERS	    corners
    )
    {
        double h, s, b ;

        const ThemeColor& c = get_color( THEME_COLOR_SELECT ) ;

        Gdk::Color cgdk ;
        cgdk.set_rgb_p( c.r, c.g, c.b ) ;

        Util::color_to_hsb( cgdk, h, s, b ) ;
        b = std::min( 1., b+0.04 ) ;
        s = std::min( 1., s+0.05 ) ;
        Gdk::Color c1 = Util::color_from_hsb( h, s, b ) ;

        cairo->save() ;
        cairo->set_operator( Cairo::OPERATOR_OVER ) ;
        cairo->set_source_rgba(
              c1.get_red_p() 
            , c1.get_green_p()
            , c1.get_blue_p()
            , 1. 
        ) ;
        RoundedRectangle(
              cairo
            , r.x
            , r.y
            , r.width
            , r.height
            , rounding
	    , corners
        ) ;

        std::valarray<double> dashes ( 2 ) ;
        dashes[0] = 1.5 ;
        dashes[1] = 1. ;

        cairo->set_dash( dashes, 0 ) ;
        cairo->set_line_width( 1.75 ) ; 
        cairo->stroke () ;
        cairo->restore() ;
    }
}
