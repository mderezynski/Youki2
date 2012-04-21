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
#include "mpx/widgets/cell-renderer-album-data.hh"
#include "mpx/widgets/cell-renderer-vbox.hh"
#include "mpx/widgets/cell-renderer-expander.h"
#include "mpx/widgets/rounded-layout.hh"
#include "mpx/widgets/timed-confirmation.hh"

#include "mpx/com/view-albums.hh"
#include "mpx/com/album-info-window.hh"

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

        const char ui_albums_popup [] =
                "<ui>"
                ""
                "<menubar name='popup-albumlist-%1%'>"
                ""
                "   <menu action='dummy' name='menu-albumlist-%1%'>"
                "       <menuitem action='action-album-info-%1%'/>"
                "       <placeholder name='action-album-placeholder-%1%'/>"
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


        // Release Types

        ReleaseType
                determine_release_type (const std::string& type)
                {
                        if( type == "album" )
                                return RT_ALBUM;

                        if( type == "single" )
                                return RT_SINGLE;

                        if( type == "compilation" )
                                return RT_COMPILATION;

                        if( type == "ep" )
                                return RT_EP;

                        if( type == "live" )
                                return RT_LIVE;

                        if( type == "remix" )
                                return RT_REMIX;

                        if( type == "soundtrack" )
                                return RT_SOUNDTRACK;

                        return RT_OTHER;
                }

        std::string
                get_release_string (ReleaseType rt, bool plural = false)
                {
                            if( rt == RT_ALBUM )
                                return plural ? N_("Albums") : N_("Album");

                            if( rt == RT_SINGLE )
                                return plural ? N_("Singles") : N_("Single");

                            if( rt == RT_COMPILATION )
                                return plural ? N_("Compilations") : N_("Compilation");

                            if( rt == RT_EP )
                                return plural ? N_("EPs") : N_("EP");

                            if( rt == RT_LIVE )
                                return plural ? N_("Live Recordings") : N_("Live Recording");

                            if( rt == RT_REMIX )
                                return plural ? N_("Remixes") : N_("Remix");

                            if( rt == RT_SOUNDTRACK )
                                return plural ? N_("Soundtracks") : N_("Soundtrack");

                        return "";
                }

}

