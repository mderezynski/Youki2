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
#include "config.h"
#include <glibmm/i18n.h>
#include <gtkmm.h>
#include <glib.h>
#include <giomm.h>
#include <libglademm.h>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include "mpx/mpx-library.hh"
#include "mpx/mpx-sql.hh"
#include "mpx/mpx-stock.hh"
#include "mpx/mpx-types.hh"
#include "mpx/util-graphics.hh"
#include "mpx/util-ui.hh"
#include "mpx/util-string.hh"
#include "mpx/widgets/widgetloader.hh"

#include "mpx/widgets/cell-renderer-cairo-surface.hh"
#include "mpx/widgets/cell-renderer-count.hh"
#include "mpx/widgets/cell-renderer-vbox.hh"
#include "mpx/widgets/cell-renderer-expander.h"
#include "mpx/widgets/rounded-layout.hh"
#include "mpx/widgets/timed-confirmation.hh"

#include "mpx/com/view-collections.hh"

using namespace Gtk;
using namespace Glib;
using namespace Gnome::Glade;
using namespace MPX;
using boost::get;
using boost::algorithm::trim;
using boost::algorithm::split;

namespace
{
        const int N_STARS = 6;

        const char ui_collections_popup [] =
                "<ui>"
                ""
                "<menubar name='popup-collectionlist-list-%1%'>"
                ""
                "   <menu action='dummy' name='menu-collectionlist-list-%1%'>"
                "   </menu>"
                ""
                "</menubar>"
                ""
                "</ui>";

        std::string
                get_timestr_from_time_t (time_t atime)
                {
                        struct tm atm, btm;
                        localtime_r (&atime, &atm);
                        time_t curtime = time(NULL);
                        localtime_r (&curtime, &btm);

                        static boost::format date_f ("%s, %s");

                        if (atm.tm_year == btm.tm_year &&
                                        atm.tm_yday == btm.tm_yday)
                        {
                                char btime[64];
                                strftime (btime, 64, "%H:00", &atm); // we just ASSUME that no podcast updates more than once an hour, for cleaner readbility

                                return locale_to_utf8 ((date_f % _("Today") % btime).str());
                        }
                        else
                        {
                                char bdate[64];
                                strftime (bdate, 64, "%d %b %Y", &atm);

                                char btime[64];
                                strftime (btime, 64, "%H:00", &atm); // we just ASSUME that no podcast updates more than once an hour, for cleaner readbility

                                return locale_to_utf8 ((date_f % bdate % btime).str());
                        }
                }

        enum Order
        {
                NO_ORDER,
                ORDER
        };
}

