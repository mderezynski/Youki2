//  MPX
//  Copyright (C) 2005-2007 MPX development.
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
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

#ifndef _YOUKI_I_HAL__HH
#define _YOUKI_I_HAL__HH

#include "config.h"

#include <string>
#include <vector>
#include <map>
#include <set>
#ifdef HAVE_TR1
#include <tr1/unordered_map>
#else
#include <map>
#endif
#include <ctime>
#include <glib/gtypes.h>
#include <glibmm/ustring.h>
#include <boost/functional/hash.hpp>
#include "libhal++/hal++.hh"

namespace MPX
{
    struct Volume
    {
        std::string	    volume_udi;
        std::string	    device_udi;
        std::string	    label;
        guint		    size;
        std::string	    mount_point;
        std::time_t	    mount_time;
        std::string	    device_file;
        std::string	    drive_serial;
        Hal::DriveBus	    drive_bus;
        Hal::DriveType	    drive_type;
        guint		    drive_size;
        bool		    disc;

        Volume(
        )
        {}

        Volume(
              const SQL::Row&
        ) ;

        Volume(
              const Hal::RefPtr<Hal::Context>
            , const Hal::RefPtr<Hal::Volume>
        ) throw() ;
    };

    //// FIXME: OLD SHIT
    typedef std::vector<Hal::RefPtr<Hal::Volume> >                                                  VolumesHALCC_v;
    typedef std::pair<Volume, Hal::RefPtr<Hal::Volume> >                                            Volume_VolumeHALCC_pair;
    typedef std::pair<std::string, std::string>                                                     VolumeKey;
    typedef std::vector<VolumeKey>                                                                  VolumeKey_v;
#ifdef HAVE_TR1
    typedef std::tr1::unordered_map<VolumeKey , Volume_VolumeHALCC_pair, boost::hash<VolumeKey> >   Volumes;
    typedef std::tr1::unordered_map<VolumeKey , bool, boost::hash<VolumeKey> >                      VolumesMounted;
#else
    typedef std::map<VolumeKey , Volume_VolumeHALCC_pair>                                           Volumes;
    typedef std::map<VolumeKey , bool>                                                              VolumesMounted;
#endif
    typedef std::set<std::string>                                                                   MountedPaths;

    //// DONTFIXME: NEW SHIT
#ifdef HAVE_TR1
    typedef std::tr1::unordered_map<VolumeKey, guint, boost::hash<VolumeKey> >                     VolumeIdMap_t ;
    typedef std::tr1::unordered_map<guint, std::string>                                            IdMountMap_t ;
#else
    typedef std::map<VolumeKey, guint>                                                             VolumeIdMap_t ;
    typedef std::map<guint, std::string>                                                           IdMountMap_t ;
#endif

    class IHAL
    {
        public:

#include "mpx/exception.hh"
            EXCEPTION(NotInitializedError)
            EXCEPTION(NoVolumeForUriError)
            EXCEPTION(NoMountPathForVolumeError)
            EXCEPTION(InvalidVolumeSpecifiedError)

            typedef sigc::signal  <void, const Volume&>                 SignalVolume;
            typedef sigc::signal  <void, std::string, std::string >     SignalCDDAInserted;
            typedef sigc::signal  <void, std::string >                  SignalDeviceRemoved;
            typedef sigc::signal  <void, std::string >                  SignalEjected;

            IHAL() {}
            virtual ~IHAL () {}

            virtual Hal::RefPtr<Hal::Context>
            get_context() = 0 ;

            virtual void
            volumes(
                  VolumesHALCC_v&
            ) const = 0 ;

            virtual bool
            path_is_mount_path(
                  const std::string&
            ) const = 0 ;

            virtual Volume
            get_volume_for_uri(
                  const std::string&
            ) const = 0 ;

            virtual guint
            get_id_for_volume(
                  const std::string&
                , const std::string&
            ) const = 0 ;

            virtual const std::string&
            get_mount_point_for_id(
                  guint 
            ) const = 0 ;

            virtual SignalVolume&
            signal_volume_removed(
            ) = 0 ;

            virtual SignalVolume&
            signal_volume_added(
            ) = 0 ;

            virtual SignalCDDAInserted&
            signal_cdda_inserted(
            ) = 0 ;

            virtual SignalEjected&
            signal_ejected(
            ) = 0 ;

            virtual bool
            is_initialized(
            ) const = 0 ;
    };
}
#endif //!_YOUKI_HAL__HH
