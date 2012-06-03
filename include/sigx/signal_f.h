#ifndef _SIGX_SIGNAL_F_HPP_
#define _SIGX_SIGNAL_F_HPP_

/*
 * Copyright 2006 Klaus Triendl
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free 
 * Software Foundation, 51 Franklin Street, Fifth Floor, 
 * Boston, MA 02110-1301, USA.
*/

#include <sigc++/signal.h>
#include <sigxconfig.h>
#include <sigx/fwddecl.h>
#include <sigx/tunnel_functor.h>
#include <sigx/connection_handler.h>
#include <sigx/signal_source_base.h>
#include <sigx/signal_source_threadprivate.h>
#include <sigx/signal_source_obj_mem.h>
#include <sigx/signal_source_func.h>
#include <sigx/signal_wrapper.h>
#include <sigx/signal_f_base.h>


namespace sigx
{

/**	@short Functor returning a sigx::signal_wrapper as a threadsafe signal wrapper 
 *	
 *	A signal functor is used to expose a thread's signals and replaces a traditional access function.
 *	It acts as a middle tier between two threads, creating a 
 *	sigx::signal_wrapper to a signal existing in the server thread's context, 
 *	handing it over to the calling thread.
 *	
 *	A signal functor can be created from different signal sources:
 *	- a signal living in a thread private
 *	- a signal from an object's member (also late bound object instances)
 *	- a signal returned by an object's member function (also late bound object 
 *	  instances)
 *	- a signal returned by a functor
 *	
 *	Here are some examples for signal sources:
 *	From a thread private:
 *	Class MyThread has a signal "did something" in a thread private data.
 *	The class exposes this signal through a threadsafe signal wrapper.
 *	@code
 *	class MyThread: public sigx::glib_threadable
	{
	protected:
		typedef sigc::signal<void> signal_did_something_type;

	public:
		MyThread();

		// expose signal "did_something"
		sigx::signal_f<signal_did_something_type> signal_did_something;
	
	private:
		struct ThreadData
		{
			signal_did_something_type m_sigDidSomething;
		};
		Glib::Private<ThreadData> m_threadpriv;
	};
	
	MyThread::MyThread(): 
		sigx::glib_threadable(), 
		// params: dispatchable, thread private key, signal pointer
		signal_did_something(this, m_threadpriv, &ThreadData::m_sigDidSomething)
	{}
 *	@endcode
 *	
 *	From an object's member function, object instance is late bound:
 *	There is a window object that exposes a button's "clicked" signal 
 *	through a threadsafe signal wrapper.
 *	@code
	class TheGUI: public Gtk::Window, public sigx::glib_auto_dispatchable
	{
	public:
		TheGUI();
	
	
	public:
		// expose Gtk::Button::signal_clicked
		sigx::signal_f<Glib::SignalProxy0<void> > signal_button_clicked;
	
	
	private:	
		Gtk::Button* m_btn;
	};

	TheGUI::TheGUI(): 
		Gtk::Window(), 
		sigx::glib_auto_dispatchable(), 
		// set up signal functor with late object instance binding
		signal_button_clicked(this, sigc::ref(m_btn), &Gtk::Button::signal_clicked), 
		m_btn()
	{
		// now object instance m_btn is pointing to is created, 
		// signal_button_clicked now has a valid object instance
		m_btn = manage(new Gtk::Button("notify thread"));
		add(*m_btn);
		show_all_children();
	}
 *	@endcode
 *	
 *	@ingroup signals
 */
template<typename T_signal>
class signal_f: protected signal_f_base
{
public:
	typedef T_signal signal_type;
	typedef signal_f<signal_type> self_type;

	/**	@short Constructs a signal functor from a thread private object's 
	 *	member signal of type @e T_signal.
	 *	@param _A_disp The dispatchable to operate on.
	 *	@param _A_priv Key to the thread private data
	 *	@param _A_sig pointer to the thread private object's member signal
	 */
	template<typename T_threadpriv>
	signal_f(const shared_dispatchable& _A_disp, Glib::Private<T_threadpriv>& _A_priv, signal_type T_threadpriv::*_A_sig): 
		signal_f_base(_A_disp, 
			new signal_source_threadprivate<T_threadpriv, T_signal>(_A_priv, _A_sig))
	{}

	/**	@short Constructs a signal functor from a dispatchable's member signal 
	 *	of type @e T_signal.
	 *	@param _A_obj The dispatchable to operate on and the object instance
	 *	for @e _A_sig
	 *	@param _A_sig Pointer to the @e _A_obj's member signal
	 */
	template<typename T_dispatchable>
	signal_f(T_dispatchable& _A_obj, signal_type T_dispatchable::*_A_sig): 
		signal_f_base(_A_obj, 
			new signal_source_obj_mem<T_dispatchable, signal_type>(&_A_obj, _A_sig))
	{}

