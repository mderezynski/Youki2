//
// libhal++ (C) GPL 2006 M. Derezynski
//

#ifndef _HAL_CC_VOLUME_HH
#define _HAL_CC_VOLUME_HH

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif //HAVE_CONFIG_H

#include <libhal.h>
#include <libhal-storage.h>

#include "refptr.hh"
#include "types.hh"
#include "storage.hh"
#include "context.hh"
#include "device.hh"

namespace Hal
{
  class Volume : public Device
  {
    public:

#include "exception.hh"

    HALCC_EXCEPTION(VolumeIsNotDiscError)   

        static Hal::RefPtr<Volume> create_from_udi    (Hal::RefPtr<Context>  context,
                                                       std::string const&)

                                        throw (DeviceDoesNotExistError);


        static Hal::RefPtr<Volume> create_from_dev    (Hal::RefPtr<Context>  context,
                                                       std::string const&)

                                        throw (DeviceDoesNotExistError);
      
        dbus_uint64_t       get_size (); 
        dbus_uint64_t       get_disc_capacity ();
        std::string         get_device_file (); 
        unsigned int        get_device_major (); 
        unsigned int        get_device_minor (); 
        std::string         get_fstype (); 
        std::string         get_fsversion (); 
        Hal::VolumeUsage    get_fsusage (); 
        bool                is_mounted (); 

        bool                is_partition (); 
        bool                is_disc (); 
        bool                is_pmp (); // portable media player

        bool                is_mounted_read_only (); 

        std::string         get_partition_scheme (); 
        std::string         get_partition_label (); 
        unsigned int        get_partition_number (); 
        std::string         get_label (); 
        std::string         get_mount_point (); 
        std::string         get_uuid (); 
        std::string         get_storage_device_udi (); 

        std::string         crypto_get_backing_volume_udi ();
        std::string         crypto_get_clear_volume_udi (); 
        std::string         get_partition_type (); 
        std::string         get_partition_uuid (); 
        Hal::StrV           get_partition_flags (); 
        dbus_uint64_t       get_partition_start_offset (); 
        dbus_uint64_t       get_partition_media_size (); 

        bool                disc_has_audio () throw (VolumeIsNotDiscError); 
        bool                disc_has_data () throw (VolumeIsNotDiscError);
        bool                disc_is_blank () throw (VolumeIsNotDiscError); 
        bool                disc_is_rewritable () throw (VolumeIsNotDiscError); 
        bool                disc_is_appendable () throw (VolumeIsNotDiscError); 
        Hal::VolumeDiscType get_disc_type () throw (VolumeIsNotDiscError); 

        bool                should_ignore ();

        // Following API is an addition by hal++
        DiscProperties      get_disc_properties () throw (VolumeIsNotDiscError);

    private:    

        friend class Hal::RefPtr<Volume>;

        explicit Volume (Hal::RefPtr<Context>   context,
                         std::string const&     udi) throw (DeviceDoesNotExistError);

        explicit Volume (Hal::RefPtr<Context>   context,
                         LibHalVolume         * volume); 
        
        virtual ~Volume ();

        LibHalVolume * m_volume; 
  };
}
#endif //!_HAL_CC_VOLUME_HH
