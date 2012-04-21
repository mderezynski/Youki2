#ifndef MPX_PY_HH
#define MPX_PY_HH

#define NO_IMPORT
#include <Python.h>
#include <pygobject.h>
#include <glib-object.h>
#include "mpx/mpx-types.hh"

namespace MPX
{
    void mpx_py_init ();
}

namespace mpxpy
{
    template <typename T>
	PyObject*
	get_gobject (T & obj)
	{
		return pygobject_new((GObject*)(obj.gobj()));
	}

    template <typename T> 
    struct refptr_to_gobject
    {
        static
        PyObject*
        convert(Glib::RefPtr<T> const& p)
        {
            if(!p)
            {
                Py_INCREF(Py_None);
                return Py_None;
            }

            PyObject * p_py = pygobject_new((GObject*)(p->gobj()));
            return p_py;
        }

        static
        PyTypeObject const*
        get_pytype()
        {
            return pygobject_lookup_class(T::get_type());
        }
    };
}

#endif
