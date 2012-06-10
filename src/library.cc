//  MPX
//  Copyright (C) 2010 MPX development.
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
#include <glibmm.h>
#include <glibmm/i18n.h>
#include <giomm.h>

#include "mpx/mpx-covers.hh"
#include "mpx/mpx-main.hh"
#include "mpx/mpx-sql.hh"
#include "mpx/mpx-uri.hh"

#include "mpx/algorithm/random.hh"
#include "mpx/metadatareader-taglib.hh"
#include "mpx/util-string.hh"

#ifdef HAVE_HAL
#include "mpx/i-youki-hal.hh"
#endif // HAVE_HAL

#include "library.hh"

#undef PACKAGE
#define PACKAGE "youki"

using namespace Glib;
using boost::get;

namespace MPX
{
        using namespace  MPX::SQL ;

        Library::Library(
        )
        : Service::Base("mpx-service-library")
        , m_Flags(0)
        {
#ifdef HAVE_HAL
                m_HAL = services->get<IHAL>("mpx-service-hal").get() ;
#endif // HAVE_HAL

                const int MLIB_VERSION_CUR = 2;
                const int MLIB_VERSION_REV = 0;
                const int MLIB_VERSION_AGE = 0;

                try{
                        m_SQL = new SQL::SQLDB(
                                (boost::format ("mpx-musicdata-%d-%d-%d")
                                    % MLIB_VERSION_CUR 
                                    % MLIB_VERSION_REV
                                    % MLIB_VERSION_AGE
                                ).str(),
                                build_filename(g_get_user_data_dir(),PACKAGE),
                                SQLDB_OPEN
                        );
                }
                catch( DbInitError& cxe )
                {
                        g_message("%s: Error Opening the DB", G_STRLOC);
                        throw;
                }


                RowV v;
                m_SQL->get( v, "SELECT flags FROM meta WHERE rowid = 1" ) ; 

                if( !v.empty() )
                {
                    m_Flags |= get<guint>(v[0]["flags"]); 
                }

                mcs->key_set<bool>("library","use-hal", bool(m_Flags & F_USING_HAL));
        }

        Library::~Library ()
        {
        }

        void
                Library::switch_mode(
                      bool          use_hal
                )
                {
                    if( !use_hal )
                    {
                        m_Flags &= ~F_USING_HAL ;
                    }
                    else
                    {
                        m_Flags |=  F_USING_HAL ;
                    }

                    execSQL((boost::format ("UPDATE meta SET flags = '%u' WHERE rowid = 1") % m_Flags).str());
                }

	void
		Library::recache_cover( SQL::Row& r )
		{
                        Track_sp t = sqlToTrack( r, false, false );

                        std::string location = get<std::string>((*(t.get()))[ATTRIBUTE_LOCATION].get()) ;
                        std::string mb_album_id;
                        std::string amazon_asin;
                        std::string album_artist;
                        std::string album;

                        if( r.count("mb_album_id"))
                                mb_album_id = get<std::string>(r["mb_album_id"]); 

                        if( r.count("amazon_asin") ) 
                                amazon_asin = get<std::string>(r["amazon_asin"]);

                        if( r.count("album_artist") )
                                album_artist = get<std::string>(r["album_artist"]);

                        if( r.count("album") )
                                album = get<std::string>(r["album"]);

                        RequestQualifier rq ;
                        rq.mbid   = mb_album_id ;
                        rq.asin   = amazon_asin ;
                        rq.uri    = location ;
                        rq.artist = album_artist ;
                        rq.album  = album ;
                        rq.id     = get<guint>(r["album_j"]) ;

                        services->get<Covers>("mpx-service-covers")->cache(
                              rq
                            , true
			    , true
                        );
		}

