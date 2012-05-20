#include "youki-view-artist.hh"

#include <glibmm/i18n.h>

namespace MPX
{
namespace View
{
namespace Artist
{
    DataModel::DataModel()
    : m_datamodel(new Model_t)
    , m_upper_bound(0)
    {
	m_ArtistImages = new ArtistImages ;
	m_ArtistImages->run() ;
	m_ArtistImages->signal_got_artist_image().connect( sigc::mem_fun( *this, &DataModel::handle_got_artist_image )) ;
    }

    void
    DataModel::handle_got_artist_image(
	  const std::string&		mbid
	, Glib::RefPtr<Gdk::Pixbuf>	icon
    )
    {
	    MBIDIterMap_t::iterator i1 = m_mbid_map.find(mbid) ;

	    if( i1 != m_mbid_map.end() )
	    {
		icon = icon->scale_simple( 80, 80, Gdk::INTERP_BILINEAR ) ; 
		
		Glib::RefPtr<Gdk::Pixbuf> icon_desaturated_1 = icon->copy() ; 
		Glib::RefPtr<Gdk::Pixbuf> icon_desaturated_2 = icon->copy() ;

		Cairo::RefPtr<Cairo::ImageSurface>
		      s1
		    , s2
		;

		icon->saturate_and_pixelate(icon_desaturated_1, 0.0, false) ;
		icon->saturate_and_pixelate(icon_desaturated_2, 0.2, false) ;

		s1 = Util::cairo_image_surface_from_pixbuf(icon_desaturated_1) ;
		s2 = Util::cairo_image_surface_from_pixbuf(icon_desaturated_2) ;

		Util::cairo_image_surface_blur( s1, 3. ) ;

		boost::get<3>(*(i1->second)) = std::move(s1) ; 
		boost::get<4>(*(i1->second)) = std::move(s2) ; 
	    }
    }

    DataModel::DataModel(
	Model_sp model
    )
    : m_datamodel(model)
    , m_upper_bound(0)
    {
	m_ArtistImages = new ArtistImages ;
	m_ArtistImages->run() ;
    }

    void
    DataModel::clear()
    {
	m_datamodel.reset() ;
    }

    bool
    DataModel::is_set()
    {
	return bool(m_datamodel);
    }

    guint
    DataModel::size()
    {
	return m_datamodel ? m_datamodel->size():0 ;
    }

    const Row_t&
    DataModel::row(guint d)
    {
	return (*m_datamodel)[d];
    }

    void
    DataModel::set_current_row(
	guint d
    )
    {
	m_upper_bound = d ;
    }

    Glib::RefPtr<Gdk::Pixbuf>
    DataModel::get_icon(
	    const std::string& mbid
	)
	{
	    Glib::RefPtr<Gdk::Pixbuf> icon = m_ArtistImages->get_image( mbid ) ;

	    if( icon )
	    { 
		return icon->scale_simple(80, 80, Gdk::INTERP_BILINEAR) ;
	    }

	    return Glib::RefPtr<Gdk::Pixbuf>(0) ; 
	}

	void
	DataModel::append_artist(
	      const std::string&	    artist
	    , const std::string&	    artist_mbid
	    , const boost::optional<guint>& artist_id
	)
	{
	    Glib::RefPtr<Gdk::Pixbuf> icon = get_icon(artist_mbid) ; 
	    
	    Cairo::RefPtr<Cairo::ImageSurface>
		  s1
		, s2
	    ;

	    if( icon )
	    {
		Glib::RefPtr<Gdk::Pixbuf> icon_desaturated_1 = icon->copy() ; 
		Glib::RefPtr<Gdk::Pixbuf> icon_desaturated_2 = icon->copy() ;

		icon->saturate_and_pixelate(icon_desaturated_1, 0.0, false) ;
		icon->saturate_and_pixelate(icon_desaturated_2, 0.2, false) ;

		s1 = Util::cairo_image_surface_from_pixbuf(icon_desaturated_1) ;
		s2 = Util::cairo_image_surface_from_pixbuf(icon_desaturated_2) ;

		Util::cairo_image_surface_blur( s1, 3. ) ;
	    }

	    Row_t r (
		  artist
		, artist_id
		, artist_mbid
		, s1 
		, s2 
	    ) ;

	    m_datamodel->push_back(r);

	    if( artist_id )
	    {
		Model_t::iterator i = m_datamodel->end();
		std::advance( i, -1 );
		m_iter_map.insert(std::make_pair(artist_id.get(), i));
		m_mbid_map.insert(std::make_pair(artist_mbid, i)) ;
	    }
	}