namespace MPX
{
                CollectionTreeView::CollectionTreeView(
                          const Glib::RefPtr<Gnome::Glade::Xml>&  xml    
                        , const std::string&                      name
                        , const std::string&                      name_showing_label
                        , const std::string&                      name_filter_entry
                        , Glib::RefPtr<Gtk::UIManager>            ui_manager
                )
                : WidgetLoader<Gtk::TreeView>(xml,name)
                , m_Name(name)
                , m_ButtonPressed(false)
                {
                        services->get<Library>("mpx-service-library")->signal_collection_new_track().connect(
                            sigc::mem_fun(
                                *this,
                                &CollectionTreeView::on_new_track
                        ));

                        services->get<Library>("mpx-service-library")->signal_collection_new().connect(
                            sigc::mem_fun(
                                *this,
                                &CollectionTreeView::on_new_collection
                        ));

#if 0
                        services->get<Library>("mpx-service-library")->signal_collection_deleted().connect(
                            sigc::mem_fun(
                                *this,
                                &CollectionTreeView::on_collection_deleted
                        ));

                        services->get<Library>("mpx-service-library")->signal_track_deleted().connect(
                            sigc::mem_fun(
                                *this,
                                &CollectionTreeView::on_track_deleted
                        ));

                        services->get<Library>("mpx-service-library")->signal_collection_updated().connect(
                            sigc::mem_fun(
                                *this,
                                &CollectionTreeView::on_collection_updated
                        ));

                        services->get<Library>("mpx-service-library")->signal_reload().connect(
                            sigc::mem_fun(
                                *this,
                                &CollectionTreeView::collection_list_load
                        ));

                        m_Covers.get().signal_got_cover().connect(
                            sigc::mem_fun(
                                *this,
                                &CollectionTreeView::on_got_cover
                        ));
#endif

                        if( !name_showing_label.empty() )
                            m_LabelShowing = new RoundedLayout(xml, name_showing_label);
                        else
                            m_LabelShowing = 0;

                        set_show_expanders( false );
                        set_level_indentation( 32 );

                        TreeViewColumn * col = manage (new TreeViewColumn());
                        GtkCellRenderer * renderer = gossip_cell_renderer_expander_new ();
                        gtk_tree_view_column_pack_start (col->gobj(), renderer, FALSE);
                        gtk_tree_view_column_set_cell_data_func (col->gobj(),
                                        renderer,
                                        GtkTreeCellDataFunc(rb_sourcelist_expander_cell_data_func),
                                        this,
                                        NULL);

                        CellRendererCairoSurface * cellcairo = manage (new CellRendererCairoSurface);
                        col->pack_start(*cellcairo, false);
                        col->set_cell_data_func(*cellcairo, sigc::mem_fun( *this, &CollectionTreeView::cellDataFuncCover ));
                        cellcairo->property_xpad() = 4;
                        cellcairo->property_ypad() = 4;
                        cellcairo->property_yalign() = 0.;
                        cellcairo->property_xalign() = 0.;

                        CellRendererText *celltext = manage (new CellRendererText);
                        celltext->property_yalign() = 0.;
                        celltext->property_ypad() = 4;
                        celltext->property_height() = 72;
                        celltext->property_ellipsize() = Pango::ELLIPSIZE_MIDDLE;

                        col->pack_start(*celltext, true);
                        col->set_cell_data_func(
                                        *celltext,
                                        sigc::mem_fun(
                                                *this,
                                                &CollectionTreeView::cellDataFuncText1
                                                ));

                        CellRendererCount *cellcount = manage (new CellRendererCount);
                        cellcount->property_box() = BOX_NORMAL;
                        col->pack_start(*cellcount, false);

                        col->set_cell_data_func(
                                        *cellcount,
                                        sigc::mem_fun(
                                                *this,
                                                &CollectionTreeView::cellDataFuncText2
                                                ));

                        celltext = manage (new CellRendererText);
                        col->pack_start(*celltext, false);

                        col->set_cell_data_func(
                                        *celltext,
                                        sigc::mem_fun(
                                                *this,
                                                &CollectionTreeView::cellDataFuncText3
                                                ));

                        celltext = manage (new CellRendererText);
                        col->pack_start(*celltext, false);

                        col->set_cell_data_func(
                                        *celltext,
                                        sigc::mem_fun(
                                                *this,
                                                &CollectionTreeView::cellDataFuncText4
                                                ));

                        celltext = manage (new CellRendererText);
                        col->pack_start(*celltext, false);
                        celltext->property_xalign() = 0.;
                        celltext->property_xpad() = 2;

                        col->set_cell_data_func(
                                        *celltext,
                                        sigc::mem_fun(
                                                *this,
                                                &CollectionTreeView::cellDataFuncText5
                                                ));

                        append_column(*col);

                        CollectionsTreeStore = Gtk::TreeStore::create(Columns);
                        CollectionsTreeStoreFilter = Gtk::TreeModelFilter::create(CollectionsTreeStore);

                        CollectionsTreeStoreFilter->set_visible_func(
                                        sigc::mem_fun(
                                                *this,
                                                &CollectionTreeView::collection_visible_func
                                                ));

                        CollectionsTreeStoreFilter->signal_row_inserted().connect((
                                                sigc::hide(sigc::hide(sigc::mem_fun(
                                                                        *this,
                                                                        &CollectionTreeView::on_row_added_or_deleted
                                                                        )))));

                        CollectionsTreeStoreFilter->signal_row_deleted().connect((
                                                sigc::hide(sigc::mem_fun(
                                                                *this,
                                                                &CollectionTreeView::on_row_added_or_deleted
                                                                ))));


                        set_model(CollectionsTreeStoreFilter);

                        Glib::RefPtr<Gdk::Pixbuf> pixbuf =
                                Gdk::Pixbuf::create_from_file(
                                                build_filename(
                                                        DATA_DIR,
                                                        build_filename("images","disc.png")
                                                        )
                                                )->scale_simple(90,90,Gdk::INTERP_BILINEAR);

                        m_DiscDefault = Util::cairo_image_surface_from_pixbuf(pixbuf->scale_simple(90,90,Gdk::INTERP_BILINEAR));

                        std::vector<TargetEntry> Entries;
                        Entries.push_back(TargetEntry("mpx-collection", TARGET_SAME_APP, 0x90));
                        Entries.push_back(TargetEntry("mpx-track", TARGET_SAME_APP, 0x81));
                        drag_source_set(Entries); 

                        xml->get_widget(name_filter_entry, m_FilterEntry);

                        m_FilterEntry->signal_changed().connect(
                                        sigc::mem_fun(
                                                *this,
                                                &CollectionTreeView::on_filter_entry_changed
                                                ));

                        m_UIManager = ui_manager;

                        m_ActionGroup = Gtk::ActionGroup::create ((boost::format ("Actions-%s") % name).str());

                        m_ActionGroup->add(Gtk::Action::create("dummy","dummy"));

                        /*
                        m_ActionGroup->add(

                                        Gtk::Action::create(
                                                "action-collection-info", 
                                                Gtk::StockID (GTK_STOCK_INFO),
                                                _("Collection _Info")
                                                ),

                                        sigc::mem_fun(
                                                *this,
                                                &CollectionTreeView::on_collection_show_info
                                                ));
                        */

                        m_UIManager->insert_action_group(m_ActionGroup);
                        m_UIManager->add_ui_from_string((boost::format(ui_collections_popup) % m_Name).str());

                        collection_list_load ();
                }

