#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "youki-view-albums.hh"

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include "mpx/util-string.hh"
#include "mpx/util-graphics.hh"
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

    const std::vector<std::string> rt_string { "Album", "Single", "Compilation", "EP", "Live", "Remix", "OST", "?" };

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
	    return a->album_id == b->album_id;
	}

	bool operator!=( const Album_sp& a, const Album_sp& b )
	{
	    return a->album_id != b->album_id;
	}

	DataModel::DataModel()
	: m_upper_bound(0)
	{
	    m_base_model = Model_sp(new Model_t);
	}

	DataModel::DataModel(Model_sp model)
	: m_base_model(model)
	, m_upper_bound(0)
	{
	}

	void
	DataModel::clear()
	{
	    m_base_model->clear();
	    m_iter_map.clear();
	    m_upper_bound = 0;
	}

	Signal_0&
	DataModel::signal_redraw()
	{
	    return m_SIGNAL__redraw;
	}

	Signal_1&
	DataModel::signal_changed()
	{
	    return m_SIGNAL__changed;
	}

	Signal_1&
	DataModel::signal_cover_updated()
	{
	    return m_SIGNAL__cover_updated;
	}

	bool
	DataModel::is_set()
	{
	    return bool(m_base_model);
	}

	guint
	DataModel::size()
	{
	    return m_base_model ? m_base_model->size() : 0;
	}

	const Album_sp&
	DataModel::row(guint d)
	{
	    return (*m_base_model)[d];
	}

	void
	DataModel::set_upper_bound(guint d)
	{
	    m_upper_bound = d;
	}

	void	
	DataModel::append_album(const Album_sp album)
	{
	    m_base_model->push_back( album );
	    Model_t::iterator i = m_base_model->end();
	    std::advance( i, -1 );
	    m_iter_map.insert( std::make_pair( *(album->album_id), i ));
	}

	void
	DataModel::insert_album(const Album_sp album)
	{
	    static OrderFunc order;

	    Model_t::iterator i = m_base_model->insert(
		std::lower_bound(
		    m_base_model->begin()
		  , m_base_model->end()
		  , album
		  , order
		)
	      , album
	    );

	    m_iter_map.insert( std::make_pair( *(album->album_id), i ));
	}

	void
	DataModel::update_album(const Album_sp album)
	{
	    if( album && m_iter_map.find( *(album->album_id)) != m_iter_map.end() )
	    {
		*(m_iter_map[*(album->album_id)]) = album;
		m_SIGNAL__redraw.emit();
	    }
	}

	void
	DataModel::erase_album(	
	      guint id
	)
	{
	    IdIterMap_t::iterator i = m_iter_map.find( id );

	    if( i != m_iter_map.end() )
	    {
		m_iter_map.erase(id);
		m_base_model->erase( i->second );
	    }
	}

	void
	DataModel::update_album_cover_cancel(guint id)
	{
	    IdIterMap_t::iterator i = m_iter_map.find( id );

	    if(i != m_iter_map.end())
	    {
		Album_sp & album = *i->second;

		if( album )
		{
		    album->caching = false;
		    m_SIGNAL__redraw.emit();
		}
	    }
	}

	void
	DataModel::update_album_cover(
	    guint				id
	  , Cairo::RefPtr<Cairo::ImageSurface>	is
	)
	{
	    IdIterMap_t::iterator i = m_iter_map.find( id );

	    if( i != m_iter_map.end() )
	    {
		Album_sp & album = *i->second;

		if( album )
		{
		    album->caching = false;

		    album->coverart = is;
		    album->surface_cache.clear();

		    m_SIGNAL__cover_updated.emit( id );
		    m_SIGNAL__redraw.emit();
		}
	    }
	}

///////////////////////

	DataModelFilter::DataModelFilter(
	      DataModel_sp model
	)
	: DataModel( model->m_base_model )
	{
	    regen_mapping();
	}

	void
	DataModelFilter::set_constraints_albums(
	    TCVector_sp&  constraint
	)
	{
	    m_constraints_albums = constraint;
	}

	void
	DataModelFilter::clear_constraints_albums(
	)
	{
	    m_constraints_albums.reset();
	}

	void
	DataModelFilter::set_constraints_artist(
	    TCVector_sp& constraint
	)
	{
	    m_constraints_artist = constraint;
	}

	void
	DataModelFilter::clear_constraints_artist(
	)
	{
	    m_constraints_artist.reset();
	}

	void
	DataModelFilter::clear_all_constraints_quiet()
	{
	    m_constraints_artist.reset();
	    m_constraints_albums.reset();
	}

	void
	DataModelFilter::clear()
	{
	    DataModel::clear();
	    m_mapping.clear();
	    m_upper_bound = 0;
	    m_SIGNAL__redraw.emit();
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
	    return *m_mapping[d];
	}

	RowRowMapping_t::const_iterator
	DataModelFilter::iter(
	      guint d
	)
	{
	    RowRowMapping_t::const_iterator i = m_mapping.begin();
	    std::advance( i, d );
	    return i;
	}

	void
	DataModelFilter::swap(
	      guint   d1
	    , guint   d2
	)
	{
	    std::swap( m_mapping[d1], m_mapping[d2] );
	    m_SIGNAL__redraw.emit();
	}

	void
	DataModelFilter::append_album(
	    const Album_sp album
	)
	{
	    DataModel::append_album( album );
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
	    DataModel::insert_album( album );
	}

	void
	DataModelFilter::update_album(
	    const Album_sp album
	)
	{
	    DataModel::update_album( album );
	}

	void
	DataModelFilter::regen_mapping(
	)
	{
	    using boost::get;

	    if( m_base_model->empty() )
	    {
		return;
	    }

	    RowRowMapping_t new_mapping;
	    new_mapping.reserve( m_base_model->size() );

	    boost::optional<guint> upper_bound_prev_id ;
    
	    if( !m_mapping.empty()) {
		if( m_upper_bound < m_base_model->size() && row(m_upper_bound) && row(m_upper_bound)->album_id) {
		    upper_bound_prev_id = row(m_upper_bound)->album_id.get() ;
		}
	    }

	    m_upper_bound = 0 ;
	    guint c = 0 ;

	    if( (!m_constraints_albums || m_constraints_albums->empty())) 
	    {
		for( auto i = m_base_model->begin(); i != m_base_model->end(); ++i )
		{
		    if( upper_bound_prev_id && ((*i)->album_id.get() == upper_bound_prev_id))
		    {
			m_upper_bound = c ; 
		    }

		    new_mapping.push_back( i );
		    ++c ;
		}
	    }
	    else
	    {
		for( auto i = m_base_model->begin(); i != m_base_model->end(); ++i )
		{
		    if( (*m_constraints_albums)[(*i)->album_id.get()].Count ) 
		    {
			if( upper_bound_prev_id && ((*i)->album_id.get() == upper_bound_prev_id ))
			{
			    m_upper_bound = c ; 
			}

			new_mapping.push_back( i );
			++c ;
		    }
		}
	    }

	    std::swap( new_mapping, m_mapping );
	    m_SIGNAL__changed.emit( m_upper_bound );
	}

	void
	DataModelFilter::regen_mapping_iterative(
	)
	{
	    using boost::get;

	    if( m_base_model->empty() )
	    {
		return;
	    }

	    RowRowMapping_t new_mapping;
	    new_mapping.reserve( m_mapping.size() ); // why? because it's only going to be at max. as large as the current mapping

	    boost::optional<guint> upper_bound_prev_id ;
    
	    if( !m_mapping.empty() )
	    {
		if( m_upper_bound < m_mapping.size() && row(m_upper_bound) && row(m_upper_bound)->album_id)
		{
		    upper_bound_prev_id = row(m_upper_bound)->album_id.get() ;
		}
	    }

	    m_upper_bound = 0 ;
	    guint c = 0 ;

	    for( auto i = m_mapping.begin(); i != m_mapping.end(); ++i )
	    {
		if( (*m_constraints_albums)[(**i)->album_id.get()].Count ) 
		{
		    if( upper_bound_prev_id && ((**i)->album_id.get() == upper_bound_prev_id ))
		    {
			m_upper_bound = c ; 
		    }

		    new_mapping.push_back( *i );
		    ++c ;
		}
	    }

	    std::swap( new_mapping, m_mapping );
	    m_SIGNAL__changed.emit( m_upper_bound );
	}

