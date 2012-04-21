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

#ifndef MPX_UI_DIALOG_ABOUT_HH
#define MPX_UI_DIALOG_ABOUT_HH

#include <glibmm/timer.h>
#include <gdkmm/pixbuf.h>
#include <gtkmm/window.h>
#include <sigc++/connection.h>

namespace MPX
{
  class AboutDialog : public Gtk::Window
  {
    public:

        AboutDialog ();
  
        virtual void present () { Gtk::Window::present(); }

    protected:

        virtual bool
        on_expose_event (GdkEventExpose *event);

        virtual bool
        on_key_press_event (GdkEventKey *event);

        virtual bool
        on_button_press_event (GdkEventButton *event);

        virtual bool
        on_delete_event (GdkEventAny *event);

        virtual void
        on_map ();

        virtual void
        on_unmap ();

    private:

        Glib::RefPtr<Gdk::Pixbuf>   m_background;
        Glib::Timer                 m_timer;
        sigc::connection            m_update_connection;

        void
        draw_frame ();

        bool
        update_frame ();
  };

} // MPX

#endif // !MPX_UI_DIALOG_ABOUT_HH
