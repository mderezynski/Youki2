// file      : xsd/cxx/parser/xerces/elements.txx
// author    : Boris Kolpackov <boris@codesynthesis.com>
// copyright : Copyright (c) 2005-2007 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <istream>
#include <cstddef> // std::size_t

#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>

#include <xsd/cxx/xml/string.hxx>
#include <xsd/cxx/xml/sax/std-input-source.hxx>
#include <xsd/cxx/xml/sax/bits/error-handler-proxy.hxx>

#include <xsd/cxx/parser/error-handler.hxx>
#include <xsd/cxx/parser/schema-exceptions.hxx>

namespace xsd
{
  namespace cxx
  {
    namespace parser
    {
      namespace xerces
      {

        // document
        //

        template <typename C>
        document<C>::
        document (parser_base<C>& parser,
                  const std::basic_string<C>& name)
            : cxx::parser::document<C> (parser, std::basic_string<C> (), name)
        {
        }

        template <typename C>
        document<C>::
        document (parser_base<C>& parser,
                  const std::basic_string<C>& ns,
                  const std::basic_string<C>& name)
            : cxx::parser::document<C> (parser, ns, name)
        {
        }

        template <typename C>
        document<C>::
        document ()
        {
        }

        // parse (uri)
        //
        template <typename C>
        void document<C>::
        parse (const std::basic_string<C>& uri,
               flags f,
               const properties<C>& p)
        {
          xml::auto_initializer init ((f & flags::dont_initialize) == 0);

          error_handler<C> eh;
          xml::sax::bits::error_handler_proxy<C> eh_proxy (eh);
          std::auto_ptr<xercesc::SAX2XMLReader> sax (create_sax_ (f, p));

          parse (uri, eh_proxy, *sax, f, p);

          eh.throw_if_failed ();
        }

        template <typename C>
        void document<C>::
        parse (const C* uri,
               flags f,
               const properties<C>& p)
        {
          parse (std::basic_string<C> (uri), f, p);
        }

        // error_handler
        //

        template <typename C>
        void document<C>::
        parse (const std::basic_string<C>& uri,
               xml::error_handler<C>& eh,
               flags f,
               const properties<C>& p)
        {
          xml::auto_initializer init ((f & flags::dont_initialize) == 0);

          xml::sax::bits::error_handler_proxy<C> eh_proxy (eh);
          std::auto_ptr<xercesc::SAX2XMLReader> sax (create_sax_ (f, p));

          parse (uri, eh_proxy, *sax, f, p);

          if (eh_proxy.failed ())
            throw parsing<C> ();
        }

        template <typename C>
        void document<C>::
        parse (const C* uri,
               xml::error_handler<C>& eh,
               flags f,
               const properties<C>& p)
        {
          parse (std::basic_string<C> (uri), eh, f, p);
        }

        // ErrorHandler
        //

        template <typename C>
        void document<C>::
        parse (const std::basic_string<C>& uri,
               xercesc::ErrorHandler& eh,
               flags f,
               const properties<C>& p)
        {
          xml::sax::bits::error_handler_proxy<C> eh_proxy (eh);
          std::auto_ptr<xercesc::SAX2XMLReader> sax (create_sax_ (f, p));

          parse (uri, eh_proxy, *sax, f, p);

          if (eh_proxy.failed ())
            throw parsing<C> ();
        }

        template <typename C>
        void document<C>::
        parse (const C* uri,
               xercesc::ErrorHandler& eh,
               flags f,
               const properties<C>& p)
        {
          parse (std::basic_string<C> (uri), eh, f, p);
        }

        // SAX2XMLReader
        //

