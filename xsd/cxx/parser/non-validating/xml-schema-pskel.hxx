// file      : xsd/cxx/parser/non-validating/xml-schema-pskel.hxx
// author    : Boris Kolpackov <boris@codesynthesis.com>
// copyright : Copyright (c) 2005-2007 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_CXX_PARSER_NON_VALIDATING_XML_SCHEMA_PSKEL_HXX
#define XSD_CXX_PARSER_NON_VALIDATING_XML_SCHEMA_PSKEL_HXX

#include <string>
#include <memory> // auto_ptr

#include <xsd/cxx/parser/xml-schema.hxx>
#include <xsd/cxx/parser/non-validating/parser.hxx>

namespace xsd
{
  namespace cxx
  {
    namespace parser
    {
      namespace non_validating
      {
        // anyType and anySimpleType. All events are routed to the
        // _any_* hooks.
        //
        template <typename C>
        struct any_type_pskel: virtual complex_content<C>
        {
          virtual bool
          _start_element_impl (const ro_string<C>&,
                               const ro_string<C>&);

          virtual bool
          _end_element_impl (const ro_string<C>&,
                             const ro_string<C>&);

          virtual bool
          _attribute_impl (const ro_string<C>&,
                           const ro_string<C>&,
                           const ro_string<C>&);

          virtual bool
          _characters_impl (const ro_string<C>&);

          virtual void
          post_any_type () = 0;
        };

        template <typename C>
        struct any_simple_type_pskel: virtual simple_content<C>
        {
          virtual bool
          _characters_impl (const ro_string<C>&);

          virtual void
          post_any_simple_type () = 0;
        };


        // Boolean.
        //
        template <typename C>
        struct boolean_pskel: virtual simple_content<C>
        {
          virtual bool
          post_boolean () = 0;
        };


        // 8-bit
        //
        template <typename C>
        struct byte_pskel: virtual simple_content<C>
        {
          virtual signed char
          post_byte () = 0;
        };

        template <typename C>
        struct unsigned_byte_pskel: virtual simple_content<C>
        {
          virtual unsigned char
          post_unsigned_byte () = 0;
        };


        // 16-bit
        //
        template <typename C>
        struct short_pskel: virtual simple_content<C>
        {
          virtual short
          post_short () = 0;
        };

        template <typename C>
        struct unsigned_short_pskel: virtual simple_content<C>
        {
          virtual unsigned short
          post_unsigned_short () = 0;
        };


        // 32-bit
        //
        template <typename C>
        struct int_pskel: virtual simple_content<C>
        {
          virtual int
          post_int () = 0;
        };

        template <typename C>
        struct unsigned_int_pskel: virtual simple_content<C>
        {
          virtual unsigned int
          post_unsigned_int () = 0;
        };


        // 64-bit
        //
        template <typename C>
        struct long_pskel: virtual simple_content<C>
        {
          virtual long long
          post_long () = 0;
        };

        template <typename C>
        struct unsigned_long_pskel: virtual simple_content<C>
        {
          virtual unsigned long long
          post_unsigned_long () = 0;
        };


        // Arbitrary-length integers.
        //
        template <typename C>
        struct integer_pskel: virtual simple_content<C>
        {
          virtual long long
          post_integer () = 0;
        };

        template <typename C>
        struct negative_integer_pskel: virtual simple_content<C>
        {
          virtual long long
          post_negative_integer () = 0;
        };

        template <typename C>
        struct non_positive_integer_pskel: virtual simple_content<C>
        {
          virtual long long
          post_non_positive_integer () = 0;
        };

        template <typename C>
        struct positive_integer_pskel: virtual simple_content<C>
        {
          virtual unsigned long long
          post_positive_integer () = 0;
        };

        template <typename C>
        struct non_negative_integer_pskel: virtual simple_content<C>
        {
          virtual unsigned long long
          post_non_negative_integer () = 0;
        };


        // Floats.
        //
        template <typename C>
        struct float_pskel: virtual simple_content<C>
        {
          virtual float
          post_float () = 0;
        };

        template <typename C>
        struct double_pskel: virtual simple_content<C>
        {
          virtual double
          post_double () = 0;
        };

