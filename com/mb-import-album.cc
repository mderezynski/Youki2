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
#include "mpx/com/mb-import-album.hh"
#include "config.h"
#include <glibmm/i18n.h>
#include <gtkmm.h>
#include <glib.h>
#include <libglademm.h>

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#include "mpx/mpx-library.hh"
#include "mpx/mpx-main.hh"
#include "mpx/mpx-sql.hh"
#include "mpx/mpx-stock.hh"
#include "mpx/mpx-types.hh"
#include "mpx/util-graphics.hh"
#include "mpx/util-ui.hh"
#include "mpx/util-string.hh"
#include "mpx/widgets/widgetloader.hh"
#include "mpx/metadatareader-taglib.hh"
#include "musicbrainz/mbxml-v2.hh"

using namespace Gtk;
using namespace Glib;
using namespace Gnome::Glade;
using namespace MPX;
using namespace MusicBrainzXml;
using boost::get;
using boost::algorithm::trim;

namespace
{
    const int N_STARS = 6;
}

namespace MPX
{
        MB_ImportAlbum*
                MB_ImportAlbum::create(
                )
                {
                        const std::string path = DATA_DIR G_DIR_SEPARATOR_S "glade" G_DIR_SEPARATOR_S "mb-import-album.glade";
                        Glib::RefPtr<Gnome::Glade::Xml> glade_xml = Gnome::Glade::Xml::create (path);
                        MB_ImportAlbum * p = new MB_ImportAlbum(glade_xml); 
                        return p;
                }


                MB_ImportAlbum::MB_ImportAlbum(
                    const Glib::RefPtr<Gnome::Glade::Xml>& xml
                )
                : WidgetLoader<Gtk::Window>(xml, "mb-import-album")
                , Service::Base("mpx-service-mbimport")
                {
                        m_Lib = services->get<Library>("mpx-service-library");
                        m_Covers = services->get<Covers>("mpx-service-covers");
    
                        glade_xml_signal_autoconnect(xml->gobj());

                        m_Xml->get_widget("mbrel-cover", m_iCover); 
                        m_Xml->get_widget("button-search", m_bSearch);
                        m_Xml->get_widget("button-import", m_bImport);
                        m_Xml->get_widget("button-cancel", m_bCancel);
                        m_Xml->get_widget("button-add-files", m_bAddFiles);
                        m_Xml->get_widget("treeview-tracks", m_tTracks);
                        m_Xml->get_widget("treeview-release", m_tRelease);

                        m_Xml->get_widget("cbe-artist", m_CBE_Artist);
                        m_Xml->get_widget("cbe-album", m_CBE_Album);
                        m_Xml->get_widget("cbe-genre", m_CBE_Genre);
                        m_Xml->get_widget("e-mbid", m_eMBID);

                        m_Model_CBE_Artist = Gtk::ListStore::create( m_Columns_CBE );
                        m_Model_CBE_Album  = Gtk::ListStore::create( m_Columns_CBE );

                        m_CBE_Artist->set_model( m_Model_CBE_Artist );
                        m_CBE_Album->set_model( m_Model_CBE_Album );
       
                        m_CBE_Artist->signal_changed().connect(
                            sigc::mem_fun(
                                    *this,
                                    &MB_ImportAlbum::on_artist_changed
                        ));

                        m_CBE_Album->signal_changed().connect(
                            sigc::mem_fun(
                                    *this,
                                    &MB_ImportAlbum::on_album_changed
                        ));

                        m_bSearch->signal_clicked().connect(
                            sigc::mem_fun(
                                *this,
                                &MB_ImportAlbum::on_button_search
                        ));

                        m_bImport->set_sensitive( false ); 
                        m_bImport->signal_clicked().connect(
                            sigc::mem_fun(
                                *this,
                                &MB_ImportAlbum::on_button_import
                        ));

                        m_bCancel->signal_clicked().connect(
                            sigc::mem_fun(
                                *this,
                                &MB_ImportAlbum::on_button_cancel
                        ));

                        m_bAddFiles->signal_clicked().connect(
                            sigc::mem_fun(
                                *this,
                                &MB_ImportAlbum::on_button_add_files
                        ));

                        m_Model_Release = Gtk::ListStore::create(m_Columns_Release);
                        m_Model_Tracks = Gtk::ListStore::create(m_Columns_Release);

                        m_tRelease->set_model(m_Model_Release);
                        m_tTracks->set_model(m_Model_Tracks);

                        setup_view(*m_tRelease);
                        setup_view(*m_tTracks);
                }

