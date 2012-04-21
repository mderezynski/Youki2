// file      : xsd/cxx/tree/stream-extraction-map.hxx
// author    : Boris Kolpackov <boris@codesynthesis.com>
// copyright : Copyright (c) 2005-2007 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_CXX_TREE_STREAM_EXTRACTION_MAP_HXX
#define XSD_CXX_TREE_STREAM_EXTRACTION_MAP_HXX

#include <map>
#include <memory> // std::auto_ptr

#include <xsd/cxx/tree/elements.hxx>
#include <xsd/cxx/tree/istream.hxx>
#include <xsd/cxx/xml/qualified-name.hxx>

namespace xsd
{
  namespace cxx
  {
    namespace tree
    {
      template <typename S, typename C>
      struct stream_extraction_map
      {
        typedef xml::qualified_name<C> qualified_name;
        typedef std::auto_ptr<type> (*extractor) (istream<S>&, flags, type*);

      public:
        stream_extraction_map ();

        void
        register_type (const qualified_name& name,
                       extractor,
                       bool override = true);

        std::auto_ptr<type>
        extract (istream<S>&, flags, type* container);

      public:
        extractor
        find (const qualified_name& name) const;

      private:
        typedef std::map<qualified_name, extractor> type_map;

        type_map type_map_;
      };

      //
      //
      template<unsigned long id, typename S, typename C>
      struct stream_extraction_plate
      {
        static stream_extraction_map<S, C>* map;
        static unsigned long count;

        stream_extraction_plate ();
        ~stream_extraction_plate ();
      };

      template<unsigned long id, typename S, typename C>
      stream_extraction_map<S, C>* stream_extraction_plate<id, S, C>::map = 0;

      template<unsigned long id, typename S, typename C>
      unsigned long stream_extraction_plate<id, S, C>::count = 0;


      //
      //
      template<unsigned long id, typename S, typename C>
      inline stream_extraction_map<S, C>&
      stream_extraction_map_instance ()
      {
        return *stream_extraction_plate<id, S, C>::map;
      }

      //
      //
      template<typename S, typename X>
      std::auto_ptr<type>
      extractor_impl (istream<S>&, flags, type*);


      template<unsigned long id, typename S, typename C, typename X>
      struct stream_extraction_initializer
      {
        stream_extraction_initializer (const C* name, const C* ns);
      };
    }
  }
}

#include <xsd/cxx/tree/stream-extraction-map.txx>

#endif // XSD_CXX_TREE_STREAM_EXTRACTION_MAP_HXX
