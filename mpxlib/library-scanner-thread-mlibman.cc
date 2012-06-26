#include "config.h"

#include <queue>
#include <boost/ref.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <giomm.h>
#include <glibmm.h>
#include <glibmm/i18n.h>

#include <set>

#include "mpx/mpx-sql.hh"
#include "mpx/mpx-types.hh"
#include "mpx/mpx-uri.hh"

#include "mpx/util-file.hh"

#include "mpx/metadatareader-taglib.hh"

#include "mpx/xml/xmltoc++.hh"
#include "xmlcpp/mb-artist-basic-1.0.hxx"

#include "library-scanner-thread-mlibman.hh"
#include "library-mlibman.hh"

#ifdef HAVE_HAL
#include "mpx/i-youki-hal.hh"
#endif // HAVE_HAL

using boost::get ;
using namespace MPX ;
using namespace MPX::SQL ;
using namespace Glib ;

namespace
{
    struct AttrInfo
    {
        char const* id ;
        ValueType   type ;
    } ;

    AttrInfo attrs[] =
    {
            {   "location",
                    VALUE_TYPE_STRING   }, 

            {   "title",
                    VALUE_TYPE_STRING   }, 

            {   "genre",
                    VALUE_TYPE_STRING   }, 

            {   "comment",
                    VALUE_TYPE_STRING   }, 

            {   "label",
                    VALUE_TYPE_STRING   }, 

            {   "musicip_puid",
                    VALUE_TYPE_STRING   }, 

            {   "hash",
                    VALUE_TYPE_STRING   }, 

            {   "mb_track_id",
                    VALUE_TYPE_STRING   }, 

            {   "artist",
                    VALUE_TYPE_STRING   }, 

            {   "artist_sortname",
                    VALUE_TYPE_STRING   }, 

            {   "mb_artist_id",
                    VALUE_TYPE_STRING   }, 

            {   "album",
                    VALUE_TYPE_STRING   }, 

            {   "mb_album_id",
                    VALUE_TYPE_STRING   }, 

            {   "mb_release_date",
                    VALUE_TYPE_STRING   }, 

            {   "mb_release_country",
                    VALUE_TYPE_STRING   }, 

            {   "mb_release_type",
                    VALUE_TYPE_STRING   }, 

            {   "amazon_asin",
                    VALUE_TYPE_STRING   }, 

            {   "album_artist",
                    VALUE_TYPE_STRING   }, 

            {   "album_artist_sortname",
                    VALUE_TYPE_STRING   }, 

            {   "mb_album_artist_id",
                    VALUE_TYPE_STRING   }, 

            {   "type",
                    VALUE_TYPE_STRING   }, 

            {   "hal_volume_udi",
                    VALUE_TYPE_STRING   }, 

            {   "hal_device_udi",
                    VALUE_TYPE_STRING   }, 

            {   "hal_vrp",
                    VALUE_TYPE_STRING   }, 

            {   "insert_path",
                    VALUE_TYPE_STRING   }, 

            {   "location_name",
                    VALUE_TYPE_STRING   }, 

            {   "disctotal",
                    VALUE_TYPE_INT      },

            {   "discnr",
                    VALUE_TYPE_INT      },

            {   "discs",
                    VALUE_TYPE_INT      },

            {   "track",
                    VALUE_TYPE_INT      },

            {   "time",
                    VALUE_TYPE_INT      },

            {   "rating",
                    VALUE_TYPE_INT      },

            {   "date",
                    VALUE_TYPE_INT      },

            {   "mtime",
                    VALUE_TYPE_INT      },

            {   "bitrate",
                    VALUE_TYPE_INT      },

            {   "samplerate",
                    VALUE_TYPE_INT      },

            {   "pcount",
                    VALUE_TYPE_INT      },

            {   "pdate",
                    VALUE_TYPE_INT      },

            {   "insert_date",
                    VALUE_TYPE_INT      },

            {   "is_mb_album_artist",
                    VALUE_TYPE_INT      },

            {   "active",
                    VALUE_TYPE_INT      },

            {   "audio_quality",
                    VALUE_TYPE_INT      },

            {   "device_id",
                    VALUE_TYPE_INT      },

            {   "loved",
                    VALUE_TYPE_INT      },

            {   "hated",
                    VALUE_TYPE_INT      },

    } ;

    const char delete_track_f[] = "DELETE FROM track WHERE id = '%u';" ;

    enum Quality
    {
          QUALITY_ABYSMAL
        , QUALITY_LOW
        , QUALITY_MED
        , QUALITY_HI
    } ;

    struct BitrateLookupTable
    {
        const char*     Type ;
        int             Range0 ;
        int             Range1 ;
        int             Range2 ;
    } ;

    BitrateLookupTable table[]=
    {
          {"audio/x-flac",          320, 640, 896 }
        , {"audio/x-ape",           256, 512, 768 }
        , {"audio/x-vorbis+ogg",     32,  96, 192 }
        , {"audio/x-musepack",      128, 192, 256 }
        , {"audio/mp4",              24,  64, 128 }
        , {"audio/mpeg",             96, 192, 256 }
        , {"audio/x-ms-wma",         48,  96, 196 }
    } ;

    guint
    get_audio_quality(
          const std::string& type
        , guint             rate
    )
    {
        for( unsigned n = 0; n < G_N_ELEMENTS(table); ++n )
        {
            if( type == table[n].Type )
            {
                if( rate < table[n].Range0 )
                    return QUALITY_ABYSMAL ;
                else if( rate < table[n].Range1 )
                    return QUALITY_LOW ;
                else if( rate < table[n].Range2 )
                    return QUALITY_MED ;
                else
                    return QUALITY_HI ;
            }
        }

        return 0 ;
    }

    template <typename T>
    void
    append_value(
          std::string&      sql
        , const Track&      track
        , unsigned int      attr
    )
    { 
        throw ;
    }

    template <>
    void
    append_value<std::string>(
          std::string&      sql
        , const Track&      track
        , unsigned int      attr
    )
    {
        sql += mprintf( "'%q'", get<std::string>(track[attr].get()).c_str())  ;
    }

    template <>
    void
    append_value<guint>(
          std::string&      sql
        , const Track&      track
        , unsigned int      attr
    )
    {
        sql += mprintf( "'%u'", get<guint>(track[attr].get()) )  ;
    }

    template <>
    void
    append_value<double>(
          std::string&      sql
        , const Track&      track
        , unsigned int      attr
    )
    {
        sql += mprintf( "'%f'", get<double>(track[attr].get()))  ;
    }



    template <typename T>
    void
    append_key_value_pair(
          std::string&      sql
        , const Track&      track
        , unsigned int      attr
    )
    { 
        throw ;
    }

    template <>
    void
    append_key_value_pair<std::string>(
          std::string&      sql
        , const Track&      track
        , unsigned int      attr
    )
    {
        sql += mprintf( "%s = '%q'", attrs[attr].id, get<std::string>(track[attr].get()).c_str())  ;
    }

    template <>
    void
    append_key_value_pair<guint>(
          std::string&      sql
        , const Track&      track
        , unsigned int      attr
    )
    {
        sql += mprintf( "%s = '%u'", attrs[attr].id, get<guint>(track[attr].get()) )  ;
    }

    template <>
    void
    append_key_value_pair<double>(
          std::string&      sql
        , const Track&      track
        , unsigned int      attr
    )
    {
        sql += mprintf( "%s = '%f'", attrs[attr].id, get<double>(track[attr].get()))  ;
    }

    std::string
    get_sortname(
          const std::string& album_artist
	, const std::string& album_artist_sortname 
    )
    {
        std::string name = album_artist ;
        gunichar c = Glib::ustring(album_artist)[0] ; 

        if( g_unichar_get_script( c ) != G_UNICODE_SCRIPT_LATIN ) 
        {
                    std::string in = album_artist_sortname ;

                    boost::iterator_range <std::string::iterator> match1 = boost::find_nth( in, ", ", 0 ) ;
                    boost::iterator_range <std::string::iterator> match2 = boost::find_nth( in, ", ", 1 ) ;

                    if( !match1.empty() && match2.empty() ) 
                    {
                        name = std::string (match1.end(), in.end()) + " " + std::string (in.begin(), match1.begin());
                    }
                    else
                    {
                        name = in ;
                    }
        }

        return name ;
    }


}

struct MPX::LibraryScannerThread_MLibMan::ThreadData
{
    LibraryScannerThread_MLibMan::SignalScanStart_t         ScanStart  ;
    LibraryScannerThread_MLibMan::SignalScanEnd_t           ScanEnd  ;
    LibraryScannerThread_MLibMan::SignalScanSummary_t       ScanSummary  ;
    LibraryScannerThread_MLibMan::SignalNewAlbum_t          NewAlbum  ;
    LibraryScannerThread_MLibMan::SignalNewArtist_t         NewArtist  ;
    LibraryScannerThread_MLibMan::SignalNewTrack_t          NewTrack  ;
    LibraryScannerThread_MLibMan::SignalEntityDeleted_t     EntityDeleted  ;
    LibraryScannerThread_MLibMan::SignalEntityUpdated_t     EntityUpdated  ;
    LibraryScannerThread_MLibMan::SignalReload_t            Reload  ;
    LibraryScannerThread_MLibMan::SignalMessage_t           Message  ;

    int m_Cancel  ;
} ;

