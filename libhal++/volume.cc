//
// libhal++ (C) GPL 2006 M. Derezynski
//

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif //HAVE_CONFIG_H

#include <libhal.h>
#include <libhal-storage.h>
#include <glib.h>

#include "refptr.hh"
#include "util.hh"
#include "types.hh"
#include "storage.hh"
#include "context.hh"

#include "volume.hh"
#include "drive.hh"

#include <string>

namespace Hal
{
  dbus_uint64_t
  Volume::get_size () 
  {
    return libhal_volume_get_size (m_volume); 
  }

  dbus_uint64_t
  Volume::get_disc_capacity ()
  {
    return libhal_volume_get_disc_capacity (m_volume); 
  }

  std::string
  Volume::get_device_file () 
  {
    return Util::wrap_string (libhal_volume_get_device_file (m_volume));
  }

  unsigned int
  Volume::get_device_major () 
  {
    return libhal_volume_get_device_major (m_volume);
  }

  unsigned int
  Volume::get_device_minor () 
  {
    return libhal_volume_get_device_minor (m_volume);
  }

  std::string
  Volume::get_fstype ()
  {
    return Util::wrap_string (libhal_volume_get_fstype (m_volume));
  }

  std::string
  Volume::get_fsversion () 
  {
    return Util::wrap_string (libhal_volume_get_fsversion (m_volume));
  }

  Hal::VolumeUsage
  Volume::get_fsusage () 
  {
    return Hal::VolumeUsage (libhal_volume_get_fsusage (m_volume));
  }

  bool
  Volume::is_mounted () 
  {
    return bool (libhal_volume_is_mounted (m_volume));
  }

  bool
  Volume::is_partition () 
  {
    return bool (libhal_volume_is_partition (m_volume));
  }

  bool
  Volume::is_disc () 
  {
    return bool (libhal_volume_is_disc (m_volume));
  }

  bool
  Volume::is_pmp () 
  {
    bool pmp = false;
    try{
        Hal::RefPtr<Hal::Device> drive = Hal::Drive::create_from_udi (m_context,get_storage_device_udi());
        if(drive->property_exists("info.category"))
        {
            std::string category = drive->get_property<std::string>("info.category");
            pmp = (!std::strcmp(category.c_str(), "portable_audio_player"));
        }
    } catch (DeviceDoesNotExistError & cxe)
        {
            g_warning("(%s:%d): Error getting parent drive for udi '%s'", __FILE__, __LINE__, get_udi().c_str());
        }
      catch (PropertyDoesNotExistError & cxe)
        {
            g_warning("(%s:%d): Checked for info.category property but doesn't seem to actually exist", __FILE__, __LINE__);
        }
      catch (UnableToProbeDeviceError & cxe)
        {
            g_warning("(%s:%d): Fail checking property info.category for '%s'", __FILE__, __LINE__, get_udi().c_str());
        }
    return pmp;
  }

  bool
  Volume::is_mounted_read_only ()
  {
    return bool (libhal_volume_is_mounted_read_only (m_volume));
  }

  std::string
  Volume::get_partition_label () 
  {
    return Util::wrap_string (libhal_volume_get_partition_label (m_volume));
  }

  std::string
  Volume::get_partition_scheme () 
  {
    return Util::wrap_string (libhal_volume_get_partition_scheme (m_volume));
  }

  std::string
  Volume::get_partition_type ()
  {
    return Util::wrap_string (libhal_volume_get_partition_type (m_volume));
  }

  std::string
  Volume::get_partition_uuid () 
  {
    return Util::wrap_string (libhal_volume_get_partition_uuid (m_volume));
  }

  Hal::StrV
  Volume::get_partition_flags () 
  {
    return StrV (libhal_volume_get_partition_flags (m_volume));
  }

  dbus_uint64_t
  Volume::get_partition_start_offset ()
  {
    return libhal_volume_get_partition_start_offset (m_volume);
  }

  dbus_uint64_t
  Volume::get_partition_media_size () 
  {
    return libhal_volume_get_partition_media_size (m_volume);
  }

  unsigned int
  Volume::get_partition_number () 
  {
    return libhal_volume_get_partition_number (m_volume);
  }

  std::string
  Volume::get_label () 
  {
    return Util::wrap_string (libhal_volume_get_label (m_volume));
  }

  std::string
  Volume::get_mount_point () 
  {
    return Util::wrap_string (libhal_volume_get_mount_point (m_volume));
  }