                void
                        CollectionTreeView::on_filter_entry_changed ()
                        {
                                m_FilterText = m_FilterEntry->get_text().lowercase();
                                CollectionsTreeStoreFilter->refilter();
                        }

                void
                        CollectionTreeView::on_row_activated (const TreeModel::Path& path, TreeViewColumn* column)
                        {
#if 0
                                TreeIter iter = CollectionsTreeStore->get_iter (CollectionsTreeStoreFilter->convert_path_to_child_path(path));
                                if(path.get_depth() == CRT_ROW_COLLECTION)
                                {
                                        gint64 id = (*iter)[Columns.Id];
                                        Signals.PlayCollection.emit(id);
                                }
                                else
                                {
                                        gint64 id = (*iter)[Columns.TrackId];
                                        IdV v (1, id);
                                        Signals.PlayTracks.emit(v);
                                }
#endif
                        }

                void
                        CollectionTreeView::on_row_expanded (const TreeIter &iter_filter,
                                        const TreePath &path) 
                        {
                                TreeIter iter = CollectionsTreeStoreFilter->convert_iter_to_child_iter(iter_filter);
                                if(!(*iter)[Columns.HasTracks])
                                {
                                        GtkTreeIter children;
                                        bool has_children = (gtk_tree_model_iter_children(GTK_TREE_MODEL(CollectionsTreeStore->gobj()), &children, const_cast<GtkTreeIter*>(iter->gobj())));

                                        SQL::RowV v;
                                        services->get<Library>("mpx-service-library")->getSQL(v, (boost::format("SELECT track_id FROM collection_%lld;") % gint64((*iter)[Columns.Id])).str());

                                        for(SQL::RowV::iterator i = v.begin(); i != v.end(); ++i)
                                        {
                                                SQL::Row & r_id = *i;
                                                gint64 id = get<gint64>(r_id["track_id"]);

                                                SQL::RowV v;
                                                services->get<Library>("mpx-service-library")->getSQL(v, (boost::format ("SELECT * from track_view WHERE id = '%lld'") % id).str());
                                                SQL::Row & r = v[0];

                                                TreeIter child = CollectionsTreeStore->append(iter->children());
                                                (*child)[Columns.TrackArtist] = Markup::escape_text(get<std::string>(r["artist"]));
                                                (*child)[Columns.TrackTitle] = get<std::string>(r["title"]);
                                                (*child)[Columns.TrackNumber] = get<gint64>(r["track"]);
                                                (*child)[Columns.TrackLength] = get<gint64>(r["time"]);
                                                (*child)[Columns.TrackId] = get<gint64>(r["id"]);
                                                (*child)[Columns.RowType] = CRT_ROW_TRACK; 

//                                                m_TrackIterMap.insert(std::make_pair(id, child));
                                        }

                                        if(v.size())
                                        {
                                                (*iter)[Columns.HasTracks] = true;
                                                if(has_children)
                                                {
                                                        gtk_tree_store_remove(GTK_TREE_STORE(CollectionsTreeStore->gobj()), &children);
                                                } 
                                                else
                                                        g_warning("%s:%d : No placeholder row present, state seems corrupted.", __FILE__, __LINE__);
                                        }

                                }
                                scroll_to_row (path, 0.);
                        }

