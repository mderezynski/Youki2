//  MPX - The Dumb Music Player
//  Copyright (C) 2007 MPX Project 
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
#ifndef MPX_LIBRARY_TYPES_HH
#define MPX_LIBRARY_TYPES_HH

#include "config.h"
#include <glibmm.h>
#include <string>
#include <map>
#include <string>
#include <vector>
#include <set>
#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/interprocess/containers/stable_vector.hpp>

namespace MPX
{
    enum PlayStatus
    {
      PLAYSTATUS_NONE    = 0,
      PLAYSTATUS_STOPPED = 1 << 0,
      PLAYSTATUS_PLAYING = 1 << 1,
      PLAYSTATUS_PAUSED  = 1 << 2,
      PLAYSTATUS_SEEKING = 1 << 3,
      PLAYSTATUS_WAITING = 1 << 4
    };

    enum AttributeIdString
    {
        ATTRIBUTE_LOCATION
      , ATTRIBUTE_TITLE

      , ATTRIBUTE_GENRE
      , ATTRIBUTE_COMMENT
      , ATTRIBUTE_LABEL

      , ATTRIBUTE_MUSICIP_PUID

      , ATTRIBUTE_HASH     
      , ATTRIBUTE_MB_TRACK_ID 

      , ATTRIBUTE_ARTIST
      , ATTRIBUTE_ARTIST_SORTNAME
      , ATTRIBUTE_MB_ARTIST_ID

      , ATTRIBUTE_ALBUM
      , ATTRIBUTE_MB_ALBUM_ID
      , ATTRIBUTE_MB_RELEASE_DATE
      , ATTRIBUTE_MB_RELEASE_COUNTRY
      , ATTRIBUTE_MB_RELEASE_TYPE
      , ATTRIBUTE_ASIN

      , ATTRIBUTE_ALBUM_ARTIST
      , ATTRIBUTE_ALBUM_ARTIST_SORTNAME
      , ATTRIBUTE_MB_ALBUM_ARTIST_ID

      // MIME type
      , ATTRIBUTE_TYPE

      // HAL
      , ATTRIBUTE_HAL_VOLUME_UDI
      , ATTRIBUTE_HAL_DEVICE_UDI
      , ATTRIBUTE_VOLUME_RELATIVE_PATH
        
      // SQL
      , ATTRIBUTE_INSERT_PATH
      , ATTRIBUTE_LOCATION_NAME

      , N_ATTRIBUTES_STRING
    };

    enum AttributeIdInt
    {
        ATTRIBUTE_DISCTOTAL = N_ATTRIBUTES_STRING
      , ATTRIBUTE_DISCNR
      , ATTRIBUTE_DISCS
      , ATTRIBUTE_TRACK
      , ATTRIBUTE_TIME
      , ATTRIBUTE_RATING
      , ATTRIBUTE_DATE
      , ATTRIBUTE_MTIME
      , ATTRIBUTE_BITRATE
      , ATTRIBUTE_SAMPLERATE
      , ATTRIBUTE_COUNT
      , ATTRIBUTE_PLAYDATE
      , ATTRIBUTE_INSERT_DATE  
      , ATTRIBUTE_IS_MB_ALBUM_ARTIST

      , ATTRIBUTE_ACTIVE
      , ATTRIBUTE_QUALITY

      , ATTRIBUTE_MPX_DEVICE_ID

      , ATTRIBUTE_MPX_TRACK_ID
      , ATTRIBUTE_MPX_ALBUM_ID
      , ATTRIBUTE_MPX_ARTIST_ID
      , ATTRIBUTE_MPX_ALBUM_ARTIST_ID

      , ATTRIBUTE_IS_COMPILATION // only for transferring tcmp info from the metadata plugins; not in track SQL

      , N_ATTRIBUTES_INT
    };

    typedef std::set<std::string>				StrS ;
    typedef boost::variant<guint, gdouble, std::string, StrS>	Variant ;
    typedef boost::optional<Variant>				OVariant ;

    struct TracksConstraint
    {
	std::size_t	Count ;
	std::size_t	Time ;

	TracksConstraint() : Count(0), Time(0) {}
    } ;

    typedef std::vector<TracksConstraint> TCVector_t ;
    typedef boost::shared_ptr<TCVector_t> TCVector_sp ;

    enum VariantType
    {
          VT_INT
        , VT_FLOAT
        , VT_STRING
        , VT_STRSET
    };

    class Track
    {
          typedef std::vector<OVariant> Data_t ;
          
          Data_t data ;
    
        public:

          const OVariant& operator[](Data_t::size_type index) const
          {
              return data[index];
          }

          OVariant& operator[](Data_t::size_type index)
          {
              return data[index];
          }

          Track () { data = Data_t (Data_t::size_type(N_ATTRIBUTES_INT)); }

          bool
          has(Data_t::size_type index) const
          {
            return bool(data[index]);
          }
    };

    typedef boost::shared_ptr<Track> Track_sp;
    typedef std::vector<Track_sp>    Track_sp_v ;

    namespace SQL
    {
        enum ValueType
        {
              VALUE_TYPE_INT
            , VALUE_TYPE_REAL
            , VALUE_TYPE_STRING  
            , VALUE_TYPE_BLOB
        }; 

        // NOTE: Regrettably, this must be a std::map. Although more
        // efficient, std::tr1::unordered_map cannot be exported to
        // Python yet
        typedef std::map<std::string, Variant>              Row ;
        typedef std::vector<Row>                            RowV ;
        typedef std::vector<std::string>                    ColumnV ;

    } // namespace SQL

    typedef std::vector<unsigned int> IdV;

} //namespace MPX 

#endif //!MPX_LIBRARY_TYPES_HH