MPX::LibraryScannerThread_MLibMan::LibraryScannerThread_MLibMan(
    MPX::Library_MLibMan*       obj_library
  , guint                      flags
)
: sigx::glib_threadable()
, add(sigc::bind(sigc::mem_fun(*this, &LibraryScannerThread_MLibMan::on_add), false))
, scan(sigc::mem_fun(*this, &LibraryScannerThread_MLibMan::on_scan))
, scan_all(sigc::mem_fun(*this, &LibraryScannerThread_MLibMan::on_scan_all))
, scan_stop(sigc::mem_fun(*this, &LibraryScannerThread_MLibMan::on_scan_stop))
, vacuum(sigc::mem_fun(*this, &LibraryScannerThread_MLibMan::on_vacuum))
, set_priority_data(sigc::mem_fun(*this, &LibraryScannerThread_MLibMan::on_set_priority_data))
#ifdef HAVE_HAL
, vacuum_volume_list(sigc::bind(sigc::mem_fun(*this, &LibraryScannerThread_MLibMan::on_vacuum_volume_list), true))
#endif // HAVE_HAL
, update_statistics(sigc::mem_fun(*this, &LibraryScannerThread_MLibMan::on_update_statistics))
, signal_scan_start(*this, m_ThreadData, &ThreadData::ScanStart)
, signal_scan_end(*this, m_ThreadData, &ThreadData::ScanEnd)
, signal_scan_summary(*this, m_ThreadData, &ThreadData::ScanSummary)
, signal_new_album(*this, m_ThreadData, &ThreadData::NewAlbum)
, signal_new_artist(*this, m_ThreadData, &ThreadData::NewArtist)
, signal_new_track(*this, m_ThreadData, &ThreadData::NewTrack)
, signal_entity_deleted(*this, m_ThreadData, &ThreadData::EntityDeleted)
, signal_entity_updated(*this, m_ThreadData, &ThreadData::EntityUpdated)
, signal_reload(*this, m_ThreadData, &ThreadData::Reload)
, signal_message(*this, m_ThreadData, &ThreadData::Message)
, m_Library_MLibMan(*obj_library)
, m_SQL(new SQL::SQLDB(*((m_Library_MLibMan.get_sql_db()))))
#ifdef HAVE_HAL
, m_HAL( *(services->get<IHAL>("mpx-service-hal").get()) )
#endif // HAVE_HAL
, m_Flags(flags)
{
    m_Connectable =
        new ScannerConnectable(
                  signal_scan_start
                , signal_scan_end
                , signal_scan_summary
                , signal_new_album
                , signal_new_artist
                , signal_new_track
                , signal_entity_deleted
                , signal_entity_updated
                , signal_reload
                , signal_message
    ); 
}

MPX::LibraryScannerThread_MLibMan::~LibraryScannerThread_MLibMan ()
{
    delete m_Connectable ;
}

MPX::LibraryScannerThread_MLibMan::ScannerConnectable&
MPX::LibraryScannerThread_MLibMan::connect ()
{
    return *m_Connectable ;
} 

void
MPX::LibraryScannerThread_MLibMan::on_startup ()
{
    m_ThreadData.set(new ThreadData) ;
}

void
MPX::LibraryScannerThread_MLibMan::on_cleanup ()
{
}

bool
MPX::LibraryScannerThread_MLibMan::check_abort_scan ()
{
    ThreadData * pthreaddata = m_ThreadData.get() ;

    if( g_atomic_int_get(&pthreaddata->m_Cancel) )
    {
        m_InsertionTracks.clear() ;
        m_AlbumIDs.clear() ;
        m_AlbumArtistIDs.clear() ;
        m_ProcessedAlbums.clear() ;

        pthreaddata->ScanSummary.emit( m_ScanSummary ) ;
        pthreaddata->ScanEnd.emit() ;
        return true ;
    }

    return false ;
}

void
MPX::LibraryScannerThread_MLibMan::on_set_priority_data(
      const std::vector<std::string>&   types
    , bool                              prioritize_by_filetype
    , bool                              prioritize_by_bitrate
)
{
    m_MIME_Types = types; 
    m_PrioritizeByFileType = prioritize_by_filetype ;
    m_PrioritizeByBitrate  = prioritize_by_bitrate ;
}

void
MPX::LibraryScannerThread_MLibMan::cache_mtimes(
)
{
    m_MTIME_Map.clear() ;

    RowV v ;
    m_SQL->get(
          v
        , "SELECT device_id, hal_vrp, mtime FROM track"
    ) ;

    for( RowV::iterator i = v.begin(); i != v.end(); ++i )
    {
        m_MTIME_Map.insert(
            std::make_pair(
                  boost::make_tuple(
                      get<guint>((*i)["device_id"])
                    , get<std::string>((*i)["hal_vrp"])
                  )
                , get<guint>((*i)["mtime"])
        )) ;
    }
}

void
MPX::LibraryScannerThread_MLibMan::on_scan(
    const Util::FileList& list
)
{
    if(list.empty())
    {
        return ;
    }

    on_scan_list_quick_stage_1( list ); 
    on_add( list, true ); 
}

void
MPX::LibraryScannerThread_MLibMan::on_scan_all(
)
{
    ThreadData * pthreaddata = m_ThreadData.get() ;

    g_atomic_int_set(&pthreaddata->m_Cancel, 0) ;

    pthreaddata->ScanStart.emit() ;

    m_ScanSummary = ScanSummary() ;

    boost::shared_ptr<Library_MLibMan> library = services->get<Library_MLibMan>("mpx-service-library") ;

    RowV v ;

#ifdef HAVE_HAL
    try{
        if (m_Flags & Library_MLibMan::F_USING_HAL)
        { 
            m_SQL->get(
                v,
                "SELECT id, device_id, hal_vrp, mtime, insert_path FROM track"
            ) ;
        }
        else
#endif
        {
            m_SQL->get(
                v,
                "SELECT id, location, mtime, insert_path FROM track"
            ) ;
        }
#ifdef HAVE_HAL
    }
    catch( IHAL::Exception& cxe )
    {
        g_warning( "%s: %s", G_STRLOC, cxe.what() ); 
        return ;
    }
    catch( Glib::ConvertError& cxe )
    {
        g_warning( "%s: %s", G_STRLOC, cxe.what().c_str() ); 
        return ;
    }
#endif // HAVE_HAL

    for( RowV::iterator i = v.begin(); i != v.end(); ++i )
    {
        Track_sp t = library->sqlToTrack( *i, false ); 

        std::string location = get<std::string>( (*t)[ATTRIBUTE_LOCATION].get() ) ; 

        guint mtime1 = Util::get_file_mtime( get<std::string>( (*t)[ATTRIBUTE_LOCATION].get() ) ) ;

        if( mtime1 )
        {
            guint mtime2 = get<guint>((*i)["mtime"]) ;

            if( mtime2 && mtime1 == mtime2 )
            {
                ++m_ScanSummary.FilesUpToDate ;
            }
            else
            {
                (*t)[ATTRIBUTE_MTIME] = mtime1 ;
                insert_file_no_mtime_check( t, location, get<std::string>((*i)["insert_path"]), mtime2 && mtime1 != mtime2 ) ; 
            }
        }
        else
        {
            guint id = get<guint>((*i)["id"]) ;
            m_SQL->exec_sql(mprintf( delete_track_f, id)) ;
            pthreaddata->EntityDeleted.emit( id , ENTITY_TRACK ); 
        }


        if( check_abort_scan() )
            return ;

        m_ScanSummary.FilesTotal ++  ;

        if( !(m_ScanSummary.FilesTotal % 100) )
        {
                pthreaddata->Message.emit(
                    (boost::format(_("Checking Tracks: %u"))
                        % guint(m_ScanSummary.FilesTotal)
                    ).str()
                ) ;
        }
    }

    process_insertion_list () ;
    //do_remove_dangling () ;
    update_albums () ;

    pthreaddata->Message.emit(_("Rescan: Done")) ;
    pthreaddata->ScanSummary.emit( m_ScanSummary ) ;
    pthreaddata->ScanEnd.emit() ;
}

