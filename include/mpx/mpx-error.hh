#ifndef MPX_ERROR_HH
#define MPX_ERROR_HH

#include <glibmm.h>
#include <string>
#include <deque>
#include <boost/shared_ptr.hpp>

namespace MPX
{
class Error
{
        Glib::ustring   m_component,
                        m_subcomponent,
                        m_errorstring;

    public:

        const Glib::ustring&
        get_component()
        {
            return m_component;
        }

        const Glib::ustring&
        get_subcomponent()
        { 
            return m_subcomponent;
        }

        const Glib::ustring&
        get_errorstring()
        {
            return m_errorstring;
        }
        
        Error(
            const Glib::ustring&    component,
            const Glib::ustring&    subcomponent,
            const Glib::ustring&    errorstring
        )
        : m_component(component)
        , m_subcomponent(subcomponent)
        , m_errorstring(errorstring)
        {
        }
};

typedef boost::shared_ptr<Error> ErrorRefP;

class ErrorManager
{
            Glib::RecMutex              m_queue_lock;
            std::deque<ErrorRefP>       m_queue;

            void
            append_error( const Error& );

            void
            display_errors ();

        public:

            ErrorManager (); 
            virtual ~ErrorManager();

            void
            new_error( const Error& );
};
}

#endif // MPX_ERROR_HH
