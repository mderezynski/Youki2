// (c) 2007 M. Derezynski

#include <boost/algorithm/string.hpp>
#include "mpx/mpx-types.hh"

#include <glibmm.h>
#include <gmodule.h>

#include "mpx/util-file.hh"
#include "mpx/util-string.hh"
#include "mpx/metadatareader-taglib.hh"

#include <gst/gst.h>

using namespace Glib;
using boost::algorithm::is_any_of;
using boost::get;

namespace
{
    inline bool
    is_module(
          const std::string& modname
    )
    {
        return MPX::Util::str_has_suffix_nocase( modname, G_MODULE_SUFFIX ) ;
           
    } 

    struct PipelineUnref
    {
        GstElement * pipeline ;

        PipelineUnref(GstElement * e)
            : pipeline( e )
        {}

        ~PipelineUnref()
        {
            gst_element_set_state( pipeline, GST_STATE_NULL ) ;
            gst_object_unref( pipeline ) ;
        }
    } ;

    void
    have_type_handler(
          GstElement*    typefind
        , guint          probability
        , const GstCaps* caps
        , GstCaps**      caps_p
    )
    {
        if( caps_p )
        {
            *caps_p = gst_caps_copy( caps ) ;
        }
    }

    bool
    typefind(
          const std::string& uri
        , std::string&       found
    )
    {
        GstElement*             pipeline  = 0 ;
        GstElement*             source    = 0 ;
        GstElement*             typefind  = 0 ;
        GstElement*             fakesink  = 0 ;
        GstCaps*                caps      = 0 ;

        GstState                state ;
        GstStateChangeReturn    state_change ;

        if( uri.empty() )
        {
            return false ;
        }

        pipeline  = gst_pipeline_new( "pipeline" ) ;
        source    = gst_element_factory_make( "giosrc"   , NULL ) ;
        typefind  = gst_element_factory_make( "typefind" , NULL ) ; 
        fakesink  = gst_element_factory_make( "fakesink" , NULL ) ; 

        gst_bin_add_many(
              GST_BIN( pipeline )
            , source
            , typefind
            , fakesink
            , NULL
        ) ;

        gst_element_link_many(
              source
            , typefind
            , fakesink
            , NULL
        ) ;

        g_signal_connect(
              G_OBJECT( typefind )
            , "have-type"
            , G_CALLBACK( have_type_handler)
            , &caps
        ) ;

        g_object_set(
              source
            , "location"
            , uri.c_str()
            , NULL
        ) ;

        gst_element_set_state(
              GST_ELEMENT( pipeline )
            , GST_STATE_PAUSED
        ) ;

        state_change = gst_element_get_state(
                              GST_ELEMENT( pipeline )
                            , &state
                            , NULL
                            , 5 * GST_SECOND
        ) ;

        PipelineUnref R ( pipeline ) ;

        switch( state_change )
        {
            case GST_STATE_CHANGE_FAILURE:
            {
            }
            break;

            case GST_STATE_CHANGE_SUCCESS:
            {
                using boost::algorithm::is_any_of ;
                using boost::algorithm::split ;

                if( caps )
                {
                    Glib::ScopedPtr<char> caps_c_str (gst_caps_to_string( caps )) ;
                    gst_caps_unref( caps ) ;

                    std::vector<std::string> v ;
                    std::string splitstring = caps_c_str.get() ;
                    split( v, splitstring, is_any_of(",") ) ;

                    if( v.empty() )
                    {
                        found = caps_c_str.get() ;
                    }
                    else
                    {
                        found = v[0] ;
                    }

                    return true ;
                }
            }
            break ;

            default:
            {
            }
            break ;
        }

        return false;
    }
}

namespace MPX
{
    MetadataReaderTagLib::MetadataReaderTagLib ()
    : Service::Base("mpx-service-taglib")
    {
        std::string path = build_filename(PLUGIN_DIR, "taglib");
        if(file_test(path, FILE_TEST_EXISTS) && file_test(path, FILE_TEST_IS_DIR))
        {
            Util::dir_for_each_entry (path, sigc::mem_fun(*this, &MPX::MetadataReaderTagLib::load_taglib_plugin));  
        }
    }

