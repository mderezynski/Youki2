#ifdef HAVE_CONFIG_H
#include "config.h"
#endif 

#include "youki-controller.hh"
#include "youki-view-albums.hh"
#include "youki-view-artist.hh"

#include <tr1/random>

#include <glibmm/i18n.h>
#include <gdk/gdkkeysyms.h>

#include <boost/format.hpp>
#include <boost/ref.hpp>
#include <boost/random.hpp>
#include <boost/random/mersenne_twister.hpp>

#include "xmlcpp/xsd-track-top-tags-2.0.hxx"

#include "mpx/com/view-tracks.hh"

#include "mpx/mpx-main.hh"
#include "mpx/mpx-covers.hh"
#include "mpx/mpx-types.hh"
#include "mpx/mpx-stock.hh"

#include "mpx/util-string.hh"
#include "mpx/widgets/cairo-extensions.hh"
#include "mpx/widgets/percentual-distribution-hbox.hh"
#include "mpx/widgets/percentual-distribution-hbuttonbox.hh"

#include "mpx/mpx-xspf-write.hh"
#include "mpx/xml/xmltoc++.hh"

#include "library.hh"
#include "plugin-manager-gui.hh"
#include "play.hh"
#include "preferences.hh"

#include "mpx/i-youki-theme-engine.hh"

using boost::get ;
using boost::algorithm::split ;
using boost::algorithm::is_any_of ;
using boost::algorithm::find_first ;

namespace
{
    const char* main_menubar_ui =
    "<ui>"
    ""
    "<menubar name='MenuBarMain'>"
    ""
    "	<menu action='MenuFile'>"
    "	    <menuitem action='FileActionPreferences'/>"
    "	    <menuitem action='FileActionLibrary'/>"
    "	    <menuitem action='FileActionPlugins'/>"
    "	    <separator/>"
    "	    <menuitem action='FileActionAbout'/>"
    "	</menu>"
    "	<menu action='MenuView'>"
    "	    <menuitem action='ViewActionShowHideBrowser'/>"
    "	    <menuitem action='ViewActionFollowPlayingTrack'/>"
    "	    <separator/>"
    "	    <menuitem action='ViewActionUnderlineMatches'/>"
    "	</menu>"
    "	<menu action='MenuPlaybackControl'>"
    "	    <menuitem action='PlaybackControlActionUseHistory'/>"
    "	    <menuitem action='PlaybackControlActionMarkov'/>"
    "	    <menuitem action='PlaybackControlActionContinueCurrentAlbum'/>"
    "	</menu>"
    "</menubar>"
    ""
    "<popup name='PopupMenuTrackList'>"
    "       <menuitem action='ContextAddToQueue'/>"
    "       <menuitem action='ContextAddToQueueNext'/>"
    "       <separator/>"
    "       <menuitem action='ContextShowAlbum'/>"
    "       <menuitem action='ContextShowArtist'/>"
    "       <separator/>"
    "       <menuitem action='ContextRemoveFromQueue'/>"
    "       <menuitem action='ContextClearQueue'/>"
    "       <separator/>"
    "       <menuitem action='ContextStopAfterCurrent'/>"
    "       <separator/>"
    "	    <menu name='YoukiDJ' action='youkidj'>"
    "		<menuitem action='ContextQueueOpArtist'/>"
    "		<menuitem action='ContextQueueOpAlbum'/>"
    "		<separator/>"
    "		<menuitem action='ContextShowArtistSimilar'/>"
    "	    </menu>" 
    "       <separator/>"
    "       <menuitem action='ContextLoadXSPF'/>"
    "       <menuitem action='ContextSaveXSPF'/>"
    "       <menuitem action='ContextXSPFSaveHistory'/>"
     "</popup>"
    "</ui>"
    ;

    struct AttrInfo_t
    {
	int Attr ;
	const char* AttrName ;
    } ;
   
/* 
    AttrInfo_t
    mpris_attribute_id_str[] =
    {
	{ MPX::ATTRIBUTE_ALBUM, "xesam:album" },
	{ MPX::ATTRIBUTE_ALBUM_ARTIST, "xesam:albumArtist" },
	{ MPX::ATTRIBUTE_ARTIST, "xesam:artist" },
	{ MPX::ATTRIBUTE_TITLE, "xesam:title" },
    };

    AttrInfo_t
    mpris_attribute_id_int[] =
    {
	{ MPX::ATTRIBUTE_TRACK, "xesam:trackNumber" },
	{ MPX::ATTRIBUTE_TIME, "mpris:length" },
    };
*/

    bool
    CompareAlbumArtists(
          const MPX::SQL::Row&   r1
        , const MPX::SQL::Row&   r2
    )
    {
        return Glib::ustring(::MPX::Util::row_get_album_artist_name( r1 )) < Glib::ustring(::MPX::Util::row_get_album_artist_name( r2 )) ;
    }

    void
    handle_sql_error( MPX::SQL::SqlGenericError & cxe )
    {
        Gtk::Dialog dialog ("SQL Error", true) ;
        Gtk::Label label ;
        label.set_text( cxe.what() ) ;
        dialog.get_vbox()->pack_start( label, true, true ) ;
        dialog.add_button("OK", Gtk::RESPONSE_DELETE_EVENT) ;
        dialog.show_all() ; 
        dialog.run() ;
        std::abort() ;
    }

    void
    show_about_window( Gtk::Window* win )
    {
	Gtk::AboutDialog d ;  

	d.set_program_name("Youki") ;
	d.set_version("0.10") ;
	d.set_copyright("(C) 2006-2012 Milosz Derezynki, Chong Kaixiong and others") ;
	d.set_license("GPL v3 or later") ;
	d.set_license_type(Gtk::LICENSE_GPL_3_0) ;

	std::vector<Glib::ustring> authors;
	authors.push_back ("Milosz Derezynski <mderezynski@gmail.com>");
	authors.push_back ("Chong Kai Xiong");
	authors.push_back ("David Le Brun");
	authors.push_back ("Lawrence Gold") ;
	authors.push_back ("Barbara Haupt") ;
	authors.push_back ("Robert Kruus") ;
	authors.push_back ("Boris Peterbarg <boris-p@zahav.net.il>") ;
	authors.push_back ("Roman Bogorodskiy <novel@freebsd.org>") ;
	authors.push_back ("pwned") ;
	authors.push_back ("Edward Brocklesby <ejb@goth.net>") ;
	authors.push_back ("Martin Schlemmer <azarah@gentoo.org>") ;
	authors.push_back ("Saleem Abdulrasool <compnerd@compnerd.org>") ;
	authors.push_back ("Kenneth 'Langly' Ostby <langly@langly.org>") ;
	authors.push_back ("Mart Raudsepp <leio@users.berlios.de>") ;
	authors.push_back ("Olivier Blin <oblin@mandriva.com>") ;
	authors.push_back ("Shlomi Fish <shlomi@iglu.org.il>") ; 
	authors.push_back ("") ;
	authors.push_back ("MP4 Taglib Plugin: Lukáš Lalinský") ;
	d.set_authors( authors ) ;

	std::vector<Glib::ustring> artists;
	artists.push_back ("Milosz Derezynski") ;
	d.set_artists( artists ) ;

	Glib::RefPtr<Gdk::Pixbuf> p_logo = Gdk::Pixbuf::create_from_file( Glib::build_filename( DATA_DIR, "images" G_DIR_SEPARATOR_S "youki256x256.png" )) ;

	d.set_logo( p_logo ) ;

	d.set_modal( true ) ;
	d.set_transient_for( *win ) ;
	d.set_icon( p_logo ) ;

	d.run() ;
    }
  
    typedef boost::optional<guint> OptUInt ;
    
    boost::mt19937 gen ;

    int Rand(int n)
    {
	return gen() % n ;
    }

    void
    clear_deque(
	  Glib::RefPtr<Gtk::Action> a
	, MPX::IdHistory&	    q 
    )
    {
	q.clear() ;
	a->set_sensitive(false) ;
    }
}

namespace MPX
{
    enum EntityType
    {
          ENTITY_TRACK
        , ENTITY_ALBUM
        , ENTITY_ARTIST
        , ENTITY_ALBUM_ARTIST
    };

    struct YoukiController::Private
    { 
        View::Artist::DataModelFilter_sp  FilterModelArtist ;
        View::Albums::DataModelFilter_sp  FilterModelAlbums ;
        View::Tracks::DataModelFilter_sp  FilterModelTracks ;

    } ;

