//
// libhal++ (C) GPL 2006 M. Derezynski
//

#include <iostream>

#include <libhal.h>
#include "refptr.hh"
#include "context.hh"
#include "device.hh"
#include "hcc-macros.hh"

#define CHECK_DBUS_ERROR(); \
    if (dbus_error_is_set (&error)) \
    { \
      std::string message = error.message; \
      dbus_error_free (&error); \
      throw UnableToProbeDeviceError (message); \
    } \

#define INIT_DBUS_ERROR(); \
    DBusError error; \
    dbus_error_init (&error);

namespace Hal
{
  Device::SignalUDI_Capability&
  Device::signal_new_capability ()
  {
    if (HCC_UNLIKELY(!m_signals_accessed))
    {
      m_context->add_property_watch (get_udi());
      m_signals_accessed = true;
    }

    return signal_device_new_capability_;
  }

  Device::SignalUDI_Capability&
  Device::signal_lost_capability ()
  {
    if (HCC_UNLIKELY(!m_signals_accessed))
    {
      m_context->add_property_watch (get_udi());
      m_signals_accessed = true;
    }

    return signal_device_lost_capability_;
  }

  Device::SignalUDI_Property_Modified&
  Device::signal_property_modified ()
  {
    if (HCC_UNLIKELY(!m_signals_accessed))
    {
      m_context->add_property_watch (get_udi());
      m_signals_accessed = true;
    }

    return signal_device_property_modified_;
  }

  Device::SignalUDI_Device_Condition&
  Device::signal_condition ()
  {
    if (HCC_UNLIKELY(!m_signals_accessed))
    {
      m_context->add_property_watch (get_udi());
      m_signals_accessed = true;
    }

    return signal_device_condition_;
  }

  ///// PROPERTY SET VALUE GETS ////////////////////////////////////////////////

  template<>
  std::string   Device::PropertySet::iterator::get_value  ()
  {
    if (get_type() != PROPERTY_TYPE_STRING)
      throw InvalidKeyTypeRequest();

    return Util::wrap_string ((const char*)libhal_psi_get_string (&i));
  }

  template<>
  dbus_int32_t  Device::PropertySet::iterator::get_value  ()
  {
    if (get_type() != PROPERTY_TYPE_INT32)
      throw InvalidKeyTypeRequest();

    return libhal_psi_get_int (&i);
  }

  template<>
  dbus_uint64_t Device::PropertySet::iterator::get_value  ()
  {
    if (get_type() != PROPERTY_TYPE_UINT64)
      throw InvalidKeyTypeRequest();

    return libhal_psi_get_uint64 (&i);
  }

  template<>
  double        Device::PropertySet::iterator::get_value  ()
  {
    if (get_type() != PROPERTY_TYPE_DOUBLE)
      throw InvalidKeyTypeRequest();

    return libhal_psi_get_double (&i);
  }

  template<>
  bool          Device::PropertySet::iterator::get_value  ()
  {
    if (get_type() != PROPERTY_TYPE_BOOLEAN)
      throw InvalidKeyTypeRequest();

    return bool (libhal_psi_get_bool (&i));
  }

  template<>
  StrV  Device::PropertySet::iterator::get_value  ()
  {
    if (get_type() != PROPERTY_TYPE_STRLIST)
      throw InvalidKeyTypeRequest();

    return StrV (const_cast<const char**>(libhal_psi_get_strlist (&i)));
  }

  ///// PROPGETS ///////////////////////////////////////////////////////////////

  template<>
  StrV Device::get_property (std::string const& property)
  {
    if (!property_exists (property))
      throw PropertyDoesNotExistError();
    
    INIT_DBUS_ERROR();

    char ** P = libhal_device_get_property_strlist
      (m_context->cobj(), m_udi.c_str(), property.c_str(), &error);

    CHECK_DBUS_ERROR();

    return StrV (const_cast<const char**>(P));
  }

  template<>
  bool Device::get_property (std::string const& property)
  {
    if (!property_exists (property))
      throw PropertyDoesNotExistError();
    
    INIT_DBUS_ERROR();

    bool P = libhal_device_get_property_bool
      (m_context->cobj(), m_udi.c_str(), property.c_str(), &error);

    CHECK_DBUS_ERROR();

    return P;
  }