	void
	DataModel::insert_artist(
	      const std::string&	    artist
	    , const std::string&	    artist_mbid
	    , const boost::optional<guint>& artist_id
	)
	{
	    static OrderFunc order ;

	    Glib::RefPtr<Gdk::Pixbuf> icon = get_icon(artist_mbid) ; 
	    
	    Cairo::RefPtr<Cairo::ImageSurface>
		  s1
		, s2
	    ;

	    if( icon )
	    {
		Glib::RefPtr<Gdk::Pixbuf> icon_desaturated_1 = icon->copy() ; 
		Glib::RefPtr<Gdk::Pixbuf> icon_desaturated_2 = icon->copy() ;

		icon->saturate_and_pixelate(icon_desaturated_1, 0.0, false) ;
		icon->saturate_and_pixelate(icon_desaturated_2, 0.2, false) ;

		s1 = Util::cairo_image_surface_from_pixbuf(icon_desaturated_1) ;
		s2 = Util::cairo_image_surface_from_pixbuf(icon_desaturated_2) ;

		Util::cairo_image_surface_blur( s1, 3. ) ;
	    }

	    Row_t r (
		  artist
		, artist_id
		, artist_mbid
		, s1 
		, s2 
	    ) ;

	    Model_t::iterator i = m_datamodel->insert(
		  std::lower_bound(
			m_datamodel->begin()
		      , m_datamodel->end()
		      , r
		      , order
		  )
		, r
	    ) ;
    
	    if( artist_id )
	    {
		m_iter_map.insert( std::make_pair( artist_id.get(), i ));
		m_mbid_map.insert(std::make_pair(artist_mbid, i)) ;
	    }
	}

//////////////////////////

	DataModelFilter::DataModelFilter(
	    DataModel_sp model
	)
	: DataModel( model->m_datamodel )
	{
	    regen_mapping() ;
	}