                void
                        CollectionTreeView::on_drag_data_get (const Glib::RefPtr<Gdk::DragContext>& context, SelectionData& selection_data, guint info, guint time)
                        {
                                if(m_DragCollectionId)
                                {
                                        gint64 * id = new gint64(m_DragCollectionId.get());
                                        selection_data.set("mpx-collection", 8, reinterpret_cast<const guint8*>(id), 8);
                                }
                                else if(m_DragTrackId)
                                {
                                        gint64 * id = new gint64(m_DragTrackId.get());
                                        selection_data.set("mpx-track", 8, reinterpret_cast<const guint8*>(id), 8);
                                }

                                m_DragCollectionId.reset();
                                m_DragTrackId.reset();
                        }

                void
                        CollectionTreeView::on_drag_begin (const Glib::RefPtr<Gdk::DragContext>& context) 
                        {
                                if(m_DragCollectionId)
                                {
                                        //drag_source_set_icon(pixbuf->scale_simple(128,128,Gdk::INTERP_BILINEAR)); // FIXME: Set collection pic as drag icon
                                }
                                else
                                {
                                        Glib::RefPtr<Gdk::Pixmap> pix = create_row_drag_icon(m_PathButtonPress);
                                        drag_source_set_icon(pix->get_colormap(), pix, Glib::RefPtr<Gdk::Bitmap>(0));
                                }
                        }

                bool
                        CollectionTreeView::on_button_press_event (GdkEventButton* event)
                        {
                                int cell_x, cell_y ;
                                TreeViewColumn *col ;

                                if(get_path_at_pos (event->x, event->y, m_PathButtonPress, col, cell_x, cell_y))
                                {
                                        TreeIter iter = CollectionsTreeStore->get_iter(CollectionsTreeStoreFilter->convert_path_to_child_path(m_PathButtonPress));
                                        if(m_PathButtonPress.get_depth() == CRT_ROW_COLLECTION)
                                        {
                                                m_DragCollectionId = (*iter)[Columns.Id];
                                                m_DragTrackId.reset(); 
                                                g_atomic_int_set(&m_ButtonPressed, 1);
                                        }
                                        else
                                        if(m_PathButtonPress.get_depth() == CRT_ROW_TRACK)
                                        {
                                                m_DragTrackId = (*iter)[Columns.TrackId];
                                                m_DragCollectionId.reset();
                                        }
                                }
                                TreeView::on_button_press_event(event);
                                return false;
                        }

                bool
                        CollectionTreeView::on_button_release_event (GdkEventButton* event)
                        {
                                g_atomic_int_set(&m_ButtonPressed, 0);
                                return false;
                        }

                bool
                        CollectionTreeView::on_event (GdkEvent * ev)
                        {
                                if( ev->type == GDK_BUTTON_PRESS )
                                {
                                        GdkEventButton * event = reinterpret_cast <GdkEventButton *> (ev);
                                        if( event->button == 3 )
                                        {
                                                int cell_x, cell_y ;
                                                TreeViewColumn *col ;
                                                TreePath path;

                                                if(get_path_at_pos (event->x, event->y, path, col, cell_x, cell_y))
                                                {
                                                        get_selection()->select( path );

                                                        Gtk::Menu * menu = dynamic_cast < Gtk::Menu* > (
                                                                        Util::get_popup(
                                                                                m_UIManager,
                                                                                (boost::format ("/popup-collectionlist-list-%1%/menu-collectionlist-list-%1%") % m_Name).str()
                                                                                ));

                                                        if (menu) // better safe than screwed
                                                        {
                                                                menu->popup (event->button, event->time);
                                                        }

                                                        return true;
                                                }
                                        }
                                }
                                return false;
                        }

