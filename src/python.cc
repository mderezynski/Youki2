#include "config.h"

#include <queue>
#include <string>
#include <set>
#include <vector>
#include <map>

#include <boost/cstdint.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/variant.hpp>
#include <boost/optional.hpp>

#include <Python.h>

#include <boost/python.hpp>
#include <boost/python/suite/indexing/indexing_suite.hpp>
#include <boost/python/suite/indexing/map_indexing_suite.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include <pygobject.h>
#include <pygtk/pygtk.h>
#include <pycairo/pycairo.h>
#include <gdkmm/pixbuf.h>
#include <libglademm/xml.h>

#include "pysigc.hh"
#include "gtkmmmodule.h"

#include "mpx/mpx-types.hh"
#include "mpx/mpx-covers.hh"
#ifdef HAVE_HAL
#include "mpx/mpx-hal.hh"
#endif // HAVE_HAL
#include "mpx/mpx-protected-access.hh"
#include "mpx/mpx-python.hh"

#include "mpx/util-graphics.hh"
#include "mpx/algorithm/random.hh"
#include "mpx/com/tagview.hh"

#include "library.hh"
#include "mpx/i-youki-play.hh"

#include "youki-controller.hh"

using namespace boost::python;

namespace
{
    bool py_initialized = false;
}

namespace
{
    struct PyGILLock
    {
        PyGILState_STATE m_state;

        PyGILLock ()
        {
            m_state = (PyGILState_STATE)(pyg_gil_state_ensure ());
        }

        ~PyGILLock ()
        {
            pyg_gil_state_release(m_state);
        }
    };
}

namespace MPX
{
    struct Plugin
    {
    };

    struct Bindable
    {
    };

    enum AttributeId
    {
          MPX_ATTRIBUTE_LOCATION
        , MPX_ATTRIBUTE_TITLE

        , MPX_ATTRIBUTE_GENRE
        , MPX_ATTRIBUTE_COMMENT
        , MPX_ATTRIBUTE_LABEL

        , MPX_ATTRIBUTE_MUSICIP_PUID

        , MPX_ATTRIBUTE_HASH     
        , MPX_ATTRIBUTE_MB_TRACK_ID 

        , MPX_ATTRIBUTE_ARTIST
        , MPX_ATTRIBUTE_ARTIST_SORTNAME
        , MPX_ATTRIBUTE_MB_ARTIST_ID

        , MPX_ATTRIBUTE_ALBUM
        , MPX_ATTRIBUTE_MB_ALBUM_ID
        , MPX_ATTRIBUTE_MB_RELEASE_DATE
        , MPX_ATTRIBUTE_MB_RELEASE_COUNTRY
        , MPX_ATTRIBUTE_MB_RELEASE_TYPE
        , MPX_ATTRIBUTE_ASIN

        , MPX_ATTRIBUTE_ALBUM_ARTIST
        , MPX_ATTRIBUTE_ALBUM_ARTIST_SORTNAME
        , MPX_ATTRIBUTE_MB_ALBUM_ARTIST_ID

        // MIME type
        , MPX_ATTRIBUTE_TYPE

        // HAL
        , MPX_ATTRIBUTE_HAL_VOLUME_UDI
        , MPX_ATTRIBUTE_HAL_DEVICE_UDI
        , MPX_ATTRIBUTE_VOLUME_RELATIVE_PATH
          
        // SQL
        , MPX_ATTRIBUTE_INSERT_PATH
        , MPX_ATTRIBUTE_LOCATION_NAME

        , MPX_ATTRIBUTE_TRACK 
        , MPX_ATTRIBUTE_TIME
        , MPX_ATTRIBUTE_RATING
        , MPX_ATTRIBUTE_DATE
        , MPX_ATTRIBUTE_MTIME
        , MPX_ATTRIBUTE_BITRATE
        , MPX_ATTRIBUTE_SAMPLERATE
        , MPX_ATTRIBUTE_COUNT
        , MPX_ATTRIBUTE_PLAYDATE
        , MPX_ATTRIBUTE_INSERT_DATE
        , MPX_ATTRIBUTE_IS_MB_ALBUM_ARTIST

        , MPX_ATTRIBUTE_ACTIVE
        , MPX_ATTRIBUTE_QUALITY

        , MPX_ATTRIBUTE_DEVICE_ID

        , MPX_ATTRIBUTE_MPX_TRACK_ID
        , MPX_ATTRIBUTE_MPX_ALBUM_ID
        , MPX_ATTRIBUTE_MPX_ARTIST_ID
        , MPX_ATTRIBUTE_MPX_ALBUM_ARTIST_ID
    };

