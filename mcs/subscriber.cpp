#include <mcs/types.h>
#include <mcs/subscriber.h>

namespace Mcs
{
    Subscriber::Subscriber(
        SubscriberNotify const& notify
    )
    : m_notify(notify)
    {
	}

    Subscriber::~Subscriber ()
    {
      m_notify.disconnect ();
    }

    void
    Subscriber::notify(
        const std::string& domain,
        const std::string& key,
        const KeyVariant&  value
    )
    {
      m_notify (domain, key, value);
    }
};
