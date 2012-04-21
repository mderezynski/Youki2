// (C) 2007 MPX Project, contributors see AUTHORS

#include <string>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <list>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMDocumentType.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMNodeIterator.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/dom/DOMText.hpp>

#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/framework/LocalFileInputSource.hpp>

#include <string>
#include <stdexcept>

namespace MPX
{
    using namespace xercesc;

    template<typename T>
    struct XmlInstance
    {
        private:
            T * m_Xml;
            XercesDOMParser * m_ConfigFileParser;

        public:

            ~XmlInstance ()
            {
                delete m_Xml;
                delete m_ConfigFileParser;
            }

            explicit XmlInstance ()
            : m_Xml (0)
            , m_ConfigFileParser (0)
            {
            }

            explicit XmlInstance (const std::string& url)
            : m_Xml (0)
            , m_ConfigFileParser (0)
            {
                XMLPlatformUtils::Initialize();

                m_ConfigFileParser = new XercesDOMParser;
                m_ConfigFileParser->setValidationScheme( XercesDOMParser::Val_Never );
                m_ConfigFileParser->setDoNamespaces( true );
                m_ConfigFileParser->setDoSchema( false );
                m_ConfigFileParser->setLoadExternalDTD( false );

                try{
                  m_ConfigFileParser->parse(url.c_str());

                  // no need to free this pointer - owned by the parent parser object
                  DOMDocument* xmlDoc = m_ConfigFileParser->getDocument();
                  if( !xmlDoc ) throw(std::runtime_error("No Document!"));

                  // Get the top-level element: NAme is "root". No attributes for "root"

                  DOMElement* elementRoot = xmlDoc->getDocumentElement();
                  if( !elementRoot ) throw(std::runtime_error("No Root element found!"));

                  m_Xml = new T (*elementRoot);
                }
                catch( xercesc::XMLException& e )
                {
                  char* message = xercesc::XMLString::transcode( e.getMessage() );
                  std::string msg_cpp ( message );
                  XMLString::release( &message );
                  throw(std::runtime_error(msg_cpp));
                }
            };

            T& xml ()
            {
                if(!m_Xml)
                    throw std::runtime_error ("No instance");

                return *m_Xml;
            };
    };
};
