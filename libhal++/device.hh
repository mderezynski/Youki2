//
// libhal++ (C) GPL 2006 M. Derezynski
//

#ifndef _HAL_CC_DEVICE_HH
#define _HAL_CC_DEVICE_HH

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif //HAVE_CONFIG_H

#include <libhal.h>
#include <sigc++/sigc++.h>

#include "refptr.hh"
#include "types.hh"
#include "util.hh"

#ifndef INSIDE_HAL_CC_CONTEXT
#  include "context.hh"
#endif //!INSIDE_HAL_CC_CONTEXT

namespace Hal
{
#include "exception.hh"

    HALCC_EXCEPTION(DeviceDoesNotExistError)
    HALCC_EXCEPTION(PropertyDoesNotExistError)

  class Volume;
  class Drive;
  class Device : public sigc::trackable
  {

    friend class Hal::RefPtr<Device>;
    friend class Hal::Context;

    public:

      typedef sigc::signal<void, const std::string&, const std::string&>
          SignalUDI_Capability;

      typedef sigc::signal<void, const std::string&, const std::string&, bool, bool>
          SignalUDI_Property_Modified;

      typedef sigc::signal<void, const std::string&, const std::string&, const std::string&>
          SignalUDI_Device_Condition;

      class PropertySet
      {
        public: 

          struct iterator
          {

#include "exception.hh"

          HALCC_EXCEPTION(InvalidKeyTypeRequest)

              inline bool operator==(iterator const& other)
              {
                return ((i.idx == other.i.idx) && (i.set == other.i.set));
              }

              inline bool operator!=(iterator const& other)
              {
                return !(*this == other);
              }

              inline void operator++()
              {
                libhal_psi_next (&i);
              }

              inline void operator++(int)
              {
                libhal_psi_next (&i);
              }
  
              inline Hal::PropertyType
              get_type ()
              {
                return Hal::PropertyType (libhal_psi_get_type (&i));
              }

              inline std::string
              get_key()
              {
                return Util::wrap_string ((const char*)libhal_psi_get_key (&i));
              }

              template<typename T>
              struct fail {};

              template <typename T>
              T get_value ()
              {
                fail<T>::get_property_called_with_an_inappropriate_type;
              }   

            private:
    
              friend class PropertySet;
              LibHalPropertySetIterator i;
          };

          iterator begin()
          { 
            return m_begin;
          }

          iterator end()
          {
            return m_end;
          }

          ~PropertySet ();

        private:
  
          friend class Hal::Device;

          explicit PropertySet () {}
          explicit PropertySet (Hal::RefPtr<Context> context, const std::string&);

          LibHalPropertySet * m_set;
          iterator            m_begin;
          iterator            m_end;
      };

      static Hal::RefPtr<Device> create (Hal::RefPtr<Context> context,
                                         const std::string&   udi) throw (DeviceDoesNotExistError);

      template<typename T>
      struct fail {};

      template <typename T>
      T get_property (const std::string& property)
      {
        fail<T>::get_property_called_with_an_inappropriate_type;
      }   

      bool
      property_exists (const std::string& property);

      PropertySet
      get_all_properties ();
       
      PropertyType
      get_property_type (const std::string& property);
    
      virtual const std::string&
      get_udi () const
      {
        return m_udi;
      }

      SignalUDI_Capability&
      signal_new_capability ();

      SignalUDI_Capability&
      signal_lost_capability ();

      SignalUDI_Property_Modified&
      signal_property_modified ();

      SignalUDI_Device_Condition&
      signal_condition ();

    protected:    

      explicit Device (Hal::RefPtr<Context> context, const std::string& udi);
   
      Hal::RefPtr<Context>  m_context; 
      std::string           m_udi;

      SignalUDI_Capability        signal_device_new_capability_;
      SignalUDI_Capability        signal_device_lost_capability_;
      SignalUDI_Property_Modified signal_device_property_modified_;
      SignalUDI_Device_Condition  signal_device_condition_;

      bool m_signals_accessed;

      virtual ~Device ();
  };

  // Device value specializations 
  template<> 
  std::string   Device::get_property  (const std::string& property);

  template<> 
  dbus_int32_t  Device::get_property  (const std::string& property);

  template<> 
  dbus_uint64_t Device::get_property  (const std::string& property);

  template<> 
  double        Device::get_property  (const std::string& property);

  template<>
  bool          Device::get_property  (const std::string& property);

  template<>
  StrV          Device::get_property  (const std::string& property);

  // PropertySet specializations
  template<>
  std::string   Device::PropertySet::iterator::get_value  ();

  template<> 
  dbus_int32_t  Device::PropertySet::iterator::get_value  ();

  template<> 
  dbus_uint64_t Device::PropertySet::iterator::get_value  ();

  template<> 
  double        Device::PropertySet::iterator::get_value  ();

  template<>
  bool          Device::PropertySet::iterator::get_value  ();

  template<>
  StrV          Device::PropertySet::iterator::get_value  ();
}

#endif //!_HAL_CC_DEVICE_HH
