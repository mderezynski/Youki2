#ifndef MCS_TYPES_H
#define MCS_TYPES_H

#include <glibmm.h>
#include <sigc++/sigc++.h>
#include <boost/variant.hpp>

namespace Mcs
{
    enum KeyType  
    {
      KEY_TYPE_BOOL,
      KEY_TYPE_INT,
      KEY_TYPE_FLOAT,
      KEY_TYPE_STRING
    };

    typedef boost::variant<bool, int, double, std::string> KeyVariant;
    typedef sigc::slot<void, const std::string& , const std::string&, const KeyVariant&> SubscriberNotify; 
}

#endif