  std::string
  Volume::get_uuid () 
  {
    return Util::wrap_string (libhal_volume_get_uuid (m_volume));
  }

  std::string
  Volume::get_storage_device_udi () 
  {
    return Util::wrap_string (libhal_volume_get_storage_device_udi (m_volume));
  }

  std::string
  Volume::crypto_get_backing_volume_udi ()
  {
    return Util::wrap_string (libhal_volume_crypto_get_backing_volume_udi (m_volume));
  }

  std::string
  Volume::crypto_get_clear_volume_udi () 
  {
    return Util::wrap_string (libhal_volume_crypto_get_clear_volume_udi (m_context->cobj(), m_volume));
  }


  bool
  Volume::disc_has_audio ()
    throw (VolumeIsNotDiscError) 
  {
    if (!is_disc())
      throw VolumeIsNotDiscError();

    return bool (libhal_volume_disc_has_audio (m_volume));
  }

  bool
  Volume::disc_has_data ()
    throw (VolumeIsNotDiscError)
  {
    if (!is_disc())
      throw VolumeIsNotDiscError();

    return bool (libhal_volume_disc_has_data (m_volume));
  }

  bool
  Volume::disc_is_blank ()
    throw (VolumeIsNotDiscError) 
  {
    if (!is_disc())
      throw VolumeIsNotDiscError();

    return bool (libhal_volume_disc_is_blank (m_volume));
  }

  bool
  Volume::disc_is_rewritable ()
    throw (VolumeIsNotDiscError) 
  {
    if (!is_disc())
      throw VolumeIsNotDiscError();

    return bool (libhal_volume_disc_is_rewritable (m_volume));
  }

  bool
  Volume::disc_is_appendable ()
    throw (VolumeIsNotDiscError) 
  {
    if (!is_disc())
      throw VolumeIsNotDiscError();

    return bool (libhal_volume_disc_is_appendable (m_volume));
  }

  Hal::VolumeDiscType
  Volume::get_disc_type ()
    throw (VolumeIsNotDiscError) 
  {
    if (!is_disc())
      throw VolumeIsNotDiscError();

    return Hal::VolumeDiscType (libhal_volume_get_disc_type (m_volume));
  }

  bool
  Volume::should_ignore ()
  {
    return bool (libhal_volume_should_ignore (m_volume));
  }

  DiscProperties
  Volume::get_disc_properties ()
    throw (VolumeIsNotDiscError)
  {
    if (!is_disc())
      throw VolumeIsNotDiscError();

    int properties (DISC_PROPERTIES_NONE);

    if (disc_has_audio())
      properties |= DISC_HAS_AUDIO;

    if (disc_has_data())
      properties |= DISC_HAS_DATA;

    if (disc_is_blank())
      properties |= DISC_IS_BLANK;

    if (disc_is_rewritable())
      properties |= DISC_IS_REWRITABLE;

    if (disc_is_appendable())
      properties |= DISC_IS_APPENDABLE;

    return DiscProperties (properties);
  }

  ///// INIT //////////////////////////////////////////////////////////////////////

  Volume::Volume (Hal::RefPtr<Context>            context,
                  std::string           const&    udi) throw (DeviceDoesNotExistError)
      : Hal::Device (context, udi)
  {
    m_volume = libhal_volume_from_udi (context->cobj(), udi.c_str());
    if (!m_volume)
    {
      throw DeviceDoesNotExistError();
    }
  }

  Volume::Volume (Hal::RefPtr<Context>   context,
                  LibHalVolume         * volume)

      : Hal::Device (context, libhal_volume_get_udi (volume)),
        m_volume    (volume)
  {}
        
  Volume::~Volume () 
  {
    libhal_volume_free (m_volume); 
  }
  
  Hal::RefPtr<Volume>
  Volume::create_from_udi (Hal::RefPtr<Context>             context,
                           std::string            const&    udi)

                        throw (DeviceDoesNotExistError)

  {
    return Hal::RefPtr<Volume>(new Volume(context, udi));
  }


  Hal::RefPtr<Volume>
  Volume::create_from_dev (Hal::RefPtr<Context>             context,
                           std::string            const&    dev)

                        throw (DeviceDoesNotExistError)

  {
    LibHalVolume * volume = libhal_volume_from_device_file (context->cobj(), dev.c_str());
    if (volume)
    {
      return Hal::RefPtr<Volume>(new Volume(context, volume));
    }
    throw DeviceDoesNotExistError();
  }
}