    YoukiController::YoukiController(
    )
    : Glib::ObjectBase( "YoukiController" )
    , Service::Base("mpx-service-controller")
    , private_( new Private )
    , m_main_window(0)
    {
        m_C_SIG_ID_track_new =
            g_signal_new(
                  "track-new"
                , G_OBJECT_CLASS_TYPE(G_OBJECT_GET_CLASS(gobj()))
                , GSignalFlags(G_SIGNAL_RUN_FIRST)
                , 0
                , NULL
                , NULL
                , g_cclosure_marshal_VOID__VOID
                , G_TYPE_NONE
                , 0
        ) ;

        m_C_SIG_ID_track_cancelled =
            g_signal_new(
                  "track-cancelled"
                , G_OBJECT_CLASS_TYPE(G_OBJECT_GET_CLASS(gobj()))
                , GSignalFlags(G_SIGNAL_RUN_FIRST)
                , 0
                , NULL
                , NULL
                , g_cclosure_marshal_VOID__VOID
                , G_TYPE_NONE
                , 0
        ) ;

        m_C_SIG_ID_track_out =
            g_signal_new(
                  "track-out"
                , G_OBJECT_CLASS_TYPE(G_OBJECT_GET_CLASS(gobj()))
                , GSignalFlags(G_SIGNAL_RUN_FIRST)
                , 0
                , NULL
                , NULL
                , g_cclosure_marshal_VOID__VOID
                , G_TYPE_NONE
                , 0
        ) ;

        m_covers    = (services->get<Covers>("mpx-service-covers")).get() ;
        m_play      = (services->get<MPX::Play>("mpx-service-play")).get() ;
        m_library   = (services->get<Library>("mpx-service-library")).get() ;

        m_mlibman_dbus_proxy = services->get<info::backtrace::Youki::MLibMan_proxy_actual>("mpx-service-mlibman").get() ; 

        m_mlibman_dbus_proxy->signal_scan_start().connect(
            sigc::mem_fun(
                  *this
                , &YoukiController::on_library_scan_start
        )) ;

        m_mlibman_dbus_proxy->signal_scan_end().connect(
            sigc::mem_fun(
                  *this
                , &YoukiController::on_library_scan_end
        )) ;

        m_mlibman_dbus_proxy->signal_new_album().connect(
            sigc::mem_fun(
                  *this
                , &YoukiController::on_library_new_album
        )) ;

        m_mlibman_dbus_proxy->signal_new_artist().connect(
            sigc::mem_fun(
                  *this
                , &YoukiController::on_library_new_artist
        )) ;

        m_mlibman_dbus_proxy->signal_new_track().connect(
            sigc::mem_fun(
                  *this
                , &YoukiController::on_library_new_track
        )) ;

        m_mlibman_dbus_proxy->signal_entity_deleted().connect(
            sigc::mem_fun(
                  *this
                , &YoukiController::on_library_entity_deleted
        )) ;

        m_mlibman_dbus_proxy->signal_entity_updated().connect(
            sigc::mem_fun(
                  *this
                , &YoukiController::on_library_entity_updated
        )) ;

	/* Connect Library */
        m_library->signal_album_updated().connect(
            sigc::mem_fun(
                  *this
                , &YoukiController::on_local_library_album_updated
        )) ;

	/* Connect Covers */
        m_covers->signal_got_cover().connect(
            sigc::mem_fun(
                  *this
                , &YoukiController::on_covers_got_cover
        )) ;

        m_covers->signal_no_cover().connect(
            sigc::mem_fun(
                  *this
                , &YoukiController::on_covers_no_cover
        )) ;

	/* Connect Play */
        m_play->signal_eos().connect(
            sigc::bind( sigc::mem_fun(
                  *this
                , &YoukiController::on_play_eos
        ), false )) ;

        m_play->signal_error().connect(
            sigc::mem_fun(
                  *this
                , &YoukiController::on_play_error
        )) ;

        m_play->signal_seek().connect(
            sigc::mem_fun(
                  *this
                , &YoukiController::on_play_seek
        )) ;

        m_play->signal_position().connect(
            sigc::mem_fun(
                  *this
                , &YoukiController::on_play_position
        )) ;

        m_play->property_status().signal_changed().connect(
            sigc::mem_fun(
                  *this
                , &YoukiController::on_play_playstatus_changed
        )) ;

        m_play->signal_stream_switched().connect(
            sigc::mem_fun(
                  *this
                , &YoukiController::on_play_stream_switched
        )) ;

        m_play->signal_metadata().connect(
            sigc::mem_fun(
                  *this
                , &YoukiController::on_play_metadata
        )) ;

	/* Setup main search Entry */
        m_Entry = Gtk::manage( new Gtk::Entry ) ;
	m_Entry->set_size_request( 650, -1 ) ;

        m_Entry->set_icon_from_stock(
	      Gtk::Stock::CLEAR
            , Gtk::ENTRY_ICON_SECONDARY
        ) ; 

	update_entry_placeholder_text() ;
	Glib::signal_timeout().connect( sigc::bind_return<bool>( sigc::mem_fun( *this, &YoukiController::update_entry_placeholder_text ), true ), 2 * 60 * 1000) ;

        m_Entry->signal_icon_press().connect(
	    sigc::mem_fun(
		  *this
                , &YoukiController::handle_search_entry_clear_clicked
        )) ;

        m_Entry->signal_key_press_event().connect(
            sigc::mem_fun(
                  *this
                , &YoukiController::handle_search_entry_key_press_event
        ), true ) ;

        m_Entry->signal_activate().connect(
            sigc::mem_fun(
                  *this
                , &YoukiController::handle_search_entry_activated
        )) ;

	m_Entry->signal_focus_out_event().connect(
	    sigc::mem_fun(
	          *this
                , &YoukiController::handle_search_entry_focus_out
        )) ;

	m_conn4 = m_Entry->signal_changed().connect(
	    sigc::mem_fun(
		  *this
		, &YoukiController::handle_search_entry_changed
	)) ;

        m_VBox = Gtk::manage( new Gtk::VBox ) ;
        m_VBox->set_spacing(2) ;
	m_VBox->set_border_width(0) ;

	//////////

	Gtk::HBox* HBox_PL_Top = Gtk::manage( new Gtk::HBox ) ;
	m_rb1 = Gtk::manage( new Gtk::RadioButton("Library")) ;
	m_rb2 = Gtk::manage( new Gtk::RadioButton("Queue")) ;

	m_rb1->set_mode(false) ;
	m_rb2->set_mode(false) ;

	m_rb1->set_can_focus(false) ;
	m_rb2->set_can_focus(false) ;

	m_rb2->set_sensitive(false) ;

	Gtk::RadioButton::Group gr2 = m_rb1->get_group() ;
	m_rb2->set_group(boost::ref(gr2)) ;
	m_conn3 = m_rb1->signal_toggled().connect( sigc::mem_fun( *this, &YoukiController::handle_show_play_queue_toggled )) ;

	HBox_PL_Top->set_spacing(-1) ;

	auto sc = m_rb1->get_style_context() ;
	sc->set_junction_sides( Gtk::JUNCTION_RIGHT ) ;

	     sc = m_rb2->get_style_context() ;
	sc->set_junction_sides( Gtk::JUNCTION_LEFT ) ;

	HBox_PL_Top->pack_start( *m_rb1, false, false, 0 ) ; 
	HBox_PL_Top->pack_start( *m_rb2, false, false, 0 ) ; 

	//////////

	m_VBox_TL = Gtk::manage( new Gtk::VBox ) ;
	m_VBox_TL->set_spacing(4) ;
	m_VBox_TL->set_border_width(0) ;

	m_Label_TL = Gtk::manage( new Gtk::Label ) ;
	m_Label_TL->set_alignment(0) ;
	m_Label_TL->override_color( Util::make_rgba( 0,0,0 ) ) ;

	Gtk::HBox* HBox_TL = Gtk::manage( new Gtk::HBox ) ;
	HBox_TL->pack_start( *HBox_PL_Top, false, false ) ;
	HBox_TL->pack_start( *m_Label_TL, true, true ) ;
	HBox_TL->set_spacing(10) ;

        m_HBox_Main = Gtk::manage( new Gtk::HBox ) ;
        m_HBox_Main->set_spacing(4) ; 

        m_HBox_Entry = Gtk::manage( new Gtk::HBox ) ;
        m_HBox_Entry->set_spacing(4) ;
        m_HBox_Entry->set_border_width(0) ;

	m_Label_Search = Gtk::manage( new Gtk::Label("Find Mu_sic:", true )) ;
	m_Label_Search->set_mnemonic_widget( *m_Entry ) ;

	m_EventBox_Nearest = Gtk::manage( new Gtk::EventBox ) ;
	m_Label_Nearest = Gtk::manage( new Gtk::Label("")) ;
	m_Label_Nearest->set_ellipsize( Pango::ELLIPSIZE_END ) ;
	m_EventBox_Nearest->add( *m_Label_Nearest ) ;
	m_EventBox_Nearest->signal_button_press_event().connect( sigc::bind_return<bool>(sigc::hide(sigc::mem_fun( *this, &YoukiController::handle_nearest_clicked )), true)) ;

#if 0
	Gtk::HBox * HBox_Navi = Gtk::manage( new Gtk::HBox ) ;
	HBox_Navi->set_spacing(-1) ;
	HBox_Navi->set_border_width(0) ;

	Gtk::Image * iprev = Gtk::manage( new Gtk::Image( Gtk::Stock::GO_BACK, Gtk::ICON_SIZE_BUTTON )) ;
        m_BTN_HISTORY_PREV = Gtk::manage( new Gtk::Button ) ;
	m_BTN_HISTORY_PREV->set_relief( Gtk::RELIEF_NONE ) ;
	m_BTN_HISTORY_PREV->set_border_width(0) ; 
	m_BTN_HISTORY_PREV->add( *iprev ) ;
	m_BTN_HISTORY_PREV->signal_clicked().connect( sigc::mem_fun( *this, &YoukiController::history_go_back)) ;
	m_BTN_HISTORY_PREV->set_sensitive( false ) ;

	Gtk::Image * iffwd = Gtk::manage( new Gtk::Image( Gtk::Stock::GO_FORWARD, Gtk::ICON_SIZE_BUTTON )) ;
        m_BTN_HISTORY_FFWD = Gtk::manage( new Gtk::Button ) ;
	m_BTN_HISTORY_FFWD->set_relief( Gtk::RELIEF_NONE ) ;
	m_BTN_HISTORY_FFWD->set_border_width(0) ; 
	m_BTN_HISTORY_FFWD->add( *iffwd ) ;
//	m_BTN_HISTORY_FFWD->signal_clicked().connect( sigc::mem_fun( *this, &YoukiController::history_go_ffwd)) ;
	m_BTN_HISTORY_FFWD->set_sensitive( false ) ;
#endif

	Gtk::Alignment * Buttons_Align = Gtk::manage( new Gtk::Alignment ) ;
	Gtk::HBox * Buttons_Box = Gtk::manage( new Gtk::HBox ) ;
	Buttons_Box->set_spacing(-1) ;
	Buttons_Align->property_xscale().set_value(0) ;
	Buttons_Align->property_xalign().set_value(1) ;

	Gtk::Image * ishuf = Gtk::manage( new Gtk::Image( Glib::build_filename( DATA_DIR, "images" G_DIR_SEPARATOR_S "media-shuffle.png" ))) ;
	m_BTN_SHUFFLE = Gtk::manage( new Gtk::Button ) ;
	m_BTN_SHUFFLE->add( *ishuf ) ;
	m_conn5 = m_BTN_SHUFFLE->signal_clicked().connect( sigc::mem_fun( *this, &YoukiController::handle_shuffle_toggled )) ;
	m_BTN_SHUFFLE->set_can_focus(false) ;

	Gtk::Image * irept = Gtk::manage( new Gtk::Image( Glib::build_filename( DATA_DIR, "images" G_DIR_SEPARATOR_S "media-repeat.png" ))) ;
	Gtk::Button * brept = Gtk::manage( new Gtk::ToggleButton ) ;
	brept->add( *irept ) ;
	brept->set_can_focus(false) ;

	Gtk::Image * ialbm = Gtk::manage( new Gtk::Image( Glib::build_filename( DATA_DIR, "images" G_DIR_SEPARATOR_S "disc-small.png" ))) ;
	Gtk::Button * balbm = Gtk::manage( new Gtk::ToggleButton ) ;
	balbm->add( *ialbm ) ;
	balbm->set_can_focus(false) ;
	
	sc = m_BTN_SHUFFLE->get_style_context() ;
	sc->set_junction_sides( Gtk::JUNCTION_RIGHT ) ;

	sc = brept->get_style_context() ;
	sc->set_junction_sides( Gtk::JUNCTION_RIGHT | Gtk::JUNCTION_LEFT ) ;

	sc = balbm->get_style_context() ;
	sc->set_junction_sides( Gtk::JUNCTION_LEFT ) ;

	Buttons_Box->pack_start( *m_BTN_SHUFFLE, false, false, 0 ) ;
	Buttons_Box->pack_start( *brept, false, false, 0 ) ;
	Buttons_Box->pack_start( *balbm, false, false, 0 ) ;
	Buttons_Align->add( *Buttons_Box ) ;

#if 0
        HBox_Navi->pack_start( *m_BTN_HISTORY_PREV, false, false, 0 ) ;
        HBox_Navi->pack_start( *m_BTN_HISTORY_FFWD, false, false, 0 ) ;
        m_HBox_Entry->pack_start( *HBox_Navi, false, false, 0 ) ;
#endif

	m_InfoBar = Gtk::manage( new Gtk::InfoBar ) ;
	m_InfoBar->add_button( Gtk::Stock::OK, 0 ) ;
	m_InfoBar->signal_response().connect( sigc::mem_fun( *this, &YoukiController::handle_info_bar_response )) ;
	m_InfoLabel = Gtk::manage( new Gtk::Label ) ;
	m_InfoLabel->set_alignment(0) ;
	dynamic_cast<Gtk::Container*>(m_InfoBar->get_content_area())->add(*m_InfoLabel) ;
	m_InfoBar->hide() ;

	m_AQUE_Spinner = Gtk::manage( new Gtk::Image ) ;
	Glib::RefPtr<Gdk::PixbufAnimation> anim = Gdk::PixbufAnimation::create_from_file(
						    Glib::build_filename( DATA_DIR, "images" G_DIR_SEPARATOR_S "album-cover-loading.gif" )) ;
	m_AQUE_Spinner->set( anim ) ;
	Glib::signal_timeout().connect( sigc::bind_return(sigc::mem_fun( *m_AQUE_Spinner, &Gtk::Widget::queue_draw), true), 100) ;

        m_HBox_Entry->pack_start( *m_Label_Search, false, false, 0 ) ;
        m_HBox_Entry->pack_start( *m_Entry, false, false, 0 ) ;
        m_HBox_Entry->pack_start( *m_AQUE_Spinner, false, false, 0 ) ;
        m_HBox_Entry->pack_start( *m_EventBox_Nearest, false, false, 0 ) ;
        m_HBox_Entry->pack_start( *Buttons_Align, true, true, 0 ) ;

        Gtk::Alignment* Entry_Align = Gtk::manage( new Gtk::Alignment ) ;
        Entry_Align->add( *m_HBox_Entry ) ;
	Entry_Align->set_padding( 2, 4, 4, 4 ) ;
	Entry_Align->property_xalign() = 0. ;
	Entry_Align->property_xscale() = 1. ; 

        Gtk::Alignment* Controls_Align = Gtk::manage( new Gtk::Alignment ) ;
        m_HBox_Controls = Gtk::manage( new Gtk::HBox ) ;
        m_HBox_Controls->set_spacing(0) ;
	Controls_Align->add( *m_HBox_Controls ) ;
	Controls_Align->set_padding( 0, 2, 0, 2 ) ;

        m_VBox_Bottom = Gtk::manage( new Gtk::VBox ) ;
        m_VBox_Bottom->set_spacing(0) ;

	Gtk::VBox* VBox2 = Gtk::manage( new Gtk::VBox ) ;
	VBox2->set_border_width(0) ;
	VBox2->set_spacing(4) ;

	Gtk::Alignment * Main_Align = Gtk::manage( new Gtk::Alignment ) ;
	Main_Align->add( *VBox2 ) ;
	Main_Align->set_padding( 0, 0, 4, 2 ) ;

	Gtk::Alignment * HBox_Main_Align = Gtk::manage( new Gtk::Alignment ) ;
	HBox_Main_Align->add( *m_HBox_Main ) ;
	HBox_Main_Align->set_padding( 0, 0, 1, 4 ) ;

	std::vector<Glib::RefPtr<Gdk::Pixbuf> > pixvector ;
	pixvector.push_back( Gdk::Pixbuf::create_from_file( Glib::build_filename( DATA_DIR, "images" G_DIR_SEPARATOR_S "youki16x16.png" ))) ;
	pixvector.push_back( Gdk::Pixbuf::create_from_file( Glib::build_filename( DATA_DIR, "images" G_DIR_SEPARATOR_S "youki32x32.png" ))) ;
	pixvector.push_back( Gdk::Pixbuf::create_from_file( Glib::build_filename( DATA_DIR, "images" G_DIR_SEPARATOR_S "youki64x64.png" ))) ;
	pixvector.push_back( Gdk::Pixbuf::create_from_file( Glib::build_filename( DATA_DIR, "images" G_DIR_SEPARATOR_S "youki96x96.png" ))) ;
	pixvector.push_back( Gdk::Pixbuf::create_from_file( Glib::build_filename( DATA_DIR, "images" G_DIR_SEPARATOR_S "youki128x128.png" ))) ;

        m_main_window = new MainWindow ;
        m_main_window->set_icon_list( pixvector ) ; 

	// MAIN VIEW WIDGETS	
        m_ScrolledWinArtist = Gtk::manage( new Gtk::ScrolledWindow ) ;
        m_ScrolledWinAlbums = Gtk::manage( new Gtk::ScrolledWindow ) ;
        m_ScrolledWinTracks = Gtk::manage( new Gtk::ScrolledWindow ) ;

	sc = m_ScrolledWinArtist->get_style_context() ;
	sc->set_junction_sides( Gtk::JUNCTION_RIGHT | Gtk::JUNCTION_BOTTOM ) ;

	sc = m_ScrolledWinTracks->get_style_context() ;
	sc->set_junction_sides( Gtk::JUNCTION_RIGHT | Gtk::JUNCTION_LEFT | Gtk::JUNCTION_BOTTOM ) ;

	sc = m_ScrolledWinAlbums->get_style_context() ;
	sc->set_junction_sides( Gtk::JUNCTION_LEFT | Gtk::JUNCTION_BOTTOM ) ;

	// MENUS	
	m_UIManager = Gtk::UIManager::create() ;

	m_UIActions_Main = Gtk::ActionGroup::create("ActionsMain") ;
	m_UIActions_Tracklist = Gtk::ActionGroup::create("ActionsTracklist") ;

	m_UIActions_Main->add( Gtk::Action::create("MenuFile","_Youki")) ; 
	m_UIActions_Main->add( Gtk::Action::create("MenuView","_View")) ; 
	m_UIActions_Main->add( Gtk::Action::create("MenuPlaybackControl","_Playback")) ; 

	m_UIActions_Main->add( Gtk::Action::create_with_icon_name( "FileActionPreferences", "mpx-stock-preferences", "Preferences", "Allows you to set Audio, Library, and other Preferences"), sigc::bind( sigc::mem_fun( *this, &YoukiController::on_show_subsystem_window), 0)) ; 

	m_UIActions_Main->add( Gtk::Action::create_with_icon_name( "FileActionLibrary", "mpx-stock-musiclibrary", "Library", "Allows you to configure which Paths Youki controls"), sigc::bind( sigc::mem_fun( *this, &YoukiController::on_show_subsystem_window), 1)) ; 

	m_UIActions_Main->add( Gtk::Action::create_with_icon_name( "FileActionPlugins", MPX_STOCK_PLUGIN, "Plugins", "Allows you to configure Plugins and turn them on or off"), sigc::bind( sigc::mem_fun( *this, &YoukiController::on_show_subsystem_window), 2)) ; 

	m_UIActions_Main->add( Gtk::Action::create( "FileActionAbout", Gtk::Stock::ABOUT, "About" ), sigc::bind(sigc::ptr_fun(&show_about_window), m_main_window)) ; 

	m_UIActions_Main->add( Gtk::Action::create( "FocusEntry", "FocusEntry" ), Gtk::AccelKey("<F6>"), sigc::mem_fun( *m_Entry, &Gtk::Widget::grab_focus)) ; 

	m_UIActions_Main->add( Gtk::ToggleAction::create( "ViewActionUnderlineMatches", "Highlight Search" ), sigc::mem_fun( *this, &YoukiController::handle_action_underline_matches)) ; 

	m_UIActions_Main->add( Gtk::ToggleAction::create( "PlaybackControlActionStartAlbumAtFavorite", "Start Albums at Favorite Track")) ; 

	auto action_CAM = Gtk::ToggleAction::create( "PlaybackControlActionMarkov", "Enable PlaySense") ; 
	m_UIActions_Main->add( action_CAM, sigc::mem_fun(*this, &YoukiController::check_markov_queue_append )) ;

	auto action_CCA = Gtk::ToggleAction::create( "PlaybackControlActionContinueCurrentAlbum", "Enable AlbumPlay" ) ; 
	m_UIActions_Main->add( action_CCA ); 
	balbm->set_related_action( action_CCA ) ;

	auto action_REP = Gtk::ToggleAction::create( "PlaybackControlActionRepeat", "Repeat Playback" ) ; 
	m_UIActions_Main->add( action_REP ); 
	brept->set_related_action( action_REP ) ;

	auto action_SHF = Gtk::Action::create( "PlaybackControlActionShuffle", "Shuffle Playback" ) ; 
	m_UIActions_Main->add( action_SHF ); 
	m_BTN_SHUFFLE->set_related_action( action_SHF ) ;
	
	auto action_AUH = Gtk::ToggleAction::create( "PlaybackControlActionUseHistory", "Enable Playback History") ;
	m_UIActions_Main->add( action_AUH, sigc::mem_fun( *this, &YoukiController::handle_use_history_toggled )) ; 
	action_AUH->set_active() ;

#if 0
	auto func1 =

	[](Glib::RefPtr<Gtk::Action> shf, Glib::RefPtr<Gtk::ToggleAction> cca)
	{
	    cca->set_active(false) ;
	};

	auto func2 =

	[&](Glib::RefPtr<Gtk::Action> shf, Glib::RefPtr<Gtk::ToggleAction> cca)
	{
	    if( cca->get_active() )
	    {
		clear_play_queue() ;

		if(m_rb2->get_active())
		{
		    m_rb1->set_active() ;
		    m_rb2->set_sensitive(false) ;
		}
	    }
	};

	action_SHF->signal_activate().connect(sigc::bind(func1,action_SHF,action_CCA)) ;
	action_CCA->signal_activate().connect(sigc::bind(func2,action_SHF,action_CCA)) ;
#endif

	auto action_SHB = Gtk::ToggleAction::create( "ViewActionShowHideBrowser", "Show/Hide Browse Panes" ) ;
	action_SHB->set_active() ;

	auto func3 =
    
	[&]()
	{
	    bool active = Glib::RefPtr<Gtk::ToggleAction>::cast_static( m_UIActions_Main->get_action("ViewActionShowHideBrowser"))->get_active() ;
	    if(!active)
	    {
		m_ScrolledWinArtist->hide() ;
		m_ScrolledWinAlbums->hide() ;
	    }
	    else
	    {
		m_ScrolledWinArtist->show() ;
		m_ScrolledWinAlbums->show() ;
	    }
	};

	m_UIActions_Main->add( action_SHB, func3 ) ;


	m_UIActions_Main->add( Gtk::ToggleAction::create( "ViewActionAlbumsShowTimeDiscsTracks", "Show Additional Album Info" ), sigc::mem_fun( *this, &YoukiController::handle_action_underline_matches ) ); 

	Glib::RefPtr<Gtk::ToggleAction> action_MOP = Gtk::ToggleAction::create( "PlaybackControlActionMinimizeOnPause", "Minimize Youki on Pause" ) ;
	m_UIActions_Main->add( action_MOP ) ; 
	//mcs_bind->bind_toggle_action( action_MOP, "mpx", "minimize-on-pause" ) ;

	Glib::RefPtr<Gtk::ToggleAction> action_FPT = Gtk::ToggleAction::create( "ViewActionFollowPlayingTrack", "Follow Playing Track" ) ;
	m_UIActions_Main->add( action_FPT, sigc::mem_fun(*this, &YoukiController::handle_follow_playing_track_toggled)) ; 
	//mcs_bind->bind_toggle_action( action_MOP, "mpx", "follow-current-track" ) ;

	Glib::RefPtr<Gtk::ToggleAction> action_PST = Gtk::ToggleAction::create( "PlaybackControlActionPlayTrackOnSingleTap", "Play Tracks on Single Tap" ) ;
	m_UIActions_Main->add( action_PST, sigc::mem_fun(*this, &YoukiController::handle_play_track_on_single_tap)) ; 
	action_PST->set_active( mcs->key_get<bool>("mpx","play-on-single-tap")) ;
	mcs_bind->bind_toggle_action( action_PST, "mpx", "play-on-single-tap" ) ;

	m_UIActions_Tracklist->add( Gtk::ToggleAction::create( "ContextStopAfterCurrent", "Stop After Current Track" )) ;

	m_UIManager->insert_action_group( m_UIActions_Main ) ;
	m_UIManager->insert_action_group( m_UIActions_Tracklist ) ;
	m_UIManager->add_ui_from_string( main_menubar_ui ) ;

        m_ListViewTracks = Gtk::manage( new View::Tracks::Class(m_play_queue, m_play_history.get(), m_UIManager, m_UIActions_Tracklist)) ;
        m_ListViewArtist = Gtk::manage( new View::Artist::Class ) ;
        m_ListViewAlbums = Gtk::manage( new View::Albums::Class ) ;

	Gtk::Widget * menubar = m_UIManager->get_widget( "/MenuBarMain" ) ;

        m_ScrolledWinArtist->set_shadow_type( Gtk::SHADOW_IN ) ;
        m_ScrolledWinAlbums->set_shadow_type( Gtk::SHADOW_IN ) ;
        m_ScrolledWinTracks->set_shadow_type( Gtk::SHADOW_IN ) ;

        m_ScrolledWinArtist->set_kinetic_scrolling() ;
        m_ScrolledWinAlbums->set_kinetic_scrolling() ;
        m_ScrolledWinTracks->set_kinetic_scrolling() ;

        m_main_position = Gtk::manage( new KoboPosition ) ;
	m_main_position->set_size_request( -1, 21 ) ;
        m_main_position->signal_seek_event().connect(
            sigc::mem_fun(
                  *this
                , &YoukiController::on_position_seek
        )) ;

        m_main_info = new YoukiSpectrumTitleinfo ;
        m_main_info->signal_tapped().connect(
            sigc::mem_fun(
                  *this
                , &YoukiController::on_info_area_clicked
        )) ;

        VBox2->pack_start( *Entry_Align, false, false, 0 ) ;
        VBox2->pack_start( *HBox_Main_Align, true, true, 0 ) ;
        VBox2->pack_start( *m_VBox_Bottom, false, false, 0 ) ;

//	m_VBox_Bottom->pack_start( *m_main_info, false, false, 0 ) ;
        m_VBox_Bottom->pack_start( *Controls_Align, false, false, 0 ) ;

        m_main_volume = Gtk::manage( new KoboVolume ) ;
	m_main_volume->set_size_request( 204, 21 ) ;
        m_main_volume->set_volume(
            m_play->property_volume().get_value()
        ) ;
        m_main_volume->signal_set_volume().connect(
            sigc::mem_fun(
                  *this 
                , &YoukiController::on_volume_set_volume
        )) ;

        m_HBox_Controls->pack_start( *m_main_position, true, true, 0 ) ;
        m_HBox_Controls->pack_start( *m_main_volume, false, false, 0 ) ;
	m_HBox_Controls->show_all() ;

        m_VBox_Bottom->show_all() ;

        m_ScrolledWinArtist->set_policy( Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS ) ; 
        m_ScrolledWinAlbums->set_policy( Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS ) ; 
        m_ScrolledWinTracks->set_policy( Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS ) ; 

        {
	    // TRACKS
	    View::Tracks::DataModel_sp m ( new View::Tracks::DataModel ) ;
	    private_->FilterModelTracks = View::Tracks::DataModelFilter_sp (new View::Tracks::DataModelFilter( m )) ;

	    private_->FilterModelTracks->signal_process_begin().connect( sigc::bind( sigc::mem_fun( *this, &YoukiController::show_hide_spinner), true )) ;
	    private_->FilterModelTracks->signal_process_begin().connect( sigc::bind( sigc::mem_fun( *m_Entry, &Gtk::Widget::set_sensitive), false )) ;
	    private_->FilterModelTracks->signal_process_begin().connect( sigc::bind( sigc::mem_fun( *m_ListViewArtist, &Gtk::Widget::set_sensitive), false )) ;
	    private_->FilterModelTracks->signal_process_begin().connect( sigc::bind( sigc::mem_fun( *m_ListViewAlbums, &Gtk::Widget::set_sensitive), false )) ;
	    private_->FilterModelTracks->signal_process_begin().connect( sigc::bind( sigc::mem_fun( *m_ListViewTracks, &Gtk::Widget::set_sensitive), false )) ;

	    private_->FilterModelTracks->signal_process_end().connect( sigc::bind( sigc::mem_fun( *this, &YoukiController::show_hide_spinner), false )) ;
	    private_->FilterModelTracks->signal_process_end().connect( sigc::bind( sigc::mem_fun( *m_Entry, &Gtk::Widget::set_sensitive), true )) ;
	    private_->FilterModelTracks->signal_process_end().connect( sigc::bind( sigc::mem_fun( *m_ListViewArtist, &Gtk::Widget::set_sensitive), true )) ;
	    private_->FilterModelTracks->signal_process_end().connect( sigc::bind( sigc::mem_fun( *m_ListViewAlbums, &Gtk::Widget::set_sensitive), true )) ;
	    private_->FilterModelTracks->signal_process_end().connect( sigc::bind( sigc::mem_fun( *m_ListViewTracks, &Gtk::Widget::set_sensitive), true )) ;

	    preload__tracks() ;

	    m_ListViewTracks->set_model( private_->FilterModelTracks ) ; 

	    View::Tracks::Column_sp c1 (new View::Tracks::Column(_("Track"))) ;
	    c1->set_column(5) ;
	    c1->set_alignment( Pango::ALIGN_RIGHT ) ;

	    View::Tracks::Column_sp c2 (new View::Tracks::Column(_("Title"))) ;
	    c2->set_column(0) ;

	    View::Tracks::Column_sp c3 (new View::Tracks::Column(_("Time"))) ;
	    c3->set_column(9) ;
	    c3->set_alignment( Pango::ALIGN_RIGHT ) ;

	    View::Tracks::Column_sp c4 (new View::Tracks::Column(_("Artist"))) ;
	    c4->set_column(1) ;

	    View::Tracks::Column_sp c5 (new View::Tracks::Column(_("Album"))) ;
	    c5->set_column(2) ;

	    m_ListViewTracks->append_column(c1) ;
	    m_ListViewTracks->append_column(c2) ;
	    m_ListViewTracks->append_column(c3) ;
	    m_ListViewTracks->append_column(c4) ;
	    m_ListViewTracks->append_column(c5) ;

	    m_ListViewTracks->column_set_fixed(
		  0
		, true
		, 60
	    ) ;

	    m_ListViewTracks->column_set_fixed(
		  2
		, true
		, 60
	    ) ;

	    m_ScrolledWinTracks->set_border_width(0) ;
	    m_ScrolledWinTracks->add( *m_ListViewTracks ) ;
	    m_ScrolledWinTracks->show_all() ;

	    private_->FilterModelTracks->signal_changed().connect( sigc::mem_fun(*this, &YoukiController::handle_model_changed )) ; 
        }

        {
	    // ARTISTS 
	    View::Artist::DataModel_sp m (new View::Artist::DataModel) ;
	    private_->FilterModelArtist = View::Artist::DataModelFilter_sp (new View::Artist::DataModelFilter( m )) ;

	    preload__artists() ;

	    m_ListViewArtist->set_model( private_->FilterModelArtist ) ;

	    View::Artist::Column_sp c1 (new View::Artist::Column) ;
	    c1->set_column(0) ;
	    m_ListViewArtist->append_column(c1) ;

	    m_ScrolledWinArtist->set_border_width(0) ;
	    m_ScrolledWinArtist->add( *m_ListViewArtist ) ;
	    m_ScrolledWinArtist->show_all() ;
        }

        {
	    // ALBUMS

	    View::Albums::DataModel_sp m ( new View::Albums::DataModel ) ;
	    private_->FilterModelAlbums = View::Albums::DataModelFilter_sp (new View::Albums::DataModelFilter( m )) ;

	    preload__albums() ;

	    m_ListViewAlbums->set_model( private_->FilterModelAlbums ) ;

	    View::Albums::Column_sp c1 ( new View::Albums::Column ) ;
	    c1->set_column(0) ;
	    m_ListViewAlbums->append_column( c1 ) ;

	    m_ScrolledWinAlbums->set_border_width(0) ;
	    m_ScrolledWinAlbums->add( *m_ListViewAlbums ) ;
	    m_ScrolledWinAlbums->show_all() ;
        }

        m_ListViewTracks->signal_track_activated().connect(
            sigc::mem_fun(
                      *this
                    , &YoukiController::handle_tracklist_track_activated
        )) ;

        m_ListViewTracks->signal_save_xspf().connect(
            sigc::mem_fun(
                      *this
                    , &YoukiController::handle_save_xspf
        )) ;
 
        m_ListViewTracks->signal_save_xspf_history().connect(
            sigc::mem_fun(
                      *this
                    , &YoukiController::handle_save_xspf_history
        )) ;
 
        m_ListViewTracks->signal_load_xspf().connect(
            sigc::mem_fun(
                      *this
                    , &YoukiController::handle_load_xspf
        )) ;
 
        m_ListViewTracks->signal_remove_track_from_queue().connect(
            sigc::mem_fun(
                      *this
                    , &YoukiController::handle_remove_track_from_queue
        )) ;
 
        m_ListViewTracks->signal_clear_queue().connect(
            sigc::mem_fun(
                      *this
                    , &YoukiController::handle_clear_play_queue
        )) ;
 
        m_ListViewTracks->signal_queue_op_artist().connect(
            sigc::mem_fun(
                      *this
                    , &YoukiController::handle_queue_op_artist
        )) ;
 
        m_ListViewTracks->signal_queue_op_album().connect(
            sigc::mem_fun(
                      *this
                    , &YoukiController::handle_queue_op_album
        )) ;
 
        m_ListViewTracks->signal_find_propagate().connect(
            sigc::mem_fun(
                      *this
                    , &YoukiController::handle_tracklist_find_propagate
        )) ;

        m_ListViewTracks->signal_only_this_album_mbid().connect(
            sigc::mem_fun(
                  *this
                , &YoukiController::handle_albumlist_select_album
        )) ;

        m_ListViewTracks->signal_only_this_artist_mbid().connect(
            sigc::mem_fun(
                  *this
                , &YoukiController::handle_albumlist_select_artist
        )) ;

        m_ListViewTracks->signal_similar_artists_mbid().connect(
            sigc::mem_fun(
                  *this
                , &YoukiController::handle_albumlist_similar_artists
        )) ;

        m_conn1 = m_ListViewArtist->signal_selection_changed().connect(
            sigc::mem_fun(
                  *this
                , &YoukiController::on_list_view_aa_selection_changed
        )) ;

#if 0
        m_conn1 = m_ListViewArtist->signal_selection_changed().connect(
            sigc::mem_fun(
                  *this
                , &YoukiController::history_save
        )) ;
#endif

        m_ListViewArtist->signal_find_accepted().connect(
            sigc::mem_fun(
                  *m_ListViewAlbums
                , &Gtk::Widget::grab_focus
        )) ;

        m_ListViewAlbums->signal_find_accepted().connect(
            sigc::mem_fun(
                  *m_ListViewTracks
                , &Gtk::Widget::grab_focus
        )) ;

        m_conn2 = m_ListViewAlbums->signal_selection_changed().connect(
            sigc::mem_fun(
                  *this
                , &YoukiController::handle_albumlist_selection_changed
        )) ;

#if 0
        m_conn2 = m_ListViewAlbums->signal_selection_changed().connect(
            sigc::mem_fun(
                  *this
                , &YoukiController::history_save
        )) ;
#endif
 
        m_ListViewAlbums->signal_only_this_album_mbid().connect(
            sigc::mem_fun(
                  *this
                , &YoukiController::handle_albumlist_select_album
        )) ;

        m_ListViewAlbums->signal_only_this_artist_mbid().connect(
            sigc::mem_fun(
                  *this
                , &YoukiController::handle_albumlist_select_artist
        )) ;

        m_ListViewAlbums->signal_refetch_cover().connect(
            sigc::mem_fun(
                  *this
                , &YoukiController::handle_albumlist_refetch_cover
        )) ;

        m_ListViewAlbums->signal_start_playback().connect(
            sigc::mem_fun(
                  *this
                , &YoukiController::handle_albumlist_start_playback
        )) ;

        m_ListViewArtist->signal_start_playback().connect(
            sigc::mem_fun(
                  *this
                , &YoukiController::handle_albumlist_start_playback
        )) ;

	m_ListViewArtist->set_size_request( 152, -1 ) ;
	m_ListViewAlbums->set_size_request( 268, -1 ) ;

	m_VBox_TL->pack_start( *HBox_TL, false, true, 0 ) ; 
	m_VBox_TL->pack_start( *m_InfoBar, false, true, 0 ) ; 
	m_VBox_TL->pack_start( *m_ScrolledWinTracks, true, true, 0 ) ; 
	m_VBox_TL->pack_start( *m_main_info, false, true, 0 ) ;

        m_HBox_Main->pack_start( *m_ScrolledWinArtist, false, true, 0 ) ;
        m_HBox_Main->pack_start( *m_ScrolledWinAlbums, false, true, 0 ) ;
        m_HBox_Main->pack_start( *m_VBox_TL, true, true, 0 ) ;

        std::vector<Gtk::Widget*> widget_v( 4 ) ; /* NEEDS TO BE CHANGED IF WIDGETS ARE ADDED */
        widget_v[0] = m_Entry ;
        widget_v[1] = m_ListViewTracks ;
        widget_v[2] = m_ListViewArtist ;
        widget_v[3] = m_ListViewAlbums ;

        m_main_window->set_focus_chain( widget_v ) ;
        m_main_window->set_widget_top( *m_VBox ) ;

	m_VBox->pack_start( *menubar, false, false, 0 ) ;
        m_VBox->pack_start( *Main_Align, true, true, 0 ) ;

	/* Status icon */
        m_status_icon = Gtk::StatusIcon::create( Gdk::Pixbuf::create_from_file( Glib::build_filename( DATA_DIR, "images" G_DIR_SEPARATOR_S "youki32x32.png" ))) ;
        m_status_icon->signal_button_press_event().connect(
            sigc::hide<-1>(sigc::mem_fun(
                  *this
                , &YoukiController::on_status_icon_clicked
        ))) ;

#if 0
	m_view_history_pos = m_view_history.begin() ;
	history_save() ;
#endif
	
	handle_model_changed(0,false) ;
        on_style_changed() ;
        m_main_window->show_all() ;
	m_InfoBar->hide() ;
	m_AQUE_Spinner->hide() ; 
    }

