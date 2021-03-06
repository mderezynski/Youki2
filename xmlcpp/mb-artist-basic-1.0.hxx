// Copyright (C) 2005-2007 Code Synthesis Tools CC
//
// This program was generated by CodeSynthesis XSD, an XML Schema to
// C++ data binding compiler.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
//
// In addition, as a special exception, Code Synthesis Tools CC gives
// permission to link this program with the Xerces-C++ library (or with
// modified versions of Xerces-C++ that use the same license as Xerces-C++),
// and distribute linked combinations including the two. You must obey
// the GNU General Public License version 2 in all respects for all of
// the code used other than Xerces-C++. If you modify this copy of the
// program, you may extend this exception to your version of the program,
// but you are not obligated to do so. If you do not wish to do so, delete
// this exception statement from your version.
//
// Furthermore, Code Synthesis Tools CC makes a special exception for
// the Free/Libre and Open Source Software (FLOSS) which is described
// in the accompanying FLOSSE file.
//

#ifndef MB_ARTIST_BASIC_1_0_HXX
#define MB_ARTIST_BASIC_1_0_HXX

// Begin prologue.
//
//
// End prologue.

#include <xsd/cxx/version.hxx>

#if (XSD_INT_VERSION != 3000000L)
#error XSD runtime version mismatch
#endif

#include <xsd/cxx/pre.hxx>

#ifndef XSD_USE_CHAR
#define XSD_USE_CHAR
#endif

#ifndef XSD_CXX_TREE_USE_CHAR
#define XSD_CXX_TREE_USE_CHAR
#endif

#include <xsd/cxx/tree/exceptions.hxx>
#include <xsd/cxx/tree/elements.hxx>
#include <xsd/cxx/tree/types.hxx>

#include <xsd/cxx/xml/error-handler.hxx>

#include <xsd/cxx/tree/parsing.hxx>

namespace xml_schema
{
  // anyType and anySimpleType.
  //
  typedef ::xsd::cxx::tree::type type;
  typedef ::xsd::cxx::tree::simple_type<type> simple_type;

  // 8-bit
  //
  typedef signed char byte;
  typedef unsigned char unsigned_byte;

  // 16-bit
  //
  typedef short short_;
  typedef unsigned short unsigned_short;

  // 32-bit
  //
  typedef int int_;
  typedef unsigned int unsigned_int;

  // 64-bit
  //
  typedef long long long_;
  typedef unsigned long long unsigned_long;

  // Supposed to be arbitrary-length integral types.
  //
  typedef long long integer;
  typedef integer non_positive_integer;
  typedef integer non_negative_integer;
  typedef integer positive_integer;
  typedef integer negative_integer;

  // Boolean.
  //
  typedef bool boolean;

  // Floating-point types.
  //
  typedef float float_;
  typedef double double_;
  typedef double decimal;

  // String types.
  //
  typedef ::xsd::cxx::tree::string< char, simple_type > string;
  typedef ::xsd::cxx::tree::normalized_string< char, string > normalized_string;
  typedef ::xsd::cxx::tree::token< char, normalized_string > token;
  typedef ::xsd::cxx::tree::name< char, token > name;
  typedef ::xsd::cxx::tree::nmtoken< char, token > nmtoken;
  typedef ::xsd::cxx::tree::nmtokens< char, simple_type, nmtoken> nmtokens;
  typedef ::xsd::cxx::tree::ncname< char, name > ncname;
  typedef ::xsd::cxx::tree::language< char, token > language;

  // ID/IDREF.
  //
  typedef ::xsd::cxx::tree::id< char, ncname > id;
  typedef ::xsd::cxx::tree::idref< type, char, ncname > idref;
  typedef ::xsd::cxx::tree::idrefs< char, simple_type, idref > idrefs;

  // URI.
  //
  typedef ::xsd::cxx::tree::uri< char, simple_type > uri;

  // Qualified name.
  //
  typedef ::xsd::cxx::tree::qname< char, simple_type, uri, ncname > qname;

  // Binary.
  //
  typedef ::xsd::cxx::tree::buffer< char > buffer;
  typedef ::xsd::cxx::tree::base64_binary< char, simple_type > base64_binary;
  typedef ::xsd::cxx::tree::hex_binary< char, simple_type > hex_binary;

