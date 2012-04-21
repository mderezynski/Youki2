//  MPX
//  Copyright (C) 2005-2007 MPX development.
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License Version 2
//  as published by the Free Software Foundation.
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
//  The MPXx project hereby grants permission for non-GPL compatible GStreamer
//  plugins to be used and distributed together with GStreamer and MPXx. This
//  permission is above and beyond the permissions granted by the GPL license
//  MPXx is covered by.

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xmlreader.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <stdlib.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <string>
#include <stdexcept>
#include "mpx/xml/xml.hh"

namespace
{
  int
  register_namespaces (xmlXPathContextPtr xpathCtx, const xmlChar* nsList)
  {
    xmlChar* nsListDup;
    xmlChar* prefix;
    xmlChar* href;
    xmlChar* next;
    
    nsListDup = xmlStrdup(nsList);
    if(nsListDup == NULL)
    {
    g_warning (G_STRLOC ": unable to strdup namespaces list");
    return(-1); 
    }
    
    next = nsListDup; 
    while (next != NULL)
    {
      // skip spaces
      while((*next) == ' ') next++;
      if((*next) == '\0') break;

      // find prefix
      prefix = next;
      next = (xmlChar*)xmlStrchr(next, '=');

      if (next == NULL) 
      {
          g_warning (G_STRLOC ": invalid namespaces list format");
          //xmlFree (nsListDup);
          return(-1); 
      }
      *(next++) = '\0';   
      
      // find href
      href = next;
      next = (xmlChar*)xmlStrchr(next, ' ');
      if (next != NULL)
      {
          *(next++) = '\0';   
      }

      // do register namespace
      g_message("prefix: '%s', href='%s'", prefix, href) ;
      if(xmlXPathRegisterNs(xpathCtx, prefix, href) != 0)
      {
          g_warning (G_STRLOC ": unable to register NS with prefix=\"%s\" and href=\"%s\"", prefix, href);
          //xmlFree(nsListDup);
          return(-1); 
      }
    }
    //xmlFree(nsListDup);
    return(0);
  }
}

namespace MPX
{
  xmlXPathObjectPtr
  xpath_query (xmlDocPtr doc, xmlChar const* xpathExpr, xmlChar const* nsList)
  {
    xmlXPathContextPtr xpathCtx;
    xmlXPathObjectPtr xpathObj;

    /*
     * Create xpath evaluation context
     */
    xpathCtx = xmlXPathNewContext (doc);
    if (xpathCtx == NULL)
    {
		return NULL;
    }

     if (nsList)
     register_namespaces (xpathCtx, nsList);

    /*
     * Evaluate xpath expression
     */
	xpathObj = xmlXPathEvalExpression (xpathExpr, xpathCtx);
    if (xpathObj == NULL)
    {
		//xmlXPathFreeContext (xpathCtx);
		return NULL;
    }

    /*
     * Cleanup
     */
    //xmlXPathFreeContext (xpathCtx);

    return xpathObj;
  }

  std::string
  xpath_get_text (char const* data, guint size, char const* xpathExpr, char const* nsList)
  {
        xmlDocPtr             doc;
        xmlXPathObjectPtr     xpathobj;
        xmlNodeSetPtr         nv;

        doc = xmlParseMemory (data, size); 
        if (!doc)
        {  
          throw std::runtime_error(_("Couldn't parse document"));
        }

        xpathobj = xpath_query (doc, (xmlChar*)xpathExpr, (xmlChar*)nsList);
        if (!xpathobj)
        {
          throw std::runtime_error(_("Invalid XPath"));
        }

        nv = xpathobj->nodesetval;
        if (!nv || !nv->nodeNr)
        {
          //xmlXPathFreeObject (xpathobj);
          throw std::runtime_error(_("No nodes in XPath result"));
        }

        xmlNodePtr node = nv->nodeTab[0];
        if (!node || !node->children)
        {
          //xmlXPathFreeObject (xpathobj);
          throw std::runtime_error(_("No nodes in XPath result"));
        }

        xmlChar * pcdata = XML_GET_CONTENT (node->children);
		std::string pcdata_cc = (char*)pcdata;
		//xmlFree(pcdata);
		//xmlXPathFreeObject(xpathobj);	
		//xmlFreeDoc(doc);
		return pcdata_cc;
	}
}
