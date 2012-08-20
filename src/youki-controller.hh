#ifndef YOUKI_CONTROLLER_HH
#define YOUKI_CONTROLLER_HH

#include "config.h"

#include "mpx/i-youki-controller.hh"
#include "mpx-mlibman-dbus-proxy-actual.hh"

#include <gtkmm.h>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <array>

#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>

#include "mpx/mpx-services.hh"
#include "mpx/mpx-types.hh"

#include "mpx/util-string.hh"

#include "mpx/algorithm/aque.hh"

#include "kobo-main.hh"
#include "kobo-position.hh"
#include "kobo-volume.hh"
#include "kobo-titleinfo.hh"
#include "webkit-album-info.hh"

#include "ld-find-nearest.hh"
#include "id-history.hh"

#include "resource_manager.hh"
#include "rm-covers.hh"

namespace MPX
{
    class Covers ;
    class Library ;
    class Play ;

    namespace View
    {
        namespace Tracks
        {
            class Class ;
        }

        namespace Albums
        {
            struct Album ;
            class Class ;
        }

        namespace Artist
        {
            class Class ;
        }
    }

    struct ViewHistoryItem
    {
	    std::string			FilterText ;
	    boost::optional<guint>	SelectionArtist ;
	    boost::optional<guint>	SelectionAlbums ;
    };

    typedef std::list<ViewHistoryItem> ViewHistory ;
    typedef ViewHistory::iterator      ViewHistoryIter ;

    class PercentualDistributionHBox ;