  template<>
  double Device::get_property (std::string const& property)
  {
    if (!property_exists (property))
      throw PropertyDoesNotExistError();

    INIT_DBUS_ERROR();
    
    double P = libhal_device_get_property_double
      (m_context->cobj(), m_udi.c_str(), property.c_str(), &error);

    CHECK_DBUS_ERROR();

    return P;
  }

  template<>
  dbus_uint64_t Device::get_property (std::string const& property)
  {
    if (!property_exists (property))
      throw PropertyDoesNotExistError();
    
    INIT_DBUS_ERROR();

    dbus_uint64_t P = libhal_device_get_property_uint64
      (m_context->cobj(), m_udi.c_str(), property.c_str(), &error);

    CHECK_DBUS_ERROR();

    return P;
  }

  template<>
  dbus_int32_t Device::get_property (std::string const& property)
  {
    if (!property_exists (property))
      throw PropertyDoesNotExistError();
    
    INIT_DBUS_ERROR();

    dbus_int32_t P = libhal_device_get_property_int
      (m_context->cobj(), m_udi.c_str(), property.c_str(), &error);

    CHECK_DBUS_ERROR();

    return P;
  }

  template<> 
  std::string Device::get_property (std::string const& property)
  {
    if (!property_exists (property))
      throw PropertyDoesNotExistError();
    
    INIT_DBUS_ERROR();

    char * P = libhal_device_get_property_string
      (m_context->cobj(), m_udi.c_str(), property.c_str(), &error);

    CHECK_DBUS_ERROR();

    return Util::wrap_string (P); 
  }

  ///////////////////////////////////////////////////////////////////////////// 

  PropertyType
  Device::get_property_type  (std::string const& property)
  {
    
    INIT_DBUS_ERROR();

    PropertyType prop_type = PropertyType (libhal_device_get_property_type (m_context->cobj(),
                get_udi().c_str(), property.c_str(), &error));
                
    CHECK_DBUS_ERROR();

    return prop_type;
  }

  bool
  Device::property_exists (std::string const& property)
  {
    
    INIT_DBUS_ERROR();

    bool exists (libhal_device_property_exists (m_context->cobj(), m_udi.c_str(), property.c_str(), &error));

    CHECK_DBUS_ERROR();

    return exists;
  }

  Device::PropertySet
  Device::get_all_properties ()
  {
    return PropertySet (m_context, m_udi);
  }

  Device::PropertySet::~PropertySet ()
  {
    libhal_free_property_set (m_set);
  }

  Device::PropertySet::PropertySet (Hal::RefPtr<Context>          context,
                                    std::string           const&  udi)
  {
    DBusError error;
    dbus_error_init (&error);

    m_set = libhal_device_get_all_properties
      (context->cobj(), udi.c_str(), &error);

    // begin ()
    libhal_psi_init (&m_begin.i, m_set);

    // end ()
    libhal_psi_init (&m_end.i, m_set);
    while (libhal_psi_has_more (&m_end.i))
      libhal_psi_next (&m_end.i);
  }

  ///// INIT //////////////////////////////////////////////////////////////////

  Device::Device (Hal::RefPtr<Context> _context, std::string const& _udi)

    : m_context           (_context),
      m_udi               (_udi),
      m_signals_accessed  (false)

  {
    try{
        if (!m_context->device_exists (_udi))
        {
          throw DeviceDoesNotExistError();
        }
      }
    catch (UnableToProbeDeviceError)
      {
        throw DeviceDoesNotExistError(); //FIXME: Not _quite_ right
      }
  
    _context->device_created (this);
  }
  
  Device::~Device ()
  {
  }

  Hal::RefPtr<Device>
  Device::create (Hal::RefPtr<Context> context, std::string const& udi)
    throw (DeviceDoesNotExistError)
  {
    Hal::Device * device = context->get_registered_device (udi);

    if (device)
      return Hal::RefPtr<Device> (device);
    else
      return Hal::RefPtr<Device> (new Device(context, udi));
  }
}
