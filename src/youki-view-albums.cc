#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "youki-view-albums.hh"

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include "mpx/util-string.hh"
#include "mpx/util-graphics.hh"
#include "mpx/mpx-covers.hh"
#include "mpx/i-youki-theme-engine.hh"
#include "mpx/widgets/cairo-extensions.hh"
#include "mpx/widgets/cairo-blur.hh"
#include "mpx/algorithm/vector_compare.hh"
#include <cmath>

namespace
{
    enum ReleaseType
    {
          RT_ALBUM
        , RT_SINGLE
        , RT_COMPILATION
        , RT_EP
        , RT_LIVE
        , RT_REMIX
        , RT_SOUNDTRACK
        , RT_OTHER
    };

    const std::vector<std::string> rt_string { "Album", "Single", "Compilation", "EP", "Live", "Remix", "OST", "?" } ;

    ReleaseType
    get_rt(const std::string& type)
    {
        if( type == "album" )
                return RT_ALBUM;

        if( type == "single" )
                return RT_SINGLE;

        if( type == "compilation" )
                return RT_COMPILATION;

        if( type == "ep" )
                return RT_EP;

        if( type == "live" )
                return RT_LIVE;

        if( type == "remix" )
                return RT_REMIX;

        if( type == "soundtrack" )
                return RT_SOUNDTRACK;

        return RT_OTHER;
    }
}

namespace MPX
{
namespace View
{
namespace Albums
{
	bool operator==( const Album_sp& a, const Album_sp& b )
	{
	    return a->album_id == b->album_id ;
	}

	bool operator!=( const Album_sp& a, const Album_sp& b )
	{
	    return a->album_id != b->album_id ;
	}

	DataModel::DataModel()
	: m_upper_bound(0)
	{
	    m_realmodel = Model_sp(new Model_t);
	}

	DataModel::DataModel(Model_sp model)
	: m_realmodel(model)
	, m_upper_bound(0)
	{
	}

	void
	DataModel::clear()
	{
	    m_realmodel->clear() ;
	    m_iter_map.clear() ;
	    m_upper_bound = 0 ;
	}

	Signal_0&
	DataModel::signal_redraw()
	{
	    return m_SIGNAL__redraw ;
	}

	Signal_1&
	DataModel::signal_changed()
	{
	    return m_SIGNAL__changed ;
	}

	Signal_1&
	DataModel::signal_cover_updated()
	{
	    return m_SIGNAL__cover_updated ;
	}

	bool
	DataModel::is_set()
	{
	    return bool(m_realmodel) ;
	}

	guint
	DataModel::size()
	{
	    return m_realmodel ? m_realmodel->size() : 0 ;
	}

	const Album_sp&
	DataModel::row(guint d)
	{
	    return (*m_realmodel)[d] ;
	}

	void
	DataModel::set_current_row(guint d)
	{
	    m_upper_bound = d ;
	}

	void	
	DataModel::append_album(const Album_sp album)
	{
	    m_realmodel->push_back( album ) ;
	    Model_t::iterator i = m_realmodel->end() ;
	    std::advance( i, -1 ) ;
	    m_iter_map.insert( std::make_pair( album->album_id, i )) ;
	}

	void
	DataModel::insert_album(const Album_sp album)
	{
	    static OrderFunc order ;

	    Model_t::iterator i = m_realmodel->insert(
		std::lower_bound(
		    m_realmodel->begin()
		  , m_realmodel->end()
		  , album
		  , order
		)
	      , album
	    ) ;

	    m_iter_map.insert( std::make_pair( album->album_id, i )) ;
	}

	void
	DataModel::update_album(const Album_sp album)
	{
	    if( album && m_iter_map.find( album->album_id ) != m_iter_map.end() )
	    {
		*(m_iter_map[album->album_id]) = album ;
		m_SIGNAL__redraw.emit() ;
	    }
	}

	void
	DataModel::erase_album(guint id)
	{
	    IdIterMap_t::iterator i = m_iter_map.find( id ) ;

	    if( i != m_iter_map.end() )
	    {
		m_realmodel->erase( i->second );
		m_iter_map.erase( id );
	    }
	}

	void
	DataModel::update_album_cover_cancel(guint id)
	{
	    IdIterMap_t::iterator i = m_iter_map.find( id ) ;

	    if(i != m_iter_map.end())
	    {
		Album_sp & album = *i->second ;

		if( album )
		{
		    album->caching = false ;
		    m_SIGNAL__redraw.emit() ;
		}
	    }
	}

	void
	DataModel::update_album_cover(
	    guint				id
	  , Cairo::RefPtr<Cairo::ImageSurface>	is
	)
	{
	    IdIterMap_t::iterator i = m_iter_map.find( id ) ;

	    if( i != m_iter_map.end() )
	    {
		Album_sp & album = *i->second ;

		if( album )
		{
		    album->caching = false ;

		    album->coverart = is ;
		    album->surface_cache.clear() ;

		    m_SIGNAL__cover_updated.emit( id ) ;
		    m_SIGNAL__redraw.emit() ;
		}
	    }
	}

///////////////////////

	DataModelFilter::DataModelFilter(
	      DataModel_sp model
	)
	: DataModel( model->m_realmodel )
	{
	    regen_mapping() ;
	}

	void
	DataModelFilter::set_constraints_albums(
	    TCVector_sp&  constraint
	)
	{
	    m_constraints_albums = constraint ;
	}

	void
	DataModelFilter::clear_constraints_albums(
	)
	{
	    m_constraints_albums.reset() ;
	}

	void
	DataModelFilter::set_constraints_artist(
	    IdVector_sp& constraint
	)
	{
	    m_constraints_artist = constraint ;
	}

	void
	DataModelFilter::clear_constraints_artist(
	)
	{
	    m_constraints_artist.reset() ;
	}

	void
	DataModelFilter::clear_all_constraints_quiet()
	{
	    m_constraints_artist.reset() ;
	    m_constraints_albums.reset() ;
	}

	void
	DataModelFilter::clear()
	{
	    DataModel::clear() ;
	    m_mapping.clear() ;
	    m_upper_bound = 0 ;
	    m_SIGNAL__redraw.emit() ;
	}

	guint
	DataModelFilter::size()
	{
	    return m_mapping.size();
	}

	const Album_sp&
	DataModelFilter::row(
	      guint d
	)
	{
	    return *m_mapping[d] ;
	}

	RowRowMapping_t::const_iterator
	DataModelFilter::iter(
	      guint d
	)
	{
	    RowRowMapping_t::const_iterator i = m_mapping.begin() ;
	    std::advance( i, d ) ;
	    return i ;
	}

	void
	DataModelFilter::swap(
	      guint   d1
	    , guint   d2
	)
	{
	    std::swap( m_mapping[d1], m_mapping[d2] ) ;
	    m_SIGNAL__redraw.emit() ;
	}

	void
	DataModelFilter::append_album(
	    const Album_sp album
	)
	{
	    DataModel::append_album( album ) ;
	}

	void
	DataModelFilter::erase_album(
	      guint id_album
	)
	{
	    DataModel::erase_album( id_album );
	}

	void
	DataModelFilter::insert_album(
	    const Album_sp album
	)
	{
	    DataModel::insert_album( album ) ;
	}

	void
	DataModelFilter::update_album(
	    const Album_sp album
	)
	{
	    DataModel::update_album( album ) ;
	    regen_mapping() ;
	}

