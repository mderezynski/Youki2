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

#ifndef URL_CLIENT_H
#define URL_CLIENT_H

#include <string>

class UrlClient
{
public:
    UrlClient();
    ~UrlClient();

    void get(const std::string& url, std::string& response);
    void getBinary(const std::string& url, void* callback, void* parameter);
    void post(const std::string& url, const std::string& data, std::string& response);

private:
    void initialize();
    void cleanup();
};

#endif
