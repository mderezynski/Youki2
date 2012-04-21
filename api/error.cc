#include "config.h"

#include "mpx/mpx-error.hh"

#include <string>
#include <glibmm.h>

namespace MPX
{
    void
    ErrorManager::append_error( const Error& error )
    {
        Glib::RecMutex::Lock L (m_queue_lock);
        m_queue.push_back(ErrorRefP(new Error(error)));
    }

    void
    ErrorManager::display_errors ()
    {
        while( true )
        {
            ErrorRefP error;

            {
                Glib::RecMutex::Lock L (m_queue_lock);
                if( !m_queue.empty() )
                {
                    error = m_queue.front();
                    m_queue.pop_front();
                }
                else
                    return;
            }

            if( error )
            {
                g_message("ERR: [%s](%s): %s",
                    error->get_component().c_str(),
                    error->get_subcomponent().c_str(),
                    error->get_errorstring().c_str()
                );
            }
        }
    }

    ErrorManager::ErrorManager () 
    {
    }

    ErrorManager::~ErrorManager()
    {
    }

    void
    ErrorManager::new_error( const Error& error )
    {
        append_error (error);
        display_errors ();
    }
}