	void
	DataModelFilter::set_constraints_artist(
	    const IdVector_sp& constraint
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
	DataModelFilter::clear()
	{
	    DataModel::clear() ;
	    m_mapping.clear() ;
	    m_upper_bound = 0 ;
	    m_SIGNAL__changed.emit( m_upper_bound ) ;
	}

	guint
	DataModelFilter::size()
	{
	    return m_mapping.size();
	}

	const Row_t&
	DataModelFilter::row(
	      guint d 
	)
	{
	    return *m_mapping[d] ; 
	}

	RowRowMapping_t::const_iterator
	DataModelFilter::iter(guint idx)
	{
	    RowRowMapping_t::const_iterator i  = m_mapping.begin() ;
	    std::advance( i, idx ) ;
	    return i ;
	}

	void
	DataModelFilter::append_artist(
	      const std::string&	    artist_name
	    , const std::string&	    artist_mbid
	    , const boost::optional<guint>& artist_id
	)
	{
	    DataModel::append_artist(
		  artist_name
		, artist_mbid
		, artist_id 
	    ) ;
	}

	void
	DataModelFilter::insert_artist(
	      const std::string&	    artist_name
	    , const std::string&	    artist_mbid
	    , const boost::optional<guint>& artist_id
	)
	{
	    DataModel::insert_artist(
		  artist_name
		, artist_mbid
		, artist_id
	    ) ;
	}

	void
	DataModelFilter::erase_artist(
	      guint artist_id
	)
	{
	    IdIterMap_t::iterator i = m_iter_map.find(artist_id) ;

	    if(i != m_iter_map.end())
	    {
		Model_t::iterator iter = (*i).second ;
		m_iter_map.erase(i) ;
		m_datamodel->erase(iter) ;
		m_SIGNAL__changed.emit( m_upper_bound ) ;
	    }
	}

	void
	DataModelFilter::regen_mapping(
	)
	{
	    using boost::get;

	    if(m_datamodel->empty())
		return ;

	    RowRowMapping_t new_mapping;
	    new_mapping.reserve(m_datamodel->size()) ;
	    m_upper_bound = 0 ;

	    for( auto i = m_datamodel->begin() ; i != m_datamodel->end(); ++i )
	    {
		boost::optional<guint> id = get<1>(*i) ;

		if(!m_constraints_artist || (*m_constraints_artist)[id.get()])
		{
		    new_mapping.push_back( i ) ;
		}
	    }

	    std::swap( m_mapping, new_mapping ) ;
	    m_SIGNAL__changed.emit(0) ;
	}

	void
	DataModelFilter::regen_mapping_iterative(
	)
	{
	    using boost::get;

	    if(m_mapping.empty())
		return ;

	    RowRowMapping_t new_mapping;
	    new_mapping.reserve(m_mapping.size()) ;
	    m_upper_bound = 0 ;

	    for( auto& i : m_mapping ) 
	    {
		boost::optional<guint> id = get<1>(*i) ;

		if(!m_constraints_artist || (*m_constraints_artist)[id.get()])
		{
		    new_mapping.push_back( i ) ;
		}
	    }

	    std::swap( m_mapping, new_mapping ) ;
	    m_SIGNAL__changed.emit(0) ;
	}

	void
	DataModelFilter::update_count(
	)
	{
	    guint map_size = m_mapping.size() - 1 ; // -1, because of our dummy album
	    Row_t& row = **m_mapping.begin() ;

	    if( map_size < (m_datamodel->size() - 1) )
	    {
		get<0>(row) = (boost::format(_("%u %s")) % map_size % ((map_size > 1) ? _("Artists") : _("Artist"))).str() ;
	    }
	    else
	    {
		get<0>(row) = _("All Artists") ;
	    }
	}

//////////////////////////

	Column::Column()

	    : m_width(0)
	    , m_column(0)
	    , m_alignment( Pango::ALIGN_LEFT )

	{
	    m_image_disc = Util::cairo_image_surface_from_pixbuf(
				    Gdk::Pixbuf::create_from_file(
					    Glib::build_filename( DATA_DIR, "images" G_DIR_SEPARATOR_S "artist.png" )
	    )->scale_simple(80, 80, Gdk::INTERP_BILINEAR)) ;

	    m_rect_shadow = render_rect_shadow( 80, 80 ) ;
	}

	void
	Column::set_width(guint width)
	{
	    m_width = width ;
	}

	guint
	Column::get_width()
	{
	    return m_width ;
	}

	void
	Column::set_column(guint column)
	{
	    m_column = column ;
	}

	guint
	Column::get_column()
	{
	    return m_column ;
	}

	void
	Column::set_alignment(
	    Pango::Alignment align
	)
	{
	    m_alignment = align ;
	}


	Pango::Alignment
	Column::get_alignment(
	)
	{
	    return m_alignment ;
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
	    c2->rectangle(0, 0, w, h) ;
	    c2->fill() ;
	    Util::cairo_image_surface_blur( s, 2.5 ) ;
	    return s ;
	}

	void
	Column::render_header(
	      const Cairo::RefPtr<Cairo::Context>&  cairo
	    , Gtk::Widget&			    widget
	    , int				    xpos
	    , int				    ypos
	    , const ThemeColor&			    color
	)
	{
	    cairo->move_to(
		  xpos + 6
		, ypos + 4
	    ) ;

	    Glib::RefPtr<Pango::Layout> layout = widget.create_pango_layout(_("Artists")) ;
	    layout->set_ellipsize(Pango::ELLIPSIZE_END) ;
	    layout->set_width( (m_width-12)*PANGO_SCALE ) ;
	    layout->set_alignment( m_alignment ) ;
	    Gdk::Cairo::set_source_rgba(cairo, Util::make_rgba(color,0.8));
	    layout->show_in_cairo_context( cairo ) ;
	}

	void
	Column::render(
	      const Cairo::RefPtr<Cairo::Context>&  cairo
	    , const Row_t&			    r
	    , Gtk::Widget&			    widget
	    , int				    d
	    , int				    xpos
	    , int				    ypos
	    , int				    row_height
	    , bool				    selected
	    , const ThemeColor&			    color
	)
	{
	    using boost::get;

	    cairo->save() ;

	    const int text_size_px = 15 ;
	    const int text_size_pt = static_cast<int>((text_size_px* 72)
					/ Util::screen_get_y_resolution(Gdk::Screen::get_default())) ;

	    Pango::FontDescription font_desc ;
	    font_desc = widget.get_style_context()->get_font() ;
	    font_desc.set_size( text_size_pt * PANGO_SCALE ) ;
//	    font_desc.set_weight( Pango::WEIGHT_BOLD ) ; 
	
	    Glib::RefPtr<Pango::Layout> L = widget.create_pango_layout(get<0>(r)) ;

	    L->set_font_description( font_desc ) ;

	    L->set_ellipsize(
		  Pango::ELLIPSIZE_END
	    ) ;

	    L->set_width(
		  (m_width-8) * PANGO_SCALE
	    ) ;

	    L->set_alignment( Pango::ALIGN_LEFT ) ;

	    int width, height ;
	    L->get_pixel_size( width, height ) ;

	    double h,s,b ;

	    Gdk::RGBA c3 = color ;
	    c3.set_alpha( 0.7 ) ;
	    Util::color_to_hsb( color, h, s, b ) ;
	    s *= 0.25 ;
	    b *= 1.80 ;
	    c3 = Util::color_from_hsb( h, s, b ) ;
	    c3.set_alpha( 1. ) ;

	    Util::color_to_hsb( color, h, s, b ) ;
	    s *= 0.95 ;
	    Gdk::RGBA c2 = Util::color_from_hsb( h, s, b ) ;
	    c2.set_alpha( 1. ) ;

	    /* Icon */
	    Cairo::RefPtr<Cairo::ImageSurface> surface ; 
	
	    if(selected)
		surface = boost::get<4>(r) ;
	    else
		surface = boost::get<3>(r) ;

	    int x = xpos+(m_width-80)/2. ;
	    int y = ypos+12 ;

	    if(!surface)
		surface = m_image_disc ;
//	    else
//		Util::draw_cairo_image( cairo, m_rect_shadow, x+1, y+1, 1., 0 ) ;

	    if(surface)
	    {
		cairo->save() ;
		cairo->translate( x, y ) ;
		RoundedRectangle( cairo, 0, 0, 80, 80, 3.5 ) ;
		cairo->set_source( surface, 0, 0 ) ;
		cairo->fill() ;

//		cairo->set_line_width( 1 ) ;	
//		Gdk::Cairo::set_source_rgba( cairo, Util::make_rgba(color,0.6)) ;
//		cairo->stroke() ;

		cairo->restore() ;
	    }

	    /* Label */
	    cairo->save() ;
	    cairo->translate( xpos+(m_width-width)/2., ypos+/*76*/98 ) ;

	    if( selected )
	    {
		Util::render_text_shadow( L, 0.0, 0.0, cairo ) ;
	    }

	    auto set_color_stop_rgba = 

	    [](Gdk::RGBA c, double p, double alpha, Cairo::RefPtr<Cairo::LinearGradient> gr)
	    {
		gr->add_color_stop_rgba(
		      p
		    , c.get_red()
		    , c.get_green()
		    , c.get_blue()
		    , alpha
		) ;
	    } ;

	    Cairo::RefPtr<Cairo::LinearGradient> gr = Cairo::LinearGradient::create( width/2, 0, width/2, height ) ;
	    set_color_stop_rgba( selected?color:c3, 0, 1, gr ) ;
	    set_color_stop_rgba( Util::color_shade(selected?color:c3, 0.9), 0.7, 1, gr ) ;
	    set_color_stop_rgba( Util::color_shade(selected?color:c3, 0.8), 1.0, 1, gr ) ;
	    cairo->set_source( gr ) ;
	    cairo->move_to(0.0, 0.0) ;
	    pango_cairo_show_layout(
		  cairo->cobj()
		, L->gobj()
	    ) ;
	    cairo->restore() ;

	    cairo->restore() ;
	}

//////////////////////////

	void
	Class::initialize_metrics()
	{
	    Glib::RefPtr<Pango::Context> context = get_pango_context();

	    Pango::FontMetrics metrics = context->get_metrics(
					      get_style_context()->get_font()
					    , context->get_language()
	    ) ;

	    m_height__row = 120 ;

	    const int header_pad = 5 ;

	    m_height__headers = (metrics.get_ascent()/PANGO_SCALE) +
			        (metrics.get_descent()/PANGO_SCALE) + 5 + header_pad ;
	}

	void
	Class::on_vadj_prop_changed()
	{
	    if( !property_vadjustment().get_value() )
		return ;

	    property_vadjustment().get_value()->signal_value_changed().connect(
		sigc::mem_fun(
		    *this,
		    &Class::on_vadj_value_changed
	    ));

	    configure_vadj(
		  (m_model->size() * m_height__row)
		, m_height__current_viewport
		, m_height__row
	    ) ;
	}

	void
	Class::on_vadj_value_changed()
	{
	    if( m_model->m_mapping.size() )
	    {
		m_Viewport_I = Interval<guint>(
		      Interval<guint>::IN_IN
		    , get_upper_row()
		    , get_upper_row() + (m_height__current_viewport / m_height__row)
		) ;

		m_model->set_current_row( get_upper_row() ) ;
		queue_draw() ;
	    }
	}

	inline guint
	Class::get_page_size(
	)
	{
	    if( m_height__current_viewport && m_height__row )
		return m_height__current_viewport / m_height__row ;
	    else
		return 0 ;
	}

	inline guint
	Class::get_upper_row()
	{
	    if( property_vadjustment().get_value() )
		return property_vadjustment().get_value()->get_value() / m_height__row ;

	    return 0 ;
	}

	bool
	Class::on_key_press_event(
	      GdkEventKey* event
	)
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
		m_SearchEntry->event (new_event) ;
		//gdk_event_free(new_event) ;

		return true ;
	    }

