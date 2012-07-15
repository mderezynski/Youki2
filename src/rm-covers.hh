#ifndef RM_ALBUM_IMAGES_HH
#define RM_ALBUM_IMAGES_HH

#include <string>
#include <map>
#include <set>

#include <gdkmm/pixbuf.h>
#include <cairomm/cairomm.h>
#include <glibmm/ustring.h>

#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>

#include "mpx/mpx-minisoup.hh"
#include "mpx/mpx-main.hh"
#include "mpx/mpx-network.hh"
#include "mpx/mpx-services.hh"
#include "mpx/metadatareader-taglib.hh"

#include "rm-covers-stores.hh"
#include "resource_manager.hh"

namespace MPX
{
namespace RM
{
    class AlbumImage
    {
	public:

	    typedef RequestQualifier Key ;

	    AlbumImage()
	    {}

	    explicit AlbumImage(Key const& key)
	    : m_key(key)
	    {}

	    AlbumImage(AlbumImage const& other)
	    : m_key(other.m_key)
	    , m_contents(other.m_contents)
	    {}

	    AlbumImage(Key const& key, Glib::RefPtr<Gdk::Pixbuf> const& contents)
		: m_key(key)
		, m_contents(contents)
	    { 
	    }

	    static AlbumImage retrieve_cached(Key const& key) ; 
	    static AlbumImage retrieve(Key const& key) ;

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
}}
#endif/*RM_ALBUM_IMAGES_HH*/
