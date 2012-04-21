// file      : xsd/cxx/tree/xdr-stream-extraction.hxx
// author    : Boris Kolpackov <boris@codesynthesis.com>
// copyright : Copyright (c) 2005-2007 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_CXX_TREE_XDR_STREAM_EXTRACTION_HXX
#define XSD_CXX_TREE_XDR_STREAM_EXTRACTION_HXX

#include <rpc/xdr.h>

#include <string>

#include <xsd/cxx/tree/buffer.hxx>
#include <xsd/cxx/tree/istream.hxx>
#include <xsd/cxx/tree/xdr-stream-common.hxx>

namespace xsd
{
  namespace cxx
  {
    namespace tree
    {
      struct xdr_stream_extraction: xdr_stream_operation
      {
        virtual const char*
        what () const throw ()
        {
          return "XDR stream extraction operation failed";
        }
      };


      // as_size
      //
#ifdef XSD_CXX_TREE_USE_64_BIT_SIZE
      template <typename X>
      inline istream<XDR>&
      operator>> (istream<XDR>& s, istream<XDR>::as_size<X>& x)
      {
        uint64_t v;

        if (!xdr_uint64_t (&s.impl (), &v) || v > ~(X (0)))
          throw xdr_stream_extraction ();

        x.x_ = static_cast<X> (v);

        return s;
      }
#else
      template <typename X>
      inline istream<XDR>&
      operator>> (istream<XDR>& s, istream<XDR>::as_size<X>& x)
      {
        uint32_t v;

        if (!xdr_uint32_t (&s.impl (), &v))
          throw xdr_stream_extraction ();

        x.x_ = static_cast<X> (v);

        return s;
      }
#endif


      // 8-bit
      //
      template <typename X>
      inline istream<XDR>&
      operator>> (istream<XDR>& s, istream<XDR>::as_int8<X>& x)
      {
        int8_t v;

        if (!xdr_int8_t (&s.impl (), &v))
          throw xdr_stream_extraction ();

        x.x_ = static_cast<X> (v);

        return s;
      }

      template <typename X>
      inline istream<XDR>&
      operator>> (istream<XDR>& s, istream<XDR>::as_uint8<X>& x)
      {
        uint8_t v;

        if (!xdr_uint8_t (&s.impl (), &v))
          throw xdr_stream_extraction ();

        x.x_ = static_cast<X> (v);

        return s;
      }


      // 16-bit
      //
      template <typename X>
      inline istream<XDR>&
      operator>> (istream<XDR>& s, istream<XDR>::as_int16<X>& x)
      {
        int16_t v;

        if (!xdr_int16_t (&s.impl (), &v))
          throw xdr_stream_extraction ();

        x.x_ = static_cast<X> (v);

        return s;
      }

      template <typename X>
      inline istream<XDR>&
      operator>> (istream<XDR>& s, istream<XDR>::as_uint16<X>& x)
      {
        uint16_t v;

        if (!xdr_uint16_t (&s.impl (), &v))
          throw xdr_stream_extraction ();

        x.x_ = static_cast<X> (v);

        return s;
      }


      // 32-bit
      //
      template <typename X>
      inline istream<XDR>&
      operator>> (istream<XDR>& s, istream<XDR>::as_int32<X>& x)
      {
        int32_t v;

        if (!xdr_int32_t (&s.impl (), &v))
          throw xdr_stream_extraction ();

        x.x_ = static_cast<X> (v);

        return s;
      }

      template <typename X>
      inline istream<XDR>&
      operator>> (istream<XDR>& s, istream<XDR>::as_uint32<X>& x)
      {
        uint32_t v;

        if (!xdr_uint32_t (&s.impl (), &v))
          throw xdr_stream_extraction ();

        x.x_ = static_cast<X> (v);

        return s;
      }


      // 64-bit
      //
      template <typename X>
      inline istream<XDR>&
      operator>> (istream<XDR>& s, istream<XDR>::as_int64<X>& x)
      {
        int64_t v;

        if (!xdr_int64_t (&s.impl (), &v))
          throw xdr_stream_extraction ();

        x.x_ = static_cast<X> (v);

        return s;
      }

      template <typename X>
      inline istream<XDR>&
      operator>> (istream<XDR>& s, istream<XDR>::as_uint64<X>& x)
      {
        uint64_t v;

        if (!xdr_uint64_t (&s.impl (), &v))
          throw xdr_stream_extraction ();

        x.x_ = static_cast<X> (v);

        return s;
      }


      // Boolean
      //
      template <typename X>
      inline istream<XDR>&
      operator>> (istream<XDR>& s, istream<XDR>::as_bool<X>& x)
      {
        bool_t v;

        if (!xdr_bool (&s.impl (), &v))
          throw xdr_stream_extraction ();

        x.x_ = static_cast<X> (v);

        return s;
      }


      // Floating-point
      //
      template <typename X>
      inline istream<XDR>&
      operator>> (istream<XDR>& s, istream<XDR>::as_float32<X>& x)
      {
        float v;

        if (!xdr_float (&s.impl (), &v))
          throw xdr_stream_extraction ();

        x.x_ = static_cast<X> (v);

        return s;
      }

      template <typename X>
      inline istream<XDR>&
      operator>> (istream<XDR>& s, istream<XDR>::as_float64<X>& x)
      {
        double v;

        if (!xdr_double (&s.impl (), &v))
          throw xdr_stream_extraction ();

        x.x_ = static_cast<X> (v);

        return s;
      }

      // Extraction of std::basic_string.
      //

      inline istream<XDR>&
      operator>> (istream<XDR>& s, std::basic_string<char>& x)
      {
        unsigned int n;

        if (!xdr_u_int (&s.impl (), &n))
          throw xdr_stream_extraction ();

        // Dangerous but fast.
        //
        x.resize (n);
        char* p (const_cast<char*> (x.c_str ()));

        if (!xdr_opaque (&s.impl (), p, n))
          throw xdr_stream_extraction ();

        return s;
      }

      // Wide strings are not supported by XDR.
      //
      // inline istream<XDR>&
      // operator>> (istream<XDR>& s, std::basic_string<wchar_t>& x)
      // {
      // }


      // Extraction of a binary buffer.
      //
      template <typename C>
      istream<XDR>&
      operator>> (istream<XDR>& s, buffer<C>& x)
      {
        unsigned int n;

        if (!xdr_u_int (&s.impl (), &n))
          throw xdr_stream_extraction ();

        x.size (n);

        if (!xdr_opaque (&s.impl (), x.data (), n))
          throw xdr_stream_extraction ();

        return s;
      }
    }
  }
}

#endif  // XSD_CXX_TREE_XDR_STREAM_EXTRACTION_HXX