	    int step = 0 ;
	    guint d = 0 ;

	    switch( event->keyval )
	    {
		case GDK_KEY_Up:
		case GDK_KEY_KP_Up:
		case GDK_KEY_Page_Up:
		{
		    if( event->keyval == GDK_KEY_Page_Up )
		    {
			step = -(m_height__current_viewport / m_height__row) ;
		    }
		    else
		    {
			step = -1 ;
		    }

		    if( !m_selection )
		    {
			mark_first_row_up:
			select_index( get_upper_row() ) ;
		    }
		    else
		    {
			int origin = boost::get<S_INDEX>(m_selection.get()) ;

			if( origin > 0 )
			{
			    if( m_Viewport_I( origin ))
			    {
				d = std::max<int>( origin+step, 0 ) ;
				select_index( d ) ;

				double adj_value = vadj_value() ; 

				if((d * m_height__row) < adj_value )
				{
				    if( event->keyval == GDK_KEY_Page_Up )
				    {
					vadj_value_set( std::max<int>( adj_value + (step*int(m_height__row)), 0 )) ;
				    }
				    else
				    {
					scroll_to_index( d ) ;
				    }
				}
			    }
			    else
			    {
				goto mark_first_row_up ;
			    }
			}
			else
			{
			    scroll_to_index(0) ;
			}
		    }

		    return true;
		}

		case GDK_KEY_Home:
		{
		    scroll_to_index(0) ;
		    select_index(0) ;

		    return true ;
		}

		case GDK_KEY_End:
		{
		    scroll_to_index( m_model->size()-get_page_size()) ;
		    select_index( ModelCount(m_model->size())) ; 
		    return true ;
		}

		case GDK_KEY_Down:
		case GDK_KEY_KP_Down:
		case GDK_KEY_Page_Down:
		{
		    if( event->keyval == GDK_KEY_Page_Down )
		    {
			step = (m_height__current_viewport / m_height__row) ;
		    }
		    else
		    {
			step = 1 ;
		    }

		    if( !m_selection )
		    {
			mark_first_row_down:
			select_index( get_upper_row() ) ;
		    }
		    else
		    {
			int origin = boost::get<S_INDEX>(m_selection.get()) ;

			if( m_Viewport_I( origin ))
			{
			    d = std::min<int>( origin+step, ModelCount(m_model->size())) ;

			    select_index( d ) ;

			    if( event->keyval == GDK_KEY_Page_Down )
			    {
				guint new_val = vadj_value() + ( step * m_height__row ) ;

				if( new_val > ( vadj_upper() - m_height__current_viewport ))
				{
				    scroll_to_index( d ) ;
				}
				else
				{
				    vadj_value_set( vadj_value() + ( step*m_height__row )) ;
				}
			    }
			    else
			    {
				guint d_adjust  = vadj_value() / m_height__row + m_height__current_viewport / m_height__row ;
				guint diff = m_height__current_viewport - (m_height__current_viewport/m_height__row)*m_height__row ;

				if( d >= d_adjust ) 
				{
				    guint position_new = (get_upper_row()+1+(d-d_adjust)) * m_height__row - diff ; 
				    vadj_value_set( std::min<int>(vadj_upper(), position_new )) ; 
				}
			    }
		       }
			else
			{
			    goto mark_first_row_down ;
			}
		    }


		    return true ;
		}

		default:

		    if( !m_search_active )
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
	    event->focus_change.window = (*m_SearchEntry).get_window()->gobj ();
	    event->focus_change.in     = in;

	    (*m_SearchEntry).send_focus_change( event ) ;
	    (*m_SearchEntry).property_has_focus() = in;

	    //gdk_event_free( event ) ;
	}