	void
	DataModelFilter::regen_mapping(
	)
	{
	    using boost::get;

	    if( m_realmodel->empty() )
	    {
		return ;
	    }

	    RowRowMapping_t new_mapping ;
	    new_mapping.reserve( m_realmodel->size() ) ;

	    m_upper_bound = 0 ;

	    if( (!m_constraints_albums || m_constraints_albums->empty())) 
	    {
		for( auto i = m_realmodel->begin() ; i != m_realmodel->end() ; ++i )
		{
		    new_mapping.push_back( i ) ;
		}
	    }
	    else
	    {
		    const TCVector_t& constraints_ = *m_constraints_albums ;

		    for( auto i = m_realmodel->begin() ; i != m_realmodel->end(); ++i )
		    {
			if( constraints_[(*i)->album_id.get()].Count ) 
			{
			    new_mapping.push_back( i ) ;
			}
		    }
	    }

	    if( !vector_compare( m_mapping, new_mapping ))
	    {
		std::swap( new_mapping, m_mapping ) ;
		m_SIGNAL__changed.emit( m_upper_bound ) ;
	    }
	}

	void
	DataModelFilter::regen_mapping_iterative(
	)
	{
	    using boost::get;

	    if( m_realmodel->empty() )
	    {
		return ;
	    }

	    RowRowMapping_t new_mapping ;
	    new_mapping.reserve( m_mapping.size() ) ; // why? because it's only going to be at max. as large as the current mapping
	    m_upper_bound = 0 ;

	    const TCVector_t& constraints_ = *m_constraints_albums ;

	    for( auto i = m_mapping.begin() ; i != m_mapping.end(); ++i )
	    {
		if( constraints_[(**i)->album_id.get()].Count ) 
		{
		    new_mapping.push_back( *i ) ;
		}
	    }

	    if( !vector_compare( m_mapping, new_mapping ))
	    {
		std::swap( new_mapping, m_mapping ) ;
		m_SIGNAL__changed.emit( m_upper_bound ) ;
	    }
	}

///////////////////////

	Column::Column(
	)
	: m_width(0)
	, m_column(0)
	, m_show_additional_info( false )
	, m_rounding(0)
	{
	    m_image_disc = Util::cairo_image_surface_from_pixbuf(
				    Gdk::Pixbuf::create_from_file(
					    Glib::build_filename( DATA_DIR, "images" G_DIR_SEPARATOR_S "disc.png" )
	    )->scale_simple(90, 90, Gdk::INTERP_BILINEAR)) ;

	    m_image_new = Util::cairo_image_surface_from_pixbuf(
				    Gdk::Pixbuf::create_from_file(
					    Glib::build_filename( DATA_DIR, "images" G_DIR_SEPARATOR_S "new.png" ))) ;

	    m_image_album_loading = Gdk::PixbufAnimation::create_from_file(
					    Glib::build_filename( DATA_DIR, "images" G_DIR_SEPARATOR_S "album-cover-loading.gif" )) ;

	    m_image_album_loading_iter = m_image_album_loading->get_iter( NULL ) ;

	    m_rect_shadow = render_rect_shadow( 90, 90 ) ;
	}

	void
	Column::set_rounding(
	    double r
	)
	{
	    m_rounding = r ;
	}

	double
	Column::get_rounding()
	{
	    return m_rounding ;
	}

	void
	Column::set_width(
	    guint w
	)
	{
	    m_width = w ;
	}

	guint
	Column::get_width()
	{
	    return m_width ;
	}

	void
	Column::set_column(
	    guint c
	)
	{
	    m_column = c ;
	}

	guint
	Column::get_column()
	{
	    return m_column ;
	}

	Cairo::RefPtr<Cairo::ImageSurface>
	Column::render_icon(
	      const Album_sp                        album
	    , guint				    width 
	)
	{
	    Cairo::RefPtr<Cairo::ImageSurface> s = Cairo::ImageSurface::create( Cairo::FORMAT_ARGB32, 94, 94 ) ;
	    Cairo::RefPtr<Cairo::Context> cairo = Cairo::Context::create( s ) ;

	    cairo->set_operator( Cairo::OPERATOR_CLEAR ) ;
	    cairo->paint() ;

	    cairo->set_operator( Cairo::OPERATOR_OVER ) ;

	    cairo->set_source(
		  album->coverart ? album->coverart : m_image_disc
		, 2
		, 2
	    ) ;

	    RoundedRectangle(
		  cairo
		, 2
		, 2
		, 90
		, 90
		, m_rounding 
	    ) ;
	    cairo->fill() ;

	    return s ;
	}

	Cairo::RefPtr<Cairo::ImageSurface>
	Column::render_rect_shadow(	
	      guint w
	    , guint h
	)
	{
	    Cairo::RefPtr<Cairo::ImageSurface> s = Cairo::ImageSurface::create( Cairo::FORMAT_ARGB32, w+2, h+2 ) ;
	    Cairo::RefPtr<Cairo::Context> c2 = Cairo::Context::create(s) ;

	    c2->set_operator( Cairo::OPERATOR_CLEAR ) ;
	    c2->paint() ;

	    c2->set_operator( Cairo::OPERATOR_OVER ) ;
	    c2->set_source_rgba(
		      0 
		    , 0 
		    , 0 
		    , 0.35 
	    ) ;
	    RoundedRectangle( c2, 0, 0, w, h, m_rounding+1 ) ;
	    c2->fill() ;
	    Util::cairo_image_surface_blur( s, 2.5 ) ;
	    return s ;
	}

