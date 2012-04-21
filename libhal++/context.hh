//
// libhal++ (C) GPL 2006 M. Derezynski
//

#ifndef _HAL_CC_CONTEXT_HH
#define _HAL_CC_CONTEXT_HH

#include <string>
#include <map>

#include <libhal.h>
#include <dbus/dbus.h>
#include <sigc++/sigc++.h>

#include "types.hh"
#include "refptr.hh"

namespace Hal
{

#include "exception.hh"

      HALCC_EXCEPTION(UnableToCreateContextError)
      HALCC_EXCEPTION(UnableToSetDBusConnectionError)
      HALCC_EXCEPTION(UnableToGetDevicesError)
      HALCC_EXCEPTION(UnableToProbeDeviceError)

      HALCC_EXCEPTION(CallbackSetupError)
      HALCC_EXCEPTION(WatchSetupError)
      HALCC_EXCEPTION(HALGenericError)

  class Device;
  class Context 
  {
    public:

      typedef sigc::signal<void, std::string const&> SignalUDI;

      /** Creates a RefPtr holding a Hal::Context
        *
        * @param conn The DBusConnection to be used for this context
        */
      static Hal::RefPtr<Context> create (DBusConnection  * conn);

      /** Wraps a C LibHalContext, and optionally takes ownership of it 
        * NOTE: This will fail/not work right when used with a context created using libhal_ctx_init_direct() !
        *
        * @param ctx The C LibHalContext to wrap 
        * @param ownership Whether to take ownership of the C instance or not
        */
      static Hal::RefPtr<Context> wrap (LibHalContext * ctx, bool ownership = false);

      StrV
      get_all_devices () throw (UnableToGetDevicesError);

      bool
      device_exists (std::string const& udi) throw (UnableToProbeDeviceError);
         

      bool
      device_query_capability (std::string const& udi, std::string const& capability) throw (UnableToProbeDeviceError);
         
       

      // Methods for device lookups

      StrV
      find_device_by_capability (std::string const& capability) throw (HALGenericError);

      template<typename T>
      struct fail {};

      template <typename T>
      void filter_device_list (StrV & in, std::string const& property, T value) throw (HALGenericError)
      {
        fail<T>::filter_device_list_called_with_an_inappropriate_type;
      }

      void set_use_cache (bool use_cache = true);

      // Signals
      SignalUDI&
      signal_device_added ()
        throw (CallbackSetupError);

      SignalUDI&
      signal_device_removed ()
        throw (CallbackSetupError);

      LibHalContext * cobj () { return _cobj; }

    private:    

      friend class Hal::RefPtr<Context>;
      friend class Hal::Device;

      LibHalContext * _cobj;
      void connect_context ();
      bool m_ownership;
      
      explicit Context (DBusConnection  * conn);
      explicit Context (LibHalContext   * ctx, bool ownership = false);
      explicit Context () {}
      ~Context ();

      enum Signals
      {
        DEVICE_ADDED,
        DEVICE_REMOVED,
        N_SIGNALS
      };

      bool signals[N_SIGNALS];

      // Referencing/Housekeeping of devices

      typedef std::map < std::string , Hal::Device* > MDevices;
      MDevices m_devices;

      struct DestroyData
      {
        Hal::Context *  m_context;
        std::string     m_udi;

        DestroyData (Hal::Context * context, std::string const& udi);
      };

      void          device_created        (Hal::Device * device);
      Hal::Device * get_registered_device (std::string const& udi);
      static void * device_destroyed      (void * data); // sigc::trackable::internal::func_destroy_notify

      static void _ccallback_libhal_device_added
        (LibHalContext *ctx, const char * udi);  

      static void _ccallback_libhal_device_removed
        (LibHalContext *ctx, const char * udi);  

      // We use these to invoke the callbacks on the devices

      static void _ccallback_libhal_device_new_capability
        (LibHalContext *ctx, const char * udi, const char * capability);  

      static void _ccallback_libhal_device_lost_capability
        (LibHalContext *ctx, const char * udi, const char * capability);  

      static void _ccallback_libhal_device_property_modified
        (LibHalContext *ctx, const char * udi, const char * key,
          dbus_bool_t is_removed, dbus_bool_t is_added);  

      static void _ccallback_libhal_device_condition
        (LibHalContext *ctx, const char * udi, const char * condition_name,
          const char * condition_detail); 

      SignalUDI  signal_device_added_;
      SignalUDI  signal_device_removed_;

      void
      property_watch_all ()
        throw (WatchSetupError);

      void
      add_property_watch (std::string const& udi) const
        throw (WatchSetupError);

      void
      remove_property_watch (std::string const& udi) const
        throw (WatchSetupError);
  };

  template<>
  void    Context::filter_device_list
            (StrV & in, std::string const& property, std::string value);

  template<>
  void    Context::filter_device_list
            (StrV & in, std::string const& property, dbus_int32_t value);

  template<>
  void    Context::filter_device_list
            (StrV & in, std::string const& property, dbus_uint64_t& value);

  template<>
  void    Context::filter_device_list
            (StrV & in, std::string const& property, double value);

  template<>
  void    Context::filter_device_list
            (StrV & in, std::string const& property, bool value);

  template<>
  void    Context::filter_device_list
            (StrV & in, std::string const& property, StrV const& value);
}

#endif //!_HAL_CC_CONTEXT_HH