        void
                MB_ImportAlbum::setup_view(Gtk::TreeView& view)
                {
                        Gtk::CellRenderer * cell = manage (new Gtk::CellRendererText);
                        Gtk::TreeViewColumn * col = 0;

                        col = manage( new Gtk::TreeViewColumn("#"));
                        col->pack_start(*cell, false);
                        col->set_cell_data_func( *cell, sigc::bind( sigc::mem_fun( *this, &MB_ImportAlbum::cell_data_func_track ), COL_TRACK));
                        view.append_column(*col);

                        col = manage( new Gtk::TreeViewColumn(_("Title")));
                        col->pack_start(*cell, true);
                        col->set_cell_data_func( *cell, sigc::bind( sigc::mem_fun( *this, &MB_ImportAlbum::cell_data_func_track ), COL_TITLE));
                        view.append_column(*col);

                        col = manage( new Gtk::TreeViewColumn(_("Album")));
                        col->pack_start(*cell, true);
                        col->set_cell_data_func( *cell, sigc::bind( sigc::mem_fun( *this, &MB_ImportAlbum::cell_data_func_track ), COL_ALBUM));
                        view.append_column(*col);

                        col = manage( new Gtk::TreeViewColumn(_("Artist")));
                        col->pack_start(*cell, true);
                        col->set_cell_data_func( *cell, sigc::bind( sigc::mem_fun( *this, &MB_ImportAlbum::cell_data_func_track ), COL_ARTIST));
                        view.append_column(*col);

                        col = manage( new Gtk::TreeViewColumn(_("Time")));
                        col->pack_start(*cell, true);
                        col->set_cell_data_func( *cell, sigc::bind( sigc::mem_fun( *this, &MB_ImportAlbum::cell_data_func_track ), COL_TIME));
                        view.append_column(*col);
                }

        void
                MB_ImportAlbum::cell_data_func_track( Gtk::CellRenderer *basecell, const Gtk::TreeIter& iter, Columns col)
                { 
                        Gtk::CellRendererText & cell = *(dynamic_cast<Gtk::CellRendererText*>(basecell));

                        MPX::Track track = (*iter)[m_Columns_Release.Track];

                        std::string text;
                        text.reserve(256);

                        switch( col )
                        {
                            case COL_TITLE:
                                if( track.has(ATTRIBUTE_TITLE) )
                                {
                                    text = get<std::string>(track[ATTRIBUTE_TITLE].get()); 
                                }
                                break;

                            case COL_ALBUM:
                                if( track.has(ATTRIBUTE_ALBUM) )
                                {
                                    text = get<std::string>(track[ATTRIBUTE_ALBUM].get()); 
                                }
                                break;

                            case COL_ARTIST:
                                if( track.has(ATTRIBUTE_ARTIST) )
                                {
                                    text = get<std::string>(track[ATTRIBUTE_ARTIST].get()); 
                                }
                                break;

                            case COL_TIME:
                                if( track.has(ATTRIBUTE_TIME) )
                                {
                                    gint64 t = get<gint64>(track[ATTRIBUTE_TIME].get());
                                    text = (boost::format ("%lld:%02lld") % (t / 60) % (t % 60)).str();
                                }
                                break;

                            case COL_TRACK:
                                if( track.has(ATTRIBUTE_TRACK) )
                                {
                                    gint64 t = get<gint64>(track[ATTRIBUTE_TRACK].get());
                                    text = (boost::format ("%lld") % t).str(); 
                                }
                                break;

                            default: break;
                        }

                        cell.property_text() = text;
                }

        void
                MB_ImportAlbum::run ()
                {
                    show_all();
                }

        void
                MB_ImportAlbum::on_button_search ()
                {
                    MusicBrainzReleaseV releases; 
                
                    if( !m_eMBID->get_text().empty() )
                    {
                         mb_releases_by_id (m_eMBID->get_text(), releases);
                    }
                    else
                    {
                         mb_releases_query (m_CBE_Artist->get_entry()->get_text(), m_CBE_Album->get_entry()->get_text(), releases);
                    }

                    std::sort( releases.begin(), releases.end() );
                    populate_matches( releases );
                }

        void
                MB_ImportAlbum::on_button_import ()
                {
                }

        void
                MB_ImportAlbum::on_button_cancel ()
                {
                    hide ();
                    m_Model_Tracks->clear();
                    m_Model_Release->clear();
                    /* TODO: Reset release data */
                }

        void
                MB_ImportAlbum::on_button_add_files ()
                {
                    Gtk::FileChooserDialog dialog (_("AudioSource: Select Album Tracks to Import")); 
                    dialog.add_button( Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL );
                    dialog.add_button( Gtk::Stock::ADD, Gtk::RESPONSE_OK );
                    dialog.set_action(Gtk::FILE_CHOOSER_ACTION_OPEN);
                    dialog.set_select_multiple();
                    dialog.set_current_folder(mcs->key_get<std::string>("mpx", "file-chooser-path"));

                    int response = dialog.run();
                    dialog.hide();

                    if( response == Gtk::RESPONSE_OK )
                    {
                        m_Uris = dialog.get_uris();
                        reload_tracks ();
                    }
                }

        void
                MB_ImportAlbum::reload_tracks ()
                {
                    m_Model_Tracks->clear();

                    MetadataReaderTagLib & reader = *(services->get<MetadataReaderTagLib>("mpx-service-taglib"));
                    for(Uris_t::const_iterator i = m_Uris.begin(); i != m_Uris.end(); ++i)
                    {
                            Track track;
                            reader.get( *i, track );
                            (*m_Model_Tracks->append())[m_Columns_Release.Track] = track;
                    }
                }