    typedef std::queue<guint> PlayQueue_t ;
}

namespace mpxpy
{
    PyObject*
    ovariant_get (MPX::OVariant &self)
    {
        if(!self.is_initialized())
        {
            Py_INCREF(Py_None);
            return Py_None;
        }

        PyObject * obj = 0;

		switch(self.get().which())
		{
			case 0:
				obj = PyLong_FromLongLong((boost::get<guint>(self.get())));
                break;
			case 1:
				obj = PyFloat_FromDouble((boost::get<double>(self.get())));
                break;
			case 2:
				obj = PyString_FromString((boost::get<std::string>(self.get())).c_str());
                break;
		}
        return obj;
    }

    PyObject*
    variant_get (MPX::Variant &self)
    {
        PyObject * obj = 0;

		switch(self.which())
		{
			case 0:
				obj = PyLong_FromLongLong((boost::get<guint>(self)));
                break;
			case 1:
				obj = PyFloat_FromDouble((boost::get<double>(self)));
                break;
			case 2:
				obj = PyString_FromString((boost::get<std::string>(self)).c_str());
                break;
		}
        return obj;
    }

    PyObject*
    mcs_variant_get (Mcs::KeyVariant &self)
    {
        PyObject * obj = 0;

		switch(self.which())
		{
			case 0:
				obj = PyBool_FromLong(long ((boost::get<bool>(self))));
                break;
			case 1:
				obj = PyInt_FromLong(long ((boost::get<int>(self))));
                break;
			case 2:
				obj = PyFloat_FromDouble((boost::get<double>(self)));
                break;
			case 3:
				obj = PyString_FromString((boost::get<std::string>(self)).c_str());
                break;
		}
        return obj;
    }

}

namespace mpxpy
{
	guint
	variant_getint(MPX::Variant &self)
	{
		guint i = boost::get<guint>(self);
        return i;
	}

	void
	variant_setint(MPX::Variant &self, guint value)
	{
		self = value;
	}

	std::string	
	variant_getstring(MPX::Variant &self)
	{
		std::string s = boost::get<std::string>(self);
        return s;
	}

	void
	variant_setstring(MPX::Variant &self, std::string const& value)
	{
		self = value;
	}

	double
	variant_getdouble(MPX::Variant &self)
	{
		double d = boost::get<double>(self);
        return d;
	}

	void
	variant_setdouble(MPX::Variant &self, double const& value)
	{
		self = value;
	}
}

namespace mpxpy
{
	void
	ovariant_setint(MPX::OVariant & self, guint value)
	{
		self = value;
	}

	void
	ovariant_setstring(MPX::OVariant &self, std::string const& value)
	{
		self = value;
	}

	void
	ovariant_setdouble(MPX::OVariant &self, double const& value)
	{
		self = value;
	}
}

namespace mpxpy
{
	int
	mcs_variant_getint(Mcs::KeyVariant &self)
	{
		int i = boost::get<int>(self);
        return i;
	}

	void
	mcs_variant_setint(Mcs::KeyVariant &self, int value)
	{
		self = value;
	}


	std::string	
	mcs_variant_getstring(Mcs::KeyVariant &self)
	{
		std::string s = boost::get<std::string>(self);
        return s;
	}

	void
	mcs_variant_setstring(Mcs::KeyVariant &self, std::string const& value)
	{
		self = value;
	}

	double
	mcs_variant_getdouble(Mcs::KeyVariant &self)
	{
		double d = boost::get<double>(self);
        return d;
	}

	void
	mcs_variant_setdouble(Mcs::KeyVariant &self, double const& value)
	{
		self = value;
	}

    bool
	mcs_variant_getbool(Mcs::KeyVariant &self)
	{
		bool d = boost::get<bool>(self);
        return d;
	}

	void
	mcs_variant_setbool(Mcs::KeyVariant &self, bool const& value)
	{
		self = value;
	}

}

namespace mpxpy
{
	MPX::OVariant &
	track_getitem(MPX::Track &self, int id) 
	{
		return self[id];
	}

	int
	track_len(MPX::Track &self)
	{
		return MPX::N_ATTRIBUTES_INT;
	}

    bool	
	track_contains(MPX::Track &self, MPX::AttributeId id)
	{
		return self.has(id);
	}

    std::string
    track_repr (MPX::Track &self)
    {
        return "mpx.Track";
    }
}

namespace mpxpy
{
    using namespace MPX;

