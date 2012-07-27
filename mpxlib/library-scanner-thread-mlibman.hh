//
// library-scanner-thread
//
// Authors:
//     Milosz Derezynski <milosz@backtrace.info>
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
#ifndef MPX_LIBRARY_SCANNER_THREAD_MLIBMAN_HH
#define MPX_LIBRARY_SCANNER_THREAD_MLIBMAN_HH

#include <config.h>
#include <glibmm.h>
#include <sigx/sigx.h>
#include <sigx/signal_f.h>
#include <sigx/request_f.h>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <tuple>
#if 0
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#endif
#include <tuple>
#include <boost/optional.hpp>
#include "mpx/mpx-main.hh"
#include "mpx/mpx-sql.hh"
#include "mpx/mpx-types.hh"
#include "mpx/util-string.hh"
#include "mpx/util-file.hh"

#ifdef HAVE_HAL
#include "mpx/i-youki-hal.hh"
#endif // HAVE_HAL

namespace MPX
{
    typedef boost::optional<std::string>			OString_t ;
    typedef std::tuple<guint, OVariant, OVariant, OVariant>	AlbumIDCached_t ;
    typedef std::map<AlbumIDCached_t, guint>			IDCacheAlbum_t ;
}

namespace MPX
{
    enum ScanResult
    {
          SCAN_RESULT_OK
        , SCAN_RESULT_UPDATE
        , SCAN_RESULT_UPTODATE
    };

    enum EntityType
    {
          ENTITY_TRACK
        , ENTITY_ALBUM
        , ENTITY_ARTIST
        , ENTITY_ALBUM_ARTIST
    };

    typedef std::pair<std::string, std::string> SSFileInfo;

    struct ScanSummary
    {
        guint  FilesAdded;
        guint  FilesErroneous;
        guint  FilesUpToDate;
        guint  FilesUpdated;
        guint  FilesTotal;

        std::vector<SSFileInfo> FileListErroneous;
        std::vector<SSFileInfo> FileListUpdated;

        ScanSummary ()
        : FilesAdded(0)
        , FilesErroneous(0)
        , FilesUpToDate(0)
        , FilesUpdated(0)
        , FilesTotal(0)
        {
        }
    };

#include "mpx/exception.hh"

    EXCEPTION(ScanError)

    class IHAL ;
    class Library_MLibMan ;
    class MetadataReaderTagLib ;

	class LibraryScannerThread_MLibMan : public sigx::glib_threadable
	{
        public:

            typedef sigc::signal<void>                              SignalScanStart_t ;
            typedef sigc::signal<void>                              SignalScanEnd_t ;
            typedef sigc::signal<void, ScanSummary const&>          SignalScanSummary_t ;
            typedef sigc::signal<void, guint, std::string, std::string, std::string, std::string, std::string> SignalNewAlbum_t ;
            typedef sigc::signal<void, guint>                      SignalNewArtist_t ;
            typedef sigc::signal<void, guint>                      SignalNewTrack_t ;
            typedef sigc::signal<void, guint, EntityType>          SignalEntityDeleted_t ;
            typedef sigc::signal<void, guint, EntityType>          SignalEntityUpdated_t ;
            typedef sigc::signal<void>                              SignalReload_t ;
            typedef sigc::signal<void, const std::string&>          SignalMessage_t ;
            
            typedef sigx::signal_f<SignalScanStart_t>               signal_scan_start_x ;
            typedef sigx::signal_f<SignalScanEnd_t>                 signal_scan_end_x ;
            typedef sigx::signal_f<SignalScanSummary_t>             signal_scan_summary_x ;
            typedef sigx::signal_f<SignalNewAlbum_t>                signal_new_album_x ;
            typedef sigx::signal_f<SignalNewArtist_t>               signal_new_artist_x ;
            typedef sigx::signal_f<SignalNewTrack_t>                signal_new_track_x ;
            typedef sigx::signal_f<SignalEntityDeleted_t>           signal_entity_deleted_x ;
            typedef sigx::signal_f<SignalEntityUpdated_t>           signal_entity_updated_x ;
            typedef sigx::signal_f<SignalReload_t>                  signal_reload_x ;
            typedef sigx::signal_f<SignalMessage_t>                 signal_message_x ;

