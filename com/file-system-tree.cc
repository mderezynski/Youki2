//  MPX
//  Copyright (C) 2005-2007 MPX development.
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

#include "mpx/com/file-system-tree.hh"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#include <gtkmm.h>
#include <gtk/gtk.h>
#include <glibmm/i18n.h>
#include <boost/format.hpp>

#include "mpx/util-file.hh"
#include "mpx/util-string.hh"

using namespace Glib;
using namespace Gtk;

namespace MPX
{
    FileSystemTree::FileSystemTree(
        RefPtr<Gnome::Glade::Xml> const&    xml,
        std::string const&                  name 
    )
    : Gnome::Glade::WidgetLoader<Gtk::TreeView>(xml, name)
    , sigx::glib_auto_dispatchable()
    {
        std::vector<Gtk::TargetEntry> entries;
        drag_source_set(entries);
        drag_source_add_uri_targets();

        Gtk::CellRendererText * cell = 0; 
        Gtk::TreeViewColumn * col = 0; 

        cell = manage (new Gtk::CellRendererText());
        col = manage (new Gtk::TreeViewColumn());

        col->pack_start(*cell, false);

        col->set_cell_data_func(
            *cell,
            sigc::mem_fun(
                *this,
                &FileSystemTree::cell_data_func_text
        ));

        append_column(*col);

        FileSystemTreeStore = Gtk::TreeStore::create(FileSystemTreeColumns);
        FileSystemTreeStore->set_default_sort_func(
            sigc::mem_fun(
                *this,
                &FileSystemTree::file_system_tree_sort
        ));
        FileSystemTreeStore->set_sort_column(
            -1,
            Gtk::SORT_ASCENDING
        ); 

        set_model(FileSystemTreeStore);
        set_enable_search();
        //set_search_column(FileSystemTreeColumns.SegName);
        set_search_equal_func(
            sigc::mem_fun(
                *this,
                &FileSystemTree::search_func
        ));
    }

    /* ------------------------------------------------------------------------------------------------*/

    int
    FileSystemTree::file_system_tree_sort (const TreeIter & iter_a, const TreeIter & iter_b)
    {
        bool is_dir_a = (*iter_a)[FileSystemTreeColumns.IsDir];
        bool is_dir_b = (*iter_b)[FileSystemTreeColumns.IsDir];

        Glib::ustring a = (*iter_a)[FileSystemTreeColumns.SegName];
        Glib::ustring b = (*iter_b)[FileSystemTreeColumns.SegName];

        if(!is_dir_a && !is_dir_b)
            return a.compare(b);

        if(is_dir_a && is_dir_b)
            return a.compare(b);

        if(!is_dir_a)
            return  1;
        else
            return -1;
    }

    bool
    FileSystemTree::prescan_path (std::string const& scan_path, TreeIter & iter)
    {
        try{
                Glib::Dir dir (scan_path);
                std::vector<std::string> strv (dir.begin(), dir.end());
                dir.close ();

                for(std::vector<std::string>::const_iterator i = strv.begin(); i != strv.end(); ++i)
                {
                    std::string path = build_filename(scan_path, *i);
                    if((i->operator[](0) != '.'))
                    {
                        FileSystemTreeStore->append(iter->children());
                        return true;
                    }
                }
        } catch( Glib::FileError ) {
        }

        return false;
    } 

    void
    FileSystemTree::append_path (std::string const& root_path, TreeIter & root_iter)
    {
        try{
                Glib::Dir dir (root_path);
                std::vector<std::string> strv (dir.begin(), dir.end());
                dir.close ();

                for(std::vector<std::string>::const_iterator i = strv.begin(); i != strv.end(); ++i)
                {
                    std::string path = build_filename(root_path, *i);
                    if(i->operator[](0) != '.')
                        try{
                            TreeIter iter = FileSystemTreeStore->append(root_iter->children());
                            (*iter)[FileSystemTreeColumns.SegName] = *i; 
                            (*iter)[FileSystemTreeColumns.FullPath] = path;
                            (*iter)[FileSystemTreeColumns.WasExpanded] = false;

                            if(file_test(path, FILE_TEST_IS_DIR))
                            {
                                (*iter)[FileSystemTreeColumns.IsDir] = true;
                                if(!prescan_path (path, iter))
                                {
                                    FileSystemTreeStore->erase(iter);
                                }
                            }
                            else
                            {
                                (*iter)[FileSystemTreeColumns.IsDir] = false;
                            }

                        } catch (Glib::Error)
                        {
                        }
                }
        } catch( Glib::FileError ) {
        }

        (*root_iter)[FileSystemTreeColumns.WasExpanded] = true;
    }

    void
    FileSystemTree::build_file_system_tree (std::string const& root_path)
    {
        FileSystemTreeStore->clear ();
        TreeIter root_iter = FileSystemTreeStore->append();
        FileSystemTreeStore->append(root_iter->children());
        (*root_iter)[FileSystemTreeColumns.SegName] = root_path;
        (*root_iter)[FileSystemTreeColumns.FullPath] = root_path;
    }

    void
    FileSystemTree::cell_data_func_text (CellRenderer * basecell, TreeIter const& iter)
    {
        CellRendererText & cell = *(dynamic_cast<CellRendererText*>(basecell));

        TreePath path = FileSystemTreeStore->get_path(iter); 

        if(path.size() < 1)
            return;

        Glib::ustring segName = (*iter)[FileSystemTreeColumns.SegName];
        cell.property_text() = segName; 
    }

    bool
    FileSystemTree::search_func(const Glib::RefPtr<TreeModel>& model, int col, const Glib::ustring& key, const TreeModel::iterator& iter)
    {
        Glib::ustring haystack = (*iter)[FileSystemTreeColumns.SegName]; 
        return !Util::match_keys(haystack, key);
    }

    void
    FileSystemTree::on_row_activated (const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* G_GNUC_UNUSED)
    {
        TreeIter iter = FileSystemTreeStore->get_iter(path);
        std::string FullPath = (*iter)[FileSystemTreeColumns.FullPath]; 
        signalUri.emit(Glib::filename_to_uri(FullPath));
    }

    void
    FileSystemTree::on_row_expanded (const Gtk::TreeIter & iter, const Gtk::TreePath & path)
    {
        if( (*iter)[FileSystemTreeColumns.WasExpanded] )
            return;

        GtkTreeIter children;

        bool has_children = (
            gtk_tree_model_iter_children(
                GTK_TREE_MODEL(FileSystemTreeStore->gobj()),
                &children,
                const_cast<GtkTreeIter*>(iter->gobj())
        ));

        std::string full_path = (*iter)[FileSystemTreeColumns.FullPath];
        TreeIter iter_copy = iter;

        append_path(full_path, iter_copy);

        if(has_children)
        {
            gtk_tree_store_remove(GTK_TREE_STORE(FileSystemTreeStore->gobj()), &children);
        }
        else
            g_warning("%s:%d : No placeholder row present, state seems corrupted.", __FILE__, __LINE__);
    }

    void
    FileSystemTree::on_drag_data_get (const Glib::RefPtr<Gdk::DragContext>& context, SelectionData& selection_data, guint info, guint time)
    {
        TreeIter iter = get_selection()->get_selected();
        std::string FullPath = (*iter)[FileSystemTreeColumns.FullPath];
        std::vector<std::string> uris (1, Glib::filename_to_uri(FullPath));
        selection_data.set_uris(uris); 
    }
}
