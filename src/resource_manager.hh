#ifndef RESOURCE_MANAGER_HPP
#define RESOURCE_MANAGER_HPP

#include "async_queue.hh"
#include <boost/ref.hpp>
#include <unordered_map>
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

    void request (ResourceKey const& key)
    {
        m_requests.push (key);
    }

    SignalCompleted& signal_completed ()
    {
        return m_signal_completed;
    }

private:

    typedef AsyncQueue<ResourceKey> RequestQueue;
    typedef AsyncQueue<std::pair<ResourceKey, Resource>> ResultQueue;

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
                    auto key = m_requests.front ();
                    m_requests.pop ();

                    // Retrieve object
                    auto object = Resource::retrieve (key);

                    // Queue retrieved object
                    m_results.push (std::make_pair (key, object));

		    if(m_results.size() >= 10)
		    {
			// Schedule update and notification in run in main thread
			Glib::signal_idle ().connect_once (sigc::mem_fun (this, &ResourceRetriever::notify));
		    }

                }
                while (!m_requests.empty ());

            }

            std::this_thread::sleep_for (std::chrono::milliseconds (500));
        }
    }

    void notify ()
    {
        while (!m_results.empty ()) {
            // Get object in retrieval queue
            auto result = m_results.front ();
            m_results.pop ();

            // Perform in-place replacement of stand-in object
            auto& key = std::get<0> (result);
            auto& obj = std::get<1> (result);

            // Notify all listeners waiting on completed fetches
            m_signal_completed.emit (key, obj); 
        }
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

    Resource& get (ResourceKey const& key, bool acquire = true )
    {
        // Look up object in table to see if it already exists in memory

        auto match = m_resources.find (key);

        if (match != m_resources.end ()) {
            // Required object already in memory, return it
            return (*match).second ; 
        }
	else
	{
	    auto s = Resource::retrieve_cached (key);

	    if(s)
	    {
		m_resources[key] = s; 
		return m_resources[key]; 
	    }
	}

        // Resource needs to be loaded asynchronously in the background

        // create stand-in dummy object
        m_resources[key] = Resource (key);

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

    std::unordered_map<ResourceKey, Resource> m_resources;
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