        template <typename C>
        void document<C>::
        parse (const std::basic_string<C>& uri,
               xercesc::SAX2XMLReader& sax,
               flags f,
               const properties<C>& p)
        {
          // If there is no error handler, then fall back on the default
          // implementation.
          //
          xercesc::ErrorHandler* eh (sax.getErrorHandler ());

          if (eh)
          {
            xml::sax::bits::error_handler_proxy<C> eh_proxy (*eh);

            parse (uri, eh_proxy, sax, f, p);

            if (eh_proxy.failed ())
              throw parsing<C> ();
          }
          else
          {
            error_handler<C> fallback_eh;
            xml::sax::bits::error_handler_proxy<C> eh_proxy (fallback_eh);

            parse (uri, eh_proxy, sax, f, p);

            fallback_eh.throw_if_failed ();
          }
        }

        template <typename C>
        void document<C>::
        parse (const C* uri,
               xercesc::SAX2XMLReader& sax,
               flags f,
               const properties<C>& p)
        {
          parse (std::basic_string<C> (uri), sax, f, p);
        }

        // parse (istream)
        //

        template <typename C>
        void document<C>::
        parse (std::istream& is,
               flags f,
               const properties<C>& p)
        {
          xml::auto_initializer init ((f & flags::dont_initialize) == 0);

          xml::sax::std_input_source isrc (is);

          parse (isrc, f, p);
        }

        template <typename C>
        void document<C>::
        parse (std::istream& is,
               xml::error_handler<C>& eh,
               flags f,
               const properties<C>& p)
        {
          xml::auto_initializer init ((f & flags::dont_initialize) == 0);
          xml::sax::std_input_source isrc (is);
          parse (isrc, eh, f, p);
        }

        template <typename C>
        void document<C>::
        parse (std::istream& is,
               xercesc::ErrorHandler& eh,
               flags f,
               const properties<C>& p)
        {
          xml::sax::std_input_source isrc (is);
          parse (isrc, eh, f, p);
        }

        template <typename C>
        void document<C>::
        parse (std::istream& is,
               xercesc::SAX2XMLReader& sax,
               flags f,
               const properties<C>& p)
        {
          xml::sax::std_input_source isrc (is);
          parse (isrc, sax, f, p);
        }


        // parse (istream, system_id)
        //


        template <typename C>
        void document<C>::
        parse (std::istream& is,
               const std::basic_string<C>& system_id,
               flags f,
               const properties<C>& p)
        {
          xml::auto_initializer init ((f & flags::dont_initialize) == 0);
          xml::sax::std_input_source isrc (is, system_id);
          parse (isrc, f, p);
        }

        template <typename C>
        void document<C>::
        parse (std::istream& is,
               const std::basic_string<C>& system_id,
               xml::error_handler<C>& eh,
               flags f,
               const properties<C>& p)
        {
          xml::auto_initializer init ((f & flags::dont_initialize) == 0);
          xml::sax::std_input_source isrc (is, system_id);
          parse (isrc, eh, f, p);
        }

        template <typename C>
        void document<C>::
        parse (std::istream& is,
               const std::basic_string<C>& system_id,
               xercesc::ErrorHandler& eh,
               flags f,
               const properties<C>& p)
        {
          xml::sax::std_input_source isrc (is, system_id);
          parse (isrc, eh, f, p);
        }

        template <typename C>
        void document<C>::
        parse (std::istream& is,
               const std::basic_string<C>& system_id,
               xercesc::SAX2XMLReader& sax,
               flags f,
               const properties<C>& p)
        {
          xml::sax::std_input_source isrc (is, system_id);
          parse (isrc, sax, f, p);
        }


        // parse (istream, system_id, public_id)
        //

        template <typename C>
        void document<C>::
        parse (std::istream& is,
               const std::basic_string<C>& system_id,
               const std::basic_string<C>& public_id,
               flags f,
               const properties<C>& p)
        {
          xml::auto_initializer init ((f & flags::dont_initialize) == 0);
          xml::sax::std_input_source isrc (is, system_id, public_id);
          parse (isrc, f, p);
        }

        template <typename C>
        void document<C>::
        parse (std::istream& is,
               const std::basic_string<C>& system_id,
               const std::basic_string<C>& public_id,
               xml::error_handler<C>& eh,
               flags f,
               const properties<C>& p)
        {
          xml::auto_initializer init ((f & flags::dont_initialize) == 0);
          xml::sax::std_input_source isrc (is, system_id, public_id);
          parse (isrc, eh, f, p);
        }

