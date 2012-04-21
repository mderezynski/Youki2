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
#ifndef MPX_MB_IMPORT_ALBUM_HH
#define MPX_MB_IMPORT_ALBUM_HH
#include "config.h"
#include <glibmm/i18n.h>
#include <gtkmm.h>
#include <glib.h>
#include <libglademm.h>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "mpx/mpx-library.hh"
#include "mpx/mpx-sql.hh"
#include "mpx/mpx-covers.hh"
#include "mpx/mpx-services.hh"
#include "mpx/mpx-types.hh"
#include "mpx/util-graphics.hh"
#include "mpx/util-ui.hh"
#include "mpx/util-string.hh"
#include "mpx/widgets/widgetloader.hh"

#include "musicbrainz/mbxml-v2.hh"

using namespace Gtk;
using namespace Glib;
using namespace Gnome::Glade;
using namespace MPX;
using boost::get;
using boost::algorithm::trim;

namespace MPX
{
        struct MB_ImportAlbum_Model : public Gtk::TreeModelColumnRecord
        {
            Gtk::TreeModelColumn<MPX::Track>    Track;

            MB_ImportAlbum_Model ()
            {
                add(Track);
            }
        };

        struct CBE_Model_Text_ID : public Gtk::TreeModelColumnRecord
        {
            Gtk::TreeModelColumn<std::string>    Text;
            Gtk::TreeModelColumn<std::string>    ID;

            CBE_Model_Text_ID ()
            {
                add(Text);
                add(ID);
            }
        };

        typedef std::vector<std::string>                                  Uris_t;
        typedef std::map<std::string, MusicBrainzXml::MusicBrainzRelease> ReleaseIdToReleaseMap;
        typedef std::map<std::string, Uris_t>                             ArtistIdToReleasesMap;

        class MB_ImportAlbum : public Gnome::Glade::WidgetLoader<Gtk::Window>, public Service::Base
        {
                enum Columns
                {
                    COL_TRACK,
                    COL_TITLE,
                    COL_ALBUM,
                    COL_ARTIST,
                    COL_TIME
                };

                boost::shared_ptr<MPX::Library>   m_Lib;
                boost::shared_ptr<MPX::Covers>    m_Covers;

                Gtk::Image                    * m_iCover;

                Gtk::Button                   * m_bImport,
                                              * m_bCancel,
                                              * m_bAddFiles,
                                              * m_bSearch;

                Gtk::ComboBox                 * m_CB_Events;
                Gtk::ComboBoxEntry            * m_CBE_Artist,
                                              * m_CBE_Album,
                                              * m_CBE_Genre;

                Gtk::Entry                    * m_eMBID;
                Gtk::TreeView                 * m_tTracks,
                                              * m_tRelease;

                Glib::RefPtr<Gtk::ListStore>    m_Model_Release;
                Glib::RefPtr<Gtk::ListStore>    m_Model_Tracks;
                Glib::RefPtr<Gtk::ListStore>    m_Model_CBE_Artist;
                Glib::RefPtr<Gtk::ListStore>    m_Model_CBE_Album;
                    
                MB_ImportAlbum_Model            m_Columns_Release;
                CBE_Model_Text_ID               m_Columns_CBE;

                Uris_t                              m_Uris;
                MusicBrainzXml::MusicBrainzRelease  m_Release;
                ReleaseIdToReleaseMap               m_Releases;
                ArtistIdToReleasesMap               m_Artist_To_Releases_Map;

                void
                setup_view(Gtk::TreeView&);

                void
                cell_data_func_track (Gtk::CellRenderer*, const Gtk::TreeIter&, Columns);

                void
                reload_tracks ();

                void
                populate_matches (const MusicBrainzXml::MusicBrainzReleaseV&);

                void
                on_artist_changed ();
        
                void
                on_album_changed ();


                void
                on_button_search ();

                void
                on_button_import ();
        
                void
                on_button_cancel ();

                void
                on_button_add_files ();


            public:

                static MB_ImportAlbum*
                create(
                );

                MB_ImportAlbum(
                    const Glib::RefPtr<Gnome::Glade::Xml>& xml
                );

                void
                run ();
        };
}

#endif