	void
		Library::recacheAlbumCover( guint id )
		{
                        RowV v ; 
                        getSQL(v, (boost::format("SELECT DISTINCT album_j, location, device_id, hal_volume_udi, hal_device_udi, hal_vrp, album.mb_album_id, album.amazon_asin, album_artist.album_artist, album.album FROM track JOIN album ON album_j = album.id JOIN album_artist ON album.album_artist_j = album_artist.id WHERE album_j = '%u' GROUP BY album_j") % id).str());

			if( !v.empty() )
			{
				recache_cover( v[0] ) ;
			}
		}

        bool
                Library::recache_covers_handler (SQL::RowV *v, std::size_t* position)
                {
                        Row & r = (*v)[*position]; 

			recache_cover( r ) ;

                        (*position)++;

                        if((*position) == (*v).size())
                        {
                                delete v;
                                delete position;
                                return false;
                        }

                        return true;
                }

        void
                Library::recacheCovers()
                {
                        services->get<Covers>("mpx-service-covers")->purge() ;
                        reload () ;

                        RowV * v = new RowV ;
                        getSQL(*v, "SELECT DISTINCT album_j, location, device_id, hal_volume_udi, hal_device_udi, hal_vrp, album.mb_album_id, album.amazon_asin, album_artist.album_artist, album.album "
                                   "FROM track JOIN album ON album_j = album.id JOIN album_artist ON album.album_artist_j = album_artist.id GROUP BY album_j");

                        std::size_t * position = new std::size_t ;

                        *position = 0;

                        signal_timeout().connect( sigc::bind( sigc::mem_fun( *this, &Library::recache_covers_handler ), v, position), 500);
                }

        void
                Library::reload ()
                {
//                        Signals.Reload.emit();
                }


        RowV
                Library::getTrackTags (guint id)
                {
                        char const get_f[] = "SELECT tagid, amplitude FROM tags WHERE trackid = %u" ;

                        RowV v ;
                        getSQL( v, mprintf(get_f, id) ) ; 

                        return v ;
                }

        Library::LovedHatedStatus
                Library::getTrackLovedHated (guint id)
                {
                        char const get_f[] = "SELECT loved, hated FROM track WHERE id = %u" ;

                        RowV v ;
                        getSQL(v, mprintf(get_f, id)) ; 

                        guint loved = 0 ; 
                        guint hated = 0 ; 

                        if ( v[0].count("loved") )
                            loved = get<guint>(v[0]["loved"]) ;

                        if ( v[0].count("hated") )
                            hated = get<guint>(v[0]["hated"]) ;

                        return (loved > hated) ? TRACK_LOVED : ((hated > loved) ? TRACK_HATED : TRACK_INDIFFERENT) ;
                }


        void
                Library::getMetadata (const std::string& uri, Track & track)
                {
                        services->get<MetadataReaderTagLib>("mpx-service-taglib")->get(uri, track);

                        track[ATTRIBUTE_LOCATION] = uri; 
  
#ifdef HAVE_HAL
                        if( m_Flags & F_USING_HAL )
                        {
                                try{
                                        URI u (uri);
                                        if( u.get_protocol() == URI::PROTOCOL_FILE )
                                        {
                                                try{
                                                        const Volume& volume = m_HAL->get_volume_for_uri (uri) ;

                                                        track[ATTRIBUTE_MPX_DEVICE_ID] =
                                                                m_HAL->get_id_for_volume( volume.volume_udi, volume.device_udi ) ;

                                                        track[ATTRIBUTE_VOLUME_RELATIVE_PATH] =
                                                                filename_from_uri (uri).substr (volume.mount_point.length()) ;
                                                }
                                                catch( IHAL::Exception& cxe )
                                                {
                                                        g_warning( "%s: %s", G_STRLOC, cxe.what() ); 
                                                        throw FileQualificationError((boost::format("%s: %s") % uri % cxe.what()).str());
                                                }
                                                catch( Glib::ConvertError& cxe )
                                                {
                                                        g_warning( "%s: %s", G_STRLOC, cxe.what().c_str() ); 
                                                        throw FileQualificationError((boost::format("%s: %s") % uri % cxe.what()).str());
                                                }
                                        }
                                } catch( URI::ParseError )
                                {
                                        throw FileQualificationError((boost::format("URI Parse Error: %s") % uri).str());
                                }
                        }
#endif // HAVE_HAL
                }

