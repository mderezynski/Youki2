//  MPX
//  Copyright (C) 2005-2007 MPX development.
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
//  The MPX project hereby grants permission for non GPL-compatible GStreamer
//  plugins to be used and distributed together with GStreamer and MPX. This
//  permission is above and beyond the permissions granted by the GPL license
//  MPX is covered by.

#ifndef KOBO_TITLEINFO_HH
#define KOBO_TITLEINFO_HH

#include <gtkmm.h>
#include <glibmm/timer.h>
#include <gdkmm/pixbuf.h>
#include <gtkmm/drawingarea.h>
#include <sigc++/connection.h>
#include "mpx/algorithm/modulo.hh"
#include "mpx/i-youki-theme-engine.hh"
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>

namespace MPX
{
  typedef boost::optional<Gdk::RGBA> Color_opt_t ;

  enum TapArea
  {
      TAP_LEFT
    , TAP_CENTER
    , TAP_RIGHT
  };

  class KoboTitleInfo : public Gtk::DrawingArea
  {
    public:

        KoboTitleInfo ();

        void
        set_info(
            const std::vector<std::string>&
        ) ;

        void
        set_cover(
              Glib::RefPtr<Gdk::Pixbuf>
        ) ;

        void
        clear() ;
  
	sigc::signal<void, int>&
	signal_tapped()
	{
	  return m_SIGNAL__area_tapped ;
	}

	void
	set_color(
	      const Color_opt_t& v = Color_opt_t()
	)
	{
	    m_color = v ;
	    queue_draw() ;
	}

    protected:

	sigc::signal<void, int>	m_SIGNAL__area_tapped ;

	Color_opt_t m_color ;

        virtual bool
        on_draw(const Cairo::RefPtr<Cairo::Context>&) ;

	bool
	on_button_press_event(
	    GdkEventButton* event 
	) ;

    private:

        double total_animation_time ;
        double start_time ;
        double end_time ; 

        std::vector<std::string>    m_info ;
	Glib::RefPtr<Gdk::Pixbuf>   m_cover ;

        sigc::connection            m_update_connection;

        Glib::Timer                 m_timer;
        Modulo<double>              m_tmod ;
        double                      m_current_time ;

        boost::shared_ptr<IYoukiThemeEngine>  m_theme ;

        bool
        update_frame ();

        double
        cos_smooth (double x) ;

	guint
        get_text_at_time(std::string&) ;

        double
        get_text_alpha_at_time() ;
  };

} // MPX

#endif // !MPX_UI_DIALOG_ABOUT_HH
