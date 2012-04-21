//  BMPx - The Dumb Music Player
//  Copyright (C) 2005-2007 BMPx development team.
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
//  --
//
//  The BMPx project hereby grants permission for non GPL-compatible GStreamer
//  plugins to be used and distributed together with GStreamer and BMPx. This
//  permission is above and beyond the permissions granted by the GPL license
//  BMPx is covered by.

#ifndef BMP_SAX_BASE_PARSER
#define BMP_SAX_BASE_PARSER

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif //HAVE_CONFIG_H

#include <glibmm.h>
#include <string>
#include <map>
#include "xpath.hh"

typedef std::map <std::string, std::string> Props; 
struct ParseContextBase;

typedef sigc::slot<void, std::string const&, Props&, ParseContextBase&> ElementHandler;
typedef sigc::slot<void, ParseContextBase&>                             ElementHandlerEnd;
typedef sigc::slot<void, std::string const&, ParseContextBase&>         HandlerText;

typedef std::map <MPX::XPath, ElementHandler> HandlerMap;
typedef HandlerMap::value_type HandlerPair;

typedef std::map <MPX::XPath, ElementHandlerEnd> HandlerEndMap;
typedef HandlerEndMap::value_type HandlerEndPair;

typedef std::map <MPX::XPath, HandlerText> HandlerTextMap;
typedef HandlerTextMap::value_type HandlerTextPair;

struct ParseContextBase
{
  HandlerMap        mHandlers;
  HandlerEndMap     mHandlersEnd;
  HandlerTextMap    mHandlersText;

  MPX::XPath        mPath;
  Props             mProps;
  
  ParseContextBase ()
  : mPath ("") {}

  void operator<< (HandlerPair & pair)
  {
    mHandlers.insert (pair);
  }

  void operator<< (HandlerEndPair & pair)
  {
    mHandlersEnd.insert (pair);
  }

  void operator<< (HandlerTextPair & pair)
  {
    mHandlersText.insert (pair);
  }
};

  //--------- Actual Handlers 
#define HANDLER(X) \
  void  \
  X (std::string const& prefix, Props & props, ParseContextBase & _context)

#define HANDLER_END(X) \
  void  \
  X (ParseContextBase & _context)

#define HANDLER_Text(X) \
  void  \
  X (std::string const& text, ParseContextBase & _context)

namespace MPX
{
  namespace SaxParserBase
  {
    int xml_base_parse (std::string const &data, ParseContextBase & context);
    int xml_base_parse (const char * data, guint size, ParseContextBase & context);
  }
}

#endif //!BMP_MBXML_V2_BASE_PARSER