    void
    YoukiController::infobar_set_message(
	  const std::string& msg_str
	, Gtk::MessageType   msg_type
    )
    {
	m_InfoBar->set_message_type( msg_type ) ; 
	m_InfoLabel->set_markup( msg_str ) ;
	m_InfoBar->show_all() ;
    }

    void
    YoukiController::history_save()
    { 
	ViewHistoryIter pos = m_view_history.end() ;
	std::advance(pos,-1) ;

	ViewHistoryItem i ;

	i.FilterText = m_Entry->get_text() ;
	i.SelectionArtist = m_ListViewArtist->get_selected_id() ;
	i.SelectionArtist = m_ListViewAlbums->get_selected_id() ;

	if( m_view_history_pos != pos )
	{
	    m_view_history.erase(pos,m_view_history.end()) ;
	}

	m_view_history.push_back(i) ;
	m_view_history_pos = m_view_history.end() ;
	std::advance(m_view_history_pos,-1) ;
	
	if(!m_view_history.empty())
	{
	    m_BTN_HISTORY_PREV->set_sensitive() ;
	}
    }

    void
    YoukiController::history_go_back()
    { 
	ViewHistoryIter& pos = m_view_history_pos ;
	std::advance(pos,-1) ;

	const ViewHistoryItem& i = *pos ;

	m_Entry->set_text(i.FilterText) ;
	m_ListViewArtist->select_id( i.SelectionArtist ) ;
	m_ListViewArtist->select_id( i.SelectionAlbums ) ;

	if(!m_view_history.size() > 1)
	{
	    m_BTN_HISTORY_FFWD->set_sensitive() ;
	}
    }