///////////////////////

	Column::Column(
	)

		: m_width(0)
		, m_column(0)
		, m_show_additional_info( false )
		, m_rounding(0)

	{
	    m_image_lensflare = Util::cairo_image_surface_from_pixbuf(
				    Gdk::Pixbuf::create_from_file(
					    Glib::build_filename( DATA_DIR, "images" G_DIR_SEPARATOR_S "lensflare.png" )
	    )) ;

	    m_image_disc = Util::cairo_image_surface_from_pixbuf(
				    Gdk::Pixbuf::create_from_file(
					    Glib::build_filename( DATA_DIR, "images" G_DIR_SEPARATOR_S "disc.png" )
	    )->scale_simple(256, 256, Gdk::INTERP_BILINEAR)) ;

	    m_image_new = Util::cairo_image_surface_from_pixbuf(
				    Gdk::Pixbuf::create_from_file(
					    Glib::build_filename( DATA_DIR, "images" G_DIR_SEPARATOR_S "new.png" ))) ;

	    m_image_album_loading = Gdk::PixbufAnimation::create_from_file(
					    Glib::build_filename( DATA_DIR, "images" G_DIR_SEPARATOR_S "album-cover-loading.gif" )) ;

	    m_image_album_loading_iter = m_image_album_loading->get_iter(NULL) ;

	    auto s = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, 248, 89 ) ;
	    auto c = Cairo::Context::create(s) ;

	    c->set_operator(Cairo::OPERATOR_CLEAR) ;
	    c->paint() ;

	    c->set_operator(Cairo::OPERATOR_SOURCE) ;
	    RoundedRectangle(
		  c
		, 3
		, 3
		, 242
		, 83
		, m_rounding 
	    ) ;
	    c->set_source_rgba(0.6,0.6,0.6,0.7) ;
	    c->fill() ;
	    
	    Util::cairo_image_surface_blur( s, 2 ) ;

	    m_image_shadow = s ;
	}

	void
	Column::set_rounding(
	    double r
	)
	{
	    m_rounding = 1 ; // r;
	}

	double
	Column::get_rounding()
	{
	    return m_rounding;
	}

	void
	Column::set_width(
	    guint w
	)
	{
	    m_width = w;
	}

	guint
	Column::get_width()
	{
	    return m_width;
	}

	void
	Column::set_column(
	    guint c
	)
	{
	    m_column = c;
	}

	guint
	Column::get_column()
	{
	    return m_column;
	}

	void
	Column::render_rgba(
	    Album_sp album
	)	
	{
	    Glib::RefPtr<Gdk::Pixbuf> pb = Util::cairo_image_surface_to_pixbuf(album->coverart?album->coverart:album->surface_cache) ;
	    album->rgba = Util::pick_color_for_pixbuf(pb) ;
	    album->rgba->set_alpha(1);
	}

	void	
	Column::render_icon(
	    Album_sp album
	)
	{
	    album->surface_cache = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, 256, 98) ;
	    auto cairo = Cairo::Context::create(album->surface_cache) ;

	    cairo->set_operator(Cairo::OPERATOR_CLEAR) ;
	    cairo->paint() ;

	    auto icon_desaturated_1 =
		Util::cairo_image_surface_to_pixbuf( album->coverart ? album->coverart : m_image_disc ) ; 
	    auto icon_desaturated_2 = icon_desaturated_1->copy() ;

	    auto icon = icon_desaturated_1->copy() ; 

	    icon->saturate_and_pixelate(icon_desaturated_1, 0.85, false) ;
	    icon->saturate_and_pixelate(icon_desaturated_2, 0.25, false) ;

	    cairo->set_operator(Cairo::OPERATOR_OVER) ;
	    cairo->set_source(
		  Util::cairo_image_surface_from_pixbuf(icon_desaturated_1)
		,   0
		, -80 
	    ) ;
	    cairo->rectangle(
		  0
		, 0
		, 256
		, 130
	    ) ;
	    cairo->fill() ;


	    album->surface_cache_blur = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, 256, 98) ;
	    cairo = Cairo::Context::create(album->surface_cache_blur) ;

	    cairo->set_operator(Cairo::OPERATOR_CLEAR) ;
	    cairo->paint() ;

	    cairo->set_operator(Cairo::OPERATOR_OVER) ;
	    cairo->set_source(
		    Util::cairo_image_surface_from_pixbuf(icon_desaturated_2) 
		,   0
		, -80 
	    ) ;

	    cairo->rectangle(
		  0
		, 0
		, 256
		, 130
	    ) ;
	    cairo->clip() ;
	    cairo->fill() ;

