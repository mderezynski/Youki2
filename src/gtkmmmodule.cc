/* -*- Mode: C; c-basic-offset: 4 -*- */

/* include this first, before NO_IMPORT_PYGOBJECT is defined */
#define NO_IMPORT
#include <pygobject.h>
#include "gtkmmmodule.h"

extern "C" void pygtkmm_register_classes (PyObject *d);

extern "C" PyMethodDef pygtkmm_functions[];

void
initgtkmm(void)
{
    PyObject *m, *d;
	
    init_pygobject ();

    m = Py_InitModule ("gtkmm", pygtkmm_functions);
    d = PyModule_GetDict (m);
	
    pygtkmm_register_classes (d);

    g_message("%s: PyGtkmm initialized", G_STRLOC);
}
