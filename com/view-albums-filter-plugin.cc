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

#include <boost/format.hpp>
#include <glibmm/i18n.h>
#include <sigx/sigx.h>

#include "mpx/com/view-albums-filter-plugin.hh"

#include "mpx/mpx-artist-images.hh"
#include "mpx/mpx-library.hh"
#include "mpx/mpx-main.hh"
#include "mpx/mpx-uri.hh"
#include "mpx/algorithm/aque.hh"
#include "mpx/util-string.hh"
#include "mpx/util-graphics.hh"

#include "xmlcpp/xsd-topalbums-2.0.hxx"
#include "xmlcpp/xsd-artist-similar-2.0.hxx"
#include "mpx/xml/xmltoc++.hh"
#include "mpx/widgets/cell-renderer-vbox.hh"
#include "mpx/widgets/widgetloader.hh"


using namespace MPX::AQE;
using namespace Gnome::Glade;
using namespace Gtk;
using namespace Glib;
using boost::get;

namespace
{
    std::string
    locale_date_string(
        const std::string& date
    )
    {
        struct tm atm;
        char buf[512];

        if( date.size() == 4)
        {
            strptime(date.c_str(), "%Y", &atm);            
            strftime(buf, 512, "%Y", &atm);
        }
        else if( date.size() == 7)
        {
            strptime(date.c_str(), "%Y-%m", &atm);            
            strftime(buf, 512, "%B %Y", &atm);
        }
        else if( date.size() == 10)
        {
            strptime(date.c_str(), "%Y-%m-%d", &atm);            
            strftime(buf, 512, "%d %B %Y", &atm);
        }
        else
            return "";

        std::string rv (buf);
        return rv; 
    }
}

namespace MPX
{
    typedef sigc::signal<void>      SignalChanged_t;