        void
                Library::trackSetLocation(
                      Track_sp&           t
                    , const std::string&  uri
                )
                {
                        MPX::Track& track = *(t.get()) ;

                        track[ATTRIBUTE_LOCATION] = uri;
#ifdef HAVE_HAL
                        if( m_Flags & F_USING_HAL )
                        {
                                try{
                                        URI u (uri);
                                        if( u.get_protocol() == URI::PROTOCOL_FILE )
                                        {
                                                try{
                                                        const Volume& volume = m_HAL->get_volume_for_uri( uri ) ;

                                                        track[ATTRIBUTE_MPX_DEVICE_ID] =
                                                                m_HAL->get_id_for_volume( volume.volume_udi, volume.device_udi ) ;

                                                        track[ATTRIBUTE_VOLUME_RELATIVE_PATH] =
                                                                filename_from_uri( uri ).substr( volume.mount_point.length() ) ;
                                                }
                                                catch( IHAL::Exception & cxe )
                                                {
                                                        g_warning( "%s: %s", G_STRLOC, cxe.what() ); 
                                                        throw FileQualificationError((boost::format("%s: %s") % uri % cxe.what()).str());
                                                }
                                                catch( Glib::ConvertError & cxe )
                                                {
                                                        g_warning( "%s: %s", G_STRLOC, cxe.what().c_str() ); 
                                                        throw FileQualificationError((boost::format("%s: %s") % uri % cxe.what()).str());
                                                }
                                        }
                                        else
                                        {
                                            throw FileQualificationError((boost::format("Unable to handle non-file:/// URI using HAL: %s") % uri).str());
                                        }

                                } catch( URI::ParseError )
                                {
                                    throw FileQualificationError((boost::format("URI Parse Error: %s") % uri).str());
                                }
                        }
#endif // HAVE_HAL
                }

        std::string
                Library::trackGetLocation( const Track_sp& t )
                {
                    const MPX::Track& track = *(t.get()) ;
#ifdef HAVE_HAL
                        if( m_Flags & F_USING_HAL )
                        {
                                try{
                                        const guint&	   id          = get<guint>(track[ATTRIBUTE_MPX_DEVICE_ID].get()) ;
                                        const std::string& path        = get<std::string>(track[ATTRIBUTE_VOLUME_RELATIVE_PATH].get()) ;
                                        const std::string& mount_point = m_HAL->get_mount_point_for_id( id ) ;

                                        return filename_to_uri( build_filename( Util::normalize_path(mount_point), path ) );

                                } catch( IHAL::NoMountPathForVolumeError & cxe )
                                {
                                        g_message("%s: Error: What: %s", G_STRLOC, cxe.what());
                                        throw FileQualificationError((boost::format("No available mountpoint for Track %u: %s") % get<guint>(track[ATTRIBUTE_MPX_TRACK_ID].get()) % cxe.what() ).str());
                                }
                        }
                        else
#endif // HAVE_HAL
                        {
                                return get<std::string>(track[ATTRIBUTE_LOCATION].get());
                        }
                }

        void
                Library::getSQL( RowV & rows, const std::string& sql) const
                {
                        m_SQL->get (rows, sql); 
                }

        guint
                Library::execSQL(const std::string& sql)
                {
                        return m_SQL->exec_sql(sql) ;
                }

