#ifndef _SIGX_CONNECTION_HPP_
#define _SIGX_CONNECTION_HPP_

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

#include <tr1/memory>	// std::tr1::shared_ptr
#include <sigxconfig.h>
#include <sigx/fwddecl.h>
#include <sigx/shared_dispatchable.h>


namespace sigx
{

/**	@short A threadsafe representation of a sigc::connection.
 *	@ingroup signals
 */
class SIGX_API connection_wrapper
{
private:
	// volatile because the value should be always read from the variable itself;
	// this is necessary because the sigc::connection* is "late bound", i.e.
	// it is set by the server thread after an asynchronous "connect" message.
	// When the next tunnel functor is created (let's say with disconnect()) 
	// then m_sigc_conn is dereferenced (*m_sigc_conn) and the actual pointer 
	// value (the sigc::connection*) is treated volatile at the other side of 
	// the tunnel (at the server thread's side).
	// It is const because the connection_wrapper object doesn't modify the pointer.
	typedef std::tr1::shared_ptr<const volatile sigc_connection_ptr> shared_sigc_conn_ptr;


public:
	/**	@short	Construct an empty connection_wrapper.
	 */
	connection_wrapper();
	/**	@short	Construct from a dispatchable and a late bound pointer to the server thread's 
	 *			sigc::connection.
	 */
	connection_wrapper(const shared_dispatchable& _A_disp, const shared_sigc_conn_ptr& _A_conn);
	/**	@short	Copy construct from another connection_wrapper
	 */
	connection_wrapper(const connection_wrapper& other) throw();

	/** 
	 *	@note destroying is asynchronous
	 */
	~connection_wrapper() throw();

	/**	@note nonvolatile, may only be executed by a single thread.
	 */
	connection_wrapper& operator =(const connection_wrapper& other);

public:
	/** 
	 *	@note synchronous
	 */
	bool empty() const;

	/** 
	 *	@note synchronous
	 */
	bool connected() const;

	/** 
	 *	@note synchronous
	 */
	bool blocked() const;

	/** 
	 *	@note synchronous
	 */
	bool block(bool should_block = true);

	/** 
	 *	@note synchronous
	 */
	bool unblock();

	/** 
	 *	@note synchronous
	 */
	void disconnect();

	/**	
	 *	@note non-const because sigc::connection::operator bool is non-const
	 *	@note synchronous
	 */
	operator bool();

protected:
	void destroy_self();


private:
	///	the sigc::connection living in the context of the server thread
	shared_sigc_conn_ptr m_sigc_conn;
	///	the dispatchable over which we are sending the messages to the 
	///	server thread's sigc::connection
	shared_dispatchable m_shared_disp;
	///	how many times the sigc::connection is shared by multiple 
	///	connection_wrapper objects
	volatile int* m_sigcconn_refcount;
};


} // namespace sigx


#endif // end file guard