void
MPX::LibraryScannerThread_MLibMan::on_add(
      const Util::FileList& list
    , bool                  check_mtimes
)
{
    ThreadData * pthreaddata = m_ThreadData.get() ;

    if( check_mtimes )
    {
        cache_mtimes() ;
    }
    else
    {
        pthreaddata->ScanStart.emit() ;
    }

    g_atomic_int_set(&pthreaddata->m_Cancel, 0) ;

    RowV rows;
    m_SQL->get(rows, "SELECT last_scan_date FROM meta WHERE rowid = 1") ;
    guint last_scan_date = boost::get<guint>(rows[0]["last_scan_date"]) ;

    m_ScanSummary = ScanSummary() ;

    boost::shared_ptr<Library_MLibMan> library = services->get<Library_MLibMan>("mpx-service-library") ;

    Util::FileList collection ;

    for(Util::FileList::const_iterator i = list.begin(); i != list.end(); ++i)
    {  
        std::string insert_path  ;
        std::string insert_path_sql  ;
        Util::FileList collection ;

        // Collection from Filesystem

#ifdef HAVE_HAL
        try{
#endif // HAVE_HAL

            insert_path = Util::normalize_path( *i )  ;

#ifdef HAVE_HAL
            try{
                if (m_Flags & Library_MLibMan::F_USING_HAL)
                { 
                    const Volume&   volume          = m_HAL.get_volume_for_uri( *i ) ;
                                    insert_path_sql = Util::normalize_path(Glib::filename_from_uri(*i).substr (volume.mount_point.length())) ;
                }
                else
#endif // HAVE_HAL

                {
                                    insert_path_sql = insert_path ;
                }

#ifdef HAVE_HAL
            }
            catch(
              IHAL::Exception&      cxe
            )
            {
                g_warning( "%s: %s", G_STRLOC, cxe.what() ); 
                continue ;
            }
            catch(
              Glib::ConvertError&   cxe
            )
            {
                g_warning( "%s: %s", G_STRLOC, cxe.what().c_str() ); 
                continue ;
            }
#endif // HAVE_HAL

            collection.clear() ;

            try{
                Util::collect_audio_paths_recursive( insert_path, collection ) ;
            } catch(...) {}

            m_ScanSummary.FilesTotal += collection.size() ;

#ifdef HAVE_HAL
        }
        catch( Glib::ConvertError & cxe )
        {
            g_warning("%s: %s", G_STRLOC, cxe.what().c_str()) ;
            continue ;
        }
#endif // HAVE_HAL

        // Collect + Process Tracks

        for( Util::FileList::iterator i2 = collection.begin(); i2 != collection.end(); ++i2 )
        {
            if( check_abort_scan() )
                return ;

            Track_sp t (new Track) ;
            library->trackSetLocation( (*t), *i2 ) ;

            const std::string& location = get<std::string>( (*t)[ATTRIBUTE_LOCATION].get() ) ;

            guint mtime1 = Util::get_file_mtime( location ); 
            guint mtime2 = get_track_mtime( (*t) ) ;

            // Check if the file hasn't changed
            if( check_mtimes && mtime2 )
            {
                    if( ( mtime1 <= last_scan_date)
                                 ||
                        ( mtime1 == mtime2 )
                    )
                    {
                        ++m_ScanSummary.FilesUpToDate ;
                        continue ;
                    }
            }

            (*t)[ATTRIBUTE_MTIME] = mtime1 ; // Update the mtime in the track we're "building"
            insert_file_no_mtime_check( t, *i2, insert_path_sql, true ) ;
                        
            if( !(std::distance(collection.begin(), i2) % 50) )
            {
                pthreaddata->Message.emit(
                    (boost::format(_("Collecting Tracks: %u of %u"))
                        % guint(std::distance(collection.begin(), i2))
                        % guint(m_ScanSummary.FilesTotal)
                    ).str()
                ) ;
            }
        }
    }

    process_insertion_list () ;
    //do_remove_dangling () ;
    update_albums () ;

    pthreaddata->Message.emit(_("Rescan: Done")) ;
    pthreaddata->ScanSummary.emit( m_ScanSummary ) ;
    pthreaddata->ScanEnd.emit() ;
}

void
MPX::LibraryScannerThread_MLibMan::on_scan_list_quick_stage_1(
    const Util::FileList& list
)
{
    ThreadData * pthreaddata = m_ThreadData.get() ;

    pthreaddata->ScanStart.emit() ;

    boost::shared_ptr<Library_MLibMan> library = services->get<Library_MLibMan>("mpx-service-library")  ;

    for( Util::FileList::const_iterator i = list.begin(); i != list.end(); ++i )
    {  
        std::string insert_path  ;
        std::string insert_path_sql  ;

#ifdef HAVE_HAL
        try{
#endif // HAVE_HAL

            RowV v ;
            insert_path = Util::normalize_path( *i )  ;

#ifdef HAVE_HAL

            try{
                if (m_Flags & Library_MLibMan::F_USING_HAL)
                { 
                    const Volume&   volume          = m_HAL.get_volume_for_uri (*i)  ;
                    guint          device_id       = m_HAL.get_id_for_volume( volume.volume_udi, volume.device_udi )  ;
                                    insert_path_sql = Util::normalize_path(Glib::filename_from_uri(*i).substr(volume.mount_point.length()))  ;

                    m_SQL->get(
                        v,
                        mprintf( 
                              "SELECT id, device_id, hal_vrp, mtime FROM track WHERE device_id ='%u' AND insert_path ='%q'"
                            , device_id 
                            , insert_path_sql.c_str())
                    ) ;
                }
                else
#endif
                {
                    insert_path_sql = insert_path ; 

                    m_SQL->get(
                        v,
                        mprintf( 
                              "SELECT id, location, mtime FROM track WHERE insert_path ='%q'"
                            , insert_path_sql.c_str())
                    ) ;
                }

#ifdef HAVE_HAL
            }
            catch( IHAL::Exception& cxe )
            {
                g_warning( "%s: %s", G_STRLOC, cxe.what() ); 
                continue ;
            }
            catch( Glib::ConvertError& cxe )
            {
                g_warning( "%s: %s", G_STRLOC, cxe.what().c_str() ); 
                continue ;
            }
#endif // HAVE_HAL

            for( RowV::iterator i = v.begin(); i != v.end(); ++i )
            {
                Track_sp t = library->sqlToTrack( *i, false ); 
                const std::string& location = get<std::string>((*t)[ATTRIBUTE_LOCATION].get() ) ; 

                // delete tracks which are gone from the FS
                Glib::RefPtr<Gio::File> file = Gio::File::create_for_uri( location ) ;

                if( !file->query_exists() ) // not there anymore ?
                {
                    // delete it
                    guint id = get<guint>((*i)["id"]) ;
                    m_SQL->exec_sql(mprintf( delete_track_f, id)) ;
                    pthreaddata->EntityDeleted.emit( id , ENTITY_TRACK ); 
                }

                if( check_abort_scan() ) 
                    return ;

                if( (! (std::distance(v.begin(), i) % 100)) )
                {
                    pthreaddata->Message.emit((boost::format(_("Checking Files for Presence: %u / %u")) % std::distance(v.begin(), i) % v.size()).str()) ;
                }
            }

#ifdef HAVE_HAL
        }
        catch( Glib::ConvertError & cxe )
        {
            g_warning("%s: %s", G_STRLOC, cxe.what().c_str()) ;
        }
#endif // HAVE_HAL
    }
}

void
MPX::LibraryScannerThread_MLibMan::on_scan_stop ()
{
    ThreadData * pthreaddata = m_ThreadData.get() ;
    g_atomic_int_set(&pthreaddata->m_Cancel, 1) ;
}

MPX::LibraryScannerThread_MLibMan::EntityInfo
MPX::LibraryScannerThread_MLibMan::get_track_artist_id (Track &track, bool only_if_exists)
{
    RowV rows; 

    EntityInfo info ( 0, MPX::LibraryScannerThread_MLibMan::ENTITY_IS_UNDEFINED ) ;

    if( track.has(ATTRIBUTE_ARTIST) && !track.has(ATTRIBUTE_ARTIST_SORTNAME)) 
    {
	track[ATTRIBUTE_ARTIST_SORTNAME] = track[ATTRIBUTE_ARTIST] ;	
    }

    track[ATTRIBUTE_ARTIST] = get_sortname(
	      get<std::string>(track[ATTRIBUTE_ARTIST].get())
	    , get<std::string>(track[ATTRIBUTE_ARTIST_SORTNAME].get())
    ) ;

    char const* select_artist_f ("SELECT id FROM artist WHERE %s %s AND %s %s AND %s %s;"); 
    m_SQL->get (rows, mprintf (select_artist_f,

           attrs[ATTRIBUTE_ARTIST].id,
          (track.has(ATTRIBUTE_ARTIST)
              ? mprintf (" = '%q'", get<std::string>(track[ATTRIBUTE_ARTIST].get()).c_str()).c_str()
              : "IS NULL"),

           attrs[ATTRIBUTE_MB_ARTIST_ID].id,
          (track.has(ATTRIBUTE_MB_ARTIST_ID)
              ? mprintf (" = '%q'", get<std::string>(track[ATTRIBUTE_MB_ARTIST_ID].get()).c_str()).c_str()
              : "IS NULL"),

           attrs[ATTRIBUTE_ARTIST_SORTNAME].id,
          (track.has(ATTRIBUTE_ARTIST_SORTNAME)
              ? mprintf (" = '%q'", get<std::string>(track[ATTRIBUTE_ARTIST_SORTNAME].get()).c_str()).c_str()
              : "IS NULL"))) ;

    if (rows.size ())
    {
      info = EntityInfo( 
            get<guint>(rows[0].find ("id")->second)
          , MPX::LibraryScannerThread_MLibMan::ENTITY_IS_NOT_NEW
      ) ;
    }
    else
    if (!only_if_exists)
    {
      char const* set_artist_f ("INSERT INTO artist (%s, %s, %s) VALUES (%Q, %Q, %Q);") ;

       info = EntityInfo(

              m_SQL->exec_sql (mprintf (set_artist_f,

                 attrs[ATTRIBUTE_ARTIST].id,
                 attrs[ATTRIBUTE_ARTIST_SORTNAME].id,
                 attrs[ATTRIBUTE_MB_ARTIST_ID].id,

                (track.has(ATTRIBUTE_ARTIST)
                    ? get<std::string>(track[ATTRIBUTE_ARTIST].get()).c_str()
                    : NULL) ,

                (track.has(ATTRIBUTE_ARTIST_SORTNAME)
                    ? get<std::string>(track[ATTRIBUTE_ARTIST_SORTNAME].get()).c_str()
                    : NULL) ,

                (track.has(ATTRIBUTE_MB_ARTIST_ID)
                    ? get<std::string>(track[ATTRIBUTE_MB_ARTIST_ID].get()).c_str()
                    : NULL)
                ))

            , MPX::LibraryScannerThread_MLibMan::ENTITY_IS_NEW
       ) ;
    }

    return info ;
}