    class ArtistListView
    : public Gtk::TreeView
    , public sigx::glib_auto_dispatchable
    {
            class Columns_t : public Gtk::TreeModelColumnRecord
            {
                public:

                    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> >    Image;
                    Gtk::TreeModelColumn<std::string>                   Name;
                    Gtk::TreeModelColumn<std::string>                   Name_Raw;
                    Gtk::TreeModelColumn<std::string>                   Date_Raw;
                    Gtk::TreeModelColumn<std::string>                   SortKey;
                    Gtk::TreeModelColumn<gint64>                        ID;

                    Columns_t ()
                    {
                        add(Image);
                        add(Name);
                        add(Name_Raw);
                        add(Date_Raw);
                        add(SortKey);
                        add(ID);
                    };
            };

            Columns_t                       Columns;
            Glib::RefPtr<Gtk::TreeStore>    Store;
            IdSet_t                         m_FilterIDs;
            SignalChanged_t                 signal_changed_;
    
            typedef std::map<gint64, TreeIter>       IdIterMap_t;
            typedef std::map<std::string, TreeIter>  MBIDIterMap_t;

            IdIterMap_t m_IdIterMap;
            MBIDIterMap_t m_MBIDIterMap;

            Glib::RefPtr<Gdk::Pixbuf>       m_Artist_Default;

        public:

            SignalChanged_t&
            signal_changed(
            )
            {
                return signal_changed_;
            }

            const IdSet_t&
            get_filter_ids(
            )
            {
                return m_FilterIDs;
            }

        protected:

            void
            build_list ()    
            {
                boost::shared_ptr<Library> library = services->get<Library>("mpx-service-library");

                MPX::SQL::RowV v;
                library->getSQL(
                      v
                    , "SELECT id, album_artist, album_artist_sortname, mb_album_artist_id, life_span_begin, life_span_end FROM album_artist"
                );

                TreeIter iter = Store->append();
                (*iter)[Columns.Name] = "<b>All Artists</b>"; 

                for( MPX::SQL::RowV::iterator i = v.begin(); i != v.end(); ++i )
                {
                    TreeIter iter = Store->append();

                    std::string name = (*i).count("album_artist_sortname")
                                                ? get<std::string>((*i)["album_artist_sortname"])
                                                : get<std::string>((*i)["album_artist"]);

                    (*iter)[Columns.Name_Raw] = name; 

                    std::string date;

                    if( (*i).count("life_span_begin"))
                    {
                        std::string s = get<std::string>((*i)["life_span_begin"]);
                        if( s.length() )
                        {
                            date = "<small>" + locale_date_string( s ) + "</small>";
                        }
                    }

                    date += "\n";

                    if( (*i).count("life_span_end"))
                    {
                        std::string s = get<std::string>((*i)["life_span_end"]);
                        if( s.length() )
                        {
                            date += "<small>" + locale_date_string( s ) + "</small>"; 
                        }
                    }

                    (*iter)[Columns.Date_Raw] = date; 

                    //(*iter)[Columns.Name] = "<span><b>"+Glib::Markup::escape_text(name)+"</b></span>"; //\n" + date; 
                    (*iter)[Columns.Name] = Glib::Markup::escape_text(name);
                    (*iter)[Columns.SortKey] = Glib::ustring(name).collate_key();
                    (*iter)[Columns.ID] = get<gint64>((*i)["id"]);
                    (*iter)[Columns.Image] = m_Artist_Default;

                    m_IdIterMap.insert(std::make_pair( get<gint64>((*i)["id"]), iter));

                    if( (*i).count("mb_album_artist_id"))
                        m_MBIDIterMap.insert(std::make_pair( get<std::string>((*i)["mb_album_artist_id"]), iter));
                }
            }

            void
            cell_data_func_artist( CellRenderer * basecell, const TreeIter& iter )
            {
                CellRendererVBox & cvbox = *dynamic_cast<CellRendererVBox*>(basecell);

                CellRendererPixbuf *cell1 = dynamic_cast<CellRendererPixbuf*>(cvbox.property_renderer1().get_value());
                CellRendererText   *cell2 = dynamic_cast<CellRendererText*>(cvbox.property_renderer2().get_value());

                cell1->property_pixbuf() = (*iter)[Columns.Image] ;
                cell2->property_markup() = Glib::ustring((*iter)[Columns.Name]) ;
            }
        

        public:

            ArtistListView()
            {
                set_headers_visible( false );

                Cairo::RefPtr<Cairo::ImageSurface> surface = Util::cairo_image_surface_round( Util::cairo_image_surface_from_pixbuf(  Gdk::Pixbuf::create_from_file( build_filename( build_filename( DATA_DIR, "images" ), "artist.png" ))), 2.88 );
                Util::cairo_image_surface_rounded_border(surface, .5, 2.88, 0., 0., 0., 1.); 

                m_Artist_Default = Util::cairo_image_surface_to_pixbuf( surface ); 

                Store = Gtk::TreeStore::create(Columns);
                Store->set_default_sort_func(
                    sigc::mem_fun(
                        *this,
                        &ArtistListView::sort
                ));
                Store->set_sort_column_id( -1, Gtk::SORT_ASCENDING );

                //CellRendererVBox *cvbox = manage( new CellRendererVBox ) ;
                TreeViewColumn *col = manage( new TreeViewColumn ) ;

                /*
                CellRendererPixbuf *cell1 = manage( new CellRendererPixbuf ) ;
                cell1->property_ypad() = 8 ;
                cvbox->property_renderer1() = cell1 ;
                */

                CellRendererText *cell2 = manage( new CellRendererText ) ;
                cell2->property_xalign() = 0.0 ;
                cell2->property_ellipsize() = Pango::ELLIPSIZE_END ;
                //cvbox->property_renderer2() = cell2 ;

                col->pack_start( *cell2, true ) ;
                col->add_attribute( *cell2, "markup", Columns.Name ); 
            
                /*
                col->set_cell_data_func(
                          *cvbox    
                        , sigc::mem_fun(
                            *this,
                            &ArtistListView::cell_data_func_artist
                        ));
                */

                append_column( *col ) ;

                /*
                { 
                    // Image
                    TreeViewColumn * col = manage (new TreeViewColumn(_("Image")));
                    CellRendererPixbuf *cell = manage (new CellRendererPixbuf);
                    col->pack_start( *cell, true );
                    col->add_attribute( *cell, "pixbuf", Columns.Image );
                    append_column(*col);
                }

                { 
                    // Text
                    TreeViewColumn * col = manage (new TreeViewColumn(_("Name")));
                    CellRendererText *cell = manage (new CellRendererText);
                    col->pack_start( *cell, false );
                    col->add_attribute( *cell, "markup", Columns.Name );
                    append_column(*col);
                }
                */

                boost::shared_ptr<Library> library = services->get<Library>("mpx-service-library");

                build_list();
                set_model(Store);
                set_enable_search();
                set_search_column( Columns.Name_Raw ) ; 

                get_selection()->signal_changed().connect(
                    sigc::mem_fun(
                        *this,
                        &ArtistListView::on_selection_changed
                ));

                /*
                boost::shared_ptr<ArtistImages> artist_images = services->get<ArtistImages>("mpx-service-artist-images");

                artist_images->signal_got_artist_image().connect(
                    sigc::mem_fun(
                        *this,
                        &ArtistListView::on_got_artist_image
                ));

                artist_images->recache_images();
                */
            }

            void
            on_got_artist_image(
                  const std::string&            mbid
                , Glib::RefPtr<Gdk::Pixbuf>     artist_image
            )
            {
                if( m_MBIDIterMap.count( mbid ))
                {
                    TreeIter iter = m_MBIDIterMap.find( mbid )->second;

                    Cairo::RefPtr<Cairo::ImageSurface> surface = Util::cairo_image_surface_round( Util::cairo_image_surface_from_pixbuf( artist_image->scale_simple( 90, 90, Gdk::INTERP_BILINEAR) ), 2.88 );
                    Util::cairo_image_surface_rounded_border(surface, .5, 2.88, 0., 0., 0., 1.); 
                    (*iter)[Columns.Image] = Util::cairo_image_surface_to_pixbuf( surface ); 
                }
            }

            int
            sort(
                  const TreeIter& iter_a
                , const TreeIter& iter_b
            )
            {
                return std::string((*iter_a)[Columns.SortKey]).compare(std::string((*iter_b)[Columns.SortKey]));
            }

            void
            on_new_artist(
                gint64 id
            )
            {
                MPX::SQL::RowV v;
                boost::shared_ptr<Library> library = services->get<Library>("mpx-service-library");

                library->getSQL(
                      v
                    , (boost::format("SELECT album_artist, album_artist_sortname, mb_album_artist_id, life_span_begin, life_span_end FROM album_artist WHERE id = '%lld'") % id).str()
                );

                TreeIter iter = Store->append();

                std::string name = v[0].count("album_artist_sortname")
                                            ? get<std::string>(v[0]["album_artist_sortname"])
                                            : get<std::string>(v[0]["album_artist"]);

                (*iter)[Columns.Name_Raw] = name; 

                std::string date;

                if( v[0].count("life_span_begin"))
                {
                    std::string s = get<std::string>(v[0]["life_span_begin"]);
                    if( s.length() )
                    {
                        date = "<small>" + locale_date_string( s ) + "</small>";
                    }
                }

                date += "\n";

                if( v[0].count("life_span_end"))
                {
                    std::string s = get<std::string>(v[0]["life_span_end"]);
                    if( s.length() )
                    {
                        date += "<small>" + locale_date_string( s ) + "</small>"; 
                    }
                }

                (*iter)[Columns.Date_Raw] = date; 

                //(*iter)[Columns.Name] = "<span><b>"+Glib::Markup::escape_text(name)+"</b></span>"; //\n" + date; 
                (*iter)[Columns.Name] = Glib::Markup::escape_text(name);
                (*iter)[Columns.SortKey] = Glib::ustring(name).collate_key();
                (*iter)[Columns.ID] = id; 
                (*iter)[Columns.Image] = m_Artist_Default;

                m_IdIterMap.insert(std::make_pair( id, iter));

                if( v[0].count("mb_album_artist_id"))
                    m_MBIDIterMap.insert(std::make_pair( get<std::string>(v[0]["mb_album_artist_id"]), iter));
            }

            void
            on_new_album(
                  gint64 G_GNUC_UNUSED 
            )
            {
                signal_changed_.emit();
            }

            void
            on_entity_deleted(
                  gint64     id
                , EntityType type
            )
            {
                if( type == ENTITY_ALBUM_ARTIST && m_IdIterMap.count(id))
                {
                    TreeIter iter = m_IdIterMap.find(id)->second;
                    Store->erase(iter);
                    m_IdIterMap.erase(id); 

                    MPX::SQL::RowV v;
                    boost::shared_ptr<Library> library = services->get<Library>("mpx-service-library");

                    library->getSQL(
                          v
                        , (boost::format("SELECT mb_album_artist_id FROM album_artist WHERE id = '%lld'") % id).str()
                    ) ;

                    m_MBIDIterMap.erase(get<std::string>(v[0]["mb_album_artist_id"]));
                }

                on_selection_changed();
            }

            void
            on_entity_updated(
                  gint64     id
                , EntityType type
            )
            {
                if( type == ENTITY_ALBUM_ARTIST && m_IdIterMap.count(id))
                {
                    TreeIter iter = m_IdIterMap.find(id)->second;

                    MPX::SQL::RowV v;

                    boost::shared_ptr<Library> library = services->get<Library>("mpx-service-library");
                    library->getSQL(
                          v
                        , (boost::format("SELECT album_artist, album_artist_sortname, mb_album_artist_id, life_span_begin, life_span_end FROM album_artist WHERE id = '%lld'") % id).str()
                    );

                    std::string name = v[0].count("album_artist_sortname")
                                                ? get<std::string>(v[0]["album_artist_sortname"])
                                                : get<std::string>(v[0]["album_artist"]);

                    (*iter)[Columns.Name_Raw] = name; 

                    std::string date;

                    if( v[0].count("life_span_begin"))
                    {
                        std::string s = get<std::string>(v[0]["life_span_begin"]);
                        if( s.length() )
                        {
                            date = "<small>" + locale_date_string( s ) + "</small>";
                        }
                    }

                    date += "\n";

                    if( v[0].count("life_span_end"))
                    {
                        std::string s = get<std::string>(v[0]["life_span_end"]);
                        if( s.length() )
                        {
                            date += "<small>" + locale_date_string( s ) + "</small>"; 
                        }
                    }

                    (*iter)[Columns.Date_Raw] = date; 

                    //(*iter)[Columns.Name] = "<span><b>"+Glib::Markup::escape_text(name)+"</b></span>"; //\n" + date; 
                    (*iter)[Columns.Name] = Glib::Markup::escape_text(name);
                    (*iter)[Columns.SortKey] = Glib::ustring(name).collate_key();
                    (*iter)[Columns.ID] = get<gint64>(v[0]["id"]);

                    m_IdIterMap.insert(std::make_pair( get<gint64>(v[0]["id"]), iter));

                    if( v[0].count("mb_album_artist_id"))
                        m_MBIDIterMap.insert(std::make_pair( get<std::string>(v[0]["mb_album_artist_id"]), iter));

                }

                on_selection_changed();
            }


            void
            on_selection_changed()
            {
                if( !get_selection()->count_selected_rows())
                {
                    m_FilterIDs.clear();
                    return;
                }

                TreeIter iter = get_selection()->get_selected();
                if( iter == Store->children().begin() )
                {
                    m_FilterIDs.clear();
                }
                else
                {
                    m_FilterIDs.clear();
                    m_FilterIDs.insert( (*iter)[Columns.ID] );
                }

                signal_changed_.emit();
            }
    };
}

