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
//  The MPX project hereby grants permission for non-GPL compatible GStreamer
//  plugins to be used and distributed together with GStreamer and MPX. This
//  permission is above and beyond the permissions granted by the GPL license
//  MPX is covered by.
#ifndef MPX_IMPORT_SHARE_HH
#define MPX_IMPORT_SHARE_HH

#include "mpx/widgets/widgetloader.hh"

#include <glibmm/ustring.h>
#include <gtkmm/dialog.h>
#include <libglademm/xml.h>

namespace MPX
{
  class DialogImportShare
      : public Gnome::Glade::WidgetLoader<Gtk::Dialog>
  {
      public:
          DialogImportShare (Glib::RefPtr<Gnome::Glade::Xml> const& xml);
          static DialogImportShare* create ();
          virtual ~DialogImportShare ();

          void
          get_share_infos(Glib::ustring& share, Glib::ustring& name, Glib::ustring& login, Glib::ustring& password);

          void
          on_cb_show_credentials_toggled();

      private:

        Glib::RefPtr<Gnome::Glade::Xml>	m_ref_xml;
  };
} // namespace MPX

#endif // !MPX_PODCAST_ADD_HH

