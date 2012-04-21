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

#include "condition.h"

#include "mutex.h"
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <stdexcept>

using namespace std;

typedef unsigned long long uint64;

namespace utils
{

Condition::Condition()
{
    pthread_cond_init(&m_Condition, NULL);
}

Condition::~Condition()
{
    pthread_cond_destroy(&m_Condition);
}

void Condition::wait(Mutex& mutex)
{
    int ret = pthread_cond_wait(&m_Condition, mutex.getHandle());
    if (0 != ret)
    {
        throw std::logic_error(string("pthread_cond_wait returned: ") + strerror(ret));
    }
}

bool Condition::wait(Mutex& mutex, int timeoutInMs)
{
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);

    struct timespec timeoutTime;
    uint64 nanoSeconds = (static_cast<uint64>(timeoutInMs) * 1000000) + (static_cast<uint64>(currentTime.tv_usec) * 1000);
    int seconds = nanoSeconds / 1000000000;

    timeoutTime.tv_sec += currentTime.tv_sec + seconds;
    timeoutTime.tv_nsec = static_cast<long>(nanoSeconds % 1000000000);

    int ret = pthread_cond_timedwait(&m_Condition, mutex.getHandle(), &timeoutTime);
    if (ETIMEDOUT == ret)
    {
        return false;
    }
    else if (0 != ret)
    {
        throw std::logic_error(string("pthread_cond_timedwait returned: ") + strerror(ret));
    }

    return true;
}

void Condition::signal()
{
    pthread_cond_signal(&m_Condition);
}

void Condition::broadcast()
{
    pthread_cond_broadcast(&m_Condition);
}

}