    MetadataReaderTagLib::~MetadataReaderTagLib ()
    {}

    bool
    MetadataReaderTagLib::load_taglib_plugin (std::string const& path)
    {
        enum
        {
          LIB_BASENAME1,
          LIB_BASENAME2,
          LIB_PLUGNAME,
          LIB_SUFFIX
        };

        const std::string type = "taglib_plugin";

        std::string basename (path_get_basename (path));
        std::string pathname (path_get_dirname  (path));

        if (!is_module (basename))
          return false;

        StrV subs; 
        split (subs, basename, is_any_of ("_."));
        std::string name  = type + std::string("_") + subs[LIB_PLUGNAME];
        std::string mpath = Module::build_path (build_filename(PLUGIN_DIR, "taglib"), name);

        Module module (mpath, ModuleFlags (0)); 
        if (!module)
        {
          g_message("Taglib plugin load FAILURE '%s': %s", mpath.c_str (), module.get_last_error().c_str());
          return false;
        }

        void * _plugin_version;
        if (module.get_symbol ("_plugin_version", _plugin_version))
        {
          int version = *((int*)(_plugin_version));
          if (version != PLUGIN_VERSION)
          {
            g_message("Taglib plugin is of old version %d, not loading ('%s')", version, mpath.c_str ());
            return false;
          }
        }

        module.make_resident();
        g_message("Taglib plugin load SUCCESS '%s'", mpath.c_str ());

        void * _plugin_has_accessors = 0; //dummy
        if (module.get_symbol ("_plugin_has_accessors", _plugin_has_accessors))
        {
          TaglibPluginPtr plugin = TaglibPluginPtr (new TaglibPlugin());

          if (!g_module_symbol (module.gobj(), "_get", (gpointer*)(&plugin->get)))
            plugin->get = NULL;

          if (!g_module_symbol (module.gobj(), "_set", (gpointer*)(&plugin->set)))
            plugin->set = NULL;
          else
            g_message("     >> Plugin '%s' can write metadata", subs[LIB_PLUGNAME].c_str());

          if (g_module_symbol (module.gobj(), "_mimetypes", (gpointer*)(&plugin->mimetypes)))
          {
            const char ** mimetypes (plugin->mimetypes());
            while (*mimetypes)
            {
              g_message("     >> Plugin registers for %s", *mimetypes); 
              m_taglib_plugins.insert (std::make_pair (std::string (*mimetypes), plugin));

              ++mimetypes;
            }
          }
          else
          {
            m_taglib_plugins_keeper.push_back (plugin);
          }
        }
        return false;
    }

    bool
    MetadataReaderTagLib::get(
        const std::string& uri,
        Track&             track
    )
    {
        std::string type;

        try{
            if( typefind( uri, type ))
            {
                TaglibPluginsMap::const_iterator i = m_taglib_plugins.find( type );

                if (i != m_taglib_plugins.end() && i->second->get && i->second->get( uri, track ))
                {
                    track[ATTRIBUTE_LOCATION] = uri;

                    if( !track.has(ATTRIBUTE_DATE) && track.has(ATTRIBUTE_MB_RELEASE_DATE) )
                    {
                        std::string mb_date_str = boost::get<std::string>(track[ATTRIBUTE_MB_RELEASE_DATE].get());
                        int         mb_date_int = 0;

                        if( sscanf( mb_date_str.c_str(), "%04d", &mb_date_int ) == 1 )
                        {
                            track[ATTRIBUTE_DATE] = guint( mb_date_int ) ;
                        }
                    }

                    return true;
                }
            }
          }
        catch (Glib::ConvertError & cxe)
          {
          }
        return false; 
    }

    bool
    MetadataReaderTagLib::set (std::string const& uri, Track & track)
    {
		return false;
	}
}
