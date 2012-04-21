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
#ifndef MPX_ALBUM_INFO_WINDOW_HH
#define MPX_ALBUM_INFO_WINDOW_HH
#include "config.h"
#ifdef HAVE_TR1
#include <tr1/unordered_map>
#endif //HAVE_TR1
#include <glibmm/i18n.h>
#include <gtkmm.h>
#include <glib.h>
#include <libglademm.h>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "mpx/mpx-library.hh"
#include "mpx/mpx-sql.hh"
#include "mpx/mpx-covers.hh"
#include "mpx/mpx-types.hh"
#include "mpx/util-graphics.hh"
#include "mpx/util-ui.hh"
#include "mpx/util-string.hh"
#include "mpx/widgets/widgetloader.hh"

using namespace Gtk;
using namespace Glib;
using namespace Gnome::Glade;
using namespace MPX;
using boost::get;
using boost::algorithm::trim;

namespace MPX
{
        // Album Details Window

        typedef boost::optional<gint64> OptionalId_t;
        class AlbumRatingsList : public Gnome::Glade::WidgetLoader<Gtk::TreeView>
        {
                public:

                class Columns_t : public Gtk::TreeModelColumnRecord
                {
                        public: 

                                Gtk::TreeModelColumn<Glib::ustring> DateString;
                                Gtk::TreeModelColumn<int>           Rating;
                                Gtk::TreeModelColumn<Glib::ustring> ShortText;
                                Gtk::TreeModelColumn<Glib::ustring> Text;
                                Gtk::TreeModelColumn<gint64>        Id;

                                Columns_t ()
                                {
                                        add(DateString);
                                        add(Rating);
                                        add(ShortText);
                                        add(Text);
                                        add(Id);
                                };
                };

                Columns_t                       Columns;
                Glib::RefPtr<Gtk::ListStore>    Store;
                Glib::RefPtr<Gdk::Pixbuf>       Stars[6];
                Glib::RefPtr<Gtk::TextBuffer>   Buffer;

                AlbumRatingsList(
                    const Glib::RefPtr<Gnome::Glade::Xml>&
                );

                void
                        on_selection_changed ();

                void
                        load_ratings(gint64, MPX::Library&);

                void
                        cellDataFuncRating (CellRenderer *, TreeModel::iterator const&);

        };

        class AlbumInfoWindow : public Gnome::Glade::WidgetLoader<Gtk::Window>
        {
                gint64                    m_Id;
                MPX::Library            & m_Lib;
                MPX::Covers             & m_Covers;

                AlbumRatingsList        * m_AlbumRatingsList;
                Gtk::Image              * m_ImageCover;
                Gtk::Label              * m_l1, *m_l2, *m_l3, *m_l4, *m_l5;
                Gtk::Button             * m_delete_rating;

                public:

                static AlbumInfoWindow*
                create(
                    gint64                      id,
                    MPX::Library              & obj_library,
                    MPX::Covers               & obj_covers
                );


                AlbumInfoWindow(
                    const Glib::RefPtr<Gnome::Glade::Xml>&  xml,
                    gint64                                  id,
                    MPX::Library                          & obj_library,    
                    MPX::Covers                           & obj_covers
                );

                void
                        on_rating_list_selection_changed ();

                void
                        on_album_updated (gint64);

                void
                        delete_rating ();

                void
                        display_album ();
        };
}

#endif
