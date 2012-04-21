//  MPX
//  Copyright (C) 2003-2007 MPX Development
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

#include "config.h"

#include <utility>
#include <iostream>

#include <glibmm.h>
#include <glibmm/i18n.h>
#include <gtkmm.h>
#include <gdk/gdkx.h>

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>

#include "mpx/mpx-main.hh" 
#include "mpx/i-youki-preferences.hh"
#include "mpx/util-string.hh"

#include "src/glib-marshalers.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/XF86keysym.h>

#include "mmkeys.hh"
#include "mpx/i-youki-controller.hh"

using namespace Glib;
using namespace Gtk;

namespace MPX
{
    //// MMKeys
    MMKeys*
    MMKeys::create(gint64 id)
    {
        return new MMKeys(
            Gnome::Glade::Xml::create(
                build_filename(
                      DATA_DIR
                    , "glade" G_DIR_SEPARATOR_S "cppmod-mmkeys.glade"
            ))
          , id
       );
    }

    MMKeys::~MMKeys ()
    {
    }

    MMKeys::MMKeys(
          const Glib::RefPtr<Gnome::Glade::Xml>&    xml
        , gint64                                    id
    )
        : Gnome::Glade::WidgetLoader<Gtk::VBox>(xml, "mmkeys-vbox")
        , m_active( false )
        , m_mmkeys_dbusproxy( 0 )
    {
        show() ;

        m_Name = "Multimedia Keys" ;
        m_Description = "This plugin allows Youki to be controlled using the keyboard's multimedia keys" ;
        m_Authors = "Rhythmbox developers (original code), M. Derezynski (adaptation for Youki)" ;
        m_Copyright = "(C) 2009 MPX Project" ;
        m_IAge = 0 ;
        m_Website = "http://redmine.sivashs.org/projects/mpx" ;
        m_Active = false ;
        m_HasGUI = true ;
        m_CanActivate = true ;
        m_Hidden = false ;
        m_Id = id ;

        boost::shared_ptr<IPreferences> p = services->get<IPreferences>("mpx-service-preferences") ;
    
        mcs->domain_register ("hotkeys");
        mcs->key_register ("hotkeys", "system", int (1)); // GNOME Configured is default
        mcs->key_register ("hotkeys", "key-1", int (0)); // Play
        mcs->key_register ("hotkeys", "key-1-mask", int (0));
        mcs->key_register ("hotkeys", "key-2", int (0)); // Pause
        mcs->key_register ("hotkeys", "key-2-mask", int (0));
        mcs->key_register ("hotkeys", "key-3", int (0)); // Prev
        mcs->key_register ("hotkeys", "key-3-mask", int (0));
        mcs->key_register ("hotkeys", "key-4", int (0)); // Next
        mcs->key_register ("hotkeys", "key-4-mask", int (0));
        mcs->key_register ("hotkeys", "key-5", int (0)); // Stop
        mcs->key_register ("hotkeys", "key-5-mask", int (0));

        // MM-Keys

        const int N_MM_KEYS = 4 ;
        const int N_MM_KEY_SYSTEMS = 3 ;

        for( int n = 1; n <= N_MM_KEYS; ++n)
        {
            Gtk::Widget * entry = m_Xml->get_widget((boost::format ("mm-entry-%d") % n).str());

            entry->signal_key_press_event().connect(
                sigc::bind(
                sigc::mem_fun(
                *this,
                &MMKeys::on_entry_key_press_event
                ),
                n
                ));

            entry->signal_key_release_event().connect(
                sigc::bind(
                sigc::mem_fun(
                *this,
                &MMKeys::on_entry_key_release_event
                ),
                n
                ));

            Gtk::Button * button = dynamic_cast<Gtk::Button*>(m_Xml->get_widget ((boost::format ("mm-clear-%d") % n).str()));

            button->signal_clicked().connect(
                sigc::bind(
                sigc::mem_fun(
                *this,
                &MMKeys::on_clear_keyboard
                ),
                n
                ));
        }

        m_mm_key_controls.resize( N_MM_KEYS );

        int active = mcs->key_get<int>("hotkeys","system") ;

        for( int n = 1; n <= N_MM_KEY_SYSTEMS; ++n )
        {
            Gtk::RadioButton * button = dynamic_cast<Gtk::RadioButton*>(m_Xml->get_widget ((boost::format ("mm-rb-%d") % n).str(), button));

            button->signal_toggled().connect(
                sigc::bind(
                sigc::mem_fun(
                *this,
                &MMKeys::on_mm_option_changed
                ),
                n
                ));

            if( n == (active+1) )
            {
                button->set_active ();
            }
        }

        dynamic_cast<Gtk::Button*>(m_Xml->get_widget ("mm-revert"))->signal_clicked().connect(
            sigc::mem_fun( *this, &MMKeys::mm_load ));
        dynamic_cast<Gtk::Button*>(m_Xml->get_widget ("mm-apply"))->signal_clicked().connect(
            sigc::mem_fun( *this, &MMKeys::mm_apply ));

        mm_load ();
    }

