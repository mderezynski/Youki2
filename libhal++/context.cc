//
// libhal++ (C) GPL 2006 M. Derezynski
//

#include <iostream>

#include <dbus/dbus.h>
#include <libhal.h>

#include "types.hh"
#include "refptr.hh"
#include "context.hh"

#define INSIDE_HAL_CC_CONTEXT

#include "device.hh"

#undef  INSIDE_HAL_CC_CONTEXT

namespace Hal
{
  ///// DEVICE PROPAGATE SIGNALS ///////////////////////////////////////////////

  void
  Context::_ccallback_libhal_device_new_capability (LibHalContext * ctx, const char * udi, const char * capability)
  {
    Hal::Context * ccobj = reinterpret_cast<Hal::Context *>(libhal_ctx_get_user_data (ctx));
    MDevices::iterator i = ccobj->m_devices.find (std::string(udi));
    if (i != ccobj->m_devices.end())
      i->second->signal_device_new_capability_.emit (std::string (udi), std::string(capability));
  }

  void
  Context::_ccallback_libhal_device_lost_capability (LibHalContext * ctx, const char * udi, const char * capability)
  {
    Hal::Context * ccobj = reinterpret_cast<Hal::Context *>(libhal_ctx_get_user_data (ctx));
    MDevices::iterator i = ccobj->m_devices.find (std::string(udi));
    if (i != ccobj->m_devices.end())
      i->second->signal_device_lost_capability_.emit (std::string (udi), std::string(capability));
  }

  void
  Context::_ccallback_libhal_device_property_modified (LibHalContext * ctx, const char * udi, const char * key,
                                                  dbus_bool_t is_removed, dbus_bool_t is_added)
  {
    Hal::Context * ccobj = reinterpret_cast<Hal::Context *>(libhal_ctx_get_user_data (ctx));
    MDevices::iterator i = ccobj->m_devices.find (std::string(udi));
    if (i != ccobj->m_devices.end())
      i->second->signal_device_property_modified_.emit (std::string (udi), std::string(key), bool(is_removed), bool(is_added));
  }

  void
  Context::_ccallback_libhal_device_condition (LibHalContext * ctx, const char * udi, const char * condition_name,
                                                  const char * condition_detail) 
  {
    Hal::Context * ccobj = reinterpret_cast<Hal::Context *>(libhal_ctx_get_user_data (ctx));
    MDevices::iterator i = ccobj->m_devices.find (std::string(udi));
    if (i != ccobj->m_devices.end())
      i->second->signal_device_condition_.emit (std::string (udi), std::string(condition_name), std::string(condition_detail)); 
  }

  ///// SIGNALS ///////////////////////////////////////////////////////////////////////////

  Context::SignalUDI&
  Context::signal_device_added ()
    throw (CallbackSetupError)
  {
    // We don't connect to the libhal callback at class instantiation as it would impose 
    // an overhead in case no one ever needs this during the lifetime of the object
    if (!signals[DEVICE_ADDED])
    {
      if (!libhal_ctx_set_device_added (_cobj, _ccallback_libhal_device_added))
        throw CallbackSetupError(std::string());
      signals[DEVICE_ADDED] = true;
    }
    return signal_device_added_;
  }
  void
  Context::_ccallback_libhal_device_added (LibHalContext * ctx, const char * udi)
  {
    Hal::Context * ccobj = reinterpret_cast<Hal::Context *>(libhal_ctx_get_user_data (ctx));
    ccobj->signal_device_added_.emit (std::string (udi));
  }

  Context::SignalUDI&
  Context::signal_device_removed ()
    throw (CallbackSetupError)
  {
    // We don't connect to the libhal callback at class instantiation as it would impose 
    // an overhead in case no one ever needs this during the lifetime of the object
    if (!signals[DEVICE_REMOVED])
    {
      if (!libhal_ctx_set_device_removed (_cobj, _ccallback_libhal_device_removed))
        throw CallbackSetupError(std::string());
      signals[DEVICE_REMOVED] = true;
    }
    return signal_device_removed_;
  }
  void
  Context::_ccallback_libhal_device_removed (LibHalContext * ctx, const char * udi)
  {
    Hal::Context * ccobj = reinterpret_cast<Hal::Context *>(libhal_ctx_get_user_data (ctx));
    ccobj->signal_device_removed_.emit (std::string (udi));
  }

