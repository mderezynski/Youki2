//  MPX
//  Copyright (C) 2005-2007 MPX Project
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
#ifndef MPX_UTIL_UI_HH
#define MPX_UTIL_UI_HH
#include "config.h"
#include <string>
#include <glibmm/ustring.h>
#include <gtkmm/uimanager.h>
#include <gtkmm/window.h>

namespace MPX
{
  namespace Util
  {
    bool
    ui_manager_add_ui(
        Glib::RefPtr<Gtk::UIManager>&
      , char const*
      , Gtk::Window&
      , const Glib::ustring&
      , guint&
    );

    Gtk::Widget*
    get_popup(
          Glib::RefPtr<Gtk::UIManager>
        , const Glib::ustring&
    );
  } // Util
} // MPX

#endif // MPX_UTIL_UI_HH