                void
                        CollectionTreeView::place_collection_iter_real(
                            Gtk::TreeIter&  iter,
                            SQL::Row&       r,
                            gint64          id
                        )
                        {
                                std::string name = get<std::string>(r["name"]);

                                (*iter)[Columns.NameSortKey] = ustring(name).collate_key();
                                (*iter)[Columns.Text] = (boost::format ("<b><big>%s</big></b>") % Markup::escape_text(name).raw()).str();
                        } 

                void
                        CollectionTreeView::update_collection (SQL::Row & r, gint64 id)
                        {
#if 0
                                IdIterMap::iterator i = m_CollectionIterMap.find(id);
                                if (i == m_CollectionIterMap.end()) return;

                                TreeIter iter = (*i).second; 

                                (*iter)[Columns.NewCollection] = get<gint64>(r["collection_new"]);

                                place_collection_iter_real(iter, r, id);
#endif
                        } 

                Gtk::TreeIter
                        CollectionTreeView::place_collection(
                            SQL::Row&   r,
                            gint64      id
                        )
                        {
                                TreeIter iter = CollectionsTreeStore->append();
                                m_CollectionIterMap.insert(std::make_pair(id, iter));
                                CollectionsTreeStore->append(iter->children()); //create dummy/placeholder row for tracks

                                (*iter)[Columns.RowType] = CRT_ROW_COLLECTION; 
                                (*iter)[Columns.HasTracks] = false; 
                                (*iter)[Columns.Image] = m_DiscDefault; 
                                (*iter)[Columns.Id] = id; 

                                place_collection_iter_real(iter, r, id);

                                return iter;
                        }

                void
                        CollectionTreeView::collection_list_load ()
                        {
                                CollectionsTreeStore->clear ();

                                m_CollectionIterMap.clear();
                                m_TrackIterMap.clear();

                                SQL::RowV v;
                                services->get<Library>("mpx-service-library")->getSQL(v, "SELECT * FROM collection");

                                for(SQL::RowV::iterator i = v.begin(); i != v.end(); ++i)
                                {
                                        SQL::Row & r = *i; 
                                        place_collection(r, get<gint64>(r["id"]));
                                }
                        }

                void
                        CollectionTreeView::on_collection_updated(gint64 id)
                        {
#if 0
                                g_message(G_STRFUNC);

                                SQL::RowV v;
                                services->get<Library>("mpx-service-library")->getSQL(v, (boost::format("SELECT * FROM collection JOIN collection_artist ON collection.collection_artist_j = collection_artist.id WHERE collection.id = %lld;") % id).str());

                                g_return_if_fail(!v.empty());

                                SQL::Row & r = v[0];

                                update_collection (r, id); 
#endif
                        }

                void
                        CollectionTreeView::on_new_collection(gint64 id)
                        {
                                SQL::RowV v;
                                services->get<Library>("mpx-service-library")->getSQL(v, (boost::format("SELECT * FROM collection WHERE collection.id = %lld;") % id).str());

                                g_return_if_fail(!v.empty());

                                SQL::Row & r = v[0];

                                place_collection (r, id); 
                        }