	MPX::Track
	library_sql_to_track(
          MPX::Library&         obj
        , SQL::Row&             row
        , bool                  all_metadata
        , bool                  no_location
    )
	{
        Track_sp p = obj.sqlToTrack( row, all_metadata, no_location ) ;
        Track t = *(p.get()) ;
        return t ;
	}

	MPX::Track
	library_get_track_by_id(
          MPX::Library&         obj
        , guint                id
    )
	{
        Track_sp p = obj.getTrackById( id ) ;
        Track t = *(p.get()) ;
        return t ;
	}

	MPX::Library&
	player_get_library(MPX::YoukiController & obj)
	{
		MPX::PAccess<MPX::Library> pa (*services->get<Library>("mpx-service-library").get());
		return pa.get();
	}

	MPX::Covers&
	player_get_covers (MPX::YoukiController & obj)
	{
		MPX::PAccess<MPX::Covers> pa (*services->get<Covers>("mpx-service-covers").get());
		return pa.get();
	}

	MPX::IPlay&
	player_get_play   (MPX::YoukiController & obj)
	{
		MPX::PAccess<MPX::IPlay> pa   (*services->get<IPlay>("mpx-service-play").get());
		return pa.get();
	}

#ifdef HAVE_HAL
	MPX::HAL&
	player_get_hal (MPX::YoukiController & obj)
	{
		MPX::PAccess<MPX::HAL> pa    (*services->get<HAL>("mpx-service-hal").get());
		return pa.get();
	}
#endif // HAVE_HAL

/*
	void
	player_add_widget (MPX::YoukiController & obj, PyObject * pyobj)
	{
		obj.add_widget(Glib::wrap(((GtkWidget*)(((PyGObject*)(pyobj))->obj)), false));
	}
*/

	void
	player_add_info_widget (MPX::YoukiController & obj, PyObject * pyobj, PyObject * name_py)
	{
        const char* name = PyString_AsString (name_py);

        g_return_if_fail(name != NULL);

		obj.add_info_widget(Glib::wrap(((GtkWidget*)(((PyGObject*)(pyobj))->obj)), false), name);
	}

/*
	void
	player_remove_widget (MPX::YoukiController & obj, PyObject * pyobj)
	{
		obj.remove_widget(Glib::wrap(((GtkWidget*)(((PyGObject*)(pyobj))->obj)), false));
	}
*/

	void
	player_remove_info_widget (MPX::YoukiController & obj, PyObject * pyobj)
	{
		obj.remove_info_widget(Glib::wrap(((GtkWidget*)(((PyGObject*)(pyobj))->obj)), false));
	}
}

namespace mpxpy
{
    Glib::RefPtr<Gdk::Pixbuf>
	covers_fetch (MPX::Covers & obj, std::string const& mbid)
	{
        Glib::RefPtr<Gdk::Pixbuf> cover (0);
        obj.fetch(mbid, cover);
        return cover; // if it stays 0, the converter takes care of it
	}
}

namespace mpxpy
{
	MPX::Variant&
	sql_row_getitem (MPX::SQL::Row & self, std::string const& key)
	{
		return self[key];
	}

	void
	sql_row_setitem (MPX::SQL::Row & self, std::string const& key, MPX::Variant const& value)
	{
		self[key] = value;
	}
	
	int
	sql_row_len (MPX::SQL::Row & self)
	{
		return self.size();
	}
}

namespace mpxpy
{
    template <typename T>
    T
    unwrap_boxed (PyObject * obj)
    {
        PyGBoxed * boxed = ((PyGBoxed*)(obj));
        return *(reinterpret_cast<T*>(boxed->boxed));
    }
}


namespace pysigc
{
    struct sigc0_to_pysigc 
    {
        static PyObject* 
        convert(sigc::signal<void> const& signal)
        {
            object obj ((pysigc::sigc0(signal)));
            Py_INCREF(obj.ptr());
            return obj.ptr();
        }
    };

    template <typename T1>
    struct sigc1_to_pysigc
    {
        static PyObject* 
        convert(sigc::signal<void, T1> const& signal)
        {
            object obj ((pysigc::sigc1<T1>(signal)));
            Py_INCREF(obj.ptr());
            return obj.ptr();
        }
    };

    template <typename T1, typename T2> 
    struct sigc2_to_pysigc
    {
        static PyObject* 
        convert(sigc::signal<void, T1, T2> const& signal)
        {
            object obj ((pysigc::sigc2<T1, T2>(signal)));
            Py_INCREF(obj.ptr());
            return obj.ptr();
        }
    };