    /* mm-keys */
    void
        MMKeys::mm_toggled (
            bool active
        )
    {
        m_Xml->get_widget("mm-vbox")->set_sensitive( active );
        on_mm_edit_done() ;
    }

    void
        MMKeys::mm_apply ()
    {
        mcs->key_set<int>("hotkeys","system", m_mm_option);

        for( unsigned int n = 1; n <= 4; ++n )
        {
            KeyControls & c = m_mm_key_controls[n-1];
            mcs->key_set<int>("hotkeys", (boost::format ("key-%d") % n).str(), c.key);
            mcs->key_set<int>("hotkeys", (boost::format ("key-%d-mask") % n).str(), c.mask);
        }

        m_Xml->get_widget ("mm-apply")->set_sensitive (false);
        m_Xml->get_widget ("mm-revert")->set_sensitive (false);

        on_mm_edit_done() ;
    }

    void
        MMKeys::mm_load ()
    {
        for( unsigned int n = 1; n <= 4; ++n )
        {
            KeyControls c;
            c.key = mcs->key_get<int>("hotkeys", (boost::format ("key-%d") % n).str());
            c.mask = mcs->key_get<int>("hotkeys", (boost::format ("key-%d-mask") % n).str());
            m_mm_key_controls[n-1] = c;
            set_keytext( n, c.key, c.mask );
        }

        int sys = mcs->key_get<int>("hotkeys","system");
        if( sys < 0 || sys > 2) sys = 1;
        dynamic_cast<Gtk::RadioButton*>(m_Xml->get_widget ((boost::format ("mm-rb-%d") % (sys+1)).str()))->set_active();

        m_Xml->get_widget ("mm-apply")->set_sensitive (false);
        m_Xml->get_widget ("mm-revert")->set_sensitive (false);

        on_mm_edit_done() ;
    }

    void
        MMKeys::set_keytext(
              gint  entry_id
            , gint  key
            , gint  mask
        )
    {
        on_mm_edit_begin() ;

        std::string text ;

        if( key == 0 && mask == 0 )
        {
            text = (_("(none)"));
        }
        else
        {
            char const* modifier_string[] = { "Control", "Shift", "Alt", "Mod2", "Mod3", "Super", "Mod5" };
            const unsigned modifiers[] = { ControlMask, ShiftMask, Mod1Mask, Mod2Mask, Mod3Mask, Mod4Mask, Mod5Mask };

            std::string keytext;
            std::vector<std::string> strings;
            int i, j;
            KeySym keysym;

            keysym = XKeycodeToKeysym(gdk_x11_display_get_xdisplay (get_display()->gobj()), key, 0);
            if( keysym == 0 || keysym == NoSymbol )
            {
                keytext = (boost::format ("#%3d") % key).str();
            }
            else
            {
                keytext = XKeysymToString(keysym);
            }

            for (i = 0, j=0; j<7; j++)
            {
                if( mask & modifiers[j] )
                    strings.push_back (modifier_string[j]);
            }
            if( key != 0 )
            {
                strings.push_back( keytext );
            }

            text = Util::stdstrjoin(strings, " + ");
        }

        Gtk::Entry * entry;
        m_Xml->get_widget ((boost::format ("mm-entry-%d") % entry_id).str(), entry);

        entry->set_text (text);
        entry->set_position(-1);
    }