namespace MPX
{
                AlbumTreeView::AlbumTreeView(
                          const Glib::RefPtr<Gnome::Glade::Xml>&  xml
                        , const std::string&                      name
                        , const std::string&                      name_showing_label
                        , const std::string&                      name_search_entry
                        , const std::string&                      name_search_plugin_ui_alignment
                        , const std::string&                      name_search_plugin_choice_cbox
                        , Glib::RefPtr<Gtk::UIManager>            ui_manager
                )
                : WidgetLoader<Gtk::TreeView>(xml,name)
                , m_Name(name)
                , m_ButtonPressed(false)
                {
                        m_DiscDefault_DND = IconTheme::get_default()->load_icon("gnome-dev-cdrom-audio", 128, Gtk::ICON_LOOKUP_USE_BUILTIN);
                        m_DiscDefault = Util::cairo_image_surface_from_pixbuf(m_DiscDefault_DND->scale_simple(90,90,Gdk::INTERP_BILINEAR));

                        Options.Flags = ALBUMS_STATE_NO_FLAGS;
                        Options.Type = RT_ALL;
                        Options.HighlightMode = HIGHLIGHT_EQUAL;

                        const std::string image_base_path = build_filename( DATA_DIR, "images" );

                        m_Emblem[EM_COMPILATION] = Gdk::Pixbuf::create_from_file(
                                        build_filename(
                                               image_base_path,
                                               "emblem-compilation.png" 
                        ));

                        m_Emblem[EM_SOUNDTRACK] = Gdk::Pixbuf::create_from_file(
                                        build_filename(
                                               image_base_path,
                                               "emblem-soundtrack.png" 
                        ));

/*
                        services->get<Library>("mpx-service-library")->signal_new_album().connect(
                            sigc::mem_fun(
                                *this,
                                &AlbumTreeView::on_new_album
                        ));

                        services->get<Library>("mpx-service-library")->signal_new_track().connect(
                            sigc::mem_fun(
                                *this,
                                &AlbumTreeView::on_new_track
                        ));

                        services->get<Library>("mpx-service-library")->signal_album_deleted().connect(
                            sigc::mem_fun(
                                *this,
                                &AlbumTreeView::on_album_deleted
                        ));

                        services->get<Library>("mpx-service-library")->signal_track_deleted().connect(
                            sigc::mem_fun(
                                *this,
                                &AlbumTreeView::on_track_deleted
                        ));

                        services->get<Library>("mpx-service-library")->signal_track_updated().connect(
                            sigc::mem_fun(
                                *this,
                                &AlbumTreeView::on_track_updated
                        ));

                        services->get<Library>("mpx-service-library")->signal_album_updated().connect(
                            sigc::mem_fun(
                                *this,
                                &AlbumTreeView::on_album_updated
                        ));

                        services->get<Library>("mpx-service-library")->signal_reload().connect(
                            sigc::mem_fun(
                                *this,
                                &AlbumTreeView::album_list_load
                        ));

                        services->get<Covers>("mpx-service-covers")->signal_got_cover().connect(
                            sigc::mem_fun(
                                *this,
                                &AlbumTreeView::on_got_cover
                        )); 
*/

                        if( !name_showing_label.empty() )
                            m_LabelShowing = new RoundedLayout(xml, name_showing_label);
                        else
                            m_LabelShowing = 0;

                        set_show_expanders( false );
                        set_level_indentation( 50 );

                        TreeViewColumn * col = manage (new TreeViewColumn());

                        { // Album Expander

                                GtkCellRenderer * renderer = gossip_cell_renderer_expander_new ();

                                gtk_tree_view_column_pack_start(
                                      col->gobj()
                                    , renderer
                                    , FALSE
                                );

                                gtk_tree_view_column_set_cell_data_func(
                                      col->gobj()
                                    , renderer
                                    , GtkTreeCellDataFunc(rb_sourcelist_expander_cell_data_func)
                                    , this
                                    , NULL
                                );
                        }



                        { // Album Cover

                                CellRendererCairoSurface *cell = manage (new CellRendererCairoSurface);

                                col->pack_start( *cell, false );

                                col->set_cell_data_func(
                                                *cell,
                                                sigc::mem_fun(
                                                        *this,
                                                        &AlbumTreeView::cellDataFuncCover
                                ));

                                cell->property_xpad() = 1;
                                cell->property_ypad() = 4;
                                cell->property_yalign() = 0.;
                                cell->property_xalign() = 0.;
                        }



                        { // Album 

                                CellRendererAlbumData *cell = manage( new CellRendererAlbumData );

                                col->pack_start( *cell, true );

                                col->set_cell_data_func(
                                                *cell,
                                                sigc::mem_fun(
                                                        *this,
                                                        &AlbumTreeView::cellDataFuncText1
                                ));
                        }



                        { // Add/Remove

                                CellRendererPixbuf *cell = manage (new CellRendererPixbuf);

                                col->pack_start( *cell, false );

                                col->set_cell_data_func(
                                                *cell,
                                                sigc::mem_fun(
                                                        *this,
                                                        &AlbumTreeView::cellDataFuncAdd
                                ));

                                cell->property_width() = 16;

                                m_CellAdd = cell;
                        }



                        { // Track Count 

                                CellRendererCount *cell = manage (new CellRendererCount);

                                col->pack_start( *cell, false );

                                col->set_cell_data_func(
                                                *cell,
                                                sigc::mem_fun(
                                                        *this,
                                                        &AlbumTreeView::cellDataFuncText2
                                ));

                                cell->property_box() = BOX_NORMAL;
                        }



                        { // Track Title
        
                                CellRendererText *cell = manage (new CellRendererText);

                                col->pack_start( *cell, false );

                                col->set_cell_data_func(
                                                *cell,
                                                sigc::mem_fun(
                                                        *this,
                                                        &AlbumTreeView::cellDataFuncText3
                               ));
                        }



                        { // Track Time

                                CellRendererText *cell = manage (new CellRendererText);

                                col->pack_start( *cell, false );

                                col->set_cell_data_func(
                                                *cell,
                                                sigc::mem_fun(
                                                        *this,
                                                        &AlbumTreeView::cellDataFuncText4
                                ));
                        }



                        { // Track Artist

                                CellRendererText *cell = manage (new CellRendererText);

                                col->pack_start( *cell, false );

                                col->set_cell_data_func(
                                                *cell,
                                                sigc::mem_fun(
                                                        *this,
                                                        &AlbumTreeView::cellDataFuncText5
                                ));

                                cell->property_xalign() = 0.;
                                cell->property_xpad() = 2;
                        }

                        append_column(*col);


                        AlbumsTreeStore = Gtk::TreeStore::create(Columns);
                        AlbumsTreeStoreFilter = Gtk::TreeModelFilter::create(AlbumsTreeStore);

                        // Filter Widgets   

                        xml->get_widget(name_search_entry, m_FilterEntry);
                        xml->get_widget(name_search_plugin_ui_alignment, m_FilterPluginUI_Alignment);

                        // Filter Plugins

                        xml->get_widget(name_search_plugin_choice_cbox, m_FilterPluginsCBox);
 
                        {
                                Plugin_p p (new MPX::ViewAlbumsFilterPlugin::TextMatch( AlbumsTreeStore, Columns ));
                                p->signal_refilter().connect(
                                                sigc::mem_fun(
                                                        *this,
                                                        &AlbumTreeView::refilter
                                ));
                                m_FilterPlugins.push_back( p );
                        } 

                        {
                                Plugin_p p (new MPX::ViewAlbumsFilterPlugin::LFMTopAlbums( AlbumsTreeStore, Columns ));
                                p->signal_refilter().connect(
                                                sigc::mem_fun(
                                                        *this,
                                                        &AlbumTreeView::refilter
                                ));
                                m_FilterPlugins.push_back( p );
                        } 

                        {
                                Plugin_p p (new MPX::ViewAlbumsFilterPlugin::LFMSimilarArtists( AlbumsTreeStore, Columns ));
                                p->signal_refilter().connect(
                                                sigc::mem_fun(
                                                        *this,
                                                        &AlbumTreeView::refilter
                                ));
                                m_FilterPlugins.push_back( p );
                        } 

                        m_ConnFilterEntry_Changed = m_FilterEntry->signal_changed().connect(
                                        sigc::mem_fun(
                                                *this,
                                                &AlbumTreeView::on_filter_entry_changed
                        ));

                        m_ConnFilterEntry_Activate = m_FilterEntry->signal_activate().connect(
                                        sigc::mem_fun(
                                                *this,
                                                &AlbumTreeView::on_filter_entry_activate
                        ));

                        m_FilterPluginUI = 0;
                        m_FilterPluginsCBox->set_active( 0 );

                        m_FilterPluginsCBox->signal_changed().connect(
                                sigc::mem_fun(
                                        *this,
                                        &AlbumTreeView::on_plugin_cbox_changed
                        ));

                        on_plugin_cbox_changed();

                        // UI Manager

                        m_UIManager = ui_manager;
                        m_ActionGroup = Gtk::ActionGroup::create ((boost::format ("Actions-%s") % name).str());
                        m_ActionGroup->add(Gtk::Action::create("dummy","dummy"));
                        m_ActionGroup->add(

                                        Gtk::Action::create(
                                                (boost::format ("action-album-info-%s") % m_Name).str(),
                                                Gtk::StockID (GTK_STOCK_INFO),
                                                _("Album _Info")
                                                ),

                                        sigc::mem_fun(
                                                *this,
                                                &AlbumTreeView::on_album_show_info
                        ));

                        m_UIManager->insert_action_group(m_ActionGroup);
                        m_UIManager->add_ui_from_string((boost::format(ui_albums_popup) % m_Name).str());

                        // DND

                        std::vector<TargetEntry> Entries;
                        Entries.push_back(TargetEntry("mpx-album", TARGET_SAME_APP, 0x80));
                        Entries.push_back(TargetEntry("mpx-track", TARGET_SAME_APP, 0x81));
                        drag_source_set(Entries); 

                        Entries = std::vector<TargetEntry>();
                        Entries.push_back(TargetEntry("image/png"));
                        Entries.push_back(TargetEntry("image/jpeg"));
                        Entries.push_back(TargetEntry("text/x-moz-url"));
                        drag_dest_set(Entries, DEST_DEFAULT_ALL);
                        drag_dest_add_uri_targets();

                        // Init album list

                        gtk_widget_realize(GTK_WIDGET(gobj()));
                        album_list_load ();
                        set_model(AlbumsTreeStoreFilter);

                        // Setup sorting

                        AlbumsTreeStoreFilter->set_visible_func(
                                        sigc::mem_fun(
                                                *this,
                                                &AlbumTreeView::album_visible_func
                                                ));

                        AlbumsTreeStoreFilter->signal_row_inserted().connect((
                                                sigc::hide(sigc::hide(sigc::mem_fun(
                                                                        *this,
                                                                        &AlbumTreeView::on_row_added_or_deleted
                                                                        )))));

                        AlbumsTreeStoreFilter->signal_row_deleted().connect((
                                                sigc::hide(sigc::mem_fun(
                                                                *this,
                                                                &AlbumTreeView::on_row_added_or_deleted
                                                                ))));

                        AlbumsTreeStore->set_sort_func(0 , sigc::mem_fun( *this, &AlbumTreeView::slotSortAlpha ));
                        AlbumsTreeStore->set_sort_func(1 , sigc::mem_fun( *this, &AlbumTreeView::slotSortDate ));
                        AlbumsTreeStore->set_sort_func(2 , sigc::mem_fun( *this, &AlbumTreeView::slotSortRating ));
                        AlbumsTreeStore->set_sort_func(3 , sigc::mem_fun( *this, &AlbumTreeView::slotSortStrictAlpha ));
                        AlbumsTreeStore->set_sort_func(4 , sigc::mem_fun( *this, &AlbumTreeView::slotSortPlayScore ));
                        AlbumsTreeStore->set_sort_column(0, Gtk::SORT_ASCENDING);

                        set_search_column(Columns.Album);
                }