    template <typename T1, typename T2, typename T3> 
    struct sigc3_to_pysigc
    {
        static PyObject* 
        convert(sigc::signal<void, T1, T2, T3> const& signal)
        {
            object obj ((pysigc::sigc3<T1, T2, T3>(signal)));
            Py_INCREF(obj.ptr());
            return obj.ptr();
        }
    };

    void wrap_signal0()
    {
        class_< pysigc::sigc0 >(typeid(pysigc::sigc0).name(), boost::python::no_init)
        .def("connect",     &pysigc::sigc0::connect)
        .def("disconnect",  &pysigc::sigc0::disconnect)
        ;

        to_python_converter<sigc::signal<void>, sigc0_to_pysigc
#if defined BOOST_PYTHON_SUPPORTS_PY_SIGNATURES
		, false
#endif
		>(); 
    }

    template <typename T1>
    void wrap_signal1()
    {
        typedef typename sigc::signal<void, T1>   signal_sigc;
        typedef typename pysigc::sigc1<T1>        signal_pysigc;

        class_< signal_pysigc >(typeid(signal_pysigc).name(), boost::python::no_init)
        .def("connect",     &signal_pysigc::connect)
        .def("disconnect",  &signal_pysigc::disconnect)
        ;

        to_python_converter<signal_sigc, sigc1_to_pysigc<T1>
#if defined BOOST_PYTHON_SUPPORTS_PY_SIGNATURES
		, false
#endif
		>(); 
    }

    template <typename T1, typename T2> 
    void wrap_signal2()
    {
        typedef typename sigc::signal<void, T1, T2>   signal_sigc;
        typedef typename pysigc::sigc2<T1, T2>        signal_pysigc;

        class_< signal_pysigc >(typeid(signal_pysigc).name(), boost::python::no_init)
        .def("connect",     &signal_pysigc::connect)
        .def("disconnect",  &signal_pysigc::disconnect)
        ;

        to_python_converter<signal_sigc, sigc2_to_pysigc<T1, T2>
#if defined BOOST_PYTHON_SUPPORTS_PY_SIGNATURES
		, false
#endif
		>(); 
    }

    template <typename T1, typename T2, typename T3> 
    void wrap_signal3()
    {
        typedef typename sigc::signal<void, T1, T2, T3>   signal_sigc;
        typedef typename pysigc::sigc3<T1, T2, T3>        signal_pysigc;

        class_< signal_pysigc >(typeid(signal_pysigc).name(), boost::python::no_init)
        .def("connect",     &signal_pysigc::connect)
        .def("disconnect",  &signal_pysigc::disconnect)
        ;

        to_python_converter<signal_sigc, sigc3_to_pysigc<T1, T2, T3>
#if defined BOOST_PYTHON_SUPPORTS_PY_SIGNATURES
		, false
#endif
		>(); 
    }
}

namespace mpxpy
{
    std::string
    get_config_dir ()
    {
        return Glib::build_filename(Glib::get_user_config_dir(), "audiosource");
    }
}