        template <typename C>
        struct decimal_pskel: virtual simple_content<C>
        {
          virtual double
          post_decimal () = 0;
        };


        // Strings.
        //
        template <typename C>
        struct string_pskel: virtual simple_content<C>
        {
          virtual std::basic_string<C>
          post_string () = 0;
        };

        template <typename C>
        struct normalized_string_pskel: virtual simple_content<C>
        {
          virtual std::basic_string<C>
          post_normalized_string () = 0;
        };

        template <typename C>
        struct token_pskel: virtual simple_content<C>
        {
          virtual std::basic_string<C>
          post_token () = 0;
        };

        template <typename C>
        struct name_pskel: virtual simple_content<C>
        {
          virtual std::basic_string<C>
          post_name () = 0;
        };

        template <typename C>
        struct nmtoken_pskel: virtual simple_content<C>
        {
          virtual std::basic_string<C>
          post_nmtoken () = 0;
        };

        template <typename C>
        struct nmtokens_pskel: list_base<C>
        {
          virtual string_sequence<C>
          post_nmtokens () = 0;
        };

        template <typename C>
        struct ncname_pskel: virtual simple_content<C>
        {
          virtual std::basic_string<C>
          post_ncname () = 0;
        };

        template <typename C>
        struct id_pskel: virtual simple_content<C>
        {
          virtual std::basic_string<C>
          post_id () = 0;
        };

        template <typename C>
        struct idref_pskel: virtual simple_content<C>
        {
          virtual std::basic_string<C>
          post_idref () = 0;
        };

        template <typename C>
        struct idrefs_pskel: list_base<C>
        {
          virtual string_sequence<C>
          post_idrefs () = 0;
        };

        // Language.
        //
        template <typename C>
        struct language_pskel: virtual simple_content<C>
        {
          virtual std::basic_string<C>
          post_language () = 0;
        };

        // URI.
        //
        template <typename C>
        struct uri_pskel: virtual simple_content<C>
        {
          virtual std::basic_string<C>
          post_uri () = 0;
        };

        // QName.
        //
        template <typename C>
        struct qname_pskel: virtual simple_content<C>
        {
          virtual qname<C>
          post_qname () = 0;
        };

        // Base64 and hex binaries.
        //
        template <typename C>
        struct base64_binary_pskel: virtual simple_content<C>
        {
          virtual std::auto_ptr<buffer>
          post_base64_binary () = 0;
        };

        template <typename C>
        struct hex_binary_pskel: virtual simple_content<C>
        {
          virtual std::auto_ptr<buffer>
          post_hex_binary () = 0;
        };

        // Time and date types.
        //
        template <typename C>
        struct gday_pskel: virtual simple_content<C>
        {
          virtual gday<C>
          post_gday () = 0;
        };

        template <typename C>
        struct gmonth_pskel: virtual simple_content<C>
        {
          virtual gmonth<C>
          post_gmonth () = 0;
        };

        template <typename C>
        struct gyear_pskel: virtual simple_content<C>
        {
          virtual gyear<C>
          post_gyear () = 0;
        };

        template <typename C>
        struct gmonth_day_pskel: virtual simple_content<C>
        {
          virtual gmonth_day<C>
          post_gmonth_day () = 0;
        };

        template <typename C>
        struct gyear_month_pskel: virtual simple_content<C>
        {
          virtual gyear_month<C>
          post_gyear_month () = 0;
        };

        template <typename C>
        struct date_pskel: virtual simple_content<C>
        {
          virtual date<C>
          post_date () = 0;
        };

        template <typename C>
        struct time_pskel: virtual simple_content<C>
        {
          virtual time<C>
          post_time () = 0;
        };

        template <typename C>
        struct date_time_pskel: virtual simple_content<C>
        {
          virtual date_time<C>
          post_date_time () = 0;
        };

        template <typename C>
        struct duration_pskel: virtual simple_content<C>
        {
          virtual duration
          post_duration () = 0;
        };
      }
    }
  }
}

#include <xsd/cxx/parser/non-validating/xml-schema-pskel.txx>

#endif  // XSD_CXX_PARSER_NON_VALIDATING_XML_SCHEMA_PSKEL_HXX
