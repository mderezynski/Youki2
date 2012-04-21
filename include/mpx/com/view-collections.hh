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
#ifndef MPX_MUSICLIB_VIEW_COLLECTIONS_HH
#define MPX_MUSICLIB_VIEW_COLLECTIONS_HH
#include "config.h"
#include <glibmm/i18n.h>
#include <gtkmm.h>
#include <glib.h>
#include <giomm.h>
#include <libglademm.h>
#include <Python.h>
#define NO_IMPORT
#include <pygobject.h>

#ifdef HAVE_TR1
#include <tr1/unordered_map>
#else
#include <map>
#endif

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "mpx/mpx-library.hh"
#include "mpx/mpx-sql.hh"
#include "mpx/mpx-stock.hh"
#include "mpx/mpx-types.hh"
#include "mpx/util-graphics.hh"
#include "mpx/util-ui.hh"
#include "mpx/util-string.hh"
#include "mpx/widgets/widgetloader.hh"
#include "mpx/mpx-protected-access.hh"

#include "mpx/widgets/cell-renderer-cairo-surface.hh"
#include "mpx/widgets/cell-renderer-count.hh"
#include "mpx/widgets/cell-renderer-vbox.hh"
#include "mpx/widgets/cell-renderer-expander.h"
#include "mpx/widgets/rounded-layout.hh"
#include "mpx/widgets/timed-confirmation.hh"

namespace MPX
{
                typedef sigc::signal<void, gint64>          SignalPlayCollection;
                typedef sigc::signal<void, IdV, bool>       SignalPlayTracks;

                enum CollectionRowType
                {
                        CRT_ROW_COLLECTION  =   1,
                        CRT_ROW_TRACK       =   2,
                };

