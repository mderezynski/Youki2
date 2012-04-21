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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif //HAVE_CONFIG_H

#include <cstring>
#include <libxml/parser.h>
#include <glibmm.h>

using namespace Glib;
using namespace std;

#define GET_CONTEXT                                                                                         \
    xmlParserCtxtPtr xmlptr G_GNUC_UNUSED (reinterpret_cast<xmlParserCtxtPtr>(ctxptr));                     \
    ParseContextBase & context G_GNUC_UNUSED (*(reinterpret_cast<ParseContextBase*>(xmlptr->_private)));

#include "mpx/parser/libxml2-sax-base.hh"

namespace
{
  xmlEntityPtr
  get_entity      (void * ctxptr, const xmlChar *name)
  {   
    return xmlGetPredefinedEntity (name);
  } 
  
  void
  start_document  (void * ctxptr)
  {
    GET_CONTEXT
  }
  
  void
  end_document    (void * ctxptr)
  {
    GET_CONTEXT
  }

  void
  attribute_decl    (void * ctxptr, 
                     const xmlChar * elem, 
                     const xmlChar * fullname, 
                     int type, 
                     int def, 
                     const xmlChar * defaultValue, 
                     xmlEnumerationPtr tree)
  {
    GET_CONTEXT
  }

  void
  start_sax2_ns   (void * ctxptr, 
                   const xmlChar * _name, 
                   const xmlChar * _prefix, 
                   const xmlChar * _URI, 
                   int nb_namespaces, 
                   const xmlChar ** _namespaces, 
                   int nb_attributes, 
                   int nb_defaulted, 
                   const xmlChar ** _props)
  {
    GET_CONTEXT
    
    std::string prefix;
    if (_prefix)
      prefix = ((const char*)(_prefix));
    
    Props props;
    if (nb_attributes)
    {
      for (int i = 0; i < nb_attributes * 5;i += 5)
      {
        std::string x;
        int len  = ((int)(_props[i+4] - _props[i+3]));
        x.append ((const char*)(_props[i+3]), len);
        props.insert (std::make_pair (ustring ((const char*)(_props[i])), ustring (x)));
      }
    }

    context.mPath /= (const char*)(_name);
    HandlerMap::iterator i2 = context.mHandlers.find (context.mPath);
    if (i2 != context.mHandlers.end())
    {
      i2->second (prefix, props, context);
    }
  }

  void
  end_sax2_ns (void * ctxptr, 
               const xmlChar * _name, 
               const xmlChar * prefix, 
               const xmlChar * URI)
  {
    GET_CONTEXT
  
    if (context.mPath.leaf() == (const char*)(_name)) {
      HandlerEndMap::iterator i2 = context.mHandlersEnd.find (context.mPath);
      if (i2 != context.mHandlersEnd.end())
      {
        i2->second (context);
      }

      context.mPath.remove_leaf();
    }
  }
 
  void
  whitespace      (void * ctxptr, const xmlChar * _text, int length)
  {
    GET_CONTEXT
  }
  
  void
  pcdata          (void * ctxptr, const xmlChar * _text, int length)
  {
    GET_CONTEXT

    HandlerTextMap::iterator i2 = context.mHandlersText.find (context.mPath);
    if (i2 != context.mHandlersText.end())
    {
      std::string charv;
      charv.append ((const char*)(_text), length);
      i2->second (charv, context);
    }
  
  }
  
  void
  warning         (void * ctxptr, const char * message, ...)
  {
    GET_CONTEXT
  }
  
  void
  error           (void * ctxptr, const char * message, ...)  
  {
    GET_CONTEXT
  }
  
  void
  fatal_error     (void * ctxptr, const char * message, ...)
  {
    GET_CONTEXT
  }
}

namespace MPX
{
  namespace SaxParserBase
  {
    int xml_base_parse (const char * data, guint size, ParseContextBase & context)
    {
      xmlSAXHandler sax; 
      memset (&sax,0,sizeof(xmlSAXHandler));
  
      sax.initialized = XML_SAX2_MAGIC;      
      sax.warning = warning;
      sax.fatalError = fatal_error;
      sax.error = error;
      sax.startElementNs = start_sax2_ns;
      sax.endElementNs = end_sax2_ns;
      sax.characters = pcdata;
      sax.ignorableWhitespace = whitespace;
      sax.startDocument = start_document;
      sax.endDocument = end_document;
      sax.attributeDecl = attribute_decl;
      sax.getEntity = get_entity;

      xmlParserCtxtPtr ctxt = xmlCreatePushParserCtxt (&sax, NULL, NULL, 0, NULL);
      ctxt->_private = &context;
      if (!ctxt)
      {
        g_warning ("%s: Couldn't create context", G_STRLOC);
        return -1;
      }

      if (xmlParseChunk (ctxt, data, size, 1))
      {
        g_warning ("%s: Error parsing document!", G_STRLOC);
        return -1;
      }

      return 0;
    }

    int xml_base_parse (std::string const &data, ParseContextBase & context)
    {
      xmlSAXHandler sax; 
      memset (&sax,0,sizeof(xmlSAXHandler));
  
      sax.initialized = XML_SAX2_MAGIC;      
      sax.warning = warning;
      sax.fatalError = fatal_error;
      sax.error = error;
      sax.startElementNs = start_sax2_ns;
      sax.endElementNs = end_sax2_ns;
      sax.characters = pcdata;
      sax.ignorableWhitespace = whitespace;
      sax.startDocument = start_document;
      sax.endDocument = end_document;
      sax.attributeDecl = attribute_decl;
      sax.getEntity = get_entity;

      xmlParserCtxtPtr ctxt = xmlCreatePushParserCtxt (&sax, NULL, NULL, 0, NULL);
      ctxt->_private = &context;
      if (!ctxt)
      {
        g_warning ("%s: Couldn't create context", G_STRLOC);
        return -1;
      }

      if (xmlParseChunk (ctxt, data.c_str(), strlen (data.c_str()), 1))
      {
        g_warning ("%s: Error parsing document!", G_STRLOC);
        return -1;
      }

      return 0;
    }
  }
}