        void
                Library::albumAddNewRating(guint id, int rating, std::string const& comment)
                {
                        RowV v;
                        getSQL(v, (boost::format ("SELECT mb_album_id FROM album WHERE id = '%u'") % id).str());
                        if( v.empty ())
                        {
                                throw std::runtime_error("Invalid ID specified");
                        }

                        std::string mbid = get<std::string>(v[0]["mb_album_id"]);

                        execSQL(
                                        mprintf(" INSERT INTO album_rating_history (mbid, rating, comment, time) VALUES ('%q', '%d', '%q', '%u') ",
                                                mbid.c_str(),
                                                rating,
                                                comment.c_str(),
                                                guint(time(NULL))
                                               ));

                        //Signals.AlbumUpdated.emit(id);
                }

        void
                Library::albumGetAllRatings(guint id, RowV & v)
                {
                        RowV v2;
                        getSQL(v2, (boost::format ("SELECT mb_album_id FROM album WHERE id = '%u'") % id).str());
                        if( v2.empty ())
                        {
                                throw std::runtime_error("Invalid ID specified");
                        }

                        std::string mbid = get<std::string>(v2[0]["mb_album_id"]);
                        getSQL(v, mprintf("SELECT * FROM album_rating_history WHERE mbid = '%q'",mbid.c_str()));
                }

        int
                Library::albumGetMeanRatingValue(guint id)
                {
                        RowV v;

                        getSQL(v, (boost::format ("SELECT mb_album_id FROM album WHERE id = '%u'") % id).str());
                        if( v.empty ())
                        {
                                throw std::runtime_error("Invalid ID specified");
                        }

                        std::string mbid = get<std::string>(v[0]["mb_album_id"]);

                        v.clear();
                        getSQL(v, mprintf("SELECT rating FROM album_rating_history WHERE mbid = '%q'",mbid.c_str()));

                        if( !v.empty() )
                        {
                                double rating = 0;

                                for(RowV::iterator i = v.begin(); i != v.end(); ++i)
                                {
                                        rating += double(get<guint>((*i)["rating"]));
                                }

                                rating /= double(v.size());

                                return rating;
                        }

                        return 0;
                }

        void
                Library::albumDeleteRating(guint rating_id, guint album_id)
                {
                        execSQL((boost::format ("DELETE FROM album_rating_history WHERE id = %u") % rating_id).str());
                        //Signals.AlbumUpdated.emit(album_id);
                }

        void
                Library::albumTagged(guint id, std::string const& tag)
                {
                        guint tag_id = get_tag_id( tag );

                        char const insert_f[] = "INSERT INTO tags_album (tagid, albumid) VALUES (%u, %u)";
                        char const update_f[] = "UPDATE tags SET amplitude = amplitude + 1 WHERE albumid = %u AND tagid = %u";
                        try{
                                execSQL(mprintf(insert_f, id, tag_id)); 
                        } catch( SqlConstraintError & cxe )
                        {
                                execSQL(mprintf(update_f, id, tag_id));
                        }
                }

        void	
                Library::trackRated(guint id, int rating)
                {
                        execSQL((boost::format ("UPDATE track SET rating = '%d' WHERE id = %u") % rating % id).str());

/*
                        RowV v;
                        getSQL(
                              v
                            , (boost::format("SELECT * FROM track_view WHERE id = '%u'") % id).str()
                        );

                        Track_sp t = sqlToTrack( v[0] );

                        guint id_album = get<guint>(v[0]["album_j"]);
                        guint id_artst = get<guint>(v[0]["album_artist_j"]);

                        Signals.TrackUpdated.emit( *(t.get()), id_album, id_artst );
*/
                }

