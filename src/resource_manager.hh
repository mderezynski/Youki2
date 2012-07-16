#ifndef RESOURCE_MANAGER_HPP
#define RESOURCE_MANAGER_HPP

#include "async_queue.hh"
#include <boost/ref.hpp>
#include <unordered_map>
#include <map>
#include <utility>
#include <atomic>
#include <thread>
#include <chrono>
#include <glibmm.h>

template <class ResourceT>
class ResourceRetriever
{
public:

    typedef ResourceT Resource;
    typedef typename Resource::Key ResourceKey;

    struct RetrieveData
    {
	bool		Acquire ;
	ResourceKey	Key ;

	RetrieveData(ResourceKey const& key, bool acquire = true)
	: Acquire(acquire)
	, Key(key)
	{}

	RetrieveData(RetrieveData const& other)
	: Acquire(other.Acquire)
	, Key(other.Key)
	{}
    } ;
    
    typedef sigc::signal<void, ResourceKey const&, Resource&> SignalCompleted;

    ResourceRetriever ()
        : m_thread  (&ResourceRetriever::process_func, this)
        , m_running (true)
    {}

    ResourceRetriever (ResourceRetriever const&) = delete;

    ResourceRetriever (ResourceRetriever&&) = delete;

    ~ResourceRetriever ()
    {
        m_running = false;
        m_thread.join ();
    }

    void request (ResourceKey const& key, bool acquire = true)
    {
        m_requests.push (RetrieveData(key,acquire));
    }

    SignalCompleted& signal_completed ()
    {
        return m_signal_completed;
    }

private:

    typedef AsyncQueue<RetrieveData> RequestQueue;
    typedef AsyncQueue<std::pair<ResourceKey, Resource>> ResultQueue;

    sigc::connection  m_connection_notify ;

    std::thread       m_thread;
    std::atomic<bool> m_running;
    RequestQueue      m_requests;
    ResultQueue       m_results;
    SignalCompleted   m_signal_completed;
    std::mutex	      m_process_lock ;

    void process_func ()
    {
        while (m_running) {

            if (!m_requests.empty ()) {
                do {
                    // Get request to process
                    auto rd = m_requests.front ();
                    m_requests.pop ();

		    if(!rd.Acquire)
		    {
			// Retrieve object
			auto object = Resource::retrieve_cached (rd.Key);

			// Queue retrieved object
			m_results.push (std::make_pair (rd.Key, object));
		    }
		    else
		    {
			// Retrieve object
			auto object = Resource::retrieve (rd.Key);

			// Queue retrieved object
			m_results.push (std::make_pair (rd.Key, object));
		    }
                }
                while (!m_requests.empty ());

		if(m_results.size())
		{
		    // Schedule update and notification in run in main thread
		    m_connection_notify.disconnect() ;
		    m_connection_notify = Glib::signal_timeout ().connect (sigc::mem_fun (this, &ResourceRetriever::notify), 333);
		}
            }

            std::this_thread::sleep_for (std::chrono::milliseconds (500));
        }
    }

    bool notify ()
    {
	guint counter = 0 ;

        while (counter < 5 && !m_results.empty ()) {
            // Get object in retrieval queue
            auto result = m_results.front ();
            m_results.pop ();

            // Perform in-place replacement of stand-in object
            auto& key = std::get<0> (result);
            auto& obj = std::get<1> (result);

            // Notify all listeners waiting on completed fetches
            m_signal_completed.emit (key, obj); 

	    ++counter ;
        }
	
	if(m_results.empty())
	{
	    m_connection_notify.disconnect() ;
	    return false ;
	}

	return true ; 
    }
};

template <class ResourceT>
class ResourceManager
{
public:

    typedef ResourceT Resource;
    typedef typename Resource::Key ResourceKey;
    typedef typename ResourceRetriever<Resource>::SignalCompleted SignalCompleted;

    ResourceManager ()
    {
        m_retriever.signal_completed ().connect (sigc::mem_fun (this, &ResourceManager::update_resource));
    }

    ResourceManager (ResourceManager const&) = delete;

    ResourceManager (ResourceManager&&) = delete;

    ~ResourceManager ()
    {}

    void queue (ResourceKey const& key)
    {
	m_retriever.request (key,false);
    }

    Resource& get (ResourceKey const& key, bool acquire = true )
    {
        // Look up object in table to see if it already exists in memory

	if( !acquire )
	{
	    auto match = m_resources.find (key);

	    if (match != std::end(m_resources) && bool((*match).second)) {
		// Required object already in memory, return it
		return (*match).second ; 
	    }

	    // create stand-in dummy object
	    m_resources[key] = Resource (key);
	}

	if(acquire)
	{
	    // request retrieval for actual object
	    m_retriever.request (key);
	}

	return m_resources[key] ;
    }

    SignalCompleted& signal_completed ()
    {
        return m_signal_completed ; 
    }

private:

    std::map<ResourceKey, Resource>	      m_resources;
    ResourceRetriever<Resource>               m_retriever;
    SignalCompleted			      m_signal_completed;

    void update_resource (ResourceKey const& key, Resource& resource)
    {
        // Replace resource in-place
        m_resources[key] = resource ;
	m_signal_completed.emit (key, resource);
    }
};

#endif // RESOURCE_MANAGER_HPP