	void
	Column::render(
	      const Cairo::RefPtr<Cairo::Context>&  cairo
	    , const Album_sp&			    album
	    , const Gtk::Widget&                    widget
	    , guint				    row
	    , int                                   ypos
	    , guint				    row_height
	    , const ThemeColor&			    color
	    , guint				    model_mapping_size
	    , guint				    model_size
	    , bool				    selected
	    , const TCVector_sp&		    album_constraints
	)
	{
	    double h,s,b ;

	    Util::color_to_hsb( color, h, s, b ) ;
	    s *= 0.45 ;
	    Gdk::RGBA c1 = Util::color_from_hsb( h, s, b ) ;
	    c1.set_alpha( 1. ) ;

	    Util::color_to_hsb( color, h, s, b ) ;
	    s *= 0.95 ;
	    Gdk::RGBA c2 = Util::color_from_hsb( h, s, b ) ;
	    c2.set_alpha( 1. ) ;
	    
	    Gdk::RGBA c3 = color ;
	    c3.set_alpha( 0.7 ) ;
	    Util::color_to_hsb( color, h, s, b ) ;
	    s *= 0.25 ;
	    b *= 1.80 ;
	    c3 = Util::color_from_hsb( h, s, b ) ;
	    c3.set_alpha( 1. ) ;

	    GdkRectangle r ;

	    r.y = ypos + 7 ;
	    r.x = 0 ;

	    if( !album->caching )
	    {
		time_t now = time(NULL) ;
		Range<time_t> WithinPastDay(now-(24*60*60), now) ;

		if( !album->surface_cache ) 
		{
		    album->surface_cache = render_icon( album, m_width ) ;
		}

		if( album->coverart )
		{
		    Util::draw_cairo_image( cairo, m_rect_shadow, 12+2, r.y+4, 1., m_rounding ) ;
		}

		/* The actual coverart surface cache */
		Util::draw_cairo_image( cairo, album->surface_cache, 10, r.y, 1., m_rounding ) ;

		if( album->coverart )
		{
		    cairo->save();
		    cairo->translate(12,r.y+2) ;
		    RoundedRectangle(
			  cairo
			, 0 
			, 0 
			, 90
			, 90
			, m_rounding 
		    ) ;
		    cairo->set_line_width( 1.5 ) ; 
		    Gdk::Cairo::set_source_rgba(cairo, c2) ;
		    cairo->stroke() ;
		    cairo->restore() ;
		}

		/* 'new' */
		if( WithinPastDay( album->insert_date ))
		{
		    int x = 8 ; 
		    int y = r.y + 10 ; 
		    Util::draw_cairo_image( cairo, m_image_new, x, y, 1., 0 ) ;
		}
	    }
	    else
	    {
		cairo->save();
		cairo->translate(12,r.y+2) ;
		RoundedRectangle(
		      cairo
		    , 0 
		    , 0 
		    , 90
		    , 90
		    , m_rounding 
		) ;
		cairo->set_line_width( 1.5 ) ; 
		Gdk::Cairo::set_source_rgba(cairo, c2) ;
		cairo->stroke() ;
		cairo->restore() ;

		cairo->save() ;
		cairo->translate( 47, r.y+35 ) ;
		Glib::RefPtr<Gdk::Pixbuf> pb = m_image_album_loading_iter->get_pixbuf() ;
		Gdk::Cairo::set_source_pixbuf( cairo, pb, 0, 0 ) ; 
		cairo->rectangle(0,0, 20,20) ;
		cairo->fill() ;
		cairo->restore() ;
	    }

	    enum { L1, L2, L3, N_LS } ;
	    const int text_size_px[N_LS] = { 15, 15, 12 } ;
	    const int text_size_pt[N_LS] = {   static_cast<int> ((text_size_px[L1] * 72)
						    / Util::screen_get_y_resolution(Gdk::Screen::get_default()))
					     , static_cast<int> ((text_size_px[L2] * 72)
						    / Util::screen_get_y_resolution(Gdk::Screen::get_default()))
					     , static_cast<int> ((text_size_px[L3] * 72)
						    / Util::screen_get_y_resolution(Gdk::Screen::get_default()))} ;

	    std::vector<Pango::FontDescription> font_desc (3) ;
	    std::vector<Glib::RefPtr<Pango::Layout> > layout { Glib::wrap( pango_cairo_create_layout(cairo->cobj())),
							       Glib::wrap( pango_cairo_create_layout(cairo->cobj())),
							       Glib::wrap( pango_cairo_create_layout(cairo->cobj()))} ;

	    font_desc[L1] = widget.get_style_context()->get_font() ;
	    font_desc[L1].set_size( text_size_pt[L1] * PANGO_SCALE ) ;
	    font_desc[L1].set_stretch( Pango::STRETCH_EXTRA_CONDENSED ) ;

	    font_desc[L2] = widget.get_style_context()->get_font() ;
	    font_desc[L2].set_size( text_size_pt[L2] * PANGO_SCALE ) ;
	    font_desc[L2].set_stretch( Pango::STRETCH_EXTRA_CONDENSED ) ;

	    font_desc[L3] = widget.get_style_context()->get_font() ;
	    font_desc[L3].set_size( text_size_pt[L3] * PANGO_SCALE ) ;

	    layout[L1]->set_font_description( font_desc[L1] ) ;
	    layout[L1]->set_ellipsize( Pango::ELLIPSIZE_END ) ;

	    layout[L2]->set_font_description( font_desc[L2] ) ;
	    layout[L2]->set_ellipsize( Pango::ELLIPSIZE_END ) ;

	    layout[L3]->set_font_description( font_desc[L3] ) ;
	    layout[L3]->set_ellipsize( Pango::ELLIPSIZE_END ) ;

	    layout[L1]->set_width((m_width-16) * PANGO_SCALE ) ;
	    layout[L2]->set_width((m_width-16) * PANGO_SCALE ) ;
	    layout[L3]->set_width((m_width-16) * PANGO_SCALE ) ;

	    int width, height;

	    guint yoff = 98 ; 
	    guint xpos = 12 ;

	    /* Artist */
	    layout[L1]->set_text( album->album_artist ) ;
	    layout[L1]->get_pixel_size( width, height ) ;

	    if( selected )
	    {
		Util::render_text_shadow( layout[L1], xpos, r.y+yoff, cairo ) ;
	    }

	    cairo->move_to(
		  xpos
		, r.y + yoff
	    ) ;

	    Gdk::Cairo::set_source_rgba(cairo, c3) ; 
	    pango_cairo_show_layout(cairo->cobj(), layout[L1]->gobj()) ;

	    /* Album */
	    yoff += 17 ;

	    layout[L2]->set_text( album->album )  ;
	    layout[L2]->get_pixel_size( width, height ) ;

	    if( selected )
	    {
		Util::render_text_shadow( layout[L2], xpos, r.y+yoff, cairo ) ;
	    }

	    cairo->move_to(
		  xpos 
		, r.y + yoff
	    ) ;

	    Gdk::Cairo::set_source_rgba(cairo, color) ;
	    pango_cairo_show_layout(cairo->cobj(), layout[L2]->gobj()) ;

	    /* Total Time, no. of Discs, no. of Tracks */ 
	    if( m_show_additional_info )
	    {
		font_desc[L3].set_weight( Pango::WEIGHT_NORMAL ) ;
		layout[L3]->set_font_description( font_desc[L3] ) ;

		layout[L3]->set_width((m_width-120)*PANGO_SCALE) ; 
		layout[L3]->set_ellipsize( Pango::ELLIPSIZE_END ) ;

		guint tm = 0 ;

		std::string s_time, s_tracks, s_discs, s_rt, s_year ;

		if( album_constraints )
		{
		    tm = ((*album_constraints)[album->album_id.get()]).Time ;
    
		    guint hrs = tm / 3600 ;
		    guint min = (tm-hrs*3600) / 60 ;
		    guint sec = tm % 60 ;

		    guint totaltracks = ((*album_constraints)[album->album_id.get()]).Count ;

		    guint hrs_total = album->total_time / 3600 ;
		    guint min_total = (album->total_time-hrs_total*3600) / 60 ;
		    guint sec_total = album->total_time % 60 ;

		    if( album->total_time != tm )
			s_time = ((boost::format("%02u:%02u:%02u / %02u:%02u:%02u") % hrs % min % sec % hrs_total % min_total % sec_total).str()) ;
		    else
			s_time = ((boost::format("%02u:%02u:%02u") % hrs % min % sec).str()) ;

		    if( totaltracks != album->track_count )
			s_tracks = ((boost::format("<b>%u</b>/<b>%u</b>") % totaltracks % album->track_count).str()) ;
		    else
			s_tracks = ((boost::format("<b>%u</b> %s") % totaltracks % ((totaltracks>1) ? "Tracks" : "Track")).str()) ;
		}
		else
		{
		    tm = album->total_time ;

		    guint hrs = tm / 3600 ;
		    guint min = (tm-hrs*3600) / 60 ;
		    guint sec = tm % 60 ;

		    guint totaltracks = album->track_count ;

		    s_time = ((boost::format("%02u:%02u:%02u") % hrs % min % sec).str()) ;
		    s_tracks = ((boost::format("<b>%u</b> %s") % totaltracks % ((totaltracks>1) ? "Tracks" : "Track")).str()) ;
		}

		if( album->discs_count > 1 && !album_constraints )
		{ 
		    s_discs = ((boost::format("<b>%u</b> Discs") % album->discs_count).str()) ;
		}

		ReleaseType rt = get_rt( album->type ) ;

		if( rt != RT_OTHER && rt != RT_ALBUM )
		{
		    s_rt = rt_string[rt] ;
		}

		guint sx = 0 ;
		guint sy = r.y + 2 ;

		/* s_tracks */
		layout[L3]->set_markup( s_tracks ) ;
		layout[L3]->get_pixel_size( width, height ) ;

		sx = 112 ; 

		if( selected )
		{
		    Util::render_text_shadow( layout[L3], sx, sy, cairo ) ; 
		}

		cairo->move_to(
		      sx 
		    , sy 
		) ;
		Gdk::Cairo::set_source_rgba(cairo, Util::make_rgba(color,0.95)) ;
		pango_cairo_show_layout(cairo->cobj(), layout[L3]->gobj()) ;
		sy += height + 2 ;

		/* s_discs */
		if( !s_discs.empty() )
		{
		    layout[L3]->set_markup( s_discs ) ;
		    layout[L3]->get_pixel_size( width, height ) ;

		    if( selected )
		    {
			Util::render_text_shadow( layout[L3], sx, sy, cairo ) ; 
		    }

		    cairo->move_to(
			  sx 
			, sy 
		    ) ;
		    Gdk::Cairo::set_source_rgba(cairo, Util::make_rgba(color,0.95)) ;
		    pango_cairo_show_layout(cairo->cobj(), layout[L3]->gobj()) ;
		    sy += height + 2 ;
		}

		/* s_time */
		layout[L3]->set_markup( s_time ) ;
		layout[L3]->get_pixel_size( width, height ) ;

		if( selected )
		{
		    Util::render_text_shadow( layout[L3], sx, sy, cairo ) ; 
		}

		cairo->move_to(
		      sx 
		    , sy 
		) ;
		Gdk::Cairo::set_source_rgba(cairo, Util::make_rgba(color,0.95)) ;
		pango_cairo_show_layout(cairo->cobj(), layout[L3]->gobj()) ;
		sy += height + 2 ;

		/* s_rt */
		if( !s_rt.empty() )
		{
		    layout[L3]->set_markup( s_rt ) ;
		    layout[L3]->get_pixel_size( width, height ) ;

		    if( selected )
		    {
			Util::render_text_shadow( layout[L3], sx, sy, cairo ) ; 
		    }

		    cairo->move_to(
			  sx 
			, sy 
		    ) ;
		    Gdk::Cairo::set_source_rgba(cairo, Util::make_rgba(color,0.95)) ;
		    pango_cairo_show_layout(cairo->cobj(), layout[L3]->gobj()) ;
		    sy += height + 2 ;
		}

		/* year */ 
		if( !album->year.empty() )
		{
		    layout[L3]->set_markup( album->year.substr(0,4)) ;
		    layout[L3]->get_pixel_size( width, height ) ;

		    if( selected )
		    {
			Util::render_text_shadow( layout[L3], sx, sy, cairo ) ; 
		    }

		    cairo->move_to(
			  sx 
			, sy 
		    ) ;
		    Gdk::Cairo::set_source_rgba(cairo, Util::make_rgba(color,0.95)) ;
		    pango_cairo_show_layout(cairo->cobj(), layout[L3]->gobj()) ;
		    sy += height + 2 ;
		}

		/* label */ 
		if( !album->label.empty() )
		{
		    layout[L3]->set_markup( album->label ) ;
		    layout[L3]->get_pixel_size( width, height ) ;

		    if( selected )
		    {
			Util::render_text_shadow( layout[L3], sx, sy, cairo ) ; 
		    }

		    cairo->move_to(
			  sx 
			, sy 
		    ) ;
		    Gdk::Cairo::set_source_rgba(cairo, Util::make_rgba(color,0.95)) ;
		    pango_cairo_show_layout(cairo->cobj(), layout[L3]->gobj()) ;
		    sy += height + 2 ;
		}
	    }
	}

///////////////////////

