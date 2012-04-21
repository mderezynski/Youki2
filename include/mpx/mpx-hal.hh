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

#ifndef _YOUKI_HAL__HH
#define _YOUKI_HAL__HH

#include "config.h"

#include <string>
#include <glib/gtypes.h>
#include "libhal++/hal++.hh"
#include "mpx/mpx-sql.hh"
#include "mpx/mpx-services.hh"

#include "mpx/i-youki-hal.hh"

namespace MPX
{
    class HAL
    : public Service::Base
    , public IHAL
    {
        public:

            HAL ();
            ~HAL ();

            virtual Hal::RefPtr<Hal::Context>
            get_context() ;

            virtual void
            volumes(
                  VolumesHALCC_v&
            ) const ;

            virtual bool
            path_is_mount_path(
                  const std::string&
            ) const ;

            virtual Volume
            get_volume_for_uri(
                  const std::string&
            ) const ;

            virtual guint
            get_id_for_volume(
                  const std::string&
                , const std::string&
            ) const ;

            virtual const std::string&
            get_mount_point_for_id(
                  guint 
            ) const ;



            virtual SignalVolume&
            signal_volume_removed(
            ) ;

            virtual SignalVolume&
            signal_volume_added(
            ) ;

            virtual SignalCDDAInserted&
            signal_cdda_inserted(
            ) ;

            virtual SignalEjected&
            signal_ejected(
            ) ;



            virtual bool
            is_initialized(
            ) const
            {
                return m_initialized;
            }

        protected:

            bool
            hal_init(
            ) ;

            void
            cdrom_policy(
                  const Hal::RefPtr<Hal::Volume>&
            ) ;

            guint 
            volume_register(
                  const Volume&
            ) ;

            void
            volume_insert(
                  const std::string&
            ) ;

            void
            volume_remove(
                  const std::string&
            ) ;

            void
            volume_process(
                  const std::string&
            ) ;

            void
            device_condition(
                  const std::string&        /*udi*/
                , const std::string&        /*cond_name*/
                , const std::string&        /*cond_details*/
            ) ;

            void
            device_added(
                  const std::string&
            ) ;

            void
            device_removed(
                  const std::string&
            ) ;

            void
            device_property(
                  const std::string&        /*udi*/
                , const std::string&        /*key*/
                , bool                      /*is_removed*/
                , bool                      /*is_added*/
            ) ;

            SignalVolume                    signal_volume_removed_ ;
            SignalVolume                    signal_volume_added_ ;
            SignalCDDAInserted              signal_cdda_inserted_ ;
            SignalEjected                   signal_ejected_ ;

            Hal::RefPtr<Hal::Context>       m_context ;
            Volumes		                    m_volumes ;
            VolumesMounted                  m_volumes_mounted ;
            MountedPaths                    m_mounted_paths ;

            VolumeIdMap_t                   m_volume_id_map ;
            IdMountMap_t                    m_id_mount_map ;

            SQL::SQLDB                    * m_SQL ;
            bool                            m_initialized ;
    };
}
#endif //!_YOUKI_HAL__HH