        template <typename C>
        void document<C>::
        parse (std::istream& is,
               const std::basic_string<C>& system_id,
               const std::basic_string<C>& public_id,
               xercesc::ErrorHandler& eh,
               flags f,
               const properties<C>& p)
        {
          xml::sax::std_input_source isrc (is, system_id, public_id);
          parse (isrc, eh, f, p);
        }

        template <typename C>
        void document<C>::
        parse (std::istream& is,
               const std::basic_string<C>& system_id,
               const std::basic_string<C>& public_id,
               xercesc::SAX2XMLReader& sax,
               flags f,
               const properties<C>& p)
        {
          xml::sax::std_input_source isrc (is, system_id, public_id);
          parse (isrc, sax, f, p);
        }


        // parse (InputSource)
        //


        template <typename C>
        void document<C>::
        parse (const xercesc::InputSource& is,
               flags f,
               const properties<C>& p)
        {
          error_handler<C> eh;
          xml::sax::bits::error_handler_proxy<C> eh_proxy (eh);
          std::auto_ptr<xercesc::SAX2XMLReader> sax (create_sax_ (f, p));

          parse (is, eh_proxy, *sax, f, p);

          eh.throw_if_failed ();
        }

        template <typename C>
        void document<C>::
        parse (const xercesc::InputSource& is,
               xml::error_handler<C>& eh,
               flags f,
               const properties<C>& p)
        {
          xml::sax::bits::error_handler_proxy<C> eh_proxy (eh);
          std::auto_ptr<xercesc::SAX2XMLReader> sax (create_sax_ (f, p));

          parse (is, eh_proxy, *sax, f, p);

          if (eh_proxy.failed ())
            throw parsing<C> ();
        }

        template <typename C>
        void document<C>::
        parse (const xercesc::InputSource& is,
               xercesc::ErrorHandler& eh,
               flags f,
               const properties<C>& p)
        {
          xml::sax::bits::error_handler_proxy<C> eh_proxy (eh);
          std::auto_ptr<xercesc::SAX2XMLReader> sax (create_sax_ (f, p));

          parse (is, eh_proxy, *sax, f, p);

          if (eh_proxy.failed ())
            throw parsing<C> ();
        }


        template <typename C>
        void document<C>::
        parse (const xercesc::InputSource& is,
               xercesc::SAX2XMLReader& sax,
               flags f,
               const properties<C>& p)
        {
          // If there is no error handler, then fall back on the default
          // implementation.
          //
          xercesc::ErrorHandler* eh (sax.getErrorHandler ());

          if (eh)
          {
            xml::sax::bits::error_handler_proxy<C> eh_proxy (*eh);

            parse (is, eh_proxy, sax, f, p);

            if (eh_proxy.failed ())
              throw parsing<C> ();
          }
          else
          {
            error_handler<C> fallback_eh;
            xml::sax::bits::error_handler_proxy<C> eh_proxy (fallback_eh);

            parse (is, eh_proxy, sax, f, p);

            fallback_eh.throw_if_failed ();
          }
        }

        namespace Bits
        {
          struct ErrorHandlingController
          {
            ErrorHandlingController (xercesc::SAX2XMLReader& sax,
                                     xercesc::ErrorHandler& eh)
                : sax_ (sax), eh_ (sax_.getErrorHandler ())
            {
              sax_.setErrorHandler (&eh);
            }

            ~ErrorHandlingController ()
            {
              sax_.setErrorHandler (eh_);
            }

          private:
            xercesc::SAX2XMLReader& sax_;
            xercesc::ErrorHandler* eh_;
          };

          struct ContentHandlingController
          {
            ContentHandlingController (xercesc::SAX2XMLReader& sax,
                                       xercesc::ContentHandler& ch)
                : sax_ (sax), ch_ (sax_.getContentHandler ())
            {
              sax_.setContentHandler (&ch);
            }

