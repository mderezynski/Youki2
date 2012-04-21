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

#include "mutex.h"

namespace utils
{

Mutex::Mutex()
{
    pthread_mutex_init(&m_Mutex, NULL);
}

Mutex::~Mutex()
{
    pthread_mutex_destroy(&m_Mutex);
}

void Mutex::lock()
{
    pthread_mutex_lock(&m_Mutex);
}

void Mutex::unlock()
{
    pthread_mutex_unlock(&m_Mutex);
}

pthread_mutex_t* Mutex::getHandle()
{
    return &m_Mutex;
}

}