  // Date/time.
  //
  typedef ::xsd::cxx::tree::date< char, simple_type > date;
  typedef ::xsd::cxx::tree::date_time< char, simple_type > date_time;
  typedef ::xsd::cxx::tree::duration< char, simple_type > duration;
  typedef ::xsd::cxx::tree::day< char, simple_type > day;
  typedef ::xsd::cxx::tree::month< char, simple_type > month;
  typedef ::xsd::cxx::tree::month_day< char, simple_type > month_day;
  typedef ::xsd::cxx::tree::year< char, simple_type > year;
  typedef ::xsd::cxx::tree::year_month< char, simple_type > year_month;
  typedef ::xsd::cxx::tree::time< char, simple_type > time;

  // Entity.
  //
  typedef ::xsd::cxx::tree::entity< char, ncname > entity;
  typedef ::xsd::cxx::tree::entities< char, simple_type, entity > entities;

  // Flags and properties.
  //
  typedef ::xsd::cxx::tree::flags flags;
  typedef ::xsd::cxx::tree::properties< char > properties;

  // DOM user data key for back pointers to tree nodes.
  //
#ifndef XSD_CXX_TREE_TREE_NODE_KEY_IN___XML_SCHEMA
#define XSD_CXX_TREE_TREE_NODE_KEY_IN___XML_SCHEMA

  const XMLCh* const tree_node_key = ::xsd::cxx::tree::user_data_keys::node;

#endif

  // Exceptions.
  //
  typedef ::xsd::cxx::tree::exception< char > exception;
  typedef ::xsd::cxx::tree::parsing< char > parsing;
  typedef ::xsd::cxx::tree::expected_element< char > expected_element;
  typedef ::xsd::cxx::tree::unexpected_element< char > unexpected_element;
  typedef ::xsd::cxx::tree::expected_attribute< char > expected_attribute;
  typedef ::xsd::cxx::tree::unexpected_enumerator< char > unexpected_enumerator;
  typedef ::xsd::cxx::tree::expected_text_content< char > expected_text_content;
  typedef ::xsd::cxx::tree::no_type_info< char > no_type_info;
  typedef ::xsd::cxx::tree::not_derived< char > not_derived;
  typedef ::xsd::cxx::tree::duplicate_id< char > duplicate_id;
  typedef ::xsd::cxx::tree::serialization< char > serialization;
  typedef ::xsd::cxx::tree::no_namespace_mapping< char > no_namespace_mapping;
  typedef ::xsd::cxx::tree::no_prefix_mapping< char > no_prefix_mapping;
  typedef ::xsd::cxx::tree::xsi_already_in_use< char > xsi_already_in_use;
  typedef ::xsd::cxx::tree::bounds< char > bounds;

  // Parsing/serialization diagnostics.
  //
  typedef ::xsd::cxx::tree::severity severity;
  typedef ::xsd::cxx::tree::error< char > error;
  typedef ::xsd::cxx::tree::diagnostics< char > diagnostics;

  // Error handler interface.
  //
  typedef ::xsd::cxx::xml::error_handler< char > error_handler;
}

// Forward declarations.
//
namespace mmd
{
  class metadata;
  class artist;
  class life_span;
}


#include <memory>    // std::auto_ptr
#include <algorithm> // std::binary_search

#include <xsd/cxx/tree/exceptions.hxx>
#include <xsd/cxx/tree/elements.hxx>
#include <xsd/cxx/tree/containers.hxx>
#include <xsd/cxx/tree/list.hxx>

#include <xsd/cxx/xml/dom/parsing-header.hxx>

namespace mmd
{
  class metadata: public ::xml_schema::type
  {
    public:
    // artist
    // 
    typedef ::mmd::artist artist_type;
    typedef ::xsd::cxx::tree::traits< artist_type, char > artist_traits;

    const artist_type&
    artist () const;

    artist_type&
    artist ();

    void
    artist (const artist_type& x);

    void
    artist (::std::auto_ptr< artist_type > p);

    // Constructors.
    //
    metadata (const artist_type&);

    metadata (const ::xercesc::DOMElement& e,
              ::xml_schema::flags f = 0,
              ::xml_schema::type* c = 0);

    metadata (const metadata& x,
              ::xml_schema::flags f = 0,
              ::xml_schema::type* c = 0);

    virtual metadata*
    _clone (::xml_schema::flags f = 0,
            ::xml_schema::type* c = 0) const;