    void
    YoukiController::show_hide_spinner( 
	  bool show
    )
    {
	if( show )
	{
	    m_AQUE_Spinner->show() ;
	}	
	else
	{
	    m_AQUE_Spinner->hide() ;
	}
    }

    void
    YoukiController::update_entry_placeholder_text()
    {
	const guint MAX_WORDS = 6 ; // Psychology says the human mind can hold 5-7 "items"; so we take the middle here
	const guint MAX_TRIES = 20 ;

	SQL::RowV v ;

	std::tr1::mt19937 eng;
	eng.seed(time(NULL)) ;
	std::tr1::uniform_int<unsigned int> uni(0,2) ;
    
	std::string s ;

	for( guint n = 0 ; n < MAX_TRIES ; ++n )
	{
	    s.clear() ;

	    guint r = uni(eng) ; 

	    switch(r)
	    {
		case 0:
		    m_library->getSQL(v, "SELECT artist AS s FROM artist ORDER BY random() LIMIT 1") ;
		    break;

		case 1:
		    m_library->getSQL(v, "SELECT album AS s FROM album ORDER BY random() LIMIT 1") ;
		    break;

		case 2:
		    m_library->getSQL(v, "SELECT title AS s FROM track WHERE pcount IS NOT NULL ORDER BY random() LIMIT 1") ;
		    break;
	    }

	    if(!v.empty())
	    {
		s = std::move(boost::get<std::string>(v[0]["s"])) ;
		break ;
	    }
	}

	if(!s.empty())
	{
	    StrV m;
	    split( m, s, is_any_of(" "));
	    if( m.size() > MAX_WORDS )
	    {
		m_Entry->set_placeholder_text(_("Search Your Music here...")) ;
	    }
	    else
	    {
		m_Entry->set_placeholder_text((boost::format("%s '%s'") % _("Type to search... for example") % s).str()) ;
	    }
	}	
	// else we keep the current text
    }

    void
    YoukiController::preload__artists()
    {
	SQL::RowV v ;
	m_library->getSQL(v, (boost::format("SELECT * FROM album_artist")).str()) ; 
	std::stable_sort(v.begin(), v.end(), CompareAlbumArtists) ;

	StrV v_ ;
	StrV artists_lc ;

	for( auto& r : v ) 
	{
	    StrV m;

	    std::string s = std::move(Util::row_get_album_artist_name(r)) ;

	    split( m, s, is_any_of(" ")) ;

	    if( m.size() >= 2 )
	    {
		std::string&& key = Glib::ustring(m[0]).lowercase() ;
		std::string&& val = Glib::ustring(m[1]).lowercase();

		m_ssmap[key].insert(val) ;
	    }
    
	    for( auto& artist : m )
	    {
		v_.push_back(artist) ;
	    }

	    m_nearest__artists.push_back(s) ;
	    artists_lc.push_back(Glib::ustring(s).lowercase()) ;

	    SQL::RowV v2 ;
	    m_library->getSQL(v2, (boost::format("SELECT count(*) AS cnt FROM track_view WHERE mpx_album_artist_id = '%u'") % boost::get<guint>(r["id"])).str()) ; 

	    SQL::RowV v3 ;
	    m_library->getSQL( v3, (boost::format("SELECT sum(time) AS total FROM track_view WHERE mpx_album_artist_id = %u") % boost::get<guint>(r["id"])).str() ) ;

	    private_->FilterModelArtist->append_artist(
		  s  
		, boost::get<std::string>(r["mb_album_artist_id"])
		, boost::get<guint>(v2[0]["cnt"])
		, boost::get<guint>(v3[0]["total"])
		, boost::get<guint>(r["id"])
	    ) ;
	}

	v.clear() ;
	m_library->getSQL(v, (boost::format("SELECT * FROM artist")).str()) ; 

	for( auto& r : v ) 
	{
	    StrV m;

	    auto& s = boost::get<std::string>(r["artist"]) ; 

	    split( m, s, is_any_of(" ")) ;

	    if( m.size() >= 2 )
	    {
		std::string&& key = Glib::ustring(m[0]).lowercase() ;
		std::string&& val = Glib::ustring(m[1]).lowercase();

		m_ssmap[key].insert(val) ;
	    }
    
	    for( auto& artist : m )
	    {
		v_.push_back(artist) ;
	    }

	    m_nearest__artists.push_back(s) ;
	    artists_lc.push_back(Glib::ustring(s).lowercase()) ;

	    if( Glib::ustring(s).lowercase().substr(0,3) == "the" )
	    {
		m_nearest__artists.push_back(s.substr(4)) ;
		artists_lc.push_back(Glib::ustring(s.substr(4)).lowercase()) ;
	    }
	}

	for( auto& p : m_ssmap )
	{
	    LDFN_p ldfn (new LDFindNearest ) ;

	    StrV v2_ ;

	    for( auto& s : p.second )
	    {
		v2_.push_back(s) ;
	    }

	    ldfn->load_dictionary(v2_) ;

	    m_ldmap.insert(std::make_pair(p.first,ldfn)) ;
	}

	v.clear() ;
	m_library->getSQL(v, (boost::format("SELECT artist,pcount FROM artist ORDER BY pcount DESC")).str()) ; 

	for( auto& r : v ) 
	{
	    m_nearest__artists_popular.push_back(boost::get<std::string>(r["artist"])) ; 
	}

	m_find_nearest_artist.load_dictionary(v_) ;
	m_find_nearest_artist_full.load_dictionary(m_nearest__artists) ;
	m_find_nearest_artist_full_lc.load_dictionary(artists_lc) ;

        private_->FilterModelArtist->regen_mapping() ;
    }

    void
    YoukiController::preload__albums()
    {
	SQL::RowV v ;

	try{

	    m_library->getSQL(
	      v , "SELECT album.id AS id, album_artist.mb_album_artist_id AS mbid_artist, album FROM album"
		  " JOIN album_artist"
		  " ON album.album_artist_j = album_artist.id ORDER BY"
		  " album_artist, album_artist_j, mb_release_date, album"
	    ) ; 

	} catch (MPX::SQL::SqlGenericError & cxe )
	{
	    handle_sql_error( cxe ) ;
	}

	for( auto& r : v ) 
	{
	    try{
		private_->FilterModelAlbums->append_album( get_album_from_id( get<guint>(r["id"]))) ;
	    } catch( std::logic_error& cxe )
	    {
		g_message("%s: Error while appending album to model: %s", G_STRLOC, cxe.what()) ;
	    }
	}

        private_->FilterModelAlbums->regen_mapping() ;
    }

    void
    YoukiController::preload__tracks()
    {
	SQL::RowV v ;

	m_library->getSQL(v, (boost::format("SELECT max(id) AS id FROM album_artist")).str()) ; 
	guint max_artist = boost::get<guint>(v[0]["id"]) ;

	v.clear();
	m_library->getSQL(v, (boost::format("SELECT max(id) AS id FROM album")).str()) ; 
	guint max_albums = boost::get<guint>(v[0]["id"]) ;

	private_->FilterModelTracks->set_sizes( max_artist, max_albums ) ;

	v.clear() ;
	m_library->getSQL(v, "SELECT * FROM track_view ORDER BY album_artist, mpx_album_artist_id, mb_release_date, album, discnr, track_view.track") ;

	for( auto& r : v ) 
	{
	    try{
		private_->FilterModelTracks->append_track(r, m_library->sqlToTrack(r, true, false )) ;
	    } catch( Library::FileQualificationError )
	    {
	    }
	}

        private_->FilterModelTracks->create_identity_mapping() ;
	tracklist_regen_mapping() ;
    }

    void
    YoukiController::handle_remove_track_from_queue(
	  guint id
    )
    {
	std::vector<guint>::iterator i = std::find(m_play_queue.begin(), m_play_queue.end(), id) ; 

	if(i != m_play_queue.end())
	{
	    m_ListViewTracks->id_set_sort_order(*i) ;
	    m_play_queue.erase(i) ;
	}

	if(!m_play_queue.empty())
	{
	    guint c = 1 ;

	    for( auto& n : m_play_queue )
	    {
		m_ListViewTracks->id_set_sort_order(n,c++) ;
	    }

	    if(m_rb2->get_active())
	    {
		private_->FilterModelTracks->regen_mapping_sort_order() ;
	    }
	}
	else
	{
	    m_rb1->set_active() ;
	    m_rb2->set_sensitive(false) ;
	}
    }

    void
    YoukiController::clear_play_queue()
    {
	for( auto& n : m_play_queue )
	{
	    m_ListViewTracks->id_set_sort_order(n);
	}

	m_play_queue.clear() ;
    }

    void
    YoukiController::append_to_queue(
	  const SQL::RowV& v_
    )
    {
	guint c = m_play_queue.size() + 1 ;

	for( auto& r : v_ )
	{
	    guint r_id = boost::get<guint>(r.find("id")->second) ;
	    m_play_queue.push_back(r_id) ;
	    m_ListViewTracks->id_set_sort_order(r_id,c++) ;
	}
    }

    void
    YoukiController::append_to_queue(
	  const IdV& v_
    )
    {
	guint c = m_play_queue.size() + 1 ;

	for( auto& id : v_ )
	{
	    m_play_queue.push_back( id ) ;
	    m_ListViewTracks->id_set_sort_order(id,c++) ;
	}
    }

    void
    YoukiController::apply_queue()
    {
	guint c = 1 ;

	for( auto& n : m_play_queue )
	{
	    m_ListViewTracks->id_set_sort_order(n,c++) ;
	}
    }

    void
    YoukiController::handle_clear_play_queue()
    {
	for( auto& n : m_play_queue )
	{
	    m_ListViewTracks->id_set_sort_order(n) ;
	}

	m_play_queue.clear() ;

#if 0
	if( m_track_current )
	{
	    check_markov_queue_append_real(m_track_current) ;
	}
#endif

	if(m_rb2->get_active() || m_play_queue.empty())
	{
	    m_rb1->set_active() ;
	}

	m_rb2->set_sensitive(false) ;
    }
    
    void
    YoukiController::save_xspf_real(
	  const std::string& xspf
    )
    {
	Gtk::FileChooserDialog fcd (_("Youki :: Select Folder to Save Playlist"), Gtk::FILE_CHOOSER_ACTION_SAVE ) ; 
	fcd.add_button(_("Cancel"), Gtk::RESPONSE_REJECT) ;
	fcd.add_button(_("Save Playlist"), Gtk::RESPONSE_ACCEPT) ;

	if( fcd.run() == Gtk::RESPONSE_ACCEPT )
	{
	    try{
		Glib::RefPtr<Gio::File> gfile = Gio::File::create_for_path(fcd.get_filename()) ;
		Glib::RefPtr<Gio::FileOutputStream> out_s = gfile->create_file() ;
		out_s->write(xspf) ;
	    } catch( Glib::Error& cxe ) {
		infobar_set_message(
		      (boost::format("<b>XSPF:</b> while saving: '%s'") % cxe.what()).str()
		    , Gtk::MESSAGE_ERROR
		) ;
	    }
	}	
    }

    void
    YoukiController::handle_save_xspf_history(
    )
    {
	Track_sp_v track_v ;

	IdV v_ = m_play_history.get() ;

	for( auto id : v_ )
	{
	    Track_sp p = m_library->getTrackById(id) ;
	    track_v.push_back(p) ; 
	}	

	std::string xspf = XSPF_write(track_v, m_library->get_uuid()) ;
	save_xspf_real(xspf) ;
    }
 
    void
    YoukiController::handle_save_xspf(
    )
    {
	Track_sp_v track_v ;

	for( auto id : m_play_queue )
	{
	    Track_sp p = m_library->getTrackById(id) ;
	    track_v.push_back(p) ; 
	}	

	std::string xspf = XSPF_write(track_v, m_library->get_uuid()) ;
	save_xspf_real(xspf) ;
    }
 