	void
	Class::initialize_metrics ()
	{
	    ViewMetrics.set_base__row_height(
		  144
	    ) ;
	}

	void
	Class::on_vadj_value_changed()
	{
	    if( m_model->m_mapping.size() )
	    {
		int y1 = ViewMetrics.ViewPortPx.upper() ;
		int y2 = vadj_value() ;

		ViewMetrics.set( 
		      get_allocation().get_height() 
		    , vadj_value()
		) ;

		m_model->set_current_row( ViewMetrics.ViewPort.upper() ) ;
		get_window()->scroll( 0, y1 - y2 ) ; 
	    }
	}

	bool
	Class::on_key_press_event(GdkEventKey* event)
	{
	    if( !m_search_active && event->is_modifier )
		return false ;

	    if( !m_model->size() )
		return false ;

	    if( m_search_active )
	    {
		switch( event->keyval )
		{
		    case GDK_KEY_Return:
		    case GDK_KEY_KP_Enter:
		    case GDK_KEY_ISO_Enter:
		    case GDK_KEY_3270_Enter:
			cancel_search() ;
			m_SIGNAL_start_playback.emit() ;
			return true ;

		    case GDK_KEY_Up:
		    case GDK_KEY_KP_Up:
			find_prev_match() ;
			return true ;

		    case GDK_KEY_Down:
		    case GDK_KEY_KP_Down:
			find_next_match() ;
			return true ;

		    case GDK_KEY_Escape:
			cancel_search() ;
			return true ;

		    case GDK_KEY_Tab:
			cancel_search() ;
			return false ;

		    case GDK_KEY_BackSpace:
			if( m_SearchEntry->get_text().empty() )
			{
			    cancel_search() ;
			    return true ;
			}

		    default: ;
		}

		GdkEvent *new_event = gdk_event_copy( (GdkEvent*)(event) ) ;
		//g_object_unref( ((GdkEventKey*)new_event)->window ) ;
		((GdkEventKey *) new_event)->window = m_SearchWindow->get_window()->gobj();
		m_SearchEntry->event(new_event) ;
		//gdk_event_free(new_event) ;

		return true ;
	    }

	    int step = 0 ;
	    guint d = 0 ;

	    switch( event->keyval )
	    {
		case GDK_KEY_Return:
		case GDK_KEY_KP_Enter:
		case GDK_KEY_ISO_Enter:
		case GDK_KEY_3270_Enter:
		    if( m_selection )
		    {
			m_SIGNAL_start_playback.emit() ;
		    }
		    return true;

		case GDK_KEY_Up:
		case GDK_KEY_KP_Up:
		case GDK_KEY_Page_Up:

		    if( event->keyval == GDK_KEY_Page_Up )
		    {
			step = -ViewMetrics.ViewPort.size() ;
		    }
		    else
		    {
			step = -1 ;
		    }

		    if( !m_selection )
		    {
			mark_first_row_up:
			select_index( ViewMetrics.ViewPort.upper() ) ;
		    }
		    else
		    {
			guint origin = boost::get<Selection::INDEX>(m_selection.get()) ;

			if( ViewMetrics.ViewPort( origin ))
			{
			    d = std::max<int>( origin+step, 0 ) ;
			    select_index( d ) ;

			    double ymod = fmod( vadj_value(), ViewMetrics.RowHeight ) ;

			    if( event->keyval == GDK_KEY_Page_Up || d < ViewMetrics.ViewPort ) 
			    {
				scroll_to_index( std::max<int>( ViewMetrics.ViewPort.upper() + step, 0 )) ;
			    }
			    else if( ymod && d == ViewMetrics.ViewPort.upper() ) 
			    {
				vadj_value_set( std::max<int>( ViewMetrics.ViewPortPx.upper() - ymod, 0 )) ;
			    }
			}
			else
			{
			    goto mark_first_row_up ;
			}
		    }

		    return true;

		case GDK_KEY_Home:
		{
		    select_index(0) ;
		    scroll_to_index(0) ;

		    return true ;
		}

		case GDK_KEY_End:
		{
		    select_index(ModelCount(m_model->size())) ;
		    scroll_to_index(ModelCount(m_model->size())) ; 

		    return true ;
		}

		case GDK_KEY_Down:
		case GDK_KEY_KP_Down:
		case GDK_KEY_Page_Down:

		    if( event->keyval == GDK_KEY_Page_Down )
		    {
			step = ViewMetrics.ViewPort.size() ; 
		    }
		    else
		    {
			step = 1 ;
		    }

		    if( !m_selection )
		    {
			mark_first_row_down:
			select_index( ViewMetrics.ViewPort.upper() ) ;
		    }
		    else
		    {
			guint origin = boost::get<Selection::INDEX>(m_selection.get()) ;

			if( ViewMetrics.ViewPort( origin ))
			{
			    d = std::min<int>( origin+step, ModelCount(m_model->size())) ;

			    select_index( d ) ;

			    if( d >= ViewMetrics.ViewPort ) 
			    {
				guint pn = (ViewMetrics.ViewPort.upper()+1+(d-ViewMetrics.ViewPort.lower())) * ViewMetrics.RowHeight - ViewMetrics.Excess ; 
				vadj_value_set( std::min<int>( vadj_upper(), pn )) ; 
			    }
		       }
		       else
		       {
			    goto mark_first_row_down ;
		       }
		    }

		    return true;

		default:

		    if( !m_search_active && event->keyval != GDK_KEY_Tab )
		    {
			int x, y ;

			get_window()->get_origin( x, y ) ;
			y += get_allocation().get_height() ;

			m_SearchWindow->set_size_request( get_allocation().get_width(), -1 ) ;
			m_SearchWindow->move( x, y ) ;
			m_SearchWindow->show() ;

			focus_entry( true ) ;

			GdkEvent *new_event = gdk_event_copy( (GdkEvent*)(event) ) ;
			//g_object_unref( ((GdkEventKey*)new_event)->window ) ;
			((GdkEventKey *) new_event)->window = m_SearchWindow->get_window()->gobj();
			m_SearchEntry->event(new_event) ;
			//gdk_event_free(new_event) ;

			m_search_active = true ;

			return false ;
		    }
	    }

	    return false ;
	}