                void
                        AlbumTreeView::on_filter_entry_changed ()
                        {
                                m_FilterText = m_FilterEntry->get_text().lowercase();
                                m_FilterPlugin_Current->on_filter_changed( m_FilterText );
                        }
        
                void
                        AlbumTreeView::on_filter_entry_activate ()
                        {
                                m_FilterPlugin_Current->on_filter_issued( m_FilterText );
                        }

                void
                        AlbumTreeView::refilter ()
                        {
                                m_MouseOverIter.reset();
                                AlbumsTreeStoreFilter->refilter();
                                update_album_count_display();
                        }

            void
                        AlbumTreeView::on_plugin_cbox_changed ()
                        {
                                if( m_FilterPluginUI && m_FilterPluginUI_Alignment->get_child() )
                                {
                                    m_FilterPluginUI_Alignment->remove();
                                }

                                m_FilterPlugin_Current = m_FilterPlugins[m_FilterPluginsCBox->get_active_row_number()];
                                m_FilterPluginUI = m_FilterPlugin_Current->get_ui();
                                if( m_FilterPluginUI )
                                    m_FilterPluginUI_Alignment->add( *m_FilterPluginUI );

                                m_FilterEntry->set_text("");
                        }

                bool
                        AlbumTreeView::album_visible_func (TreeIter const& iter)
                        {
                                TreePath path = AlbumsTreeStore->get_path(iter);

                                if( path.size() == 1 && (Options.Flags & ALBUMS_STATE_SHOW_NEW) && !(*iter)[Columns.NewAlbum])
                                {
                                        return false;
                                } 

                                if( path.size() == 1 && !(Options.Type & (*iter)[Columns.RT]))
                                {
                                        return false;
                                }

                                if( path.size() > 1 ) // track row 
                                {
                                        return true;
                                }

                                return m_FilterPlugin_Current->filter_delegate( iter );
                        }

                void
                        AlbumTreeView::on_row_expanded (const TreeIter & iter_filter,const TreePath &path) 
                        {
                                TreeIter iter = AlbumsTreeStoreFilter->convert_iter_to_child_iter(iter_filter);
                                if(!(*iter)[Columns.HasTracks])
                                {
                                        GtkTreeIter children;
                                        bool has_children = (gtk_tree_model_iter_children(GTK_TREE_MODEL(AlbumsTreeStore->gobj()), &children, const_cast<GtkTreeIter*>(iter->gobj())));

                                        std::string album_artist_mb_id = (*iter)[Columns.AlbumArtistMBID];
                                        std::string track_artist_mb_id;

                                        SQL::RowV v;
                                        services->get<Library>("mpx-service-library")->getSQL(v, (boost::format("SELECT * FROM track_view WHERE album_j = %lld ORDER BY track;") % gint64((*iter)[Columns.AlbumId])).str());

                                        for(SQL::RowV::iterator i = v.begin(); i != v.end(); ++i)
                                        {
                                                SQL::Row & r = *i;

                                                TreeIter child = AlbumsTreeStore->append(iter->children());

                                                gint64 id = get<gint64>(r["id"]);

                                                if(r.count("mb_artist_id"))
                                                {
                                                        track_artist_mb_id = get<std::string>(r["mb_artist_id"]);
                                                }

                                                if( album_artist_mb_id != track_artist_mb_id )
                                                {
                                                        (*child)[Columns.TrackArtist] = Markup::escape_text(get<std::string>(r["artist"]));
                                                }

                                                (*child)[Columns.TrackTitle] = get<std::string>(r["title"]);
                                                (*child)[Columns.TrackNumber] = get<gint64>(r["track"]);
                                                (*child)[Columns.TrackLength] = get<gint64>(r["time"]);
                                                (*child)[Columns.TrackId] = get<gint64>(r["id"]);
                                                (*child)[Columns.RowType] = ROW_TRACK; 

                                                m_Track_Iter_Map.insert(std::make_pair(id, child));
                                        }

                                        if(v.size())
                                        {
                                                (*iter)[Columns.HasTracks] = true;
                                                if(has_children)
                                                {
                                                        gtk_tree_store_remove(GTK_TREE_STORE(AlbumsTreeStore->gobj()), &children);
                                                } 
                                                else
                                                        g_warning("%s:%d : No placeholder row present, state seems corrupted.", __FILE__, __LINE__);
                                        }

                                }
                        }

                void
                        AlbumTreeView::on_drag_data_get (const Glib::RefPtr<Gdk::DragContext>& context, SelectionData& selection_data, guint info, guint time)
                        {
                                if(m_DragAlbumId)
                                {
                                        gint64 * id = new gint64(m_DragAlbumId.get());
                                        selection_data.set("mpx-album", 8, reinterpret_cast<const guint8*>(id), 8);
                                }
                                else if(m_DragTrackId)
                                {
                                        gint64 * id = new gint64(m_DragTrackId.get());
                                        selection_data.set("mpx-track", 8, reinterpret_cast<const guint8*>(id), 8);
                                }

                                m_DragAlbumMBID.reset();
                                m_DragAlbumId.reset();
                                m_DragTrackId.reset();
                        }

                void
                        AlbumTreeView::on_drag_begin (const Glib::RefPtr<Gdk::DragContext>& context) 
                        {
                                if(m_DragAlbumId)
                                {
                                        if(m_DragAlbumMBID)
                                        {
                                                Cairo::RefPtr<Cairo::ImageSurface> surface;

                                                services->get<Covers>("mpx-service-covers")->fetch(m_DragAlbumMBID.get(), surface, COVER_SIZE_DEFAULT);

                                                if( surface )
                                                {
                                                        Gdk::Color c = get_style()->get_black();
                                                        surface = Util::cairo_image_surface_round(surface, 28.); 
                                                        Util::cairo_image_surface_rounded_border(surface, .5, 28., c.get_red_p(), c.get_green_p(), c.get_blue_p(), 1.);
                                                        Glib::RefPtr<Gdk::Pixbuf> CoverPixbuf = Util::cairo_image_surface_to_pixbuf(surface);
                                                        drag_source_set_icon(CoverPixbuf->scale_simple(128,128,Gdk::INTERP_BILINEAR));
                                                        return;
                                                }
                                        }

                                        drag_source_set_icon(m_DiscDefault_DND->scale_simple(128,128,Gdk::INTERP_BILINEAR));
                                }
                                else
                                {
                                        Glib::RefPtr<Gdk::Pixmap> pix = create_row_drag_icon(m_PathButtonPress);
                                        drag_source_set_icon(pix->get_colormap(), pix, Glib::RefPtr<Gdk::Bitmap>(0));
                                }
                        }

                bool
                        AlbumTreeView::on_drag_motion (const Glib::RefPtr<Gdk::DragContext>& context_, int x, int y, guint time)
                        {
                            return true;
                        }