	void
	Class::on_size_allocate( Gtk::Allocation& a )
	{
	    a.set_x(0) ;
	    a.set_y(0) ;
	    Gtk::DrawingArea::on_size_allocate( a ) ;
	    queue_draw() ;
	}

	bool
	Class::on_button_press_event( GdkEventButton* event )
	{
	    using boost::get;

	    cancel_search() ;

	    double ymod = fmod( vadj_value(), m_height__row ) ;
	    guint d = (vadj_value() + event->y - m_height__headers) / m_height__row ;

	    if( !m_selection || (get<S_INDEX>(m_selection.get()) != d))
	    {
		if( ModelExtents( d ))
		{
		    select_index( d ) ;
		}

		double Excess = get_allocation().get_height() - (get_page_size()*m_height__row) ;

		if( d == get_upper_row()) 
		{
		    vadj_value_set( std::max<int>(0, vadj_value() - ymod )) ;
		}
		else
		if( (!ymod && d == (get_upper_row()+get_page_size())))

		{
		    vadj_value_set( std::min<int>(vadj_upper(), vadj_value() + (m_height__row-ymod) - Excess )) ;
		}
		else
		if( (ymod && d > (get_upper_row()+get_page_size())))

		{
		    vadj_value_set( std::min<int>(vadj_upper(), vadj_value() + m_height__row + (m_height__row-ymod) - Excess )) ;
		}
	    }
	    else
	    if( event->button == 1 && m_selection && (get<S_INDEX>(m_selection.get()) == d))
	    {
		if( has_focus() )
		{
		    clear_selection() ;
		}
	    }

	    grab_focus() ;
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
		property_vadjustment().get_value()->set_page_increment( step_increment ) ;
	    }
	}