    bool
        MMKeys::on_entry_key_press_event(GdkEventKey * event, gint entry_id)
    {
        KeyControls & controls = m_mm_key_controls[entry_id-1];
        int is_mod;
        int mod;

        if( event->keyval == GDK_Tab ) return false;
        mod = 0;
        is_mod = 0;

        if( (event->state & GDK_CONTROL_MASK) | (!is_mod && (is_mod = (event->keyval == GDK_Control_L || event->keyval == GDK_Control_R))) )
            mod |= ControlMask;

        if( (event->state & GDK_MOD1_MASK) | (!is_mod && (is_mod = (event->keyval == GDK_Alt_L || event->keyval == GDK_Alt_R))) )
            mod |= Mod1Mask;

        if( (event->state & GDK_SHIFT_MASK) | (!is_mod && (is_mod = (event->keyval == GDK_Shift_L || event->keyval == GDK_Shift_R))) )
            mod |= ShiftMask;

        if( (event->state & GDK_MOD5_MASK) | (!is_mod && (is_mod = (event->keyval == GDK_ISO_Level3_Shift))) )
            mod |= Mod5Mask;

        if( (event->state & GDK_MOD4_MASK) | (!is_mod && (is_mod = (event->keyval == GDK_Super_L || event->keyval == GDK_Super_R))) )
            mod |= Mod4Mask;

        if( !is_mod )
        {
            controls.key = event->hardware_keycode;
            controls.mask = mod;
        } else controls.key = 0;

        set_keytext (entry_id, is_mod ? 0 : event->hardware_keycode, mod);
        m_Xml->get_widget ("mm-apply")->set_sensitive (true);
        m_Xml->get_widget ("mm-revert")->set_sensitive (true);
        return false;
    }

    bool
        MMKeys::on_entry_key_release_event(GdkEventKey * event, gint entry_id)
    {
        KeyControls & controls = m_mm_key_controls[entry_id-1];
        if( controls.key == 0 )
        {
            controls.mask = 0;
        }
        set_keytext (entry_id, controls.key, controls.mask);
        m_Xml->get_widget ("mm-apply")->set_sensitive (true);
        m_Xml->get_widget ("mm-revert")->set_sensitive (true);
        return false;
    }

    void
        MMKeys::on_clear_keyboard (gint entry_id)
    {
        KeyControls & controls = m_mm_key_controls[entry_id-1];
        controls.key = 0;
        controls.mask = 0;
        set_keytext(entry_id, 0, 0);
        m_Xml->get_widget ("mm-apply")->set_sensitive (true);
        m_Xml->get_widget ("mm-revert")->set_sensitive (true);
    }

    void
        MMKeys::on_mm_option_changed (gint option)
    {
        on_mm_edit_begin() ;

        switch( option )
        {
            case 1:
            case 2:
                m_Xml->get_widget ("mm-table")->set_sensitive (false);
                break;

            case 3:
                m_Xml->get_widget ("mm-table")->set_sensitive (true);
                break;
        }
        m_mm_option = option - 1;
        m_Xml->get_widget ("mm-apply")->set_sensitive (true);
    }

