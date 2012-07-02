//  BMP
//  Copyright (C) 2005 BMP development.
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License Version 2 as 
//  published by the Free Software Foundation.
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
//  The BMPx project hereby grants permission for non-GPL compatible GStreamer
//  plugins to be used and distributed together with GStreamer and BMPx. This
//  permission is above and beyond the permissions granted by the GPL license
//  BMPx is covered by.

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif //HAVE_CONFIG_H

#include <glibmm.h>
#include <glibmm/i18n.h>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xmlreader.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include "mpx/util-string.hh"
#include "mpx/util-file.hh"
#include "mpx/mpx-uri.hh"
#include "mpx/xml/xml.hh"
#include "mpx/mpx-types.hh"

#include "mpx/mpx-xspf-write.hh"
#include <cstring>

namespace
{
    std::string
    get_cstr(xmlChar* str)
    {
      std::string cstr ((const char*)str) ;
      //xmlFree(str);
      return cstr ;
    }

    const char* XSPF_ROOT_NODE_NAME = "playlist";
    const char* XSPF_XMLNS = "http://xspf.org/ns/0/";
}

namespace MPX
{
    std::string
    XSPF_write(
	  const Track_sp_v&	tracklist
	, const std::string&	uuid
    )
    {
	xmlDocPtr  doc;
	xmlNsPtr   ns_xspf, ns_yki ;
	xmlNodePtr node_playlist,
		   node_tracklist,
		   node_track,
		   node_location,
		   node;
	int	   size;

	doc = xmlNewDoc(BAD_CAST "1.0"); 

	node_playlist = xmlNewNode (NULL, BAD_CAST XSPF_ROOT_NODE_NAME);

	ns_yki  = xmlNewNs (node_playlist, BAD_CAST "http://youki.mp/ns/0/", BAD_CAST "yki");
	ns_xspf = xmlNewNs (node_playlist, BAD_CAST XSPF_XMLNS, BAD_CAST "xspf");

	xmlSetProp (node_playlist, BAD_CAST "version", BAD_CAST "1");
	xmlDocSetRootElement (doc, node_playlist);

	xmlNodePtr node_extension;
	node_extension = xmlNewNode (ns_xspf, BAD_CAST "extension");
	xmlSetProp (node_extension, BAD_CAST "application", BAD_CAST "yki");

	node = xmlNewNode (ns_yki, BAD_CAST "sqldb-uuid"); 
	xmlAddChild (node, xmlNewText(BAD_CAST(uuid.c_str())));
	xmlAddChild (node_extension, node);
	xmlAddChild (node_playlist, node_extension);

	node = xmlNewNode (ns_xspf, BAD_CAST "creator");
	xmlAddChild (node, xmlNewText(BAD_CAST "Youki"));
	xmlAddChild (node_playlist, node);

	node_tracklist = xmlNewNode (ns_xspf, BAD_CAST "trackList");
	xmlAddChild (node_playlist, node_tracklist);

	for( auto t : tracklist )
	{
	  node_track = xmlNewNode (ns_xspf, BAD_CAST "track");
	  node_location = xmlNewNode (ns_xspf, BAD_CAST "location");
	  xmlAddChild (node_location, xmlNewText(BAD_CAST((boost::get<std::string>((*t)[ATTRIBUTE_LOCATION].get())).c_str())));
	  xmlAddChild (node_track, node_location); 
	  xmlAddChild (node_tracklist, node_track);

	  node = xmlNewNode (ns_xspf, BAD_CAST "identifier"); 
	  xmlAddChild (node_track, node);
	  xmlAddChild (node, xmlNewText(BAD_CAST(boost::lexical_cast<std::string>(boost::get<guint>((*t)[ATTRIBUTE_MPX_TRACK_ID].get())).c_str() )));

	  if( (*t).has(ATTRIBUTE_MB_TRACK_ID))
	  { 
	      node = xmlNewNode (ns_xspf, BAD_CAST "meta"); 
	      std::string mb_rel ("http://musicbrainz.org/mm-2.1/track/");
	      mb_rel.append((boost::get<std::string>((*t)[ATTRIBUTE_MB_TRACK_ID].get())).c_str()) ;
	      xmlAddChild (node, xmlNewText(BAD_CAST(mb_rel.c_str())));
	      xmlSetProp  (node, BAD_CAST "rel", BAD_CAST "http://musicbrainz.org/track");
	      xmlAddChild (node_track, node);
	  }
     
	  node = xmlNewNode (ns_xspf, BAD_CAST "creator"); 
	  xmlAddChild (node, xmlNewText(BAD_CAST((boost::get<std::string>((*t)[ATTRIBUTE_ARTIST].get()).c_str()))));
	  xmlAddChild (node_track, node);

	  node = xmlNewNode (ns_xspf, BAD_CAST "album"); 
	  xmlAddChild (node, xmlNewText(BAD_CAST((boost::get<std::string>((*t)[ATTRIBUTE_ALBUM].get()).c_str()))));
	  xmlAddChild (node_track, node);

	  node = xmlNewNode (ns_xspf, BAD_CAST "title"); 
	  xmlAddChild (node, xmlNewText(BAD_CAST((boost::get<std::string>((*t)[ATTRIBUTE_TITLE].get()).c_str()))));
	  xmlAddChild (node_track, node);

	}

	xmlKeepBlanksDefault(0);
	xmlChar *data;
	xmlDocDumpFormatMemoryEnc(doc, &data, &size, "UTF-8", 1);
	std::string r ((const char*)data,size) ;
	return r ;
    }

namespace
{
    std::string
    get_xml_uuid(
	  const std::string& xml
    )
    {
	std::string xmluuid ;

	xmlXPathObjectPtr   xo ;
	xmlNodeSetPtr	    nv ;
	xmlDocPtr	    doc ;

	doc = xmlParseDoc (BAD_CAST xml.c_str());

	if (!doc)
	    throw std::runtime_error("Could not parse Playlist XML.") ; 

	xo = xpath_query
	      (doc,
	       BAD_CAST "//yki:sqldb-uuid",
	       BAD_CAST "xspf=http://xspf.org/ns/0/ yki=http://youki.mp/ns/0/"); 

	if (!xo) 
	{
	    xmlFreeDoc (doc);
	    throw std::runtime_error("No nodes on XPath query (wrong file format?)") ; 
	}

	nv = xo->nodesetval;

	if (!nv)
	{
	    xmlFreeDoc (doc);
	    throw std::runtime_error("No nodes on XPath query (wrong file format?)") ; 
	}

        if( nv->nodeNr < 1 )
	{
	    xmlFreeDoc (doc) ; 
	    throw std::runtime_error("No nodes on XPath query (wrong file format?)") ; 
	}

	xmlNodePtr rnode, snode ;

	rnode = nv->nodeTab[0];
	snode = rnode->children;

	xmlChar * c = XML_GET_CONTENT(snode) ;

	xmluuid = (const char*)(c) ;

	xmlFreeDoc (doc) ;
	xmlXPathFreeObject (xo);

	return xmluuid ;
    }
}

