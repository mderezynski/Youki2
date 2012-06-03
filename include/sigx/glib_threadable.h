#ifndef _SIGX_GLIB_THREADABLE_HPP_
#define _SIGX_GLIB_THREADABLE_HPP_

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

#include <sigc++/functors/slot.h>
#include <glibmm/refptr.h>
#include <glibmm/thread.h>
#include <glibmm/main.h>
#include <sigxconfig.h>
#include <sigx/threadable.h>
#include <sigx/tunnel_functor.h>
#include <sigx/lockable.h>
#include <sigx/volatile_trait.h>
#include <sigx/signal_f.h>



namespace sigx
{

/**	@short A useful and convenient thread wrapper for Glib threads.
 *	
 *	A glib_threadable is used as a baseclass handling the major part of dealing with 
 *	Glib threads in a threadsafe manner.
 *	It encapsulates starting and ending the thread, setting and cleaning up a 
 *	Glib::MainContext and Glib::MainLoop for the thread and a sigx::dispatcher.
 *	
 *	Derived classes just need a request interface and a signal interface 
 *	other threads can connect to.
 *	The request interface consists of methods instructing the thread to do 
 *	something by
 *	tunneling a message with sigx::open_tunnel() to a thread private request 
 *	handler method that gets called when the message gets dispatched.
 *	
 *	The thread in turn emits a signal that it has successfully (or not) completed 
 *	the task. All thread's connected to that signal then know of the thread's
 *	attempt to execute the request.
 *	
 *	A glib_threadable uses a sigx::glib_dispatcher (that in turn uses a 
 *	Glib::Dispatcher) to dispatch requests in a threadsafe manner.
 *	
 *	See the following code example to get an idea how to derive from glib_threadable and its usage.
 *	
 *	@code
 *	class MyThread: public sigx::glib_threadable
 *	{
 *	protected:
 *		typedef sigc::signal<void, bool> signal_did_something_t;
 *	
 *	private:
 *		struct ThreadPrivate
 *		{
 *			signal_did_something_t m_sigDidSomething;
 *		};
 *	
 *	
 *	private:
 *		Glib::Private<ThreadPrivate> m_threadpriv;
 *	
 *	public:
 *		MyThread();
 *	
 *		// request interface
 *		sigx::request_f<> do_something;
 *		
 *		// signal interface;
 *		// return a sigx::signal_wrapper for threadsafe access to the signal did_something
 *		sigx::signal_f<signal_did_something_t> signal_did_something;
 *	
 *	protected:
 *		// virtuals from threadable
 *		virtual void on_startup();
 *		
 *		// dispatcher methods, get called when requests of the request interface
 *		// get dispatched
 *		void on_do_something();
 *	};
 *	
 *	
 *	MyThread::MyThread(): 
 *		m_threadpriv(), 
 *		// initialize request interface
 *		do_something(sigc::mem_fun(this, &MyThread::on_do_something)), 
 *		// initialize signal interface
 * 		signal_did_something(this, m_threadpriv, &ThreadPrivate::m_sigDidSomething)
 * 	{}
 *	
 *	void MyThread::on_startup()
 *	{
 *		m_threadpriv.set(new ThreadPrivate);
 *	}
 *	
 *	void MyThread::on_do_something()
 *	{
 *		// do something
 *		// ...
 *		
 *		// broadcast that I have done something
 *		const success = true;
 *		ThreadPrivate* privdata = m_threadpriv.get();
 *		privdata->m_sigDidSomething.emit(success);
 *	}
 *	@endcode
 *	
 *	When a glib_threadable is instantiated the thread does not immediately start its
 *	execution. You have to start it explicitly by calling run().
 *	
 *	@note The thread must have finished before the destructor is called.
 *	
 *	@ingroup Threading
 */
class SIGX_API glib_threadable: public threadable
{
public:
	/**	@short Constructs the threadable object.
	 *	@note The thread must be started explicitly by calling run().
	 */
	glib_threadable();

	/**	@short dtor.
	 *	@attention Thread must have finished before dtor is called.
	 */
	virtual ~glib_threadable();