                void
                        AlbumTreeView::on_drag_data_received (const Glib::RefPtr<Gdk::DragContext>&, int x, int y,
                                                const Gtk::SelectionData& data, guint, guint)
                        {
                                std::string uri;

                                if( data.get_data_type() == "text/x-moz-url" )
                                {
                                        gchar *out;
                                        gsize bytes_read = 0;
                                        gsize bytes_written = 0;
                                        GError *error = NULL;

                                        out = g_convert ((char*)data.get_data(), data.get_length(), "UTF-8", "UTF-16", 
                                           &bytes_read, &bytes_written, &error);

                                        std::string text = out;
                                        g_free(out);

                                        using boost::algorithm::split;
                                        using boost::algorithm::is_any_of;
                                        using boost::algorithm::replace_all;

                                        replace_all (text, "\r", "");

                                        StrV v;
                                        split (v, text, is_any_of ("\n"));
    
                                        uri = v[0]; 
                                }
                                else if( data.get_data_type() == "text/uri-list" )
                                {
                                        std::vector<Glib::ustring> uris = data.get_uris();

                                        if( uris.empty() )
                                        {
                                            uri = data.get_data_as_string(); 
                                        }
                                        else
                                        {
                                            uri = uris[0];
                                        }
                                }

                                try{
                                    TreeModel::Path path;
                                    TreeViewDropPosition pos;
                                    if( get_dest_row_at_pos (x, y, path, pos) )
                                    {
                                        Glib::RefPtr<Gdk::Pixbuf> cover = Util::get_image_from_uri( uri ); 
                                        Cairo::RefPtr<Cairo::ImageSurface> surface = Util::cairo_image_surface_from_pixbuf( cover->scale_simple( 90, 90, Gdk::INTERP_BILINEAR) );
                                        surface = Util::cairo_image_surface_round(surface, 6.);
                                        Gdk::Color c = get_style()->get_black();
                                        Util::cairo_image_surface_rounded_border(surface, .5, 6., c.get_red_p(), c.get_green_p(), c.get_blue_p(), 1.);

                                        TreeIter iter = AlbumsTreeStoreFilter->convert_iter_to_child_iter( AlbumsTreeStoreFilter->get_iter( path ) );
                                        switch( (*iter)[Columns.RT] ) 
                                        {
                                            case RT_COMPILATION:
                                                (*iter)[Columns.Image] = Util::cairo_image_surface_overlay( surface, Util::cairo_image_surface_from_pixbuf(m_Emblem[EM_COMPILATION]), 0., 0., 1.); 
                                                break;

                                            case RT_SOUNDTRACK:
                                                (*iter)[Columns.Image] = Util::cairo_image_surface_overlay( surface, Util::cairo_image_surface_from_pixbuf(m_Emblem[EM_SOUNDTRACK]), 0., 0., 1.); 
                                                break;

                                            default:
                                                (*iter)[Columns.Image] = surface;
                                                break;
                                        }
    
                                       services->get<Covers>("mpx-service-covers")->cache_artwork( (*iter)[Columns.AlbumMBID], cover ); 
                                    }
                                } catch(...) {
                                    g_message("%s: Error saving Pixbuf", G_STRLOC);
                                }
                        }

                void
                        AlbumTreeView::run_rating_comment_dialog(int rating, gint64 id)
                        {
                                Gtk::TextView   * textview;
                                Gtk::Dialog     * dialog;

                                m_Xml->get_widget("albums-textview-comment", textview);
                                m_Xml->get_widget("albums-dialog-rate-and-comment", dialog);
                                Glib::RefPtr<Gtk::TextBuffer> buffer = textview->get_buffer();
                                buffer->set_text("");

                                int response = dialog->run();

                                if( response == GTK_RESPONSE_OK )
                                {
                                        Glib::RefPtr<Gtk::TextBuffer> buffer = textview->get_buffer();
                                        Glib::ustring text = buffer->get_text();

                                        services->get<Library>("mpx-service-library")->albumAddNewRating(id, rating, text);
                                }

                                dialog->hide();
                        }

                bool
                        AlbumTreeView::on_button_press_event (GdkEventButton* event)
                        {
                                int cell_x, cell_y ;
                                TreeViewColumn *col ;

                                if(get_path_at_pos (event->x, event->y, m_PathButtonPress, col, cell_x, cell_y))
                                {
                                        TreeIter iter = AlbumsTreeStore->get_iter(AlbumsTreeStoreFilter->convert_path_to_child_path(m_PathButtonPress));
                                        if(m_PathButtonPress.get_depth() == ROW_ALBUM)
                                        {
                                                m_DragAlbumMBID = (*iter)[Columns.AlbumMBID];
                                                m_DragAlbumId = (*iter)[Columns.AlbumId];
                                                m_DragTrackId.reset(); 

                                                g_atomic_int_set(&m_ButtonPressed, 1);

                                                if( (cell_x >= 114) && (cell_x <= 176) && (cell_y > 80) && (cell_y < 92))
                                                {
                                                        int rating = ((cell_x - 114)+7) / 12;

                                                        (*iter)[Columns.Rating] = rating;  
                                                        AlbumInfo_pt((*iter)[Columns.RenderData])->Rating = rating;
                                                        queue_draw ();

                                                        run_rating_comment_dialog(rating, m_DragAlbumId.get());

                                                        rating = services->get<Library>("mpx-service-library")->albumGetMeanRatingValue(m_DragAlbumId.get());
                                                        (*iter)[Columns.Rating] = rating;  
                                                        AlbumInfo_pt((*iter)[Columns.RenderData])->Rating = rating;
                                                        queue_draw ();
                                                }
                                        }
                                        else
                                        if(m_PathButtonPress.get_depth() == ROW_TRACK)
                                        {
                                            m_DragAlbumMBID.reset(); 
                                            m_DragAlbumId.reset();
                                            m_DragTrackId = (*iter)[Columns.TrackId];

                                            int start_pos, width;

                                            if( m_MouseOverIter && m_MouseOverIter.get() == AlbumsTreeStoreFilter->convert_child_iter_to_iter( iter ) )
                                            {
                                                if( get_column(0)->get_cell_position( *m_CellAdd, start_pos, width ) )
                                                {
                                                    if( cell_x >= 54 && cell_x <= 64 )
                                                    {
                                                        IdV v (1, m_DragTrackId.get());
                                                        Signals.PlayTracks.emit(v, false);
                                                        m_MouseOverIter.reset();
                                                        queue_draw();
                                                        return true;
                                                    }
                                                }
                                            }
                                        }

                                }
                                TreeView::on_button_press_event(event);
                                return false;
                        }

                bool
                        AlbumTreeView::on_button_release_event (GdkEventButton* event)
                        {
                                g_atomic_int_set(&m_ButtonPressed, 0);
                                return false;
                        }

                bool
                        AlbumTreeView::on_motion_notify_event (GdkEventMotion* event)
                        {
                            int x, y;
                            GdkModifierType state;

                            if (event->is_hint)
                            {
                                gdk_window_get_pointer (event->window, &x, &y, &state);
                            }
                            else
                            {
                                x = event->x;
                                y = event->y;
                            }

                            m_motion_x = x;
                            m_motion_y = y; 

                            TreePath path;
                            TreeViewColumn * col;
                            int cell_x, cell_y;

                            if( get_path_at_pos (event->x, event->y, path, col, cell_x, cell_y) )
                            {
                                if( path.get_depth() == ROW_TRACK )
                                {
                                    m_MouseOverIter = AlbumsTreeStoreFilter->get_iter( path );
                                    queue_draw ();
                                    return false;
                                }
                            }

                            m_MouseOverIter.reset();
                            queue_draw();
                            return false;
                        } 