	bool
	Class::on_configure_event(
	    GdkEventConfigure* event
	)
	{
	    m_height__current_viewport = event->height - m_height__headers ;

	    if( m_height__row )
	    {
		m_Viewport_I = Interval<guint> (
		      Interval<guint>::IN_IN
		    , get_upper_row()
		    , get_upper_row() + (m_height__current_viewport / m_height__row)
		) ;

		configure_vadj(
		      (m_model->size() * m_height__row)
		    , m_height__current_viewport
		    , m_height__row 
		) ;
	    }

	    double column_width = (double(event->width) - m_columns__fixed_total_width - (40*m_columns__collapsed.size()) ) / double(m_columns.size()-m_columns__collapsed.size()-m_columns__fixed.size());

	    for( guint n = 0 ; n < m_columns.size() ; ++n )
	    {
		if( m_columns__fixed.count( n ) )
		{
		    continue ;
		}

		if( m_columns__collapsed.count( n ) )
		{
		    m_columns[n]->set_width( 40 ) ;
		}
		else
		{
		    m_columns[n]->set_width( column_width ) ;
		}
	    }

	    queue_draw() ;

	    return false;
	}

	void
	Class::render_header_background(
	      const Cairo::RefPtr<Cairo::Context>&  cairo
	    , guint mw
	    , guint mh
	    , const Gdk::RGBA& c_base
	    , const Gdk::RGBA& c_treelines
	)
	{
	    cairo->save() ;
	    cairo->set_operator( Cairo::OPERATOR_OVER ) ;
	    cairo->rectangle(
		  0
		, 0
		, mw 
		, mh 
	    ) ;
	
	    double h,s,b ;

	    Gdk::RGBA c1 ;
	    c1.set_rgba( c_base.get_red(), c_base.get_green(), c_base.get_blue()) ;
	    Util::color_to_hsb( c1, h, s, b ) ;
	    b *= 0.95 ; 
	    c1 = Util::color_from_hsb( h, s, b ) ;

	    Gdk::RGBA c2 ;
	    c2.set_rgba( c_base.get_red(), c_base.get_green(), c_base.get_blue()) ;
	    Util::color_to_hsb( c2, h, s, b ) ;
	    b *= 0.88 ;
	    c2 = Util::color_from_hsb( h, s, b ) ;

	    Cairo::RefPtr<Cairo::LinearGradient> gr =
		Cairo::LinearGradient::create( mw/2., 0, mw/2., mh ) ;

	    gr->add_color_stop_rgba( 0., c2.get_red(), c2.get_green(), c2.get_blue(), 1. ) ;
	    gr->add_color_stop_rgba( .65, c1.get_red(), c1.get_green(), c1.get_blue(), 1. ) ;
	    gr->add_color_stop_rgba( 1., c1.get_red(), c1.get_green(), c1.get_blue(), 1. ) ;

	    cairo->set_source( gr ) ;
	    cairo->fill() ;
	    cairo->restore() ;

	    cairo->save() ;

	    Gdk::Cairo::set_source_rgba( cairo, Util::make_rgba( c_treelines, 0.6 )) ;

	    cairo->set_antialias( Cairo::ANTIALIAS_NONE ) ;
	    cairo->set_line_width( 1. ) ;
	    cairo->move_to( 0, mh ) ;
	    cairo->line_to( mw, mh ) ;
	    cairo->stroke() ;

	    cairo->restore() ;
	}