MPX::LibraryScannerThread_MLibMan::EntityInfo
MPX::LibraryScannerThread_MLibMan::get_album_artist_id (Track& track, bool only_if_exists)
{
    EntityInfo info ( 0, MPX::LibraryScannerThread_MLibMan::ENTITY_IS_UNDEFINED ) ;

    if( track.has(ATTRIBUTE_ALBUM_ARTIST) && track.has(ATTRIBUTE_MB_ALBUM_ARTIST_ID) )
    {
        track[ATTRIBUTE_IS_MB_ALBUM_ARTIST] = guint(1); 
    }
    else
    {
	if( track.has( ATTRIBUTE_IS_COMPILATION ))
	{
        	track[ATTRIBUTE_IS_MB_ALBUM_ARTIST] = guint(1); 
		track[ATTRIBUTE_ALBUM_ARTIST] = "Various Artists" ;
		track[ATTRIBUTE_ALBUM_ARTIST_SORTNAME] = "Various Artists" ;
		track[ATTRIBUTE_MB_ALBUM_ARTIST_ID] = "89ad4ac3-39f7-470e-963a-56509c546377" ;
	}
	else
	{
        	track[ATTRIBUTE_IS_MB_ALBUM_ARTIST] = guint(0); 
        	track[ATTRIBUTE_ALBUM_ARTIST] = track[ATTRIBUTE_ARTIST] ;
	}
    }

    if( track.has(ATTRIBUTE_ALBUM_ARTIST) && !track.has(ATTRIBUTE_ALBUM_ARTIST_SORTNAME)) 
    {
	track[ATTRIBUTE_ALBUM_ARTIST_SORTNAME] = track[ATTRIBUTE_ALBUM_ARTIST] ;	
    }

    track[ATTRIBUTE_ALBUM_ARTIST] = get_sortname(
	      get<std::string>(track[ATTRIBUTE_ALBUM_ARTIST].get())
	    , get<std::string>(track[ATTRIBUTE_ALBUM_ARTIST_SORTNAME].get())
    ) ;

    RowV rows;

    if( track.has(ATTRIBUTE_MB_ALBUM_ARTIST_ID) )
    {
      char const* select_artist_f ("SELECT id FROM album_artist WHERE (%s %s) AND (%s %s);"); 
      m_SQL->get (rows, (mprintf (select_artist_f, 

         attrs[ATTRIBUTE_MB_ALBUM_ARTIST_ID].id,
        (track.has(ATTRIBUTE_MB_ALBUM_ARTIST_ID)
            ? mprintf (" = '%q'", get<std::string>(track[ATTRIBUTE_MB_ALBUM_ARTIST_ID].get()).c_str()).c_str()
            : "IS NULL"), 

         attrs[ATTRIBUTE_IS_MB_ALBUM_ARTIST].id,
        (get<guint>(track[ATTRIBUTE_IS_MB_ALBUM_ARTIST].get())
            ? "= '1'"
            : "IS NULL"))

      )) ;
    }
    else // TODO: MB lookup to fix sortname, etc, for a given ID (and possibly even retag files on the fly?)
    {
	char const* select_artist_f ("SELECT id FROM album_artist WHERE (%s %s) AND (%s %s);"); 

	m_SQL->get (rows, (mprintf (select_artist_f, 

		 attrs[ATTRIBUTE_ALBUM_ARTIST].id,
		(track.has(ATTRIBUTE_ALBUM_ARTIST)
		? mprintf (" = '%q'", get<std::string>(track[ATTRIBUTE_ALBUM_ARTIST].get()).c_str()).c_str()
		: "IS NULL"), 

		 attrs[ATTRIBUTE_IS_MB_ALBUM_ARTIST].id,
		(get<guint>(track[ATTRIBUTE_IS_MB_ALBUM_ARTIST].get())
		? "= '1'"
		: "IS NULL"))

       )) ; 
    }

    if (!rows.empty())
    {
        info = EntityInfo(
              get<guint>(rows[0].find ("id")->second)
            , MPX::LibraryScannerThread_MLibMan::ENTITY_IS_NOT_NEW
        ) ;

	return info ;
    }
    else
    if( !only_if_exists )
    {
        if( !track.has(ATTRIBUTE_MB_ALBUM_ARTIST_ID) )
	{
             track[ATTRIBUTE_MB_ALBUM_ARTIST_ID] = "mpx-" + get<std::string>(track[ATTRIBUTE_ALBUM_ARTIST].get()); 
	}

        char const* set_artist_f ("INSERT INTO album_artist (%s, %s, %s, %s) VALUES (%Q, %Q, %Q, %Q);") ;

        info = EntityInfo(

            m_SQL->exec_sql (mprintf (set_artist_f,

              attrs[ATTRIBUTE_ALBUM_ARTIST].id,
              attrs[ATTRIBUTE_ALBUM_ARTIST_SORTNAME].id,
              attrs[ATTRIBUTE_MB_ALBUM_ARTIST_ID].id,
              attrs[ATTRIBUTE_IS_MB_ALBUM_ARTIST].id,

            (track.has(ATTRIBUTE_ALBUM_ARTIST)
                ? get<std::string>(track[ATTRIBUTE_ALBUM_ARTIST].get()).c_str() 
                : NULL) , 

            (track.has(ATTRIBUTE_ALBUM_ARTIST_SORTNAME)
                ? get<std::string>(track[ATTRIBUTE_ALBUM_ARTIST_SORTNAME].get()).c_str()
                : NULL) , 

            (track.has(ATTRIBUTE_MB_ALBUM_ARTIST_ID)
                ? get<std::string>(track[ATTRIBUTE_MB_ALBUM_ARTIST_ID].get()).c_str()
                : NULL) ,

            (get<guint>(track[ATTRIBUTE_IS_MB_ALBUM_ARTIST].get())
                ? "1"
                : NULL)
            ))

            , MPX::LibraryScannerThread_MLibMan::ENTITY_IS_NEW
        ) ;
    }

    return info ;
}

