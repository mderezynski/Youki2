//  MPX
//  Copyright (C) 2005-2007 MPX Project 
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
#ifndef MPX_LIBRARY_CLASS_HH
#define MPX_LIBRARY_CLASS_HH

#include "mpx/mpx-sql.hh"
#include "mpx/mpx-types.hh"
#include "mpx/mpx-services.hh"
#include "mpx/util-file.hh"
#include "mpx/util-string.hh"

#include "mpx/i-youki-library.hh"

namespace MPX
{
    struct CollectionMeta
    {
        std::string Name;
        std::string Blurb;
        std::string Cover_URL;
        guint      Id;
    };

    class MetadataReaderTagLib;
    class Library
    : public ILibrary
    , public Service::Base
    {
        public:

#include "mpx/exception.hh"

            EXCEPTION(FileQualificationError)

            Library(
            ) ;

            virtual ~Library () ;

            void
            switch_mode(
                  bool
            ) ;

            guint
            get_flags(
            )
            {
                return m_Flags;
            }

            SQL::SQLDB*
            get_sql_db(
            )
            {
                return m_SQL;
            }

            void
            reload(
            ) ;

            void
            getSQL(
                SQL::RowV&,
                const std::string&
            ) const ;

			guint
			execSQL(
                const std::string&
            ) ;

            Track_sp 
            sqlToTrack(
                  SQL::Row&
                , bool /*all metadata?*/ = true
                , bool /*no_location?*/  = false
            ) ;

            Track_sp
            getTrackById(
                  guint
            ) ;

            void
            getMetadata(
                  const std::string&
                , Track&
            ) ;
            void
            albumAddNewRating(
                  guint
                , int
                , const std::string&
            ) ;
            void
            albumGetAllRatings(
                  guint
                , SQL::RowV& v
            ) ;
            int
            albumGetMeanRatingValue(
                  guint
            ) ;
            void
            albumDeleteRating(
                  guint
                , guint
            ) ;
            void
            albumTagged(
                  guint
                , const std::string&
            ) ;

			void
			trackRated(
                  guint
                , int
            ) ;
			void
			trackPlayed(
                 const Track_sp
               , time_t
            ) ;
            void
            trackTagged(
                  guint
                , const std::string&
            ) ;

            enum LovedHatedStatus
            {
                  TRACK_LOVED
                , TRACK_HATED
                , TRACK_INDIFFERENT
            } ;

            void
            trackLovedHated(
                  guint
                , LovedHatedStatus 
            ) ;

            void
            trackSetLocation( Track_sp&, const std::string& );

            std::string
            trackGetLocation( const Track_sp& );

            SQL::RowV
            getTrackTags(
                guint
            ) ;

            LovedHatedStatus
            getTrackLovedHated(
                guint
            ) ;

	    const std::string&
	    get_uuid()
	    {
		return m_UUID ;
	    }

            void
            markovUpdate(guint /* track a */, guint /* track b */) ;

            guint 
            markovGetRandomProbableTrack(int /* track a*/); 

            guint
            collectionCreate(const std::string& /*name*/, const std::string& /*blurb*/) ;
    
            void
            collectionDelete(guint /*id*/) ;

            void
            collectionAppend(guint /*id*/, const IdV&) ;

            void
            collectionErase(guint /*id*/, const IdV&) ;

            void
            collectionGetMeta(guint /*id*/, CollectionMeta& /*collection_meta*/) ;

            void
            collectionGetAll(IdV& /*collections*/) ;

            void
            collectionGetTracks(guint id, IdV& /*collections*/) ;

        public:

            typedef sigc::signal<void,
                guint /*album id*/>                            SignalNewAlbum; 

            typedef sigc::signal<void,
                guint /*artist id*/>                           SignalNewArtist;

            typedef sigc::signal<void,
                Track&,
                guint /*albumid*/,
                guint/*artistid*/>                             SignalNewTrack;

            typedef sigc::signal<void,
                guint /*collection id*/>                       SignalNewCollection; 

            typedef sigc::signal<void,
                guint /*collection id*/,
                guint /*track id*/>                            SignalNewCollectionTrack;



            typedef sigc::signal<void,
                guint /*collection id*/>                       SignalCollectionDeleted; 

            typedef sigc::signal<void,
                guint /*album id*/>                            SignalAlbumDeleted; 

            typedef sigc::signal<void,
                guint/*artistid*/>                             SignalAlbumArtistDeleted;

            typedef sigc::signal<void,
                guint/*artistid*/>                             SignalArtistDeleted;

            typedef sigc::signal<void,
                guint /*collection id*/,
                guint /*track id*/>                            SignalCollectionTrackDeleted;

            typedef sigc::signal<void,
                guint/*artistid*/>                             SignalTrackDeleted;


			typedef sigc::signal<void,
                Track&,
                guint /*album_id*/,
                guint /*artist id*/>                           SignalTrackUpdated;

            typedef sigc::signal<void,
                guint /*album id*/>                            SignalAlbumUpdated;


			typedef sigc::signal<void,
                guint /*id*/,
                guint /*tagid*/>                               SignalTrackTagged;



            typedef sigc::signal<void>                         SignalScanStart;

            typedef sigc::signal<void,
                guint,
                guint>                                         SignalScanRun;

            typedef sigc::signal<void,
                guint,
                guint,
                guint,
                guint,
                guint>                                         SignalScanEnd;

            typedef sigc::signal<void>                          SignalReload;

/*
            struct CollectionSignalsT
            {
                SignalNewCollection             New; 
                SignalCollectionDeleted         Deleted;
                SignalNewCollectionTrack        NewTrack;
                SignalCollectionTrackDeleted    TrackDeleted;
            };
*/

            struct SignalsT
            {
                SignalAlbumUpdated          AlbumUpdated ;
            };

            SignalsT Signals ;

            SignalAlbumUpdated&
            signal_album_updated()
            { return Signals.AlbumUpdated ; }
            
        public:

            enum Flags
            {
                  F_NONE            = 0
            };

        private:

            SQL::SQLDB  * m_SQL ;
            guint	  m_Flags ;
	    std::string	  m_UUID ;

        protected:

            guint
            get_tag_id(
                std::string const&
            ) ;
    };
} // namespace MPX

#endif // !MPX_LIBRARY_CLASS_HH
