#ifndef ASYNC_QUEUE_HPP
#define ASYNC_QUEUE_HPP

#include <queue>
#include <deque>
#include <mutex>

template <class T, class Container = std::deque<T>>
class AsyncQueue
{
    typedef std::lock_guard<std::mutex> LockGuard;

public:

    typedef Container                           container_type;
    typedef typename Container::value_type      value_type;
    typedef typename Container::size_type       size_type;
    typedef typename Container::reference       reference;
    typedef typename Container::const_reference const_reference;

    AsyncQueue ()
    {}

    ~AsyncQueue ()
    {}

    AsyncQueue (AsyncQueue const&) = delete;

    bool empty () const
    {
	LockGuard lock (m_mutex) ;
        return m_queue.empty ();
    }

    size_type size () const
    {
	LockGuard lock (m_mutex) ;
        return m_queue.size ();
    }

    void push (T const& value)
    {
	LockGuard lock (m_mutex) ;
        m_queue.push (value);
    }

    void push (T&& value)
    {
	LockGuard lock (m_mutex) ;
        m_queue.push (value);
    }

    void pop ()
    {
	LockGuard lock (m_mutex) ;
        m_queue.pop ();
    }

    const_reference front () const
    {
	LockGuard lock (m_mutex) ;
        return m_queue.front ();
    }

    reference front ()
    {
	LockGuard lock (m_mutex) ;
        return m_queue.front ();
    }

private:

    std::queue<T, Container> m_queue;
    mutable std::mutex       m_mutex;
};

#endif // ASYNC_QUEUE_HPP