    void
    YoukiController::handle_load_xspf(
    )
    {
	Gtk::FileChooserDialog fcd (_("Youki :: Select .xspf Playlist to Load"), Gtk::FILE_CHOOSER_ACTION_OPEN ) ; 
	fcd.add_button(_("Cancel"), Gtk::RESPONSE_REJECT) ;
	fcd.add_button(_("Load Playlist"), Gtk::RESPONSE_ACCEPT) ;

	std::string xspf ;
	std::string uuid = m_library->get_uuid() ;
	std::vector<guint> v ;

	if( fcd.run() == Gtk::RESPONSE_ACCEPT )
	{
	    xspf = Glib::file_get_contents(fcd.get_filename()) ;

	    try{
		XSPF_read( xspf, uuid, v ) ;
		append_to_queue(v) ;
		m_rb2->set_sensitive() ;
	    } catch( std::runtime_error& cxe ) {
		infobar_set_message(
		      (boost::format("<b>XSPF:</b> while loading: '%s'") % cxe.what()).str()
		    , Gtk::MESSAGE_ERROR
		) ;
	    }
	}	
    }
 
    void
    YoukiController::handle_queue_op_artist(
	  guint id
    )
    {
        SQL::RowV v ;

        try{
          m_library->getSQL( v, (boost::format("SELECT id,title FROM track_view WHERE mpx_album_artist_id='%u' ORDER BY pcount DESC LIMIT 5") % id).str()) ;
        } catch( MPX::SQL::SqlGenericError & cxe )
        {
            handle_sql_error( cxe ) ;
        }

	append_to_queue(v) ;
	m_rb2->set_sensitive() ;

        PlayStatus status = PlayStatus( m_play->property_status().get_value() ) ;

	if( status == PLAYSTATUS_STOPPED )
	{
	    play_next_queue_item() ;
	}
    }
    
    void
    YoukiController::handle_queue_op_album(
	  guint id
    )
    {
        SQL::RowV v ;

        try{
          m_library->getSQL( v, (boost::format("SELECT id,title FROM track_view WHERE mpx_album_id='%u' ORDER BY pcount DESC LIMIT 3") % id).str()) ;
        } catch( MPX::SQL::SqlGenericError & cxe )
        {
            handle_sql_error( cxe ) ;
        }

	append_to_queue(v) ;
	m_rb2->set_sensitive() ;

        PlayStatus status = PlayStatus( m_play->property_status().get_value() ) ;

	if( status == PLAYSTATUS_STOPPED )
	{
	    play_next_queue_item() ;
	}
    }
 
    void
    YoukiController::handle_show_play_queue_toggled()
    {
	bool v = m_rb2->get_active() ; 

	if(v) {
	    private_->FilterModelTracks->regen_mapping_sort_order() ;
	} else {
	    tracklist_regen_mapping() ;
	}
    }
    
    void
    YoukiController::handle_follow_playing_track_toggled()
    {
        if( m_track_current && 
	    Glib::RefPtr<Gtk::ToggleAction>::cast_static( m_UIActions_Main->get_action("ViewActionFollowPlayingTrack"))->get_active())
        {
            const MPX::Track& track = *m_track_current ;
            guint id_track = boost::get<guint>(*(track[ATTRIBUTE_MPX_TRACK_ID])) ;
	    m_ListViewTracks->scroll_to_id(id_track) ;
        }
    }

    void
    YoukiController::handle_model_changed(guint,bool)
    {
	auto sel_row_albums = m_ListViewAlbums->get_selected_index() ; 
	auto sel_row_artist = m_ListViewArtist->get_selected_index() ; 
	
	std::string s ;

	if(!m_rb2->get_active())
	{
	    if(  sel_row_albums )
	    {
		auto a = private_->FilterModelAlbums->row(*sel_row_albums) ;
		s += (boost::format("<small><b>%s</b> by <b>%s</b></small>") % Glib::Markup::escape_text(a->album) % Glib::Markup::escape_text(a->album_artist)).str() ;
	    }
	    else
	    if(  sel_row_artist )
	    {
		auto a = private_->FilterModelArtist->row(*sel_row_artist) ;
		s += (boost::format("<small>Everything by <b>%s</b></small>") % Glib::Markup::escape_text(boost::get<0>(a))).str() ;
	    }
	}

	m_Label_TL->set_markup(s) ;
    }

    void
    YoukiController::handle_nearest_clicked()
    {
	m_Entry->set_text( m_nearest ) ;
	m_Entry->select_region(0,-1) ;
    } 

    void
    YoukiController::handle_use_history_toggled()
    {
    }
	
    void
    YoukiController::handle_action_underline_matches()
    {
	bool active = Glib::RefPtr<Gtk::ToggleAction>::cast_static( m_UIActions_Main->get_action("ViewActionUnderlineMatches"))->get_active() ;
	m_ListViewTracks->set_highlight( active ) ;

	active = Glib::RefPtr<Gtk::ToggleAction>::cast_static( m_UIActions_Main->get_action("ViewActionAlbumsShowTimeDiscsTracks"))->get_active() ;
	m_ListViewAlbums->set_show_additional_info( active ) ;
    }

    void
    YoukiController::handle_play_track_on_single_tap()
    {
	bool active = Glib::RefPtr<Gtk::ToggleAction>::cast_static( m_UIActions_Main->get_action("PlaybackControlActionPlayTrackOnSingleTap"))->get_active() ;
	m_ListViewTracks->set_play_on_single_tap( active ) ;
    }

    bool
    YoukiController::handle_search_entry_focus_out( GdkEventFocus* G_GNUC_UNUSED )
    {
	update_entry_placeholder_text() ;
	return false ;
    }

    bool
    YoukiController::handle_keytimer()
    {
	if( m_switchfocus && m_keytimer.elapsed() >= 0.080 )
	{
	    m_switchfocus = false ;
	    Glib::signal_idle().connect_once( sigc::mem_fun( *this, &YoukiController::handle_search_entry_changed )) ;
	}

	return true ;
    }

    YoukiController::~YoukiController ()
    {
	Glib::RefPtr<Gtk::ToggleAction>::cast_static( m_UIActions_Main->get_action("PlaybackControlActionStartAlbumAtFavorite"))->set_active(false) ;

        m_play->request_status( PLAYSTATUS_STOPPED ) ; 

        delete m_main_window ;
        delete private_ ;

        m_main_window = 0 ;
        private_ = 0 ;
    }

    Gtk::Window*
    YoukiController::get_widget ()
    {
        return m_main_window ;
    }

/*////////// */
    MPX::View::Albums::Album_sp
    YoukiController::get_album_from_id( guint id )
    {
        SQL::RowV v ;

        try{
          m_library->getSQL( v, (boost::format( "SELECT album, album_disctotal, album.mb_album_id, album.id, "
                                                "album_artist.id AS album_artist_id, album_artist, "
                                                "album_artist_sortname, mb_album_artist_id, mb_album_id, mb_release_type, "
                                                "mb_release_date, album_label, album_discs, album_playscore, album_insert_date FROM album "
                                                "JOIN album_artist ON album.album_artist_j = album_artist.id "
                                                "WHERE album.id = '%u'") % id
                                ).str()) ; 
        } catch( MPX::SQL::SqlGenericError & cxe )
        {
            handle_sql_error( cxe ) ;
        }

        if( v.size() != 1 )
        {
            throw std::logic_error("More than one album with same id = impossible") ;
        }

        SQL::Row & r = v[0] ; 

        Glib::RefPtr<Gdk::Pixbuf> cover_pb ;
        Cairo::RefPtr<Cairo::ImageSurface> cover_is ;

        m_covers->fetch(
              get<std::string>(r["mb_album_id"])
            , cover_pb
            , 256
        ) ;

        if( cover_pb ) 
        {
            cover_is = Util::cairo_image_surface_from_pixbuf( cover_pb ) ;
        }

        SQL::RowV v2 ;
        m_library->getSQL( v2, (boost::format("SELECT count(*) AS cnt FROM track_view WHERE album_j = %u") % id).str() ) ;

        SQL::RowV v3 ;
        m_library->getSQL( v3, (boost::format("SELECT sum(time) AS total FROM track WHERE album_j = %u") % id).str() ) ;

        MPX::View::Albums::Album_sp album ( new MPX::View::Albums::Album ) ;

        album->coverart = cover_is ;
        album->album_id = id ;
        album->artist_id = get<guint>(r["album_artist_id"]) ;
        album->album = get<std::string>(r["album"]) ;
        album->album_artist = get<std::string>(r["album_artist"]) ;
        album->mbid = get<std::string>(r["mb_album_id"]) ;
        album->mbid_artist = get<std::string>(r["mb_album_artist_id"]) ;
        album->type = r.count("mb_release_type") ? get<std::string>(r["mb_release_type"]) : "" ;
        album->year = r.count("mb_release_date") ? get<std::string>(r["mb_release_date"]) : "" ;
        album->label = r.count("album_label") ? get<std::string>(r["album_label"]) : ""  ;
        album->track_count = get<guint>(v2[0]["cnt"]) ;
	album->track_count_release_total = r.count("album_disctotal") ? get<guint>(r["album_disctotal"]) : album->track_count ; 
	album->discs_count = r.count("album_discs") ? get<guint>(r["album_discs"]) : 1 ; // For lack of better knowledge it has to be one? 
        album->album_playscore = get<gdouble>(r["album_playscore"]) ;
	album->insert_date = get<guint>(r["album_insert_date"]) ;
	album->total_time = get<guint>(v3[0]["total"]) ;

        return album ;
    }

    void
    YoukiController::on_style_changed(
    )
    {
        boost::shared_ptr<IYoukiThemeEngine> theme = services->get<IYoukiThemeEngine>("mpx-service-theme") ;

        theme->reload() ;

        const ThemeColor& c_bg   = theme->get_color( THEME_COLOR_BACKGROUND ) ; 
        const ThemeColor& c_base = theme->get_color( THEME_COLOR_BASE ) ; 

        Gdk::RGBA c ;
        c.set_rgba( c_base.get_red(), c_base.get_green(), c_base.get_blue(), 1.0 ) ;

        Gdk::RGBA c2 ;
        c2.set_rgba( c_bg.get_red(), c_bg.get_green(), c_bg.get_blue(), 1.0 ) ;

        m_ListViewArtist->override_background_color( c, Gtk::STATE_FLAG_NORMAL ) ;
        m_ListViewAlbums->override_background_color( c, Gtk::STATE_FLAG_NORMAL ) ;
        m_ListViewTracks->override_background_color( c, Gtk::STATE_FLAG_NORMAL ) ;
    }

    void
    YoukiController::on_library_scan_start(
    )
    {
        private_->FilterModelTracks->disable_fragment_cache() ;
        private_->FilterModelTracks->clear_fragment_cache() ;
    }

    void
    YoukiController::on_library_scan_end(
    )
    {
        push_new_tracks() ;

	boost::optional<guint> id ;

        if( m_track_current && 
	    Glib::RefPtr<Gtk::ToggleAction>::cast_static( m_UIActions_Main->get_action("ViewActionFollowPlayingTrack"))->get_active())
        {
            id = boost::get<guint>((*m_track_current)[ATTRIBUTE_MPX_TRACK_ID].get()) ;
        }

        m_conn1.block() ;
        m_conn2.block() ;
        m_conn4.block() ;

	SQL::RowV v ;
	m_library->getSQL(v, (boost::format("SELECT max(id) AS id FROM album_artist")).str()) ; 
	guint max_artist = boost::get<guint>(v[0]["id"]) ;

	v.clear();
	m_library->getSQL(v, (boost::format("SELECT max(id) AS id FROM album")).str()) ; 
	guint max_albums = boost::get<guint>(v[0]["id"]) ;
	private_->FilterModelTracks->set_sizes( max_artist, max_albums ) ;

	private_->FilterModelTracks->clear_fragment_cache() ;
	private_->FilterModelTracks->enable_fragment_cache() ;

        m_conn1.unblock() ;
        m_conn2.unblock() ;
        m_conn4.unblock() ;

	handle_search_entry_changed() ;
	m_ListViewTracks->queue_draw() ;
    }

    void
    YoukiController::on_library_new_track(
          guint id
    )
    {
        m_new_tracks.push_back( id ) ;
       
        if( m_new_tracks.size() == 50 )  
        {
            push_new_tracks() ;
        }
    }

    void
    YoukiController::on_library_new_artist(
          guint               id
    )
    {
        SQL::RowV v ;

        m_library->getSQL(v, (boost::format("SELECT max(id) AS id FROM album_artist")).str()) ; 
        guint max_artist = boost::get<guint>(v[0]["id"]) ;
        v.clear();
        m_library->getSQL(v, (boost::format("SELECT max(id) AS id FROM album")).str()) ; 
        guint max_albums = boost::get<guint>(v[0]["id"]) ;
        private_->FilterModelTracks->set_sizes( max_artist, max_albums ) ;

        v.clear();
        m_library->getSQL( v, (boost::format( "SELECT * FROM album_artist WHERE id = '%u'" ) % id ).str() ) ; 

	SQL::RowV v2 ;
	m_library->getSQL(v2, (boost::format("SELECT count(*) AS cnt FROM track_view WHERE mpx_album_artist_id = '%u'") % id).str()) ; 

        private_->FilterModelArtist->insert_artist(
              boost::get<std::string>(v[0]["album_artist"])
            , boost::get<std::string>(v[0]["mb_album_artist_id"])
	    , boost::get<guint>(v2[0]["cnt"])
	    , 0 // FIXME FIXME FIXME
            , id 
        ) ; 
	
	private_->FilterModelArtist->regen_mapping() ; 
    }

    struct YoukiController::NewAlbumFetchStruct
    {
	guint id ;
	std::string s1,s2,s3,s4,s5 ;
    };

    bool	
    YoukiController::new_album_idle(
	NewAlbumFetchStruct * p
    )
    {
	MPX::View::Albums::Album_sp a_sp = get_album_from_id( p->id ) ;	

        try{
            SQL::RowV v ;
            m_library->getSQL(v, (boost::format("SELECT max(id) AS id FROM album_artist")).str()) ; 
            guint max_artist = boost::get<guint>(v[0]["id"]) ;
            v.clear();
            m_library->getSQL(v, (boost::format("SELECT max(id) AS id FROM album")).str()) ; 
            guint max_albums = boost::get<guint>(v[0]["id"]) ;
            private_->FilterModelTracks->set_sizes( max_artist, max_albums ) ;

            private_->FilterModelAlbums->insert_album( a_sp ) ; 
	    private_->FilterModelAlbums->regen_mapping() ;

        } catch( std::logic_error ) 
        {
            g_message("Oops") ;
        }

        if( !a_sp->coverart )
        {
            RequestQualifier rq ; 
            rq.mbid     =   p->s1 ;
            rq.asin     =   p->s2 ;
            rq.uri      =   p->s3 ;
            rq.artist   =   p->s4 ;
            rq.album    =   p->s5 ;
            rq.id       =   p->id ;

            m_covers->cache( rq, true, false ) ;
        }

	delete p ;

	return false ;
    }

    void
    YoukiController::on_library_new_album(
          guint                id
        , const std::string&    s1
        , const std::string&    s2
        , const std::string&    s3
        , const std::string&    s4
        , const std::string&    s5
    )
    {
	NewAlbumFetchStruct * p = new NewAlbumFetchStruct ;
      
        p->id = id ;
	p->s1 = s1 ;
	p->s2 = s2 ;
	p->s3 = s3 ;
	p->s4 = s4 ;
	p->s5 = s5 ;

	Glib::signal_idle().connect( sigc::bind( sigc::mem_fun( *this, &YoukiController::new_album_idle), p )) ;
    }

