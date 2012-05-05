#include "config.h"

#include <tr1/random>

#include <libindicate/indicator-messages.h>
#include <libindicate/indicator.h>
#include <libindicate/interests.h>
#include <libindicate/listener.h>
#include <libindicate/server.h>

#include <glibmm/i18n.h>
#include <gdk/gdkkeysyms.h>

#include <boost/format.hpp>
#include <boost/ref.hpp>
#include <boost/random.hpp>

#include "mpx/mpx-main.hh"
#include "mpx/mpx-covers.hh"
#include "mpx/mpx-types.hh"
#include "mpx/mpx-stock.hh"

#include "mpx/util-string.hh"

#include "mpx/com/view-album-artist.hh"
#include "mpx/com/view-album.hh"
#include "mpx/com/view-tracks.hh"

#include "mpx/widgets/cairo-extensions.hh"
#include "mpx/widgets/percentual-distribution-hbox.hh"

#include "mpx/i-youki-theme-engine.hh"

#include "library.hh"
#include "plugin-manager-gui.hh"
#include "play.hh"
#include "preferences.hh"

#include "youki-controller.hh"

using boost::get ;

namespace
{
    const char* main_menubar_ui =
    "<ui>"
    ""
    "<menubar name='MenuBarMain'>"
    ""
    "	<menu action='MenuFile'>"
    "	    <menuitem action='MenuFileActionPreferences'/>"
    "	    <menuitem action='MenuFileActionLibrary'/>"
    "	    <menuitem action='MenuFileActionPlugins'/>"
    "	    <separator/>"
    "	    <menuitem action='MenuFileActionAbout'/>"
    "	    <separator/>"
    "	    <menuitem action='MenuFileActionQuit'/>"
    "	</menu>"
    "	<menu action='MenuView'>"
    "	    <menuitem action='MenuViewActionAlbumsShowTimeDiscsTracks'/>"
    "	    <menuitem action='MenuViewActionAlbumRTViewModeBottom'/>"
    "	    <menuitem action='MenuViewActionUnderlineMatches'/>"
    "	    <menuitem action='MenuViewActionFollowPlayingTrack'/>"
    "	</menu>"
    "	<menu action='MenuPlaybackControl'>"
    "	    <menuitem action='MenuPlaybackControlActionStartAlbumAtFavorite'/>"
    "	    <menuitem action='MenuPlaybackControlActionContinueCurrentAlbum'/>"
    "	    <menuitem action='MenuPlaybackControlActionMinimizeOnPause'/>"
    "	    <menuitem action='MenuPlaybackControlActionPlayTrackOnSingleTap'/>"
    "	</menu>"
    "</menubar>"
    ""
    "</ui>"
    ;

    struct AttrInfo_t
    {
	int Attr ;
	const char* AttrName ;
    } ;
    
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
	d.set_copyright("(C) 2006-2012 Milosz Derezynki, Chong Kaixiong") ;
	d.set_license("GPL v3 or later") ;
	d.set_license_type(Gtk::LICENSE_GPL_3_0) ;

	std::vector<Glib::ustring> authors;
	authors.push_back ("Milosz Derezynski");
	authors.push_back ("Chong Kai Xiong");
	authors.push_back ("David Le Brun");
	d.set_authors( authors ) ;

	std::vector<Glib::ustring> artists;
    artists.push_back ("Milosz Derezynski");
	d.set_artists( artists ) ;

	Glib::RefPtr<Gdk::Pixbuf> p_logo = Gdk::Pixbuf::create_from_file( Glib::build_filename( DATA_DIR, "images" G_DIR_SEPARATOR_S "youki256x256.png" )) ;

	d.set_logo( p_logo ) ;

	d.set_modal( true ) ;
	d.set_transient_for( *win ) ;
	d.set_icon( p_logo ) ;

	d.run() ;
    }
  
    typedef boost::optional<guint> OptUInt ;
}

namespace MPX
{
    struct YoukiController::Private
    { 
        View::Artist::DataModelFilter_sp_t  FilterModelArtist ;
        View::Albums::DataModelFilter_sp_t  FilterModelAlbums ;
        View::Tracks::DataModelFilter_sp_t  FilterModelTracks ;

    } ;

    YoukiController::YoukiController(
    )
    : Glib::ObjectBase( "YoukiController" )
    , Service::Base("mpx-service-controller")
    , private_( new Private )
    , m_main_window( 0 )
    , m_history_ignore_save( false )
    {
	IndicateServer* is = indicate_server_ref_default() ;
	indicate_server_set_type( is, "music.youki" ) ;
	indicate_server_set_desktop_file( is, "/usr/share/applications/youki.desktop") ;
	indicate_server_show( is ) ;

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

        m_covers    = services->get<Covers>("mpx-service-covers").get() ;
        m_play      = services->get<MPX::Play>("mpx-service-play").get() ;
        m_library   = services->get<Library>("mpx-service-library").get() ;

/*
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
 */

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
            sigc::mem_fun(
                  *this
                , &YoukiController::on_play_eos
        )) ;

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
	//Glib::signal_timeout().connect( sigc::bind_return<bool>( sigc::mem_fun( *this, &YoukiController::update_entry_placeholder_text ), true ), 10 * 60000) ;

        m_Entry->signal_icon_press().connect(
                sigc::mem_fun(
                      *this
                    , &YoukiController::on_entry_clear_clicked
        )) ;

        m_Entry->signal_key_press_event().connect(
            sigc::mem_fun(
                  *this
                , &YoukiController::on_entry_key_press_event
        )) ;

        m_Entry->signal_activate().connect(
            sigc::mem_fun(
                  *this
                , &YoukiController::on_entry_activated
        )) ;

	m_Entry->signal_focus_out_event().connect(
	      sigc::mem_fun(
	            *this
                  , &YoukiController::handle_search_entry_focus_out
        )) ;

	m_conn4 = m_Entry->signal_changed().connect(
	    sigc::mem_fun(
		  *this
		, &YoukiController::on_entry_changed
	)) ;

        m_VBox = Gtk::manage( new Gtk::VBox ) ;
        m_VBox->set_spacing( 2 ) ;
	m_VBox->set_border_width( 0 ) ;

        m_HBox_Main = Gtk::manage( new PercentualDistributionHBox ) ;
        m_HBox_Main->set_spacing( 6 ) ; 

        m_HBox_Entry = Gtk::manage( new Gtk::HBox ) ;
        m_HBox_Entry->set_spacing( 4 ) ;
        m_HBox_Entry->set_border_width( 0 ) ;

