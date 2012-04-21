#ifndef MPX_PY_SIGC_HH
#define MPX_PY_SIGC_HH

#include <glibmm.h>
#include <sigc++/sigc++.h>
#include <boost/python.hpp>
#include <Python.h>

namespace pysigc
{
using namespace boost::python;
class sigc0
{
    private:

        sigc::signal<void> & m_signal;
        PyObject * m_callable;
        Glib::Mutex m_CallableLock;

        void
        signal_conn ()
        {
            Glib::Mutex::Lock L (m_CallableLock);
            if(m_callable)
            {
                try{
                    boost::python::call<void>(m_callable);
                } catch (boost::python::error_already_set)
                {
                    PyErr_Print();
                    PyErr_Clear();
                }
            }
        }

    public:

        sigc0 (sigc0 const& other)
        : m_signal(other.m_signal)
        , m_callable(0)
        {
            if(other.m_callable)
            {
                Py_INCREF(other.m_callable);
                m_callable = other.m_callable;
            }
        }

        sigc0 (sigc::signal<void> const& signal)
        : m_signal(const_cast<sigc::signal<void>&>(signal))
        , m_callable(0)
        {
            m_signal.connect( sigc::mem_fun( *this, &sigc0::signal_conn ));
        }

        ~sigc0 ()
        {
        }

        void
        connect(PyObject * callable)
        {
            Glib::Mutex::Lock L (m_CallableLock);
            Py_INCREF(m_callable);
            m_callable = callable; 
        }

        void
        disconnect()
        {
            Glib::Mutex::Lock L (m_CallableLock);
            Py_DECREF(m_callable);
            m_callable = 0;
        }

        void
        emit()
        { 
            Glib::Mutex::Lock L (m_CallableLock);
            m_signal.emit();
        }
};


template <typename T1>
class sigc1
{
    private:

        sigc::signal<void, T1> & m_signal;
        PyObject * m_callable;
        Glib::Mutex m_CallableLock;

        void
        signal_conn (T1 t1)
        {
            Glib::Mutex::Lock L (m_CallableLock);
            if(m_callable)
            {
                try{
                    boost::python::object obj (handle<>(borrowed(m_callable)));
                    obj (t1);
                } catch (boost::python::error_already_set)
                {
                    PyErr_Print();
                    PyErr_Clear();
                }
            }
        }

    public:

        sigc1 (sigc1 const& other)
        : m_signal(other.m_signal)
        , m_callable(0)
        {
            if(other.m_callable)
            {
                Py_INCREF(other.m_callable);
                m_callable = other.m_callable;
            }
        }

        sigc1 (sigc::signal<void, T1> const& signal)
        : m_signal(const_cast<sigc::signal<void, T1>&>(signal))
        , m_callable(0)
        {
            m_signal.connect( sigc::mem_fun( *this, &sigc1::signal_conn ));
        }

        ~sigc1 ()
        {
        }

        void
        connect(PyObject * callable)
        {
            Glib::Mutex::Lock L (m_CallableLock);
            Py_INCREF(m_callable);
            m_callable = callable; 
        }

        void
        disconnect()
        {
            Glib::Mutex::Lock L (m_CallableLock);
            Py_DECREF(m_callable);
            m_callable = 0;
        }

        void
        emit(T1 & t1)
        { 
            Glib::Mutex::Lock L (m_CallableLock);
            m_signal.emit(t1);
        }
};


template <typename T1, typename T2>
class sigc2
{
    private:

        sigc::signal<void, T1, T2> & m_signal;
        PyObject * m_callable;
        Glib::Mutex m_CallableLock;

        void
        signal_conn (T1 t1, T2 t2)
        {
            Glib::Mutex::Lock L (m_CallableLock);
            if(m_callable)
            {
                try{
                    boost::python::object obj (handle<>(borrowed(m_callable)));
                    obj (t1, t2);
                } catch (boost::python::error_already_set)
                {
                    PyErr_Print();
                    PyErr_Clear();
                }
            }
        }

    public:

        sigc2 (sigc2 const& other)
        : m_signal(other.m_signal)
        , m_callable(0)
        {
            if(other.m_callable)
            {
                Py_INCREF(other.m_callable);
                m_callable = other.m_callable;
            }
        }

        sigc2 (sigc::signal<void, T1, T2> const& signal)
        : m_signal(const_cast<sigc::signal<void, T1, T2>&>(signal))
        , m_callable(0)
        {
            m_signal.connect( sigc::mem_fun( *this, &sigc2::signal_conn ));
        }

        ~sigc2 ()
        {
        }

        void
        connect(PyObject * callable)
        {
            Glib::Mutex::Lock L (m_CallableLock);
            Py_INCREF(m_callable);
            m_callable = callable; 
        }

        void
        disconnect()
        {
            Glib::Mutex::Lock L (m_CallableLock);
            Py_DECREF(m_callable);
            m_callable = 0;
        }

        void
        emit(T1 & t1, T2 & t2)
        { 
            Glib::Mutex::Lock L (m_CallableLock);
            m_signal.emit(t1, t2);
        }
};

template <typename T1, typename T2, typename T3>
class sigc3
{
    private:

        sigc::signal<void, T1, T2, T3> & m_signal;
        PyObject * m_callable;
        Glib::Mutex m_CallableLock;

        void
        signal_conn (T1 t1, T2 t2, T3 t3)
        {
            Glib::Mutex::Lock L (m_CallableLock);
            if(m_callable)
            {
                try{
                    boost::python::object obj (handle<>(borrowed(m_callable)));
                    obj (t1, t2, t3);
                } catch (boost::python::error_already_set)
                {
                    PyErr_Print();
                    PyErr_Clear();
                }
            }
        }

    public:

        sigc3 (sigc3 const& other)
        : m_signal(other.m_signal)
        , m_callable(0)
        {
            if(other.m_callable)
            {
                Py_INCREF(other.m_callable);
                m_callable = other.m_callable;
            }
        }

        sigc3 (sigc::signal<void, T1, T2, T3> const& signal)
        : m_signal(const_cast<sigc::signal<void, T1, T2, T3>&>(signal))
        , m_callable(0)
        {
            m_signal.connect( sigc::mem_fun( *this, &sigc3::signal_conn ));
        }

        ~sigc3 ()
        {
        }

        void
        connect(PyObject * callable)
        {
            Glib::Mutex::Lock L (m_CallableLock);
            Py_INCREF(m_callable);
            m_callable = callable; 
        }

        void
        disconnect()
        {
            Glib::Mutex::Lock L (m_CallableLock);
            Py_DECREF(m_callable);
            m_callable = 0;
        }

        void
        emit(T1 & t1, T2 & t2, T3 & t3)
        { 
            Glib::Mutex::Lock L (m_CallableLock);
            m_signal.emit(t1, t2, t3);
        }
};
}

#endif 
