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

#ifndef MPX_FILE_SYSTEM_TREE_HH 
#define MPX_FILE_SYSTEM_TREE_HH

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif // HAVE_CONFIG_H
#include <giomm.h>
#include <gtkmm.h>
#include <libglademm/xml.h>
#include <sigx/sigx.h>
#include "mpx/widgets/widgetloader.hh"

namespace MPX
{
    typedef sigc::signal<void, Glib::ustring> SignalUri;

    class FileSystemTree
      : public Gnome::Glade::WidgetLoader<Gtk::TreeView>
      , public sigx::glib_auto_dispatchable
    {
            SignalUri       signalUri;

        public:
                
            SignalUri&
            signal_uri()
            {
                return signalUri;
            }

            FileSystemTree (Glib::RefPtr<Gnome::Glade::Xml> const& xml, std::string const&); 
            virtual ~FileSystemTree () {}

            void
            build_file_system_tree (std::string const& root_path);

        protected:

            virtual void
            on_drag_data_get (const Glib::RefPtr<Gdk::DragContext>& context, Gtk::SelectionData& selection_data, guint info, guint time);

            virtual void
            on_row_expanded (const Gtk::TreeIter & iter, const Gtk::TreePath & path);

            virtual void
            on_row_activated (const Gtk::TreeModel::Path&, Gtk::TreeViewColumn*);

        private:

            int
            file_system_tree_sort (const Gtk::TreeIter & iter_a, const Gtk::TreeIter & iter_b);

            bool
            prescan_path (std::string const& path, Gtk::TreeIter & iter);

            void
            append_path (std::string const& root_path, Gtk::TreeIter & root_iter);
            
            void
            cell_data_func_text (Gtk::CellRenderer * basecell, Gtk::TreeIter const& iter);

            bool
            search_func(const Glib::RefPtr<Gtk::TreeModel>&, int, const Glib::ustring&, const Gtk::TreeModel::iterator&);


            struct FileSystemTreeColumnsT : public Gtk::TreeModel::ColumnRecord
            {
                Gtk::TreeModelColumn<Glib::ustring>   SegName;
                Gtk::TreeModelColumn<std::string>     FullPath;
                Gtk::TreeModelColumn<bool>            WasExpanded;
                Gtk::TreeModelColumn<bool>            IsDir;

                FileSystemTreeColumnsT ()
                {
                    add (SegName);
                    add (FullPath);
                    add (WasExpanded);
                    add (IsDir);
                };
            };

            FileSystemTreeColumnsT FileSystemTreeColumns;
            Glib::RefPtr<Gtk::TreeStore> FileSystemTreeStore;
    };
}
#endif