  ///// METHODS ///////////////////////////////////////////////////////////////////////////

  Hal::StrV 
  Context::find_device_by_capability (std::string const& capability)
    throw (HALGenericError)
  {
    DBusError error;
    dbus_error_init (&error);

    int n_devices;
    char ** devices = libhal_find_device_by_capability (_cobj, capability.c_str(), &n_devices, &error);

    if (!dbus_error_is_set (&error))
    {
      return StrV (devices); 
    }

    std::string message = error.message;
    dbus_error_free (&error);
    throw HALGenericError (message);
  }
 
  bool
  Context::device_exists (std::string const& udi)
    throw (UnableToProbeDeviceError)
  {
    DBusError error;
    dbus_error_init (&error);

    bool exists (libhal_device_exists (_cobj, udi.c_str(), &error));
    if (!dbus_error_is_set (&error))
      return exists;

    std::string message = error.message;
    dbus_error_free (&error);
    throw UnableToProbeDeviceError (message);
  }

  bool
  Context::device_query_capability
      (std::string const& udi, std::string const& capability)
    throw (UnableToProbeDeviceError)
  {
    DBusError error;
    dbus_error_init (&error);

    bool has_cap (libhal_device_query_capability
      (_cobj, udi.c_str(), capability.c_str(), &error));

    if (!dbus_error_is_set (&error))
      return has_cap;

   std::string message = error.message;
   dbus_error_free (&error);
   throw UnableToProbeDeviceError (message);
  }

  StrV 
  Context::get_all_devices ()
    throw (UnableToGetDevicesError)
  {
    DBusError error;
    dbus_error_init (&error);

    int n_devices = 0;
    char ** _devices = libhal_get_all_devices (_cobj, &n_devices, &error);

    if (!dbus_error_is_set (&error))
    {
      return StrV (_devices);
    }

    std::string message = error.message;
    dbus_error_free (&error);
    throw UnableToGetDevicesError (message);
  }

  void
  Context::property_watch_all ()
    throw (WatchSetupError)
  {
    DBusError error;
    dbus_error_init (&error);
    
    libhal_device_property_watch_all (_cobj, &error);

    if (!dbus_error_is_set (&error))
      return;

    std::string message = error.message;
    dbus_error_free (&error);
    throw WatchSetupError(message);
  }

  Hal::Device * 
  Context::get_registered_device (std::string const& udi)
  {
    MDevices::const_iterator d = m_devices.find (udi);

    if (d != m_devices.end())
      return d->second;
    else
      return 0;
  }

  Context::DestroyData::DestroyData (Hal::Context * context, std::string const& udi)
    : m_context (context), m_udi (udi)
  {}

  void
  Context::device_created (Hal::Device * device)
  {
    if (m_devices.find (device->get_udi()) != m_devices.end())
      return;

    try{
        //add_property_watch (device->get_udi());
        m_devices.insert (std::make_pair (device->get_udi(), device));
        /*
        device->add_destroy_notify_callback
            (reinterpret_cast<void*>(new DestroyData (this, device->get_udi())),
             &Hal::Context::device_destroyed);
        */
      }
    catch (WatchSetupError)
      {
        std::cerr << "Failed to set up property watch for udi " << device->get_udi() << std::endl; 
      }
  }

  void *
  Context::device_destroyed (void * p)
  {
    DestroyData * data = reinterpret_cast < DestroyData *> (p);

    MDevices::iterator i = data->m_context->m_devices.find (data->m_udi);
    if (i != data->m_context->m_devices.end())
    {
      try{
          data->m_context->remove_property_watch (data->m_udi);
          data->m_context->m_devices.erase (i);
        }
      catch (WatchSetupError)
        {
          std::cerr << "Failed to clear property watch for udi '" << data->m_udi << "'" << std::endl;
        }
    }
    delete data;
    return 0;
  }

