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

#include "submissioninfocollection.h"

#include <sstream>

using namespace std;

static const size_t MAX_QUEUE_SIZE = 50;

void SubmissionInfoCollection::addInfo(const SubmissionInfo& info)
{
    if (m_Infos.size() == MAX_QUEUE_SIZE)
    {
        m_Infos.pop_front();
    }
    m_Infos.push_back(info);
}

void SubmissionInfoCollection::clear()
{
    m_Infos.clear();
}

string SubmissionInfoCollection::getPostData() const
{
    stringstream ss;
    for (std::vector<SubmissionInfo>::size_type i = 0; i < m_Infos.size(); ++i)
    {
        ss << m_Infos[i].getPostData(static_cast<int>(i));
    }

    return ss.str();
}