        void	
                Library::trackPlayed(
                      const Track_sp    track
                    , time_t            time_
                )
                {
                        const MPX::Track& t = (*track.get()) ;

                        guint track_id = get<guint>(t[ATTRIBUTE_MPX_TRACK_ID].get()) ;
                        guint artist_id = get<guint>(t[ATTRIBUTE_MPX_ARTIST_ID].get()) ;
                        guint album_id = get<guint>(t[ATTRIBUTE_MPX_ALBUM_ID].get()) ;

                        execSQL((boost::format ("UPDATE track SET pdate = '%u' WHERE id = '%u'") % guint(time_) % track_id).str());
                        execSQL((boost::format ("UPDATE track SET pcount = ifnull(pcount,0) + 1 WHERE id = '%u'") % track_id).str());
                        execSQL((boost::format ("UPDATE artist SET pcount = ifnull(pcount,0) + 1 WHERE id = %u") % artist_id).str());

                        RowV v;
                        getSQL(v, (boost::format ("SELECT SUM(pcount) AS count FROM track_view WHERE album_j = '%u'") % album_id).str());
                        guint count1 = get<guint>(v[0]["count"]);

                        v.clear();
                        getSQL(v, (boost::format ("SELECT count(id) AS count FROM track_view WHERE album_j = '%u'") % album_id).str());
                        guint count2 = get<guint>(v[0]["count"]);

                        double score = double(count1) / double(count2);

                        execSQL((boost::format ("UPDATE album SET album_playscore = '%f' WHERE album.id = '%u'") % score % album_id).str());

		//        Signals.AlbumUpdated.emit( album_id ) ;
                //        Signals.TrackUpdated.emit( (*t.get()), id_album, id_artst );
                }

        void
                Library::trackTagged(guint id, std::string const& tag)
                {
                        guint tag_id = get_tag_id( tag );

                        char const insert_f[] = "INSERT INTO tags (tagid, trackid) VALUES (%u, %u)";
                        char const update_f[] = "UPDATE tags SET amplitude = amplitude + 1 WHERE trackid = %u AND tagid = %u";
                        try{
                                execSQL(mprintf(insert_f, id, tag_id)); 
                        } catch( SqlConstraintError & cxe )
                        {
                                execSQL(mprintf(update_f, id, tag_id));
                        }
                }

        void
                Library::trackLovedHated(guint id, LovedHatedStatus status ) 
                {
                        if( status == TRACK_INDIFFERENT )
                            return ;

                        guint val = 0 ;
                        SQL::RowV v ;

                        std::string col = (status == TRACK_LOVED) ? "loved" : "hated" ;

                        char const get_f[] = "SELECT %s FROM track WHERE id = %u" ;
                    
                        getSQL(v, mprintf(get_f,
                              col.c_str()
                            , id
                        ));

                        if( !v.empty() )
                        {
                            val = get<guint>(v[0][col]) ;
                        }

                        char const update_f[] = "UPDATE track SET '%s' = '%u' WHERE id = '%u'" ;

                        try{
                                execSQL(mprintf(update_f,
                                      col.c_str()
                                    , ++val
                                    , id
                                ));
                        } catch( SqlConstraintError & cxe )
                        {
                        }
                }

        void
                Library::markovUpdate(guint a, guint b)
                {
                        RowV rows;
                        getSQL (rows, (boost::format("SELECT id FROM markov WHERE track_id_a = %u AND track_id_b = %u") % a % b).str());
                        if( rows.empty ())
                        {
                                execSQL((boost::format("INSERT INTO markov (count, track_id_a, track_id_b) VALUES (1, %u, %u)") % a % b).str());
                        }
                        else
                        {
                                guint id = get <guint> (rows[0].find ("id")->second);
                                execSQL((boost::format("UPDATE markov SET count = count + 1 WHERE id = %u") % id).str());
                        }
                }

        guint
                Library::markovGetRandomProbableTrack( int a )
                {
                        try{
                                std::vector<double> ratios ;

                                RowV v ;
                                getSQL( v, (boost::format("SELECT * FROM markov WHERE track_id_a = '%u' AND count > 0") % a).str() );

                                if( !v.empty() )
                                {
                                        for( RowV::iterator i = v.begin(); i != v.end(); ++i )
                                        {
                                            ratios.push_back(double(get<guint>((*i)["count"]))/double(v.size()));
                                        }

                                        std::size_t row = MPX::rand(ratios);

                                        g_assert( row < v.size() );

                                        return get<guint>( v[row]["track_id_b"] ) ;
                                }
                        } catch( ... ) {
                        }

                        return 0;
                }