//	    Util::cairo_image_surface_blur( album->surface_cache_blur, 3 ) ;
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
	    , const ThemeColor&			    color_sel
	    , const ThemeColor&			    color_bg_sel
	    , guint				    model_mapping_size
	    , guint				    model_size
	    , bool				    selected
	    , const TCVector_sp&		    album_constraints
	)
	{
	    enum { L1, L2, L3, N_LS } ;
	    const int text_size_px[N_LS] = { 15, 15, 12 };
	    const int text_size_pt[N_LS] = {   static_cast<int> ((text_size_px[L1] * 72)
						    / Util::screen_get_y_resolution(Gdk::Screen::get_default()))
					     , static_cast<int> ((text_size_px[L2] * 72)
						    / Util::screen_get_y_resolution(Gdk::Screen::get_default()))
					     , static_cast<int> ((text_size_px[L3] * 72)
						    / Util::screen_get_y_resolution(Gdk::Screen::get_default()))};

	    std::vector<Pango::FontDescription> font_desc (3);
	    std::vector<Glib::RefPtr<Pango::Layout> > layout { Glib::wrap( pango_cairo_create_layout(cairo->cobj())),
							       Glib::wrap( pango_cairo_create_layout(cairo->cobj())),
							       Glib::wrap( pango_cairo_create_layout(cairo->cobj()))};

	    font_desc[L1] = widget.get_style_context()->get_font();
	    font_desc[L1].set_size( text_size_pt[L1] * PANGO_SCALE );
	    font_desc[L1].set_stretch( Pango::STRETCH_EXTRA_CONDENSED );
	    font_desc[L1].set_variant( Pango::VARIANT_SMALL_CAPS ) ;

	    font_desc[L2] = widget.get_style_context()->get_font();
	    font_desc[L2].set_size( text_size_pt[L2] * PANGO_SCALE );
	    font_desc[L2].set_stretch( Pango::STRETCH_EXTRA_CONDENSED );
	    font_desc[L2].set_weight( Pango::WEIGHT_BOLD );
	    font_desc[L2].set_variant( Pango::VARIANT_SMALL_CAPS ) ;

	    font_desc[L3] = widget.get_style_context()->get_font();
	    font_desc[L3].set_size( text_size_pt[L3] * PANGO_SCALE );
	    font_desc[L3].set_weight( Pango::WEIGHT_BOLD );

	    layout[L1]->set_font_description( font_desc[L1] );
	    layout[L1]->set_ellipsize( Pango::ELLIPSIZE_END );

	    layout[L2]->set_font_description( font_desc[L2] );
	    layout[L2]->set_ellipsize( Pango::ELLIPSIZE_END );

	    layout[L3]->set_font_description( font_desc[L3] );
	    layout[L3]->set_ellipsize( Pango::ELLIPSIZE_END );

	    layout[L1]->set_width((220) * PANGO_SCALE);
	    layout[L2]->set_width((220) * PANGO_SCALE);

	    int width, height;

	    double h,s,b ;

	    Util::color_to_hsb( color, h, s, b );
	    s *= 0.45;
	    Gdk::RGBA c1 = Util::color_from_hsb( h, s, b );
	    c1.set_alpha(1.);

	    Util::color_to_hsb( color, h, s, b );
	    s *= 0.95;
	    Gdk::RGBA c2 = Util::color_from_hsb( h, s, b );
	    c2.set_alpha(1.);
	    
	    Util::color_to_hsb( color, h, s, b );
	    b = fmin( 1, b*1.3 ) ;
	    Gdk::RGBA c3 = Util::color_from_hsb( h, s, b );

	    Gdk::RGBA c5 ;

	    if( selected )
		c5 = color_bg_sel ; 
	    else
		c5 = Util::make_rgba(.4,.4,.4,1.) ;

	    time_t time_now = time(NULL) ;
	    Range<time_t> WithinPast3Days(time_now-(3*24*60*60), time_now) ;

	    if(!album->surface_cache) 
	    {
		render_icon( album ) ;
	    }

	    if(!album->rgba)
	    {
		render_rgba( album ) ;
	    }

	    cairo->save() ;
	    cairo->translate((m_width-256)/2.,ypos+7) ;

	    //////////

	    cairo->save() ;
	    RoundedRectangle(
		  cairo
		, 5   
		, 3   
		, 247 
		, 86 
		, m_rounding
	    ) ;
	    Gdk::Cairo::set_source_rgba(cairo, Util::make_rgba(0,0,0,1)) ;
	    cairo->set_line_width(3) ;
	    cairo->stroke() ;
	    cairo->restore() ;

	    //////////

	    if(1) // !album->caching )
	    {
		RoundedRectangle(
		      cairo
		    , 5 
		    , 3 
		    , 247
		    , 86
		    , m_rounding
		) ;
		cairo->set_source(album->surface_cache,0,0) ;
		cairo->fill_preserve() ;

		//////////

		auto gradient = Cairo::LinearGradient::create( 96, 3, 96, 84 ) ;

		gradient->add_color_stop_rgba(
		      0
		    , 0 
		    , 0 
		    , 0
		    , 0.20 
		) ;
		gradient->add_color_stop_rgba(
		      0.48
		    , 0
		    , 0
		    , 0
		    , 0.10
		) ;
		gradient->add_color_stop_rgba(
		      0.72
		    , 0
		    , 0
		    , 0
		    , 0.10
		) ;
		gradient->add_color_stop_rgba(
		      1 
		    , 0
		    , 0
		    , 0
		    , .05 
		) ;
		cairo->set_source(gradient) ;
		cairo->fill() ;

		//////////

		if(0) // ( selected )
		{
		    RoundedRectangle(
			  cairo
			, 5 
			, 3 
			, 247
			, 86
			, m_rounding
		    ) ;
		    cairo->set_source(m_image_lensflare,0,0) ;
		    cairo->fill() ;
		}

		//////////

		cairo->save() ;
		Gdk::Cairo::set_source_rgba(cairo, Util::make_rgba(selected?c5:Util::make_rgba(0,0,0,1),selected?.80:.5)) ;
		RoundedRectangle(
		      cairo
		    , 5
		    , 49
		    , 247  
		    , 42 
		    , m_rounding
		    , MPX::CairoCorners::CORNERS(12)
		) ;
		cairo->fill() ;
		cairo->restore() ;
	    }

	    if( album->caching )
	    {
		cairo->save() ;
		cairo->translate(14,12) ;
		Gdk::Cairo::set_source_pixbuf(
		      cairo
		    , m_image_album_loading_iter->get_pixbuf()
		    , 0
		    , 0
		) ;
		cairo->rectangle(0,0,20,20) ;
		cairo->fill() ;
		cairo->restore() ;
	    }

	    //////////

	    cairo->save() ;
	    cairo->translate(10,52) ;
	    layout[L2]->set_text( album->album ) ;
	    layout[L2]->get_pixel_size( width, height );
	    cairo->move_to(
		  0 
		, 0 
	    );
	    Gdk::Cairo::set_source_rgba(cairo, Util::make_rgba(1,1,1)) ; 
	    pango_cairo_show_layout(cairo->cobj(), layout[L2]->gobj()) ;
	    cairo->restore() ;

	    //////////

	    cairo->save() ;
	    cairo->translate(10,69) ;
	    layout[L1]->set_text( album->album_artist );
	    layout[L1]->get_pixel_size( width, height );
	    cairo->move_to(
		  0 
		, 0 
	    );
	    Gdk::Cairo::set_source_rgba(cairo, Util::make_rgba(1,1,1,0.9)) ; 
	    pango_cairo_show_layout(cairo->cobj(), layout[L1]->gobj()) ;
	    cairo->restore() ;

#if 0
	    //////////

	    if( !album->year.empty() )
	    {
		cairo->save() ;

		RoundedRectangle(
		      cairo
		    , 5 
		    , 3 
		    , 39
		    , 18
		    , m_rounding
		    , MPX::CairoCorners::CORNERS(9)
		) ;

		Gdk::Cairo::set_source_rgba(cairo, Util::make_rgba(c5,.7)) ;
		cairo->fill() ;

		cairo->restore() ;

		//////////

		cairo->save() ;

		layout[L3]->set_text(album->year.substr(0,4)) ;
		layout[L3]->get_pixel_size( width, height );

		int x = 10 ; 
		int y = 4 ;

		cairo->translate(x,y) ;
		cairo->move_to(
		      0 
		    , 0 
		) ;

		Gdk::Cairo::set_source_rgba(cairo, Util::make_rgba(1,1,1,0.90));
		pango_cairo_show_layout(cairo->cobj(), layout[L3]->gobj());

		cairo->restore() ;
	    }
#endif

#if 0
	    //////////

	    font_desc[L3].set_weight( Pango::WEIGHT_NORMAL );
	    layout[L3]->set_font_description( font_desc[L3] );
	    layout[L3]->set_width(184*PANGO_SCALE); 
	    layout[L3]->set_ellipsize( Pango::ELLIPSIZE_END );

	    guint album_trks = album_constraints ? ((*album_constraints)[album->album_id.get()]).Count : album->track_count ;
	    guint album_time = album_constraints ? ((*album_constraints)[album->album_id.get()]).Time : album->total_time ;

	    std::string infostr ; 

	    //////////

	    if( album_trks != album->track_count )
	    {
		infostr = ((boost::format("<b>%u</b> %s of <b>%u</b>") % album_trks % ((album_trks>1)?"Tracks":"Track")% album->track_count).str());
	    }
	    else
	    {
		infostr = ((boost::format("<b>%u</b> %s") % album->track_count % ((album->track_count>1)?"Tracks":"Track")).str());
	    }

	    layout[L3]->set_markup( infostr );
	    layout[L3]->get_pixel_size( width, height );
	    cairo->move_to(
		  4 
		, 92
	    ) ;
	    Gdk::Cairo::set_source_rgba(cairo, selected?color_sel:color) ; 
	    pango_cairo_show_layout(cairo->cobj(), layout[L3]->gobj());


	    guint hrs = album_time / 3600;
	    guint min = (album_time-hrs*3600) / 60;
	    guint sec = album_time % 60;

	    guint hrs_total = album->total_time / 3600;
	    guint min_total = (album->total_time-hrs_total*3600) / 60;
	    guint sec_total = album->total_time % 60;

	    if( album->total_time != album_time )
		infostr = ((boost::format("<b>%02u</b>:<b>%02u</b>:<b>%02u</b> of <b>%02u</b>:<b>%02u</b>:<b>%02u</b>") % hrs % min % sec % hrs_total % min_total % sec_total).str());
	    else
		infostr = ((boost::format("<b>%02u</b>:<b>%02u</b>:<b>%02u</b>") % hrs % min % sec).str());
	    
	    layout[L3]->set_markup( infostr );
	    layout[L3]->get_pixel_size( width, height );
	    cairo->move_to(
		  250 - width 
		, 92
	    ) ;
	    Gdk::Cairo::set_source_rgba(cairo, selected?color_sel:color) ; 
	    pango_cairo_show_layout(cairo->cobj(), layout[L3]->gobj());

	    //////////
#endif


	    if(WithinPast3Days( album->insert_date ))
	    {
		Util::draw_cairo_image( cairo, m_image_new, 228, 10 );
	    }

	    cairo->restore() ;
	}