	void
	Class::focus_entry(
	    bool in 
	)
	{
	    gtk_widget_realize(GTK_WIDGET(m_SearchEntry->gobj())) ;

	    GdkEvent *event = gdk_event_new (GDK_FOCUS_CHANGE);

	    event->focus_change.type   = GDK_FOCUS_CHANGE;
	    event->focus_change.window = GDK_WINDOW(g_object_ref((*m_SearchEntry).get_window()->gobj())) ;
	    event->focus_change.in     = in;

	    (*m_SearchEntry).send_focus_change( event ) ;

	    //gdk_event_free( event ) ;
	}

	bool
	Class::on_button_press_event( GdkEventButton* event )
	{
	    using boost::get ;
	    cancel_search() ;

	    double ymod = fmod( vadj_value(), ViewMetrics.RowHeight ) ;
	    guint d = (vadj_value() + event->y) / ViewMetrics.RowHeight ;

	    if( !m_selection || (get<Selection::INDEX>(m_selection.get()) != d))
	    {
		if( ModelExtents( d ))
		{
		    select_index( d ) ;
		}

		if( d == ViewMetrics.ViewPort.upper()) 
		{
		    scroll_to_index(d) ;
		    //vadj_value_set( std::max<int>(0, ViewMetrics.ViewPortPx.upper() - ymod )) ;
		}
		else
		if( (!ymod && d == ViewMetrics.ViewPort.lower()))

		{
		    vadj_value_set( std::min<int>(vadj_upper(), ViewMetrics.ViewPortPx.upper() + (ViewMetrics.RowHeight - ymod) - ViewMetrics.Excess )) ;
		}
		else
		if( (ymod && d > ViewMetrics.ViewPort.lower()))

		{
		    vadj_value_set( std::min<int>(vadj_upper(), ViewMetrics.ViewPortPx.upper() + ViewMetrics.RowHeight + (ViewMetrics.RowHeight - ymod) - ViewMetrics.Excess )) ;
		}
	    }
	    else
	    if( event->button == 1 && m_selection && (get<Selection::INDEX>(m_selection.get()) == d))
	    {
		if( has_focus() )
		{
		    clear_selection() ;
		}
	    }

	    grab_focus() ;

	    if( event->button == 3 )
	    {
		m_pMenuPopup->popup(event->button, event->time) ;
	    }

	    return true ;
	}

	void
	Class::configure_vadj(
	      guint   upper
	    , guint   page_size
	    , guint   step_increment
	)
	{
	    if( property_vadjustment().get_value() )
	    {
		property_vadjustment().get_value()->set_upper( upper ) ;
		property_vadjustment().get_value()->set_page_size( page_size ) ;
		property_vadjustment().get_value()->set_step_increment( step_increment ) ;
		property_vadjustment().get_value()->set_page_increment( step_increment*4 ) ;
	    }
	}

	void
	Class::on_size_allocate( Gtk::Allocation& a )
	{
	    a.set_x(0) ;
	    a.set_y(0) ;
	    Gtk::DrawingArea::on_size_allocate(a) ;
	    queue_draw() ;
	}

	bool
	Class::on_focus_in_event(GdkEventFocus* G_GNUC_UNUSED)
	{
	    using boost::get ;
/*
	    if( !m_selection || (!ViewMetrics.ViewPort(get<Selection::INDEX>(m_selection.get()))))
	    {
		select_index( ViewMetrics.ViewPort.upper()) ;
		scroll_to_index( ViewMetrics.ViewPort.upper()) ;
	    }
*/

	    return false ;
	}

	bool
	Class::on_configure_event(
	    GdkEventConfigure* event
	)
	{
	    Gtk::DrawingArea::on_configure_event(event) ;

	    if( ViewMetrics.RowHeight )
	    {
		configure_vadj(
		      m_model->m_mapping.size() * ViewMetrics.RowHeight
		    , event->height
		    , ViewMetrics.RowHeight
		) ;
	    }

	    ViewMetrics.set( 
		  event->height 
		, vadj_value()
	    ) ;

	    double n                       = m_columns.size() ;
	    double column_width_calculated = event->width / n ;

	    for( guint n = 0; n < m_columns.size(); ++n )
	    {
		m_columns[n]->set_width( column_width_calculated ) ;
	    }

	    queue_draw() ;

	    return true ;
	}