MPX::LibraryScannerThread_MLibMan::EntityInfo
MPX::LibraryScannerThread_MLibMan::get_album_id (Track& track, guint album_artist_id, bool only_if_exists)
{
    RowV rows;

    std::string sql ;

    EntityInfo info ( 0, MPX::LibraryScannerThread_MLibMan::ENTITY_IS_UNDEFINED ) ;

    if( track.has( ATTRIBUTE_MB_ALBUM_ID ))
    {
      char const* select_album_f ("SELECT album, id, mb_album_id FROM album WHERE (%s = '%q') AND (%s %s) AND (%s %s) AND (%s = %u);"); 

      sql = mprintf (select_album_f,

             attrs[ATTRIBUTE_MB_ALBUM_ID].id,
             get<std::string>(track[ATTRIBUTE_MB_ALBUM_ID].get()).c_str(),

             attrs[ATTRIBUTE_MB_RELEASE_DATE].id,
            (track.has(ATTRIBUTE_MB_RELEASE_DATE)
                ? mprintf (" = '%q'", get<std::string>(track[ATTRIBUTE_MB_RELEASE_DATE].get()).c_str()).c_str()
                : "IS NULL"), 

             attrs[ATTRIBUTE_MB_RELEASE_COUNTRY].id,
            (track.has(ATTRIBUTE_MB_RELEASE_COUNTRY)
                ? mprintf (" = '%q'", get<std::string>(track[ATTRIBUTE_MB_RELEASE_COUNTRY].get()).c_str()).c_str()
                : "IS NULL"), 

            "album_artist_j",
            album_artist_id
      ) ;
    }
    else
    {
	char const* select_album_f ("SELECT album, id, mb_album_id FROM album WHERE (%s %s) AND (%s %s) AND (%s = %u);"); 

	sql = mprintf( select_album_f,

	     attrs[ATTRIBUTE_ALBUM].id,
	    (track.has( ATTRIBUTE_ALBUM )
		? mprintf (" = '%q'", get<std::string>(track[ATTRIBUTE_ALBUM].get()).c_str()).c_str()
		: "IS NULL"), 

	     attrs[ATTRIBUTE_ASIN].id,
	    (track.has( ATTRIBUTE_ASIN )
		? mprintf (" = '%q'", get<std::string>(track[ATTRIBUTE_ASIN].get()).c_str()).c_str()
		: "IS NULL"), 

	    "album_artist_j",
	    album_artist_id
	) ;
    }

    OVariant mbid ;
    m_SQL->get( rows, sql ) ; 

    if( !rows.empty() )
    {
        info = EntityInfo(
              get<guint>(rows[0].find ("id")->second)
            , MPX::LibraryScannerThread_MLibMan::ENTITY_IS_NOT_NEW
        ) ;

        if( rows[0].count("mb_album_id" ))
        {
            track[ATTRIBUTE_MB_ALBUM_ID] = get<std::string>( rows[0].find("mb_album_id")->second ) ;
        }
    }
    else
    if( !only_if_exists )
    {
        if( !track.has(ATTRIBUTE_MB_ALBUM_ID ))
        {
	    if( !track.has( ATTRIBUTE_IS_COMPILATION ))
	    {
            	track[ATTRIBUTE_MB_ALBUM_ID] = "mpx-" + get<std::string>(track[ATTRIBUTE_ARTIST].get())
                	                              + "-"
                        	                      + get<std::string>(track[ATTRIBUTE_ALBUM].get()); 
	    }
	    else
	    {
            	track[ATTRIBUTE_MB_ALBUM_ID] = "mpx-VARIOUS_ARTISTS-CPL-" + get<std::string>(track[ATTRIBUTE_ALBUM].get()); 
	    }
        }

        char const* insert_album_f ("INSERT INTO album (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, album_new) VALUES (%Q, %Q, %Q, %Q, %Q, %Q, %Q, %u, %u, %u, %u, %u, 1);") ;

        std::string sql = mprintf(

              insert_album_f

            , attrs[ATTRIBUTE_ALBUM].id
            , attrs[ATTRIBUTE_MB_ALBUM_ID].id
            , attrs[ATTRIBUTE_MB_RELEASE_DATE].id
            , attrs[ATTRIBUTE_MB_RELEASE_COUNTRY].id
            , attrs[ATTRIBUTE_MB_RELEASE_TYPE].id
            , attrs[ATTRIBUTE_ASIN].id
            , "album_label"
            , "album_artist_j"
            , "album_insert_date"
            , "album_disctotal"
	    , "album_discs"
	    , "album_is_cpl"

            ,

            (track.has(ATTRIBUTE_ALBUM)
                ? get<std::string>(track[ATTRIBUTE_ALBUM].get()).c_str()
                : NULL) , 

            (track.has(ATTRIBUTE_MB_ALBUM_ID)
                ? get<std::string>(track[ATTRIBUTE_MB_ALBUM_ID].get()).c_str()
                : NULL) , 

            (track.has(ATTRIBUTE_MB_RELEASE_DATE)
                ? get<std::string>(track[ATTRIBUTE_MB_RELEASE_DATE].get()).c_str()
                : NULL) , 

            (track.has(ATTRIBUTE_MB_RELEASE_COUNTRY)
                ? get<std::string>(track[ATTRIBUTE_MB_RELEASE_COUNTRY].get()).c_str()
                : NULL) , 

            (track.has(ATTRIBUTE_MB_RELEASE_TYPE)
                ? get<std::string>(track[ATTRIBUTE_MB_RELEASE_TYPE].get()).c_str()
                : NULL) , 

            (track.has(ATTRIBUTE_ASIN)
                ? get<std::string>(track[ATTRIBUTE_ASIN].get()).c_str()
                : NULL) , 

            (track.has(ATTRIBUTE_LABEL)
                ? get<std::string>(track[ATTRIBUTE_LABEL].get()).c_str()
                : NULL) , 

            album_artist_id,

            (track.has(ATTRIBUTE_MTIME)
                ? get<guint>(track[ATTRIBUTE_MTIME].get())
                : 0),

            (track.has(ATTRIBUTE_DISCTOTAL)
                ? get<guint>(track[ATTRIBUTE_DISCTOTAL].get())
                : 0),

            (track.has(ATTRIBUTE_DISCS)
                ? get<guint>(track[ATTRIBUTE_DISCS].get())
                : 0),

            (track.has(ATTRIBUTE_IS_COMPILATION)
                ? get<guint>(track[ATTRIBUTE_IS_COMPILATION].get())
                : 0)
        ) ;

	guint album_id = m_SQL->exec_sql( sql ) ;

        info = EntityInfo(
            album_id 
          , MPX::LibraryScannerThread_MLibMan::ENTITY_IS_NEW
        ) ;

	if( track.has(ATTRIBUTE_GENRE))
	{
        	using boost::algorithm::split;
	        using boost::algorithm::is_any_of;
        	using boost::algorithm::find_first;
        	using boost::algorithm::trim;

	        StrV m;

		const std::string& genre_str = get<std::string>(track[ATTRIBUTE_GENRE].get()) ;

	        split( m, genre_str, is_any_of(",")) ;

		for( StrV::const_iterator n = m.begin() ; n != m.end() ; ++n )
		{
			std::string s = *n ;

			trim( s ) ;	

			guint id = 0 ;

			try{	
		        	char const* insert_tag_f ("INSERT INTO tag (tag) VALUES ('%q')") ;
				std::string tag_sql = mprintf( insert_tag_f, s.c_str() ) ;
				id = m_SQL->exec_sql( tag_sql ) ;
			} catch( SqlConstraintError& cxe ) 
			{
		        	char const* update_tag_f ("UPDATE tag SET weight = weight + 1 WHERE tag = '%q'") ; 
				std::string tag_sql = mprintf( update_tag_f, s.c_str() ) ;
				m_SQL->exec_sql( tag_sql ) ;

				char const* select_tag_f ("SELECT id FROM tag WHERE tag = '%q'") ;
				RowV v ;
				m_SQL->get( v, mprintf( select_tag_f, s.c_str() )) ;
				id = get<guint>(v[0]["id"]) ;
			}

			if( id ) // if NOT then screwed
			{
				char const * set_album_tag_f ("INSERT INTO tags_album (tagid, albumid) VALUES ('%u', '%u')") ; 
				std::string insert_album_tag = mprintf( set_album_tag_f, id, album_id ) ;

				try{
					m_SQL->exec_sql( insert_album_tag ) ;
				} catch( SqlConstraintError& cxe )
				{
				}	
			}
		}
	}

    }

    return info ;
}

guint
MPX::LibraryScannerThread_MLibMan::get_track_mtime(
    Track& track
) const
{
    Duplet_MTIME_t::const_iterator i =
         m_MTIME_Map.find(
            boost::make_tuple(
                get<guint>(track[ATTRIBUTE_MPX_DEVICE_ID].get())
              , get<std::string>(track[ATTRIBUTE_VOLUME_RELATIVE_PATH].get())
    )) ;

    if( i != m_MTIME_Map.end() )
    {
        return i->second ;
    }

    return 0 ;
}

guint
MPX::LibraryScannerThread_MLibMan::get_track_id (Track& track) const
{
    RowV v ;

#ifdef HAVE_HAL
    if( m_Flags & Library_MLibMan::F_USING_HAL )
    {
        static boost::format select_f ("SELECT id FROM track WHERE %s='%u' AND %s='%s';") ;

        if( !(track.has(ATTRIBUTE_MPX_DEVICE_ID) && track.has(ATTRIBUTE_VOLUME_RELATIVE_PATH)))
        {
            return 0 ;
        }
         
        m_SQL->get(
                v
              , (select_f % attrs[ATTRIBUTE_MPX_DEVICE_ID].id
                          % get<guint>(track[ATTRIBUTE_MPX_DEVICE_ID].get())
                          % attrs[ATTRIBUTE_VOLUME_RELATIVE_PATH].id
                          % mprintf ("%q", Util::normalize_path(get<std::string>(track[ATTRIBUTE_VOLUME_RELATIVE_PATH].get())).c_str())
                ).str()
        ) ;
    }
    else
#endif
    {
        static boost::format select_f ("SELECT id FROM track WHERE %s='%s';") ;

        if( !(track.has(ATTRIBUTE_LOCATION)) )
        {
            return 0 ;
        }

        m_SQL->get(
                v
              , (select_f % attrs[ATTRIBUTE_LOCATION].id 
                          % mprintf ("%q", get<std::string>(track[ATTRIBUTE_LOCATION].get()).c_str())
                ).str()
        ) ;
    }

    if( v.size() && ( v[0].count("id") != 0 ))
    {
        return boost::get<guint>( v[0]["id"] ) ;
    }

    return 0 ;
}

void
MPX::LibraryScannerThread_MLibMan::add_erroneous_track(
      const std::string& uri
    , const std::string& info
)
{
    ++m_ScanSummary.FilesErroneous;

    URI u (uri) ;
    u.unescape() ;

    try{
          m_ScanSummary.FileListErroneous.push_back( SSFileInfo( filename_to_utf8((ustring(u))), info)) ;
    } catch( Glib::ConvertError & cxe )
    {
          m_ScanSummary.FileListErroneous.push_back( SSFileInfo( _("(invalid UTF-8)"), (boost::format (_("Could not convert URI to UTF-8 for display: %s")) % cxe.what()).str())) ;
    }
}

void
MPX::LibraryScannerThread_MLibMan::insert_file_no_mtime_check(
      Track_sp           track
    , const std::string& uri
    , const std::string& insert_path
    , bool               update
)
{
    Track& t = *(track.get()) ;

    try{
        try{

            // Get the MIME type
            t[ATTRIBUTE_TYPE] = Gio::File::create_for_uri(uri)->query_info("standard::content-type")->get_attribute_string("standard::content-type") ;

        } catch(Gio::Error & cxe)
        {
            add_erroneous_track( uri, _("An error ocurred trying to determine the file type")) ;
            return ;
        }

        if( !services->get<MetadataReaderTagLib>("mpx-service-taglib")->get( uri, t )) // Failed to read metadata at all?
        {
            // If so, the track is erroneous
            add_erroneous_track( uri, _("Could not acquire metadata (using taglib-gio)")) ;
            quarantine_file( uri ) ;
        }
        else
        try{
            create_insertion_track( t, uri, insert_path, update ) ;
        }
        catch( ScanError & cxe )
        {
            add_erroneous_track( uri, (boost::format (_("Error inserting file: %s")) % cxe.what()).str()) ;
        }
    }
    catch( Library_MLibMan::FileQualificationError & cxe )
    {
        add_erroneous_track( uri, (boost::format (_("Error inserting file: %s")) % cxe.what()).str()) ;
    }
}