	/**	@short Constructs a signal functor from an object's member signal 
	 *	of type @e T_signal.
	 *	@param _A_disp The dispatchable to operate on.
	 *	@param _A_obj The object instance for @e _A_sig
	 *	@param _A_sig Pointer to the @e _A_obj's member signal
	 */
	template<typename T_obj>
	signal_f(const shared_dispatchable& _A_disp, T_obj& _A_obj, signal_type T_obj::*_A_sig): 
		signal_f_base(_A_disp, 
			new signal_source_obj_mem<T_obj, signal_type>(&_A_obj, _A_sig))
	{}

	/**	@short Constructs a signal functor from an object's member signal 
	 *	of type @e T_signal.
	 *	Object instance is late bound.
	 *	@param _A_disp The dispatchable to operate on.
	 *	@param _A_obj Pointer reference to the object for @e _A_sig
	 *	@param _A_sig Pointer to the @e _A_obj's member signal
	 */
	template<typename T_obj>
	signal_f(const shared_dispatchable& _A_disp, sigc::const_reference_wrapper<T_obj*> _A_obj, signal_type T_obj::*_A_sig): 
		signal_f_base(_A_disp, 
			new signal_source_obj_mem<const T_obj, signal_type>(sigc::unwrap(_A_obj), _A_sig))
	{}
	template<typename T_obj>
	signal_f(const shared_dispatchable& _A_disp, sigc::reference_wrapper<T_obj*> _A_obj, signal_type T_obj::*_A_sig): 
		signal_f_base(_A_disp, 
			new signal_source_obj_mem<T_obj, signal_type>(sigc::unwrap(_A_obj), _A_sig))
	{}

	/**	@short Constructs a signal functor from a member functor returning a signal
	 *	of type @e T_signal and a member functor's bound object.
	 *	Object instance is late bound.
	 *	@param _A_disp The dispatchable to operate on.
	 *	@param _A_obj pointer reference to the object for @e _A_sig_func
	 *	@param _A_sig_func Functor returning a signal of type @e T_signal;
	 *	must take a @e T_obj* as argument, e.g. create the functor with 
	 *	sigc::mem_fun(&MyObj::signal_did_something)
	 */
	template<typename T_obj, typename T_functor>
	signal_f(const shared_dispatchable& _A_disp, sigc::const_reference_wrapper<T_obj*> _A_obj, const T_functor& _A_sig_func): 
		signal_f_base(_A_disp, 
			new signal_source_pobj_mem_fun<const T_obj, T_functor, signal_type>(sigc::unwrap(_A_obj), _A_sig_func))
	{}
	template<typename T_obj, typename T_functor>
	signal_f(const shared_dispatchable& _A_disp, sigc::reference_wrapper<T_obj*> _A_obj, const T_functor& _A_sig_func): 
		signal_f_base(_A_disp, 
			new signal_source_pobj_mem_fun<T_obj, T_functor, signal_type>(sigc::unwrap(_A_obj), _A_sig_func))
	{}

	/**	@short Constructs a signal functor from any functor returning a signal
	 *	of type @e T_signal.
	 *	@param _A_disp The dispatchable to operate on.
	 *	@param _A_sig_func Functor returning a signal of type @e T_signal
	 */
	template<typename T_functor>
	signal_f(const shared_dispatchable& _A_disp, const T_functor& _A_sig_func): 
		signal_f_base(_A_disp, 
			new signal_source_func<T_functor, signal_type>(_A_sig_func))
	{}

	/**	@short Constructs a signal functor from a dispatchable functor (i.e. a 
	 *	functor on a dispatchable's method) returning a signal of type 
	 *	@e T_signal.
	 *	@param _A_sig_func Dispatchable functor returning a signal of type 
	 *	@e T_signal
	 */
	template<typename T_functor>
	explicit signal_f(const T_functor& _A_sig_func): 
		signal_f_base(internal::dispatchable_constraint<T_functor>::find_dispatchable(_A_sig_func), 
			new signal_source_func<T_functor, signal_type>(_A_sig_func))
	{}

	/**	@return A threadsafe representation of a signal of type @e T_signal.
	 *	@note Executed by any client thread.
	 */
	signal_wrapper<signal_type> operator ()() const
	{
		return signal_wrapper<signal_type>(m_disp, m_sigsource);
	}
};


} // namespace sigx


#endif // end file guard