    // Implementation.
    //
    protected:
    void
    parse (::xsd::cxx::xml::dom::parser< char >&,
           ::xml_schema::flags);

    private:
    ::xsd::cxx::tree::one< artist_type > artist_;
  };

  class artist: public ::xml_schema::type
  {
    public:
    // name
    // 
    typedef ::xml_schema::string name_type;
    typedef ::xsd::cxx::tree::traits< name_type, char > name_traits;

    const name_type&
    name () const;

    name_type&
    name ();

    void
    name (const name_type& x);

    void
    name (::std::auto_ptr< name_type > p);

    // sort-name
    // 
    typedef ::xml_schema::string sort_name_type;
    typedef ::xsd::cxx::tree::traits< sort_name_type, char > sort_name_traits;

    const sort_name_type&
    sort_name () const;

    sort_name_type&
    sort_name ();

    void
    sort_name (const sort_name_type& x);

    void
    sort_name (::std::auto_ptr< sort_name_type > p);

    // life-span
    // 
    typedef ::mmd::life_span life_span_type;
    typedef ::xsd::cxx::tree::optional< life_span_type > life_span_optional;
    typedef ::xsd::cxx::tree::traits< life_span_type, char > life_span_traits;

    const life_span_optional&
    life_span () const;

    life_span_optional&
    life_span ();

    void
    life_span (const life_span_type& x);

    void
    life_span (const life_span_optional& x);

    void
    life_span (::std::auto_ptr< life_span_type > p);

    // id
    // 
    typedef ::xml_schema::string id_type;
    typedef ::xsd::cxx::tree::traits< id_type, char > id_traits;

    const id_type&
    id () const;

    id_type&
    id ();

    void
    id (const id_type& x);

    void
    id (::std::auto_ptr< id_type > p);

    // type
    // 
    typedef ::xml_schema::ncname type_type;
    typedef ::xsd::cxx::tree::traits< type_type, char > type_traits;

    const type_type&
    type () const;

    type_type&
    type ();

    void
    type (const type_type& x);

    void
    type (::std::auto_ptr< type_type > p);

    // Constructors.
    //
    artist (const name_type&,
            const sort_name_type&,
            const id_type&,
            const type_type&);

    artist (const ::xercesc::DOMElement& e,
            ::xml_schema::flags f = 0,
            ::xml_schema::type* c = 0);

    artist (const artist& x,
            ::xml_schema::flags f = 0,
            ::xml_schema::type* c = 0);

    virtual artist*
    _clone (::xml_schema::flags f = 0,
            ::xml_schema::type* c = 0) const;

    // Implementation.
    //
    protected:
    void
    parse (::xsd::cxx::xml::dom::parser< char >&,
           ::xml_schema::flags);

    private:
    ::xsd::cxx::tree::one< name_type > name_;
    ::xsd::cxx::tree::one< sort_name_type > sort_name_;
    life_span_optional life_span_;
    ::xsd::cxx::tree::one< id_type > id_;
    ::xsd::cxx::tree::one< type_type > type_;
  };

  class life_span: public ::xml_schema::type
  {
    public:
    // begin
    // 
    typedef ::xml_schema::nmtoken begin_type;
    typedef ::xsd::cxx::tree::optional< begin_type > begin_optional;
    typedef ::xsd::cxx::tree::traits< begin_type, char > begin_traits;

    const begin_optional&
    begin () const;

    begin_optional&
    begin ();

    void
    begin (const begin_type& x);

    void
    begin (const begin_optional& x);

    void
    begin (::std::auto_ptr< begin_type > p);

    // end
    // 
    typedef ::xml_schema::nmtoken end_type;
    typedef ::xsd::cxx::tree::optional< end_type > end_optional;
    typedef ::xsd::cxx::tree::traits< end_type, char > end_traits;

    const end_optional&
    end () const;

    end_optional&
    end ();

    void
    end (const end_type& x);

    void
    end (const end_optional& x);

    void
    end (::std::auto_ptr< end_type > p);

    // Constructors.
    //
    life_span ();

    life_span (const ::xercesc::DOMElement& e,
               ::xml_schema::flags f = 0,
               ::xml_schema::type* c = 0);

    life_span (const life_span& x,
               ::xml_schema::flags f = 0,
               ::xml_schema::type* c = 0);

    virtual life_span*
    _clone (::xml_schema::flags f = 0,
            ::xml_schema::type* c = 0) const;