    void
        MMKeys::mmkeys_get_offending_modifiers ()
    {
        Display * dpy = gdk_x11_display_get_xdisplay (gdk_display_get_default());
        int i;
        XModifierKeymap *modmap;
        KeyCode nlock, slock;
        static int mask_table[8] =
        {
            ShiftMask, LockMask, ControlMask, Mod1Mask,
            Mod2Mask, Mod3Mask, Mod4Mask, Mod5Mask
        };

        nlock = XKeysymToKeycode (dpy, XK_Num_Lock);
        slock = XKeysymToKeycode (dpy, XK_Scroll_Lock);

        /*
         * Find out the masks for the NumLock and ScrollLock modifiers,
         * so that we can bind the grabs for when they are enabled too.
         */
        modmap = XGetModifierMapping (dpy);

        if( modmap != NULL && modmap->max_keypermod > 0 )
        {
            for (i = 0; i < 8 * modmap->max_keypermod; i++)
            {
                if( modmap->modifiermap[i] == nlock && nlock != 0 )
                    m_numlock_mask = mask_table[i / modmap->max_keypermod];
                else if( modmap->modifiermap[i] == slock && slock != 0 )
                    m_scrolllock_mask = mask_table[i / modmap->max_keypermod];
            }
        }

        m_capslock_mask = LockMask;

        if( modmap )
            XFreeModifiermap (modmap);
    }

    /*static*/ void
        MMKeys::grab_mmkey(
            int        key_code,
            GdkWindow *root
        )
    {
        gdk_error_trap_push ();

        XGrabKey (GDK_DISPLAY (), key_code,
            0,
            GDK_WINDOW_XID (root), True,
            GrabModeAsync, GrabModeAsync);
        XGrabKey (GDK_DISPLAY (), key_code,
            Mod2Mask,
            GDK_WINDOW_XID (root), True,
            GrabModeAsync, GrabModeAsync);
        XGrabKey (GDK_DISPLAY (), key_code,
            Mod5Mask,
            GDK_WINDOW_XID (root), True,
            GrabModeAsync, GrabModeAsync);
        XGrabKey (GDK_DISPLAY (), key_code,
            LockMask,
            GDK_WINDOW_XID (root), True,
            GrabModeAsync, GrabModeAsync);
        XGrabKey (GDK_DISPLAY (), key_code,
            Mod2Mask | Mod5Mask,
            GDK_WINDOW_XID (root), True,
            GrabModeAsync, GrabModeAsync);
        XGrabKey (GDK_DISPLAY (), key_code,
            Mod2Mask | LockMask,
            GDK_WINDOW_XID (root), True,
            GrabModeAsync, GrabModeAsync);
        XGrabKey (GDK_DISPLAY (), key_code,
            Mod5Mask | LockMask,
            GDK_WINDOW_XID (root), True,
            GrabModeAsync, GrabModeAsync);
        XGrabKey (GDK_DISPLAY (), key_code,
            Mod2Mask | Mod5Mask | LockMask,
            GDK_WINDOW_XID (root), True,
            GrabModeAsync, GrabModeAsync);

        gdk_flush ();

        if( gdk_error_trap_pop () )
        {
            g_message(G_STRLOC ": Error grabbing key");
        }
    }

    /*static*/ void
        MMKeys::grab_mmkey(
            int        key_code,
            int        modifier,
            GdkWindow *root
        )
    {
        gdk_error_trap_push ();

        modifier &= ~(m_numlock_mask | m_capslock_mask | m_scrolllock_mask);

        XGrabKey (GDK_DISPLAY (), key_code, modifier, GDK_WINDOW_XID (root),
            False, GrabModeAsync, GrabModeAsync);

        if( modifier == AnyModifier )
            return;

        if( m_numlock_mask )
            XGrabKey (GDK_DISPLAY (), key_code, modifier | m_numlock_mask,
                GDK_WINDOW_XID (root),
                False, GrabModeAsync, GrabModeAsync);

        if( m_capslock_mask )
            XGrabKey (GDK_DISPLAY (), key_code, modifier | m_capslock_mask,
                GDK_WINDOW_XID (root),
                False, GrabModeAsync, GrabModeAsync);

        if( m_scrolllock_mask )
            XGrabKey (GDK_DISPLAY (), key_code, modifier | m_scrolllock_mask,
                GDK_WINDOW_XID (root),
                False, GrabModeAsync, GrabModeAsync);

        if( m_numlock_mask && m_capslock_mask )
            XGrabKey (GDK_DISPLAY (), key_code, modifier | m_numlock_mask | m_capslock_mask,
                GDK_WINDOW_XID (root),
                False, GrabModeAsync, GrabModeAsync);

        if( m_numlock_mask && m_scrolllock_mask )
            XGrabKey (GDK_DISPLAY (), key_code, modifier | m_numlock_mask | m_scrolllock_mask,
                GDK_WINDOW_XID (root),
                False, GrabModeAsync, GrabModeAsync);

        if( m_capslock_mask && m_scrolllock_mask )
            XGrabKey (GDK_DISPLAY (), key_code, modifier | m_capslock_mask | m_scrolllock_mask,
                GDK_WINDOW_XID (root),
                False, GrabModeAsync, GrabModeAsync);

        if( m_numlock_mask && m_capslock_mask && m_scrolllock_mask )
            XGrabKey (GDK_DISPLAY (), key_code,
                modifier | m_numlock_mask | m_capslock_mask | m_scrolllock_mask,
                GDK_WINDOW_XID (root), False, GrabModeAsync,
                GrabModeAsync);

        gdk_flush ();

        if( gdk_error_trap_pop () )
        {
            g_message(G_STRLOC ": Error grabbing key");
        }
    }