namespace MPX
{
    namespace ViewAlbumsFilterPlugin
    {
            TextMatch::TextMatch(
                          Glib::RefPtr<Gtk::TreeStore>&     store
                        , ViewAlbumsColumns_t&                columns
            )
            : Base( store, columns )
            , m_ArtistListView( new ArtistListView )
            {
                m_UI = manage( new Gtk::VBox );

                Gtk::ScrolledWindow * sw = manage( new Gtk::ScrolledWindow );

                m_ArtistListView = new ArtistListView; 
                m_ArtistListView->set_rules_hint();
                m_ArtistListView->signal_changed().connect(
                    sigc::mem_fun(
                        *this,
                        &TextMatch::on_artist_filter_changed
                ));

                sw->add( *m_ArtistListView );
                sw->set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC );
                sw->set_size_request( -1, 180 );
                sw->set_shadow_type( Gtk::SHADOW_IN );

                m_UI->pack_start( *sw );
                m_UI->show_all();
            }

            TextMatch::~TextMatch ()
            {
            }

            Gtk::Widget*
            TextMatch::get_ui ()
            {
                return m_UI;
            }

            void
            TextMatch::on_artist_filter_changed(
            )
            {
                Signals.Refilter.emit();
            }

            void
            TextMatch::on_filter_issued( const Glib::ustring& G_GNUC_UNUSED )
            {
                // for textmach, we don't need to do anything
            }

