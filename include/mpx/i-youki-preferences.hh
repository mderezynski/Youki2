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

#ifndef _YOUKI_PREFERENCES__HH
#define _YOUKI_PREFERENCES__HH

#include "config.h"

#include <gtkmm.h>

#include "mpx/widgets/widgetloader.hh"
#include "mpx/mpx-main.hh"

namespace MPX
{
  /** Preferences dialog
   *
   * MPX::Preferences is a complex dialog for adjusting run time parameters
   * of MPX trough the GUI instead of having to manipulate the configuration
   * file.
   */
  class IPreferences
  {
    public:

        virtual void
        add_page(
              Gtk::Widget*
            , const std::string&
        ) = 0 ;
  } ; // class IPreferences
} // namespace MPX

#endif // _YOUKI_PREFERENCES_HH