        Track_sp
                Library::sqlToTrack(
                      SQL::Row & row
                    , bool all_metadata
                    , bool no_location
                )
                {
                        Track_sp track (new Track);

                        if( !no_location )
                        {
#ifdef HAVE_HAL
                                if( m_Flags & F_USING_HAL )
                                {
                                    if( row.count("device_id") )
                                            (*track.get())[ATTRIBUTE_MPX_DEVICE_ID] = get<guint>(row["device_id"]);

                                    if( row.count("hal_vrp") )
                                            (*track.get())[ATTRIBUTE_VOLUME_RELATIVE_PATH] = get<std::string>(row["hal_vrp"]);
#if 0
                                    (*track.get())[ATTRIBUTE_LOCATION] = trackGetLocation( track ); 
                                    g_assert( (*track.get()).has(ATTRIBUTE_LOCATION) );
#endif
                                }
                                else
#endif
                                if( row.count("location") )
                                {
                                    (*track.get())[ATTRIBUTE_LOCATION] = get<std::string>(row["location"]);
                                }
                        }

                        if( row.count("id") )
                        {
                            (*track.get())[ATTRIBUTE_MPX_TRACK_ID] = get<guint>(row["id"]);
                        }

                        if( all_metadata )
                        {
                                if( row.count("album_artist") )
                                        (*track.get())[ATTRIBUTE_ALBUM_ARTIST] = get<std::string>(row["album_artist"]);

                                if( row.count("album_artist_sortname") )
                                        (*track.get())[ATTRIBUTE_ALBUM_ARTIST_SORTNAME] = get<std::string>(row["album_artist_sortname"]);

                                if( row.count("artist") )
                                        (*track.get())[ATTRIBUTE_ARTIST] = get<std::string>(row["artist"]);

                                if( row.count("album") )
                                        (*track.get())[ATTRIBUTE_ALBUM] = get<std::string>(row["album"]);

                                if( row.count("track") )
                                        (*track.get())[ATTRIBUTE_TRACK] = guint(get<guint>(row["track"]));

                                if( row.count("title") )
                                        (*track.get())[ATTRIBUTE_TITLE] = get<std::string>(row["title"]);

                                if( row.count("album_genre") )
                                        (*track.get())[ATTRIBUTE_GENRE] = get<std::string>(row["album_genre"]);

                                if( row.count("label") )
                                        (*track.get())[ATTRIBUTE_LABEL] = get<std::string>(row["label"]);

                                if( row.count("time") )
                                        (*track.get())[ATTRIBUTE_TIME] = guint(get<guint>(row["time"]));

                                if( row.count("mb_artist_id") )
                                        (*track.get())[ATTRIBUTE_MB_ARTIST_ID] = get<std::string>(row["mb_artist_id"]);

                                if( row.count("mb_track_id") )
                                        (*track.get())[ATTRIBUTE_MB_TRACK_ID] = get<std::string>(row["mb_track_id"]);

                                if( row.count("mb_album_artist_id") )
                                        (*track.get())[ATTRIBUTE_MB_ALBUM_ARTIST_ID] = get<std::string>(row["mb_album_artist_id"]);

                                if( row.count("mb_release_country") )
                                        (*track.get())[ATTRIBUTE_MB_RELEASE_COUNTRY] = get<std::string>(row["mb_release_country"]);

                                if( row.count("mb_release_type") )
                                        (*track.get())[ATTRIBUTE_MB_RELEASE_TYPE] = get<std::string>(row["mb_release_type"]);

                                if( row.count("mb_album_id") )
                                        (*track.get())[ATTRIBUTE_MB_ALBUM_ID] = get<std::string>(row["mb_album_id"]);

                                if( row.count("musicip_puid") )
                                        (*track.get())[ATTRIBUTE_MUSICIP_PUID] = get<std::string>(row["musicip_puid"]);

                                if( row.count("date") )
                                        (*track.get())[ATTRIBUTE_DATE] = get<guint>(row["date"]);

                                if( row.count("amazon_asin") )
                                        (*track.get())[ATTRIBUTE_ASIN] = get<std::string>(row["amazon_asin"]);

                                if( row.count("album_j") )
                                        (*track.get())[ATTRIBUTE_MPX_ALBUM_ID] = get<guint>(row["album_j"]);

                                if( row.count("artist_j") )
                                        (*track.get())[ATTRIBUTE_MPX_ARTIST_ID] = get<guint>(row["artist_j"]);

                                if( row.count("mpx_album_artist_id") )
                                        (*track.get())[ATTRIBUTE_MPX_ALBUM_ARTIST_ID] = get<guint>(row["mpx_album_artist_id"]);

                                if( row.count("type") )
                                        (*track.get())[ATTRIBUTE_TYPE] = get<std::string>(row["type"]);

                                if( row.count("bitrate") )
                                        (*track.get())[ATTRIBUTE_BITRATE] = get<guint>(row["bitrate"]);

                                if( row.count("samplerate") )
                                        (*track.get())[ATTRIBUTE_SAMPLERATE] = get<guint>(row["samplerate"]);

                                if( row.count("type") )
                                        (*track.get())[ATTRIBUTE_TYPE] = get<std::string>(row["type"]);

                                if( row.count("audio_quality") )
                                        (*track.get())[ATTRIBUTE_QUALITY] = get<guint>(row["audio_quality"]);

                                if( row.count("discnr") )
                                        (*track.get())[ATTRIBUTE_DISCNR] = get<guint>(row["discnr"]);
                        }

                        return track;
                }

