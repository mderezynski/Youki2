//
// libhal++ (C) GPL 2006 M. Derezynski
//

#ifndef _HAL_CC_DRIVE_HH
#define _HAL_CC_DRIVE_HH

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
  class Drive : public Device
  {
    public:

        static Hal::RefPtr<Drive>
               create_from_udi (Hal::RefPtr<Context>  context,
                                std::string const&    udi)
               throw (DeviceDoesNotExistError);


        static Hal::RefPtr<Drive>
               create_from_dev (Hal::RefPtr<Context>  context,
                                std::string const&    udi)
               throw (DeviceDoesNotExistError);

        bool                    is_hotpluggable (); 
        bool                    uses_removable_media ();

        bool                    is_media_detected ();
        dbus_uint64_t           get_size ();
        dbus_uint64_t           get_media_size ();
        std::string             get_partition_scheme ();

        bool                    no_partitions_hint ();
        bool                    requires_eject ();
        Hal::DriveType          get_type ();
        Hal::DriveBus           get_bus ();
        Hal::DriveCdromCaps     get_cdrom_caps ();
        unsigned int            get_device_major ();
        unsigned int            get_device_minor ();
        std::string             get_type_textual ();
        std::string             get_device_file ();
        std::string             get_serial ();
        std::string             get_firmware_version ();
        std::string             get_model ();
        std::string             get_vendor ();
        std::string             get_physical_device_udi ();
        std::string             get_dedicated_icon_drive (); 
        std::string             get_dedicated_icon_volume (); 

        Hal::StrV               find_all_volumes ();

    private:    

        friend class Hal::RefPtr<Drive>;
    
        explicit Drive (Hal::RefPtr<Context>  context,
                        std::string const&    udi)

                 throw (DeviceDoesNotExistError);
                                           

        explicit Drive (Hal::RefPtr<Context>   context,
                        LibHalDrive          * drive); 
        
        virtual ~Drive ();
        
        LibHalDrive * m_drive; 
  };
}
#endif //!_HAL_CC_DRIVE_HH
