/*
 * Copyright 2008 Klaus Triendl
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

#include <glib.h>	// message macros
#include "sigx/glib_dispatcher.h"
#include "sigx/tunnel_functor.h"
#include "sigx/glib_threadable.h"
#include "__sigx_pchfence__.h"
#include "sigx/private/glib_threadable_p.h"
#include "sigx/lock_acquirer.h"


namespace sigx
{

glib_threadable::glib_threadable(): 
	threadable(), 
	m_threaddata(), 
	m_thread()
{}

// virtual 
glib_threadable::~glib_threadable()
{
	// 2006-07-02, klaus triendl:
	// we could finish() ourselves (in the context of the calling thread)
	// but the problem is that finish() quits the mainloop and right after 
	// on_cleanup() (a virtual method) is called;
	// By that time the virtual function pointers are probably not set anymore
	// (?) and the derived object has been already destructed.
	// 
	// For now, just assert that the thread is not running anymore
	g_return_if_fail(!m_thread.access_nonvolatile());
}

Glib::RefPtr<Glib::MainContext> glib_threadable::maincontext()
{
	return m_threaddata.get()->m_pContext;
}

Glib::RefPtr<Glib::MainLoop> glib_threadable::mainloop()
{
	return m_threaddata.get()->m_pLoop;
}

void glib_threadable::run()
{
	Glib::Mutex mtx;
	// condition to wait until the thread is in a running state
	Glib::Cond cond;
	
	const Glib::Mutex::Lock lock(mtx);
	create_thread(sigc::slot<void>(
		sigc::bind(
			sigc::mem_fun(this, &glib_threadable::on_idle_and_ready), 
			sigc::ref(mtx), 
			sigc::ref(cond)
		)
	));
	// now, wait for the thread to be idle and ready
	cond.wait(mtx);
}

void glib_threadable::create_thread(const sigc::slot<void>& slot_on_thread_ready)
{
	const sigc::slot<void> run_func(
		sigc::bind(
			sigc::mem_fun(this, &glib_threadable::on_run), 
			// bind makes a copy of the slot;
			// threadsafe as the copy is created and destroyed in the context
			// of the calling thread
			slot_on_thread_ready
		)
	);

	m_thread.access_nonvolatile() = Glib::Thread::create(run_func, true);
}

void glib_threadable::on_idle_and_ready(Glib::Mutex& mtx, Glib::Cond& cond)
{
	const Glib::Mutex::Lock lock(mtx);
	// now, the thread is idle and ready, signal the calling thread that invoked
	// thread.run() before
	cond.signal();
}


void glib_threadable::finish()
{
	writelock_acquirer<mutex_lockable_thread> locker(m_thread);
	Glib::Thread*& refthread = access_acquiree(locker);

	// end thread if not yet finished
	if (refthread)
	{
		// this creates an asynchronous tunnel to 
		// this->mainloop()->quit()
		open_tunnel_with(
			sigc::compose(
				sigc::mem_fun(&Glib::MainLoop::quit), 
				// getter for Glib::MainLoop
				sigc::compose(
					&Glib::RefPtr<Glib::MainLoop>::operator ->, 
					// getter for Glib::RefPtr<Glib::MainLoop>
					sigc::mem_fun(this, &glib_threadable::mainloop)
				)
			), 
			*this
		)();

		// a joinable thread object lives as long as join() hasn't been called;		
		// join() frees all resources and the object itself
		refthread->join();
		refthread = 0;
	}
}

void glib_threadable::on_run(const sigc::slot<void>& slot_on_thread_ready)
{
	// pdata will be freed when the thread ends according to glib docs;
	// see the online docs: glib-Threads.html#g-private-new
	threaddata* const pdata = new threaddata;
	m_threaddata.set(pdata);

	pdata->m_pContext = Glib::MainContext::create();
	pdata->m_pLoop = Glib::MainLoop::create(pdata->m_pContext);
	
	// scope for glib_disp (not necessary but easy to understand)
	{
		glib_dispatcher glib_disp(pdata->m_pContext);
	
		// register the dispatcher in the manual_dispatchable
		set_dispatcher(&glib_disp);
	
		// one-shot idle handler calling slot_on_thread_ready
		pdata->m_pContext->signal_idle().connect(
			sigc::bind_return(slot_on_thread_ready, false));
		
		// give derived classes the possibility to initialize their own stuff
		on_startup();
		pdata->m_pLoop->run();
		// give derived classes the possibility to cleanup their own stuff
		on_cleanup();

		// unregister dispatcher
		set_dispatcher(0);
	}
}


} // namespace sigx
