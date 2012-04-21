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

#ifndef STRING_OPERATIONS_H
#define STRING_OPERATIONS_H

#include <string>
#include <vector>
#include <sstream>
#include <iostream>

#include <algorithm>
#include <wchar.h>
#include <stdexcept>
#include <iostream>

namespace StringOperations
{
    void lowercase(std::string& aString);
    void trim(std::string& aString);
    std::string trim(const std::string& aString);
    void replace(std::string& aString, const std::string& toSearch, const std::string& toReplace);
    void dos2unix(std::string& aString);
    std::string urlEncode(const std::string& aString);
    std::vector<std::string> tokenize(const std::string& str, const std::string& delimiter);
    void wideCharToUtf8(const std::wstring& wideString, std::string& utf8String);
    void utf8ToWideChar(const std::string& utf8String, std::wstring& wideString);

    template<typename T>
    inline void toNumeric(const std::string& aString, T& numeric)
    {
        std::stringstream ss(aString);
        ss >> numeric;
    }

    template<typename T>
    inline std::string getPostData(T& numeric)
    {
        std::stringstream ss;
        ss << numeric;
        return ss.str();
    }

    template<typename T>
    inline std::string toWstring(T& numeric)
    {
        std::wstringstream ss;
        ss << numeric;
        return ss.str();
    }
}

#endif
