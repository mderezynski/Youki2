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

#ifndef MPX_MMKEYS_HH
#define MPX_MMKEYS_HH

#include "config.h"

#include "mpx/mpx-services.hh"
#include "mpx/widgets/widgetloader.hh"

#include <set>
#include <string>
#include <vector>

#include <glibmm/ustring.h>
#include <gdkmm/pixbuf.h>
#include <gtkmm.h>
#include <mcs/mcs.h>
#include <mcs/gtk-bind.h>
#include "mpx/plugin-types.hh"

namespace MPX
{
  class MMKeys
  : public Gnome::Glade::WidgetLoader<Gtk::VBox>
  , public PluginHolderBase
  {
    public:

        MMKeys(
              const Glib::RefPtr<Gnome::Glade::Xml>&
            , gint64
        ) ;

        static MMKeys*
        create(gint64);

        virtual
        ~MMKeys();

        virtual bool
        activate()
        {
            mmkeys_activate() ;
            mm_toggled( true ) ;
            return true ;
        }
    
        virtual bool
        deactivate()
        { 
            mmkeys_deactivate() ;
            mm_toggled( false ) ;
            return true ;
        }

        virtual Gtk::Widget*
        get_gui()
        {
            return this ; 
        }

    private:

        bool m_active ;

        struct KeyControls
        {
          gint key, mask;
          KeyControls () : key(0), mask(0) {}
        };

        std::vector<KeyControls> m_mm_key_controls;

        int m_mm_option;

        void
        set_keytext(
              gint
            , gint
            , gint
        ) ;

        bool
        on_entry_key_press_event(
              GdkEventKey*
            , gint
        ) ;

        bool
        on_entry_key_release_event(
              GdkEventKey*
            , gint
        ) ;

        void
        on_clear_keyboard(
              gint
        ) ;

        void
        on_mm_option_changed(
              gint
        ) ;

        void
        mm_load ();

        void
        mm_apply ();

        void
        mm_toggled(
            bool
        ) ;

    private:

        enum grab_type
        {
          NONE = 0,
          SETTINGS_DAEMON,
          X_KEY_GRAB
        };

        DBusGProxy * m_mmkeys_dbusproxy;
        grab_type m_mmkeys_grab_type;

        static void
        media_player_key_pressed (DBusGProxy *proxy,
                                  const gchar *application,
                                  const gchar *key,
                                  gpointer data);

        bool
        window_focus_cb (GdkEventFocus *event);

        void grab_mmkey (int key_code,
                         int mask,
                         GdkWindow *root);

        void grab_mmkey (int key_code,
                         GdkWindow *root);

        void ungrab_mmkeys (GdkWindow *root);

        static GdkFilterReturn
        filter_mmkeys (GdkXEvent *xevent,
                       GdkEvent *event,
                       gpointer data);
        void
        mmkeys_grab (bool grab);

        void
        mmkeys_get_offending_modifiers ();

        guint m_capslock_mask, m_numlock_mask, m_scrolllock_mask;

        void
        mmkeys_activate ();

        void 
        mmkeys_deactivate ();

        void
        on_mm_edit_begin (); 

        void
        on_mm_edit_done (); 

        sigc::connection mWindowFocusConn;

  }; // class MMKeys
} // namespace MPX


extern "C" MPX::PluginHolderBase*
get_instance(gint64 id)
{
    return MPX::MMKeys::create( id ) ;
}

extern "C" bool
del_instance(MPX::PluginHolderBase* b)
{
    delete dynamic_cast<MPX::MMKeys*>(b) ;
    return true ;
}

#endif // MPX_MMKEYS_HH
