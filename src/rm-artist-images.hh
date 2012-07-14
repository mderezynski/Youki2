#ifndef RM_ARTIST_IMAGES_HH
#define RM_ARTIST_IMAGES_HH

#include <string>
#include <gdkmm/pixbuf.h>
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>

namespace MPX
{
    class ArtistImage
    {
	public:

	    typedef std::string Key ;

	    ArtistImage()
	    {}

	    explicit ArtistImage(Key const& key)
	    : m_key(key)
	    {}

	    ArtistImage(Key const& key, Glib::RefPtr<Gdk::Pixbuf> const& contents)
		: m_key(key)
		, m_contents(contents)
	    { 
	    }

	    static ArtistImage retrieve_cached(Key const& key) ; 
	    static ArtistImage retrieve(Key const& key) ;

	    operator bool ()
	    {
		return bool(get_image()) ;
	    }

	    Glib::RefPtr<Gdk::Pixbuf>
	    get_image()
	    { 
		return m_contents ;
	    }

	private:
	
	    Key				m_key ;
	    Glib::RefPtr<Gdk::Pixbuf>	m_contents ;
    } ;
}

#endif/*RM_ARTIST_IMAGES_HH*/