//////////////////////

	void
	Class::initialize_metrics ()
	{
	    ViewMetrics.set_base__row_height(
		  106
	    );
	}

	void
	Class::on_vadj_value_changed()
	{
	    if( m_model->m_mapping.size() )
	    {
		int y1 = ViewMetrics.ViewPortPx.upper();
		int y2 = vadj_value();

		ViewMetrics.set( 
		      get_allocation().get_height() 
		    , vadj_value()
		);

		m_model->set_upper_bound( ViewMetrics.ViewPort.upper() );

		//get_window()->scroll( 0, y1 - y2 ); 

		if( m_button_depressed )
		{
		    guint d = (vadj_value() + m_y_old) / ViewMetrics.RowHeight;

		    if( !m_selection || (boost::get<Selection::INDEX>(m_selection.get()) != d))
		    {
			if( ModelExtents( d ))
			{
			    select_index( d );
			}
		    }
		}

		queue_draw() ;
	    }
	}

	bool
	Class::on_motion_notify_event(
	    GdkEventMotion* event
	)
	{
#if 0
	    if( m_button_depressed )
	    {
		int x_orig, y_orig;

		GdkModifierType state;

		if( event->is_hint )
		{
		    gdk_window_get_device_position( event->window, event->device, &x_orig, &y_orig, &state ) ;
		}
		else
		{
		    x_orig = int(event->x);
		    y_orig = int(event->y);

		    state = GdkModifierType(event->state);
		} 

		m_y_old = y_orig ;

		double ymod = fmod( vadj_value(), ViewMetrics.RowHeight );
		guint d = (vadj_value() + y_orig) / ViewMetrics.RowHeight;

		if( d <= ViewMetrics.ViewPort.upper()) 
		{
		    scroll_to_index(d);
		}
		else
		if( (!ymod && d == ViewMetrics.ViewPort.lower()))

		{
		    vadj_value_set( std::min<int>(vadj_upper(), ViewMetrics.ViewPortPx.upper() + (ViewMetrics.RowHeight - ymod) - ViewMetrics.Excess ));
		}
		else
		if( (ymod && d > ViewMetrics.ViewPort.lower()))

		{
		    vadj_value_set( std::min<int>(vadj_upper(), ViewMetrics.ViewPortPx.upper() + ViewMetrics.RowHeight + (ViewMetrics.RowHeight - ymod) - ViewMetrics.Excess ));
		}

		if( !m_selection || (boost::get<Selection::INDEX>(m_selection.get()) != d))
		{
		    if( ModelExtents( d ))
		    {
			select_index( d );
		    }
		}
	    }
#endif

	    return true ;
	}


	bool
	Class::on_focus_in_event(
	    GdkEventFocus* G_GNUC_UNUSED
	)
	{
	    if( m_selection )
	    { 
		guint d = boost::get<2>(m_selection.get()) ;

		if(( d <= ViewMetrics.ViewPort.upper())||(d > ViewMetrics.ViewPort.lower()))
		{
		    scroll_to_index(d) ;
		}
	    }

	    return false ;
	}

	bool
	Class::on_key_press_event(GdkEventKey* event)
	{
	    if( !m_search_active && event->is_modifier )
		return false;

	    if( !m_model->size() )
		return false;

	    if( m_search_active )
	    {
		switch( event->keyval )
		{
		    case GDK_KEY_Return:
		    case GDK_KEY_KP_Enter:
		    case GDK_KEY_ISO_Enter:
		    case GDK_KEY_3270_Enter:
			cancel_search();
			m_SIGNAL_start_playback.emit();
			return true;

		    case GDK_KEY_Up:
		    case GDK_KEY_KP_Up:
			find_prev_match();
			return true;

		    case GDK_KEY_Down:
		    case GDK_KEY_KP_Down:
			find_next_match();
			return true;

		    case GDK_KEY_Escape:
			cancel_search();
			return true;

		    case GDK_KEY_Tab:
			cancel_search();
			return false;

		    case GDK_KEY_BackSpace:
			if( m_SearchEntry->get_text().empty() )
			{
			    cancel_search();
			    return true;
			}

		    default:;
		}

		GdkEvent *new_event = gdk_event_copy( (GdkEvent*)(event) );
		//g_object_unref( ((GdkEventKey*)new_event)->window );
		((GdkEventKey *) new_event)->window = m_SearchWindow->get_window()->gobj();
		m_SearchEntry->event(new_event);
		//gdk_event_free(new_event);

		return true;
	    }

	    int step = 0;
	    guint d = 0;

	    switch( event->keyval )
	    {
		case GDK_KEY_Escape:
		    clear_selection() ;
		    return true;

		case GDK_KEY_Return:
		case GDK_KEY_KP_Enter:
		case GDK_KEY_ISO_Enter:
		case GDK_KEY_3270_Enter:
		    if( m_selection )
		    {
			m_SIGNAL_start_playback.emit();
		    }
		    return true;

		case GDK_KEY_Up:
		case GDK_KEY_KP_Up:
		case GDK_KEY_Page_Up:

		    if( event->keyval == GDK_KEY_Page_Up )
		    {
			step = -ViewMetrics.ViewPort.size();
		    }
		    else
		    {
			step = -1;
		    }

		    if( !m_selection )
		    {
			mark_first_row_up:
			select_index( ViewMetrics.ViewPort.upper() );
		    }
		    else
		    {
			guint origin = boost::get<Selection::INDEX>(m_selection.get());

			if( ViewMetrics.ViewPort( origin ))
			{
			    d = std::max<int>( origin+step, 0 );
			    select_index( d );

			    double ymod = fmod( vadj_value(), ViewMetrics.RowHeight );

			    if( event->keyval == GDK_KEY_Page_Up || d < ViewMetrics.ViewPort ) 
			    {
				scroll_to_index( std::max<int>( ViewMetrics.ViewPort.upper() + step, 0 ));
			    }
			    else if( ymod && d == ViewMetrics.ViewPort.upper() ) 
			    {
				vadj_value_set( std::max<int>( ViewMetrics.ViewPortPx.upper() - ymod, 0 ));
			    }
			}
			else
			{
			    goto mark_first_row_up;
			}
		    }

		    return true;

		case GDK_KEY_Home:
		{
		    select_index(0);
		    scroll_to_index(0);

		    return true;
		}

		case GDK_KEY_End:
		{
		    select_index(ModelCount(m_model->size()));
		    scroll_to_index(ModelCount(m_model->size())); 

		    return true;
		}

		case GDK_KEY_Down:
		case GDK_KEY_KP_Down:
		case GDK_KEY_Page_Down:

		    if( event->keyval == GDK_KEY_Page_Down )
		    {
			step = ViewMetrics.ViewPort.size(); 
		    }
		    else
		    {
			step = 1;
		    }

		    if( !m_selection )
		    {
			mark_first_row_down:
			select_index( ViewMetrics.ViewPort.upper() );
		    }
		    else
		    {
			guint origin = boost::get<Selection::INDEX>(m_selection.get());

			if( ViewMetrics.ViewPort( origin ))
			{
			    d = std::min<int>( origin+step, ModelCount(m_model->size()));

			    select_index( d );

			    if( d >= ViewMetrics.ViewPort ) 
			    {
				guint pn = (ViewMetrics.ViewPort.upper()+1+(d-ViewMetrics.ViewPort.lower())) * ViewMetrics.RowHeight - ViewMetrics.Excess; 
				vadj_value_set( std::min<int>( vadj_upper(), pn )); 
			    }
		       }
		       else
		       {
			    goto mark_first_row_down;
		       }
		    }

		    return true;

		default:

		    if( !m_search_active && event->keyval != GDK_KEY_Tab )
		    {
			int x, y;

			get_window()->get_origin( x, y );
			y += get_allocation().get_height();

			m_SearchWindow->set_size_request( get_allocation().get_width(), -1 );
			m_SearchWindow->move( x, y );
			m_SearchWindow->show();

			focus_entry( true );

			GdkEvent *new_event = gdk_event_copy( (GdkEvent*)(event) );
			//g_object_unref( ((GdkEventKey*)new_event)->window );
			((GdkEventKey *) new_event)->window = m_SearchWindow->get_window()->gobj();
			m_SearchEntry->event(new_event);
			//gdk_event_free(new_event);

			m_search_active = true;

			return false;
		    }
	    }

	    return false;
	}

	void
	Class::focus_entry(
	    bool in 
	)
	{
	    gtk_widget_realize(GTK_WIDGET(m_SearchEntry->gobj()));

	    GdkEvent *event = gdk_event_new (GDK_FOCUS_CHANGE);

	    event->focus_change.type   = GDK_FOCUS_CHANGE;
	    event->focus_change.window = GDK_WINDOW(g_object_ref((*m_SearchEntry).get_window()->gobj()));
	    event->focus_change.in     = in;

	    (*m_SearchEntry).send_focus_change( event );

	    //gdk_event_free( event );
	}

	bool
	Class::on_button_release_event(
	    GdkEventButton*
	)
	{
	    m_button_depressed = false ;
	    return false ;
	}

	bool
	Class::on_button_press_event(
	    GdkEventButton* event
	)
	{
	    using boost::get;

	    cancel_search();

	    m_y_old = event->y ;

	    double ymod = fmod( vadj_value(), ViewMetrics.RowHeight );
	    guint d = (vadj_value() + event->y) / ViewMetrics.RowHeight;

	    if( d == ViewMetrics.ViewPort.upper()) 
	    {
		scroll_to_index(d);
	    }
	    else
	    if( (!ymod && d == ViewMetrics.ViewPort.lower()))

	    {
		vadj_value_set( std::min<int>(vadj_upper(), ViewMetrics.ViewPortPx.upper() + (ViewMetrics.RowHeight - ymod) - ViewMetrics.Excess ));
	    }
	    else
	    if( (ymod && d > ViewMetrics.ViewPort.lower()))

	    {
		vadj_value_set( std::min<int>(vadj_upper(), ViewMetrics.ViewPortPx.upper() + ViewMetrics.RowHeight + (ViewMetrics.RowHeight - ymod) - ViewMetrics.Excess ));
	    }

	    if( event->button == 1 && (event->type == GDK_2BUTTON_PRESS) && m_selection )
	    {
		m_SIGNAL_start_playback.emit() ;
	    }

	    if( (!m_selection || (get<Selection::INDEX>(m_selection.get()) != d)))
	    {
		if( ModelExtents( d ))
		{
		    select_index( d );
		}
	    }
	    else
	    if( event->button == 1 && (m_selection && (get<Selection::INDEX>(m_selection.get()) == d)))
	    {
		if( has_focus() )
		{
		    clear_selection();
		}
	    }

	    grab_focus();

	    if( event->button == 3 )
	    {
		m_pMenuPopup->popup(event->button, event->time);
	    }
	    else
		m_button_depressed = true ;

	    return true;
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
		property_vadjustment().get_value()->freeze_notify() ;
		property_vadjustment().get_value()->set_upper((upper<page_size)?page_size:upper);
		property_vadjustment().get_value()->set_page_size( page_size ) ; 
		property_vadjustment().get_value()->thaw_notify() ;
	    }
	}

	void
	Class::on_size_allocate( Gtk::Allocation& a )
	{
	    a.set_x(0);
	    a.set_y(0);
	    Gtk::DrawingArea::on_size_allocate(a);
	    queue_draw();
	}

	bool
	Class::on_configure_event(
	    GdkEventConfigure* event
	)
	{
	    Gtk::DrawingArea::on_configure_event(event);

	    if( ViewMetrics.RowHeight )
	    {
		configure_vadj(
		      m_model->m_mapping.size() * ViewMetrics.RowHeight
		    , event->height
		    , ViewMetrics.RowHeight
		);
	    }

	    ViewMetrics.set( 
		  event->height 
		, vadj_value()
	    );

	    double n                       = m_columns.size();
	    double column_width_calculated = event->width / n;

	    for( guint n = 0; n < m_columns.size(); ++n )
	    {
		m_columns[n]->set_width( column_width_calculated );
	    }

	    queue_draw();

	    return true;
	}

	bool
	Class::on_draw(
	    const Cairo::RefPtr<Cairo::Context>& cairo 
	)
	{
	    cairo->save();

	    const ThemeColor& c_text	    = m_theme->get_color( THEME_COLOR_TEXT );
	    const ThemeColor& c_text_sel    = m_theme->get_color( THEME_COLOR_TEXT_SELECTED );
	    const ThemeColor& c_sel	    = m_theme->get_color( THEME_COLOR_SELECT );

	    guint d       = ViewMetrics.ViewPort.upper(); 
	    guint d_max   = std::min<guint>( m_model->size(), ViewMetrics.ViewPort.size() + 1 );
	    gint  ypos	  = 0;
	    gint  offset  = ViewMetrics.ViewPortPx.upper() - (d*ViewMetrics.RowHeight);
	   
	    if( offset )
	    {
		ypos  -= offset;
		d_max += 1;
	    }

#if 0
	    /* Let's see if we can save some rendering */	
	    double clip_x1, clip_y1, clip_x2, clip_y2;
	
	    cairo->get_clip_extents( clip_x1, clip_y1, clip_x2, clip_y2 );

	    if( clip_y1 > 0 && clip_y2 == ViewMetrics.ViewPortPx.lower() )
	    {
		guint d_clip = clip_y1 / ViewMetrics.RowHeight;
		ypos += d_clip * ViewMetrics.RowHeight;
		d += d_clip;
		d_max -= (d_clip-1) ;
	    }
	    else
	    if( clip_y1 == 0 && clip_y2 < ViewMetrics.ViewPortPx.lower() )
	    {
		guint d_clip = clip_y2 / ViewMetrics.RowHeight;
		d_max = d_clip+1 ;
	    }
#endif

	    guint n = 0;
	    Algorithm::Adder<guint> d_cur( d, n );

	    RowRowMapping_t::const_iterator i = m_model->iter( d_cur );

	    while( n < d_max && d_cur < m_model->size()) 
	    {
		int selected = m_selection && boost::get<Selection::INDEX>(m_selection.get()) == d_cur;

		m_columns[0]->render(
		      cairo
		    , **i
		    ,  *this
		    , d_cur 
		    , ypos
		    , ViewMetrics.RowHeight 
		    , selected ? c_text_sel : c_text
		    , selected ? c_text : c_text_sel
		    , c_sel
		    , ModelCount(m_model->size())
		    , ModelCount(m_model->m_base_model->size())
		    , selected
		    , m_model->m_constraints_albums
		);

		ypos += ViewMetrics.RowHeight;

		++n;
		++i;
	    }

	    if( !is_sensitive() )
	    {
		cairo->rectangle(0,0,get_allocated_width(),get_allocated_height());
		cairo->set_source_rgba(0,0,0,0.2);
		cairo->fill();
	    }

	    cairo->restore();
	    return true;
	}

	double
	Class::vadj_value()
	{
	    if(  property_vadjustment().get_value() )
		return property_vadjustment().get_value()->get_value();

	    return 0;
	}

	double
	Class::vadj_upper()
	{
	    if(  property_vadjustment().get_value() )
		return property_vadjustment().get_value()->get_upper();

	    return 0;
	}

	void
	Class::vadj_value_set( double v_ )
	{
	    if(  property_vadjustment().get_value() )
		return property_vadjustment().get_value()->set_value( v_ );
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
	    );

	    ModelExtents = Interval<guint>(
		  Interval<guint>::IN_EX
		, 0
		, m_model->m_mapping.size()
	    );

	    if( m_model->m_mapping.size() < ViewMetrics.ViewPort.size() )
	    {
		scroll_to_index(0);
	    }
	    else
	    {
		scroll_to_index( d );
	    }

	    queue_resize();
	}

	void
	Class::on_vadj_prop_changed()
	{
	    if( !property_vadjustment().get_value() )
		return;

	    conn_vadj.disconnect(); 

	    conn_vadj = property_vadjustment().get_value()->signal_value_changed().connect(
		sigc::mem_fun(
		    *this,
		    &Class::on_vadj_value_changed
	    ));

	    configure_vadj(
		  m_model->m_mapping.size() * ViewMetrics.RowHeight
		, ViewMetrics.ViewPortPx.size()
		, ViewMetrics.RowHeight
	    );
	}

	void
	Class::invalidate_covers()
	{
	    for( auto& i : *(m_model->m_base_model))
	    {
		i->surface_cache.clear();
	    }
	}

	void
	Class::set_show_additional_info( bool show )
	{
	    m_columns[0]->m_show_additional_info = show;
	    queue_draw();
	}

	void
	Class::select_id(
	    boost::optional<guint> id
	)
	{
	    guint d = 0;

	    for( auto& i : m_model->m_mapping )
	    {
		if( id == (*i)->album_id )
		{
		    select_index(d); 
		    return;
		}
		++d;
	    }
	}

	void
	Class::scroll_to_index(
	      guint d
	)
	{
		if( m_model->m_mapping.size() < ViewMetrics.ViewPort.size() ) 
		{
		    vadj_value_set(0);
		}
		else
		{
		    Limiter<guint> d_lim (
			  Limiter<guint>::ABS_ABS
			, 0
			, (m_model->size() * ViewMetrics.RowHeight) - ViewMetrics.ViewPort.size()
			, d * ViewMetrics.RowHeight 
		    );

		    vadj_value_set(d_lim);
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
		boost::optional<guint> id = (*m_model->m_mapping[d])->album_id;

		m_selection = boost::make_tuple( m_model->m_mapping[d], id, d );

		if( !quiet )
		{
		    m_SIGNAL_selection_changed.emit();
		}

		queue_draw();
	    }
	}

	boost::optional<guint>
	Class::get_selected_id()
	{
	    boost::optional<guint> id;

	    if( m_selection )
	    {
		id = boost::get<1>(m_selection.get());
	    }
    
	    return id;
	}

	boost::optional<guint>
	Class::get_selected_index()
	{
	    boost::optional<guint> idx;

	    if( m_selection )
	    {
		idx = boost::get<Selection::INDEX>(m_selection.get());
	    }

	    return idx; 
	}

	boost::optional<guint>
	Class::get_selected()
	{
	    return get_selected_id();
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

	    on_model_changed(0);
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
	    using boost::get;

	    Glib::ustring text = m_SearchEntry->get_text().casefold();

	    if(text.empty())
		return;

	    auto i = m_model->m_mapping.begin();

	    if( m_selection )
	    {
		std::advance( i, get<Selection::INDEX>(m_selection.get()) );
		++i;
	    }

	    auto d = std::distance( m_model->m_mapping.begin(), i );

	    for(; i != m_model->m_mapping.end(); ++i )
	    {
		Glib::ustring match = Glib::ustring((**i)->album).casefold();

		if( match.length() && match.substr(0, text.length()) == text.substr(0, text.length()))
		{
		    scroll_to_index( std::max<int>(0, d-ViewMetrics.ViewPort.size()/2));
		    select_index( d );
		    return;
		}
		++d;
	    }
	}

	void
	Class::find_prev_match()
	{
	    using boost::get;

	    Glib::ustring text = m_SearchEntry->get_text().casefold();

	    if( text.empty() )
	    {
		return;
	    }

	    auto i = m_model->m_mapping.begin();

	    if( m_selection )
	    {
		std::advance( i, get<Selection::INDEX>(m_selection.get()) );
		--i;
	    }

	    guint d = std::distance( m_model->m_mapping.begin(), i );

	    for(; i >= m_model->m_mapping.begin(); --i )
	    {
		Glib::ustring match = Glib::ustring((**i)->album).casefold();

		if( match.length() && match.substr(0, text.length()) == text.substr(0, text.length()))
		{
		    scroll_to_index( std::max<int>(0, d-ViewMetrics.ViewPort.size()/2));
		    select_index( d );
		    return;
		}
		--d;
	    }
	}

	void
	Class::on_search_entry_changed()
	{
	    using boost::get;

	    Glib::ustring text = m_SearchEntry->get_text().casefold();

	    if(text.empty())
		return;

	    guint d = 0; 

	    for( auto& i : m_model->m_mapping ) 
	    {
		Glib::ustring match = Glib::ustring((*i)->album).casefold();

		if( match.length() && match.substr(0, text.length()) == text.substr(0, text.length()))
		{
		    scroll_to_index( std::max<int>(0,d-ViewMetrics.ViewPort.size()/2));
		    select_index( d );
		    m_SearchEntry->unset_color();
		    return;
		}
		++d;
	    }

	    m_SearchEntry->override_color(Util::make_rgba(1.,0.,0.,1.));
	}

	void
	Class::on_search_entry_activated()
	{
	    cancel_search();
	    m_SIGNAL_find_accepted.emit();
	}

	bool
	Class::on_search_window_focus_out(
	      GdkEventFocus* G_GNUC_UNUSED
	)
	{
	    cancel_search();
	    return false;
	}

	void
	Class::on_show_only_this_album()
	{
	    if( m_selection )
	    {
		guint d = boost::get<2>(m_selection.get()) ;

		if( ModelExtents(d))
		{
		    const Album_sp& album = m_model->row(d) ;
		    _signal_0.emit( album->mbid );
		}
	    }
	}

	void
	Class::on_show_only_this_artist()
	{
	    if( m_selection )
	    {
		guint d = boost::get<2>(m_selection.get()) ;

		if( ModelExtents(d)) 
		{
		    const Album_sp& album = m_model->row(d) ;
		    _signal_1.emit( album->mbid_artist );
		}
	    }
	}

	void
	Class::on_jump_to_selected()
	{
	    if(m_selection)
	    {
		guint index = boost::get<2>(m_selection.get()) ;
		scroll_to_index(index) ;
	    }
	}

	void
	Class::on_refetch_album_cover()
	{
	    if(!m_selection)
	    {
		return ;
	    }
	    
	    guint d = boost::get<2>(m_selection.get()) ;

	    if(ModelExtents(d)) 
	    {
		const Album_sp& album = m_model->row(d) ; 

		album->caching = true;
		m_caching.insert( album->album_id.get());
		_signal_2.emit( album->album_id.get());

		m_redraw_spinner_conn.disconnect();
		m_redraw_spinner_conn = Glib::signal_timeout().connect( sigc::mem_fun( *this, &Class::handle_redraw ), 100 );
	    }
	}

	bool
	Class::handle_redraw()
	{
	    m_columns[0]->m_image_album_loading_iter->advance();
	    queue_draw();
	    while(gtk_events_pending()) gtk_main_iteration();

	    if(m_caching.empty())
	    {
		m_redraw_spinner_conn.disconnect();
	    }

	    return !m_caching.empty();
	}

	void
	Class::handle_cover_updated( guint id )
	{
	    m_caching.erase( id );
	}

	void
	Class::cancel_search()
	{
	    focus_entry( false );
	    m_SearchWindow->hide();
	    m_search_changed_conn.block ();
	    m_SearchEntry->set_text("");
	    m_search_changed_conn.unblock ();
	    m_search_active = false;
	}

	void
	Class::on_realize()
	{
	    Gtk::DrawingArea::on_realize();
	    initialize_metrics();

	    Glib::RefPtr<Gtk::StyleContext> sc = get_parent()->get_style_context();
	    sc->context_save();
	    sc->add_class("frame");
	    GValue v = G_VALUE_INIT;
	    gtk_style_context_get_property(
		  GTK_STYLE_CONTEXT(sc->gobj())
		, "border-radius"
		, GTK_STATE_FLAG_NORMAL
		, &v
	    ); 
	    int radius = g_value_get_int(&v);
	    sc->context_restore();
	    set_rounding(radius);
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
#if 0
	    guint d = (ViewMetrics.ViewPortPx.upper() + tooltip_y) / ViewMetrics.RowHeight;

	    if(!ModelExtents(d)) return false;

	    Album_sp album = *(m_model->m_mapping[d]);

	    boost::shared_ptr<Covers> covers = services->get<Covers>("mpx-service-covers");

	    Gtk::Image * image = Gtk::manage( new Gtk::Image );

	    Glib::RefPtr<Gdk::Pixbuf> cover;

	    if( covers->fetch(
		  album->mbid
		, cover
	    ))
	    {   
		image->set( cover->scale_simple( 320, 320, Gdk::INTERP_BILINEAR));
		tooltip->set_custom( *image );
		return true;
	    }
#endif

	    return false;
	}

	void
	Class::clear_selection(
	)
	{
	    m_selection.reset();
	    m_SIGNAL_selection_changed.emit();
	    queue_draw();
	}

	void
	Class::clear_selection_quiet(
	)
	{
	    m_selection.reset();
	}

	void
	Class::set_rounding(double r)
	{
	    m_columns[0]->set_rounding(r);
	    queue_draw();
	}

	Class::Class()

	    : ObjectBase( "YoukiViewAlbums" )

	    , property_vadj(*this, "vadjustment", RPAdj(0))
	    , property_hadj(*this, "hadjustment", RPAdj(0))
	    , property_vsp(*this, "vscroll-policy", Gtk::SCROLL_NATURAL )
	    , property_hsp(*this, "hscroll-policy", Gtk::SCROLL_NATURAL )

	    , m_search_active( false )
	    , m_button_depressed( false )

	{
	    property_vadjustment().signal_changed().connect( sigc::mem_fun( *this, &Class::on_vadj_prop_changed ));

	    set_can_focus(true);

	    ModelCount = Minus<int>( 1 );

	    m_theme = services->get<IYoukiThemeEngine>("mpx-service-theme");

	    add_events(Gdk::EventMask(GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK | GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_SMOOTH_SCROLL_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_ENTER_NOTIFY_MASK ));

	    m_SearchEntry = Gtk::manage( new Gtk::Entry );
	    m_SearchEntry->show();

	    m_search_changed_conn = m_SearchEntry->signal_changed().connect(
		    sigc::mem_fun(
			  *this
			, &Class::on_search_entry_changed
	    ));

	    m_SearchEntry->signal_activate().connect(
		    sigc::mem_fun(
			  *this
			, &Class::on_search_entry_activated
	    ));

	    m_SearchWindow = new Gtk::Window( Gtk::WINDOW_POPUP );
	    m_SearchWindow->set_decorated( false );
	    m_SearchWindow->set_border_width( 4 );

	    m_SearchWindow->signal_focus_out_event().connect(
		    sigc::mem_fun(
			  *this
			, &Class::on_search_window_focus_out
	    ));

	    signal_focus_out_event().connect(
		    sigc::mem_fun(
			  *this
			, &Class::on_search_window_focus_out
	    ));

	    m_SearchWindow->add( *m_SearchEntry );
	    m_SearchEntry->show();

	    signal_query_tooltip().connect(
		sigc::mem_fun(
		      *this
		    , &Class::query_tooltip
	    ));

	    //set_has_tooltip(true);

	    m_refActionGroup = Gtk::ActionGroup::create();
	    m_refActionGroup->add( Gtk::Action::create("ContextMenu", "Context Menu"));

	    m_refActionGroup->add( Gtk::Action::create("ContextShowAlbum", "Filter by this Album"),
		sigc::mem_fun(*this, &Class::on_show_only_this_album));
	    m_refActionGroup->add( Gtk::Action::create("ContextShowArtist", "Filter by this Album Artist"),
		sigc::mem_fun(*this, &Class::on_show_only_this_artist));
	    m_refActionGroup->add( Gtk::Action::create("ContextFetchCover", "(Re-)fetch Album Cover"),
		sigc::mem_fun(*this, &Class::on_refetch_album_cover));
	    m_refActionGroup->add( Gtk::Action::create("ContextDisplayInfo", "Display Album Info (Last.fm)"),
		sigc::mem_fun(m_SIGNAL_display_album_info, &SignalVoid::emit));

	    m_refUIManager = Gtk::UIManager::create();
	    m_refUIManager->insert_action_group(m_refActionGroup);

	    std::string ui_info =
	    "<ui>"
	    "   <popup name='PopupMenu'>"
	    "       <menuitem action='ContextDisplayInfo'/>"
	    "	    <separator/>"
	    "       <menuitem action='ContextShowAlbum'/>"
	    "       <menuitem action='ContextShowArtist'/>"
	    "	    <separator/>"
	    "       <menuitem action='ContextFetchCover'/>"
	    "   </popup>"
	    "</ui>";

	    m_refUIManager->add_ui_from_string( ui_info );
	    m_pMenuPopup = dynamic_cast<Gtk::Menu*>(m_refUIManager->get_widget("/PopupMenu"));
	}
}
}
}