	bool
	Class::on_draw(
	    const Cairo::RefPtr<Cairo::Context>& cairo 
	)
	{
	    cairo->save() ;

#if 0
	    Cairo::RefPtr<Cairo::LinearGradient> gradient = Cairo::LinearGradient::create(
		  0 
		, get_allocated_height()/2. 
		, get_allocated_width() 
		, get_allocated_height()/2. 
	    ) ;

	    gradient->add_color_stop_rgba(
		  0
		, 1. 
		, 1. 
		, 1. 
		, 0.3 
	    ) ;
	   
	    gradient->add_color_stop_rgba(
		  0.33 
		, 1. 
		, 1. 
		, 1. 
		, 0.18 
	    ) ;

	    gradient->add_color_stop_rgba(
		  0.43 
		, 1. 
		, 1. 
		, 1. 
		, 0.12
	    ) ;

	    gradient->add_color_stop_rgba(
		  0.5 
		, 1. 
		, 1. 
		, 1. 
		, 0.10 
	    ) ;

	    gradient->add_color_stop_rgba(
		  1. - 0.43 
		, 1.
		, 1. 
		, 1. 
		, 0.12 
	    ) ;

	    gradient->add_color_stop_rgba(
		  1. - 0.33
		, 1. 
		, 1. 
		, 1. 
		, 0.18
	    ) ;

	    gradient->add_color_stop_rgba(
		  1. 
		, 1.
		, 1. 
		, 1. 
		, 0.3 
	    ) ;

//	    cairo->set_source( gradient ) ;
//	    cairo->fill() ;

	    Cairo::RefPtr<Cairo::SurfacePattern> pattern = Cairo::SurfacePattern::create( m_background ) ;
	    pattern->set_extend( Cairo::EXTEND_REPEAT ) ;
	    cairo->set_source( pattern ) ;
	    cairo->rectangle( 0, 0, get_allocated_width(), get_allocated_height() ) ;
	    cairo->mask(gradient) ;
	    cairo->restore() ;
#endif

	    const ThemeColor& c_text     = m_theme->get_color( THEME_COLOR_TEXT ) ;
	    const ThemeColor& c_text_sel = m_theme->get_color( THEME_COLOR_TEXT_SELECTED ) ;
	    const ThemeColor& c_rules_h  = m_theme->get_color( THEME_COLOR_BASE_ALTERNATE ) ;

	    const std::vector<double> dashes { 1., 2. } ; 

	    guint d       = ViewMetrics.ViewPort.upper() ; 
	    guint d_max   = std::min<guint>( m_model->size(), ViewMetrics.ViewPort.size() + 1 ) ;
	    gint  ypos	  = 0 ;
	    gint  offset  = ViewMetrics.ViewPortPx.upper() - (d*ViewMetrics.RowHeight) ;
	   
	    boost::optional<GdkRectangle> r_sel ; 
	    
	    if( offset )
	    {
		ypos  -= offset ;
		d_max += 1 ;
	    }

	    /* Let's see if we can save some rendering */	
	    double clip_x1, clip_y1, clip_x2, clip_y2 ;
	
	    cairo->get_clip_extents( clip_x1, clip_y1, clip_x2, clip_y2 ) ;

	    if( clip_y1 > 0 && clip_y2 == ViewMetrics.ViewPortPx.lower() )
	    {
		guint d_clip = clip_y1 / ViewMetrics.RowHeight ;
		ypos += d_clip * ViewMetrics.RowHeight ;
		d += d_clip ;
		d_max -= (d_clip-1) ;
	    }
	    else
	    if( clip_y1 == 0 && clip_y2 < ViewMetrics.ViewPortPx.lower() )
	    {
		guint d_clip = clip_y2 / ViewMetrics.RowHeight ;
		d_max = d_clip+2 ;
	    }

	    guint n = 0 ;
	    Algorithm::Adder<guint> d_cur( d, n ) ;

	    GdkRectangle rr ;
	    rr.x = 0 ; 
	    rr.width = get_allocated_width() ;
	    rr.height = ViewMetrics.RowHeight ;

	    RowRowMapping_t::const_iterator i = m_model->iter( d_cur ) ;

	    while( n < d_max && ModelExtents(d_cur)) 
	    {
		int selected = m_selection && boost::get<Selection::INDEX>(m_selection.get()) == d_cur ;

		if( selected )
		{
		    rr.y = ypos ; 
		    r_sel = rr ;

		    m_theme->draw_selection_rectangle(
			  cairo
			, rr
			, has_focus()
			, 0
			, MPX::CairoCorners::CORNERS(0)
		    ) ;
		}
		else
		if(d_cur%2)
		{
		    rr.y = ypos ;
		    cairo->rectangle(
			  rr.x
			, rr.y
			, rr.width
			, rr.height
		    ) ;
		    Gdk::Cairo::set_source_rgba( cairo, c_rules_h ) ;
		    cairo->fill() ;
		}

		m_columns[0]->render(
		      cairo
		    , **i
		    ,  *this
		    , d_cur 
		    , ypos
		    , ViewMetrics.RowHeight 
		    , selected ? c_text_sel : c_text
		    , ModelCount(m_model->size())
		    , ModelCount(m_model->m_realmodel->size())
		    , selected
		    , m_model->m_constraints_albums
		) ;

		ypos += ViewMetrics.RowHeight ;

		++n ;
		++i ;
	    }

	    if( !is_sensitive() )
	    {
		cairo->rectangle(0,0,get_allocated_width(),get_allocated_height()) ;
		cairo->set_source_rgba(0,0,0,0.2) ;
		cairo->fill() ;
	    }

	    cairo->restore() ;

/*
	    if( has_focus() )
	    {
		GdkRectangle r ;

		r.x = 1 ;
		r.y = 1 ;
		r.width = get_allocated_width() - 1 ;
		r.height = get_allocated_height() - 1 ;

		m_theme->draw_focus(
		      cairo
		    , r
		    , has_focus()
		    , 0
		    , MPX::CairoCorners::CORNERS(0)
		) ;
	    }
*/

	    return true;
	}

	double
	Class::vadj_value()
	{
	    if(  property_vadjustment().get_value() )
		return property_vadjustment().get_value()->get_value() ;

	    return 0 ;
	}

	double
	Class::vadj_upper()
	{
	    if(  property_vadjustment().get_value() )
		return property_vadjustment().get_value()->get_upper() ;

	    return 0 ;
	}

	void
	Class::vadj_value_set( double v_ )
	{
	    if(  property_vadjustment().get_value() )
		return property_vadjustment().get_value()->set_value( v_ ) ;
	}

	void
	Class::on_model_changed(
	      guint d
	)
	{
	    configure_vadj(
		  m_model->size() * ViewMetrics.RowHeight
		, ViewMetrics.ViewPortPx.size() 
		, ViewMetrics.RowHeight 
	    ) ;

	    ModelExtents = Interval<guint>(
		  Interval<guint>::IN_EX
		, 0
		, m_model->m_mapping.size()
	    ) ;

	    if( m_model->m_mapping.size() < ViewMetrics.ViewPort.size() )
	    {
		scroll_to_index(0) ;
	    }
	    else
	    {
		scroll_to_index( d ) ;
	    }

	    queue_resize() ;
	}