	bool
	Class::on_draw(
	    const Cairo::RefPtr<Cairo::Context>& cairo 
	)
	{
	    cairo->save() ;

	    boost::shared_ptr<IYoukiThemeEngine> theme = services->get<IYoukiThemeEngine>("mpx-service-theme") ;

	    const ThemeColor& c_text       = theme->get_color( THEME_COLOR_TEXT ) ;
	    const ThemeColor& c_text_sel   = theme->get_color( THEME_COLOR_TEXT_SELECTED ) ;
	    const ThemeColor& c_rules_hint = theme->get_color( THEME_COLOR_BASE_ALTERNATE ) ;
	    const ThemeColor& c_bg	   = theme->get_color( THEME_COLOR_BACKGROUND ) ;
	    const ThemeColor& c_treelines  = theme->get_color( THEME_COLOR_TREELINES ) ;

	    guint d	= get_upper_row() ;
	    guint d_max = Limiter<guint>(
				    Limiter<guint>::ABS_ABS
				  , 0
				  , m_model->size()
				  , get_page_size() + 2
			  ) ;
	    guint xpos = 0 ;
	    gint  ypos = m_height__headers ;

	    int offset  = vadj_value() - (d*m_height__row) ;
	    ypos -= offset ? offset : 0 ;

	    guint n = 0 ;
	    Algorithm::Adder<guint> d_cur( d, n ) ;

	    GdkRectangle rr ;
	    rr.x = 0 ;
	    rr.width = get_allocated_width() ;
	    rr.height = m_height__row ;

	    render_header_background( cairo, get_allocated_width(), m_height__headers, c_bg, c_treelines ) ;

	    for( auto& c : m_columns )
	    {
		c->render_header(
		      cairo
		    , *this
		    , 0
		    , 0 
		    , c_text 
		) ;

		xpos += c->get_width();
	    }

	    cairo->rectangle( 0, m_height__headers, get_allocated_width(), get_allocated_height() - m_height__headers ) ;
	    cairo->clip() ;

	    while( n < d_max && ModelExtents(d_cur)) 
	    {
		auto iter = m_model->iter(d_cur) ;

		int is_selected = (m_selection && get<S_ID>(m_selection.get()) == get<S_ID>(**iter)) ;

		if( is_selected )
		{
		    rr.y = ypos ;

		    theme->draw_selection_rectangle(
			  cairo
			, rr
			, has_focus()
			, 0
			, MPX::CairoCorners::CORNERS(0)
		    ) ;
		}
#if 0
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
		    Gdk::Cairo::set_source_rgba( cairo, c_rules_hint ) ;
		    cairo->fill() ;
		}
#endif

		xpos = 0 ;
		for( auto& c : m_columns )
		{
		    c->render(
			  cairo
			, **iter
			,  *this
			, d_cur 
			, xpos
			, ypos
			, m_height__row
			, is_selected
			, is_selected ? c_text_sel : c_text
		    ) ;

		    xpos += c->get_width();
		}

		ypos += m_height__row;

		++n ;
		++iter ;
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

		theme->draw_focus(
		      cairo
		    , r
		    , has_focus()
		    , 0
		    , MPX::CairoCorners::CORNERS(0)
		) ;
	    }
*/

	    get_window()->process_all_updates() ;

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
	    guint index
	)
	{
	    configure_vadj(
		 (m_model->size() * m_height__row)
	       , m_height__current_viewport
	       , m_height__row
	    ) ;

	    ModelExtents = Interval<guint>(
		  Interval<guint>::IN_EX
		, 0
		, m_model->m_mapping.size()
	    ) ;

	    scroll_to_index( index ) ;
	    queue_resize() ;
	}

	void
	Class::select_id(
	    boost::optional<guint> id
	)
	{
	    using boost::get;

	    if( id )
	    {
		guint d = 0 ;
		for( auto& i : m_model->m_mapping )
		{
		    if( id.get() == get<1>(*i))
		    {
			select_index( d ) ; 
			return ;
		    }
		    ++d ;
		}
	    }

	    clear_selection() ;
	}

	void
	Class::select_index(
	      guint   d
	)
	{
	    if( d < m_model->size() )
	    {
		boost::optional<guint>& id = get<1>(*m_model->m_mapping[d]) ;
		m_selection = boost::make_tuple( m_model->m_mapping[d], id, d ) ;
		m_SIGNAL_selection_changed.emit() ;
		queue_draw();
	    }
	}

	void
	Class::scroll_to_index(
	      guint d
	)
	{
	    if( m_height__current_viewport && m_height__row )
	    {
		if( m_model->m_mapping.size() < get_page_size() )
		{
		    vadj_value_set(0) ;
		}
		else
		{
		    Limiter<guint> d_lim (
			  Limiter<guint>::ABS_ABS
			, 0
			, m_model->m_mapping.size() - get_page_size()
			, d
		    ) ;

		    vadj_value_set( d_lim * m_height__row ) ;
		}
	    }
	}

	boost::optional<guint>
	Class::get_selected_id()
	{
	    return get<S_ID>(m_selection.get()) ;
	}

	boost::optional<guint>
	Class::get_selected_index()
	{
	    boost::optional<guint> idx ;

	    if( m_selection )
	    {
		idx = get<S_INDEX>(m_selection.get()) ;
	    }

	    return idx ;
	}