std::string
MPX::LibraryScannerThread_MLibMan::create_update_sql(
      const Track&  track
    , guint        album_j
    , guint        artist_j
)
{
    std::string sql = "UPDATE track SET " ;

    for( unsigned int n = 0; n < ATTRIBUTE_MPX_TRACK_ID; ++n )
    {
        if( track.has( n ))
        {
            switch( attrs[n].type )
            {
                  case VALUE_TYPE_STRING:
                      append_key_value_pair<std::string>( sql, track, n ) ;
                      break ;

                  case VALUE_TYPE_INT:
                      append_key_value_pair<guint>( sql, track, n ) ;
                      break ;

                  case VALUE_TYPE_REAL:
                      append_key_value_pair<double>( sql, track, n ) ;
                      break ;
            }

            sql += ","; 
        }
    }

    sql += mprintf("artist_j = '%u', ", artist_j ); 
    sql += mprintf("album_j = '%u'  ", album_j ); 

    return sql ;
}

std::string
MPX::LibraryScannerThread_MLibMan::create_insertion_sql(
      const Track&  track
    , guint        album_j
    , guint        artist_j
)
{
    char const track_set_f[] = "INSERT INTO track (%s) VALUES (%s);" ;

    std::string sql ;

    std::string column_n ;
    std::string column_v ;

    column_n.reserve( 0x400 ) ;
    column_v.reserve( 0x400 ) ;

    for( unsigned int n = 0; n < ATTRIBUTE_MPX_TRACK_ID; ++n )
    {
        if( track.has( n ))
        {
            switch( attrs[n].type )
            {
                  case VALUE_TYPE_STRING:
                      append_value<std::string>( column_v, track, n ) ;
                      break ;

                  case VALUE_TYPE_INT:
                      append_value<guint>( column_v, track, n ) ;
                      break ;

                  case VALUE_TYPE_REAL:
                      append_value<double>( column_v, track, n ) ;
                      break ;
            }

            column_n += std::string( attrs[n].id ) + "," ;
            column_v += "," ;
        }
    }

    column_n += "artist_j, album_j" ;
    column_v += mprintf( "'%u'", artist_j ) + "," + mprintf( "'%u'", album_j ) ; 

    return mprintf( track_set_f, column_n.c_str(), column_v.c_str()) ;
}

void
MPX::LibraryScannerThread_MLibMan::create_insertion_track(
      Track&             track
    , const std::string& uri
    , const std::string& insert_path
    , bool               update
)
{
    if( uri.empty() )
    {
      throw ScanError(_("Empty URI/no URI given")) ;
    }

    ThreadData * pthreaddata = m_ThreadData.get() ;

    if( !(track.has(ATTRIBUTE_ALBUM) && track.has(ATTRIBUTE_ARTIST) && track.has(ATTRIBUTE_TITLE)) ) // Does this track lack mandatory metadata?
    {
        // If yes, see if we already have it (i.e. obtain its ID, which must be non-0 if it already has one, i.e. get_track_id() will not create
        // a track unlike (yesh, sucks) get_album_id() and get_album_artist_id()

        guint id = get_track_id( track ) ;

        if( id != 0 )
        {
            // OK so it's in the db, delete it from there
            m_SQL->exec_sql( mprintf( delete_track_f, id )) ;
            pthreaddata->EntityDeleted.emit( id , ENTITY_TRACK ); 
        }

        quarantine_file( uri )  ;

        throw ScanError(_("Insufficient Metadata (artist, album and title must be given)")) ;
    }

    track[ATTRIBUTE_INSERT_PATH] = Util::normalize_path( insert_path )  ;
    track[ATTRIBUTE_INSERT_DATE] = guint(time(NULL)) ;
    track[ATTRIBUTE_QUALITY]     = get_audio_quality( get<std::string>(track[ATTRIBUTE_TYPE].get()), get<guint>(track[ATTRIBUTE_BITRATE].get()) ) ;

    TrackInfo_p p (new TrackInfo) ;

    p->Artist         = get_track_artist_id( track )  ;
    p->AlbumArtist    = get_album_artist_id( track )  ;
    p->Album          = get_album_id( track, p->AlbumArtist.first ) ;
    p->Title          = get<std::string>(track[ATTRIBUTE_TITLE].get()) ;
    p->TrackNumber    = get<guint>(track[ATTRIBUTE_TRACK].get()) ;
    p->Type           = get<std::string>(track[ATTRIBUTE_TYPE].get()) ;
    p->Update         = update ;
    p->Track          = Track_sp( new Track( track )) ;

    if( p->Album.second == ENTITY_IS_NEW )
    {
        m_AlbumIDs.insert( p->Album.first ) ;
    }

    if( p->AlbumArtist.second == ENTITY_IS_NEW )
    {
        m_AlbumArtistIDs.insert( p->AlbumArtist.first ) ;
    }

    m_InsertionTracks[p->Album.first][p->Artist.first][p->Title][p->TrackNumber].push_back( p ) ;
}

MPX::LibraryScannerThread_MLibMan::TrackInfo_p
MPX::LibraryScannerThread_MLibMan::prioritize(
    const TrackInfo_p_Vector& v
)
{
    if( v.size() == 1 )
    {
        return v[0] ;
    }

    TrackInfo_p_Vector v2 ;

    // This is basically an awkward way to sort the tracks by MIME type in the current preference
    // order

    if( m_PrioritizeByFileType )
    {
            for( MIME_Types_t::const_iterator t = m_MIME_Types.begin(); t != m_MIME_Types.end(); ++t )
            {
                for( TrackInfo_p_Vector::const_iterator i = v.begin(); i != v.end(); ++i )
                {
                    if( (*i)->Type == (*t) )
                    {
                        v2.push_back( *i ) ;
                    }
                }

                if( !v2.empty() )
                    break ;
            }
            
            if( v2.empty() )
            {
                v2 = v ;
            }
    }
    else
    {
            v2 = v ;
    }

    if( v2.size() == 1 || !m_PrioritizeByBitrate )
    {
        return v2[0] ;  // If it's just 1 track anyway, or we don't want to further prioritze by bitrate anyway,
                        // we just return in. Otherwise, we go through the tracks again and find the one with the
                        // highest bitrate
    }
    else
    {
        TrackInfo_p p_highest_bitrate ;
        guint      bitrate = 0 ;

        for( TrackInfo_p_Vector::const_iterator i = v2.begin(); i != v2.end(); ++i )
        {
            guint c_bitrate = get<guint>((*(*i)->Track.get())[ATTRIBUTE_BITRATE].get()) ;

            if( c_bitrate > bitrate )
            {
                bitrate = c_bitrate ;
                p_highest_bitrate = (*i) ;
            }
        }

        return p_highest_bitrate ;
    }

    return v[0] ;
}

void
MPX::LibraryScannerThread_MLibMan::process_insertion_list()
{
    ThreadData * pthreaddata = m_ThreadData.get() ;

    guint tracks = 0 ;

    for( Map_L1::const_iterator l1 = m_InsertionTracks.begin(); l1 != m_InsertionTracks.end(); ++l1 ) {

    for( Map_L2::const_iterator l2 = (l1->second).begin(); l2 != (l1->second).end(); ++l2 ) {
    for( Map_L3::const_iterator l3 = (l2->second).begin(); l3 != (l2->second).end(); ++l3 ) {
    for( Map_L4::const_iterator l4 = (l3->second).begin(); l4 != (l3->second).end(); ++l4 ) {


                    const TrackInfo_p_Vector& v = (*l4).second ; 

                    std::string location ;
    
                    if( !v.empty() )
                    try{

                        TrackInfo_p p = prioritize( v ) ;

                        location = get<std::string>((*(p->Track.get()))[ATTRIBUTE_LOCATION].get()) ;

                        switch( insert( p, v ) ) 
                        {
                            case SCAN_RESULT_OK:
                                 pthreaddata->Message.emit(
                                   (boost::format(_("Inserting Tracks: %u"))
                                        % ++tracks 
                                    ).str()
                                 ) ;
                                ++m_ScanSummary.FilesAdded ;
                                break ;

                            case SCAN_RESULT_UPDATE:
                                try{
                                    ++m_ScanSummary.FilesUpdated ;
                                      m_ScanSummary.FileListUpdated.push_back( SSFileInfo( filename_to_utf8(location), _("Updated"))) ;
                                } catch (Glib::ConvertError & cxe )
                                {
                                      m_ScanSummary.FileListErroneous.push_back( SSFileInfo( _("(invalid UTF-8)"), _("Could not convert URI to UTF-8 for display"))) ;
                                }
                                break ;

                            case SCAN_RESULT_UPTODATE:
                                ++m_ScanSummary.FilesUpToDate ;
                                break ;
                        }
                    }
                    catch( ScanError & cxe )
                    {
                        add_erroneous_track( location, (boost::format (_("Error inserting file: %s")) % cxe.what()).str() ) ;
                    }

    }
    }
    }
    }
       
    m_InsertionTracks.clear(); 
    m_AlbumIDs.clear() ;
    m_AlbumArtistIDs.clear() ;
}

