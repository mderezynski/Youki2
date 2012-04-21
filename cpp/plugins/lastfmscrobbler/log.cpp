//    Copyright (C) 2008 Dirk Vanden Boer <dirk.vdb@gmail.com>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "log.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef ENABLE_LOGGING
    #include <syslog.h>
#endif

#include <iostream>

#define ENABLE_DEBUG 1

namespace logger
{

void error(ILog& obj, const std::string& message)
{
#ifdef ENABLE_LOGGING
    syslog(LOG_ERR, "%s", message.c_str());
#endif

    obj << "ERROR: " + message + "\n";
}

void info(ILog& obj, const std::string& message)
{
#ifdef ENABLE_LOGGING
    syslog(LOG_INFO, "%s", message.c_str());
#endif

#ifdef ENABLE_DEBUG
    obj << "INFO:  " + message + "\n";
#endif
}

void warn(ILog& obj, const std::string& message)
{
#ifdef ENABLE_LOGGING
    syslog(LOG_WARNING, "%s", message.c_str());
#endif

    obj << "WARN:  " + message + "\n";
}

void debug(ILog& obj, const std::string& message)
{
#ifdef ENABLE_LOGGING
    syslog(LOG_DEBUG, "%s", message.c_str());
#endif

#ifdef ENABLE_DEBUG
    obj << "DEBUG: " + message + "\n";
#endif
}

void critical(ILog& obj, const std::string& message)
{
#ifdef ENABLE_LOGGING
    syslog(LOG_CRIT, "%s", message.c_str());
#endif

    obj << "CRIT:  " + message + "\n";
}

}