        public:

            sigx::request_f<Util::FileList const&>                          add ;
            sigx::request_f<Util::FileList const&>                          scan ;
            sigx::request_f<>                                               scan_all ;
            sigx::request_f<>                                               scan_stop ;
            sigx::request_f<>                                               vacuum ;
            sigx::request_f<const std::vector<std::string>&, bool, bool>    set_priority_data ;
#ifdef HAVE_HAL
            sigx::request_f<const VolumeKey_v&>                        vacuum_volume_list ;
#endif // HAVE_HAL
            sigx::request_f<>                                               update_statistics ;

            signal_scan_start_x             signal_scan_start ;
            signal_scan_end_x               signal_scan_end ;
            signal_scan_summary_x           signal_scan_summary ;
            signal_new_album_x              signal_new_album ;
            signal_new_artist_x             signal_new_artist ;
            signal_new_track_x              signal_new_track ;
            signal_entity_deleted_x         signal_entity_deleted ;
            signal_entity_updated_x         signal_entity_updated ;
            signal_reload_x                 signal_reload ;
            signal_message_x                signal_message ;

            struct ScannerConnectable
            {
                ScannerConnectable(
                      signal_scan_start_x&            start_x
                    , signal_scan_end_x&              end_x
                    , signal_scan_summary_x&          summary_x
                    , signal_new_album_x&             album_x
                    , signal_new_artist_x&            artist_x
                    , signal_new_track_x&             track_x
                    , signal_entity_deleted_x&        entity_deleted_x
                    , signal_entity_updated_x&        entity_updated_x
                    , signal_reload_x&                reload_x
                    , signal_message_x&               message_x 
                )
                : signal_scan_start(start_x)
                , signal_scan_end(end_x)
                , signal_scan_summary(summary_x)
                , signal_new_album(album_x)
                , signal_new_artist(artist_x)
                , signal_new_track(track_x)
                , signal_entity_deleted(entity_deleted_x)
                , signal_entity_updated(entity_updated_x)
                , signal_reload(reload_x)
                , signal_message(message_x)
                {
                }

                signal_scan_start_x         & signal_scan_start ;
                signal_scan_end_x           & signal_scan_end ;
                signal_scan_summary_x       & signal_scan_summary ;
                signal_new_album_x          & signal_new_album ;
                signal_new_artist_x         & signal_new_artist ;
                signal_new_track_x          & signal_new_track ;
                signal_entity_deleted_x     & signal_entity_deleted ;
                signal_entity_updated_x     & signal_entity_updated ;
                signal_reload_x             & signal_reload ;
                signal_message_x            & signal_message ;
            };

            LibraryScannerThread_MLibMan(
                MPX::Library_MLibMan*,
                guint
            ) ;

            virtual ~LibraryScannerThread_MLibMan () ;

            ScannerConnectable&
            connect ();

	    void /* sync tunnel */
	    remove_dangling() ;

        protected:

            virtual void on_startup () ; 
            virtual void on_cleanup () ;

// Requests /////////////

            void on_cancel(
            ) ;

            void on_scan(
                const Util::FileList&
            ) ;

            void on_scan_all(
            ) ;

            void on_scan_stop() ;

            void on_vacuum ();

#ifdef HAVE_HAL
            void on_vacuum_volume_list(
                  const VolumeKey_v&
                , bool = true
            );
#endif // HAVE_HAL

            void on_update_statistics ();

/////////////////////////

            void on_add(
                  const Util::FileList&
                , bool = false
            );

            void on_scan_list_quick_stage_1(
                  const Util::FileList&
            ); // on_scan delegate

            guint
            get_track_mtime(
                Track&
            ) const;

            guint
            get_track_id(
                Track&
            ) const;

            /// INSERTION

            enum EntityIsNew
            {
                  ENTITY_IS_NEW
                , ENTITY_IS_NOT_NEW
                , ENTITY_IS_UNDEFINED
            };

            typedef std::pair<guint, EntityIsNew> EntityInfo;
 
            EntityInfo 
            get_track_artist_id(
                Track&,
                bool = false
            );

            EntityInfo 
            get_album_artist_id(
                Track&,
                bool = false
            );

