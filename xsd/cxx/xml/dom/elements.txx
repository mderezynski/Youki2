// file      : xsd/cxx/xml/dom/elements.txx
// author    : Boris Kolpackov <boris@codesynthesis.com>
// copyright : Copyright (c) 2005-2007 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <xsd/cxx/xml/string.hxx>

namespace xsd
{
  namespace cxx
  {
    namespace xml
    {
      namespace dom
      {
        template <typename C>
        qualified_name<C>
        name (const xercesc::DOMAttr& a)
        {
          const XMLCh* n (a.getLocalName ());

          // If this DOM doesn't support namespaces then use getName.
          //
          if (n != 0)
          {
            return qualified_name<C> (transcode<C> (n),
                                      transcode<C> (a.getNamespaceURI ()));
          }
          else
            return qualified_name<C> (transcode<C> (a.getName ()));
        }


        template <typename C>
        qualified_name<C>
        name (const xercesc::DOMElement& e)
        {
          const XMLCh* n (e.getLocalName ());

          // If this DOM doesn't support namespaces then use getTagName.
          //
          if (n != 0)
          {
            return qualified_name<C> (transcode<C> (n),
                                      transcode<C> (e.getNamespaceURI ()));
          }
          else
            return qualified_name<C> (transcode<C> (e.getTagName ()));
        }
      }
    }
  }
}