            void
            TextMatch::on_filter_changed( const Glib::ustring& text )
            {
                m_FilterText = text;
                m_Constraints.clear();
                m_FilterEffective = AQE::parse_advanced_query( m_Constraints, m_FilterText ); 
                Signals.Refilter.emit();
            }

            bool
            TextMatch::filter_delegate(const Gtk::TreeIter& iter)
            {
                const IdSet_t& s (m_ArtistListView->get_filter_ids());

                if( s.empty() || s.count( (*iter)[Columns.AlbumArtistId] ))
                {
                        if( m_FilterEffective.empty() && m_Constraints.empty() ) 
                        {
                            return true;
                        }
                        else
                        {
                            bool truth = true;

                            if( !m_Constraints.empty() )
                            {
                                MPX::Track& track = *(Track_sp((*iter)[Columns.AlbumTrack]));
                                truth = AQE::match_track( m_Constraints, track );
                            }

                            ustring_sp p = (*iter)[Columns.Text];

                            if( p )
                                return truth && Util::match_keys( *p, m_FilterEffective ); 
                            else
                                return truth;
                        }
                }
                else
                        return false;
            }
    }
} // end namespace MPX 

namespace MPX
{
    namespace ViewAlbumsFilterPlugin
    {
            LFMTopAlbums::LFMTopAlbums(
                          Glib::RefPtr<Gtk::TreeStore>&     store
                        , ViewAlbumsColumns_t&                columns
            )
            : Base( store, columns )
            {
            }