    /*static*/ void
        MMKeys::ungrab_mmkeys (GdkWindow *root)
    {
        gdk_error_trap_push ();

        XUngrabKey (GDK_DISPLAY (), AnyKey, AnyModifier, GDK_WINDOW_XID (root));

        gdk_flush ();

        if( gdk_error_trap_pop () )
        {
            g_message(G_STRLOC ": Error grabbing key");
        }
    }

    /*static*/ GdkFilterReturn
        MMKeys::filter_mmkeys(
            GdkXEvent   *xevent,
            GdkEvent    *event,
            gpointer     data
        )
    {
        boost::shared_ptr<IYoukiController> ctrl = services->get<IYoukiController>("mpx-service-controller") ; 

        XEvent * xev = (XEvent *) xevent;

        if( xev->type != KeyPress )
        {
            return GDK_FILTER_CONTINUE;
        }

        XKeyEvent * key = (XKeyEvent *) xevent;

        guint keycodes[] = { 0, 0, 0, 0 };

        int sys = mcs->key_get<int>("hotkeys","system");

        if( sys == 0 )
        {
            keycodes[0] = XKeysymToKeycode (GDK_DISPLAY (), XF86XK_AudioPlay);
            keycodes[1] = XKeysymToKeycode (GDK_DISPLAY (), XF86XK_AudioPrev);
            keycodes[2] = XKeysymToKeycode (GDK_DISPLAY (), XF86XK_AudioNext);
            keycodes[3] = XKeysymToKeycode (GDK_DISPLAY (), XF86XK_AudioStop);
        }
        else
        {
            keycodes[0] = mcs->key_get<int>("hotkeys","key-1");
            keycodes[1] = mcs->key_get<int>("hotkeys","key-2");
            keycodes[2] = mcs->key_get<int>("hotkeys","key-3");
            keycodes[3] = mcs->key_get<int>("hotkeys","key-4");
        }

        if( keycodes[0] == key->keycode )
        {
            ctrl->API_pause_toggle() ;
            return GDK_FILTER_REMOVE ;
        }
        else
        if( keycodes[1] == key->keycode )
        {
            ctrl->API_prev() ;
            return GDK_FILTER_REMOVE ;
        }
        else    
        if( keycodes[2] == key->keycode )
        {
            ctrl->API_next() ;
            return GDK_FILTER_REMOVE ;
        }
        else    
        if( keycodes[3] == key->keycode )
        {
            ctrl->API_stop() ;
            return GDK_FILTER_REMOVE ;
        }

        return GDK_FILTER_CONTINUE ;
    }

