#ifndef MPX_WEBKIT_ALBUM_INFO
#define MPX_WEBKIT_ALBUM_INFO

#include <webkit/webkit.h>
#include <gtkmm.h>

namespace MPX
{
    class AlbumInfo : public Gtk::Window
    {
	public:

	    struct Qualifier
	    {
		std::string	MBID ;
		std::string	Name_Artist ;
		std::string	Name_Album ;
	    } ;

	    AlbumInfo() ;
	    virtual ~AlbumInfo() ;

	    bool
	    display(
		const Qualifier&
	    ) ;

	protected:

	    WebKitWebView * mView ;
	    Gtk::ScrolledWindow * mScroll ;
    } ; 
}

#endif