            LFMTopAlbums::~LFMTopAlbums ()
            {
            }

            Gtk::Widget*
            LFMTopAlbums::get_ui ()
            {
                return 0;
            }

            void
            LFMTopAlbums::on_filter_issued( const Glib::ustring& text )
            {
                m_FilterText = text;
                m_Names.clear();

                if( !m_FilterText.empty() )
                {
                        try{
                                URI u ((boost::format ("http://ws.audioscrobbler.com/2.0/?method=tag.gettopalbums&tag=%s&api_key=37cd50ae88b85b764b72bb4fe4041fe4") % m_FilterText.c_str()).str(), true);
                                MPX::XmlInstance<lfm> * Xml = new MPX::XmlInstance<lfm>(Glib::ustring(u));

                                for( topalbums::album_sequence::const_iterator i = Xml->xml().topalbums().album().begin(); i != Xml->xml().topalbums().album().end(); ++i )
                                {
                                    m_Names.insert( AlbumQualifier_t((*i).name(),(*i).artist().name()));
                                }

                                delete Xml;
                        }
                        catch( ... ) {
                                g_message("Exception!");
                        }
                }

                Signals.Refilter.emit();
            }

            void
            LFMTopAlbums::on_filter_changed( const Glib::ustring& G_GNUC_UNUSED )
            {
            }

            bool
            LFMTopAlbums::filter_delegate(const Gtk::TreeIter& iter)
            {
                return m_FilterText.empty() || m_Names.count( AlbumQualifier_t((*iter)[Columns.Album], (*iter)[Columns.AlbumArtist]));
            }
    }
} // end namespace MPX 

namespace MPX
{
    namespace ViewAlbumsFilterPlugin
    {
            LFMSimilarArtists::LFMSimilarArtists(
                          Glib::RefPtr<Gtk::TreeStore>&     store
                        , ViewAlbumsColumns_t&              columns
            )
            : Base( store, columns )
            {
            }

            LFMSimilarArtists::~LFMSimilarArtists ()
            {
            }

            Gtk::Widget*
            LFMSimilarArtists::get_ui ()
            {
                return 0;
            }

            void
            LFMSimilarArtists::on_filter_issued( const Glib::ustring& text )
            {
                m_FilterText = text;
                m_Names.clear();

                if( !m_FilterText.empty() )
                {
                        try{
                                URI u ((boost::format ("http://ws.audioscrobbler.com/2.0/?method=artist.getsimilar&artist=%s&api_key=37cd50ae88b85b764b72bb4fe4041fe4") % m_FilterText.c_str()).str(), true);
                                MPX::XmlInstance<LastFM_SimilarArtists::lfm> * Xml = new MPX::XmlInstance<LastFM_SimilarArtists::lfm>(Glib::ustring(u));

                                for( LastFM_SimilarArtists::similarartists::artist_sequence::const_iterator i = Xml->xml().similarartists().artist().begin(); i != Xml->xml().similarartists().artist().end(); ++i )
                                {
                                    m_Names.insert( (*i).name() );
                                }

                                delete Xml;
                        }
                        catch( ... ) {
                                g_message("Exception!");
                        }
                }

                Signals.Refilter.emit();
            }

            void
            LFMSimilarArtists::on_filter_changed( const Glib::ustring& G_GNUC_UNUSED )
            {
            }

            bool
            LFMSimilarArtists::filter_delegate(const Gtk::TreeIter& iter)
            {
                return m_FilterText.empty() || m_Names.count( (*iter)[Columns.AlbumArtist] );
            }
    }
} // end namespace MPX 