        Track_sp
                Library::getTrackById(
                      guint id
                )
                {
                        SQL::RowV v ;
                        getSQL( v, (boost::format("SELECT * FROM track_view WHERE id = '%u'") % id).str() ) ;
                    
                        if( !v.empty() )
                        {
                            return sqlToTrack( v[0], true, false ) ;
                        }
                        else
                        {
                            throw std::runtime_error((boost::format("No result set for ID[%u]!") % id).str()) ;
                        }
                }


        guint
                Library::collectionCreate(
                    const std::string& name,
                    const std::string& blurb
                )
                {

                    static std::string 
                            collection_table_create_f ("CREATE TABLE collection_%u (track_id INTEGER NOT NULL)");

                    static std::string 
                            collection_create_f ("INSERT INTO collection (name, blurb) VALUES ('%q', '%q')");

                    execSQL("BEGIN IMMEDIATE");

                    execSQL(mprintf(collection_create_f.c_str(),
                        name.c_str(),
                        blurb.c_str()
                    ));

                    guint id = m_SQL->last_insert_rowid(); // FIXME: NOT MULTIPLE CONNECTION SAFE

                    execSQL(mprintf(collection_table_create_f.c_str(),
                            id
                    ));

                    execSQL("COMMIT");

//                    Signals.Collection.New.emit( id );
                   
                    return id; 
                }

        void
                Library::collectionDelete(
                    guint id
                )
                {

                    static std::string
                            collection_table_delete_f ("DROP TABLE collection_%u");    
                        
                    static std::string
                            collection_delete_f ("DELETE FROM collection WHERE id = '%u'");

                    execSQL("BEGIN IMMEDIATE");

                    execSQL(mprintf(collection_delete_f.c_str(),
                            id
                    ));

                    execSQL(mprintf(collection_table_delete_f.c_str(),
                            id
                    ));

                    execSQL("COMMIT");

//                    Signals.Collection.Deleted.emit( id );
                }