            EntityInfo 
            get_album_id(
                Track&,
                guint,
                bool = false
            );

            typedef boost::shared_ptr<Track>        Track_p;

            struct TrackInfo
            {
                EntityInfo      Artist;
                EntityInfo      Album;
                EntityInfo      AlbumArtist;

                guint          TrackNumber;
                std::string     Title;
                std::string     Type;

                bool            Update;

                Track_p         Track;
            };
 
            typedef boost::shared_ptr<TrackInfo>    TrackInfo_p;
            typedef std::vector<TrackInfo_p>        TrackInfo_v_sp;

#if 0
            typedef std::map<guint,       TrackInfo_v_sp>   Map_L4;
            typedef std::map<std::string, Map_L4>               Map_L3;
            typedef std::map<guint,       Map_L3>               Map_L2;
            typedef std::map<guint,       Map_L2>               Map_L1;
#endif

            //typedef std::map<guint , std::map<guint, std::map<std::string, TrackInfo_v_sp> > > InsertionTracks_t;

	    typedef std::tuple<guint, guint, std::string, guint> InsertionKey_t ; 
	    typedef std::map<InsertionKey_t, TrackInfo_v_sp> InsertionMap_t ;

//	    InsertionMap_t m_InsertionTracks ;
            // Map_L1      m_InsertionTracks;
    
	    TrackInfo_v_sp m_InsertionTracks ;

            typedef std::tuple<guint, std::string>    FileDuplet_t;
            typedef std::map<FileDuplet_t, time_t>    Duplet_MTIME_t;
            typedef std::unordered_set<guint>	      IdSet_t;

            IdSet_t                                 m_AlbumIDs, m_AlbumArtistIDs; 
            IdSet_t                                 m_ProcessedAlbums;
            Duplet_MTIME_t                          m_MTIME_Map;

            void
            cache_mtimes(
            ) ;

            void
            insert_file_no_mtime_check(
                  Track_sp           track
                , const std::string& uri
                , const std::string& insert_path
                , bool               update_track = false
            );

            void
            create_insertion_track(
                  Track_sp&          track
                , const std::string& uri
                , const std::string& insert_path
                , bool               update
            );
    
            void
            process_insertion_list();

	    bool
	    insert_idle(
	    ) ;

            void
            add_erroneous_track(
                  const std::string& uri
                , const std::string& info
            );
    
            typedef std::vector<std::string>        MIME_Types_t;

            MIME_Types_t m_MIME_Types;
            bool         m_PrioritizeByFileType;
            bool         m_PrioritizeByBitrate;

            void
            on_set_priority_data(
                  const std::vector<std::string>&
                , bool
                , bool
            ) ;

            TrackInfo_p
            prioritize(
                  const TrackInfo_v_sp&
            ) ;

            ScanResult
            insert(
                  const TrackInfo_p&
            ) ;

            void
            signal_new_entities(
                  const TrackInfo_p&
            ) ;

            int
            compare_types(
                  const std::string&
                , const std::string&
            ) ;

            std::string
            create_insertion_sql(
                  const Track&
                , guint
                , guint
            ) ;


            std::string
            create_update_sql(
                  const Track&
                , guint
                , guint
            ) ;

            void
            update_album(
                guint
            ) ;

            void
            update_albums(
            ) ;

            void
            do_remove_dangling() ;

            bool
            check_abort_scan( 
            ) ;

            void
            quarantine_file(
                  const std::string&
            ) ;

        private:

	    IDCacheAlbum_t			    m_AlbumIDCache ;

            ScannerConnectable                    * m_Connectable ;
            struct ThreadData;
            Glib::Private<ThreadData>               m_ThreadData ;
            MPX::Library_MLibMan                  & m_Library_MLibMan;
	    MPX::MetadataReaderTagLib		  & m_TagLib ;
            boost::shared_ptr<MPX::SQL::SQLDB>      m_SQL ;
#ifdef HAVE_HAL
            const IHAL                            & m_HAL ;
#endif // HAVE_HAL
            guint                                   m_Flags ;
            ScanSummary                             m_ScanSummary ;


	private:
    
	    void
	    insert_mb_album_tuple(
		 SQL::Row&
	    ) ;
	    
    } ;
}

#endif