    // Implementation.
    //
    protected:
    void
    parse (::xsd::cxx::xml::dom::parser< char >&,
           ::xml_schema::flags);

    private:
    begin_optional begin_;
    end_optional end_;
  };
}

#include <iosfwd>

#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMInputSource.hpp>
#include <xercesc/dom/DOMErrorHandler.hpp>

namespace mmd
{
  // Parse a URI or a local file.
  //

  ::std::auto_ptr< ::mmd::metadata >
  metadata_ (const ::std::string& uri,
             ::xml_schema::flags f = 0,
             const ::xml_schema::properties& p = ::xml_schema::properties ());

  ::std::auto_ptr< ::mmd::metadata >
  metadata_ (const ::std::string& uri,
             ::xml_schema::error_handler& eh,
             ::xml_schema::flags f = 0,
             const ::xml_schema::properties& p = ::xml_schema::properties ());

  ::std::auto_ptr< ::mmd::metadata >
  metadata_ (const ::std::string& uri,
             ::xercesc::DOMErrorHandler& eh,
             ::xml_schema::flags f = 0,
             const ::xml_schema::properties& p = ::xml_schema::properties ());

  // Parse std::istream.
  //

  ::std::auto_ptr< ::mmd::metadata >
  metadata_ (::std::istream& is,
             ::xml_schema::flags f = 0,
             const ::xml_schema::properties& p = ::xml_schema::properties ());

  ::std::auto_ptr< ::mmd::metadata >
  metadata_ (::std::istream& is,
             ::xml_schema::error_handler& eh,
             ::xml_schema::flags f = 0,
             const ::xml_schema::properties& p = ::xml_schema::properties ());

  ::std::auto_ptr< ::mmd::metadata >
  metadata_ (::std::istream& is,
             ::xercesc::DOMErrorHandler& eh,
             ::xml_schema::flags f = 0,
             const ::xml_schema::properties& p = ::xml_schema::properties ());

  ::std::auto_ptr< ::mmd::metadata >
  metadata_ (::std::istream& is,
             const ::std::string& id,
             ::xml_schema::flags f = 0,
             const ::xml_schema::properties& p = ::xml_schema::properties ());

  ::std::auto_ptr< ::mmd::metadata >
  metadata_ (::std::istream& is,
             const ::std::string& id,
             ::xml_schema::error_handler& eh,
             ::xml_schema::flags f = 0,
             const ::xml_schema::properties& p = ::xml_schema::properties ());

  ::std::auto_ptr< ::mmd::metadata >
  metadata_ (::std::istream& is,
             const ::std::string& id,
             ::xercesc::DOMErrorHandler& eh,
             ::xml_schema::flags f = 0,
             const ::xml_schema::properties& p = ::xml_schema::properties ());

  // Parse xercesc::DOMInputSource.
  //

  ::std::auto_ptr< ::mmd::metadata >
  metadata_ (const ::xercesc::DOMInputSource& is,
             ::xml_schema::flags f = 0,
             const ::xml_schema::properties& p = ::xml_schema::properties ());

  ::std::auto_ptr< ::mmd::metadata >
  metadata_ (const ::xercesc::DOMInputSource& is,
             ::xml_schema::error_handler& eh,
             ::xml_schema::flags f = 0,
             const ::xml_schema::properties& p = ::xml_schema::properties ());

  ::std::auto_ptr< ::mmd::metadata >
  metadata_ (const ::xercesc::DOMInputSource& is,
             ::xercesc::DOMErrorHandler& eh,
             ::xml_schema::flags f = 0,
             const ::xml_schema::properties& p = ::xml_schema::properties ());

  // Parse xercesc::DOMDocument.
  //

  ::std::auto_ptr< ::mmd::metadata >
  metadata_ (const ::xercesc::DOMDocument& d,
             ::xml_schema::flags f = 0,
             const ::xml_schema::properties& p = ::xml_schema::properties ());

  ::std::auto_ptr< ::mmd::metadata >
  metadata_ (::xercesc::DOMDocument* d,
             ::xml_schema::flags f = 0,
             const ::xml_schema::properties& p = ::xml_schema::properties ());
}

#include <xsd/cxx/post.hxx>

// Begin epilogue.
//
//
// End epilogue.

#endif // MB_ARTIST_BASIC_1_0_HXX