        void
                MB_ImportAlbum::populate_matches( const MusicBrainzReleaseV& releases )
                {
                      g_message("Got %lld releases.", gint64(releases.size()));

                      m_CBE_Artist->get_entry()->set_text ("");
                      m_CBE_Album->get_entry()->set_text ("");
                      m_eMBID->set_text ("");

                      m_Model_CBE_Artist->clear ();
                      m_Model_CBE_Album->clear ();

                      std::set<std::string> set_artists;

                      for( MusicBrainzReleaseV::const_iterator i = releases.begin (); i != releases.end(); ++i )
                      {
                        const MusicBrainzRelease& release = *i; 

                        m_Releases.insert( std::make_pair( release.releaseId, release ));

                        ArtistIdToReleasesMap::iterator atri = m_Artist_To_Releases_Map.find( release.mArtist.artistId );

                        if( atri != m_Artist_To_Releases_Map.end() ) 
                        {
                            (*atri).second.push_back( release.releaseId );
                        }
                        else
                        {
                            TreeIter iter = m_Model_CBE_Artist->append ();
                            (*iter)[m_Columns_CBE.Text] = release.mArtist.artistName;
                            (*iter)[m_Columns_CBE.ID]   = release.mArtist.artistId;

                            Uris_t id_v;
                            id_v.push_back( release.releaseId );
                            m_Artist_To_Releases_Map.insert( std::make_pair( release.mArtist.artistId, id_v ));
                        }
                      }

                      m_CBE_Artist->set_active (0);
                      m_CBE_Album->set_active (0);
                }

        void
                MB_ImportAlbum::on_artist_changed()
                {
                      if( m_CBE_Artist->get_active_row_number () >= 0 )
                      {
                        m_Model_CBE_Album->clear ();
                        m_CBE_Album->get_entry()->set_text ("");

                        m_Model_Release->clear ();

                        const TreeIter& i = m_CBE_Artist->get_active ();
                        const Uris_t& id_v = m_Artist_To_Releases_Map.find((*i)[m_Columns_CBE.ID])->second;

                        std::set<std::string> ReleaseSet;

                        for (Uris_t::const_iterator i  = id_v.begin(); i != id_v.end(); ++i)
                        {
                          const MusicBrainzRelease& release = m_Releases.find(*i)->second;
                          if( ReleaseSet.find( release.releaseId ) == ReleaseSet.end() )
                          {
                            ReleaseSet.insert( release.releaseId );

                            TreeIter iter = m_Model_CBE_Album->append ();
                            (*iter)[m_Columns_CBE.Text] = release.releaseTitle;
                            (*iter)[m_Columns_CBE.ID]   = release.releaseId;
                          }
                        }

                        //populate_local_store (ORDERING_AUTO);
                        m_CBE_Album->set_active(0);
                        m_bImport->set_sensitive(m_Model_Release->children().size());
                      }
                }

        void
                MB_ImportAlbum::on_album_changed()
                {
                    if( m_CBE_Album->get_active_row_number () >= 0 )
                    {
                        m_Model_Release->clear ();

                        std::string id = m_Releases.find((*m_CBE_Album->get_active())[m_Columns_CBE.ID])->second.releaseId;
                        MusicBrainzReleaseV v;
                        mb_releases_by_id( id, v );

                        if(v.empty())
                        {
                          g_warning("%s: Couldn't fetch release %s from musicbrainz!", G_STRLOC, id.c_str());
                          return;
                        }

                        m_Release = v[0];
                        m_eMBID->set_text( m_Release.releaseId );

                        const MusicBrainzTrackV& x ( m_Release.mTrackV );
                        for (MusicBrainzTrackV::const_iterator i = x.begin(); i != x.end() ; ++i)
                        {
                              TreeIter iter = m_Model_Release->append();
                              MPX::Track t;
                              imbue_track( *i, t );
                              (*iter)[m_Columns_Release.Track] = t;
                        }

    #if 0
                        m_cb_release_date->set_sensitive (0);
                        m_cb_release_event_store->clear ();

                        if( m_Release.mReleaseEventV.size() )
                        {
                          for (MusicBrainzReleaseEventV::const_iterator i = m_Release.mReleaseEventV.begin(); i != m_Release.mReleaseEventV.end(); ++i)
                          {
                            TreeIter iter = m_cb_release_event_store->append ();
                            if (! i->releaseEventCountry.empty())
                              (*iter)[mCrEvents.event_string] = (boost::format ("%s (%s)") % i->releaseEventDate % i->releaseEventCountry).str();
                            else
                              (*iter)[mCrEvents.event_string] = i->releaseEventDate;
                          }
                          m_cb_release_date->set_active (0);
                          m_cb_release_date->set_sensitive (1);
                        }
    #endif

                        m_tRelease->columns_autosize ();
                        //populate_local_store (ORDERING_AUTO);
                        m_bImport->set_sensitive();
                    }
                }
}
