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

#ifndef _YOUKI_TRISTATE_BUTTON__HH
#define _YOUKI_TOGGUEBUTTON__HH

#include <glibmm/timer.h>
#include <gdkmm/pixbuf.h>
#include <gtkmm/button.h>
#include <sigc++/connection.h>

namespace MPX
{
    enum TristateButtonState
    {
          TRISTATE_BUTTON_STATE_ON
        , TRISTATE_BUTTON_STATE_OFF
        , TRISTATE_BUTTON_STATE_NONE

        , N_TRISTATE_BUTTON_STATES
    } ;

    class YoukiTristateButton

        : public Gtk::Button

    {
        public:

            YoukiTristateButton(
                  int
                , const std::string&
                , const std::string&
                , const std::string&
            ) ;

            void
            set_state(
                  TristateButtonState
            ) ;

            void
            set_default_state(
                  TristateButtonState
            ) ;

            TristateButtonState
            get_state(
            ) ;

        protected:

            virtual void
            on_clicked();

            virtual bool
            on_expose_event(
                  GdkEventExpose*
            ) ;

        protected:

            void
            draw_frame ();

            int                         m_pixbuf_size;
            TristateButtonState         m_state ;
            TristateButtonState         m_default_state ;

            Glib::RefPtr<Gdk::Pixbuf>   m_pixbuf[N_TRISTATE_BUTTON_STATES] ;
    } ;
} // MPX

#endif // _YOUKI_TRISTATE_BUTTON__HH