                void
                        CollectionTreeView::on_new_track(gint64 collection_id, gint64 track_id)
                        {
                                if(m_CollectionIterMap.count(collection_id))
                                {
                                        TreeIter iter = m_CollectionIterMap[collection_id];
                                        if (((*iter)[Columns.HasTracks]))
                                        {
                                                SQL::RowV v;
                                                services->get<Library>("mpx-service-library")->getSQL(v, (boost::format ("SELECT * from track_view WHERE id = '%lld'") % track_id).str());
                                                SQL::Row & r = v[0];

                                                TreeIter child = CollectionsTreeStore->append(iter->children());
                                                (*child)[Columns.TrackArtist] = Markup::escape_text(get<std::string>(r["artist"]));
                                                (*child)[Columns.TrackTitle] = get<std::string>(r["title"]);
                                                (*child)[Columns.TrackNumber] = get<gint64>(r["track"]);
                                                (*child)[Columns.TrackLength] = get<gint64>(r["time"]);
                                                (*child)[Columns.TrackId] = get<gint64>(r["id"]);
                                                (*child)[Columns.RowType] = CRT_ROW_TRACK; 
                                        }
                                }
                                else
                                        g_warning("%s: Got new track without associated collection! Consistency error!", G_STRLOC);
                        }

                void
                        CollectionTreeView::on_collection_deleted(gint64 id)
                        {
#if 0
                            IdIterMap::iterator i = m_CollectionIterMap.find(id);
                            if( i != m_CollectionIterMap.end() )
                            {
                                std::string mbid = (*i->second)[Columns.MBID];
                                CollectionsTreeStore->erase( i->second );
                                IterSet & set = m_MBIDIterMap[mbid];
                                set.erase( i->second );
                                m_CollectionIterMap.erase( i );
                            }
#endif
                        }

                void
                        CollectionTreeView::on_track_deleted(gint64 id)
                        {
#if 0
                            IdIterMap::iterator i = m_TrackIterMap.find(id);
                            if( i != m_TrackIterMap.end() )
                            {
                                CollectionsTreeStore->erase( i->second );
                                m_TrackIterMap.erase( i );
                            }
#endif
                        }

                int
                        
                        CollectionTreeView::sortTracks(const TreeIter& iter_a, const TreeIter& iter_b)
                        {
                                gint64 trk_a = (*iter_a)[Columns.TrackNumber];
                                gint64 trk_b = (*iter_b)[Columns.TrackNumber];

                                return trk_a - trk_b;
                        }

                int
                        CollectionTreeView::slotSortStrictAlpha(const TreeIter& iter_a, const TreeIter& iter_b)
                        {
                                CollectionRowType rt_a = (*iter_a)[Columns.RowType];
                                CollectionRowType rt_b = (*iter_b)[Columns.RowType];

                                if((rt_a == CRT_ROW_COLLECTION) && (rt_b == CRT_ROW_COLLECTION))
                                {
                                        std::string coll_a = (*iter_a)[Columns.NameSortKey];
                                        std::string coll_b = (*iter_b)[Columns.NameSortKey];

                                        return coll_a.compare(coll_b);
                                }
                                else if((rt_a == CRT_ROW_TRACK) && (rt_b == CRT_ROW_TRACK))
                                {
                                        return sortTracks( iter_a, iter_b );
                                }

                                return 0;
                        }

                void
                        CollectionTreeView::cellDataFuncCover (CellRenderer * basecell, TreeModel::iterator const &iter)
                        {
                                TreePath path (iter);
                                CellRendererCairoSurface *cell = dynamic_cast<CellRendererCairoSurface*>(basecell);
                                if(path.get_depth() == CRT_ROW_COLLECTION)
                                {
                                        cell->property_visible() = true;
                                        cell->property_surface() = (*iter)[Columns.Image]; 
                                }
                                else
                                {
                                        cell->property_visible() = false;
                                }
                        }

                void
                        CollectionTreeView::cellDataFuncText1 (CellRenderer * basecell, TreeModel::iterator const &iter)
                        {
                                TreePath path (iter);
                                CellRendererText *cell1 = dynamic_cast<CellRendererText*>(basecell);
                                if(path.get_depth() == CRT_ROW_COLLECTION)
                                {
                                        cell1->property_visible() = true; 
                                        cell1->property_markup() = (*iter)[Columns.Text]; 
                                }
                                else
                                {
                                        cell1->property_visible() = false; 
                                }
                        }