    bool
    YoukiController::on_library_entity_deleted_idle(
          guint                id
        , int                  type
    )
    {
        switch( type )
        {
            case 0: /* track */
            {
                private_->FilterModelTracks->erase_track( id ) ; 
            }
            break ;

            case 1: /* album */
            {
                private_->FilterModelAlbums->erase_album( id ) ; 
            }
            break ;

            case 2: /* artist */
            {
            }
            break ;

            case 3: /* album artist */
            {
                private_->FilterModelArtist->erase_artist( id ) ; 
            }
            break;
        }

	return false ;
    }

    void
    YoukiController::on_library_entity_deleted(
          guint                id
        , int                  type
    )
    {
	on_library_entity_deleted_idle(id,type) ;
	//Glib::signal_idle().connect( sigc::bind( sigc::mem_fun( *this, &YoukiController::on_library_entity_deleted_idle ), id, type )) ;
    }

    void
    YoukiController::on_local_library_album_updated( guint id )
    {
        on_library_entity_updated( id, 1 ) ;
    }

    void
    YoukiController::on_covers_got_cover( guint id )
    {
        Glib::RefPtr<Gdk::Pixbuf> cover_pb ;
        Cairo::RefPtr<Cairo::ImageSurface> cover_is ;

        SQL::RowV v ;
        m_library->getSQL( v, (boost::format( "SELECT mb_album_id FROM album WHERE album.id = '%u'") % id ).str()) ; 
        m_covers->fetch(
              get<std::string>(v[0]["mb_album_id"])
            , cover_pb
            , 256
        ) ;

        if( cover_pb ) 
        {
            cover_is = Util::cairo_image_surface_from_pixbuf( cover_pb ) ;
        }

        private_->FilterModelAlbums->update_album_cover( id, cover_is ) ;

	if( m_track_current )
	{
	    MPX::Track& track = *m_track_current ; 

	    if( track.has( ATTRIBUTE_MB_ALBUM_ID ) )
	    {
		    Glib::RefPtr<Gdk::Pixbuf> cover ;

		    if(  m_covers->fetch(
			  boost::get<std::string>(track[ATTRIBUTE_MB_ALBUM_ID].get())
			, cover
		    ))
		    {
			Gdk::RGBA c = Util::pick_color_for_pixbuf(cover) ;

			m_main_info->set_cover( cover ) ;
			m_main_info->set_color(c) ;
			m_main_position->set_color(c) ;
		    }
	    }
	}
    }

    void
    YoukiController::on_covers_no_cover( guint id )
    {
        private_->FilterModelAlbums->update_album_cover_cancel( id ) ;
    }

    bool
    YoukiController::on_library_entity_updated_idle(
	  guint	id
	, int		type
    )
    {
        switch( type )
        {
            case 0: /* track */
            {
            }
            break ;

            case 1: /* album */
            {
                try{
                    private_->FilterModelAlbums->update_album( get_album_from_id( id )) ; 
                } catch( std::logic_error ) {
                }
                
            }
            break ;

            case 2: /* artist */
            {
            }
            break ;

            case 3: /* album artist */
            {
            }
            break;
        }

	return false ;
    }

    void
    YoukiController::on_library_entity_updated(
          guint                id
        , int                   type
    )
    {
	Glib::signal_idle().connect( sigc::bind( sigc::mem_fun( *this, &YoukiController::on_library_entity_updated_idle ), id, type )) ;
    }


    void
    YoukiController::push_new_tracks(
    )
    {
        for( auto n : m_new_tracks ) 
        {
            SQL::RowV v ;

	    m_library->getSQL(v, (boost::format("SELECT * FROM track_view WHERE track_view.id = '%u' ORDER BY ifnull(album_artist_sortname,album_artist), mb_release_date, album, discnr, track_view.track") % n ).str()) ; 

            if(v.size() != 1)
            {
                g_critical(" Got multiple tracks with the same ID / this can not happen.") ;
                continue ;
            }

            private_->FilterModelTracks->insert_track( v[0], m_library->sqlToTrack( v[0], true, false )) ;
        }

        m_new_tracks.clear() ;
    }

/*////////// */

    void
    YoukiController::play_track(
          const MPX::Track_sp& t
	, bool		       register_in_history
    )
    {
	bool history = Glib::RefPtr<Gtk::ToggleAction>::cast_static( m_UIActions_Main->get_action("PlaybackControlActionUseHistory"))->get_active() ;

        try{
                m_play->request_status( PLAYSTATUS_WAITING ) ;
                m_track_current = t ; 

		guint id1 = boost::get<guint>((*m_track_current)[ATTRIBUTE_MPX_TRACK_ID].get()) ;

		if( history && register_in_history )
		{
		    m_play_history.append(id1) ;
		}

		try{
		    m_play->switch_stream( m_library->trackGetLocation( t ) ) ;
		} catch( std::exception& cxe )
		{
		    infobar_set_message(
			  (boost::format("<b>Error</b> while preparing file: '%s'") % cxe.what()).str()
			, Gtk::MESSAGE_ERROR
		    ) ;
		}

        } catch( Library::FileQualificationError & cxe )
        {
            g_message("%s: Error: What: %s", G_STRLOC, cxe.what());
        }
    }
    
    void
    YoukiController::on_play_seek(
          guint G_GNUC_UNUSED
    )
    {
        m_seek_position.reset() ; 
    }

    void
    YoukiController::on_play_position(
          guint position
    )
    {
        guint duration = m_play->property_duration().get_value() ;
    
        if( m_seek_position && guint64(position) < m_seek_position.get() ) 
        {
            return ;
        }

        m_main_position->set_position( duration, position ) ;
    }

    void
    YoukiController::on_play_error(
	  const std::string&	a
	, const std::string&	b
	, const std::string&	c
    )
    {
	infobar_set_message(
	      (boost::format("%s: %s (%s)") % a % b % c).str()
	    , Gtk::MESSAGE_ERROR
	) ;
    }

    void
    YoukiController::on_play_eos(
	  bool no_markov
    )
    {
	bool repeat   = Glib::RefPtr<Gtk::ToggleAction>::cast_static(
			    m_UIActions_Main->get_action("PlaybackControlActionRepeat"))->get_active() ;
	bool history  = Glib::RefPtr<Gtk::ToggleAction>::cast_static(
			    m_UIActions_Main->get_action("PlaybackControlActionUseHistory"))->get_active() ;
	bool stop_now = Glib::RefPtr<Gtk::ToggleAction>::cast_static(
			    m_UIActions_Tracklist->get_action("ContextStopAfterCurrent"))->get_active() ;

	bool end = false ;
	time_t t = time(NULL) ;
        m_main_position->stop() ;
        emit_track_out() ;

	m_track_previous = m_track_current ;
	m_track_current.reset() ;

	if( stop_now )
	{
	    end = true ;
	    goto x1 ;
	}

	if( history && m_play_history.has_ffwd())
	{
	    Track_sp p = m_library->getTrackById( m_play_history.go_ffwd() ) ;
	    play_track( p, false ) ;
	    goto x1 ;
	}	
	else
	if(!m_play_queue.empty()) /* tracks in the play queue? */
	{
	    /* ... so get next track from the play queue! */
	    play_next_queue_item() ;
	    goto x1 ;
	}
	else
	{
	    bool active = Glib::RefPtr<Gtk::ToggleAction>::cast_static( m_UIActions_Main->get_action("PlaybackControlActionContinueCurrentAlbum"))->get_active() ;

	    if( active ) 
	    {
		const MPX::Track& track = *m_track_previous ;
	
		guint album_id = boost::get<guint>(track[ATTRIBUTE_MPX_ALBUM_ID].get()) ;
		const char* album_top_f = "SELECT id FROM track WHERE album_j = '%u' ORDER BY track ASC" ;

		SQL::RowV v ;
		m_library->getSQL( v, mprintf( album_top_f, album_id )) ; 

		if( !v.empty() )
		{
		    bool next = false ;

		    for( auto& r : v ) 
		    {
			guint id = boost::get<guint>(r["id"]) ;

			if( next ) 
			{
			    play_track( m_library->getTrackById( id )) ;		    
			    goto x1 ;
			}

			if( id == boost::get<guint>(track[ATTRIBUTE_MPX_TRACK_ID].get()))
			{
			    next = true ;
			}
		    }
		}
	    }
	    else
	    {
		private_->FilterModelTracks->scan_for_currently_playing() ;
		OptUInt pos = private_->FilterModelTracks->get_active_track() ;

		/* The currently playing track is in the current filter projection... */
		if( pos )
		{
		    /*...so advance to the next FIXME: Flow plugins: HERE */
		    guint pos_next = pos.get() + 1 ;

		    if( pos_next < private_->FilterModelTracks->size() )
		    {
			play_track( private_->FilterModelTracks->row( pos_next )->TrackSp ) ;
			goto x1 ;
		    }
		}
		else
		{
		    /*/ Not in the current projection? OK, so start with the top track, if the projection's size is > 0 */
		    if( private_->FilterModelTracks->size() )
		    {
			play_track( private_->FilterModelTracks->row(0)->TrackSp ) ;
			goto x1 ;
		    }
		}
	    }
	}

	end = true ;

	x1:
	if( m_track_previous )
	{
            m_library->trackPlayed( m_track_previous, t ) ;

	    if( m_track_current && !no_markov )
	    {
		guint id_a = boost::get<guint>((*m_track_previous)[ATTRIBUTE_MPX_TRACK_ID].get()) ;
		guint id_b = boost::get<guint>((*m_track_current)[ATTRIBUTE_MPX_TRACK_ID].get()) ;

		m_library->markovUpdate(id_a,id_b) ;
	    }
	}

	if( end )
	{
	    if( repeat && !m_play_history.empty() && !stop_now )
	    {
		if( history )
		{
		    m_play_history.rewind() ;
		    Track_sp p = m_library->getTrackById( m_play_history.current() ) ;
		    play_track( p, false ) ;
		}
		else
		if( private_->FilterModelTracks->size() )
		{	
		    play_track( private_->FilterModelTracks->row(0)->TrackSp ) ;
		}
	    }
	    else
	    {
		m_play->request_status( PLAYSTATUS_STOPPED ) ;
	    }
	}
    }

    void
    YoukiController::register_played_track()
    {
    }

    void
    YoukiController::tracklist_regen_mapping()
    {
        if( m_track_current && 
	    Glib::RefPtr<Gtk::ToggleAction>::cast_static( m_UIActions_Main->get_action("ViewActionFollowPlayingTrack"))->get_active())
        {
            const MPX::Track& track = *m_track_current ;
            guint id_track = boost::get<guint>(track[ATTRIBUTE_MPX_TRACK_ID].get()) ;
	    private_->FilterModelTracks->regen_mapping( id_track ) ;
        }
	else
	{
	    private_->FilterModelTracks->regen_mapping() ;
	}
    }

    void
    YoukiController::tracklist_regen_mapping_iterative()
    {
        if( m_track_current && 
	    Glib::RefPtr<Gtk::ToggleAction>::cast_static( m_UIActions_Main->get_action("ViewActionFollowPlayingTrack"))->get_active())
        {
            const MPX::Track& track = *m_track_current ;
            guint id_track = boost::get<guint>(track[ATTRIBUTE_MPX_TRACK_ID].get()) ;
	    private_->FilterModelTracks->regen_mapping_iterative( id_track ) ;
        }
	else
	{
	    private_->FilterModelTracks->regen_mapping_iterative() ;
	}
    }

    void
    YoukiController::on_play_playstatus_changed(
    )
    {
        PlayStatus status = PlayStatus( m_play->property_status().get_value() ) ;

        switch( status )
        {
            case PLAYSTATUS_PLAYING:
		m_main_position->unpause() ;
		Glib::RefPtr<Gtk::ToggleAction>::cast_static( m_UIActions_Tracklist->get_action("ContextStopAfterCurrent"))->set_sensitive(true) ;
                break ;

            case PLAYSTATUS_STOPPED:
    
		Glib::RefPtr<Gtk::ToggleAction>::cast_static( m_UIActions_Tracklist->get_action("ContextStopAfterCurrent"))->set_sensitive(false) ;

		m_play_history.clear() ;

		clear_play_queue() ;

		m_rb1->set_active();
		m_rb2->set_sensitive(false) ;

                m_track_current.reset() ;
                m_seek_position.reset() ; 

                m_main_info->clear() ;

		m_main_info->set_color() ;
		m_main_position->set_color() ;

		m_main_position->unpause() ;
                m_main_position->set_position( 0, 0 ) ;

                private_->FilterModelTracks->clear_active_track() ;

		m_main_window->set_title("Youki") ;
                m_main_window->queue_draw () ;    

                break ;

            case PLAYSTATUS_WAITING:

                m_main_position->set_position( 0, 0 ) ;
		m_main_position->unpause() ;
                m_seek_position.reset() ; 
                m_main_info->clear() ;
                m_main_window->queue_draw () ;    

                break ;

            case PLAYSTATUS_PAUSED:

		m_main_position->pause() ;

		if( m_main_window && mcs->key_get<bool>("mpx","minimize-on-pause"))
		{
		    m_main_window->iconify() ;
		}			

                break ;

            default: break ;
        }
    }

    void
    YoukiController::on_play_stream_switched()
    {
	if(!m_track_current) 
	{
	    infobar_set_message(
		  _("<b>Playback Backend</b>: No new Track after stream switch (possibly a bad file)")
		, Gtk::MESSAGE_WARNING	
	    ) ;
	    return ;
	}

        const MPX::Track& track = *m_track_current ;

        m_main_position->start() ;
        emit_track_new() ;

	guint id_track = boost::get<guint>(track[ATTRIBUTE_MPX_TRACK_ID].get()) ;

	if(!m_play_queue.empty())
	{
	    guint id_queue = m_play_queue.front() ;

	    if(id_track == id_queue)
	    {
		m_play_queue.erase(m_play_queue.begin()) ;
		m_ListViewTracks->id_set_sort_order(id_queue) ;

		if(!m_play_queue.empty())
		{
		    apply_queue();

		    if(m_rb2->get_active())
		    {
			private_->FilterModelTracks->regen_mapping_sort_order() ;
		    }
		}
		else
		{
		    m_rb1->set_active() ;
		    m_rb2->set_sensitive(false) ;
		}
	    

		auto active = Glib::RefPtr<Gtk::ToggleAction>::cast_static(
			m_UIActions_Main->get_action("PlaybackControlActionMarkov")
		)->get_active() ;

		if(active)
		{
		    check_markov_queue_append() ;

		    if(m_play_queue.empty())
		    {
			infobar_set_message(
			      _("<b>PlaySense</b>: No matching tracks found")
			    , Gtk::MESSAGE_INFO
			) ;

			Glib::RefPtr<Gtk::ToggleAction>::cast_static(
				m_UIActions_Main->get_action("PlaybackControlActionMarkov")
			)->set_active(false) ;

			m_rb1->set_active() ;
			m_rb2->set_sensitive(false) ;
			tracklist_regen_mapping() ;
		    }
		    else
		    if(m_rb2->get_active())
		    {
			private_->FilterModelTracks->regen_mapping_sort_order() ;
		    }
		}
	    }
	}

        private_->FilterModelTracks->set_active_id( id_track ) ;

        if( Glib::RefPtr<Gtk::ToggleAction>::cast_static( m_UIActions_Main->get_action("ViewActionFollowPlayingTrack"))->get_active())
        {
            m_ListViewTracks->scroll_to_id( id_track ) ;
        }

        std::vector<std::string> info ;
        info.push_back( boost::get<std::string>(track[ATTRIBUTE_ARTIST].get()) ) ;
        info.push_back( boost::get<std::string>(track[ATTRIBUTE_ALBUM].get()) ) ;
        info.push_back( boost::get<std::string>(track[ATTRIBUTE_TITLE].get()) ) ;
	m_main_info->set_info( info ) ;

	boost::shared_ptr<IYoukiThemeEngine> theme = services->get<IYoukiThemeEngine>("mpx-service-theme") ;

        if( track.has( ATTRIBUTE_MB_ALBUM_ID ) )
        {
                Glib::RefPtr<Gdk::Pixbuf> cover ;

                if(  m_covers->fetch(
                      boost::get<std::string>(track[ATTRIBUTE_MB_ALBUM_ID].get())
                    , cover
                ))
                {
		    Gdk::RGBA c = Util::pick_color_for_pixbuf(cover) ;

                    m_main_info->set_cover( cover ) ;
		    m_main_info->set_color(c) ;

		    m_main_position->set_color(c) ;

		    goto skip1 ;
                }
        }

	m_main_info->set_cover( Glib::RefPtr<Gdk::Pixbuf>(0) ) ;
	m_main_info->set_color() ;

	m_main_position->set_color() ;
	
	skip1:

	m_main_window->set_title((boost::format("Youki :: %s - %s") % info[0] % info[2]).str()) ;

	m_InfoLabel->set_text("") ;
	m_InfoBar->hide() ; 
    }