	void
	Class::on_vadj_prop_changed()
	{
	    if( !property_vadjustment().get_value() )
		return ;

	    conn_vadj.disconnect() ; 

	    conn_vadj = property_vadjustment().get_value()->signal_value_changed().connect(
		sigc::mem_fun(
		    *this,
		    &Class::on_vadj_value_changed
	    ));

	    configure_vadj(
		  m_model->m_mapping.size() * ViewMetrics.RowHeight
		, ViewMetrics.ViewPortPx.size()
		, ViewMetrics.RowHeight
	    ) ;
	}

	void
	Class::invalidate_covers()
	{
	    for( auto& i : *(m_model->m_realmodel))
	    {
		i->surface_cache.clear() ;
	    }
	}

	void
	Class::set_show_additional_info( bool show )
	{
	    m_columns[0]->m_show_additional_info = show ;
	    queue_draw() ;
	}

	void
	Class::select_id(
	    boost::optional<guint> id
	)
	{
	    guint d = 0 ;

	    for( auto& i : m_model->m_mapping )
	    {
		if( id == (*i)->album_id )
		{
		    select_index(d) ; 
		    return ;
		}
		++d ;
	    }
	}

	void
	Class::scroll_to_index(
	      guint d
	)
	{
		if( m_model->m_mapping.size() < ViewMetrics.ViewPort.size() ) 
		{
		    vadj_value_set(0) ;
		}
		else
		{
		    Limiter<guint> d_lim (
			  Limiter<guint>::ABS_ABS
			, 0
			, (m_model->size() * ViewMetrics.RowHeight) - ViewMetrics.ViewPort.size()
			, d * ViewMetrics.RowHeight 
		    ) ;

		    vadj_value_set(d_lim) ;
		}
	}

	void
	Class::select_index(
	      guint d
	    , bool  quiet
	)
	{
	    if( ModelExtents( d ))
	    {
		boost::optional<guint> id = (*m_model->m_mapping[d])->album_id ;

		m_selection = boost::make_tuple( m_model->m_mapping[d], id, d ) ;

		if( !quiet )
		{
		    m_SIGNAL_selection_changed.emit() ;
		}

		queue_draw();
	    }
	}

	boost::optional<guint>
	Class::get_selected_id()
	{
	    boost::optional<guint> id ;

	    if( m_selection )
	    {
		id = boost::get<Selection::ID>(m_selection.get()) ;
	    }

	    return id ;
	}

	boost::optional<guint>
	Class::get_selected_index()
	{
	    boost::optional<guint> idx ;

	    if( m_selection )
	    {
		idx = boost::get<Selection::INDEX>(m_selection.get()) ;
	    }

	    return idx ; 
	}

	boost::optional<guint>
	Class::get_selected()
	{
	    boost::optional<guint> id ;

	    if( m_selection )
	    {
		id = boost::get<Selection::ID>(m_selection.get()) ;
	    }

	    return id ;
	}

	void
	Class::set_model(DataModelFilter_sp model)
	{
	    m_model = model;

	    m_model->signal_changed().connect(
		sigc::mem_fun(
		    *this,
		    &Class::on_model_changed
	    ));

	    m_model->signal_redraw().connect(
		sigc::mem_fun(
		    *this,
		    &Gtk::Widget::queue_draw
	    ));

	    m_model->signal_cover_updated().connect(
		sigc::mem_fun(
		    *this,
		    &Class::handle_cover_updated
	    ));

	    on_model_changed(0) ;
	}

	void
	Class::append_column(
	      Column_sp   column
	)
	{
	    m_columns.push_back(column);
	}

	void
	Class::find_next_match()
	{
	    using boost::get ;

	    Glib::ustring text = m_SearchEntry->get_text().casefold() ;

	    if(text.empty())
		return ;

	    auto i = m_model->m_mapping.begin();

	    if( m_selection )
	    {
		std::advance( i, get<Selection::INDEX>(m_selection.get()) ) ;
		++i ;
	    }

	    auto d = std::distance( m_model->m_mapping.begin(), i ) ;

	    for( ; i != m_model->m_mapping.end(); ++i )
	    {
		Glib::ustring match = Glib::ustring((**i)->album).casefold() ;

		if( match.length() && match.substr(0, text.length()) == text.substr(0, text.length()))
		{
		    scroll_to_index( std::max<int>(0, d-ViewMetrics.ViewPort.size()/2)) ;
		    select_index( d ) ;
		    return ;
		}
		++d ;
	    }

	    error_bell() ;
	}

	void
	Class::find_prev_match()
	{
	    using boost::get ;

	    Glib::ustring text = m_SearchEntry->get_text().casefold() ;

	    if( text.empty() )
	    {
		return ;
	    }

	    auto i = m_model->m_mapping.begin() ;

	    if( m_selection )
	    {
		std::advance( i, get<Selection::INDEX>(m_selection.get()) ) ;
		--i ;
	    }

	    guint d = std::distance( m_model->m_mapping.begin(), i ) ;

	    for( ; i >= m_model->m_mapping.begin(); --i )
	    {
		Glib::ustring match = Glib::ustring((**i)->album).casefold() ;

		if( match.length() && match.substr(0, text.length()) == text.substr(0, text.length()))
		{
		    scroll_to_index( std::max<int>(0, d-ViewMetrics.ViewPort.size()/2)) ;
		    select_index( d ) ;
		    return ;
		}
		--d ;
	    }

	    error_bell() ;
	}

	void
	Class::on_search_entry_changed()
	{
	    using boost::get ;

	    Glib::ustring text = m_SearchEntry->get_text().casefold() ;

	    if(text.empty())
		return ;

	    guint d = 0 ; 

	    for( auto& i : m_model->m_mapping ) 
	    {
		Glib::ustring match = Glib::ustring((*i)->album).casefold() ;

		if( match.length() && match.substr(0, text.length()) == text.substr(0, text.length()))
		{
		    scroll_to_index( std::max<int>(0,d-ViewMetrics.ViewPort.size()/2)) ;
		    select_index( d ) ;
		    m_SearchEntry->unset_color() ;
		    return ;
		}
		++d ;
	    }

	    m_SearchEntry->override_color(Util::make_rgba(1.,0.,0.,1.)) ;
	}

	void
	Class::on_search_entry_activated()
	{
	    cancel_search() ;
	    m_SIGNAL_find_accepted.emit() ;
	}

	bool
	Class::on_search_window_focus_out(
	      GdkEventFocus* G_GNUC_UNUSED
	)
	{
	    cancel_search() ;
	    return false ;
	}

	void
	Class::on_show_only_this_album()
	{
	    if( m_selection )
	    {
		Album_sp album = *(boost::get<Selection::ITERATOR>(m_selection.get())) ;
		_signal_0.emit( album->mbid ) ;
	    }
	}

	void
	Class::on_show_only_this_artist()
	{
	    if( m_selection )
	    {
		Album_sp album = *(boost::get<Selection::ITERATOR>(m_selection.get())) ;
		_signal_1.emit( album->mbid_artist ) ;
	    }
	}

