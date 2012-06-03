#ifndef _SIGX_DISPATCHER_HPP
#define _SIGX_DISPATCHER_HPP

/*
 * Copyright 2005 Tim Mayberry and Klaus Triendl
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free 
 * Software Foundation, 51 Franklin Street, Fifth Floor, 
 * Boston, MA 02110-1301, USA.
 */

/**	@defgroup Dispatching Dispatching
 *	@short A group of types involved in dispatching messages between threads.
 */

#include <queue>
#include <map>
#include <glib/gtypes.h> // gint
#include <sigxconfig.h>
#include <sigx/fwddecl.h>
#include <sigx/bad_caller.h>
#include <sigx/bad_sync_call.h>
#include <sigx/operator_new.h>
#include <sigx/glib_lockables.h>


namespace sigx
{

typedef const void* threadhandle_type;


	namespace dld
	{

/**	@short A pair of threads where pair::first is the smaller one and pair::second
 *	the greater one.
 */
typedef std::pair<const threadhandle_type /*threadA*/, const threadhandle_type /*threadB*/> thread_pair_type;

/**	@short Creates a pair of thread handles where the first pair::first is the
 *	smaller one of both and pair::second is the greater one, compared with
 *	operator <.
 */
thread_pair_type make_thread_pair(threadhandle_type threadA, threadhandle_type threadB);


/**	@short Holds a counter of synchronous messages between two threads.
 */
class syncmessages_counter
{
public:
	/**	@short Construct a syncmessages_counter object.
	 *	@param threadA handle to thread A as a reference point to find out 
	 *	which thread is calling syncmessages_counter's methods.
	 */
	syncmessages_counter(const threadhandle_type& threadA);


public:
	/**	@short Increase the count of synchronous messages to the server thread.
	 *	@note Always called by the client thread.
	 */
	syncmessages_counter& operator ++();

	/**	@short Decrease the count of synchronous messages to the server thread.
	 *	@note Always called by the server thread.
	 */
	syncmessages_counter& operator --();
	
	/**	@short Test whether the client thread has some synchronous messages 
	 *	from the server thread pending.
	 *	@note Always called by the client thread.
	 */
	operator bool() const;


private:
	const threadhandle_type m_threadA;
	int m_countThreadA;
	int m_countThreadB;
};

struct thread_compare: public std::binary_function<thread_pair_type, thread_pair_type, bool>
{
	bool operator ()(const thread_pair_type& threadpair1, const thread_pair_type& threadpair2) const
	{
		if (threadpair1.first < threadpair2.first)
			return true;
		if (threadpair1.first > threadpair2.first)
			return false;
		if (threadpair1.second < threadpair2.second)
			return true;
		//if (threadpair1.second > threadpair2.second)
		//	return false;
		return false;
	}
};


typedef std::map<thread_pair_type, syncmessages_counter, thread_compare> sync_messages_type;
typedef static_mutex_lockable<sync_messages_type> lockable_sync_messages_type;


	} // namespace dld


// fwd decl
class tunnel_context_base;


/**	@short base class denoting the ability to dispatch messages between threads.
 *	
 *	A dispatcher holds a list of pointers to sigx::tunnel_context objects.
 *	
 *	@note abstract, use one of the %dispatcher implementations.
 *	@ingroup Dispatching
 *	
 *	@date 2006-08-12, kj	moved m_exiting into StandardDispatcher because it
 *							is only needed there - a glib_dispatcher must be
 *							created and destroyed by the same thread
 *	@date 2006-08-27, kj	derive from operator_new to ensure heap allocation
 *							in the glibmmx module
 *	@date 2006-09-02, kj	added deadlock detection: throw an exception if a 
 *							thread sends a synchronous message to itself or to 
 *							a thread that in turn has a synchronous message 
 *							pending to the sending thread
 */
class SIGX_API dispatcher: public operator_new
{
public:
	/**	@short Whether deadlock detection is turned on.
	 *	
	 *	Set to `true" from the main thread (e.g. after Glib::thread_init()) to 
	 *	turn on the deadlock detection feature at runtime for synchronous
	 *	messages.
	 *	Defaults to false.
	 *	
	 *	@note Set this flag only once and avoid setting it at any other place
	 *	than at program start up.
	 *	@note Deadlock detection comes with the price of performance there 
	 *	happens additional synchronization between threads. Use it mainly in
	 *	the debug mode of your program.
	 */
	static bool deadlock_detection;


	/**	@short constructs the dispatcher
	 */
	dispatcher();

	/**	
	 *	@throw bad_caller
	 */
	virtual ~dispatcher() = 0;
	
	/**	@short puts the tunnel context into the list of messages to dispatch
	 */
	virtual void send(tunnel_context_base* context);

	/**	@return the count of tunnel contexts in the queue
	 */
	gint queued_contexts() const;

	threadhandle_type creator_thread() const { return m_creator_thread; }

protected:
	/**	@short processes the next message in the queue.
	 *	@note only called from dispatcher thread.
	 */
	bool process_next();

	/**	
	 *	@throw bad_caller
	 */
	void test_calling_thread();


private:
	typedef std::queue<tunnel_context_base*> context_container_type;
	typedef mutex_lockable<context_container_type> lockable_tunnel_contexts;
	lockable_tunnel_contexts m_tunnel_contexts;
	
	/**	@short current number of messages in the queue
	 *	@note we could query m_tunnel_context_list for its size but we want to be sure
	 *	that querying the number of queued tunnel contexts is as lockfree as
	 *	possible. To increment and decrement the counter glib atomic
	 *	operations are used.
	 */
	volatile gint m_contexts_count;
	const threadhandle_type m_creator_thread;


// deadlock detection
private:
	/**	@short Increases the synchronous message count for the server thread and
	 *	throws an exception if there are messages pending from the server thread
	 *	to the client (sending) thread or if the client thread sends a 
	 *	synchronous message to itself.
	 *	@throw bad_sync_call
	 *	@note Call always from the client thread, i.e. execute this method in 
	 *	the context of the thread sending the message.
	 *	
	 *	@throw bad_sync_call
	 */
	static void increase_sync_messages(const dld::thread_pair_type& threadpair);

	/**	@short Decreases the synchronous message count for the server thread.
	 *	@throw bad_sync_call
	 *	@note Call always from the server thread, i.e. execute this method
	 *	in the context of the thread that has received the synchronous message.
	 */
	static void decrease_sync_messages(const dld::thread_pair_type& threadpair);

	static dld::lockable_sync_messages_type thread_paired_sync_messages;
};


} // namespace sigx


#endif // end file guard