                bool
                        AlbumTreeView::on_event (GdkEvent * ev)
                        {
                                if( ev->type == GDK_KEY_PRESS )
                                {
                                        GdkEventKey * event = reinterpret_cast <GdkEventKey *> (ev);
                                        if( (event->keyval == GDK_KP_Enter) || (event->keyval == GDK_ISO_Enter)
                                           || (event->keyval == GDK_3270_Enter) || (event->keyval == GDK_Return))
                                        {
                                                if( get_selection()->count_selected_rows() )
                                                {
                                                        TreeIter iter = AlbumsTreeStoreFilter->convert_iter_to_child_iter(get_selection()->get_selected());
                                                        TreePath path = AlbumsTreeStore->get_path( iter );

                                                        bool play = event->state & GDK_CONTROL_MASK;

                                                        if( path.get_depth() == ROW_ALBUM )
                                                        {
                                                                gint64 id = (*iter)[Columns.AlbumId];
                                                                Signals.PlayAlbum.emit(id, !play);
                                                        }
                                                        else
                                                        {
                                                                gint64 id = (*iter)[Columns.TrackId];
                                                                IdV v (1, id);
                                                                Signals.PlayTracks.emit(v, !play);
                                                        }
                                                }
                                        }
                                }
                                else if( ev->type == GDK_BUTTON_PRESS )
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
                                                                                (boost::format ("/popup-albumlist-%s/menu-albumlist-%s") % m_Name % m_Name).str()
                                                                                ));

                                                        if (menu) // better safe than screwed
                                                        {
                                                                menu->popup (event->button, event->time);
                                                        }

                                                        return true;
                                                }
                                        }
                                }
                                else if( ev->type == GDK_2BUTTON_PRESS)
                                {
                                        GdkEventButton * event = reinterpret_cast <GdkEventButton *> (ev);
                                        if( event->button == 1 )
                                        {
                                                int cell_x, cell_y ;
                                                TreeViewColumn *col ;
                                                TreePath path;

                                                if(get_path_at_pos (event->x, event->y, path, col, cell_x, cell_y))
                                                {
                                                        TreeIter iter = AlbumsTreeStore->get_iter (AlbumsTreeStoreFilter->convert_path_to_child_path(path));
                                                        bool play = event->state & GDK_CONTROL_MASK;
                                                        if( path.get_depth() == ROW_ALBUM )
                                                        {
                                                                gint64 id = (*iter)[Columns.AlbumId];
                                                                Signals.PlayAlbum.emit(id, !play);
                                                        }
                                                        else
                                                        {
                                                                gint64 id = (*iter)[Columns.TrackId];
                                                                IdV v (1, id);
                                                                Signals.PlayTracks.emit(v, !play);
                                                        }
                                                }
                                        }
                                }

                                return false;
                        }


                void
                        AlbumTreeView::on_album_show_info ()
                        {
                                AlbumInfoWindow * d = AlbumInfoWindow::create(
                                                (*get_selection()->get_selected())[Columns.AlbumId],
                                                *(services->get<Library>("mpx-service-library").get()),
                                                *(services->get<Covers>("mpx-service-covers").get())
                                                );
                        }

                void
                        AlbumTreeView::on_got_cover(const std::string& mbid)
                        {
                                Cairo::RefPtr<Cairo::ImageSurface> surface;
                                services->get<Covers>("mpx-service-covers")->fetch(mbid, surface, COVER_SIZE_ALBUM);

                                if( surface )
                                {
                                        surface = Util::cairo_image_surface_round(surface, 6.);
                                        Gdk::Color c = get_style()->get_black();
                                        Util::cairo_image_surface_rounded_border(surface, .5, 6., c.get_red_p(), c.get_green_p(), c.get_blue_p(), 1.);

                                        IterSet & set = m_Album_MBID_Iter_Map[mbid];
                                        for(IterSet::iterator i = set.begin(); i != set.end(); ++i)
                                        {       
                                                switch( (*(*i))[Columns.RT] ) 
                                                {
                                                    case RT_COMPILATION:
                                                        (*(*i))[Columns.Image] = Util::cairo_image_surface_overlay( surface, Util::cairo_image_surface_from_pixbuf(m_Emblem[EM_COMPILATION]), 0., 0., 1.); 
                                                        break;

                                                    case RT_SOUNDTRACK:
                                                        (*(*i))[Columns.Image] = Util::cairo_image_surface_overlay( surface, Util::cairo_image_surface_from_pixbuf(m_Emblem[EM_SOUNDTRACK]), 0., 0., 1.); 
                                                        break;

                                                    default:
                                                        (*(*i))[Columns.Image] = surface;
                                                        break;
                                                }
                                        }
                                }
                        }

                void
                        AlbumTreeView::place_album_iter_real(
                            Gtk::TreeIter&  iter,
                            SQL::Row&       r,
                            gint64          id
                        )
                        {
                                std::string artist;
                                std::string artist_sort;
                                std::string asin;
                                std::string country;
                                std::string genre;
                                std::string mbid;
                                std::string mbid_artist;
                                std::string rt_string;
                                std::string type;
                                std::string year; 

                                double      playscore   = 0;
                                gint64      quality     = -1; //FIXME: Use boost::optional
                                gint64      rating      = 0;

                                ReleaseType rt;

                                Track_sp p (new Track);
        
                                Track & track = *(p.get());

                                std::string album = get<std::string>(r["album"]);
                                track[ATTRIBUTE_ALBUM] = album;

                                try{
                                        rating = services->get<Library>("mpx-service-library")->albumGetMeanRatingValue(id);
                                        track[ATTRIBUTE_RATING] = rating;
                                } catch( std::runtime_error )
                                {
                                }

                                if(r.count("album_quality"))
                                {
                                        quality = get<gint64>(r["album_quality"]);
                                        track[ATTRIBUTE_QUALITY] = quality;
                                }

                                if(r.count("album_genre"))
                                {
                                        genre = get<std::string>(r["album_genre"]);
                                        track[ATTRIBUTE_GENRE] = genre;
                                }

                                if(r.count("album_playscore"))
                                {
                                        playscore = get<double>(r["album_playscore"]);
                                }

                                if(r.count("album_insert_date"))
                                {
                                        (*iter)[Columns.InsertDate] = get<gint64>(r["album_insert_date"]);
                                }

                                if(r.count("amazon_asin"))
                                {
                                        asin = get<std::string>(r["amazon_asin"]);
                                        track[ATTRIBUTE_ASIN] = asin;
                                }

                                if(r.count("mb_album_id"))
                                {
                                        mbid = get<std::string>(r["mb_album_id"]);

                                        IterSet & s = m_Album_MBID_Iter_Map[mbid];
                                        s.insert(iter);

                                        track[ATTRIBUTE_MB_ALBUM_ID] = mbid;
                                }

                                if(r.count("mb_album_artist_id"))
                                {
                                        mbid_artist = get<std::string>(r["mb_album_artist_id"]);
                                        track[ATTRIBUTE_MB_ALBUM_ARTIST_ID] = mbid_artist;
                                }

                                if(r.count("mb_release_date"))
                                {
                                        year = get<std::string>(r["mb_release_date"]);
                                        if(year.size())
                                        {
                                                year = year.substr(0,4);
                                                try{
                                                        (*iter)[Columns.Date] = boost::lexical_cast<int>(year);
                                                        track[ATTRIBUTE_DATE] = gint64(boost::lexical_cast<int>(year));
                                                } catch( boost::bad_lexical_cast ) {
                                                } 
                                        }
                                }
                                else
                                {
                                        (*iter)[Columns.Date] = 0; 
                                }

                                if(r.count("mb_release_country"))
                                {
                                        country = get<std::string>(r["mb_release_country"]); 
                                        track[ATTRIBUTE_MB_RELEASE_COUNTRY] = country; 
                                }

                                if(r.count("mb_release_type"))
                                {
                                        type = get<std::string>(r["mb_release_type"]); 
                                        track[ATTRIBUTE_MB_RELEASE_TYPE] = type; 
                                }

                                if(r.find("album_artist") != r.end())
                                {
                                        artist = get<std::string>(r["album_artist"]);
                                        track[ATTRIBUTE_ALBUM_ARTIST] = artist; 
                                }

                                if(r.find("album_artist_sortname") != r.end())
                                {
                                        artist_sort = get<std::string>(r["album_artist_sortname"]); 
                                        track[ATTRIBUTE_ALBUM_ARTIST_SORTNAME] = artist_sort; 
                                }
                                else
                                {
                                        artist_sort = artist;
                                }

                                trim(country);
                                trim(year);
                                trim(type);
                                trim(genre);

                                rt = determine_release_type(type);
                                rt_string = _(get_release_string(rt).c_str());

                                (*iter)[Columns.Album] = album;
                                (*iter)[Columns.AlbumArtist] = artist;

                                (*iter)[Columns.AlbumId] = get<gint64>(r["id"]);
                                (*iter)[Columns.AlbumMBID] = mbid; 

                                (*iter)[Columns.AlbumArtistId] = get<gint64>(r["album_artist_j"]);
                                (*iter)[Columns.AlbumArtistMBID] = mbid_artist; 

                                (*iter)[Columns.AlbumSort] = ustring(album).collate_key();
                                (*iter)[Columns.AlbumArtistSort] = ustring(artist_sort).collate_key();

                                (*iter)[Columns.RT] = rt; 
                                (*iter)[Columns.AlbumTrack] = p;
                                (*iter)[Columns.Rating] = rating;
                                (*iter)[Columns.Genre] = genre;
                                (*iter)[Columns.PlayScore] = playscore; 
                                (*iter)[Columns.Text] = ustring_sp( new ustring( ustring((boost::format("%s %s") % album % artist).str()).lowercase() ));

                                AlbumInfo_pt renderdata (new AlbumInfo);
                                renderdata->Name = album; 
                                renderdata->Artist = artist_sort;
                                renderdata->Release = country.empty() ? (year.empty() ? "" : year) : (year.empty() ? country : (boost::format("%s %s") % country % year).str());
                                renderdata->Type = rt_string;
                                renderdata->Genre = genre;
                                renderdata->Qual = quality; 
                                renderdata->Rating = rating;
                                (*iter)[Columns.RenderData] = renderdata;
                        } 

                void
                        AlbumTreeView::update_album (SQL::Row & r, gint64 id)
                        {
                                IdIterMap::iterator i = m_Album_Iter_Map.find(id);
                                if (i == m_Album_Iter_Map.end()) return;

                                TreeIter iter = (*i).second; 

                                (*iter)[Columns.NewAlbum] = get<gint64>(r["album_new"]);

                                place_album_iter_real(iter, r, id);
                        } 

                Gtk::TreeIter
                        AlbumTreeView::place_album(
                            SQL::Row&   r,
                            gint64      id
                        )
                        {
                                TreeIter iter = AlbumsTreeStore->append();
                                m_Album_Iter_Map.insert(std::make_pair(id, iter));
                                AlbumsTreeStore->append(iter->children()); //create dummy/placeholder row for tracks

                                (*iter)[Columns.RowType] = ROW_ALBUM; 
                                (*iter)[Columns.HasTracks] = false; 
                                (*iter)[Columns.NewAlbum] = get<gint64>(r["album_new"]);
                                (*iter)[Columns.AlbumId] = id; 

                                place_album_iter_real(iter, r, id);

                                switch( (*iter)[Columns.RT] ) 
                                {
                                    case RT_COMPILATION:
                                        (*iter)[Columns.Image] = Util::cairo_image_surface_overlay( m_DiscDefault, Util::cairo_image_surface_from_pixbuf(m_Emblem[EM_COMPILATION]), 0., 0., 1.); 
                                        break;

                                    case RT_SOUNDTRACK:
                                        (*iter)[Columns.Image] = Util::cairo_image_surface_overlay( m_DiscDefault, Util::cairo_image_surface_from_pixbuf(m_Emblem[EM_SOUNDTRACK]), 0., 0., 1.); 
                                        break;

                                    default:
                                        (*iter)[Columns.Image] = m_DiscDefault; 
                                        break;
                                }

                                on_got_cover( get<std::string>(r["mb_album_id"]));

                                return iter;
                        }

                void
                        AlbumTreeView::album_list_load ()
                        {
                                AlbumsTreeStore->clear ();
                                m_Album_MBID_Iter_Map.clear();
                                m_Album_Iter_Map.clear();
                                m_Track_Iter_Map.clear();

                                SQL::RowV v;
                                services->get<Library>("mpx-service-library")->getSQL(v, "SELECT * FROM album JOIN album_artist ON album.album_artist_j = album_artist.id;");

                                for(SQL::RowV::iterator i = v.begin(); i != v.end(); ++i)
                                {
                                        SQL::Row & r = *i; 
                                        place_album(r, get<gint64>(r["id"]));

                                        RequestQualifier rq;
                                        rq.mbid = get<std::string>(r["mb_album_id"]);

                                        services->get<Covers>("mpx-service-covers")->cache(
                                              rq
                                            , false 
                                        );
                                }
                        }

                void
                        AlbumTreeView::on_album_updated(gint64 id)
                        {
                                SQL::RowV v;
                                services->get<Library>("mpx-service-library")->getSQL(v, (boost::format("SELECT * FROM album JOIN album_artist ON album.album_artist_j = album_artist.id WHERE album.id = %lld;") % id).str());
                                g_return_if_fail(!v.empty());
                                SQL::Row & r = v[0];
                                update_album (r, id); 
                        }

                void
                        AlbumTreeView::on_new_album(gint64 id)
                        {
                                if(!m_Album_Iter_Map.count(id))
                                {
                                        SQL::RowV v;
                                        services->get<Library>("mpx-service-library")->getSQL(v, (boost::format("SELECT * FROM album JOIN album_artist ON album.album_artist_j = album_artist.id WHERE album.id = %lld;") % id).str());
                                        g_return_if_fail(!v.empty());
                                        SQL::Row & r = v[0];
                                        place_album (r, id); 
                                }
                        }

                void
                        AlbumTreeView::on_new_track(Track & track, gint64 album_id, gint64 artist_id)
                        {
                                if(!m_Album_Iter_Map.count(album_id))
                                {
                                    on_new_album( album_id );
                                }

                                TreeIter iter = m_Album_Iter_Map[album_id];

                                if (((*iter)[Columns.HasTracks]))
                                {
                                        TreeIter child = AlbumsTreeStore->append(iter->children());
                                        if(track[ATTRIBUTE_TITLE])
                                                (*child)[Columns.TrackTitle] = get<std::string>(track[ATTRIBUTE_TITLE].get());
                                        if(track[ATTRIBUTE_ARTIST])
                                                (*child)[Columns.TrackArtist] = get<std::string>(track[ATTRIBUTE_ARTIST].get());
                                        if(track[ATTRIBUTE_MB_ARTIST_ID])
                                                (*child)[Columns.TrackArtistMBID] = get<std::string>(track[ATTRIBUTE_MB_ARTIST_ID].get());
                                        if(track[ATTRIBUTE_TRACK])
                                                (*child)[Columns.TrackNumber] = get<gint64>(track[ATTRIBUTE_TRACK].get());
                                        if(track[ATTRIBUTE_TIME])
                                                (*child)[Columns.TrackLength] = get<gint64>(track[ATTRIBUTE_TIME].get());

                                        (*child)[Columns.TrackId] = get<gint64>(track[ATTRIBUTE_MPX_TRACK_ID].get());
                                        (*child)[Columns.RowType] = ROW_TRACK; 
                                }
                        }

                void
                        AlbumTreeView::on_album_deleted(gint64 id)
                        {
                            IdIterMap::iterator i = m_Album_Iter_Map.find(id);
                            if( i != m_Album_Iter_Map.end() )
                            {
                                std::string mbid = (*i->second)[Columns.AlbumMBID];
                                AlbumsTreeStore->erase( i->second );
                                IterSet & set = m_Album_MBID_Iter_Map[mbid];
                                set.erase( i->second );
                                m_Album_Iter_Map.erase( i );
                            }
                        }

                void
                        AlbumTreeView::on_track_deleted(gint64 id)
                        {
                            IdIterMap::iterator i = m_Track_Iter_Map.find(id);
                            if( i != m_Track_Iter_Map.end() )
                            {
                                TreeIter parent = i->second->parent();

                                AlbumsTreeStore->erase( i->second );
                                m_Track_Iter_Map.erase( i );

                                if( !parent->children().size() )
                                {
                                    on_album_deleted( gint64((*parent)[Columns.AlbumId]) );
                                }
                            }
                        }

                void
                        AlbumTreeView::on_track_updated(
                              Track&      t
                            , gint64      id_album
                            , gint64      id_artst
                        )
                        {
                            IdIterMap::iterator i = m_Track_Iter_Map.find( get<gint64>(t[ATTRIBUTE_MPX_TRACK_ID].get()) );
                            if( i != m_Track_Iter_Map.end() )
                            {
                                TreeIter parent = i->second->parent();

                                AlbumsTreeStore->erase( i->second );
                                m_Track_Iter_Map.erase( i );

                                on_new_track( t, id_album, id_artst );
                            }
                        }

                int
                        
                        AlbumTreeView::sortTracks(const TreeIter& iter_a, const TreeIter& iter_b)
                        {
                                gint64 trk_a = (*iter_a)[Columns.TrackNumber];
                                gint64 trk_b = (*iter_b)[Columns.TrackNumber];

                                return trk_a - trk_b;
                        }

                int
                        AlbumTreeView::slotSortRating(const TreeIter& iter_a, const TreeIter& iter_b)
                        {
                                AlbumRowType rt_a = (*iter_a)[Columns.RowType];
                                AlbumRowType rt_b = (*iter_b)[Columns.RowType];

                                if((rt_a == ROW_ALBUM) && (rt_b == ROW_ALBUM))
                                {
                                        gint64 alb_a = (*iter_a)[Columns.Rating];
                                        gint64 alb_b = (*iter_b)[Columns.Rating];

                                        if( !(alb_b - alb_a) )
                                            return slotSortDate( iter_a, iter_b );
                                        else
                                            return alb_b - alb_a;
                                }
                                else if((rt_a == ROW_TRACK) && (rt_b == ROW_TRACK))
                                {
                                        return sortTracks( iter_a, iter_b );
                                }

                                return 0;
                        }

                int
                        AlbumTreeView::slotSortAlpha(const TreeIter& iter_a, const TreeIter& iter_b)
                        {
                                AlbumRowType rt_a = (*iter_a)[Columns.RowType];
                                AlbumRowType rt_b = (*iter_b)[Columns.RowType];

                                if((rt_a == ROW_ALBUM) && (rt_b == ROW_ALBUM))
                                {
                                        gint64 alb_a = (*iter_a)[Columns.Date];
                                        gint64 alb_b = (*iter_b)[Columns.Date];
                                        std::string art_a = (*iter_a)[Columns.AlbumArtistSort];
                                        std::string art_b = (*iter_b)[Columns.AlbumArtistSort];

                                        if(art_a != art_b)
                                                return art_a.compare(art_b);

                                        return alb_a - alb_b;
                                }
                                else if((rt_a == ROW_TRACK) && (rt_b == ROW_TRACK))
                                {
                                        return sortTracks( iter_a, iter_b );
                                }

                                return 0;
                        }

                int
                        AlbumTreeView::slotSortDate(const TreeIter& iter_a, const TreeIter& iter_b)
                        {
                                AlbumRowType rt_a = (*iter_a)[Columns.RowType];
                                AlbumRowType rt_b = (*iter_b)[Columns.RowType];

                                if((rt_a == ROW_ALBUM) && (rt_b == ROW_ALBUM))
                                {
                                        gint64 alb_a = (*iter_a)[Columns.InsertDate];
                                        gint64 alb_b = (*iter_b)[Columns.InsertDate];

                                        return alb_b - alb_a;
                                }
                                else if((rt_a == ROW_TRACK) && (rt_b == ROW_TRACK))
                                {
                                        return sortTracks( iter_a, iter_b );
                                }

                                return 0;
                        }


                int
                        AlbumTreeView::slotSortStrictAlpha(const TreeIter& iter_a, const TreeIter& iter_b)
                        {
                                AlbumRowType rt_a = (*iter_a)[Columns.RowType];
                                AlbumRowType rt_b = (*iter_b)[Columns.RowType];

                                if((rt_a == ROW_ALBUM) && (rt_b == ROW_ALBUM))
                                {
                                        std::string alb_a = (*iter_a)[Columns.AlbumSort];
                                        std::string alb_b = (*iter_b)[Columns.AlbumSort];
                                        std::string art_a = (*iter_a)[Columns.AlbumArtistSort];
                                        std::string art_b = (*iter_b)[Columns.AlbumArtistSort];

                                        if(art_a != art_b)
                                                return art_a.compare(art_b);

                                        return alb_a.compare(alb_b);
                                }
                                else if((rt_a == ROW_TRACK) && (rt_b == ROW_TRACK))
                                {
                                        return sortTracks( iter_a, iter_b );
                                }

                                return 0;
                        }

                int
                        AlbumTreeView::slotSortPlayScore(const TreeIter& iter_a, const TreeIter& iter_b)
                        {
                                AlbumRowType rt_a = (*iter_a)[Columns.RowType];
                                AlbumRowType rt_b = (*iter_b)[Columns.RowType];

                                if((rt_a == ROW_ALBUM) && (rt_b == ROW_ALBUM))
                                {
                                        double score_a = (*iter_a)[Columns.PlayScore];
                                        double score_b = (*iter_b)[Columns.PlayScore];

                                        if( score_a < score_b )
                                            return  1;
    
                                        if( score_a > score_b )
                                            return -1;

                                        return 0;
                                }
                                else if((rt_a == ROW_TRACK) && (rt_b == ROW_TRACK))
                                {
                                        return sortTracks( iter_a, iter_b );
                                }

                                return 0;
                        }

                void
                        AlbumTreeView::cellDataFuncCover (CellRenderer * basecell, const TreeIter& iter)
                        {
                                TreePath path (iter);
                                CellRendererCairoSurface *cell = dynamic_cast<CellRendererCairoSurface*>(basecell);
                                if(path.get_depth() == ROW_ALBUM)
                                {
                                        cell->property_visible() = true;
                                        cell->property_surface() = (*iter)[Columns.Image]; 

                                        double score = (*iter)[Columns.PlayScore]; 

                                        if( Options.HighlightMode == HIGHLIGHT_EQUAL )
                                        {
                                            cell->property_alpha() = 1.0;
                                        }
                                        else if( Options.HighlightMode == HIGHLIGHT_UNPLAYED )
                                        {
                                            if( score < 1 )
                                            {
                                                cell->property_alpha() = 1.;
                                                return;
                                            }

                                            cell->property_alpha() = 0.1; 
                                        }
                                        else if( Options.HighlightMode == HIGHLIGHT_PLAYED )
                                        {
                                            if( score >= 1 )
                                            {
                                                cell->property_alpha() = 1.;
                                                return;
                                            }

                                            cell->property_alpha() = 0.1; 
                                        }
                                }
                                else
                                {
                                        cell->property_visible() = false;
                                }
                        }

                void
                        AlbumTreeView::cellDataFuncText1 (CellRenderer * basecell, const TreeIter& iter)
                        {
                                TreePath path (iter);

                                CellRendererAlbumData *cell = dynamic_cast<CellRendererAlbumData*>(basecell);

                                g_return_if_fail( cell );

                                if( path.get_depth() == ROW_ALBUM )
                                {
                                        cell->property_visible() = true; 
                                        cell->property_info() = (*iter)[Columns.RenderData]; 
                                }
                                else
                                {
                                        cell->property_visible() = false; 
                                }
                        }

                void
                        AlbumTreeView::cellDataFuncAdd (CellRenderer * basecell, const TreeIter& iter)
                        {
                                TreePath path (iter);

                                CellRendererPixbuf *cell = dynamic_cast<CellRendererPixbuf*>(basecell);

                                g_return_if_fail( cell );

                                if( path.get_depth() == ROW_TRACK )
                                {
                                         if( m_MouseOverIter && iter == m_MouseOverIter.get() )
                                          {
                                                    cell->property_stock_id() = "mpx-stock-add";
                                                    cell->property_stock_size() = ICON_SIZE_MENU; // 16
                                          }
                                          else
                                          {
                                                    cell->property_pixbuf() = Glib::RefPtr<Gdk::Pixbuf>(0); 
                                          }

                                          cell->property_visible() = true;
                                }
                                else
                                {
                                          cell->property_visible() = false;
                                }
                        }

                void
                        AlbumTreeView::cellDataFuncText2 (CellRenderer * basecell, const TreeIter& iter)
                        {
                                TreePath path (iter);

                                CellRendererCount *cell = dynamic_cast<CellRendererCount*>(basecell);

                                if(path.get_depth() == ROW_TRACK)
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
                        AlbumTreeView::cellDataFuncText3 (CellRenderer * basecell, const TreeIter& iter)
                        {
                                TreePath path (iter);
                                CellRendererText *cell = dynamic_cast<CellRendererText*>(basecell);
                                if(path.get_depth() == ROW_TRACK)
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
                        AlbumTreeView::cellDataFuncText4 (CellRenderer * basecell, const TreeIter& iter)
                        {
                                TreePath path (iter);
                                CellRendererText *cell = dynamic_cast<CellRendererText*>(basecell);
                                if(path.get_depth() == ROW_TRACK)
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
                        AlbumTreeView::cellDataFuncText5 (CellRenderer * basecell, const TreeIter& iter)
                        {
                                TreePath path (iter);
                                CellRendererText *cell = dynamic_cast<CellRendererText*>(basecell);
                                if(path.get_depth() == ROW_TRACK)
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
                        AlbumTreeView::rb_sourcelist_expander_cell_data_func (GtkTreeViewColumn *column,
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

                void
                        AlbumTreeView::update_album_count_display ()
                        {
                                TreeNodeChildren::size_type n1 = AlbumsTreeStoreFilter->children().size();
                                TreeNodeChildren::size_type n2 = AlbumsTreeStore->children().size();
                                if( m_LabelShowing )
                                {
                                    static boost::format showing_f ("Out of %lld, showing %lld %s%s");

                                    std::string extra = "";
                                    if (Options.HighlightMode != HIGHLIGHT_EQUAL)
                                    {
                                        if( Options.HighlightMode == HIGHLIGHT_PLAYED )
                                            extra = _(", highlighting played");
                                        else
                                            extra = _(", highlighting unplayed");
                                    }

                                    m_LabelShowing->set_text( (showing_f
                                        % n2
                                        % n1
                                        % ((Options.Type == RT_ALL)
                                                ? ((n1 == 1) ? _("release") : _("releases of all kind"))
                                                : get_release_string(ReleaseType(Options.Type), n1>1))
                                        % extra
                                    ).str());
                                }
                        }

                void
                        AlbumTreeView::on_row_added_or_deleted ()
                        {
                                update_album_count_display ();
                        }

                void
                        AlbumTreeView::go_to_album(gint64 id)
                        {
                                if(m_Album_Iter_Map.count(id))
                                {
                                        TreeIter iter = m_Album_Iter_Map.find(id)->second;
                                        scroll_to_row (AlbumsTreeStore->get_path(iter), 0.);
                                }
                        }

                void
                        AlbumTreeView::set_new_albums_state (bool state)
                        {
                                if(state)
                                        Options.Flags |= ALBUMS_STATE_SHOW_NEW;
                                else
                                        Options.Flags &= ~ALBUMS_STATE_SHOW_NEW;

                                AlbumsTreeStoreFilter->refilter();
                                update_album_count_display();
                        }

                void
                        AlbumTreeView::set_release_type_filter (int state)
                        {
                                Options.Type = state; 

                                AlbumsTreeStoreFilter->refilter();
                                update_album_count_display();
                        }

                void
                        AlbumTreeView::set_highlight_mode (AlbumHighlightMode mode)
                        {
                                Options.HighlightMode = mode;
                                update_album_count_display();
                                queue_draw ();
                        }
} // end namespace MPX 