	void
	Class::on_refetch_album_cover()
	{
	    if( m_selection ) 
	    {
		//set_has_tooltip(false) ;
		while(gtk_events_pending()) gtk_main_iteration() ;

		Album_sp& album = *(boost::get<Selection::ITERATOR>(m_selection.get())) ;
		album->caching = true ;
		m_caching.insert( album->album_id.get()) ;
		_signal_2.emit( album->album_id.get()) ;
		queue_draw() ;

		m_redraw_spinner_conn.disconnect() ;
		m_redraw_spinner_conn = Glib::signal_timeout().connect( sigc::mem_fun( *this, &Class::handle_redraw ), 100 ) ;
	    }
	}

	bool
	Class::handle_redraw()
	{
	    m_columns[0]->m_image_album_loading_iter->advance() ;
	    queue_draw() ;

	    while(gtk_events_pending()) gtk_main_iteration() ;

	    if(m_caching.empty())
	    {
		m_redraw_spinner_conn.disconnect() ;
		//set_has_tooltip(true) ;
	    }

	    return !m_caching.empty() ;
	}

	void
	Class::handle_cover_updated( guint id )
	{
	    m_caching.erase( id ) ;
	}

	void
	Class::cancel_search()
	{
	    focus_entry( false ) ;
	    m_SearchWindow->hide() ;
	    m_search_changed_conn.block () ;
	    m_SearchEntry->set_text("") ;
	    m_search_changed_conn.unblock () ;
	    m_search_active = false ;
	}

	void
	Class::on_realize()
	{
	    Gtk::DrawingArea::on_realize() ;
	    initialize_metrics();

	    Glib::RefPtr<Gtk::StyleContext> sc = get_parent()->get_style_context() ;
	    sc->context_save() ;
	    sc->add_class("frame") ;
	    GValue v = G_VALUE_INIT ;
	    gtk_style_context_get_property(
		  GTK_STYLE_CONTEXT(sc->gobj())
		, "border-radius"
		, GTK_STATE_FLAG_NORMAL
		, &v
	    ) ; 
	    int radius = g_value_get_int(&v) ;
	    sc->context_restore() ;

	    g_message("radius: %d", radius) ;

	    set_rounding(radius) ;
	    queue_resize();
	}

	bool
	Class::query_tooltip(
	      int                                   tooltip_x
	    , int                                   tooltip_y
	    , bool                                  keypress
	    , const Glib::RefPtr<Gtk::Tooltip>&     tooltip
	)
	{
	    guint d = (ViewMetrics.ViewPortPx.upper() + tooltip_y) / ViewMetrics.RowHeight ;

	    if(!ModelExtents(d)) return false ;

	    Album_sp album = *(m_model->m_mapping[d]) ;

	    boost::shared_ptr<Covers> covers = services->get<Covers>("mpx-service-covers") ;

	    Gtk::Image * image = Gtk::manage( new Gtk::Image ) ;

	    Glib::RefPtr<Gdk::Pixbuf> cover ;

	    if( covers->fetch(
		  album->mbid
		, cover
	    ))
	    {   
		image->set( cover->scale_simple( 320, 320, Gdk::INTERP_BILINEAR)) ;
		tooltip->set_custom( *image ) ;
		return true ;
	    }

	    return false ;
	}

	void
	Class::clear_selection(
	)
	{
	    m_selection.reset() ;
	    m_SIGNAL_selection_changed.emit() ;
	    queue_draw() ;
	}

	void
	Class::clear_selection_quiet(
	)
	{
	    m_selection.reset() ;
	}

	void
	Class::set_rounding(double r)
	{
	    m_columns[0]->set_rounding(r) ;
	    queue_draw() ;
	}

	Class::Class()

	    : ObjectBase( "YoukiViewAlbums" )
	    , property_vadj_(*this, "vadjustment", RPAdj(0))
	    , property_hadj_(*this, "hadjustment", RPAdj(0))
	    , property_vsp_(*this, "vscroll-policy", Gtk::SCROLL_NATURAL )
	    , property_hsp_(*this, "hscroll-policy", Gtk::SCROLL_NATURAL )
	    , m_search_active( false )

	{
	    m_background = Util::cairo_image_surface_from_pixbuf(
				    Gdk::Pixbuf::create_from_file(
					    Glib::build_filename( DATA_DIR, "images" G_DIR_SEPARATOR_S "album-background.png" )
	    )) ;

	    property_vadjustment().signal_changed().connect( sigc::mem_fun( *this, &Class::on_vadj_prop_changed )) ;

	    set_can_focus(true);

	    ModelCount = Minus<int>( 1 ) ;

	    m_theme = services->get<IYoukiThemeEngine>("mpx-service-theme") ;

	    add_events(Gdk::EventMask(GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK | GDK_BUTTON_PRESS_MASK | GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_SCROLL_MASK ));

	    m_SearchEntry = Gtk::manage( new Gtk::Entry ) ;
	    m_SearchEntry->show() ;

	    m_search_changed_conn = m_SearchEntry->signal_changed().connect(
		    sigc::mem_fun(
			  *this
			, &Class::on_search_entry_changed
	    )) ;

	    m_SearchEntry->signal_activate().connect(
		    sigc::mem_fun(
			  *this
			, &Class::on_search_entry_activated
	    )) ;

	    m_SearchWindow = new Gtk::Window( Gtk::WINDOW_POPUP ) ;
	    m_SearchWindow->set_decorated( false ) ;
	    m_SearchWindow->set_border_width( 4 ) ;

	    m_SearchWindow->signal_focus_out_event().connect(
		    sigc::mem_fun(
			  *this
			, &Class::on_search_window_focus_out
	    )) ;

	    signal_focus_out_event().connect(
		    sigc::mem_fun(
			  *this
			, &Class::on_search_window_focus_out
	    )) ;

	    m_SearchWindow->add( *m_SearchEntry ) ;
	    m_SearchEntry->show() ;

/*
	    signal_query_tooltip().connect(
		sigc::mem_fun(
		      *this
		    , &Class::query_tooltip
	    )) ;
*/

	    //set_has_tooltip(true) ;

	    m_refActionGroup = Gtk::ActionGroup::create() ;
	    m_refActionGroup->add( Gtk::Action::create("ContextMenu", "Context Menu")) ;

	    m_refActionGroup->add( Gtk::Action::create("ContextShowAlbum", "Show this Album"),
		sigc::mem_fun(*this, &Class::on_show_only_this_album)) ;
	    m_refActionGroup->add( Gtk::Action::create("ContextShowArtist", "Show this Artist"),
		sigc::mem_fun(*this, &Class::on_show_only_this_artist)) ;
	    m_refActionGroup->add( Gtk::Action::create("ContextFetchCover", "(Re-)fetch Album Cover"),
		sigc::mem_fun(*this, &Class::on_refetch_album_cover)) ;

	    m_refUIManager = Gtk::UIManager::create() ;
	    m_refUIManager->insert_action_group(m_refActionGroup) ;

	    std::string ui_info =
	    "<ui>"
	    "   <popup name='PopupMenu'>"
	    "       <menuitem action='ContextShowAlbum'/>"
	    "       <menuitem action='ContextShowArtist'/>"
	    "	    <separator/>"
	    "       <menuitem action='ContextFetchCover'/>"
	    "   </popup>"
	    "</ui>" ;

	    m_refUIManager->add_ui_from_string( ui_info ) ;
	    m_pMenuPopup = dynamic_cast<Gtk::Menu*>(m_refUIManager->get_widget("/PopupMenu")) ;
	}
}
}
}
