// (c) 2007 M. Derezynski

#ifndef MPX_METADATA_READER_TAG_LIB_HH
#define MPX_METADATA_READER_TAG_LIB_HH

#include <cstring>
#include <string>
#include <vector>
#include <map>

#include <boost/shared_ptr.hpp>
#include "mpx/i-metadatareader.hh"
#include "mpx/mpx-types.hh"
#include "mpx/mpx-services.hh"


namespace MPX
{
    class MetadataReaderTagLib
        : public MetadataReader
        , public Service::Base
    {
        struct TaglibPlugin
        {
          bool          (*set)       (std::string const& filename, Track &);
          bool          (*get)       (std::string const& filename, Track &);
          const char ** (*mimetypes) ();
        };

        typedef boost::shared_ptr<TaglibPlugin>        TaglibPluginPtr;
        typedef std::vector<TaglibPluginPtr>           TaglibPluginsKeeper;
        typedef std::map<std::string, TaglibPluginPtr> TaglibPluginsMap;

        TaglibPluginsMap    m_taglib_plugins;
        TaglibPluginsKeeper m_taglib_plugins_keeper;

        public:

          MetadataReaderTagLib();
          virtual ~MetadataReaderTagLib ();

          virtual bool
          get (std::string const& uri, Track & track);

          virtual bool
          set (std::string const& uri, Track & track);

        private:

          bool
          load_taglib_plugin (std::string const& path);
    };
}

#endif // MPX_METADATA_READER_TAG_LIB_HH