    /*static*/ void
        MMKeys::mmkeys_grab (bool grab)
    {
        gint keycodes[] = { 0, 0, 0, 0 };
        keycodes[0] = XKeysymToKeycode (GDK_DISPLAY (), XF86XK_AudioPlay);
        keycodes[1] = XKeysymToKeycode (GDK_DISPLAY (), XF86XK_AudioPrev);
        keycodes[2] = XKeysymToKeycode (GDK_DISPLAY (), XF86XK_AudioNext);
        keycodes[3] = XKeysymToKeycode (GDK_DISPLAY (), XF86XK_AudioStop);

        GdkDisplay  * display ;
        GdkScreen   * screen ;
        GdkWindow   * root ;

        display = gdk_display_get_default ();

        int sys = mcs->key_get<int>("hotkeys","system");

        for (int i = 0; i < gdk_display_get_n_screens (display); i++)
        {
            screen = gdk_display_get_screen (display, i);

            if( screen != NULL )
            {
                root = gdk_screen_get_root_window (screen);
                if(!grab)
                {
                    ungrab_mmkeys (root);
                    continue;
                }

                for (guint j = 1; j <= G_N_ELEMENTS(keycodes) ; ++j)
                {
                    if( sys == 2 )
                    {
                        int key = mcs->key_get<int>("hotkeys", (boost::format ("key-%d") % j).str());
                        int mask = mcs->key_get<int>("hotkeys", (boost::format ("key-%d-mask") % j).str());

                        if( key )
                        {
                            grab_mmkey (key, mask, root);
                        }
                    }
                    else
                    if( sys == 0 )
                    {
                        grab_mmkey (keycodes[j-1], root);
                    }
                }

                if( grab )
                    gdk_window_add_filter (root, filter_mmkeys, this);
                else
                    gdk_window_remove_filter (root, filter_mmkeys, this);
            }
        }
    }

    /*static*/ void
        MMKeys::mmkeys_activate ()
    {
        if( m_active )
            return ;

        m_active = true ;
        on_mm_edit_done();

        DBusGConnection *bus;

        g_message(G_STRLOC ": Activating media player keys");

        m_mmkeys_dbusproxy = 0;

        if( m_mmkeys_grab_type == SETTINGS_DAEMON )
        {
            bus = dbus_g_bus_get( DBUS_BUS_SESSION, NULL ) ;

            if( bus )
            {
                GError *error = NULL;

                m_mmkeys_dbusproxy = dbus_g_proxy_new_for_name( bus,
                    "org.gnome.SettingsDaemon",
                    "/org/gnome/SettingsDaemon/MediaKeys",
                    "org.gnome.SettingsDaemon.MediaKeys");

                if( !m_mmkeys_dbusproxy )
                {
                    m_mmkeys_dbusproxy = dbus_g_proxy_new_for_name( bus,
                        "org.gnome.SettingsDaemon",
                        "/org/gnome/SettingsDaemon",
                        "org.gnome.SettingsDaemon");
                }

                if( m_mmkeys_dbusproxy )
                {
                    dbus_g_proxy_call( m_mmkeys_dbusproxy,
                        "GrabMediaPlayerKeys", &error,
                        G_TYPE_STRING, "MPX",
                        G_TYPE_UINT, 0,
                        G_TYPE_INVALID,
                        G_TYPE_INVALID);

                    if( error == NULL )
                    {
                        g_message(G_STRLOC ": created dbus proxy for org.gnome.SettingsDaemon; grabbing keys");

                        dbus_g_object_register_marshaller (g_cclosure_user_marshal_VOID__STRING_STRING,
                            G_TYPE_NONE, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INVALID);

                        dbus_g_proxy_add_signal (m_mmkeys_dbusproxy,
                            "MediaPlayerKeyPressed",
                            G_TYPE_STRING,G_TYPE_STRING,G_TYPE_INVALID);

                        dbus_g_proxy_connect_signal (m_mmkeys_dbusproxy,
                            "MediaPlayerKeyPressed",
                            G_CALLBACK (media_player_key_pressed),
                            this, NULL);

                        boost::shared_ptr<IYoukiController> ctrl = services->get<IYoukiController>("mpx-service-controller") ; 

                        mWindowFocusConn = ctrl->get_widget()->signal_focus_in_event().connect( sigc::mem_fun( *this, &MMKeys::window_focus_cb ) );
                    }
                    else if (error->domain == DBUS_GERROR &&
                        (error->code != DBUS_GERROR_NAME_HAS_NO_OWNER ||
                        error->code != DBUS_GERROR_SERVICE_UNKNOWN))
                    {
                        /* settings daemon dbus service doesn't exist.
                         * just silently fail.
                         */
                        g_message(G_STRLOC ": org.gnome.SettingsDaemon dbus service not found: %s", error->message);
                        g_error_free (error);
                        g_object_unref(m_mmkeys_dbusproxy);
                        m_mmkeys_dbusproxy = 0;
                    }
                    else
                    {
                        g_warning (G_STRLOC ": Unable to grab media player keys: %s", error->message);
                        g_error_free (error);
                        g_object_unref(m_mmkeys_dbusproxy);
                        m_mmkeys_dbusproxy = 0;
                    }
                }

                dbus_g_connection_unref( bus );
            }
            else
            {
                g_message(G_STRLOC ": couldn't get dbus session bus");
            }
        }
        else if( m_mmkeys_grab_type == X_KEY_GRAB )
        {
            g_message(G_STRLOC ": attempting old-style key grabs");
            mmkeys_grab (true);
        }
    }

