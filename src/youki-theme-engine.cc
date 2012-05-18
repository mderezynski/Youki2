//  Youki (MPX)
//  Copyright (C) 2006-2012 Youki developers 
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

        color.set_rgba (int_from_hexbyte(s_r) / 255.,
                        int_from_hexbyte(s_g) / 255.,
                        int_from_hexbyte(s_b) / 255.,
                        int_from_hexbyte(s_a) / 255.);
    }

    using namespace MPX ;

    ThemeColor
    theme_color_from_gdk( const Gdk::RGBA& gdk )
    {
        return gdk;
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
            , { "treelines", THEME_COLOR_TREELINES }
            , { "info-area", THEME_COLOR_INFO_AREA }
            , { "volume", THEME_COLOR_VOLUME }
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

        ThemeColorMap_t colors ;

        Gdk::RGBA csel, cbase, cbg, ctext, ctext_sel ;

	tv.get_style_context()->lookup_color("base_color", cbase ) ;
	tv.get_style_context()->lookup_color("bg_color", cbg ) ;
	tv.get_style_context()->lookup_color("selected_bg_color", csel ) ;
	tv.get_style_context()->lookup_color("text_color", ctext ) ;
	tv.get_style_context()->lookup_color("selected_fg_color", ctext_sel ) ;

	Util::color_to_hsb(csel, h, s, b ) ;
	s *= 0.15 ;
	b = 0.95 ; 
	Gdk::RGBA csel_mod = Util::color_from_hsb( h, s, b ) ;
	csel_mod.set_alpha(1.) ;
	colors[THEME_COLOR_BASE_ALTERNATE] = csel_mod ; 
	

        colors[THEME_COLOR_BACKGROUND] = Util::make_rgba( cbg, 1. ) ;
        colors[THEME_COLOR_BASE]       = Util::make_rgba( cbase, 1. ) ; 
	//colors[THEME_COLOR_BASE_ALTERNATE] = Util::make_rgba( 0.98, 0.98, 0.98, 1. ) ;
        colors[THEME_COLOR_TEXT]          = Util::make_rgba( ctext, 1. ) ; 
        colors[THEME_COLOR_TEXT_SELECTED] = Util::make_rgba( ctext_sel, 1. ) ; 
        colors[THEME_COLOR_SELECT]     = Util::make_rgba( csel, 1. ) ;
        colors[THEME_COLOR_TREELINES]   = Util::make_rgba( .5, .5, .5, 1. ) ;
        colors[THEME_COLOR_INFO_AREA]   = tv.get_style_context()->get_background_color( Gtk::STATE_FLAG_NORMAL ) ;
        colors[THEME_COLOR_VOLUME]      = Util::make_rgba( .7, .7, .7, 1. ) ;

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
          const Cairo::RefPtr<Cairo::Context>&  cairo
        , const GdkRectangle&			r
        , bool					focused
        , double				rounding
	, MPX::CairoCorners::CORNERS		corners
    )
    {
        const ThemeColor& cgdk = get_color( THEME_COLOR_SELECT ) ;

        double alpha = focused ? 1. : .7 ;
        double h, s, b ;
        Util::color_to_hsb( cgdk, h, s, b ) ;
        b *= 0.90 ; 
        Gdk::RGBA c1 = Util::color_from_hsb( h, s, b ) ;

        Util::color_to_hsb( cgdk, h, s, b ) ;
        b *= 0.50 ; 
	s *= 0.95 ;
        Gdk::RGBA c2 = Util::color_from_hsb( h, s, b ) ;

        Util::color_to_hsb( cgdk, h, s, b ) ;
        b *= 0.40 ; 
	s *= 0.85 ;
        Gdk::RGBA c3 = Util::color_from_hsb( h, s, b ) ;

	cairo->save() ;

	cairo->rectangle( r.x, r.y, r.width, r.height ) ;
	cairo->clip() ;

        cairo->set_operator( Cairo::OPERATOR_OVER ) ;

	if( r.height > 40 )
	{
	    Cairo::RefPtr<Cairo::LinearGradient> gradient = Cairo::LinearGradient::create(
		  r.x + (r.width/2.) - (r.height/15.) 
		, r.y  
		, r.x + (r.width/2.) + (r.height/15.) 
		, r.y + r.height
	    ) ;

	    gradient->add_color_stop_rgba(
		  0
		, c1.get_red()
		, c1.get_green()
		, c1.get_blue()
		, alpha
	    ) ;
	   
	    gradient->add_color_stop_rgba(
		  0.70 
		, c2.get_red()
		, c2.get_green()
		, c2.get_blue()
		, alpha
	    ) ;

	    gradient->add_color_stop_rgba(
		  1. 
		, c3.get_red()
		, c3.get_green()
		, c3.get_blue()
		, alpha
	    ) ;

	    cairo->set_source( gradient ) ;
	}
	else
	{
	    Gdk::Cairo::set_source_rgba( cairo, Util::make_rgba(cgdk, alpha)) ;
	}

	RoundedRectangle(
	      cairo
	    , r.x 
	    , r.y 
	    , r.width 
	    , r.height 
	    , rounding 
	    , corners
	) ;

        cairo->fill_preserve(); 

	Gdk::Cairo::set_source_rgba( cairo, Util::make_rgba(c3,alpha)) ;
        cairo->set_line_width( 0.5 ) ; 
        cairo->stroke () ;

	cairo->restore() ;
    }

    void
    YoukiThemeEngine::draw_focus(
          const Cairo::RefPtr<Cairo::Context>&	cairo
        , const GdkRectangle&			r
        , bool					focused
        , double				rounding
	, MPX::CairoCorners::CORNERS		corners
    )
    {
        double h, s, b ;

        const ThemeColor& c = get_color( THEME_COLOR_SELECT ) ;

        Gdk::RGBA cgdk ;
        cgdk.set_rgba( c.get_red(), c.get_green(), c.get_blue() ) ;

        Util::color_to_hsb( cgdk, h, s, b ) ;
        b *= 0.70 ; 
        s *= 0.50 ; 
        Gdk::RGBA c1 = Util::color_from_hsb( h, s, b ) ;

        cairo->save() ;
        cairo->set_operator( Cairo::OPERATOR_OVER ) ;
        cairo->set_source_rgba(
              c1.get_red() 
            , c1.get_green()
            , c1.get_blue()
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
        dashes[0] = 1. ;
        dashes[1] = 1. ;

        cairo->set_dash( dashes, 0 ) ;
        cairo->set_line_width( 1 ) ; 
	cairo->set_antialias( Cairo::ANTIALIAS_NONE );
        cairo->stroke () ;
        cairo->restore() ;
    }
}