void
MPX::LibraryScannerThread_MLibMan::signal_new_entities(
    const TrackInfo_p& p
)
{
    ThreadData * pthreaddata = m_ThreadData.get() ;
    
    guint album_id = p->Album.first; 
    guint artst_id = p->AlbumArtist.first ;

    if( m_AlbumArtistIDs.find( artst_id ) != m_AlbumArtistIDs.end() )
    { 
        pthreaddata->NewArtist.emit( artst_id ) ;
        m_AlbumArtistIDs.erase( artst_id ) ;
    }

    if( m_AlbumIDs.find( album_id ) != m_AlbumIDs.end() )
    {
            m_AlbumIDs.erase( album_id ) ;

            RequestQualifier rq ;

            rq.mbid       = get<std::string>((*(p->Track.get()))[ATTRIBUTE_MB_ALBUM_ID].get()) ;
            rq.asin       = (*(p->Track.get())).has(ATTRIBUTE_ASIN)
                                  ? get<std::string>((*(p->Track.get()))[ATTRIBUTE_ASIN].get())
                                  : "" ;

            rq.uri        = get<std::string>((*(p->Track.get()))[ATTRIBUTE_LOCATION].get()) ;
            rq.artist     = (*(p->Track.get())).has(ATTRIBUTE_ALBUM_ARTIST)
                                  ? get<std::string>((*(p->Track.get()))[ATTRIBUTE_ALBUM_ARTIST].get())
                                  : get<std::string>((*(p->Track.get()))[ATTRIBUTE_ARTIST].get()) ;

            rq.album      = get<std::string>((*(p->Track.get()))[ATTRIBUTE_ALBUM].get()) ;

            pthreaddata->NewAlbum.emit( album_id, rq.mbid, rq.asin, rq.uri, rq.artist, rq.album ) ;
    }
}

int
MPX::LibraryScannerThread_MLibMan::compare_types(
      const std::string& a
    , const std::string& b
)
{
    int val_a = std::numeric_limits<int>::max() ;
    int val_b = std::numeric_limits<int>::max() ;

    for( MIME_Types_t::iterator t = m_MIME_Types.begin(); t != m_MIME_Types.end(); ++t )
    {
        if( *t == a )
        {
            val_a = std::distance( m_MIME_Types.begin(), t ) ;
            break ;
        }
    }

    
    for( MIME_Types_t::iterator t = m_MIME_Types.begin(); t != m_MIME_Types.end(); ++t )
    {
        if( *t == b )
        {
            val_b = std::distance( m_MIME_Types.begin(), t ) ;
            break ;
        }
    }

    if( val_a < val_b )
        return  1 ;
    else
    if( val_a > val_b )
        return -1 ;

    return 0 ;
}

ScanResult
MPX::LibraryScannerThread_MLibMan::insert(
      const TrackInfo_p& p
    , const TrackInfo_p_Vector& v
)
{
  ThreadData * pthreaddata = m_ThreadData.get() ;

  Track & track = *(p->Track.get()) ;

  m_ProcessedAlbums.insert( p->Album.first ) ;

  try{

    guint id_old = get_track_id( track ) ;

    if( p->Update && id_old )
    {
        m_SQL->exec_sql(
            mprintf(
                  "DELETE FROM track WHERE id = '%u'"
                , id_old
        )) ;

        pthreaddata->EntityDeleted( id_old, ENTITY_TRACK ) ;

        guint id_new = m_SQL->exec_sql( create_insertion_sql( track, p->Album.first, p->Artist.first )); 

        m_SQL->exec_sql(
            mprintf(
                  "UPDATE track SET id = '%u' WHERE id = '%u'"
                , id_old
                , id_new
        )) ;

        track[ATTRIBUTE_MPX_TRACK_ID] = id_old ; 
    }
    else
    {
        track[ATTRIBUTE_MPX_TRACK_ID] = m_SQL->exec_sql( create_insertion_sql( track, p->Album.first, p->Artist.first )); 
    } 
  } catch( SqlConstraintError & cxe )
  {
    RowV rv ;

    m_SQL->get(
          rv
        , mprintf(
              "SELECT id, type, bitrate, location FROM track WHERE album_j = '%u' AND artist_j = '%u' AND track = '%u' AND title = '%q'"
            , p->Album.first
            , p->Artist.first
            , p->TrackNumber
            , p->Title.c_str()
    )); 

    if( rv.size() == 1 ) 
    {
        const guint&       id              = get<guint>(rv[0]["id"]) ;
        const guint&       bitrate_datarow = get<guint>(rv[0]["bitrate"]) ;
        const guint&       bitrate_track   = get<guint>(track[ATTRIBUTE_BITRATE].get()) ;
        const std::string&  type            = get<std::string>(rv[0]["type"]) ;

        track[ATTRIBUTE_MPX_TRACK_ID] = id ; 

        if(
            (
              m_PrioritizeByFileType
                    &&
              compare_types( p->Type, type )
            )

                    ||

            (
              m_PrioritizeByBitrate
                    &&
              bitrate_track > bitrate_datarow
            )
        )
        try{
                m_SQL->exec_sql( create_update_sql( track, p->Album.first, p->Artist.first ) + ( boost::format( " WHERE id = '%u'" ) % id ).str() ); 
                signal_new_entities( p ) ;
                pthreaddata->EntityUpdated( id, ENTITY_TRACK ) ;

                return SCAN_RESULT_UPDATE  ;

        } catch( SqlConstraintError & cxe )
        {
                g_message("Scan Error!: %s", cxe.what()) ;
                throw ScanError((boost::format(_("Constraint error while trying to set new track data!: '%s'")) % cxe.what()).str()) ;
        }

        return SCAN_RESULT_UPTODATE  ;
    }
    else
    {
        throw ScanError(_("Got no track or more than one track for given ID! Please report!")) ;
    }
  }
  catch( SqlExceptionC & cxe )
  {
        g_message("Scan Error!: %s", cxe.what()) ;
        throw ScanError((boost::format(_("SQL error while inserting/updating track: '%s'")) % cxe.what()).str()) ;
  }

  signal_new_entities( p ) ;

  pthreaddata->NewTrack.emit( get<guint>(track[ATTRIBUTE_MPX_TRACK_ID].get()) )  ;

  if( p->Album.second == ENTITY_IS_NOT_NEW )
  {
    pthreaddata->EntityUpdated( p->Album.first, ENTITY_ALBUM ) ;
  }

  return SCAN_RESULT_OK ; 
}

void
MPX::LibraryScannerThread_MLibMan::do_remove_dangling () 
{
  ThreadData * pthreaddata = m_ThreadData.get() ;

  typedef std::set<guint> IdSet ;

  static boost::format delete_f ("DELETE FROM %s WHERE id = '%u'") ;
  IdSet idset1 ;
  IdSet idset2 ;
  RowV rows;

  /// CLEAR DANGLING ARTISTS
  pthreaddata->Message.emit(_("Finding Lost Artists...")) ;

  idset1.clear() ;
  rows.clear() ;
  m_SQL->get(rows, "SELECT DISTINCT artist_j FROM track") ;
  for (RowV::const_iterator i = rows.begin(); i != rows.end(); ++i)
          idset1.insert (get<guint>(i->find ("artist_j")->second)) ;

  idset2.clear() ;
  rows.clear() ;
  m_SQL->get(rows, "SELECT DISTINCT id FROM artist") ;
  for (RowV::const_iterator i = rows.begin(); i != rows.end(); ++i)
          idset2.insert (get<guint>(i->find ("id")->second)) ;

  for (IdSet::const_iterator i = idset2.begin(); i != idset2.end(); ++i)
  {
        if (idset1.find (*i) == idset1.end())
        {
            m_SQL->exec_sql((delete_f % "artist" % (*i)).str()) ;
            pthreaddata->EntityDeleted( *i , ENTITY_ARTIST ) ;
        }
  }


  /// CLEAR DANGLING ALBUMS
  pthreaddata->Message.emit(_("Finding Lost Albums...")) ;

  idset1.clear() ;
  rows.clear() ;
  m_SQL->get(rows, "SELECT DISTINCT album_j FROM track") ;
  for (RowV::const_iterator i = rows.begin(); i != rows.end(); ++i)
          idset1.insert (get<guint>(i->find ("album_j")->second)) ;

  idset2.clear() ;
  rows.clear() ;
  m_SQL->get(rows, "SELECT DISTINCT id FROM album") ;
  for (RowV::const_iterator i = rows.begin(); i != rows.end(); ++i)
          idset2.insert (get<guint>(i->find ("id")->second)) ;

  for (IdSet::const_iterator i = idset2.begin(); i != idset2.end(); ++i)
  {
        if (idset1.find (*i) == idset1.end())
        {
            m_SQL->exec_sql((delete_f % "album" % (*i)).str()) ;
            pthreaddata->EntityDeleted( *i , ENTITY_ALBUM ) ;
        }
  }

  /// CLEAR DANGLING ALBUM ARTISTS
  pthreaddata->Message.emit(_("Finding Lost Album Artists...")) ;

  idset1.clear() ;
  rows.clear() ;
  m_SQL->get(rows, "SELECT DISTINCT album_artist_j FROM album") ;
  for (RowV::const_iterator i = rows.begin(); i != rows.end(); ++i)
          idset1.insert (get<guint>(i->find ("album_artist_j")->second)) ;

  idset2.clear() ;
  rows.clear() ;
  m_SQL->get(rows, "SELECT DISTINCT id FROM album_artist") ;
  for (RowV::const_iterator i = rows.begin(); i != rows.end(); ++i)
          idset2.insert (get<guint>(i->find ("id")->second)) ;

  for (IdSet::const_iterator i = idset2.begin(); i != idset2.end(); ++i)
  {
        if (idset1.find (*i) == idset1.end())
        {
            m_SQL->exec_sql((delete_f % "album_artist" % (*i)).str()) ;
	    g_message("%s: Deleting Album Artist ID %u", G_STRFUNC, *i) ;
            pthreaddata->EntityDeleted( *i , ENTITY_ALBUM_ARTIST ) ;
        }
  }

  pthreaddata->Message.emit(_("Cleanup: Done")) ;
}