	m_Label_Search = Gtk::manage( new Gtk::Label("Find Mu_sic:", true )) ;
	m_Label_Search->set_mnemonic_widget( *m_Entry ) ;

	m_Label_Error = Gtk::manage( new Gtk::Label("")) ;

	Gtk::HBox * HBox_Navi = Gtk::manage( new Gtk::HBox ) ;
	HBox_Navi->set_spacing( 1 ) ;
	HBox_Navi->set_border_width( 0 ) ;

	Gtk::Image * iprev = Gtk::manage( new Gtk::Image( Gtk::Stock::GO_BACK, Gtk::ICON_SIZE_BUTTON )) ;
        m_BTN_HISTORY_PREV = Gtk::manage( new Gtk::Button ) ;
	m_BTN_HISTORY_PREV->set_relief( Gtk::RELIEF_NONE ) ;
	m_BTN_HISTORY_PREV->set_border_width( 0 ) ; 
	m_BTN_HISTORY_PREV->add( *iprev ) ;
	m_BTN_HISTORY_PREV->signal_clicked().connect( sigc::mem_fun( *this, &YoukiController::history_go_back)) ;
	m_BTN_HISTORY_PREV->set_sensitive( false ) ;

	Gtk::Image * iffwd = Gtk::manage( new Gtk::Image( Gtk::Stock::GO_FORWARD, Gtk::ICON_SIZE_BUTTON )) ;
        m_BTN_HISTORY_FFWD = Gtk::manage( new Gtk::Button ) ;
	m_BTN_HISTORY_FFWD->set_relief( Gtk::RELIEF_NONE ) ;
	m_BTN_HISTORY_FFWD->set_border_width( 0 ) ; 
	m_BTN_HISTORY_FFWD->add( *iffwd ) ;
	m_BTN_HISTORY_FFWD->signal_clicked().connect( sigc::mem_fun( *this, &YoukiController::history_go_ffwd)) ;
	m_BTN_HISTORY_FFWD->set_sensitive( false ) ;

/*
        HBox_Navi->pack_start( *m_BTN_HISTORY_PREV, false, false, 0 ) ;
        HBox_Navi->pack_start( *m_BTN_HISTORY_FFWD, false, false, 0 ) ;
 */

        m_HBox_Entry->pack_start( *m_Label_Search, true, true, 0 ) ;
        m_HBox_Entry->pack_start( *HBox_Navi, false, false, 0 ) ;
        m_HBox_Entry->pack_start( *m_Entry, false, false, 0 ) ;

        Gtk::Alignment* Entry_Align = Gtk::manage( new Gtk::Alignment ) ;
        Entry_Align->add( *m_HBox_Entry ) ;
	Entry_Align->set_padding( 2, 2, 2, 4 ) ;
	Entry_Align->property_xalign() = 0.0 ;
	Entry_Align->property_xscale() = 0.0 ;

        Gtk::Alignment* Controls_Align = Gtk::manage( new Gtk::Alignment ) ;
        m_HBox_Controls = Gtk::manage( new Gtk::HBox ) ;
        m_HBox_Controls->set_spacing( 2 ) ;
	Controls_Align->add( *m_HBox_Controls ) ;
	Controls_Align->set_padding( 0, 0, 0, 2 ) ;

        m_VBox_Bottom = Gtk::manage( new Gtk::VBox ) ;
        m_VBox_Bottom->set_spacing( 1 ) ;

	Gtk::VBox* VBox2 = Gtk::manage( new Gtk::VBox ) ;
	VBox2->set_border_width( 0 ) ;
	VBox2->set_spacing( 4 ) ;

	Gtk::Alignment * Main_Align = Gtk::manage( new Gtk::Alignment ) ;
	Main_Align->add( *VBox2 ) ;
	Main_Align->set_padding( 0, 0, 4, 2 ) ;

	Gtk::Alignment * HBox_Main_Align = Gtk::manage( new Gtk::Alignment ) ;
	HBox_Main_Align->add( *m_HBox_Main ) ;
	HBox_Main_Align->set_padding( 0, 0, 1, 2 ) ;

        m_HBox_Bottom = Gtk::manage( new Gtk::HBox ) ;
        m_HBox_Bottom->set_spacing( 4 ) ;

	std::vector<Glib::RefPtr<Gdk::Pixbuf> > pixvector ;
	pixvector.push_back( Gdk::Pixbuf::create_from_file( Glib::build_filename( DATA_DIR, "images" G_DIR_SEPARATOR_S "youki16x16.png" ))) ;
	pixvector.push_back( Gdk::Pixbuf::create_from_file( Glib::build_filename( DATA_DIR, "images" G_DIR_SEPARATOR_S "youki32x32.png" ))) ;
	pixvector.push_back( Gdk::Pixbuf::create_from_file( Glib::build_filename( DATA_DIR, "images" G_DIR_SEPARATOR_S "youki64x64.png" ))) ;
	pixvector.push_back( Gdk::Pixbuf::create_from_file( Glib::build_filename( DATA_DIR, "images" G_DIR_SEPARATOR_S "youki96x96.png" ))) ;
	pixvector.push_back( Gdk::Pixbuf::create_from_file( Glib::build_filename( DATA_DIR, "images" G_DIR_SEPARATOR_S "youki128x128.png" ))) ;

        m_main_window = new MainWindow ;
        m_main_window->set_icon_list( pixvector ) ; 

	/* Main view widgets */
        m_ListViewTracks    = Gtk::manage( new View::Tracks::Class ) ;
        m_ListViewArtist    = Gtk::manage( new View::Artist::Class ) ;
        m_ListViewAlbums    = Gtk::manage( new View::Albums::Class ) ;

        m_ScrolledWinArtist = Gtk::manage( new Gtk::ScrolledWindow ) ;
        m_ScrolledWinAlbums = Gtk::manage( new Gtk::ScrolledWindow ) ;
        m_ScrolledWinTracks = Gtk::manage( new Gtk::ScrolledWindow ) ;

	/* Menus */
	m_UI_Manager = Gtk::UIManager::create() ;
	m_UI_Actions_Main = Gtk::ActionGroup::create("ActionsMain") ;

	m_UI_Actions_Main->add( Gtk::Action::create("MenuFile","_File")) ; 
	m_UI_Actions_Main->add( Gtk::Action::create("MenuView","_View")) ; 
	m_UI_Actions_Main->add( Gtk::Action::create("MenuPlaybackControl","_Playback")) ; 
	m_UI_Actions_Main->add( Gtk::Action::create("MenuAlbumOptions","_Album Options")) ; 

