/*  MCS
 *  Copyright (C) 2005-2006 M. Derezynski 
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License Version 2 as published
 *  by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */


#ifndef HAVE_BMP

#include <mcs/config.h>
#if (1 != MCS_HAVE_XML)
#  error "This MCS installation was built without XML backend support!"
#endif

#endif

#ifndef MCS_BASE_H 
#define MCS_BASE_H 

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xmlreader.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include <boost/variant.hpp>
#include <boost/format.hpp>
#include <sigc++/sigc++.h>
#include <glibmm.h>

#include <mcs/types.h>
#include <mcs/key.h>
#include <mcs/subscriber.h>

#define MCS_CB_DEFAULT_SIGNATURE	\
      const ::std::string& domain,	\
      const ::std::string& key,		\
      const ::Mcs::KeyVariant& value

namespace Mcs
{
      class Config
      {
        public:

            enum VersionIgnore
            {
              VERSION_CHECK,      //Checks the version specified in the root node and reports a mismatch
              VERSION_IGNORE      //Ignores the version and just attempts to load the configuration 'as is'
            };

            struct NoKeyException : public std::runtime_error
            {
                NoKeyException(const std::string& message)
                : std::runtime_error(message)
                {}
            };

            struct ParseException : public std::runtime_error
            {
                ParseException(const std::string& message)
                : std::runtime_error(message)
                {}
            };

            Config(
                std::string const& /* m_xml*/,
                std::string const& /*m_root_node_name*/,
                double             /*version*/
            );

            ~Config ();

            void
            load(
                Config::VersionIgnore /*version_ignore*/ = VERSION_CHECK
            ); 

            void
            save(
            );

            void
            domain_register(
                std::string const& /*domain*/
            );

            void
            key_register(
                std::string const& /*domain*/,
                std::string const& /*key*/,
                KeyVariant  const& /*key_default*/
            );

            inline bool
            domain_key_exist(
                std::string const& domain,
                std::string const& key
            )
            {
                return (m_domains.find (domain) != m_domains.end()) && (m_domains.find (domain)->second.find(key) != m_domains.find(domain)->second.end ());
            }

            Key&
            key(
                std::string const& domain,
                std::string const& key
            )
            {   
              if(!domain_key_exist(domain, key))
              {
                throw NoKeyException((boost::format ("MCS key() : Domain [%s] Key [%s] does not exist") % domain % key).str());
              }

              return m_domains.find(domain)->second.find(key)->second;
            }
                
            template <typename T>
            void 
            key_set(
                std::string const& domain,
                std::string const& key,
                T const&           value
            ) 
            {
              g_return_if_fail(domain_key_exist (domain, key));

              m_domains.find(domain)->second.find(key)->second.set_value<T>(value);
            }

            template <typename T>
            T 
            key_get (std::string const& domain,
                     std::string const& key)
            {
              if(!domain_key_exist(domain,key))
              {
                    throw NoKeyException((boost::format ("MCS: key_get() Domain [%s] Key [%s] does not exist") % domain % key).str());
              }

              return T (m_domains.find(domain)->second.find(key)->second);
            }

            void
            key_push (std::string const& domain, std::string const& key)
            {
              if(!domain_key_exist(domain,key))
              {
                throw NoKeyException((boost::format ("MCS: key_push() Domain [%s] Key [%s] does not exist") % domain % key).str());
              }

              m_domains.find(domain)->second.find(key)->second.push();
            }
            
            void
            key_unset(
                std::string const& domain,
                std::string const& key
            );

            int
            subscribe(
                std::string const&      domain,      //Must be registered 
                std::string const&      key,         //Must be registered,
                SubscriberNotify const& notify
            );  

            void
            unsubscribe(
                int                id,          //Must be unique
                std::string const& domain,      //Must be registered 
                std::string const& key          //Must be registered,
            );

          private:

            typedef std::map<std::string /* Key Name */, Key>   KeysT;
            typedef std::map<std::string /* Domain name */, KeysT>  DomainsT;

            DomainsT        m_domains;
            std::string     m_xml;
            std::string     m_root_node_name;
            xmlDocPtr       m_doc;
            double          m_version;
            int             m_subscriber_id;
      };
};

#endif //MCS_BASE_H 
