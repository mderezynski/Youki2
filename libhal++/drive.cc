//
// libhal++ (C) GPL 2006 M. Derezynski
//

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif //HAVE_CONFIG_H

#include <libhal.h>
#include <libhal-storage.h>

#include "refptr.hh"
#include "util.hh"
#include "types.hh"
#include "storage.hh"
#include "context.hh"
#include "drive.hh"

#include <string>

namespace Hal
{
  bool
  Drive::is_hotpluggable ()
  {
    return bool (libhal_drive_is_hotpluggable (m_drive));
  }

  bool
  Drive::uses_removable_media ()
  {
    return bool (libhal_drive_uses_removable_media (m_drive));
  }

  dbus_uint64_t
  Drive::get_size ()
  {
    return libhal_drive_get_size (m_drive);
  }

  bool
  Drive::is_media_detected ()
  {
    return bool (libhal_drive_is_media_detected (m_drive));
  }

  dbus_uint64_t
  Drive::get_media_size ()
  {
    return libhal_drive_get_media_size (m_drive);
  }

  std::string
  Drive::get_partition_scheme ()
  {
    return libhal_drive_get_partition_scheme (m_drive);
  }

  bool
  Drive::no_partitions_hint ()
  {
    return bool (libhal_drive_no_partitions_hint (m_drive));
  }

  bool
  Drive::requires_eject ()
  {
    return bool (libhal_drive_requires_eject (m_drive));
  }

  Hal::DriveType
  Drive::get_type ()
  {
    return Hal::DriveType (libhal_drive_get_type (m_drive));
  }

  Hal::DriveBus
  Drive::get_bus ()
  {
    return Hal::DriveBus (libhal_drive_requires_eject (m_drive));
  } 

  Hal::DriveCdromCaps
  Drive::get_cdrom_caps ()
  {
    return Hal::DriveCdromCaps (libhal_drive_get_cdrom_caps (m_drive));
  }

  unsigned int 
  Drive::get_device_major ()
  {
    return libhal_drive_get_device_major (m_drive);
  }

  unsigned int
  Drive::get_device_minor ()
  {
    return libhal_drive_get_device_minor (m_drive);
  }

  std::string
  Drive::get_type_textual ()
  {
    return Util::wrap_string (libhal_drive_get_type_textual (m_drive));
  }

  std::string
  Drive::get_device_file ()
  {
    return Util::wrap_string (libhal_drive_get_device_file (m_drive));
  }

  std::string
  Drive::get_serial ()
  {
    return Util::wrap_string (libhal_drive_get_serial (m_drive));
  }

  std::string
  Drive::get_firmware_version ()
  {
    return Util::wrap_string (libhal_drive_get_firmware_version (m_drive));
  }

  std::string
  Drive::get_model ()
  {
    return Util::wrap_string (libhal_drive_get_model (m_drive));
  }

  std::string
  Drive::get_vendor ()
  {
    return Util::wrap_string (libhal_drive_get_vendor (m_drive));
  }

  std::string
  Drive::get_physical_device_udi ()
  {
    return Util::wrap_string (libhal_drive_get_physical_device_udi (m_drive));
  }

  std::string
  Drive::get_dedicated_icon_drive () 
  {
    return Util::wrap_string (libhal_drive_get_dedicated_icon_drive (m_drive));
  }

  std::string
  Drive::get_dedicated_icon_volume () 
  {
    return Util::wrap_string (libhal_drive_get_dedicated_icon_volume (m_drive));
  }

  Hal::StrV 
  Drive::find_all_volumes ()
  {
    int n_volumes;
    return Hal::StrV (libhal_drive_find_all_volumes (m_context->cobj(), m_drive, &n_volumes));
  }

  ///// INIT //////////////////////////////////////////////////////////////////////

  Drive::Drive (Hal::RefPtr<Context>  context,
                std::string const&    udi)

                            throw (DeviceDoesNotExistError)

      : Hal::Device (context, udi)
        
  {
    m_drive = libhal_drive_from_udi (context->cobj(), udi.c_str());
    if (!m_drive)
      throw DeviceDoesNotExistError();
  }

  Drive::Drive (Hal::RefPtr<Context>   context,
                LibHalDrive          * drive)

      : Hal::Device (context, libhal_drive_get_udi (drive)),
        m_drive     (drive)

  {}
 
 
  Drive::~Drive ()
  {
    libhal_drive_free (m_drive);
  }

  Hal::RefPtr<Drive>
  Drive::create_from_udi (Hal::RefPtr<Context>  context,
                          std::string const&    udi) throw (DeviceDoesNotExistError)
  {
    return Hal::RefPtr<Drive>(new Drive(context, udi));
  }


  Hal::RefPtr<Drive>
  Drive::create_from_dev (Hal::RefPtr<Context>  context,
                          std::string const&    dev) throw (DeviceDoesNotExistError)
   
  {
    LibHalDrive * drive = libhal_drive_from_device_file (context->cobj(), dev.c_str());
    if (drive)
      return Hal::RefPtr<Drive>(new Drive(context, drive));
    throw DeviceDoesNotExistError();
  }
}