    /*static*/ void
        MMKeys::mmkeys_deactivate ()
    {
        if( !m_active )
            return ;

        m_active = false ;

        if( m_mmkeys_dbusproxy )
        {
            if( m_mmkeys_grab_type == SETTINGS_DAEMON )
            {
                GError *error = NULL;

                dbus_g_proxy_call (m_mmkeys_dbusproxy,
                    "ReleaseMediaPlayerKeys", &error,
                    G_TYPE_STRING, "MPX",
                    G_TYPE_INVALID, G_TYPE_INVALID);

                if( error != NULL )
                {
                    g_warning (G_STRLOC ": Could not release media player keys: %s", error->message);
                    g_error_free (error);
                }

                mWindowFocusConn.disconnect ();
                m_mmkeys_grab_type = NONE;
            }

            g_object_unref( m_mmkeys_dbusproxy ) ;
            m_mmkeys_dbusproxy = 0;
        }

        if( m_mmkeys_grab_type == X_KEY_GRAB )
        {
            g_message(G_STRLOC ": undoing old-style key grabs");
            mmkeys_grab( false ) ;
            m_mmkeys_grab_type = NONE;
        }
    }

    void
        MMKeys::on_mm_edit_begin ()
    {
        mmkeys_deactivate ();
    }

    void
        MMKeys::on_mm_edit_done ()
    {
        if( m_active ) 
        {
            int sys = mcs->key_get<int>("hotkeys","system");

            if( (sys == 0) || (sys == 2))
                m_mmkeys_grab_type = X_KEY_GRAB;
            else
                m_mmkeys_grab_type = SETTINGS_DAEMON;

            mmkeys_activate ();
        }
    }

    // MMkeys code (C) Rhythmbox Developers 2007

    void
        MMKeys::media_player_key_pressed(
              DBusGProxy  *proxy
            , const gchar *application
            , const gchar *key
            , gpointer     data
        )
    {
        if( strcmp( application, "MPX" ))
            return ;

        boost::shared_ptr<IYoukiController> ctrl = services->get<IYoukiController>("mpx-service-controller") ; 

        if( strcmp( key, "Play" ) == 0 )
        {
            ctrl->API_pause_toggle() ;
        }
        else
        if( strcmp( key, "Previous" ) == 0 )
        {
            ctrl->API_prev() ;
        }
        else
        if( strcmp( key, "Next" ) == 0 )
        {
            ctrl->API_next() ;
        }
        else
        if( strcmp( key, "Stop" ) == 0 )
        {
            ctrl->API_stop() ;
        }
    }

    bool
        MMKeys::window_focus_cb (GdkEventFocus *event)
        {
                dbus_g_proxy_call (m_mmkeys_dbusproxy,
                                "GrabMediaPlayerKeys", NULL,
                                G_TYPE_STRING, "MPX",
                                G_TYPE_UINT, 0,
                                G_TYPE_INVALID, G_TYPE_INVALID);

                return false;
        }


} // namespace MPX
