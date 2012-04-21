#ifndef MPX_SERVICES_HH
#define MPX_SERVICES_HH

#include <boost/shared_ptr.hpp>
#include <string>
#include <map>

namespace MPX
{
namespace Service
{
    class Base
    {
        private:
                std::string m_guid;
        public:
                Base (std::string const& guid)
                : m_guid(guid)
                {}

                virtual ~Base()
                {}

                std::string const&
                get_guid ()
                {
                    return m_guid;
                }
    };

    typedef boost::shared_ptr< ::MPX::Service::Base> Base_p;
    typedef std::map<std::string, Base_p>            Services;

    class Manager
    {
        private: 

            Services  m_services;

        public:

            Manager ();
            virtual ~Manager ();

            void
            add(Base_p);

            template <typename T>
            boost::shared_ptr<T>
            get(std::string const& guid)
            {
                try{
                    return boost::dynamic_pointer_cast<T, Base>(m_services.find(guid)->second);
                } catch(...) {
                    return boost::shared_ptr<T>();
                }
            }
    };
}
}

#endif