    class YoukiController
    : public IYoukiController
    , public Service::Base
    {
        public: 

            typedef sigc::signal<void>  Signal0_void ;

        private:

            Signal0_void        m_SIG_track_new ;
            Signal0_void        m_SIG_track_out ;
            Signal0_void        m_SIG_track_cancelled ;

        public:

            Signal0_void&
            on_track_new ()
            {
                return m_SIG_track_new ;
            }

            Signal0_void&
            on_track_out ()
            {
                return m_SIG_track_out ;
            }

            Signal0_void&
            on_track_cancelled ()
            {
                return m_SIG_track_cancelled ;
            }

        private:

            void
            emit_track_new ()
            {
                m_SIG_track_new.emit() ;

                g_signal_emit(
                      G_OBJECT(gobj())
                    , m_C_SIG_ID_track_new
                    , 0
                ) ;
            }

            void
            emit_track_out ()
            {
                m_SIG_track_out.emit() ;

                g_signal_emit(
                      G_OBJECT(gobj())
                    , m_C_SIG_ID_track_out
                    , 0
                ) ;
            }

            void
            emit_track_cancelled ()
            {
                m_SIG_track_cancelled.emit() ;

                g_signal_emit(
                      G_OBJECT(gobj())
                    , m_C_SIG_ID_track_cancelled
                    , 0
                ) ;
            }

        public:

            YoukiController() ;
            virtual ~YoukiController() ;

            Gtk::Window*
            get_widget () ;

        public: // PLUGIN API

	    void
	    show_hide_spinner(bool) ;

            void
            queue_next_track(
                guint
            ) ;

            std::vector<guint>
            get_current_play_queue () ;

            int
            get_status(
            ) ;

            MPX::Track&
            get_metadata(
            ) ;

            MPX::Track&
            get_metadata_previous(
            ) ;

            virtual void
            API_pause_toggle(
            ) ;

            virtual void
            API_next(
            ) ;

            virtual void
            API_prev(
            ) ;

            virtual void
            API_stop(
            ) ;

            void
            API_play_track(
                guint  id
            ) ; 

            virtual void        
            add_info_widget(
                  Gtk::Widget*          w
                , const std::string&    name
            )
            {
                m_NotebookPlugins->append_page(
                      *w
                    , name
                ) ;
                w->show_all() ;
            }

            virtual void
            remove_info_widget(
                  Gtk::Widget*          w
            )
            {
                m_NotebookPlugins->remove_page(
                      *w
                ) ;
            }

        protected:

            struct Private ;
            Private                         * private_ ;

            Glib::RefPtr<Gtk::StatusIcon>     m_status_icon ;
            MainWindow                      * m_MainWindow ;

            int                               m_MainWindow_x 
                                            , m_MainWindow_y ;
            
	    KoboTitleInfo		    * m_main_info ;
            KoboPosition                    * m_main_position ;
            KoboVolume                      * m_main_volume ;

	    AlbumInfo			    * m_album_info ;

            View::Artist::Class             * m_ListViewArtist ;
            View::Albums::Class             * m_ListViewAlbums ;
            View::Tracks::Class             * m_ListViewTracks ;

            Gtk::ScrolledWindow             * m_ScrolledWinArtist ;
            Gtk::ScrolledWindow             * m_ScrolledWinAlbums ;
            Gtk::ScrolledWindow             * m_ScrolledWinTracks ;

	    Gtk::Button			    * m_BTN_HISTORY_PREV ;
	    Gtk::Button			    * m_BTN_HISTORY_FFWD ;

            Gtk::HBox			    * m_HBox_Main ;
            Gtk::HBox                       * m_HBox_Bottom ;
            Gtk::VBox                       * m_VBox_Bottom ;
	    Gtk::VBox			    * m_VBox_TL ;
	    Gtk::Label			    * m_Label_TL ;
            Gtk::Entry                      * m_Entry ;

	    Gtk::RadioButton		    * m_rb1, * m_rb2, * m_rb3 ;

            Gtk::Alignment                  * m_Alignment_Entry ;
            Gtk::HBox                       * m_HBox_Entry ;
            Gtk::HBox                       * m_HBox_Info ;
            Gtk::HBox                       * m_HBox_Controls ;

            Gtk::Label                      * m_Label_Search ;
	    Gtk::Label			    * m_Label_Nearest ;

            Gtk::VBox                       * m_VBox ;
            Gtk::Notebook                   * m_NotebookPlugins ;
	    Gtk::Image			    * m_ActivitySpinner ;
	    Gtk::EventBox		    * m_EventBox_Nearest ;

	    Gtk::InfoBar		    * m_InfoBar ;
	    Gtk::Label			    * m_InfoLabel ; 
	    Gtk::Button			    * m_BTN_SHUFFLE ;
	    std::array<Glib::RefPtr<Gdk::Pixbuf>,3> m_pin ;
	    boost::optional<std::string>      m_pinned_query ;

	    ResourceManager<MPX::RM::AlbumImage> m_covers ;

            MPX::Play                       * m_play ;
            Library                         * m_library ;

	    LDFindNearest		      m_find_nearest_artist ;
	    LDFindNearest		      m_find_nearest_artist_full ;
	    LDFindNearest		      m_find_nearest_artist_full_lc ;

	    std::string			      m_nearest ;

	    StrV			      m_nearest__artists ;
	    StrV			      m_nearest__artists_popular ;

	    AQE::Constraints_t			    m_aqe_natural ;		
	    boost::optional<AQE::Constraints_t>	    m_aqe_synthetic_pinned ;		

#if 0
	    PlaylistManager		    * m_PlaylistManager ;
	    PlaylistGUI			    * m_PlaylistGUI ;
	    Gtk::ScrolledWindow		    * m_ScrolledWindowPlaylists ;
#endif

	    typedef boost::shared_ptr<LDFindNearest>		LDFN_p ;
	    typedef std::unordered_map<std::string, LDFN_p>	StrLDFN_map_t ;
	    typedef std::unordered_set<std::string>		StrSet_t ;
	    typedef std::unordered_map<std::string, StrSet_t>	StrSet_map_t ;

	    StrSet_map_t		      m_ssmap ;
	    StrLDFN_map_t		      m_ldmap ; 
    
            Track_sp                          m_track_current ;          
            Track_sp                          m_track_previous ;          
            boost::optional<guint>            m_seek_position ;
            bool                              m_follow_track ;

            std::vector<guint>	    m_play_queue ;
	    IdHistory		    m_play_history ;

	    ViewHistory		    m_view_history ;
	    ViewHistoryIter	    m_view_history_pos ;
	    
            info::backtrace::Youki::MLibMan_proxy_actual * m_mlibman_dbus_proxy ;

            sigc::connection                  m_conn1
                                            , m_conn2
                                            , m_conn3
                                            , m_conn4
                                            , m_conn5 ;

            guint                             m_C_SIG_ID_track_new ;
            guint                             m_C_SIG_ID_track_out ;
            guint                             m_C_SIG_ID_track_cancelled ;

            std::vector<guint>                m_new_tracks ;

	    Glib::Timer			      m_keytimer ;
	    bool			      m_switchfocus ;
	    sigc::connection		      m_conn_keytimer ; 

	    Glib::RefPtr<Gtk::UIManager>      m_UIManager ;
	    Glib::RefPtr<Gtk::ActionGroup>    m_UIActions_Main ;
	    Glib::RefPtr<Gtk::ActionGroup>    m_UIActions_Tracklist ;

	    void
	    play_next_queue_item() ;

	    void
	    history_save() ;

	    void
	    history_go_back() ;

	    void
	    check_markov_queue_append() ;

	    void
	    check_markov_queue_append_real(MPX::Track_sp) ;

	    void
	    check_markov_queue_append_real(guint) ;

	    void
	    clear_play_queue() ;

	    void
	    apply_queue() ;

	    void
	    apply_history() ;

	    void
	    append_to_queue(
		  const IdV&
	    ) ;

	    void
	    append_to_queue(
		  const SQL::RowV&
	    ) ;

	    void
	    handle_info_bar_response(int) ;

	    void
	    save_xspf_real(
		  const std::string&
	    ) ;

	    void
	    handle_save_xspf() ;

	    void
	    handle_load_xspf() ;

	    void
	    handle_save_xspf_history() ;

	    void
	    handle_queue_op_artist(guint) ;

	    void
	    handle_queue_op_album(guint) ;

	    void
	    handle_show_play_queue_toggled() ;

	    void
	    handle_clear_play_queue() ;

	    void
	    handle_remove_track_from_queue(guint) ;

	    void
	    handle_follow_playing_track_toggled() ;

	    void
	    handle_model_changed(guint,bool) ;

#if 0
	    void
	    handle_playlist_selected(const Playlist_sp&) ;
#endif

	    bool
	    handle_keytimer() ;

	    void
	    handle_action_underline_matches() ;

	    void
	    handle_play_track_on_single_tap() ;

	    void
	    handle_nearest_clicked() ;	

	    bool
	    handle_search_entry_focus_out( GdkEventFocus* ) ;

            bool
            handle_search_entry_key_press_event( GdkEventKey* ) ;

            void
            handle_search_entry_changed() ;

            void
            handle_search_entry_activated() ;

            void
            handle_search_entry_icon_clicked(
		  Gtk::EntryIconPosition    = Gtk::ENTRY_ICON_SECONDARY
		, const GdkEventButton*	    = 0
            ) ;

	    void
	    handle_use_history_toggled(
	    ) ;

	    void
	    handle_shuffle_toggled(
	    ) ;

	    void
	    handle_display_album_info(
	    ) ;

        protected:

	    void
	    on_play_error( const std::string&, const std::string&, const std::string& ) ;	

            void
            on_play_eos(
		  bool /*no_markov*/ = false
            ) ;

            void
            on_play_seek(
                  guint
            ) ;

            void
            on_play_position(
                  guint
            ) ;

            void
            on_play_playstatus_changed(
            ) ;

            void
            on_play_stream_switched(        
            ) ;

	    void 
            on_play_stream_switched_real_idle(        
            ) ;

#if 0
            void
            on_play_metadata(        
                GstMetadataField
            ) ;
#endif

            void
            handle_tracklist_track_activated(
                  MPX::Track_sp     /*track*/
                , bool              /*play or not*/
		, bool		    /*next or not? only useful when play==false*/
            ) ;

            void
            handle_tracklist_find_propagate(
                  const std::string&
            ) ;

            void
            on_list_view_aa_selection_changed(
            ) ;

            void
            handle_albumlist_selection_changed(
            ) ;

            void
            handle_albumlist_select_album(
                const std::string&
            ) ;

            void
            handle_albumlist_select_artist(
                const std::string&
            ) ;

            void
            handle_albumlist_similar_artists(
                const std::string&
            ) ;
 
            void
            handle_albumlist_refetch_cover(
		guint
            ) ;

            void
            handle_tracklist_vadj_changed(
            ) ;

            void
            handle_albumlist_start_playback(
            ) ;

            void
            on_info_area_clicked(
		int area
            ) ;

            void
            on_show_subsystem_window(
		int n
            ) ;

            bool
            on_main_window_key_press_event_after(
                  GdkEventKey*
            ) ;

            void
            on_position_seek(
                  guint
            ) ;

            void
            on_volume_set_volume(
                  int
            ) ;

            bool
            on_status_icon_clicked(
            ) ;

            void
            on_status_icon_scroll_up(
            ) ;

            void
            on_status_icon_scroll_down(
            ) ;

        protected:

            void
            on_style_changed(
            ) ;

        protected:

            void
            play_track (
                  const MPX::Track_sp&
		, bool /*register_in_history*/ = true
            ) ;

        protected:

	    void
	    preload__artists() ;

	    void
	    preload__albums() ;
    
	    void
	    preload__tracks() ;

	    std::string
	    trie_find_ldmatch(
		const std::string&
	    ) ;

	    void
	    update_entry_placeholder_text() ;

	    void
	    infobar_set_message(
		  const std::string&
		, Gtk::MessageType
	    ) ;

	    //////////

            void
            on_library_scan_start() ;

            void
            on_library_scan_end() ;

	    struct NewAlbumFetchStruct ;

	    bool
	    new_album_idle( NewAlbumFetchStruct* ) ;

            void
            on_library_new_album(
                  guint
                , const std::string&
                , const std::string&
                , const std::string&
                , const std::string&
                , const std::string&
            ) ;

            void
            on_library_new_artist(
                  guint
            ) ;

            void
            on_library_new_track(
                  guint
            ) ;

	    bool
	    on_library_entity_deleted_idle( guint, int ) ;

            void
            on_library_entity_deleted(
                  guint
                , int
            ) ;

            void
            on_local_library_album_updated(
                  guint
            ) ;

            void
            on_covers_got_cover(
		  const MPX::RM::RequestQualifier&
		, MPX::RM::AlbumImage&
            ) ;

            void
            on_covers_no_cover(
                  guint
            ) ;

	    bool
	    on_library_entity_updated_idle(
		  guint
		, int 
	    ) ;

            void
            on_library_entity_updated(
                  guint
                , int
            ) ;

            void
            push_new_tracks(
            ) ;

            boost::shared_ptr<MPX::View::Albums::Album>
            get_album_from_id( guint, bool = false ) ;

	    void
	    register_played_track() ;

	    void
	    tracklist_regen_mapping() ;

	    void
	    tracklist_regen_mapping_iterative() ;

	protected:

	    void
	    assign_metadata_to_DBus_property() ;

        protected: // DBUS

            virtual void
            Raise(){ m_MainWindow->present(); } 

            virtual void
            Quit(){ }

	    virtual void
	    Next(){ API_next(); }

	    virtual void
	    Previous(){ API_prev(); }

	    virtual void
	    Pause(){}

	    virtual void
	    PlayPause(){ API_pause_toggle(); } 

	    virtual void
	    Stop(){ API_stop(); }

	    virtual void
	    Play(){}

	    virtual void
	    Seek( const int64_t& s ){ on_position_seek( s/1000 ); }

/*
	    virtual void
	    SetPosition( const DBus::Path&, const int64_t& ){} 
*/

	    virtual void
	    OpenUri( const std::string& ){}
    } ;
}

#endif // YOUKI_CONTROLLER_HH