	/**	@short Creates a joinable thread.
	 *	
	 *	Start the main loop of the thread.
	 *	The function returns as soon as the thread is created which does not
	 *	mean that it is already in a running state. 
	 *	To get notified that it is running you pass in a functor
	 *	(@e func_on_thread_ready) that gets called as soon as the thread is in 
	 *	a running state.
	 *	
	 *	@attention Multiple calls from multiple threads are not threadsafe; 
	 *	Call %run() only once from one thread, otherwise there are 
	 *	unpredictable results.
	 *	
	 *	@param func_on_thread_ready a functor that gets called as soon
	 *	as the thread is idle and ready. The passed in functor is tunneled
	 *	automatically if not yet a tunnel_functor.
	 *	Must be convertible to a sigc::slot<void>.
	 *	
	 *	@note In your on_thread_ready handler you can connect to the thread's
	 *	signals.
	 *	@code
	 *	MyThread mythread;
	 *	mythread.run(sigc::mem_fun(this, &TheGui::on_mythread_ready));
	 *	
	 *	void TheGUI::on_mythread_ready()
	 *	{
	 *		// now, the thread if fully set up, idle and ready
	 *		mythread.signal_did_something().connect(
	 *			sigc::mem_fun(this, &TheGUI::on_mythread_did_something)
	 *		);
	 *	}
	 *	@endcode
	 */
	template<typename T_functor>
	void run(const T_functor& func_on_thread_ready);

	/**	@short Creates a joinable thread.
	 *	
	 *	Start the main loop of the thread, this will block until the thread has 
	 *	been created and is in a running state.
	 *	
	 *	@attention Multiple calls from multiple threads are not threadsafe; 
	 *	Call %run() only once from one thread, otherwise there are 
	 *	unpredictable results.
	 *	
	 *	Afterwards you can connect to the thread's signals.
	 */
	void run();

	/**	@short Ends the thread, joins it and frees all its resources.
	 *	
	 *	Waits for the main loop to quit and joins the thread and in the process 
	 *	deleting all the thread private data associated with this thread and 
	 *	all the internal resources.
	 *	You MUST call %finish() before deleting a class derived from glib_threadable.
	 *	
	 *	Calling finish() from multiple threads is thread safe;
	 *	
	 *	@note Ends the thread's mainloop immediately (as soon as the message 
	 *	gets dispatched).
	 *	If your thread still must complete things before actually quitting the
	 *	mainloop then you have to create another request in your derived class
	 *	like "stop_working()" that signals the thread to stop its work.
	 *	"stop_working()" could then send back the answer that the thread has 
	 *	completed its work and is ready to get the "finish" signal.
	 */
	void finish();

	
private:
	/**	@short Common entry point for run() and run(const T_functor&).
	 */
	void create_thread(const sigc::slot<void>& slot_on_thread_ready);

	/**	@short The function the new thread executes.
	 *	
	 *	Creates and starts the thread's mainloop.
	 *	
	 *	Also sets up a dispatcher and sets the sigx::dispatchable baseclass'
	 *	dispatcher reference.
	 */
	void on_run(const sigc::slot<void>& slot_on_thread_ready);

	/**	@short Called when the thread is idle and ready
	 *	@pre run() was called.
	 */
	void on_idle_and_ready(Glib::Mutex& mtx, Glib::Cond& cond);
	
protected:
	/**	@short access the thread's maincontext
	 */
	Glib::RefPtr<Glib::MainContext> maincontext();

	/**	@short access the thread's mainloop
	 */
	Glib::RefPtr<Glib::MainLoop> mainloop();

	/**	@short Make a signal functor that returns the glib idle signal.
	 */
	signal_f<Glib::SignalIdle> make_idle_signal_f()
	{
		return signal_f<Glib::SignalIdle>(
			// the dispatchable
			*this, 
			// the signal source is a functor executing 
			// this->maincontext()->signal_idle()
			sigc::compose(
				// setter
				sigc::mem_fun(&Glib::MainContext::signal_idle), 
				// getter
				sigc::compose(
					// setter
					sigc::mem_fun(&Glib::RefPtr<Glib::MainContext>::operator ->), 
					// getter
					sigc::mem_fun(this, &glib_threadable::maincontext)
				)
			)
		);
	}