void
MPX::LibraryScannerThread_MLibMan::on_vacuum() 
{
  ThreadData * pthreaddata = m_ThreadData.get() ;

  pthreaddata->ScanStart.emit() ;

  RowV rows;
  m_SQL->get (rows, "SELECT id, device_id, hal_vrp, location FROM track"); // FIXME: We shouldn't need to do this here, it should be transparent (HAL vs. non HAL)

  for( RowV::iterator i = rows.begin(); i != rows.end(); ++i )
  {
          std::string uri = get<std::string>((*(m_Library_MLibMan.sqlToTrack( *i, false )))[ATTRIBUTE_LOCATION].get()) ;

          if( !uri.empty() )
          {
              if( (!(std::distance(rows.begin(), i) % 50)) )
              {
                      pthreaddata->Message.emit((boost::format(_("Checking files for presence: %u / %u")) % std::distance(rows.begin(), i) % rows.size()).str()) ;
              }

              try{
                      Glib::RefPtr<Gio::File> file = Gio::File::create_for_uri(uri) ;
                      if( !file->query_exists() )
                      {
                              m_SQL->exec_sql((boost::format ("DELETE FROM track WHERE id = '%u'") % get<guint>((*i)["id"])).str()); 
                              pthreaddata->EntityDeleted( get<guint>((*i)["id"]) , ENTITY_TRACK ) ;
                      }
              } catch(Glib::Error) {
                        g_message(G_STRLOC ": Error while trying to test URI '%s' for presence", uri.c_str()) ;
              }
          }
  }

  // do_remove_dangling () ;

  pthreaddata->Message.emit(_("Vacuum process done.")) ;
  pthreaddata->ScanEnd.emit() ;
}

#ifdef HAVE_HAL
void
MPX::LibraryScannerThread_MLibMan::on_vacuum_volume_list(
      const VolumeKey_v&    volumes
    , bool                    do_signal
)
{
  ThreadData * pthreaddata = m_ThreadData.get() ;
    
  if( do_signal )
      pthreaddata->ScanStart.emit() ;


  for( VolumeKey_v::const_iterator i = volumes.begin(); i != volumes.end(); ++i )
  {
          guint device_id = m_HAL.get_id_for_volume( (*i).first, (*i).second )  ;

          RowV rows;
          m_SQL->get(
              rows,
              (boost::format ("SELECT * FROM track WHERE device_id = '%u'")
                      % device_id 
              ).str()) ;

          for( RowV::iterator i = rows.begin(); i != rows.end(); ++i )
          {
                  std::string uri = get<std::string>((*(m_Library_MLibMan.sqlToTrack( *i, false )))[ATTRIBUTE_LOCATION].get()) ;

                  if( !uri.empty() )
                  {
                      if( (!(std::distance(rows.begin(), i) % 50)) )
                      {
                              pthreaddata->Message.emit((boost::format(_("Checking files for presence: %u / %u")) % std::distance(rows.begin(), i) % rows.size()).str()) ;
                      }

                      try{
                              Glib::RefPtr<Gio::File> file = Gio::File::create_for_uri(uri) ;
                              if( !file->query_exists() )
                              {
                                      m_SQL->exec_sql((boost::format ("DELETE FROM track WHERE id = '%u'") % get<guint>((*i)["id"])).str()); 
                                      pthreaddata->EntityDeleted( get<guint>((*i)["id"]), ENTITY_TRACK ) ;
                              }
                      } catch(Glib::Error) {
                                g_message(G_STRLOC ": Error while trying to test URI '%s' for presence", uri.c_str()) ;
                      }
                  }
          }
  }

  // do_remove_dangling () ;

  pthreaddata->Message.emit(_("Vacuum Process: Done")) ;

  if( do_signal )
      pthreaddata->ScanEnd.emit() ;
}
#endif // HAVE_HAL

void
MPX::LibraryScannerThread_MLibMan::update_albums(
)
{
    ThreadData * pthreaddata = m_ThreadData.get() ;

    for( IdSet_t::const_iterator i = m_ProcessedAlbums.begin(); i != m_ProcessedAlbums.end() ; ++i )
    {
        update_album( (*i) ) ;

        if( !(std::distance(m_ProcessedAlbums.begin(), i) % 50) )
        {
            pthreaddata->Message.emit((boost::format(_("Additional Metadata Update: %u of %u")) % std::distance(m_ProcessedAlbums.begin(), i) % m_ProcessedAlbums.size() ).str()) ;
        }
    }

    m_ProcessedAlbums.clear() ;
}

void
MPX::LibraryScannerThread_MLibMan::update_album(
      guint id
)
{
    ThreadData * pthreaddata = m_ThreadData.get() ;

    RowV        rows;
    std::string genre ;
    guint      quality = 0 ;

    m_SQL->get(
        rows,
        (boost::format ("SELECT DISTINCT genre FROM track WHERE album_j = %u AND genre IS NOT NULL AND genre != '' LIMIT 1") 
            % id 
        ).str()
    ); 
    if( !rows.empty() )
    {
        genre = get<std::string>(rows[0]["genre"]) ;
    }

    rows.clear() ;
    m_SQL->get(
        rows,
        (boost::format ("SELECT sum(audio_quality)/count(*) AS quality FROM track WHERE album_j = %u AND audio_quality IS NOT NULL AND audio_quality != '0'") 
            % id 
        ).str()
    ); 
    if( !rows.empty() )
    {
        quality = get<guint>(rows[0]["quality"]) ;
    }

    m_SQL->exec_sql(mprintf("UPDATE album SET album_genre = '%q', album_quality = '%u' WHERE id = '%u'", genre.c_str(), quality, id)) ;

    pthreaddata->EntityUpdated( id, ENTITY_ALBUM ) ;
}

void
MPX::LibraryScannerThread_MLibMan::quarantine_file(
      const std::string& uri
)
{
    if( mcs->key_get<bool>("library","quarantine-invalid") ) 
    {
        std::string new_dir_path  ;
        std::string new_file_path  ;

        Glib::RefPtr<Gio::File> orig = Gio::File::create_for_uri( uri )  ;

        try{
            Glib::RefPtr<Gio::File> orig_parent_dir = orig->get_parent()  ;
            new_dir_path = Glib::build_filename( mcs->key_get<std::string>("mpx","music-quarantine-path"), orig_parent_dir->get_basename() )  ;
            Glib::RefPtr<Gio::File> new_dir = Gio::File::create_for_path( new_dir_path )  ;
            Glib::RefPtr<Gio::Cancellable> cancellable = Gio::Cancellable::create() ;
            new_dir->make_directory_with_parents(cancellable)  ;
        } catch( Glib::Error & cxe ) {
        }

        try{
            new_file_path = Glib::build_filename( new_dir_path, orig->get_basename() )  ;
            Glib::RefPtr<Gio::File> target = Gio::File::create_for_path( new_file_path )  ;
            g_message("%s: Moving file to '%s'", G_STRLOC, new_file_path.c_str() )  ;
            orig->move( target )  ;
        } catch( Glib::Error & cxe )
        {
            g_message("%s: Error moving file into quarantine: %s", G_STRLOC, cxe.what().c_str() )  ;
        }
    }
}

void
MPX::LibraryScannerThread_MLibMan::on_update_statistics()
{
    ThreadData * pthreaddata = m_ThreadData.get() ;

    pthreaddata->ScanStart.emit() ;

    SQL::RowV v ;

    m_SQL->get(
          v
        , "SELECT DISTINCT id, mb_album_artist_id FROM album_artist WHERE is_mb_album_artist = 1"
    ) ;

    for( SQL::RowV::iterator i = v.begin(); i != v.end(); ++i )
    {
        try{
            MPX::XmlInstance<mmd::metadata> mmd ((boost::format("http://www.uk.musicbrainz.org/ws/1/artist/%s?type=xml") % get<std::string>((*i)["mb_album_artist_id"])).str()) ;

            std::string ls_begin, ls_end ;

            if( mmd.xml().artist().life_span().present() )
            {
                if( mmd.xml().artist().life_span().get().begin().present() )
                    ls_begin = mmd.xml().artist().life_span().get().begin().get() ;

                if( mmd.xml().artist().life_span().get().end().present() )
                    ls_end = mmd.xml().artist().life_span().get().end().get() ;
            }

            guint id = get<guint>((*i)["id"]) ;

            m_SQL->exec_sql(
                (boost::format("UPDATE album_artist SET life_span_begin = '%s', life_span_end = '%s' WHERE id = '%u'") % ls_begin % ls_end % id).str()
            ) ;

            pthreaddata->Message.emit((boost::format(_("Updating Artist Life Spans: %u / %u")) % std::distance(v.begin(), i) % v.size()).str()) ;

            pthreaddata->EntityUpdated( id, ENTITY_ALBUM_ARTIST ) ;

        } catch( ... ) {        
        }
    }

    pthreaddata->Message(_("Additional Metadata Update: Done")) ;
    pthreaddata->ScanEnd.emit() ;
}


