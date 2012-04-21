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

#ifndef MPX_UI_VOLUME_CONTROL_HH
#define MPX_UI_VOLUME_CONTROL_HH

#include <gtkmm.h>
#include <libglademm.h>

namespace MPX
{
  class VolumeControl
  {
    public:

        typedef sigc::signal<void, double> SignalValueChanged;

        VolumeControl(const Glib::RefPtr<Gnome::Glade::Xml>&);

        virtual ~VolumeControl () {}

        void
        set_volume( int );

        SignalValueChanged&
        signal_value_changed()
        {
            return ValueChanged;
        }

    private:

        SignalValueChanged  ValueChanged;

        Gtk::VScale*        m_volume_range;
        Gtk::Window*        m_volume_window;
        Gtk::Button*        m_volume_toggle;
        Gtk::Image*         m_volume_image;

        Glib::RefPtr<Gdk::Pixbuf> m_volume_pixbufs[4];

        Glib::Timer         m_keypress_timer;
        sigc::connection    m_hide_conn;
        int                 m_inside;


        bool
        volume_window_button_press_event_cb(GdkEventButton*);

        bool
        volume_window_enter_notify_event_cb(GdkEventCrossing*);

        bool
        volume_window_leave_notify_event_cb(GdkEventCrossing*);



        void
        volume_toggle_clicked_cb();

        bool
        volume_toggle_key_press_event_cb(GdkEventKey*);

        bool
        volume_toggle_scroll_event_cb(GdkEventScroll*);



        bool
        volume_range_focus_out_event_cb(GdkEventFocus*);

        bool
        volume_range_button_press_cb(GdkEventButton*);

        bool
        volume_range_button_release_cb(GdkEventButton*);

        void
        volume_range_value_changed_cb();



        void
        volume_hide_timeout_readd();

        void
        volume_hide_real ();

        bool
        volume_hide ();

        void
        volume_show ();

    
        void
        adapt_image ();
  };
} // MPX

#endif // !MPX_UI_VOLUME_CONTROL_HH
