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

#ifndef UTILS_CONDITION_H
#define UTILS_CONDITION_H

#include <pthread.h>

namespace utils
{

class Mutex;

class Condition
{
public:
    Condition();
    ~Condition();

    void wait(Mutex& mutex);
    bool wait(Mutex& mutex, int timeoutInMs);
    void signal();
    void broadcast();

private:
    pthread_cond_t m_Condition;
};

}

#endif
