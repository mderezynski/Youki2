//
// treeview-popup
//
// Authors:
//     Milosz Derezynski <milosz@backtrace.info>
//
// (C) 2008 MPX Project
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
#ifndef MPX_TREEVIEW_POPUP_HH
#define MPX_TREEVIEW_POPUP_HH
#include "config.h"
#include <glibmm.h>
#include <gtkmm/treeview.h>
#include <gtkmm/uimanager.h>

namespace MPX
{
	class TreeViewPopup : public Gtk::TreeView
	{
        protected:

            Glib::RefPtr<Gtk::UIManager>        m_ui_manager;
            Glib::RefPtr<Gtk::ActionGroup>      m_actions;
            std::string                         m_default_path;

		public:

			TreeViewPopup (GtkTreeView*);
			virtual ~TreeViewPopup ();

        protected:

            virtual void
            popup_prepare_actions (Gtk::TreePath const& path, bool valid_path) = 0;

            virtual void
            popup_menu (Gtk::TreePath const& path, bool valid_path);

            virtual void
            popup_utility_default ();

        protected:
           
            virtual bool 
            on_event (GdkEvent*);
	};
}

#endif
