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

#include "urlclient.h"

#include <cassert>
#include <stdexcept>

#include "mpx/mpx-minisoup.hh"

using namespace std;

UrlClient::UrlClient()
{
    initialize();
}

UrlClient::~UrlClient()
{
    cleanup();
}

void UrlClient::initialize()
{
}

void UrlClient::cleanup()
{
}

void UrlClient::get(const string& url, string& response)
{
    Glib::RefPtr<MPX::Soup::RequestSync> req = MPX::Soup::RequestSync::create( url, false ) ;

    int code = req->run() ;

    if(code == 200)
    {
	response = req->get_data() ;
    }
}

void UrlClient::getBinary(const string& url, void* callback, void* parameter)
{
}

int UrlClient::post(const string& url, const string& data, string& response)
{
    Glib::RefPtr<MPX::Soup::RequestSync> req = MPX::Soup::RequestSync::create( url, true ) ;

    req->add_request( "application/x-www-form-urlencoded", data ) ; 

    int code = req->run() ;

    if(code == 200)
    {
	response = req->get_data() ;
    }

    return code ;
}