  void
  Context::add_property_watch (std::string const& udi) const
    throw (WatchSetupError)
  {
    DBusError error;
    dbus_error_init (&error);
    
    libhal_device_add_property_watch (_cobj, udi.c_str(), &error);

    if (!dbus_error_is_set (&error))
      return;

    std::string message = error.message;
    dbus_error_free (&error);
    throw WatchSetupError(message);
  }

  void
  Context::remove_property_watch (std::string const& udi) const
    throw (WatchSetupError)
  {
    DBusError error;
    dbus_error_init (&error);
    
    libhal_device_remove_property_watch (_cobj, udi.c_str(), &error);

    if (!dbus_error_is_set (&error))
      return;

    std::string message = error.message;
    dbus_error_free (&error);
    throw WatchSetupError(message);
  }

  void
  Context::set_use_cache (bool use_cache)
  {
    libhal_ctx_set_cache (_cobj, use_cache);
  }

  ///// FILTERING //////////////////////////////////////////////////////////////

  template<>
  void    Context::filter_device_list  (StrV & in,
                std::string const& property, std::string value)
  {
    StrV out;

    for (StrV::const_iterator n = in.begin() ; n != in.end() ; ++n)
    {
      DBusError error;
      dbus_error_init (&error);

      if (libhal_device_property_exists (_cobj, n->c_str(), property.c_str(), &error))
      {

        if (!dbus_error_is_set (&error) &&
            (Util::wrap_string (libhal_device_get_property_string
                          (_cobj, n->c_str(), property.c_str(), &error)) == value))
          {
            if (!dbus_error_is_set (&error))
            {
              out.push_back (*n);
              continue;
            }
          }

        if (dbus_error_is_set (&error))
          dbus_error_free (&error);
      }
    }
    std::swap(in,out);
  }

  template<>
  void    Context::filter_device_list  (StrV & in,
            std::string const& property, dbus_int32_t value)
  {
    StrV out;

    for (StrV::const_iterator n = in.begin() ; n != in.end() ; ++n)
    {
      DBusError error;
      dbus_error_init (&error);

      if (libhal_device_property_exists (_cobj, n->c_str(),
                                      property.c_str(), &error))
      {

        if (!dbus_error_is_set (&error) &&
            (libhal_device_get_property_int
              (_cobj, n->c_str(),property.c_str(), &error) == value))
          {
            if (!dbus_error_is_set (&error))
            {
              out.push_back (*n);
              continue;
            }
          }

        if (dbus_error_is_set (&error))
          dbus_error_free (&error);
      }
    }
    std::swap (in, out);
  }

  template<>
  void    Context::filter_device_list  (StrV & in,
                    std::string const& property, dbus_uint64_t value)
  {
    StrV out;

    for (StrV::const_iterator n = in.begin() ; n != in.end() ; ++n)
    {
      DBusError error;
      dbus_error_init (&error);

      if (libhal_device_property_exists
            (_cobj, n->c_str(), property.c_str(), &error))
      {

        if (!dbus_error_is_set (&error) &&
            (libhal_device_get_property_uint64
                (_cobj, n->c_str(), property.c_str(), &error) == value))
          {
            if (!dbus_error_is_set (&error))
            {
              out.push_back (*n);
              continue;
            }
          }

        if (dbus_error_is_set (&error))
          dbus_error_free (&error);
      }
    }
    std::swap (in, out);
  }

  template<>
  void    Context::filter_device_list (StrV & in,
                      std::string const& property, double value)
  {
    StrV out;

    for (StrV::iterator n = in.begin() ; n != in.end() ; ++n)
    {
      DBusError error;
      dbus_error_init (&error);

      if (libhal_device_property_exists
            (_cobj, n->c_str(), property.c_str(), &error))
      {

        if (!dbus_error_is_set (&error) &&
            (libhal_device_get_property_double
              (_cobj, n->c_str(), property.c_str(), &error) == value))
          {
            if (!dbus_error_is_set (&error))
            {
              out.push_back (*n);
              continue;
            }
          }

        if (dbus_error_is_set (&error))
          dbus_error_free (&error);
      }
    }
    std::swap (in, out);
  }

