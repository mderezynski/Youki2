#include "config.h"
#include "webkit-album-info.hh"

#include <future>

#include <boost/format.hpp>
#include "mpx/xml/xmltoc++.hh"
#include "xmlcpp/xsd-album-info-2.0.hxx"
#include "mpx/mpx-main.hh"
#include "mpx/mpx-uri.hh"
#include "mpx/util-string.hh"
#include "mpx/mpx-covers.hh"

namespace MPX
{
    AlbumInfo::AlbumInfo()
    {
	mScroll = Gtk::manage( new Gtk::ScrolledWindow ) ;
	mView = WEBKIT_WEB_VIEW(webkit_web_view_new()) ;

	g_object_set(webkit_web_view_get_settings(mView), "auto-resize-window", TRUE, NULL);

	Gtk::VBox *vbox = Gtk::manage( new Gtk::VBox ) ;
	vbox->set_border_width(4) ;
	vbox->set_spacing(4) ;

	mScroll->add(*(Glib::wrap(GTK_WIDGET(mView)))) ;
	vbox->add(*mScroll) ;
	add(*vbox) ;    
	vbox->show_all() ;
    }

    AlbumInfo::~AlbumInfo()
    {
    }

    void
    AlbumInfo::display(
	const AlbumInfo::Qualifier& q
    )
    {
	typedef MPX::XmlInstance<LastFMAlbumInfo::lfm> Instance ;

	set_title(q.Name_Album) ;

	Instance* Xml = nullptr ;

	std::string wiki ;
	std::string image_url ;

	const std::string f_mbid = "http://ws.audioscrobbler.com/2.0/?method=album.getinfo&api_key=b25b959554ed76058ac220b7b2e0a026&mbid=%s" ;
	const std::string f_weak = "http://ws.audioscrobbler.com/2.0/?method=album.getinfo&api_key=b25b959554ed76058ac220b7b2e0a026&artist=%s&album=%s" ;

	std::string f_effective ;

	if( q.MBID.substr(0,4) == "mpx-" )
	    f_effective = (boost::format(f_weak) % Glib::Markup::escape_text(q.Name_Artist) % Glib::Markup::escape_text(q.Name_Album)).str() ;
	else
	    f_effective = (boost::format(f_mbid) % q.MBID).str() ; 

	try{
	    auto handle = std::async(std::launch::async, [&Xml, &f_effective]()
	    {
		Xml = new Instance(f_effective) ;
	    }) ;

	    while(!handle.wait_for(std::chrono::duration<int, std::milli>(40)))
	    {
		while(gtk_events_pending()) gtk_main_iteration() ;
	    }

	    if(!Xml)
	    {
		throw std::runtime_error("no document") ;
	    }

	    auto& album = Xml->xml().album() ;

	    wiki = album.wiki().summary() ;

	    image_url = album.image()[3] ;
	    image_url = image_url.substr(0, image_url.length()-3) ;
	    image_url += "jpg" ;

	    delete Xml ;
	}
	catch(std::exception& cxe) {
	    g_message("%s: RuntimeError: %s", G_STRLOC, cxe.what()) ;
	    delete Xml ;
	}
	catch(std::runtime_error& cxe){
	    g_message("%s: RuntimeError: %s", G_STRLOC, cxe.what()) ;
	    delete Xml ;
	}

	const std::string html = 
	    "<html>"
	    "<body>"
	    "<div id='div_img'><img id='image' src='%s'/></div>"
	    "<div id='wiki'>%s</div>"
	    "</body>"
	    "</html>"
	; 

	webkit_web_view_load_string(
	      mView
	    , (boost::format(html) % image_url % wiki).str().c_str()
	    , "text/html"
	    , "UTF-8"
	    , nullptr
	) ;

	WebKitDOMDocument *document ;

	static const guchar CSS[] =
	"body { font-size: 10pt; }\n"
	"#div_img { width: 100%; text-align: center; }\n"
	"#image { -webkit-border-radius: 12px; border: 2px solid black; }\n"
	"#wiki { "
	" -webkit-border-radius: 8px; "
	" background: -webkit-gradient(linear, 0% 100%, 0%, 0%,"
	" from(#f1f1f1), to(white));"
	" border: solid 1px #c6c6c6;"
	" -webkit-box-shadow: 0px 0px 2px #c6c6c6;"
	" margin: 12px; padding: 6px; } " ;

	gchar *tmp = g_base64_encode(CSS, strlen((gchar*)CSS)) ;
	gchar *css = g_strconcat("data:text/css;charset=utf-8;base64,",tmp,NULL) ;

	g_object_set(webkit_web_view_get_settings(mView), "user-stylesheet-uri", css, NULL);

	g_free(css) ;
	g_free(tmp) ;

	resize( 400, 520 ) ;
	set_position( Gtk::WIN_POS_CENTER ) ;
	present() ;
    }
}