            ~ContentHandlingController ()
            {
              sax_.setContentHandler (ch_);
            }

          private:
            xercesc::SAX2XMLReader& sax_;
            xercesc::ContentHandler* ch_;
          };
        };

        template <typename C>
        void document<C>::
        parse (const std::basic_string<C>& uri,
               xercesc::ErrorHandler& eh,
               xercesc::SAX2XMLReader& sax,
               flags,
               const properties<C>&)
        {
          event_router<C> router (*this);

          Bits::ErrorHandlingController ehc (sax, eh);
          Bits::ContentHandlingController chc (sax, router);

          try
          {
            sax.parse (xml::string (uri).c_str ());
          }
          catch (const schema_exception<C>& e)
          {
            xml::string id (e.id ());

            xercesc::SAXParseException se (
              xml::string (e.message ()).c_str (),
              id.c_str (),
              id.c_str (),
              static_cast<XMLSSize_t> (e.line ()),
              static_cast<XMLSSize_t> (e.column ()));

            eh.fatalError (se);
          }
        }

        template <typename C>
        void document<C>::
        parse (const xercesc::InputSource& is,
               xercesc::ErrorHandler& eh,
               xercesc::SAX2XMLReader& sax,
               flags,
               const properties<C>&)
        {
          event_router<C> router (*this);

          Bits::ErrorHandlingController controller (sax, eh);
          Bits::ContentHandlingController chc (sax, router);

          try
          {
            sax.parse (is);
          }
          catch (const schema_exception<C>& e)
          {
            xml::string id (e.id ());

            xercesc::SAXParseException se (
              xml::string (e.message ()).c_str (),
              id.c_str (),
              id.c_str (),
              static_cast<XMLSSize_t> (e.line ()),
              static_cast<XMLSSize_t> (e.column ()));

            eh.fatalError (se);
          }
        }


        template <typename C>
        std::auto_ptr<xercesc::SAX2XMLReader> document<C>::
        create_sax_ (flags f, const properties<C>& p)
        {
          // HP aCC cannot handle using namespace xercesc;
          //
          using xercesc::SAX2XMLReader;
          using xercesc::XMLReaderFactory;
          using xercesc::XMLUni;

          std::auto_ptr<SAX2XMLReader> sax (
            XMLReaderFactory::createXMLReader ());

          sax->setFeature (XMLUni::fgSAX2CoreNameSpaces, true);
          sax->setFeature (XMLUni::fgSAX2CoreNameSpacePrefixes, true);
          sax->setFeature (XMLUni::fgXercesValidationErrorAsFatal, true);

          if (f & flags::dont_validate)
          {
            sax->setFeature (XMLUni::fgSAX2CoreValidation, false);
            sax->setFeature (XMLUni::fgXercesSchema, false);
            sax->setFeature (XMLUni::fgXercesSchemaFullChecking, false);
          }
          else
          {
            sax->setFeature (XMLUni::fgSAX2CoreValidation, true);
            sax->setFeature (XMLUni::fgXercesSchema, true);

            // This feature checks the schema grammar for additional
            // errors. We most likely do not need it when validating
            // instances (assuming the schema is valid).
            //
            sax->setFeature (XMLUni::fgXercesSchemaFullChecking, false);
          }

          // Transfer properies if any.
          //

          if (!p.schema_location ().empty ())
          {
            xml::string sl (p.schema_location ());
            const void* v (sl.c_str ());

            sax->setProperty (
              XMLUni::fgXercesSchemaExternalSchemaLocation,
              const_cast<void*> (v));
          }

          if (!p.no_namespace_schema_location ().empty ())
          {
            xml::string sl (p.no_namespace_schema_location ());
            const void* v (sl.c_str ());

            sax->setProperty (
              XMLUni::fgXercesSchemaExternalNoNameSpaceSchemaLocation,
              const_cast<void*> (v));
          }

          return sax;
        }

        // event_router
        //
        template <typename C>
        event_router<C>::
        event_router (cxx::parser::document<C>& consumer)
            : loc_ (0), consumer_ (consumer)
        {
        }