BOOST_PYTHON_MODULE(mpx)
{
    to_python_converter<Glib::RefPtr<Gdk::Pixbuf>, mpxpy::refptr_to_gobject<Gdk::Pixbuf> 
#if defined BOOST_PYTHON_SUPPORTS_PY_SIGNATURES
	    , true
#endif
		    >();

    to_python_converter<Glib::RefPtr<Gtk::ListStore>, mpxpy::refptr_to_gobject<Gtk::ListStore>
#if defined BOOST_PYTHON_SUPPORTS_PY_SIGNATURES
	    , true
#endif
		    >();

    to_python_converter<Glib::RefPtr<Gtk::TreeStore>, mpxpy::refptr_to_gobject<Gtk::TreeStore>
#if defined BOOST_PYTHON_SUPPORTS_PY_SIGNATURES
	    , true
#endif
		    >();

    to_python_converter<Glib::RefPtr<Gtk::ActionGroup>, mpxpy::refptr_to_gobject<Gtk::ActionGroup>
#if defined BOOST_PYTHON_SUPPORTS_PY_SIGNATURES
	    , true
#endif
		    >();


    to_python_converter<Glib::RefPtr<Gtk::UIManager>, mpxpy::refptr_to_gobject<Gtk::UIManager>
#if defined BOOST_PYTHON_SUPPORTS_PY_SIGNATURES
	    , true
#endif
    >();

    def("unwrap_boxed_mpxtrack", &mpxpy::unwrap_boxed<MPX::Track>, return_value_policy<return_by_value>());

    class_<MPX::Plugin>("Plugin")	
    ;

    class_<MPX::Bindable>("Bindable")
    ;

    def ("rand", &MPX::rand);
    def ("get_config_dir", &mpxpy::get_config_dir);

	/*-------------------------------------------------------------------------------------*/

    enum_<MPX::PlayStatus>("PlayStatus")
        .value("NONE", MPX::PLAYSTATUS_NONE)
        .value("STOPPED", MPX::PLAYSTATUS_STOPPED)
        .value("PLAYING", MPX::PLAYSTATUS_PLAYING)
        .value("PAUSED", MPX::PLAYSTATUS_PAUSED)
        .value("SEEKING", MPX::PLAYSTATUS_SEEKING)
        .value("WAITING", MPX::PLAYSTATUS_WAITING)
    ;

	/*-------------------------------------------------------------------------------------*/

/*
	enum_<MPX::Flags>("PlaybackSourceFlags")
		.value("F_NONE", MPX::F_NONE)	
		.value("F_ASYNC", MPX::F_ASYNC)	
		.value("F_HANDLE_STREAMINFO", MPX::F_HANDLE_STREAMINFO)	
		.value("F_PHONY_NEXT", MPX::F_PHONY_NEXT)	
		.value("F_PHONY_PREV", MPX::F_PHONY_PREV)	
		.value("F_HANDLE_LASTFM", MPX::F_HANDLE_LASTFM)	
		.value("F_HANDLE_LASTFM_ACTIONS", MPX::F_HANDLE_LASTFM_ACTIONS)	
		.value("F_USES_REPEAT", MPX::F_USES_REPEAT)	
		.value("F_USES_SHUFFLE", MPX::F_USES_SHUFFLE)	
	;

	enum_<MPX::Caps>("PlaybackSourceCaps")
		.value("C_NONE", MPX::C_NONE)	
		.value("C_CAN_GO_NEXT", MPX::C_CAN_GO_NEXT)	
		.value("C_CAN_GO_PREV", MPX::C_CAN_GO_PREV)	
		.value("C_CAN_PAUSE", MPX::C_CAN_PAUSE)	
		.value("C_CAN_PLAY", MPX::C_CAN_PLAY)	
		.value("C_CAN_SEEK", MPX::C_CAN_SEEK)	
		.value("C_CAN_RESTORE_CONTEXT", MPX::C_CAN_RESTORE_CONTEXT)	
		.value("C_CAN_PROVIDE_METADATA", MPX::C_CAN_PROVIDE_METADATA)	
		.value("C_CAN_BOOKMARK", MPX::C_CAN_BOOKMARK)	
		.value("C_PROVIDES_TIMING", MPX::C_PROVIDES_TIMING)	
	;

    class_<MPX::PlaybackSource, boost::noncopyable>("PlaybackSourceAPI", boost::python::no_init)
            .def("get_guid", &MPX::PlaybackSource::get_guid)
            .def("get_class_guid", &MPX::PlaybackSource::get_class_guid)
    ;
*/

	class_<MPX::IdV>("IdVector")
		.def(vector_indexing_suite<MPX::IdV>());
	;

 	/*-------------------------------------------------------------------------------------*/

	enum_<MPX::AttributeId>("AttributeId")
      .value("LOCATION", MPX::MPX_ATTRIBUTE_LOCATION)
      .value("TITLE", MPX::MPX_ATTRIBUTE_TITLE)
      .value("GENRE", MPX::MPX_ATTRIBUTE_GENRE)
      .value("COMMENT", MPX::MPX_ATTRIBUTE_COMMENT)
      .value("MUSICIP_PUID", MPX::MPX_ATTRIBUTE_MUSICIP_PUID)
      .value("HASH", MPX::MPX_ATTRIBUTE_HASH)     
      .value("MB_TRACK_ID", MPX::MPX_ATTRIBUTE_MB_TRACK_ID)
      .value("ARTIST", MPX::MPX_ATTRIBUTE_ARTIST)
      .value("ARTIST_SORTNAME", MPX::MPX_ATTRIBUTE_ARTIST_SORTNAME)
      .value("MB_ARTIST_ID", MPX::MPX_ATTRIBUTE_MB_ARTIST_ID)
      .value("ALBUM", MPX::MPX_ATTRIBUTE_ALBUM)
      .value("MB_ALBUM_ID", MPX::MPX_ATTRIBUTE_MB_ALBUM_ID)
      .value("MB_RELEASE_DATE", MPX::MPX_ATTRIBUTE_MB_RELEASE_DATE)
      .value("MB_RELEASE_COUNTRY", MPX::MPX_ATTRIBUTE_MB_RELEASE_COUNTRY)
      .value("MB_RELEASE_TYPE", MPX::MPX_ATTRIBUTE_MB_RELEASE_TYPE)
      .value("ASIN", MPX::MPX_ATTRIBUTE_ASIN)
      .value("ALBUM_ARTIST", MPX::MPX_ATTRIBUTE_ALBUM_ARTIST)
      .value("ALBUM_ARTIST_SORTNAME", MPX::MPX_ATTRIBUTE_ALBUM_ARTIST_SORTNAME)
      .value("MB_ALBUM_ARTIST_ID", MPX::MPX_ATTRIBUTE_MB_ALBUM_ARTIST_ID)
      .value("TYPE", MPX::MPX_ATTRIBUTE_TYPE)
      .value("HAL_VOLUME_UDI", MPX::MPX_ATTRIBUTE_HAL_VOLUME_UDI)
      .value("HAL_DEVICE_UDI", MPX::MPX_ATTRIBUTE_HAL_DEVICE_UDI)
      .value("VOLUME_RELATIVE_PATH", MPX::MPX_ATTRIBUTE_VOLUME_RELATIVE_PATH)
      .value("INSERT_PATH", MPX::MPX_ATTRIBUTE_INSERT_PATH)
      .value("LOCATION_NAME", MPX::MPX_ATTRIBUTE_LOCATION_NAME)
      .value("TRACK", MPX::MPX_ATTRIBUTE_TRACK)
      .value("TIME", MPX::MPX_ATTRIBUTE_TIME)
      .value("RATING", MPX::MPX_ATTRIBUTE_RATING)
      .value("DATE", MPX::MPX_ATTRIBUTE_DATE)
      .value("MTIME", MPX::MPX_ATTRIBUTE_MTIME)
      .value("BITRATE", MPX::MPX_ATTRIBUTE_BITRATE)
      .value("SAMPLERATE", MPX::MPX_ATTRIBUTE_SAMPLERATE)
      .value("COUNT", MPX::MPX_ATTRIBUTE_COUNT)
      .value("PLAYDATE", MPX::MPX_ATTRIBUTE_PLAYDATE)
      .value("INSERT_DATE", MPX::MPX_ATTRIBUTE_INSERT_DATE)
      .value("IS_MB_ALBUM_ARTIST", MPX::MPX_ATTRIBUTE_IS_MB_ALBUM_ARTIST)
      .value("ACTIVE", MPX::MPX_ATTRIBUTE_ACTIVE)
      .value("QUALITY", MPX::MPX_ATTRIBUTE_QUALITY)
      .value("DEVICE_ID", MPX::MPX_ATTRIBUTE_DEVICE_ID)
      .value("MPX_TRACK_ID", MPX::MPX_ATTRIBUTE_MPX_TRACK_ID)
      .value("MPX_ALBUM_ID", MPX::MPX_ATTRIBUTE_MPX_ALBUM_ID)
      .value("MPX_ARTIST_ID", MPX::MPX_ATTRIBUTE_MPX_ARTIST_ID)
      .value("MPX_ALBUM_ARTIST_ID", MPX::MPX_ATTRIBUTE_MPX_ALBUM_ARTIST_ID)
	;

	/*-------------------------------------------------------------------------------------*/

	class_<MPX::YoukiController, boost::noncopyable>("YoukiController", boost::python::no_init)


		.def("get_library",             &mpxpy::player_get_library,
      						return_internal_reference<>()) 
	
		.def("get_covers",              &mpxpy::player_get_covers,
               					return_internal_reference<>())

	        .def("get_play",                &mpxpy::player_get_play,
        	                                return_internal_reference<>())
#ifdef HAVE_HAL
		.def("get_hal",                 &mpxpy::player_get_hal,
                	                        return_internal_reference<>()) 
#endif // HAVE_HAL

		.def("gobj",                    &mpxpy::get_gobject<MPX::YoukiController>)

		.def("get_status",              &MPX::YoukiController::get_status) 

		.def("get_metadata",            &MPX::YoukiController::get_metadata,
                	                        return_internal_reference<>())

		.def("pause",                   &MPX::YoukiController::API_pause_toggle)
		.def("play_track",              &MPX::YoukiController::API_play_track)

	        .def("add_info_widget",         &mpxpy::player_add_info_widget)
        	.def("remove_info_widget",      &mpxpy::player_remove_info_widget)

	        .def("queue_next_track",        &MPX::YoukiController::queue_next_track)

        	.def("get_current_play_queue",  &MPX::YoukiController::get_current_play_queue)

/*
		.def("add_widget", 	        &mpxpy::player_add_widget)
		.def("remove_widget",   	&mpxpy::player_remove_widget)
*/
	;

	/*-------------------------------------------------------------------------------------*/

	class_<MPX::OVariant>("Optional")

        .def("__nonzero__", (bool (MPX::OVariant::*) ()) &MPX::OVariant::is_initialized,
                            return_value_policy<return_by_value>()) 

        .def("get",         &mpxpy::ovariant_get,
                            return_value_policy<return_by_value>())

		.def("set_int",     &mpxpy::ovariant_setint)
		.def("set_string",  &mpxpy::ovariant_setstring)
		.def("set_double",  &mpxpy::ovariant_setdouble)
	;

	/*-------------------------------------------------------------------------------------*/

	class_<MPX::Variant >("Variant")

        .def("get",         &mpxpy::variant_get,
                            return_value_policy<return_by_value>())

		.def("set_int",     &mpxpy::variant_setint)
		.def("set_string",  &mpxpy::variant_setstring)
		.def("set_double",  &mpxpy::variant_setdouble)

		.def("get_int",     &mpxpy::variant_getint,
                            return_value_policy<return_by_value>()) 

		.def("get_string",  &mpxpy::variant_getstring,
                            return_value_policy<return_by_value>()) 

		.def("get_double",  &mpxpy::variant_getdouble,
                            return_value_policy<return_by_value>()) 
	;

	/*-------------------------------------------------------------------------------------*/

	class_<MPX::Track >("Track")
		.def("__getitem__", &mpxpy::track_getitem,  return_internal_reference<>()) 
		.def("__len__",     &mpxpy::track_len,      return_value_policy<return_by_value>())
        .def("__contains__",&mpxpy::track_contains, return_value_policy<return_by_value>())
		.def("get",         &mpxpy::track_getitem,  return_value_policy<return_by_value>()) 
	;

	/*-------------------------------------------------------------------------------------*/

	class_<MPX::SQL::Row>("SQLRow")
		.def(map_indexing_suite<MPX::SQL::Row>())
	;

	/*-------------------------------------------------------------------------------------*/

	class_<MPX::SQL::RowV>("SQLRowV")
		.def(vector_indexing_suite<MPX::SQL::RowV>())
	;

	/*-------------------------------------------------------------------------------------*/

    pysigc::wrap_signal1<guint>();
    pysigc::wrap_signal3<MPX::Track,guint,guint>();

	class_<MPX::Library, boost::noncopyable>("Library", boost::python::no_init)

		.def("getSQL", &MPX::Library::getSQL)
		.def("execSQL", &MPX::Library::execSQL)

		.def("getMetadata", &MPX::Library::getMetadata)

        .def("sqlToTrack", &mpxpy::library_sql_to_track)
        .def("getTrackById", &mpxpy::library_get_track_by_id)

        .def("getTrackTags", &MPX::Library::getTrackTags)

        .def("trackRated", &MPX::Library::trackRated)
        .def("trackPlayed", &MPX::Library::trackPlayed) // can't see how plugins could possibly need this
        .def("trackTagged", &MPX::Library::trackTagged)

        .def("markovUpdate", &MPX::Library::markovUpdate)
        .def("markovGetRandomProbableTrack", &MPX::Library::markovGetRandomProbableTrack)

        .def("collectionCreate", &MPX::Library::collectionCreate) 
        .def("collectionAppend", &MPX::Library::collectionAppend) 

/*
        .def("signal_new_album",        &MPX::Library::signal_new_album, return_value_policy<return_by_value>())
        .def("signal_new_artist",       &MPX::Library::signal_new_artist, return_value_policy<return_by_value>())
        .def("signal_new_track",        &MPX::Library::signal_new_track, return_value_policy<return_by_value>())
        .def("signal_track_updated",    &MPX::Library::signal_track_updated, return_value_policy<return_by_value>())
*/
	;

    typedef std::vector<double> IEEEV;

	class_<IEEEV>("IEEEV")
		.def(vector_indexing_suite<IEEEV>());
	;

	/*-------------------------------------------------------------------------------------*/

	class_<Mcs::KeyVariant >("McsKeyVariant")

        .def("get",         &mpxpy::mcs_variant_get,
                            return_value_policy<return_by_value>())

		.def("set_int",     &mpxpy::mcs_variant_setint)
		.def("set_string",  &mpxpy::mcs_variant_setstring)
		.def("set_double",  &mpxpy::mcs_variant_setdouble)
		.def("set_bool",    &mpxpy::mcs_variant_setbool)

		.def("get_int",     &mpxpy::mcs_variant_getint,
                            return_value_policy<return_by_value>()) 

		.def("get_string",  &mpxpy::mcs_variant_getstring,
                            return_value_policy<return_by_value>()) 

		.def("get_double",  &mpxpy::mcs_variant_getdouble,
                            return_value_policy<return_by_value>()) 

		.def("get_bool"  ,  &mpxpy::mcs_variant_getbool,
                            return_value_policy<return_by_value>()) 

	;

	class_<Mcs::Config, boost::noncopyable>("MCS", boost::python::no_init)
		.def("domain_register", &Mcs::Config::domain_register)
		.def("key_register", &Mcs::Config::key_register)
		.def("domain_key_exist", &Mcs::Config::domain_key_exist)
		.def("key_set_bool", &Mcs::Config::key_set<bool>)
		.def("key_set_int", &Mcs::Config::key_set<int>)
		.def("key_set_double", &Mcs::Config::key_set<double>)
		.def("key_set_string", &Mcs::Config::key_set<std::string>)
		.def("key_get_bool", &Mcs::Config::key_get<bool>)
		.def("key_get_int", &Mcs::Config::key_get<int>)
		.def("key_get_double", &Mcs::Config::key_get<double>)
		.def("key_get_string", &Mcs::Config::key_get<std::string>)
	;	

	/*-------------------------------------------------------------------------------------*/

    class_<MPX::TagView, boost::noncopyable>("TagView")
        .def("get_active_tag", &MPX::TagView::get_active_tag, return_internal_reference<>())
        .def("clear", &MPX::TagView::clear)
        .def("add_tag", &MPX::TagView::add_tag)
        .def("get_widget", &mpxpy::get_gobject<MPX::TagView>)
        .def("display", &MPX::TagView::display)
		.def("gobj", &mpxpy::get_gobject<MPX::TagView>)
    ;

#ifdef HAVE_HAL
	/*-------------------------------------------------------------------------------------*/
    /* HAL */
	/*-------------------------------------------------------------------------------------*/

    class_<MPX::Volume>("HalVolume", boost::python::init<>())   
        .def_readwrite("volume_udi", &MPX::Volume::volume_udi)
        .def_readwrite("device_udi", &MPX::Volume::device_udi)
        .def_readwrite("label", &MPX::Volume::label)
        .def_readwrite("size", &MPX::Volume::size)
        .def_readwrite("mount_point", &MPX::Volume::mount_point)
        .def_readwrite("mount_time", &MPX::Volume::mount_time)
        .def_readwrite("device_file", &MPX::Volume::device_file)
        .def_readwrite("drive_serial", &MPX::Volume::drive_serial)
        .def_readwrite("drive_bus", &MPX::Volume::drive_bus)
        .def_readwrite("drive_type", &MPX::Volume::drive_type)
        .def_readwrite("drive_size", &MPX::Volume::drive_size)
        .def_readwrite("disc", &MPX::Volume::disc)
    ;

    class_<MPX::HAL>("MPXHal", boost::python::no_init)
    ;
#endif // HAVE_HAL

    /*-------------------------------------------------------------------------------------*/
    /* Covers */
    /*-------------------------------------------------------------------------------------*/

    class_<MPX::Covers, boost::noncopyable>("Covers", boost::python::no_init)
        .def("fetch", &mpxpy::covers_fetch)
    ;

    /*-------------------------------------------------------------------------------------*/
    /* Play */
    /*-------------------------------------------------------------------------------------*/

    class_<MPX::IPlay, boost::noncopyable>("IPlay", boost::python::no_init)
    ;
}

namespace
{
    gpointer
    boxed_copy(gpointer boxed)
    {
        Py_INCREF(static_cast<PyObject*>(boxed));
        return boxed;
    }

    void 
    boxed_free(gpointer boxed)
    {
        Py_DECREF(static_cast<PyObject*>(boxed));
    }
}

namespace MPX
{
    void
    mpx_py_init ()
    {
        if(!py_initialized)
        {
            try {
                PyImport_AppendInittab((char*)"gtkmm", initgtkmm);
                PyImport_AppendInittab((char*)"mpx", initmpx);
                Py_Initialize();
                init_pygobject();
                init_pygtk();
                //pyg_enable_threads ();
                py_initialized = true;
            } catch( error_already_set ) {
                g_warning("%s; Python Error:", G_STRFUNC);
                PyErr_Print();
            }
        }
        else
            g_warning("%s: mpx_py_init called, but is already initialized!", G_STRLOC);
    }
}
