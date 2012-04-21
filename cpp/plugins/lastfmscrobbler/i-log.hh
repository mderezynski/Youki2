//  MPX
#ifndef _MPX_I_LOG__HH
#define _MPX_I_LOG__HH

#include "config.h"
#include <string>

class ILog
{
    public:
        virtual ILog& operator<<(const std::string&) = 0 ;
        virtual ILog& operator<<(const char*) = 0 ;
} ;
#endif // MPX_I_LOG__HH 