                void
                        CollectionTreeView::cellDataFuncText2 (CellRenderer * basecell, TreeModel::iterator const &iter)
                        {
                                TreePath path (iter);
                                CellRendererCount *cell = dynamic_cast<CellRendererCount*>(basecell);
                                if(path.get_depth() == CRT_ROW_TRACK)
                                {
                                        cell->property_visible() = true; 
                                        cell->property_text() = (boost::format("%lld") % (*iter)[Columns.TrackNumber]).str();
                                }
                                else
                                {
                                        cell->property_visible() = false; 
                                }
                        }

                void
                        CollectionTreeView::cellDataFuncText3 (CellRenderer * basecell, TreeModel::iterator const &iter)
                        {
                                TreePath path (iter);
                                CellRendererText *cell = dynamic_cast<CellRendererText*>(basecell);
                                if(path.get_depth() == CRT_ROW_TRACK)
                                {
                                        cell->property_visible() = true; 
                                        cell->property_markup() = Markup::escape_text((*iter)[Columns.TrackTitle]);
                                }
                                else
                                {
                                        cell->property_visible() = false; 
                                }
                        }

                void
                        CollectionTreeView::cellDataFuncText4 (CellRenderer * basecell, TreeModel::iterator const &iter)
                        {
                                TreePath path (iter);
                                CellRendererText *cell = dynamic_cast<CellRendererText*>(basecell);
                                if(path.get_depth() == CRT_ROW_TRACK)
                                {
                                        cell->property_visible() = true; 
                                        gint64 Length = (*iter)[Columns.TrackLength];
                                        cell->property_text() = (boost::format ("%d:%02d") % (Length / 60) % (Length % 60)).str();
                                }
                                else
                                {
                                        cell->property_visible() = false; 
                                }
                        }

                void
                        CollectionTreeView::cellDataFuncText5 (CellRenderer * basecell, TreeModel::iterator const &iter)
                        {
                                TreePath path (iter);
                                CellRendererText *cell = dynamic_cast<CellRendererText*>(basecell);
                                if(path.get_depth() == CRT_ROW_TRACK)
                                {
                                        cell->property_visible() = true; 
                                        cell->property_markup() = (*iter)[Columns.TrackArtist];
                                }
                                else
                                {
                                        cell->property_visible() = false; 
                                }
                        }

                void
                        CollectionTreeView::rb_sourcelist_expander_cell_data_func (GtkTreeViewColumn *column,
                                        GtkCellRenderer   *cell,
                                        GtkTreeModel      *model,
                                        GtkTreeIter       *iter,
                                        gpointer           data) 
                        {
                                if (gtk_tree_model_iter_has_child (model, iter))
                                {
                                        GtkTreePath *path;
                                        gboolean     row_expanded;

                                        path = gtk_tree_model_get_path (model, iter);
                                        row_expanded = gtk_tree_view_row_expanded (GTK_TREE_VIEW (column->tree_view), path);
                                        gtk_tree_path_free (path);

                                        g_object_set (cell,
                                                        "visible", TRUE,
                                                        "render", TRUE,
                                                        "expander-style", row_expanded ? GTK_EXPANDER_EXPANDED : GTK_EXPANDER_COLLAPSED,
                                                        NULL);
                                } else {
                                        g_object_set (cell, "visible", FALSE, "render", FALSE, NULL);
                                }
                        }

                bool
                        CollectionTreeView::collection_visible_func (TreeIter const& iter)
                        {
                                TreePath path = CollectionsTreeStore->get_path(iter);

                                if( path.size() > 1 ) 
                                {
                                        return true;
                                }

                                if( m_FilterText.empty() ) 
                                {
                                        return true;
                                }

                                bool truthvalue = true;

                                return truthvalue && Util::match_keys (ustring((*iter)[Columns.Text]).lowercase(), m_FilterText); 
                        }

                void
                        CollectionTreeView::update_collection_count_display ()
                        {
                                TreeNodeChildren::size_type n1 = CollectionsTreeStoreFilter->children().size();
                                TreeNodeChildren::size_type n2 = CollectionsTreeStore->children().size();
                                if( m_LabelShowing )
                                {
                                    m_LabelShowing->set_text ((boost::format (_("%lld of %lld")) % n1 % n2).str());
                                }
                        }

                void
                        CollectionTreeView::on_row_added_or_deleted ()
                        {
                                update_collection_count_display ();
                        }

} // end namespace MPX 