	/**	@short Make a signal functor that returns the glib timeout signal.
	 */
	signal_f<Glib::SignalTimeout> make_timeout_signal_f()
	{
		return signal_f<Glib::SignalTimeout>(
			// the dispatchable
			*this, 
			// the signal source is a functor executing 
			// this->maincontext()->signal_timeout()
			sigc::compose(
				// setter
				sigc::mem_fun(&Glib::MainContext::signal_timeout), 
				// getter
				sigc::compose(
					// setter
					sigc::mem_fun(&Glib::RefPtr<Glib::MainContext>::operator ->), 
					// getter
					sigc::mem_fun(this, &glib_threadable::maincontext)
				)
			)
		);
	}

	/**	@short Make a signal functor that returns the glib IO signal.
	 */
	signal_f<Glib::SignalIO> make_io_signal_f()
	{
		return signal_f<Glib::SignalIO>(
			// the dispatchable
			*this, 
			// the signal source is a functor executing 
			// this->maincontext()->signal_io()
			sigc::compose(
				// setter
				sigc::mem_fun(&Glib::MainContext::signal_io), 
				// getter
				sigc::compose(
					// setter
					sigc::mem_fun(&Glib::RefPtr<Glib::MainContext>::operator ->), 
					// getter
					sigc::mem_fun(this, &glib_threadable::maincontext)
				)
			)
		);
	}

	/**	@short Make a signal functor that returns the glib childwatch signal.
	 */
	signal_f<Glib::SignalChildWatch> make_childwatch_signal_f()
	{
		return signal_f<Glib::SignalChildWatch>(
			// the dispatchable
			*this, 
			// the signal source is a functor executing 
			// this->maincontext()->signal_childwatch()
			sigc::compose(
				// setter
				sigc::mem_fun(&Glib::MainContext::signal_child_watch), 
				// getter
				sigc::compose(
					// setter
					sigc::mem_fun(&Glib::RefPtr<Glib::MainContext>::operator ->), 
					// getter
					sigc::mem_fun(this, &glib_threadable::maincontext)
				)
			)
		);
	}


private:
	struct threaddata;
	Glib::Private<threaddata> m_threaddata;
	typedef mutex_lockable<Glib::Thread*> mutex_lockable_thread;
	mutex_lockable_thread m_thread;
};


/**	@example ipresolver/main.cpp
 *	The IPResolver example shows a way how to delegate IP to hostname resolving
 *	to a thread.
 *	
 *	Starting with the IPResolverThread
 *	@include ipresolver/resolver.h
 *	
 *	<br>.. its thread private data
 *	@include ipresolver/resolver_p.h
 *	
 *	<br>.. and its implementation
 *	@include ipresolver/resolver.cpp
 *	
 *	<br>The user interface
 *	@include ipresolver/thegui.h
 *	
 *	<br>.. the user interface implementation
 *	@include ipresolver/thegui.cpp
 *	
 *	<br>and finally the main entry point
 */


} // namespace sigx




#include <sigx/auto_tunneler.h>


namespace sigx
{

template<typename T_functor>
void glib_threadable::run(const T_functor& func_on_thread_ready)
{
	typedef internal::auto_tunneler<T_functor> auto_tunneler_t;

	// passed in functor must not be a slot or adapt a slot;
	// we have to apply this restriction because slots might have bound
	// trackables that can cause non-threadsafe access to the passed in slot
	// which will live in the context of the server thread
	static_assert((sigx::internal::is_or_adapts_slot<T_functor>::value == false),"");
	
	// toplevel functor must be a tunnel functor
	static_assert((sigc::is_base_and_derived<tunnel_base, typename auto_tunneler_t::functor_type>::value == true),"");

	const typename auto_tunneler_t::functor_type& functor2callback = 
		auto_tunneler_t::auto_open_tunnel(func_on_thread_ready);

	// a sigc::slot is created out of the functor func_on_thread_ready and bound
	// to another functor suitable for Glib::Thread::create.
	// this still happens in the context of the calling thread and is therefore
	// threadsafe
	create_thread(sigc::slot<void>(functor2callback));
}


} // namespace sigx


#endif // end file guard
