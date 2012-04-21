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

#ifndef _YOUKI_SIMPLEINFO__HH
#define _YOUKI_SIMPLEINFO__HH

#include <glibmm/timer.h>
#include <gdkmm/pixbuf.h>
#include <gtkmm/drawingarea.h>
#include <sigc++/connection.h>

namespace MPX
{
  class YoukiSimpleInfo : public Gtk::DrawingArea
  {
    public:

        YoukiSimpleInfo ();

        void
        set_info(
            const std::string& 
        ) ;

        void
        clear () ;
  
    protected:

        virtual bool
        on_expose_event (GdkEventExpose *event);

    private:

        std::string m_info ;

        void
        draw_frame ();
  };
} // MPX

#endif // _YOUKI_SIMPLEINFO__HH