    void
    YoukiController::handle_info_bar_response(
	  int G_GNUC_UNUSED
    )
    {
	m_InfoLabel->set_text("") ;
	m_InfoBar->hide() ;
    }

    void
    YoukiController::check_markov_queue_append(
    )
    {
	check_markov_queue_append_real(m_track_current) ;
    }

    void
    YoukiController::check_markov_queue_append_real(
	  guint id_track
    )
    {
	auto active = Glib::RefPtr<Gtk::ToggleAction>::cast_static(
		m_UIActions_Main->get_action("PlaybackControlActionMarkov")
	)->get_active() ;

	if( active )
	{
	    guint id_next = m_library->markovGetRandomProbableTrack( id_track ) ;

	    if(id_next != 0 && (id_next != id_track))
	    {
		queue_next_track(id_next) ;
	    }
	    else
	    {
		SQL::RowV v ;
		m_library->getSQL(v, (boost::format("SELECT id FROM track ORDER BY random() LIMIT 1")).str()) ; 
		id_next = boost::get<guint>(v[0]["id"]) ;
		queue_next_track(id_next) ;
#if 0
		Glib::RefPtr<Gtk::ToggleAction>::cast_static(
			m_UIActions_Main->get_action("PlaybackControlActionMarkov")
		)->set_active(false) ;

		infobar_set_message(
		      _("<b>PlaySense</b>: No matching tracks found.")
		    ,  Gtk::MESSAGE_INFO
		) ;
#endif
	    }
	}
    }

    void
    YoukiController::check_markov_queue_append_real(
	  MPX::Track_sp track
    )
    {
	if(track)
	{
	    auto active = Glib::RefPtr<Gtk::ToggleAction>::cast_static(
		    m_UIActions_Main->get_action("PlaybackControlActionMarkov")
	    )->get_active() ;

	    guint id_track = boost::get<guint>((*track)[ATTRIBUTE_MPX_TRACK_ID].get()) ;

	    if( m_play_queue.empty() && active )
	    {
		guint id_next = m_library->markovGetRandomProbableTrack( id_track ) ;
    
		if(id_next != 0 && (id_next != id_track))
		{
		    queue_next_track(id_next) ;
		}
		else
		{
		    Glib::RefPtr<Gtk::ToggleAction>::cast_static(
			    m_UIActions_Main->get_action("PlaybackControlActionMarkov")
		    )->set_active(false) ;

		    infobar_set_message(
			  _("<b>PlaySense</b>: No matching tracks found")
			, Gtk::MESSAGE_INFO
		    ) ;
		}
	    }
	}
    }

    void
    YoukiController::on_play_metadata(
            GstMetadataField    field
    )
    {
	const GstMetadata& m = m_play->get_metadata() ;

        if( field == FIELD_AUDIO_BITRATE )
        {
		guint r = m.m_audio_bitrate.get() ;
		r /= 1000 ;
		m_main_info->set_bitrate( r ) ; 
        }
        else if( field == FIELD_AUDIO_CODEC )
        {
		m_main_info->set_codec( m.m_audio_codec.get() ) ;
        }
    }

    void
    YoukiController::handle_tracklist_track_activated(
          MPX::Track_sp t 
        , bool          play
	, bool		next
    ) 
    {
        const MPX::Track& track = *t ;

        emit_track_cancelled() ;

	guint id = boost::get<guint>(track[ATTRIBUTE_MPX_TRACK_ID].get()) ;

	std::vector<guint>::iterator i = std::find(m_play_queue.begin(), m_play_queue.end(), id) ; 

	if( play && i != m_play_queue.end())
	{
	    m_play_queue.erase(i) ;
	    m_ListViewTracks->id_set_sort_order(id) ;		

	    if(!m_play_queue.empty())
	    {
		apply_queue() ;
	    }
	    else
	    {
		m_rb1->set_active() ;
		m_rb2->set_sensitive(false) ;
	    }

	    if(m_rb2->get_active())
	    {
		private_->FilterModelTracks->regen_mapping_sort_order() ;
	    }

	    play_track(t) ;
	}
	else
	if( play && i == m_play_queue.end())
	{
	    play_track(t) ;
	    check_markov_queue_append_real(t) ;
	}
	else
	if( !play && i == m_play_queue.end())
	{
	    if( next )
	    {
		m_play_queue.insert(m_play_queue.begin(), id) ; 
		apply_queue() ;
		m_rb2->set_sensitive() ;
	    }
	    else
	    {
		// FIXME: To avoid infinite recursion we have to duplicate this code here
		m_play_queue.push_back(id) ;
		m_ListViewTracks->id_set_sort_order(id, m_play_queue.size()) ;
		check_markov_queue_append_real(id) ;
		m_rb2->set_sensitive() ;
	    }
	}
    }

    void
    YoukiController::handle_tracklist_find_propagate(
          const std::string&    text
    )
    {
        m_Entry->set_text( (boost::format("title %% \"%s\"") % text).str() ) ;
    }

    void
    YoukiController::on_list_view_aa_selection_changed(
    ) 
    {
	m_conn1.block() ;
	m_conn2.block() ;
	m_conn4.block() ;
	m_conn3.block() ;

        m_ListViewAlbums->clear_selection() ;

        private_->FilterModelTracks->clear_synthetic_constraints_quiet() ;
        private_->FilterModelAlbums->clear_all_constraints_quiet() ;

        OptUInt id_artist = m_ListViewArtist->get_selected() ;

	if( id_artist )
	{
	    AQE::Constraint_t c ;
	    c.TargetAttr = ATTRIBUTE_MPX_ALBUM_ARTIST_ID ;
	    c.TargetValue = id_artist.get() ;
	    c.MatchType = AQE::MT_EQUAL ;

	    private_->FilterModelTracks->add_synthetic_constraint_quiet( c ) ;
	}

	if(m_rb2->get_active())
	{
	    m_rb1->set_active() ;
	}

	tracklist_regen_mapping() ;

	private_->FilterModelAlbums->set_constraints_albums( private_->FilterModelTracks->m_constraints_albums ) ;
	private_->FilterModelAlbums->set_constraints_artist( private_->FilterModelTracks->m_constraints_artist ) ;
        private_->FilterModelAlbums->regen_mapping() ;

	m_conn1.unblock() ;
	m_conn2.unblock() ;
	m_conn4.unblock() ;
	m_conn3.unblock() ;
    }

    void
    YoukiController::handle_albumlist_selection_changed(
    ) 
    {
        private_->FilterModelTracks->clear_synthetic_constraints_quiet() ;

        OptUInt id_artist = m_ListViewArtist->get_selected() ;
        OptUInt id_albums = m_ListViewAlbums->get_selected() ;

	AQE::Constraint_t ac ; 

        if( id_albums ) 
        {
	    ac.TargetAttr = ATTRIBUTE_MPX_ALBUM_ID ;
	    ac.MatchType = AQE::MT_EQUAL ;
	    ac.TargetValue = id_albums.get() ;
	    private_->FilterModelTracks->add_synthetic_constraint_quiet( ac ) ;
	}

	if( id_artist ) 
	{
	    ac.TargetAttr = ATTRIBUTE_MPX_ALBUM_ARTIST_ID ;
	    ac.MatchType = AQE::MT_EQUAL ;
	    ac.TargetValue = id_artist.get() ;
	    private_->FilterModelTracks->add_synthetic_constraint_quiet( ac ) ;
	}

	bool v = m_rb2->get_active() ; 

	if(!v) {
	    tracklist_regen_mapping() ;
	} else {
	    m_rb1->set_active() ; 
	}

	handle_follow_playing_track_toggled() ;
    }

    void
    YoukiController::handle_albumlist_select_album(
        const std::string&  mbid
    )
    {
        handle_search_entry_clear_clicked() ;
        m_Entry->set_text( (boost::format("album-mbid = \"%s\"") % mbid).str() ) ;
    }

    void
    YoukiController::handle_albumlist_select_artist(
        const std::string&  mbid
    )
    {
        handle_search_entry_clear_clicked() ;
        m_Entry->set_text( (boost::format("album-artist-mbid = \"%s\"") % mbid).str() ) ;
    }

    void
    YoukiController::handle_albumlist_similar_artists(
        const std::string&  mbid
    )
    {
        handle_search_entry_clear_clicked() ;
        m_Entry->set_text( (boost::format("mbid-artists-similar-to = \"%s\"") % mbid).str() ) ;
    }

    void
    YoukiController::handle_albumlist_refetch_cover(
	guint id
    )
    {
	m_library->recacheAlbumCover( id ) ;
    }

 
    void
    YoukiController::handle_tracklist_vadj_changed(
    ) 
    {
	Glib::RefPtr<Gtk::ToggleAction>::cast_static( m_UIActions_Main->get_action("PlaybackControlActionStartAlbumAtFavorite"))->set_active(false) ;
    }

    void
    YoukiController::handle_search_entry_activated(
    )
    {
	if( private_->FilterModelTracks->size() )
        {
	    play_track( private_->FilterModelTracks->row( m_ListViewTracks->get_upper_row())->TrackSp ) ;
	}
    }

    bool
    YoukiController::handle_search_entry_key_press_event(
          GdkEventKey* event
    ) 
    {
        switch( event->keyval )
        {
	    case GDK_KEY_Down:
            {
                m_ListViewTracks->grab_focus() ;
                return true ;
            }

            case GDK_KEY_BackSpace:
            {
                if( m_Entry->get_text().empty() )
                {
                    handle_search_entry_clear_clicked() ;
                    return true ;
                }

                break ;
            }

            case GDK_KEY_c:
            case GDK_KEY_C:
            {
                if( event->state & GDK_CONTROL_MASK )
                {
                    handle_search_entry_clear_clicked() ;
                    return true ;
                }

                break ;
            }

            case GDK_KEY_s:
            case GDK_KEY_S:
            {
                if( event->state & GDK_CONTROL_MASK )
                {
		    handle_nearest_clicked() ;
                    return true ;
                }

                break ;
            }

            case GDK_KEY_Escape:
            {
		handle_search_entry_clear_clicked() ;
		m_ListViewTracks->grab_focus() ;
		return true ;
            }

            case GDK_KEY_w:
            case GDK_KEY_W:
            {
                if( event->state & GDK_CONTROL_MASK )  
                {
                    Glib::ustring text = m_Entry->get_text() ;

                    if( text.rfind(' ') != Glib::ustring::npos ) 
                    {
                        text = text.substr( 0, text.rfind(' ')) ;
                    }

                    else

                    if( !text.empty() )    
                    {
                        text.clear() ;
                    }

                    m_Entry->set_text( text ) ;
                    m_Entry->set_position( -1 ) ;

                    return true ;
                }

                break ;
            }

            default: break ;
        }

        return false ;
    }

    void
    YoukiController::handle_search_entry_clear_clicked(
	  Gtk::EntryIconPosition p
	, const GdkEventButton* G_GNUC_UNUSED
    )
    {
	if( p != Gtk::ENTRY_ICON_SECONDARY )
	    return ;

        m_conn1.block() ;
        m_conn2.block() ;
        m_conn4.block() ;
	m_conn3.block() ; 

        private_->FilterModelAlbums->clear_constraints_artist() ;
        private_->FilterModelAlbums->clear_constraints_albums() ;
        private_->FilterModelAlbums->clear_all_constraints_quiet() ;
        private_->FilterModelArtist->clear_constraints_artist() ;
        private_->FilterModelTracks->clear_synthetic_constraints_quiet() ;

        m_ListViewArtist->clear_selection() ;
        m_ListViewAlbums->clear_selection() ;

	bool was_empty = m_Entry->get_text().empty() ;

        m_Entry->set_text("") ;
	m_rb1->set_active() ;

	std::string text_noaque ;

	boost::optional<guint> id ;

        if( m_track_current && 
	    Glib::RefPtr<Gtk::ToggleAction>::cast_static( m_UIActions_Main->get_action("ViewActionFollowPlayingTrack"))->get_active())
        {
            id = boost::get<guint>((*m_track_current)[ATTRIBUTE_MPX_TRACK_ID].get()) ;
        }

	if( was_empty )
	{
	    private_->FilterModelTracks->regen_mapping(id) ;
	}
	else
	{
	    private_->FilterModelTracks->process_filter("",id,text_noaque) ;
	}

	private_->FilterModelArtist->regen_mapping() ;
	private_->FilterModelAlbums->regen_mapping() ;

        m_conn1.unblock() ;
        m_conn2.unblock() ;
        m_conn4.unblock() ;
	m_conn3.unblock() ;

	m_Entry->grab_focus() ;
    }

    std::string
    YoukiController::trie_find_ldmatch(
	const std::string& text_std
    )
    {
	std::string r_ ;

	StrV m ;
	split( m, text_std, is_any_of(" ")) ;
   
	if( m.size() >= 2 ) 
	{
	    std::string&& s1 = Glib::ustring(m[0]).lowercase() ;
	    std::string&& s2 = Glib::ustring(m[1]).lowercase() ;

	    std::string match1 = m_find_nearest_artist.find_nearest_match_leven(3,s1) ;
	    std::string match2 ;

	    if(!match1.empty())
	    {
		StrLDFN_map_t::iterator i = m_ldmap.find(Glib::ustring(match1).lowercase()) ;

		if(i != m_ldmap.end())
		{
		    LDFN_p p = i->second ;
		    match2 = p->find_nearest_match_leven(3, s2) ;

		    if(!match2.empty())	
		    {
			std::string joined = match1 ;
			joined += " " ;
			joined += match2 ;

			joined = Glib::ustring(joined).lowercase() ;

			for( auto& s : m_nearest__artists )
			{
			    std::string s_lc = Glib::ustring(s).lowercase() ;

			    if( s_lc.substr(0,joined.length()) == joined ) 
			    {
				r_ = s ;
				break ;
			    }
			}
		    }
		}
	    }
	}

	return r_ ;
    }