	boost::optional<guint>
	Class::get_selected()
	{
	    boost::optional<guint> v_ ; 

	    if( m_selection )
	    {
		    v_ = boost::get<S_ID>(m_selection.get()) ;
	    }

	    return v_ ; 
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
	Class::set_model(
	      DataModelFilter_sp  model
	)
	{
	    m_model = model;

	    m_model->signal_changed().connect(
		sigc::mem_fun(
		    *this,
		    &Class::on_model_changed
	    ));

	    on_model_changed(0) ;
	    queue_resize() ;
	}

	void
	Class::append_column(
	      Column_sp   column
	)
	{
	    m_columns.push_back(column);
	}

	void
	Class::column_set_collapsed(
	      guint column
	    , bool  is_collapsed
	)
	{
	    if( is_collapsed )
	    {
		m_columns__collapsed.insert( column ) ;
		queue_resize();
		queue_draw();
	    }
	    else
	    {
		m_columns__collapsed.erase( column ) ;
		queue_resize();
		queue_draw();
	    }
	}

	void
	Class::column_set_fixed(
	      guint column
	    , bool  is_fixed
	    , guint fixed_width 
	)
	{
	    if( is_fixed )
	    {
		m_columns__fixed.insert( column ) ;
		m_columns__fixed_total_width += fixed_width ;
		m_columns[column]->set_width( fixed_width ) ;
		queue_resize() ;
		queue_draw() ;
	    }
	    else
	    {
		m_columns__fixed.erase( column ) ;
		m_columns__fixed_total_width -= m_columns[column]->get_width() ;
		queue_resize () ;
		queue_draw () ;
	    }
	}

	void
	Class::find_next_match()
	{
	    using boost::get ;

	    Glib::ustring text = m_SearchEntry->get_text().casefold() ;

	    if( text.empty() )
	    {
		return ;
	    }

	    RowRowMapping_t::iterator i = m_model->m_mapping.begin();

	    if( m_selection )
	    {
		std::advance( i, get<S_INDEX>(m_selection.get()) ) ;
		++i ;
	    }

	    guint d = std::distance( m_model->m_mapping.begin(), i ) ;

	    for( ; i != m_model->m_mapping.end(); ++i )
	    {
		Glib::ustring match = Glib::ustring(get<0>(**i)).casefold() ;

		if( match.length() && match.substr( 0, text.length()) == text.substr( 0, text.length()) )
		{
		    scroll_to_index( std::max<int>(0, d-get_page_size()/2)) ;
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

	    RowRowMapping_t::iterator i = m_model->m_mapping.begin();

	    if( m_selection )
	    {
		std::advance( i, get<S_INDEX>(m_selection.get()) ) ;
		--i ;
	    }

	    guint d = std::distance( m_model->m_mapping.begin(), i ) ;

	    for( ; i >= m_model->m_mapping.begin(); --i )
	    {
		Glib::ustring match = Glib::ustring(get<0>(**i)).casefold() ;

		if( match.length() && match.substr( 0, text.length()) == text.substr( 0, text.length()) )
		{
		    scroll_to_index( std::max<int>(0, d-get_page_size()/2)) ;
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

	    if( text.empty() )
	    {
		return ;
	    }

	    guint d = 0 ; 

	    for( auto i = m_model->m_mapping.begin() ; i != m_model->m_mapping.end(); ++i )
	    {
		Glib::ustring match = Glib::ustring(get<0>(**i)).casefold() ;

		if( match.length() && match.substr( 0, text.length()) == text.substr( 0, text.length()) )
		{
		    scroll_to_index( std::max<int>(0, d-get_page_size()/2)) ;
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
	    guint d = (vadj_value() + tooltip_y) / m_height__row ;

	    if(!ModelExtents(d)) return false ;

	    const Row_t& r = *(m_model->m_mapping[d]) ;

	    Glib::RefPtr<Gdk::Pixbuf> icon = m_model->m_ArtistImages->get_image( boost::get<2>(r)) ;

	    if( icon ) 
	    {   
		Gtk::Image * image = Gtk::manage( new Gtk::Image ) ;
		image->set( icon ) ; 
		tooltip->set_custom( *image ) ;
		return true ;
	    }

	    return false ;
	}

	Class::Class()

	    : ObjectBase( "YoukiViewArtists" )
	    , property_vadj_(*this, "vadjustment", RPAdj(0))
	    , property_hadj_(*this, "hadjustment", RPAdj(0))
	    , property_vsp_(*this, "vscroll-policy", Gtk::SCROLL_NATURAL )
	    , property_hsp_(*this, "hscroll-policy", Gtk::SCROLL_NATURAL )
	    , m_columns__fixed_total_width(0)
	    , m_search_active( false )

	{
	    m_background = Util::cairo_image_surface_from_pixbuf(
				    Gdk::Pixbuf::create_from_file(
					    Glib::build_filename( DATA_DIR, "images" G_DIR_SEPARATOR_S "album-background.png" )
	    )) ;

	    property_vadjustment().signal_changed().connect( sigc::mem_fun( *this, &Class::on_vadj_prop_changed )) ;

	    ModelCount = Minus<int>( -1 ) ;

	    signal_query_tooltip().connect(
		sigc::mem_fun(
		      *this
		    , &Class::query_tooltip
	    )) ;

	    set_has_tooltip(true) ;
	    set_can_focus (true);

	    add_events(Gdk::EventMask(GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK | GDK_BUTTON_PRESS_MASK | GDK_EXPOSURE_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_LEAVE_NOTIFY_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_BUTTON_MOTION_MASK | GDK_SCROLL_MASK ));

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
	}
}
}
}