  template<>
  void    Context::filter_device_list  (StrV & in,
                      std::string const& property, bool value)
  {
    StrV out;
  
    for (StrV::iterator n = in.begin() ; n != in.end() ; ++n)
    {
      DBusError error;
      dbus_error_init (&error);

      if (libhal_device_property_exists
          (_cobj, n->c_str(), property.c_str(), &error))
      {
        if (!dbus_error_is_set (&error) &&
            (libhal_device_get_property_bool
              (_cobj, n->c_str(), property.c_str(), &error) == value))
          {
            if (!dbus_error_is_set (&error))
            {
              out.push_back (*n);
              continue;
            }
          }

        if (dbus_error_is_set (&error))
          dbus_error_free (&error);
      }
    }
    std::swap (in, out);
  }

  template<>
  void    Context::filter_device_list (StrV & in,
                    std::string const& property, StrV const& value)
  {
    StrV out;

    for (StrV::iterator n = in.begin() ; n != in.end() ; ++n)
    {
      DBusError error;
      dbus_error_init (&error);

      if (libhal_device_property_exists
            (_cobj, n->c_str(), property.c_str(), &error))
      {

        if (!dbus_error_is_set (&error) &&
            (StrV (const_cast<const char **>(libhal_device_get_property_strlist
                (_cobj, n->c_str(), property.c_str(), &error))) == value))
          {
            if (!dbus_error_is_set (&error))
            {
              out.push_back (*n);
              continue;
            }
          }

        if (dbus_error_is_set (&error))
          dbus_error_free (&error);
      }
    }
    std::swap (in, out);
  }

  ///// INIT ///////////////////////////////////////////////////////////////////

  Context::Context (LibHalContext * context, bool ownership)
    : m_ownership (ownership)
  {
    _cobj = context; 
    if (!_cobj)
      throw UnableToCreateContextError (std::string());
  
    for (int n = 0; n < N_SIGNALS; signals[n++] = false);

    libhal_ctx_set_user_data (_cobj, this);
    connect_context ();
  }

  Context::Context (DBusConnection * connection)
    : m_ownership (true)
  {
    _cobj = libhal_ctx_new ();
    if (!_cobj)
      throw UnableToCreateContextError (std::string());
  
    for (int n = 0; n < N_SIGNALS; signals[n++] = false);

    libhal_ctx_set_user_data (_cobj, this);
    libhal_ctx_set_dbus_connection (_cobj, connection);

    DBusError error;
    dbus_error_init (&error);

    libhal_ctx_init (_cobj, &error);

    if (dbus_error_is_set(&error))
    {
      std::string message = error.message;
      dbus_error_free (&error);
      throw UnableToCreateContextError (message);
    }
    connect_context ();
  }

  void
  Context::connect_context ()
  {
    if (!libhal_ctx_set_device_new_capability
        (_cobj, _ccallback_libhal_device_new_capability))
      throw CallbackSetupError(std::string());

    if (!libhal_ctx_set_device_lost_capability
        (_cobj, _ccallback_libhal_device_lost_capability))
      throw CallbackSetupError(std::string());

    if (!libhal_ctx_set_device_property_modified
        (_cobj, _ccallback_libhal_device_property_modified))
      throw CallbackSetupError(std::string());

    if (!libhal_ctx_set_device_condition
        (_cobj, _ccallback_libhal_device_condition))
      throw CallbackSetupError(std::string());
  }
  
  Context::~Context ()
  { 
    if (m_ownership)
    {
      DBusError error;
      dbus_error_init (&error);

      libhal_ctx_shutdown (_cobj, &error);
      if (dbus_error_is_set (&error))
      {
        std::cerr << "Error disconnecting from DBus: " << error.message << std::endl;
        dbus_error_free (&error);
      }
      libhal_ctx_free (_cobj);
    }
  }

  Hal::RefPtr<Context>
  Context::create (DBusConnection * connection)
  {
    dbus_connection_ref(connection);
    return Hal::RefPtr<Context>(new Context(connection));
  }

  Hal::RefPtr<Context>
  Context::wrap (LibHalContext * context, bool ownership)
  {
    return Hal::RefPtr<Context>(new Context(context, ownership));
  }
}