        template <typename C>
        void event_router<C>::
        setDocumentLocator (const xercesc::Locator* const loc)
        {
          loc_ = loc;
        }

        template <typename C>
        void event_router<C>::
        startElement(const XMLCh* const uri,
                     const XMLCh* const lname,
                     const XMLCh* const /*qname*/,
                     const xercesc::Attributes& attributes)
        {
          typedef std::basic_string<C> string;

          {
            string ns (xml::transcode<C> (uri));
            string name (xml::transcode<C> (lname));

            // Without this explicit construction IBM XL C++ complains
            // about ro_string's copy ctor being private even though the
            // temporary has been eliminated. Note that we cannot
            // eliminate ns and name since ro_string does not make a copy.
            //
            ro_string<C> ro_ns (ns);
            ro_string<C> ro_name (name);

            try
            {
              consumer_.start_element (ro_ns, ro_name);
            }
            catch (schema_exception<C>& e)
            {
              set_location (e);
              throw;
            }
          }

          // Xerces SAX uses unsigned int for indexing.
          //
          for (unsigned int i (0); i < attributes.getLength(); ++i)
          {
            string ns (xml::transcode<C> (attributes.getURI (i)));
            string name (xml::transcode<C> (attributes.getLocalName (i)));
            string value (xml::transcode<C> (attributes.getValue (i)));

            // Without this explicit construction IBM XL C++ complains
            // about ro_string's copy ctor being private even though the
            // temporary has been eliminated. Note that we cannot
            // eliminate ns, name and value since ro_string does not make
            // a copy.
            //
            ro_string<C> ro_ns (ns);
            ro_string<C> ro_name (name);
            ro_string<C> ro_value (value);

            try
            {
              consumer_.attribute (ro_ns, ro_name, ro_value);
            }
            catch (schema_exception<C>& e)
            {
              set_location (e);
              throw;
            }
          }
        }

        template <typename C>
        void event_router<C>::
        endElement(const XMLCh* const uri,
                   const XMLCh* const lname,
                   const XMLCh* const /*qname*/)
        {
          typedef std::basic_string<C> string;

          string ns (xml::transcode<C> (uri));
          string name (xml::transcode<C> (lname));

          // Without this explicit construction IBM XL C++ complains
          // about ro_string's copy ctor being private even though the
          // temporary has been eliminated. Note that we cannot
          // eliminate ns and name since ro_string does not make a copy.
          //
          ro_string<C> ro_ns (ns);
          ro_string<C> ro_name (name);

          try
          {
            consumer_.end_element (ro_ns, ro_name);
          }
          catch (schema_exception<C>& e)
          {
            set_location (e);
            throw;
          }
        }

        template <typename C>
        void event_router<C>::
        characters (const XMLCh* const s, const unsigned int n)
        {
          typedef std::basic_string<C> string;

          if (n != 0)
          {
            string str (xml::transcode<C> (s, n));

            // Without this explicit construction IBM XL C++ complains
            // about ro_string's copy ctor being private even though the
            // temporary has been eliminated. Note that we cannot
            // eliminate str since ro_string does not make a copy.
            //
            ro_string<C> ro_str (str);

            try
            {
              consumer_.characters (ro_str);
            }
            catch (schema_exception<C>& e)
            {
              set_location (e);
              throw;
            }
          }
        }

        template <typename C>
        void event_router<C>::
        set_location (schema_exception<C>& e)
        {
          if (loc_ != 0)
          {
            const XMLCh* id (loc_->getPublicId ());

            if (id == 0)
              id = loc_->getSystemId ();

            if (id != 0)
              e.id (xml::transcode<C> (id));

            XMLSSize_t l (loc_->getLineNumber ());
            XMLSSize_t c (loc_->getColumnNumber ());

            e.line (l == -1 ? 0 : static_cast<unsigned long> (l));
            e.column (c == -1 ? 0: static_cast<unsigned long> (c));
          }
        }
      }
    }
  }
}