               class CollectionTreeView
                        :   public Gnome::Glade::WidgetLoader<Gtk::TreeView>
                {
                    public:

                        typedef std::set<Gtk::TreeIter>                         IterSet;
#ifdef HAVE_TR1
                        typedef std::tr1::unordered_map<gint64, Gtk::TreeIter>  IdIterMap;
#else
                        typedef std::map<gint64, Gtk::TreeIter>                 IdIterMap;
#endif

                        struct ColumnsT : public Gtk::TreeModel::ColumnRecord 
                        {
                                Gtk::TreeModelColumn<CollectionRowType>                     RowType;
                                Gtk::TreeModelColumn<Cairo::RefPtr<Cairo::ImageSurface> >   Image;
                                Gtk::TreeModelColumn<Glib::ustring>                         Text;
                                Gtk::TreeModelColumn<bool>                                  HasTracks;

                                Gtk::TreeModelColumn<std::string>                           Name;
                                Gtk::TreeModelColumn<std::string>                           NameSortKey;
                                Gtk::TreeModelColumn<std::string>                           Blurb;
                                Gtk::TreeModelColumn<gint64>                                Id;

                                Gtk::TreeModelColumn<Glib::ustring>                         TrackTitle;
                                Gtk::TreeModelColumn<Glib::ustring>                         TrackArtist;
                                Gtk::TreeModelColumn<std::string>                           TrackArtistMBID;
                                Gtk::TreeModelColumn<gint64>                                TrackNumber;
                                Gtk::TreeModelColumn<gint64>                                TrackLength;
                                Gtk::TreeModelColumn<gint64>                                TrackId;

                                ColumnsT ()
                                {
                                        add (RowType);
                                        add (Image);
                                        add (Text);
                                        add (HasTracks);

                                        add (Name);
                                        add (NameSortKey);
                                        add (Blurb);
                                        add (Id);

                                        add (TrackTitle);
                                        add (TrackArtist);
                                        add (TrackArtistMBID);
                                        add (TrackNumber);
                                        add (TrackLength);
                                        add (TrackId);
                                }
                        };

                        public:

                        // treemodel stuff

                          Glib::RefPtr<Gtk::TreeStore>          CollectionsTreeStore;
                          Glib::RefPtr<Gtk::TreeModelFilter>    CollectionsTreeStoreFilter;
                          ColumnsT                              Columns;

                        protected:

                          std::string                           m_Name;

                        // ui

                          Glib::RefPtr<Gtk::UIManager>          m_UIManager;
                          Glib::RefPtr<Gtk::ActionGroup>        m_ActionGroup;      

                        // objects

                          PAccess<MPX::Library>                 m_Lib;

                        // view mappings

                          IdIterMap                             m_CollectionIterMap;
                          IdIterMap                             m_TrackIterMap;

                        // disc+rating pixbufs

                          Cairo::RefPtr<Cairo::ImageSurface>    m_DiscDefault;

                        // DND state variables

                          boost::optional<gint64>               m_DragCollectionId;
                          boost::optional<gint64>               m_DragTrackId;
                          Gtk::TreePath                         m_PathButtonPress;

                        // state variables

                          bool                                  m_ButtonPressed;
                          Glib::ustring                         m_FilterText;

                        // widgets

                          Gtk::Entry*                           m_FilterEntry;
                          RoundedLayout*                        m_LabelShowing;

                        // signals

                          struct Signals_t
                          {
                              SignalPlayCollection              PlayCollection;
                              SignalPlayTracks                  PlayTracks; 
                          };

                          Signals_t Signals;

                        public:

                        SignalPlayTracks&
                        signal_play_tracks()
                        {
                            return Signals.PlayTracks;
                        }

                        SignalPlayCollection&
                        signal_play_collection()
                        {
                            return Signals.PlayCollection;
                        }

                        CollectionTreeView(
                              const Glib::RefPtr<Gnome::Glade::Xml>&
                            , const std::string&
                            , const std::string&
                            , const std::string&
                            , Glib::RefPtr<Gtk::UIManager>
                        );

                        protected:

                        virtual void
                                on_row_activated (const Gtk::TreeModel::Path&, Gtk::TreeViewColumn*);

                        virtual void
                                on_row_expanded (const Gtk::TreeIter &iter_filter,
                                                 const Gtk::TreePath &path);

                        virtual void
                                on_drag_data_get (const Glib::RefPtr<Gdk::DragContext>&, Gtk::SelectionData&, guint, guint);

                        virtual void
                                on_drag_begin (const Glib::RefPtr<Gdk::DragContext>&);

                        virtual bool
                                on_button_press_event (GdkEventButton*);

                        virtual bool
                                on_button_release_event (GdkEventButton*);

                        virtual bool
                                on_event (GdkEvent * ev);

                        virtual Gtk::TreeIter 
                                place_collection (SQL::Row&, gint64);

                        virtual void
                                update_collection (SQL::Row&, gint64);

                        virtual void
                                place_collection_iter_real(
                                    Gtk::TreeIter&  iter,
                                    SQL::Row&       r,
                                    gint64          id
                                );

                        virtual void
                                collection_list_load ();

                        virtual void
                                on_collection_updated(gint64);

                        virtual void
                                on_new_collection(gint64);

                        virtual void
                                on_new_track(gint64, gint64);

                        virtual void
                                on_collection_deleted(gint64);

                        virtual void
                                on_track_deleted(gint64);

                        virtual int
                                slotSortStrictAlpha(const Gtk::TreeIter&, const Gtk::TreeIter&);

                        virtual int
                                sortTracks(const Gtk::TreeIter&, const Gtk::TreeIter&);

                        virtual void
                                cellDataFuncCover (Gtk::CellRenderer *, Gtk::TreeModel::iterator const&);

                        virtual void
                                cellDataFuncText1 (Gtk::CellRenderer *, Gtk::TreeModel::iterator const&);

                        virtual void
                                cellDataFuncText2 (Gtk::CellRenderer *, Gtk::TreeModel::iterator const&);

                        virtual void
                                cellDataFuncText3 (Gtk::CellRenderer *, Gtk::TreeModel::iterator const&);

                        virtual void
                                cellDataFuncText4 (Gtk::CellRenderer *, Gtk::TreeModel::iterator const&);

                        virtual void
                                cellDataFuncText5 (Gtk::CellRenderer *, Gtk::TreeModel::iterator const&);

                        static void
                                rb_sourcelist_expander_cell_data_func(
                                    GtkTreeViewColumn*,
                                    GtkCellRenderer*,
                                    GtkTreeModel*,
                                    GtkTreeIter*,
                                    gpointer 
                                ); 

                        virtual bool
                                collection_visible_func (Gtk::TreeIter const&);

                        virtual void
                                update_collection_count_display ();

                        virtual void
                                on_row_added_or_deleted ();

                        virtual void
                                on_filter_entry_changed ();

                };

} // end namespace MPX 

#endif
