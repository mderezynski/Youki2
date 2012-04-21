//  MPX
//  Copyright (C) 2010 MPX development.
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
//  --
//
//  The MPX project hereby grants permission for non-GPL compatible GStreamer
//  plugins to be used and distributed together with GStreamer and MPX. This
//  permission is above and beyond the permissions granted by the GPL license
//  MPX is covered by.

#include <config.h>
#include <glibmm.h>

#define NO_IMPORT
#include <iostream>
#include <pygtk/pygtk.h>
#include <pygobject.h>

#if PY_VERSION_HEX < 0x02050000
typedef int Py_ssize_t;
#endif

#include <boost/python.hpp>
#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/cstdint.hpp>
#include <Python.h>

#include "mpx/mpx-main.hh"
#include "youki-controller.hh"

#include "plugin-loader-python.hh"

using namespace Glib;
using namespace boost::python;

namespace MPX
{
    PluginLoaderPython::PluginLoaderPython(
    )
    {
        const std::string user_path =
                build_filename(
                                build_filename(
                                        g_get_user_data_dir(),
                                        "mpx"),
                                "python-plugins"
                              );

        if(file_test(user_path, FILE_TEST_EXISTS))
        {
                append_search_path (user_path);
        }

        append_search_path
                (build_filename(
                                PLUGIN_DIR,
                                "plugins-python"
                               ));
    }

    PluginLoaderPython::~PluginLoaderPython(
    )
    {
    }

	void
	PluginLoaderPython::append_search_path(
        const std::string& path
    )
	{	
		m_Paths.push_back( path ) ;
	}

	void
	PluginLoaderPython::load_plugins(
        guint& ID
    )
	{
        PyObject * main_module = PyImport_AddModule ("__main__");
        if(main_module == NULL)
        {
            g_message("Couldn't get __main__");
            return;	
        }

		PyObject * sys_path = PySys_GetObject ((char*)"path");

        {
            std::string pyapi_path = build_filename(DATA_DIR, "pyapi");
	        PyObject *path = PyString_FromString ((char*)pyapi_path.c_str());
            PyList_Insert (sys_path, 0, path); 
            Py_DECREF(path);
        }

		for(Strings::const_iterator i = m_Paths.begin(); i != m_Paths.end(); ++i)
		{
			PyObject *path = PyString_FromString ((char*)i->c_str());
			if (PySequence_Contains(sys_path, path) == 0)
				PyList_Insert (sys_path, 0, path);
			Py_DECREF(path);
		}

        PyObject * main_locals_orig = PyModule_GetDict(main_module);
        PyObject * mpx_orig = PyImport_ImportModule("mpx"); 
        PyObject * mpx_dict = PyModule_GetDict(mpx_orig);
        PyTypeObject * PyMPXPlugin_Type = (PyTypeObject *) PyDict_GetItemString(mpx_dict, "Plugin"); 

		for(Strings::const_iterator p = m_Paths.begin(); p != m_Paths.end(); ++p)
		{
			if(!file_test(*p, FILE_TEST_EXISTS))
				continue;

			Glib::Dir dir (*p);
			std::vector<std::string> strv (dir.begin(), dir.end());
			dir.close();

			for(std::vector<std::string>::const_iterator i = strv.begin(); i != strv.end(); ++i)
			{
                    PyGILState_STATE state = (PyGILState_STATE)(pyg_gil_state_ensure ());

                    PyObject * main_locals = PyDict_Copy(main_locals_orig);
					PyObject * module = PyImport_ImportModuleEx ((char*)i->c_str(), main_locals, main_locals, Py_None);
					if (!module) {
						g_message("%s: Couldn't load module '%s'", G_STRLOC, i->c_str());
						PyErr_Print ();
						continue;	
					}

					PyObject *locals = PyModule_GetDict (module);
					Py_ssize_t pos = 0;
					PyObject *key, *value;
					while (PyDict_Next (locals, &pos, &key, &value))
					{
						if(!PyType_Check(value))
							continue;

						if (PyObject_IsSubclass (value, (PyObject*) PyMPXPlugin_Type))
						{
                            try{
                                object instance = boost::python::call<object>(value, ID, boost::ref(*(services->get<YoukiController>("mpx-service-controller").get())), boost::ref(*mcs));
                                if(!instance)
                                {
                                    PyErr_Print();
                                    continue;
                                }

                                PluginHolderPython * ptr = new PluginHolderPython;

                                ptr->m_PluginInstance = instance.ptr();
                                Py_INCREF(instance.ptr());

                                if(PyObject_HasAttrString(instance.ptr(), "__doc__"))
                                {
                                    const char* doc = PyString_AsString (PyObject_GetAttrString (instance.ptr(), "__doc__")); 
                                    ptr->m_Description = doc ? doc : "(No Description given)";
                                    if(PyErr_Occurred())
                                    {
                                        g_message("%s: Got no __doc__ from plugin", G_STRLOC);
                                        PyErr_Clear();
                                    }
                                }
                                else
                                {
                                    ptr->m_Description = "(No Description given)";
                                }

                                ptr->m_Id = ID++;

                                Glib::KeyFile keyfile;
                                std::string key_filename = build_filename(*p, build_filename(*i, (*i)+".youki-plugin"));
                                if(file_test(key_filename, FILE_TEST_EXISTS))
                                {
                                    try{
                                        keyfile.load_from_file(build_filename(*p, build_filename(*i, (*i)+".youki-plugin")));
                                        ptr->m_Name = keyfile.get_string("YoukiPlugin", "Name"); 
                                        ptr->m_Authors = keyfile.get_string("YoukiPlugin", "Authors"); 
                                        ptr->m_Copyright = keyfile.get_string("YoukiPlugin", "Copyright"); 
                                        ptr->m_IAge = keyfile.get_integer("YoukiPlugin", "IAge");
                                        ptr->m_Website = keyfile.get_string("YoukiPlugin", "Website");
                                    } catch (Glib::KeyFileError) {
                                    }
                                }

                                if(ptr->m_Name.empty())
                                    ptr->m_Name = PyString_AsString (PyObject_GetAttrString (module, "__name__")); 

                                /* TODO: Query MCS for active state */

                                mcs->key_register("plugins", ptr->m_Name, false);

                                ptr->m_Active = false; 
                                ptr->m_Hidden = false; // FIXME: Make this an option from the plugin 
                                ptr->m_HasGUI = PyObject_HasAttrString(instance.ptr(), "get_gui");
                                if(PyErr_Occurred())
                                {
                                    g_message("%s: No get_gui in plugin", G_STRLOC);
                                    PyErr_Clear();
                                }

                                ptr->m_CanActivate = PyObject_HasAttrString(instance.ptr(), "activate");
                                if(PyErr_Occurred())
                                {
                                    g_message("%s: No activate in plugin", G_STRLOC);
                                    PyErr_Clear();
                                }


                                PluginHolderRefP_t refptr = PluginHolderRefP_t(ptr);

                                signal_plugin_loaded_.emit( refptr );

                                g_message("%s: >> Loaded: '%s'", G_STRLOC, ptr->m_Name.c_str());
                            } catch( error_already_set )
                            {
                                g_message("Plugin: %s", i->c_str());
                                PyErr_Print();
                            }
			break;
			}
			}
                		pyg_gil_state_release(state);
			}
		}
	}
}