	m_UI_Actions_Main->add( Gtk::Action::create( "MenuFileActionQuit", Gtk::Stock::QUIT, "Quit Youki" ), Gtk::AccelKey('q',Gdk::CONTROL_MASK),sigc::mem_fun( *this, &YoukiController::initiate_quit)) ; 

	m_UI_Actions_Main->add( Gtk::Action::create_with_icon_name( "MenuFileActionPreferences", "mpx-stock-preferences", "Preferences", "Allows you to set Audio, Library, and other Preferences"), sigc::bind( sigc::mem_fun( *this, &YoukiController::on_show_subsystem_window), 0)) ; 
	m_UI_Actions_Main->add( Gtk::Action::create_with_icon_name( "MenuFileActionLibrary", "mpx-stock-musiclibrary", "Library", "Allows you to configure which Paths Youki controls"), sigc::bind( sigc::mem_fun( *this, &YoukiController::on_show_subsystem_window), 1)) ; 
	m_UI_Actions_Main->add( Gtk::Action::create_with_icon_name( "MenuFileActionPlugins", MPX_STOCK_PLUGIN, "Plugins", "Allows you to configure Plugins and turn them on or off"), sigc::bind( sigc::mem_fun( *this, &YoukiController::on_show_subsystem_window), 2)) ; 
	m_UI_Actions_Main->add( Gtk::Action::create( "MenuFileActionAbout", Gtk::Stock::ABOUT, "About" ), sigc::bind(sigc::ptr_fun(&show_about_window), m_main_window)) ; 
	m_UI_Actions_Main->add( Gtk::Action::create( "FocusEntry", "FocusEntry" ), Gtk::AccelKey("<F6>"), sigc::mem_fun( *m_Entry, &Gtk::Widget::grab_focus)) ; 

	m_UI_Actions_Main->add( Gtk::ToggleAction::create( "MenuViewActionUnderlineMatches", "Underline Search Matches" ), sigc::mem_fun( *this, &YoukiController::handle_action_underline_matches)) ; 
	m_UI_Actions_Main->add( Gtk::ToggleAction::create( "MenuPlaybackControlActionStartAlbumAtFavorite", "Start Albums at Favorite Track")) ; 
	m_UI_Actions_Main->add( Gtk::ToggleAction::create( "MenuPlaybackControlActionContinueCurrentAlbum", "Always Continue Playing Current Album" )) ; 
	m_UI_Actions_Main->add( Gtk::ToggleAction::create( "MenuViewActionAlbumRTViewModeBottom", "Show Release Type" ), sigc::mem_fun( *this, &YoukiController::on_rt_viewmode_change  )) ; 
	m_UI_Actions_Main->add( Gtk::ToggleAction::create( "MenuViewActionAlbumsShowTimeDiscsTracks", "Show Playtime, Discs, Track Count" ), sigc::mem_fun( *this, &YoukiController::handle_action_underline_matches ) ); 

	Glib::RefPtr<Gtk::ToggleAction> action_MOP = Gtk::ToggleAction::create( "MenuPlaybackControlActionMinimizeOnPause", "Minimize Youki on Pause" ) ;
	m_UI_Actions_Main->add( action_MOP ) ; 
	//mcs_bind->bind_toggle_action( action_MOP, "mpx", "minimize-on-pause" ) ;

	Glib::RefPtr<Gtk::ToggleAction> action_FPT = Gtk::ToggleAction::create( "MenuViewActionFollowPlayingTrack", "Follow Playing Track in View" ) ;
	m_UI_Actions_Main->add( action_FPT ) ; 
	//mcs_bind->bind_toggle_action( action_MOP, "mpx", "follow-current-track" ) ;

	Glib::RefPtr<Gtk::ToggleAction> action_PST = Gtk::ToggleAction::create( "MenuPlaybackControlActionPlayTrackOnSingleTap", "Play Tracks on Single Tap" ) ;
	m_UI_Actions_Main->add( action_PST, sigc::mem_fun(*this, &YoukiController::handle_play_track_on_single_tap)) ; 
	action_PST->set_active( mcs->key_get<bool>("mpx","play-on-single-tap")) ;
	mcs_bind->bind_toggle_action( action_PST, "mpx", "play-on-single-tap" ) ;

	m_UI_Manager->insert_action_group( m_UI_Actions_Main ) ;
	m_UI_Manager->add_ui_from_string( main_menubar_ui ) ;

	Gtk::Widget * menubar = m_UI_Manager->get_widget( "/MenuBarMain" ) ;
	dynamic_cast<Gtk::Container*>(menubar)->set_border_width( 0 ) ;

/*
	gtk_widget_realize(GTK_WIDGET(m_ScrolledWinTracks->gobj())) ;
	m_ScrolledWinTracks->get_vadjustment()->signal_changed().connect(sigc::mem_fun(*this,&YoukiController::on_list_view_tr_vadj_changed)) ;
 */

        m_ScrolledWinArtist->set_shadow_type( Gtk::SHADOW_IN ) ;
        m_ScrolledWinAlbums->set_shadow_type( Gtk::SHADOW_IN ) ;
        m_ScrolledWinTracks->set_shadow_type( Gtk::SHADOW_IN ) ;

        m_ScrolledWinArtist->set_kinetic_scrolling() ;
        m_ScrolledWinAlbums->set_kinetic_scrolling() ;
        m_ScrolledWinTracks->set_kinetic_scrolling() ;

        m_main_position = Gtk::manage( new KoboPosition ) ;
	m_main_position->set_size_request( -1, 19 ) ;
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

        m_VBox_Bottom->pack_start( *m_main_info, false, false, 0 ) ;
        m_VBox_Bottom->pack_start( *Controls_Align, false, false, 0 ) ;

        m_main_volume = Gtk::manage( new KoboVolume ) ;
	m_main_volume->set_size_request( 204, 19 ) ;
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
	    /* Tracks */

	    View::Tracks::DataModel_sp_t m ( new View::Tracks::DataModel ) ;
	    private_->FilterModelTracks = View::Tracks::DataModelFilter_sp_t (new View::Tracks::DataModelFilter( m )) ;

	    preload__tracks() ;

	    m_ListViewTracks->set_model( private_->FilterModelTracks ) ; 

	    View::Tracks::Column_sp_t c1 (new View::Tracks::Column(_("Track"))) ;
	    c1->set_column(5) ;
	    c1->set_alignment( Pango::ALIGN_RIGHT ) ;

	    View::Tracks::Column_sp_t c2 (new View::Tracks::Column(_("Title"))) ;
	    c2->set_column(0) ;