    void
    YoukiController::handle_search_entry_changed(
    )
    {
	using namespace MPX::SearchController ;

        m_conn1.block() ;
        m_conn2.block() ;
	m_conn3.block() ;

	std::string text_noaque ;

	boost::optional<guint> id ;

	bool queue_view = m_rb2->get_active(); 

	if( queue_view )
	{
	    m_rb1->set_active() ; 
	}

        if( m_track_current
		&& 
	    Glib::RefPtr<Gtk::ToggleAction>::cast_static( m_UIActions_Main->get_action("ViewActionFollowPlayingTrack"))->get_active()
		&&
	    !queue_view )
        {
            id = boost::get<guint>((*m_track_current)[ATTRIBUTE_MPX_TRACK_ID].get()) ;
        }

	private_->FilterModelTracks->clear_synthetic_constraints_quiet() ;

	FilterMode mode = private_->FilterModelTracks->process_filter( m_Entry->get_text(), id, text_noaque ) ;

	if( mode != FilterMode::NONE ) {

	    bool artist_view_has_selection = m_ListViewArtist->get_selected_index() ;

            m_ListViewAlbums->clear_selection() ;
	    m_ListViewArtist->clear_selection() ;
 
	    private_->FilterModelArtist->set_constraints_artist( private_->FilterModelTracks->m_constraints_artist ) ;
	    private_->FilterModelAlbums->set_constraints_albums( private_->FilterModelTracks->m_constraints_albums ) ;

	    if( mode == FilterMode::ITERATIVE && !queue_view ) {

		private_->FilterModelArtist->regen_mapping_iterative() ;

		if( artist_view_has_selection )
		    private_->FilterModelAlbums->regen_mapping() ;
		else
		    private_->FilterModelAlbums->regen_mapping_iterative() ;
	    }
	    else {
		private_->FilterModelArtist->regen_mapping() ;
		private_->FilterModelAlbums->regen_mapping() ;
	    }
	}

	m_conn1.unblock() ;
	m_conn2.unblock() ;
	m_conn3.unblock() ;

	if(private_->FilterModelTracks->m_mapping->empty()) {

	    m_Entry->override_color(Util::make_rgba(1.,0.,0.,1.)) ;

	    m_nearest = m_find_nearest_artist_full_lc.find_nearest_match_leven(3,Glib::ustring(text_noaque).lowercase()) ;

	    if(!m_nearest.empty())
	    {
		for( auto& s : m_nearest__artists )
		{
		    if( (Glib::ustring(s).lowercase() == Glib::ustring(m_nearest).lowercase()))
		    {
			m_nearest = s ;
			break ;
		    }
		}
	    }

	    if(m_nearest.empty()) {
		m_nearest = trie_find_ldmatch(text_noaque) ;
	    }

	    if(m_nearest.empty()) {
		StrV m ;
		split( m, text_noaque, is_any_of(" ")) ;
		std::string joined = boost::algorithm::join( m, "" ) ;
		m_nearest = m_find_nearest_artist_full.find_nearest_match_leven(3,text_noaque) ;
	    }

	    if(m_nearest.empty()) {
		m_nearest = m_find_nearest_artist.find_nearest_match_leven(3,text_noaque) ;
	    }

	    if(m_nearest.empty()) {
		m_Label_Nearest->set_text("") ;
	    } else {
		m_Label_Nearest->set_markup((boost::format("%s <b>%s</b>?") % _("Did you mean") % m_nearest).str()) ;
	    }
	}
	else {
	    m_Label_Nearest->set_text("") ;
	    m_Entry->unset_color() ;
	}

   
	if(m_Entry->get_text().empty())
	{
	    m_Label_Nearest->set_text("") ;
	    m_Entry->unset_color() ;
	}
    }

    void
    YoukiController::on_position_seek(
          guint        position
    )
    {
        m_seek_position = position ;
        m_play->seek( position ) ;
    }

    void
    YoukiController::on_volume_set_volume(
          int           volume
    )
    {
        mcs->key_set<int>(
              "mpx"
            ,"volume"
            , volume
        ) ;

        m_play->property_volume().set_value( volume ) ;
    }

    void
    YoukiController::handle_shuffle_toggled()
    {
	Glib::RefPtr<Gtk::ToggleAction>::cast_static( m_UIActions_Main->get_action("PlaybackControlActionContinueCurrentAlbum"))->set_active(false) ;

	if(!m_play_queue.empty() && m_rb2->get_active())
	{
	    std::random_shuffle( m_play_queue.begin(), m_play_queue.end(), std::pointer_to_unary_function<int,int>(Rand)) ;

	    apply_queue() ;
	    private_->FilterModelTracks->regen_mapping_sort_order() ;

	    return ;
	}

	/* Get the vector of all projection IDs */
	MPX::View::Tracks::IdVector_sp v = private_->FilterModelTracks->get_id_vector() ; 
	MPX::View::Tracks::IdVector_t& v_ = *v ;

	/* Shuffle the vector */ 
	std::random_shuffle( v_.begin(), v_.end(), std::pointer_to_unary_function<int,int>(Rand)) ;

	/* If a track is playing, omit it from the shuffle map */
	if( m_track_current )
	{ 
	    guint id = boost::get<guint>((*m_track_current)[ATTRIBUTE_MPX_TRACK_ID].get()) ;
	    std::remove(v_.begin(),v_.end(),id) ;
	}

	append_to_queue(v_) ;
	m_rb2->set_sensitive() ;
    }

    void
    YoukiController::play_next_queue_item()
    {
	Track_sp p = m_library->getTrackById(m_play_queue.front()) ; 
	play_track( p ) ;
    }

    void
    YoukiController::on_info_area_clicked( int i )
    {
	TapArea area = TapArea(i) ;

	if( area == TAP_CENTER )
	{
	    if( m_play->property_status().get_value() == PLAYSTATUS_STOPPED )
	    {
		if(!m_play_queue.empty()) /* tracks in the play queue? */
		{
		    /* ... so get next track from the play queue! */
		    play_next_queue_item() ;
		}
		else
		if( private_->FilterModelTracks->size() )
		{
		    OptUInt idx = m_ListViewTracks->get_selected_index() ; 
		    play_track( private_->FilterModelTracks->row( idx ? idx.get() : 0)->TrackSp ) ;
		}
	    }
	    else
	    {
		API_pause_toggle() ;
	    }
	}
	else
	if( area == TAP_LEFT )
	{
	    API_prev() ;
	}
	else
	if( area == TAP_RIGHT )
	{
	    API_next() ;
	}
    }

    void
    YoukiController::handle_albumlist_start_playback ()
    {
	bool active = Glib::RefPtr<Gtk::ToggleAction>::cast_static( m_UIActions_Main->get_action("PlaybackControlActionStartAlbumAtFavorite"))->get_active() ;

        if( private_->FilterModelTracks->size() )
        {
	    const char * album_top_f = "SELECT id FROM track WHERE album_j = '%u' AND pcount IS NOT NULL ORDER BY pcount DESC" ;
	    OptUInt id = m_ListViewAlbums->get_selected() ;

	    if( id && active ) 
	    {
		SQL::RowV v ;
		m_library->getSQL( v, mprintf( album_top_f, id.get() )) ; 

		if( !v.empty() )
		{
			private_->FilterModelTracks->set_active_id( boost::get<guint>(v[0]["id"])) ;

			OptUInt model_track_pos = private_->FilterModelTracks->get_active_track() ;

			if( model_track_pos )
			{
			    play_track( private_->FilterModelTracks->row(model_track_pos.get())->TrackSp ) ;
			    return ;
			}
			/* ...unwind */
		}
		/* unwind... */
	    }

	    /* ...aaaand... fallback */
	   
	    play_track( private_->FilterModelTracks->row(0)->TrackSp ) ;
        }
    }

    void
    YoukiController::on_show_subsystem_window(
	int n
    ) 
    {
        switch( n ) 
        {
            case 0:
                services->get<Preferences>("mpx-service-preferences")->present () ;
		break; 

            case 1:
                m_mlibman_dbus_proxy->ShowWindow () ;
		break; 

            case 2:
                services->get<PluginManagerGUI>("mpx-service-plugins-gui")->present () ;
		break; 

            default: break ;
        }
    }

    bool
    YoukiController::on_main_window_key_press_event_after(
          GdkEventKey* event
    )
    {
        switch( event->keyval )
        {
            case GDK_KEY_F6:
                m_Entry->grab_focus() ;
                return true ;

            default: break ;
        }
        return false ;
    }

    bool
    YoukiController::on_status_icon_clicked(
    )
    {
	API_pause_toggle() ;
/*
        if( m_main_window->is_visible() )
        {
            m_main_window->get_position( m_main_window_x, m_main_window_y ) ;
            m_main_window->hide () ;
        }
        else
        {
            m_main_window->move( m_main_window_x, m_main_window_y ) ;
            m_main_window->show () ;
            m_main_window->raise () ;
        }
 */

	return true ;
    }

    void
    YoukiController::on_status_icon_scroll_up(
    )
    {
        int volume = m_play->property_volume().get_value() ; 

        volume = std::max( 0, (((volume+4)/5)*5) - 5 ) ;

        mcs->key_set<int>(
              "mpx"
            ,"volume"
            , volume
        ) ;

        m_play->property_volume().set_value( volume ) ;

        m_main_volume->set_volume(
            volume 
        ) ;
    }

    void
    YoukiController::on_status_icon_scroll_down(
    )
    {
        int volume = m_play->property_volume().get_value() ; 

        volume = std::min( 100, ((volume/5)*5) + 5 ) ;

        mcs->key_set<int>(
              "mpx"
            ,"volume"
            , volume
        ) ;

        m_play->property_volume().set_value( volume ) ;

        m_main_volume->set_volume(
            volume 
        ) ;
    }

    /* DBUS */

void
    YoukiController::assign_metadata_to_DBus_property()
    {
/*
        if( !m_track_current )
        {
	    Metadata = std::map<std::string, DBus::Variant>() ;
	    return ;
        }

        const MPX::Track& track = *(m_track_current.get()) ;

        //DBus::MessageIter i_out = Metadata.get_value_writer() ;
    DBus::MessageIter i_out;
	DBus::MessageIter i_arr = i_out.new_array("{sv}") ;

        for( guint n = 0 ; n < G_N_ELEMENTS(mpris_attribute_id_str) ; ++n )
        {
            if( track.has(mpris_attribute_id_str[n].Attr))
            {
		    DBus::MessageIter i_dict = i_arr.new_dict_entry() ;

		    i_dict << std::string(mpris_attribute_id_str[n].AttrName) ;

		    DBus::MessageIter var = i_dict.new_variant("s");
		    var << std::string(boost::get<std::string>( track[mpris_attribute_id_str[n].Attr].get())) ; 
		    i_dict << var ;
            }
        }

        for( guint n = 0 ; n < G_N_ELEMENTS(mpris_attribute_id_int) ; ++n )
        {
            if( track.has(mpris_attribute_id_int[n].Attr))
            {
		    DBus::MessageIter i_dict = i_arr.new_dict_entry() ;

		    i_dict << std::string(mpris_attribute_id_str[n].AttrName) ;

		    DBus::MessageIter var = i_dict.new_variant("x") ;
		    var << gint64(boost::get<guint>( track[mpris_attribute_id_int[n].Attr].get())) ;
		    i_dict << var ;
            }
        }

	if( track.has(ATTRIBUTE_MB_ALBUM_ID) )
	{
	    std::string artUrl = m_covers->get_thumb_path( boost::get<std::string>(track[ATTRIBUTE_MB_ALBUM_ID].get())) ;
	
	    if(Glib::file_test( artUrl, Glib::FILE_TEST_EXISTS ))
	    {
		DBus::MessageIter i_dict = i_arr.new_dict_entry() ;

		i_dict << std::string("mpris:artUrl") ;

		DBus::MessageIter var = i_dict.new_variant("s") ;
		var << std::string(Glib::filename_to_uri( artUrl )) ;
		i_dict << var ;
	    }
	}

	i_out.close_container( i_arr ) ;
 */
    }

    void
    YoukiController::queue_next_track(
          guint id
    )
    {
        m_play_queue.push_back(id) ;
	m_ListViewTracks->id_set_sort_order(id, m_play_queue.size()) ;
	m_rb2->set_sensitive() ;
    }

    std::vector<guint>
    YoukiController::get_current_play_queue(
    )
    {
        return m_play_queue ;
    }

    int
    YoukiController::get_status(
    )
    {
        return PlayStatus( m_play->property_status().get_value() ) ;
    }

    MPX::Track&
    YoukiController::get_metadata(
    )
    {
        if( m_track_current )
            return *m_track_current ;
        else
            throw std::runtime_error("No current track!") ; /* FIXME: Well */
    }

    MPX::Track&
    YoukiController::get_metadata_previous(
    )
    {
        if( m_track_previous )
            return *m_track_previous ;
        else
            throw std::runtime_error("No previous track!") ;
    }

    void
    YoukiController::API_pause_toggle(
    )
    {
        if( m_track_current )
        {
                PlayStatus s = PlayStatus(m_play->property_status().get_value()) ;

                if( s == PLAYSTATUS_PAUSED )
                    m_play->request_status( PLAYSTATUS_PLAYING ) ;
                else
                if( s == PLAYSTATUS_PLAYING )
                    m_play->request_status( PLAYSTATUS_PAUSED ) ;
        }
    }

    void
    YoukiController::API_next(
    )
    {
        if( m_track_current )
        {
            on_play_eos(true) ;
        }
    }

    void
    YoukiController::API_prev(
    )
    {
	bool history = Glib::RefPtr<Gtk::ToggleAction>::cast_static( m_UIActions_Main->get_action("PlaybackControlActionUseHistory"))->get_active() ;
	bool rwnd = (m_play->property_position().get_value() > 5)?0:1 ;

	//FIXME: This doesn't work at all with flow plugins, of course we also don't have any yet
	if(rwnd)
	{
	    if( history && m_play_history.has_prev())
	    {
		guint id = m_play_history.go_back() ;
		m_play_history.print() ;
		play_track( m_library->getTrackById( id )) ;		    
	    }
	    else
	    if( private_->FilterModelTracks->size() )
	    {
		OptUInt idx = private_->FilterModelTracks->get_active_track() ; 

		if( idx && (idx.get() > 0)) 
		{
		    gint d = std::max<int>( 0, idx.get() - 1 ) ;
		    play_track( private_->FilterModelTracks->row( d )->TrackSp, (idx.get()-1) != d ) ;

		    if(!m_play_history.has_prev()) {
			m_play_history.prepend(private_->FilterModelTracks->row(d)->ID) ;
		    }
		}
	    }
	}
	else
	{
	    if( private_->FilterModelTracks->size() )
	    {
		OptUInt idx = private_->FilterModelTracks->get_active_track() ; 

		if( idx ) 
		{
		    play_track( private_->FilterModelTracks->row(idx.get())->TrackSp, false ) ;
		}
	    }
	}
    }

    void
    YoukiController::API_stop(
    )
    {
        m_play->request_status( PLAYSTATUS_STOPPED ) ; 
    }

    void
    YoukiController::API_play_track(
        guint  id
    )
    {
        play_track( m_library->getTrackById( id )) ; 
    }
}
