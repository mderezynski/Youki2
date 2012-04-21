#include <mcs/key.h>
#include <glib.h>

namespace Mcs
{
    Key::Key(
        std::string const& domain,
        std::string const& key,
        KeyVariant  const& def,
        KeyType		       type
    )
	: m_domain(domain) 
    , m_key(key)
    , m_default(def)
    , m_value(def)
    , m_type(type)
    {
	}

	Key::Key ()
	{
	}

	Key::~Key ()
	{
	}

    void 
    Key::subscriber_add(
        int id, 
        SubscriberNotify const& notify
    )
    {
        g_return_if_fail(m_subscribers.find(id) == m_subscribers.end());
        m_subscribers.insert( std::make_pair(id, Subscriber(notify)));
    }

    void 
    Key::subscriber_del(
        int id 
    )
    {
        g_return_if_fail (m_subscribers.find(id) != m_subscribers.end());
        m_subscribers.erase(id);
    }

    KeyVariant 
    Key::get_value () const
    {
        return m_value;
    }

    KeyType
    Key::get_type () const
    {
        return m_type;
    }

    void 
    Key::unset () 
    {
        m_value = m_default;
    }
}
