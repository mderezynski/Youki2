#ifndef YOUKI_CONTROLLER_HH
#define YOUKI_CONTROLLER_HH

#include "config.h"
#include "mpx/i-youki-controller.hh"

#include <gtkmm.h>
#include <list>
#include <unordered_map>

#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>

#include "mpx/mpx-services.hh"
#include "mpx/mpx-types.hh"

#include "mpx/util-string.hh"

#include "kobo-main.hh"
#include "kobo-position.hh"
#include "kobo-volume.hh"

#include "youki-spectrum-titleinfo.hh"
#include "mpx-mlibman-dbus-proxy-actual.hh"

#include "ld-find-nearest.hh"
#include "id-history.hh"

#include <sigx/sigx.h>

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

    class PercentualDistributionHBox ;

    class YoukiController
    : public IYoukiController
    , public sigx::glib_auto_dispatchable
    , public Service::Base
    {
        public: 

            typedef sigc::signal<void>          Signal0_void ;

	    struct HistoryItem
	    {
		boost::optional<std::string>	Searchtext ;

		boost::optional<guint>		AlbumArtistId ;
		boost::optional<guint>		AlbumId ;
            } ;

	    typedef std::list<HistoryItem>	History_t ;
	    typedef History_t::iterator		HistoryPosition_t ;

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
            MainWindow                      * m_main_window ;

            int                               m_main_window_x 
                                            , m_main_window_y ;
            
            YoukiSpectrumTitleinfo          * m_main_info ;
            KoboPosition                    * m_main_position ;
            KoboVolume                      * m_main_volume ;

            View::Artist::Class             * m_ListViewArtist ;
            View::Albums::Class             * m_ListViewAlbums ;
            View::Tracks::Class             * m_ListViewTracks ;

            Gtk::ScrolledWindow             * m_ScrolledWinArtist ;
            Gtk::ScrolledWindow             * m_ScrolledWinAlbums ;
            Gtk::ScrolledWindow             * m_ScrolledWinTracks ;

            Gtk::HPaned                     * m_Paned1 ;
            Gtk::HPaned                     * m_Paned2 ;

            Gtk::HBox			    * m_HBox_Main ;
            Gtk::HBox                       * m_HBox_Bottom ;
            Gtk::VBox                       * m_VBox_Bottom ;
	    Gtk::VBox			    * m_VBox_TL ;
	    Gtk::Label			    * m_Label_TL ;

            Gtk::Entry                      * m_Entry ;

	    Gtk::RadioButton		    * m_rb1, * m_rb2 ;

            Gtk::Alignment                  * m_Alignment_Entry ;
            Gtk::HBox                       * m_HBox_Entry ;
            Gtk::HBox                       * m_HBox_Info ;
            Gtk::HBox                       * m_HBox_Controls ;

            Gtk::Label                      * m_Label_Search ;
	    Gtk::Label			    * m_Label_Nearest ;

            Gtk::VBox                       * m_VBox ;
            Gtk::Notebook                   * m_NotebookPlugins ;
	    Gtk::Image			    * m_AQUE_Spinner ;
	    Gtk::EventBox		    * m_EventBox_Nearest ;

            Covers                          * m_covers ;
            MPX::Play                       * m_play ;
            Library                         * m_library ;
	    LDFindNearest		      m_find_nearest_artist ;
	    LDFindNearest		      m_find_nearest_artist_full ;
	    LDFindNearest		      m_find_nearest_artist_full_lc ;
	    std::string			      m_nearest ;
	    StrV			      m_nearest__artists ;
	    StrV			      m_nearest__artists_popular ;

	    typedef boost::shared_ptr<LDFindNearest> LDFN_p ;
	    typedef std::map<std::string, LDFN_p>    StrLDFN_map_t ;
	    typedef std::set<std::string>	     StrSet_t ;
	    typedef std::map<std::string, StrSet_t>  StrSet_map_t ;

	    StrSet_map_t		      m_ssmap ;
	    StrLDFN_map_t		      m_ldmap ; 
    
            Track_sp                          m_track_current ;          
            Track_sp                          m_track_previous ;          
            boost::optional<guint>            m_seek_position ;
            bool                              m_follow_track ;

            std::list<guint>                  m_play_queue ;
	    IdHistory			      m_play_history ;

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

	    Glib::RefPtr<Gtk::UIManager>      m_UI_Manager ;
	    Glib::RefPtr<Gtk::ActionGroup>    m_UI_Actions_Main ;

	    void
	    handle_show_queue_toggled() ;

	    void
	    handle_clear_queue() ;

	    void
	    handle_remove_track_from_queue(guint) ;

	    void
	    handle_follow_playing_track_toggled() ;

	    void
	    handle_model_changed(guint,bool) ;

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
            handle_search_entry_clear_clicked(
		  Gtk::EntryIconPosition    = Gtk::ENTRY_ICON_SECONDARY
		, const GdkEventButton*	    = 0
            ) ;

	    void
	    handle_use_history_toggled(
	    ) ;

	    void
	    handle_shuffle_toggled(
	    ) ;

        protected:

	    void
	    on_play_error( const std::string&, const std::string&, const std::string& ) ;	

            void
            on_play_eos(
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
            on_play_metadata(        
                GstMetadataField
            ) ;

            void
            handle_tracklist_track_activated(
                  MPX::Track_sp     /*track*/
                , bool              /*play or not*/
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
            on_advanced_changed(
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
                  guint
            ) ;

            void
            on_covers_no_cover(
                  guint
            ) ;

	    bool
	    on_library_entity_updated_idle( guint, int ) ;

            void
            on_library_entity_updated(
                  guint
                , int
            ) ;

	    void
	    play_next_queue_item() ;

            void
            push_new_tracks(
            ) ;

            boost::shared_ptr<MPX::View::Albums::Album>
            get_album_from_id( guint ) ;

            void
	    history_save() ;

	    void
	    history_load_current_iter() ;

	    void
	    history_go_back() ;

	    void
	    history_go_ffwd() ;

	    History_t m_HISTORY ;
            HistoryPosition_t m_HISTORY_POSITION ;	   
	    bool m_view_changed ;

	    Gtk::Button* m_BTN_HISTORY_PREV ;
	    Gtk::Button* m_BTN_HISTORY_FFWD ;
	    Gtk::ToggleButton* m_BTN_SHUFFLE ;

	    bool m_history_ignore_save ;
	
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
            Raise(){ m_main_window->present(); } 

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