        void
                Library::collectionAppend(
                    guint      id,
                    const IdV&  tracks
                )
                {
                    static std::string
                            collection_append_f ("INSERT INTO collection_%u (track_id) VALUES ('%u')");

                    execSQL("BEGIN IMMEDIATE");

                    for( IdV::const_iterator i = tracks.begin(); i != tracks.end(); ++i )
                    {
                            execSQL(mprintf(collection_append_f.c_str(),
                                    id,
                                    *i
                            ));
                            // XXX: We can afford to emit the signal here even though we are in a transaction, because the tracks already exist in the library
//                            Signals.Collection.NewTrack.emit( id, *i );
                    }

                    execSQL("COMMIT");
                } 

        void
                Library::collectionErase(
                    guint      id,
                    const IdV&  tracks
                )
                {
                    static std::string
                            collection_erase_f ("DELETE FROM collection_%u WHERE track_id ='%u'");

                    for( IdV::const_iterator i = tracks.begin(); i != tracks.end(); ++i )
                    {
                            execSQL(mprintf(collection_erase_f.c_str(),
                                    id,
                                    *i
                            ));

//                            Signals.Collection.TrackDeleted.emit( id, *i );
                    }
                }

        void
                Library::collectionGetMeta(
                      guint              id
                    , CollectionMeta&     collection
                )
                {
                    static std::string
                            collection_acquire_meta ("SELECT * FROM collection WHERE id ='%u'");

                    static std::string
                            collection_acquire_tracks ("SELECT track_id FROM collection_%u");

                    SQL::RowV v;

                    getSQL(
                        v,
                        mprintf(
                            collection_acquire_meta.c_str(),
                            id
                        )
                    );

                    collection.Id = get<guint>(v[0]["id"]);

                    if( v[0].count("name") )
                        collection.Name = get<std::string>(v[0]["name"]);

                    if( v[0].count("blurb") )
                        collection.Blurb = get<std::string>(v[0]["blurb"]);

                    if( v[0].count("cover_url") )
                        collection.Cover_URL = get<std::string>(v[0]["cover_url"]);
                }

        void
                Library::collectionGetAll(
                    IdV&   collections
                )
                {
                    static std::string
                           collections_get_f ("SELECT id FROM collection");

                    SQL::RowV v;

                    getSQL(
                        v,
                        collections_get_f.c_str()
                    );
                         
                    for( SQL::RowV::iterator i = v.begin(); i != v.end(); ++i )   
                    {
                        collections.push_back(get<guint>((*i)["id"]));
                    }
                }

        void
                Library::collectionGetTracks(
                    guint          id,
                    IdV&            tracks
                )
                {
                    static std::string
                            collection_acquire_tracks ("SELECT track_id FROM collection_%u");

                    SQL::RowV v;

                    getSQL(
                        v,
                        mprintf(
                            collection_acquire_tracks.c_str(),
                            id
                        )
                    );

                    for( SQL::RowV::iterator i = v.begin(); i != v.end(); ++i )
                    {
                        tracks.push_back( get<guint>((*i)["track_id"]));
                    }
                }

        guint
                Library::get_tag_id (std::string const& tag)
                {
                        guint tag_id = 0;
                        RowV rows;
                        std::string sql;

                        get_tag_id:

                        char const* select_tag_f ("SELECT id FROM tag WHERE tag = '%q'"); 
                        sql = mprintf(select_tag_f, tag.c_str());
                        m_SQL->get (rows, sql); 

                        if(!rows.empty())
                        {
                                tag_id = get <guint> (rows[0].find ("id")->second);
                        }
                        else
                        {
                                char const* set_tag_f ("INSERT INTO tag (tag) VALUES ('%q')");
                                sql = mprintf(set_tag_f, tag.c_str());
                                m_SQL->exec_sql (sql);
                                goto get_tag_id; // threadsafe, as opposed to getting last insert rowid
                        }
                        g_assert(tag_id != 0);
                        return tag_id;
                }
} // namespace MPX