	    View::Tracks::Column_sp_t c3 (new View::Tracks::Column(_("Time"))) ;
	    c3->set_column(9) ;
	    c3->set_alignment( Pango::ALIGN_RIGHT ) ;

	    View::Tracks::Column_sp_t c4 (new View::Tracks::Column(_("Artist"))) ;
	    c4->set_column(1) ;

	    View::Tracks::Column_sp_t c5 (new View::Tracks::Column(_("Album"))) ;
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

	    m_ScrolledWinTracks->set_border_width( 0 ) ;
	    m_ScrolledWinTracks->add( *m_ListViewTracks ) ;
	    m_ScrolledWinTracks->show_all() ;
        }

        {
	    /* Album Artists */

	    View::Artist::DataModel_sp_t m (new View::Artist::DataModel) ;
	    private_->FilterModelArtist = View::Artist::DataModelFilter_sp_t (new View::Artist::DataModelFilter( m )) ;

	    preload__artists() ;

	    m_ListViewArtist->set_model( private_->FilterModelArtist ) ;

	    View::Artist::Column_sp_t c1 (new View::Artist::Column(_("Album Artist"))) ;
	    c1->set_column(0) ;
	    m_ListViewArtist->append_column(c1) ;

	    m_ScrolledWinArtist->set_border_width( 0 ) ;
	    m_ScrolledWinArtist->add( *m_ListViewArtist ) ;
	    m_ScrolledWinArtist->show_all() ;
        }

        {
	    /* Albums */

	    View::Albums::DataModel_sp_t m ( new View::Albums::DataModel ) ;
	    private_->FilterModelAlbums = View::Albums::DataModelFilter_sp_t (new View::Albums::DataModelFilter( m )) ;

	    preload__albums() ;

	    m_ListViewAlbums->set_model( private_->FilterModelAlbums ) ;

	    View::Albums::Column_sp_t c1 ( new View::Albums::Column ) ;
	    c1->set_column(0) ;
	    m_ListViewAlbums->append_column( c1 ) ;

	    m_ScrolledWinAlbums->set_border_width( 0 ) ;
	    m_ScrolledWinAlbums->add( *m_ListViewAlbums ) ;
	    m_ScrolledWinAlbums->show_all() ;
        }

        m_ListViewTracks->signal_track_activated().connect(
            sigc::mem_fun(
                      *this
                    , &YoukiController::on_list_view_tr_track_activated
        )) ;
        m_ListViewTracks->signal_find_propagate().connect(
            sigc::mem_fun(
                      *this
                    , &YoukiController::on_list_view_tr_find_propagate
        )) ;

        m_ListViewTracks->signal_only_this_album_mbid().connect(
            sigc::mem_fun(
                  *this
                , &YoukiController::on_list_view_ab_select_album
        )) ;

        m_ListViewTracks->signal_only_this_artist_mbid().connect(
            sigc::mem_fun(
                  *this
                , &YoukiController::on_list_view_ab_select_artist
        )) ;

        m_conn1 = m_ListViewArtist->signal_selection_changed().connect(
            sigc::mem_fun(
                  *this
                , &YoukiController::on_list_view_aa_selection_changed
        )) ;

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
                , &YoukiController::on_list_view_ab_selection_changed
        )) ;

        m_ListViewAlbums->signal_only_this_album_mbid().connect(
            sigc::mem_fun(
                  *this
                , &YoukiController::on_list_view_ab_select_album
        )) ;

        m_ListViewAlbums->signal_only_this_artist_mbid().connect(
            sigc::mem_fun(
                  *this
                , &YoukiController::on_list_view_ab_select_artist
        )) ;

        m_ListViewAlbums->signal_refetch_cover().connect(
            sigc::mem_fun(
                  *this
                , &YoukiController::on_list_view_ab_refetch_cover
        )) ;

        m_ListViewAlbums->signal_start_playback().connect(
            sigc::mem_fun(
                  *this
                , &YoukiController::on_list_view_ab_start_playback
        )) ;

        m_ListViewArtist->signal_start_playback().connect(
            sigc::mem_fun(
                  *this
                , &YoukiController::on_list_view_ab_start_playback
        )) ;

        m_HBox_Main->add_percentage( 0.15 ) ;
        m_HBox_Main->add_percentage( 0.25 ) ;
        m_HBox_Main->add_percentage( 0.60 ) ;

        m_HBox_Main->pack_start( *m_ScrolledWinArtist, true, true, 0 ) ;
        m_HBox_Main->pack_start( *m_ScrolledWinAlbums, true, true, 0 ) ;
        m_HBox_Main->pack_start( *m_ScrolledWinTracks, true, true, 0 ) ;

        std::vector<Gtk::Widget*> widget_v( 4 ) ; /* NEEDS TO BE CHANGED IF WIDGETS ARE RE-ADDED */
        widget_v[0] = m_Entry ;
        widget_v[1] = m_ScrolledWinArtist ;
        widget_v[2] = m_ScrolledWinAlbums ;
        widget_v[3] = m_ScrolledWinTracks ;

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

	Glib::RefPtr<Gtk::ToggleAction>::cast_static( m_UI_Actions_Main->get_action("MenuViewActionAlbumsShowTimeDiscsTracks"))->set_active( true ) ;
	Glib::RefPtr<Gtk::RadioAction>::cast_static( m_UI_Actions_Main->get_action("MenuViewActionAlbumRTViewModeBottom"))->set_active( true ) ;

        on_style_changed() ;
        m_VBox->show_all() ;

	m_HISTORY_POSITION = m_HISTORY.end() ;
	//history_save() ;
    }

    void
    YoukiController::update_entry_placeholder_text()
    {
	SQL::RowV v ;

	std::tr1::mt19937 eng;
	eng.seed(time(NULL)) ;
	std::tr1::uniform_int<unsigned int> uni(0,2) ;

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

	const std::string& s = boost::get<std::string>(v[0]["s"]) ;
	m_Entry->set_placeholder_text((boost::format("%s \"%s\"") % _("Search Your Music... for example:") % s).str()) ;
    }

    void
    YoukiController::preload__artists()
    {
	private_->FilterModelArtist->append_artist("",-1);

	SQL::RowV v ;
	m_library->getSQL(v, (boost::format("SELECT * FROM album_artist")).str()) ; 

	std::stable_sort( v.begin(), v.end(), CompareAlbumArtists ) ;

	for( SQL::RowV::iterator i = v.begin(); i != v.end(); ++i )
	{
	    SQL::Row & r = *i;

	    private_->FilterModelArtist->append_artist(
		  Util::row_get_album_artist_name( r )
		, boost::get<guint>(r["id"])
	    ) ;
	}

        private_->FilterModelArtist->regen_mapping() ;
    }

    void
    YoukiController::preload__albums()
    {
	MPX::View::Albums::Album_sp dummy_album ( new MPX::View::Albums::Album ) ;
	dummy_album->album_id = -1 ;
	private_->FilterModelAlbums->append_album( dummy_album ) ;

	//

	SQL::RowV v ;

	try{

	    m_library->getSQL(
	      v , "SELECT album.id AS id, album_artist.mb_album_artist_id AS mbid_artist FROM album"
		  " JOIN album_artist"
		  " ON album.album_artist_j = album_artist.id ORDER BY"
		  " album_artist, mb_release_date, album"
	    ) ; 

	} catch (MPX::SQL::SqlGenericError & cxe )
	{
	    handle_sql_error( cxe ) ;
	}

	for( SQL::RowV::iterator i = v.begin(); i != v.end(); ++i )
	{
	    guint id = get<guint>((*i)["id"]); 

	    try{
		private_->FilterModelAlbums->append_album( get_album_from_id( id )) ;
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
	m_library->getSQL(v, "SELECT * FROM track_view ORDER BY album_artist, mb_release_date, album, discnr, track_view.track") ;

	for( SQL::RowV::iterator i = v.begin(); i != v.end(); ++i )
	{
	    SQL::Row & r = *i;

	    try{
		private_->FilterModelTracks->append_track(r, m_library->sqlToTrack(r, true, false ) ) ;
	    } catch( Library::FileQualificationError )
	    {
	    }
	}

        private_->FilterModelTracks->create_identity_mapping() ;
	tracklist_regen_mapping() ;
    }

    void
    YoukiController::history_load_current_iter()
    {
	if( m_HISTORY_POSITION != m_HISTORY.end() )
	{
	    const HistoryItem& item = *m_HISTORY_POSITION ;

	    if( item.Searchtext )
	    {
		m_Entry->set_text( item.Searchtext.get() ) ;
	    }
	    else
	    {
		on_entry_clear_clicked() ;
	    }

	    if( item.AlbumArtistId )
	    {
		m_ListViewArtist->select_id( item.AlbumArtistId.get() ) ;
		boost::optional<guint> idx = m_ListViewArtist->get_selected_index() ;
		if( idx )
		    m_ListViewArtist->scroll_to_index(idx.get()) ;
	    }

	    if( item.AlbumId )
	    {
		m_ListViewAlbums->select_id( item.AlbumId.get() ) ;
		boost::optional<guint> idx = m_ListViewAlbums->get_selected_index() ;
		if( idx )
		    m_ListViewAlbums->scroll_to_index(idx.get()) ;
	    }
	}
    }

    void
    YoukiController::history_go_back()
    {
	if( m_HISTORY_POSITION != m_HISTORY.begin() && m_HISTORY.size() > 0 )
	{
	    m_HISTORY_POSITION-- ;

	    m_history_ignore_save = true ;
	    history_load_current_iter() ;
	    m_history_ignore_save = false ;
	}

	m_BTN_HISTORY_PREV->set_sensitive( m_HISTORY_POSITION != m_HISTORY.begin() ) ;
	m_BTN_HISTORY_FFWD->set_sensitive( m_HISTORY.size() > 1 ) ;
    }

    void
    YoukiController::history_go_ffwd()
    {
	HistoryPosition_t iter_last = m_HISTORY.end() ;
	iter_last-- ;

	if( m_HISTORY_POSITION != iter_last && m_HISTORY.size() > 0 )
	{
	    m_HISTORY_POSITION++ ;

	    m_history_ignore_save = true ;
	    history_load_current_iter() ;
	    m_history_ignore_save = false ;
	}

	iter_last = m_HISTORY.end() ;
	iter_last-- ;

	m_BTN_HISTORY_FFWD->set_sensitive( m_HISTORY_POSITION != iter_last ) ;
	m_BTN_HISTORY_PREV->set_sensitive( m_HISTORY.size() > 1 ) ;
    }

    void
    YoukiController::history_save()
    {
	if( m_history_ignore_save )
		return ;

	HistoryItem item ;

	if( !m_Entry->get_text().empty() )
	{
	    item.Searchtext = m_Entry->get_text() ;
	}

	item.AlbumArtistId = m_ListViewArtist->get_selected() ;
	item.AlbumId = m_ListViewAlbums->get_selected() ;

	if( m_HISTORY_POSITION != m_HISTORY.end() )
	{
	    HistoryPosition_t iter = m_HISTORY_POSITION ;
	    iter++ ;
	    m_HISTORY.erase( iter, m_HISTORY.end() ) ;
	}

	m_HISTORY.push_back( item ) ;
	m_HISTORY_POSITION = m_HISTORY.end() ; 
        m_HISTORY_POSITION-- ;

	m_BTN_HISTORY_FFWD->set_sensitive( false ) ; 
	m_BTN_HISTORY_PREV->set_sensitive( m_HISTORY.size() > 1 ) ;
    }
    
    void
    YoukiController::on_rt_viewmode_change()
    {
	MPX::View::Albums::RTViewMode m = MPX::View::Albums::RTViewMode( int(Glib::RefPtr<Gtk::ToggleAction>::cast_static( m_UI_Actions_Main->get_action("MenuViewActionAlbumRTViewModeBottom"))->get_active())) ;
	m_ListViewAlbums->set_rt_viewmode( m ) ;
    }

    void
    YoukiController::handle_action_underline_matches()
    {
	bool active = Glib::RefPtr<Gtk::ToggleAction>::cast_static( m_UI_Actions_Main->get_action("MenuViewActionUnderlineMatches"))->get_active() ;
	m_ListViewTracks->set_highlight( active ) ;

	active = Glib::RefPtr<Gtk::ToggleAction>::cast_static( m_UI_Actions_Main->get_action("MenuViewActionAlbumsShowTimeDiscsTracks"))->get_active() ;
	m_ListViewAlbums->set_show_release_label( active ) ;
    }

    void
    YoukiController::handle_play_track_on_single_tap()
    {
	bool active = Glib::RefPtr<Gtk::ToggleAction>::cast_static( m_UI_Actions_Main->get_action("MenuPlaybackControlActionPlayTrackOnSingleTap"))->get_active() ;
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
	    Glib::signal_idle().connect_once( sigc::mem_fun( *this, &YoukiController::on_entry_changed )) ;
	}

	return true ;
    }

    YoukiController::~YoukiController ()
    {
	Glib::RefPtr<Gtk::ToggleAction>::cast_static( m_UI_Actions_Main->get_action("MenuPlaybackControlActionStartAlbumAtFavorite"))->set_active(false) ;

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
            , 64
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
        album->album_artist = get<std::string>(r["album_artist"]) ; //Util::row_get_album_artist_name( r ) ; 
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
    YoukiController::initiate_quit ()
    {
        Gtk::Main::quit() ;
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
        private_->FilterModelTracks->enable_fragment_cache() ;
	tracklist_regen_mapping() ;
        private_->FilterModelArtist->regen_mapping() ;
        private_->FilterModelAlbums->regen_mapping() ;
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

        private_->FilterModelArtist->insert_artist(
              boost::get<std::string>(v[0]["album_artist"])
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
        , int                   type
    )
    {
        switch( type )
        {
            case 0: /* track */
            {
                private_->FilterModelTracks->erase_track( id ) ; 
		tracklist_regen_mapping() ;
            }
            break ;

            case 1: /* album */
            {
                private_->FilterModelAlbums->erase_album( id ) ; 
                private_->FilterModelAlbums->regen_mapping() ;

                SQL::RowV v ;

                m_library->getSQL(v, (boost::format("SELECT max(id) AS id FROM album_artist")).str()) ; 
                guint max_artist = boost::get<guint>(v[0]["id"]) ;

                v.clear();

                m_library->getSQL(v, (boost::format("SELECT max(id) AS id FROM album")).str()) ; 
                guint max_albums = boost::get<guint>(v[0]["id"]) ;
                private_->FilterModelTracks->set_sizes( max_artist, max_albums ) ;
            }
            break ;

            case 2: /* artist */
            {
            }
            break ;

            case 3: /* album artist */
            {
                private_->FilterModelArtist->erase_artist( id ) ; 
                private_->FilterModelArtist->regen_mapping() ; 

                SQL::RowV v ;
                m_library->getSQL(v, (boost::format("SELECT max(id) AS id FROM album_artist")).str()) ; 
                guint max_artist = boost::get<guint>(v[0]["id"]) ;
                v.clear();
                m_library->getSQL(v, (boost::format("SELECT max(id) AS id FROM album")).str()) ; 
                guint max_albums = boost::get<guint>(v[0]["id"]) ;
                private_->FilterModelTracks->set_sizes( max_artist, max_albums ) ;
            }
            break;
        }

	return false ;
    }

    void
    YoukiController::on_library_entity_deleted(
          guint                id
        , int                   type
    )
    {
	Glib::signal_idle().connect( sigc::bind( sigc::mem_fun( *this, &YoukiController::on_library_entity_deleted_idle ), id, type )) ;
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
            , 64
        ) ;

        if( cover_pb ) 
        {
            cover_is = Util::cairo_image_surface_from_pixbuf( cover_pb ) ;
        }

        private_->FilterModelAlbums->update_album_cover( id, cover_is ) ;

	if( m_track_current )
	{
	    MPX::Track& track = *(m_track_current.get()) ;

	    if( track.has( ATTRIBUTE_MB_ALBUM_ID ) )
	    {
		    Glib::RefPtr<Gdk::Pixbuf> cover ;

		    if(  m_covers->fetch(
			  boost::get<std::string>(track[ATTRIBUTE_MB_ALBUM_ID].get())
			, cover
		    ))
		    {
			m_main_info->set_cover( cover ) ;
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
        for( std::vector<guint>::const_iterator i = m_new_tracks.begin(); i != m_new_tracks.end() ; ++i )
        {
            SQL::RowV v ;

	    m_library->getSQL(v, (boost::format("SELECT * FROM track_view WHERE track_view.id = '%u' ORDER BY ifnull(album_artist_sortname,album_artist), mb_release_date, album, discnr, track_view.track") % *i ).str()) ; 

            if( v.size() != 1)
            {
                g_critical(" Got multiple tracks with the same ID / this can not happen.") ;
                continue ;
            }

            private_->FilterModelTracks->insert_track( v[0], m_library->sqlToTrack( v[0], true, false ) ) ;
        }

        m_new_tracks.clear() ;

	tracklist_regen_mapping() ;

        while (gtk_events_pending())
            gtk_main_iteration() ;
    }

/*////////// */

    void
    YoukiController::play_track(
          const MPX::Track_sp& t
    )
    {
        try{
                m_track_current = t ; 
                m_play->switch_stream( m_library->trackGetLocation( t ) ) ;
		assign_metadata_to_DBus_property() ;

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
	std::string e = (boost::format("[%s]: %s") % a % b).str() ;	
	m_Label_Error->set_text( e ) ;
    }

    void
    YoukiController::on_play_eos ()
    {
	time_t t = time(NULL) ;

        m_main_position->stop() ;
        emit_track_out () ;

	m_track_previous = m_track_current ;
	m_track_current.reset() ;

	if( !m_play_queue.empty() ) /* tracks in the play queue? */
	{
	    /* ... so get next track from the play queue! */

	    Track_sp p = m_library->getTrackById( m_play_queue.front() ) ;
	    m_play_queue.pop_front() ;

	    play_track( p ) ;
	    goto x1 ;
	}
	else
	{
	    bool active = Glib::RefPtr<Gtk::ToggleAction>::cast_static( m_UI_Actions_Main->get_action("MenuPlaybackControlActionContinueCurrentAlbum"))->get_active() ;

	    if( active ) 
	    {
		const MPX::Track& track = *(m_track_previous.get()) ;
	
		guint album_id = boost::get<guint>(track[ATTRIBUTE_MPX_ALBUM_ID].get()) ;
    
		const char * album_top_f = "SELECT id FROM track WHERE album_j = '%u' ORDER BY track ASC" ;

		SQL::RowV v ;
		m_library->getSQL( v, mprintf( album_top_f, album_id )) ; 

		if( !v.empty() )
		{
		    SQL::RowV::iterator i = v.begin() ;

		    for( ; i != v.end(); ++i )
		    {
			guint id = boost::get<guint>((*i)["id"]) ;

			if( id == boost::get<guint>(track[ATTRIBUTE_MPX_TRACK_ID].get()))
			{
		    	    ++i ;		    
			    break ;
			}
		    }

		    if( i != v.end() ) 
		    {
			guint next_id = boost::get<guint>((*i)["id"]) ;
			play_track( m_library->getTrackById( next_id )) ;		    
			goto x1 ;
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
		    /*/ ...so advance to the next FIXME: Flow plugins: HERE */
		    guint pos_next = pos.get() + 1 ;

		    if( pos_next < private_->FilterModelTracks->size() )
		    {
			play_track( boost::get<4>(private_->FilterModelTracks->row( pos_next )) ) ;
			goto x1 ;
		    }
		}
		else
		{
		    /*/ Not in the current projection? OK, so start with the top tracki, if the projection's size is > 0 */
		    if( private_->FilterModelTracks->size() )
		    {
			play_track( boost::get<4>(private_->FilterModelTracks->row( 0 )) ) ;
			goto x1 ;
		    }
		}
	    }
	}

	m_play->request_status( PLAYSTATUS_STOPPED ) ; 

	x1:
        m_library->trackPlayed( m_track_previous, t ) ;
    }

    void
    YoukiController::register_played_track()
    {
    }

    void
    YoukiController::tracklist_regen_mapping()
    {
        if( m_track_current && 
	    Glib::RefPtr<Gtk::ToggleAction>::cast_static( m_UI_Actions_Main->get_action("MenuViewActionFollowPlayingTrack"))->get_active())
        {
            const MPX::Track& track = *(m_track_current.get()) ;
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
	    Glib::RefPtr<Gtk::ToggleAction>::cast_static( m_UI_Actions_Main->get_action("MenuViewActionFollowPlayingTrack"))->get_active())
        {
            const MPX::Track& track = *(m_track_current.get()) ;
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
		m_Label_Error->set_text("") ;
		m_main_position->unpause() ;
                break ;

            case PLAYSTATUS_STOPPED:

                m_track_current.reset() ;
		assign_metadata_to_DBus_property() ;

                m_seek_position.reset() ; 
                m_main_info->clear() ;
		m_main_position->unpause() ;
                m_main_position->set_position( 0, 0 ) ;
		m_main_window->set_title("Youki") ;

                private_->FilterModelTracks->clear_active_track() ;

                if( m_main_window )
                    m_main_window->queue_draw () ;    

                break ;

            case PLAYSTATUS_WAITING:

		m_main_position->unpause() ;
                m_seek_position.reset() ; 
                m_main_info->clear() ;

                if( m_main_window )
		{
                    m_main_window->queue_draw () ;    
		}

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
        m_main_position->start() ;

        Track_sp t = m_track_current ;
        g_return_if_fail( bool(t) ) ;

        const MPX::Track& track = *(t.get()) ;

        emit_track_new() ;

        guint id_track = boost::get<guint>(track[ATTRIBUTE_MPX_TRACK_ID].get()) ;

        private_->FilterModelTracks->set_active_id( id_track ) ;

        if( Glib::RefPtr<Gtk::ToggleAction>::cast_static( m_UI_Actions_Main->get_action("MenuViewActionFollowPlayingTrack"))->get_active())
        {
            m_ListViewTracks->scroll_to_id( id_track ) ;
        }

        std::vector<std::string> info ;
        info.push_back( boost::get<std::string>(track[ATTRIBUTE_ARTIST].get()) ) ;
        info.push_back( boost::get<std::string>(track[ATTRIBUTE_ALBUM].get()) ) ;
        info.push_back( boost::get<std::string>(track[ATTRIBUTE_TITLE].get()) ) ;
	m_main_info->set_info( info ) ;

        if( track.has( ATTRIBUTE_MB_ALBUM_ID ) )
        {
                Glib::RefPtr<Gdk::Pixbuf> cover ;

                if(  m_covers->fetch(
                      boost::get<std::string>(track[ATTRIBUTE_MB_ALBUM_ID].get())
                    , cover
                ))
                {
                    m_main_info->set_cover( cover ) ;
		    goto skip1 ;
                }
        }

	m_main_info->set_cover( Glib::RefPtr<Gdk::Pixbuf>(0) ) ;
	
	skip1:

	m_main_window->set_title((boost::format("Youki :: %s - %s") % info[0] % info[2]).str()) ;
    }

    void
    YoukiController::on_play_metadata(
            GstMetadataField    field
    )
    {
	const GstMetadata& m = m_play->get_metadata() ;

        if( field == FIELD_AUDIO_BITRATE )
        {
		m_main_info->set_bitrate( m.m_audio_bitrate.get() / 999.9 ) ;
        }
        else
        if( field == FIELD_AUDIO_CODEC )
        {
		m_main_info->set_codec( m.m_audio_codec.get() ) ;
        }
    }

    void
    YoukiController::on_list_view_tr_track_activated(
          MPX::Track_sp t 
        , bool          play
    ) 
    {
        const MPX::Track& track = *(t.get()) ;

        emit_track_cancelled() ;

        if( play )
        {
            play_track( t ) ;
        }
        else
        {
            m_play_queue.push_back( boost::get<guint>(track[ATTRIBUTE_MPX_TRACK_ID].get()) ) ;
        }
    }

    void
    YoukiController::on_list_view_tr_find_propagate(
          const std::string&    text
    )
    {
        m_Entry->set_text( (boost::format("title %% \"%s\"") % text).str() ) ;
    }

    void
    YoukiController::on_list_view_aa_selection_changed(
    ) 
    {
        m_ListViewAlbums->clear_selection() ;

        private_->FilterModelTracks->clear_synthetic_constraints_quiet() ;
//	private_->FilterModelTracks->clear_single_album_constraint_quiet() ;

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

	tracklist_regen_mapping() ;
	

	private_->FilterModelAlbums->set_constraints_albums( private_->FilterModelTracks->m_constraints_albums ) ;
	private_->FilterModelAlbums->set_constraints_artist( private_->FilterModelTracks->m_constraints_artist ) ;
        private_->FilterModelAlbums->regen_mapping() ;

	m_ListViewAlbums->select_index( 0, true ) ;
	
	//history_save() ;
    }

    void
    YoukiController::on_list_view_ab_selection_changed(
    ) 
    {
        private_->FilterModelTracks->clear_synthetic_constraints_quiet() ;
//	private_->FilterModelTracks->clear_single_album_constraint_quiet() ;

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
/*
	else
	{
	    private_->FilterModelTracks->set_constraint_single_album( id_albums.get() ) ;
	}
 */

	private_->FilterModelTracks->regen_mapping_iterative() ;
    
	//history_save() ;
    }

    void
    YoukiController::on_list_view_ab_select_album(
        const std::string&  mbid
    )
    {
        on_entry_clear_clicked() ;
        m_Entry->set_text( (boost::format("album-mbid = \"%s\"") % mbid).str() ) ;
    }

    void
    YoukiController::on_list_view_ab_select_artist(
        const std::string&  mbid
    )
    {
        on_entry_clear_clicked() ;
        m_Entry->set_text( (boost::format("album-artist-mbid = \"%s\"") % mbid).str() ) ;
    }

    void
    YoukiController::on_list_view_ab_refetch_cover(
	guint id
    )
    {
	m_library->recacheAlbumCover( id ) ;
    }

 
    void
    YoukiController::on_list_view_tr_vadj_changed(
    ) 
    {
	Glib::RefPtr<Gtk::ToggleAction>::cast_static( m_UI_Actions_Main->get_action("MenuPlaybackControlActionStartAlbumAtFavorite"))->set_active(false) ;
    }

    void
    YoukiController::on_entry_activated(
    )
    {
	if( private_->FilterModelTracks->size() )
        {
        	play_track( boost::get<4>(private_->FilterModelTracks->row( m_ListViewTracks->get_upper_row()  )) ) ;
	}
    }

    bool
    YoukiController::on_entry_key_press_event(
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
                    on_entry_clear_clicked() ;
                    return true ;
                }

                break ;
            }

            case GDK_KEY_c:
            case GDK_KEY_C:
            {
                if( event->state & GDK_CONTROL_MASK )
                {
                    on_entry_clear_clicked() ;
                    return true ;
                }

                break ;
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

        //m_switchfocus = true ;
	//m_keytimer.reset() ;

        return false ;
    }

    void
    YoukiController::on_entry_clear_clicked(
	  Gtk::EntryIconPosition p
	, const GdkEventButton* G_GNUC_UNUSED
    )
    {
	if( p != Gtk::ENTRY_ICON_SECONDARY )
	    return ;

        private_->FilterModelAlbums->clear_constraints_artist() ;
        private_->FilterModelAlbums->clear_constraints_album() ;
        private_->FilterModelAlbums->clear_all_constraints_quiet() ;

        private_->FilterModelArtist->clear_constraints_artist() ;

        private_->FilterModelTracks->clear_synthetic_constraints_quiet() ;
/*        private_->FilterModelTracks->clear_single_artist_constraint_quiet() ; */


        m_Entry->set_text( "" ) ;

        if( m_track_current && 
	    Glib::RefPtr<Gtk::ToggleAction>::cast_static( m_UI_Actions_Main->get_action("MenuViewActionFollowPlayingTrack"))->get_active())
        {
            const MPX::Track& track = *(m_track_current.get()) ;
            guint id_track = boost::get<guint>(track[ATTRIBUTE_MPX_TRACK_ID].get()) ;
	    private_->FilterModelTracks->regen_mapping( id_track ) ;
        }
	else
	{
	    private_->FilterModelTracks->regen_mapping() ;
	}	

        private_->FilterModelArtist->regen_mapping() ;
        private_->FilterModelAlbums->regen_mapping() ;

        m_conn1.block() ;
        m_conn2.block() ;
        m_conn4.block() ;

        m_ListViewArtist->scroll_to_index(0) ;
        m_ListViewArtist->select_index(0) ;

        m_ListViewAlbums->scroll_to_index(0) ;
        m_ListViewAlbums->select_index(0) ;

        m_conn1.unblock() ;
        m_conn2.unblock() ;
        m_conn4.unblock() ;

	m_Entry->grab_focus() ;
	//history_save() ;
    }

    void
    YoukiController::on_entry_changed(
    )
    {
        m_ListViewAlbums->clear_selection() ;
        m_ListViewArtist->clear_selection() ;

        private_->FilterModelTracks->clear_synthetic_constraints_quiet() ;
/*        private_->FilterModelTracks->clear_single_album_constraint_quiet() ; */
        private_->FilterModelTracks->set_filter( m_Entry->get_text() ) ;

        private_->FilterModelArtist->set_constraints_artist( private_->FilterModelTracks->m_constraints_artist ) ;
/*        private_->FilterModelArtist->regen_mapping() ; */

        private_->FilterModelAlbums->set_constraints_albums( private_->FilterModelTracks->m_constraints_albums ) ;
        private_->FilterModelAlbums->set_constraints_artist( private_->FilterModelTracks->m_constraints_artist ) ;
        private_->FilterModelAlbums->regen_mapping() ;

	if( private_->FilterModelTracks->m_mapping->empty() )
	{
	    m_Entry->override_color(Util::make_rgba(1.,0.,0.,1.)) ;
	}
	else
	{
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
    YoukiController::on_info_area_clicked( int i )
    {
	TapArea area = TapArea(i) ;

	if( area == TAP_CENTER )
	{
	    if( m_play->property_status().get_value() == PLAYSTATUS_STOPPED )
	    {
		if( private_->FilterModelTracks->size() )
		{
		    OptUInt idx = m_ListViewTracks->get_selected_index() ; 
		    play_track( boost::get<4>(private_->FilterModelTracks->row( idx ? idx.get() : 0 )) ) ;
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
    YoukiController::on_list_view_ab_start_playback ()
    {
	bool active = Glib::RefPtr<Gtk::ToggleAction>::cast_static( m_UI_Actions_Main->get_action("MenuPlaybackControlActionStartAlbumAtFavorite"))->get_active() ;

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
			    play_track( boost::get<4>(private_->FilterModelTracks->row(model_track_pos.get()))) ;
			    return ;
			}
			/* ...unwind */
		}
		/* unwind... */
	    }

	    /* ...aaaand... fallback */
	   
	    play_track( boost::get<4>(private_->FilterModelTracks->row(0))) ;
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
                //m_mlibman_dbus_proxy->ShowWindow () ;
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
        m_play_queue.push_back( id ) ;
    }

    std::vector<guint>
    YoukiController::get_current_play_queue(
    )
    {
        std::vector<guint> v ; 
        std::deque<guint> queue_copy = m_play_queue ;

        while( !queue_copy.empty() )
        {
            v.push_back( queue_copy.front() ) ;
            queue_copy.pop_front() ;
        }

        return v ;
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
            return *(m_track_current.get()) ;
        else
            throw std::runtime_error("No current track!") ; /* FIXME: Well */
    }

    MPX::Track&
    YoukiController::get_metadata_previous(
    )
    {
        if( m_track_previous )
            return *(m_track_previous.get()) ;
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
            on_play_eos() ;
        }
    }

    void
    YoukiController::API_prev(
    )
    {
	//FIXME: This doesn't work at all with flow plugins, of course we also don't have any yet
	if( private_->FilterModelTracks->size() )
	{
	    guint d = 0 ;

	    OptUInt idx = private_->FilterModelTracks->get_active_track() ; 

	    if( idx ) 
	    {
		d = std::max<int>( 0, idx.get() - 1 ) ;
	    }

	    play_track( boost::get<4>(private_->FilterModelTracks->row( d ))) ;
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