    void 
    XSPF_read(
	  const std::string&	    xml
	, const std::string&	    uuid
        , std::vector<guint>&	    out
    )
    {
	xmlXPathObjectPtr   xo ;
	xmlNodeSetPtr	    nv ;
	xmlDocPtr	    doc ;

	doc = xmlParseDoc (BAD_CAST xml.c_str());

	if (!doc)
	    throw std::runtime_error("Could not parse Playlist XML.") ; 

	xo = xpath_query
	      (doc,
	       BAD_CAST "//xspf:identifier",
	       BAD_CAST "xspf=http://xspf.org/ns/0/ yki=http://youki.mp/ns/0/"); 

	if (!xo) 
	{
	    xmlFreeDoc (doc);
	    throw std::runtime_error("No nodes on XPath query (wrong file format?)") ; 
	}

	nv = xo->nodesetval;

	if (!nv)
	{
	    xmlFreeDoc (doc);
	    throw std::runtime_error("No nodes on XPath query (wrong file format?)") ; 
	}

        if( nv->nodeNr < 1 )
	{
	    xmlFreeDoc (doc) ; 
	    throw std::runtime_error("No nodes on XPath query (wrong file format?)") ; 
	}

	xmlNodePtr trackNode, siblingNode;

	for(int n = 0; n < nv->nodeNr; n++)
	{
	    trackNode = nv->nodeTab[n];
	    siblingNode = trackNode->children;
	    xmlChar* c = XML_GET_CONTENT(siblingNode) ;
	    out.push_back(boost::lexical_cast<guint>(c)) ;
	}

	xmlXPathFreeObject (xo);

	if(get_xml_uuid(xml) != uuid )
	{
	    throw std::runtime_error("SQLDB UUID of XSPF playlist is from a different database.") ;
	}
    }
}
