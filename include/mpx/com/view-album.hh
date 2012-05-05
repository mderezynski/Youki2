#ifndef YOUKI_VIEW_ALBUMS__HH
#define YOUKI_VIEW_ALBUMS__HH

#include <glibmm/i18n.h>
#include <gtkmm.h>
#include <cairomm/cairomm.h>
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/optional.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/random.hpp>
#include <cmath>

#include "mpx/util-string.hh"
#include "mpx/util-graphics.hh"

#include "mpx/mpx-types.hh"
#include "mpx/mpx-main.hh"
#include "mpx/mpx-covers.hh"

#include "mpx/algorithm/limiter.hh"
#include "mpx/algorithm/interval.hh"
#include "mpx/algorithm/vector_compare.hh"
#include "mpx/algorithm/minus.hh"
#include "mpx/algorithm/range.hh"

#include "mpx/i-youki-theme-engine.hh"

#include "mpx/widgets/cairo-extensions.hh"
#include "mpx/widgets/cairo-blur.hh"

#include "mpx/aux/glibaddons.hh"

#include "mpx/com/indexed-list.hh"
#include "mpx/com/viewmetrics.hh"

#include "mpx/mpx-main.hh"

namespace
{
    typedef Glib::RefPtr<Gtk::Adjustment>		    RPAdj ;
    typedef Glib::Property<RPAdj>			    PropAdjustment ;
    typedef Glib::Property<Gtk::ScrollablePolicy>	    PropScrollPolicy ;
}

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

    const char* rt_string[] = { "ALBUM", "SINGLE", "COMPIL.", "EP", "LIVE", "REMIX", "OST", "  ?  " } ;

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
        const double rounding = 2. ;

	enum RTViewMode
	{
		RT_VIEW_NONE
	     ,  RT_VIEW_BOTTOM
	     ,  RT_VIEW_STRIPE
	     ,  RT_VIEW_RANDOM
	} ;

        struct Album
        {
            Cairo::RefPtr<Cairo::ImageSurface>      coverart ;
            Cairo::RefPtr<Cairo::ImageSurface>      surface_cache ;
            guint                                   album_id ;
            guint                                   artist_id ;
            std::string                             album ;
            std::string                             album_artist ;
            std::string                             type ;
            std::string                             year ;
            std::string                             mbid ;
            std::string                             mbid_artist ;
            std::string                             label ;
            guint                                   track_count ;
            guint				    track_count_release_total ;
	    guint				    discs_count ;
            guint				    insert_date ;
            gdouble                                 album_playscore ;
            int					    total_time ;
            bool				    caching ;

	    bool operator==( const Album& other )
	    {
		return other.album_id == album_id ;
	    }

	    bool operator!=( const Album& other )
	    {
		return other.album_id == album_id ;
	    }

	    Album()
	    : coverart( Cairo::RefPtr<Cairo::ImageSurface>(0))
	    , surface_cache( Cairo::RefPtr<Cairo::ImageSurface>(0))
	    , caching(false)
	    {}
        };

        typedef boost::shared_ptr<Album>		    Album_sp ;

        typedef IndexedList<Album_sp>                       Model_t ;
        typedef boost::shared_ptr<Model_t>                  Model_sp_t ;

        typedef std::map<guint, Model_t::iterator>          IdIterMap_t ;
        typedef std::vector<Model_t::iterator>              RowRowMapping_t ;

	bool operator==( const Album_sp& a, const Album_sp& b )
	{
	    return a->album_id == b->album_id ;
	}

	bool operator!=( const Album_sp& a, const Album_sp& b )
	{
	    return a->album_id != b->album_id ;
	}

	typedef sigc::signal<void>		    Signal_0 ;
	typedef sigc::signal<void, guint>	    Signal_1 ;

        struct OrderFunc
        : public std::binary_function<Album_sp, Album_sp, bool>
	{
	    bool operator() (const Album_sp& a,
			     const Album_sp& b)
	    {
		if( a->album_id == -1 )
		    return true ;

		if( b->album_id == -1 )
		    return false ;

		if( a->album_artist < b->album_artist )
		    return true ;

		if( b->album_artist < a->album_artist )
		    return false ;

		if( a->year < b->year )
		    return true ;

		if( b->year < a->year )
		    return false ;

		if( a->album < b->album )
			return true ;

		if( b->album < a->album )
			return false ;

		return false ;
	    }
	} ;

	struct DataModel
        : public sigc::trackable
	{
		Model_sp_t             m_realmodel ;
		IdIterMap_t            m_iter_map ;
		guint		       m_upper_bound ;

		Signal_1	       m_SIGNAL__changed ;
		Signal_0	       m_SIGNAL__redraw ;
		Signal_1	       m_SIGNAL__cover_updated ;

/*
		typedef std::vector<Model_t::iterator>	ModelIterVec_t ;
		typedef IndexedList<ModelIterVec_t>	ArtistAlbumMapping_t ;
		ArtistAlbumMapping_t			m_artist_album_mapping ;
 */

		DataModel()
		: m_upper_bound( 0 )
		{
		    m_realmodel = Model_sp_t(new Model_t);
		}

		DataModel(Model_sp_t model)
		: m_upper_bound( 0 )
		{
		    m_realmodel = model;
		}

		virtual void
		clear()
		{
		    m_realmodel->clear() ;
		    m_iter_map.clear() ;
		    m_upper_bound = 0 ;
		}

		virtual Signal_1&
		signal_changed ()
		{
			return m_SIGNAL__changed ;
		}

		virtual Signal_0&
		signal_redraw ()
		{
			return m_SIGNAL__redraw ;
		}

		virtual Signal_1&
		signal_cover_updated ()
		{
		    return m_SIGNAL__cover_updated ;
		}

		virtual bool
		is_set ()
		{
		    return bool(m_realmodel) ;
		}

		virtual guint
		size ()
		{
		    return m_realmodel ? m_realmodel->size() : 0 ;
		}

		virtual Album_sp
		row(guint row)
		{
		    return (*m_realmodel)[row] ;
		}

		virtual void
		set_current_row(guint row)
		{
		    m_upper_bound = row ;
		}

		virtual void
		append_album(const Album_sp album)
		{
		    m_realmodel->push_back( album ) ;
		    Model_t::iterator i = m_realmodel->end() ;
		    std::advance( i, -1 ) ;
		    m_iter_map.insert( std::make_pair( album->album_id, i )) ;

		    /*
		      if( album->album_id != -1 )
		      {
			  guint artist_id = album->artist_id ;
			  ModelIterVec_t& v = m_artist_album_mapping[artist_id] ;
			  v.push_back( i ) ;
		      }
 */
		}

		virtual void
		insert_album(const Album_sp album)
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

		    /*
			    if( album->album_id != -1 )
			    {
				guint artist_id = album->artist_id ;
				ModelIterVec_t& v = m_artist_album_mapping[artist_id] ;
				v.push_back( i ) ;
			    }
 */
		}

		void
		erase_album(guint id_album)
		{
		    IdIterMap_t::iterator i = m_iter_map.find( id_album ) ;

		    if( i != m_iter_map.end() )
		    {
			m_realmodel->erase( i->second );
			m_iter_map.erase( id_album );
		    }
		}

		void
		update_album(
		    const Album_sp album
		)
		{
		    if( album && m_iter_map.find( album->album_id ) != m_iter_map.end() )
		    {
			*(m_iter_map[album->album_id]) = album ;
			m_SIGNAL__redraw.emit() ;
		    }
		}

		void
		update_album_cover_cancel(
		      guint id
		)
		{
		    IdIterMap_t::iterator i = m_iter_map.find( id ) ;

		    if( i != m_iter_map.end())
		    {
			Album_sp & album = *i->second ;

			if( album )
			{
			    album->caching = false ;

			    m_SIGNAL__cover_updated.emit( id ) ;
			    m_SIGNAL__redraw.emit() ;
			}
		    }
		}

		void
		update_album_cover(
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
	};

        typedef boost::shared_ptr<DataModel> DataModel_sp_t;

        struct DataModelFilter
        : public DataModel
        {
            typedef std::vector<guint>                     IdVector_t ;
            typedef boost::shared_ptr<IdVector_t>          IdVector_sp ;

            public:

                RowRowMapping_t   m_mapping ;
                TCVector_sp       m_constraints_albums ;
                IdVector_sp       m_constraints_artist ;

/*
	        boost::optional<guint> m_constraint_single_artist ;
 */

            public:

                DataModelFilter(
                      DataModel_sp_t model
                )
		: DataModel( model->m_realmodel )
                {
                    regen_mapping() ;
                }

                virtual
                ~DataModelFilter()
                {
                }

                virtual void
                set_constraints_albums(
                    TCVector_sp&  constraint
                )
                {
                    if( constraint != m_constraints_albums )
                    {
                        m_constraints_albums = constraint ;
		    }    
                }

                virtual void
                clear_constraints_album(
                )
                {
                    m_constraints_albums.reset() ;
                }

                virtual void
                set_constraints_artist(
                    IdVector_sp& constraint
                )
                {
                    if( constraint != m_constraints_artist )
                    {
                        m_constraints_artist = constraint ;
                    }
                }

                virtual void
                clear_constraints_artist(
                )
                {
                    m_constraints_artist.reset() ;
                }

		virtual void
		clear_all_constraints_quiet()
		{
		    m_constraints_artist.reset() ;
		    m_constraints_albums.reset() ;
/*
		    m_constraint_single_artist.reset() ;
 */
		}

		virtual void
		set_constraint_single_artist( guint id )
		{
/*
		    m_constraint_single_artist = id ;
 */
		}

                virtual void
                clear()
                {
                    DataModel::clear() ;
                    m_mapping.clear() ;
		    m_upper_bound = 0 ;

		    m_SIGNAL__redraw.emit() ;
                }

                virtual guint
                size()
                {
                    return m_mapping.size();
                }

                virtual Album_sp
                row(
                      guint   row
                )
                {
                    return *(m_mapping[row]);
                }

                virtual RowRowMapping_t::const_iterator
                iter(
                      guint   row
                )
                {
                    RowRowMapping_t::const_iterator i = m_mapping.begin() ;
                    std::advance( i, row ) ;
                    return i ;
                }

                void
                swap(
                      guint   p1
                    , guint   p2
                )
                {
                    std::swap( m_mapping[p1], m_mapping[p2] ) ;

		    m_SIGNAL__redraw.emit() ;
                }

                virtual void
                append_album(
                    const Album_sp album
                )
                {
                    DataModel::append_album( album ) ;
                }

                void
                erase_album(
                      guint id_album
                )
                {
                    DataModel::erase_album( id_album );
                }

                virtual void
                insert_album(
                    const Album_sp album
                )
                {
                    DataModel::insert_album(
                        album
                    ) ;
                }

                virtual void
                update_album(
                    const Album_sp album
                )
                {
                    DataModel::update_album( album ) ;

                    regen_mapping() ;
                }

                virtual void
                regen_mapping(
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

                    typedef Model_t::iterator Iter ;
                    Iter i = m_realmodel->begin() ;

                    new_mapping.push_back( i++ ) ;

		    if( (!m_constraints_albums || m_constraints_albums->empty()) && (!m_constraints_artist || m_constraints_artist->empty()))
		    {
/*
			if( m_constraint_single_artist )
			{
				const ModelIterVec_t& v = m_artist_album_mapping[m_constraint_single_artist.get()] ;

				for( ModelIterVec_t::const_iterator i = v.begin() ; i != v.end() ; ++i )
				{
				    new_mapping.push_back( *i ) ;
				}
			}
			else
 */
			{
			    for( ; i != m_realmodel->end() ; ++i )
			    {
				new_mapping.push_back( i ) ;
			    }
			}
		    }
		    else
		    {
			    TCVector_t * constraints_albums = m_constraints_albums.get() ;
			    //IdVector_t * constraints_artist = m_constraints_artist.get() ;

			    for( ; i != m_realmodel->end(); ++i )
			    {
				int truth =
					    (!constraints_albums || ((*constraints_albums)[(*i)->album_id].Count > 0))
					/*				    &&
					    (!constraints_artist || ((*constraints_artist)[(*i)->artist_id] > 0)) */
				;

				if( truth )
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

                virtual void
                regen_mapping_iterative(
                )
                {
                    using boost::get;

                    if( m_realmodel->empty() )
                    {
                        return ;
                    }

                    RowRowMapping_t new_mapping ;
                    new_mapping.reserve( m_mapping.size() ) ;

                    m_upper_bound = 0 ;

                    typedef RowRowMapping_t::const_iterator Iter ;

                    Iter i = m_mapping.begin() ;

                    new_mapping.push_back( *i ) ;
		    ++i ;

		    TCVector_t * constraints_albums = m_constraints_albums.get() ;
		    //IdVector_t * constraints_artist = m_constraints_artist.get() ;

		    for( ; i != m_mapping.end(); ++i )
		    {
			int truth =
				    (!constraints_albums || ((*constraints_albums)[(**i)->album_id].Count > 0))
					/*	  &&
				    (!constraints_artist || ((*constraints_artist)[(**i)->artist_id] > 0)) */
			;

			if( truth )
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
        };

        typedef boost::shared_ptr<DataModelFilter> DataModelFilter_sp_t;

        class Column
        {
	    public:

                guint m_width ;
                guint m_column ;
		RTViewMode  m_rt_viewmode ;
		bool	    m_show_release_label ;

                Cairo::RefPtr<Cairo::ImageSurface>  m_image_disc ;
                Cairo::RefPtr<Cairo::ImageSurface>  m_image_new ;
		Glib::RefPtr<Gdk::PixbufAnimation>  m_image_album_loading ;
		Glib::RefPtr<Gdk::PixbufAnimationIter> m_image_album_loading_iter ;

                Column(
                )
                    : m_width( 0 )
                    , m_column( 0 )
		    , m_rt_viewmode( RT_VIEW_BOTTOM )
		    , m_show_release_label( true )
                {
                    m_image_disc = Util::cairo_image_surface_from_pixbuf( Gdk::Pixbuf::create_from_file( Glib::build_filename( DATA_DIR, "images" G_DIR_SEPARATOR_S "disc.png" ))->scale_simple(64, 64, Gdk::INTERP_BILINEAR)) ;
                    m_image_new = Util::cairo_image_surface_from_pixbuf( Gdk::Pixbuf::create_from_file( Glib::build_filename( DATA_DIR, "images" G_DIR_SEPARATOR_S "new.png" ))) ;
                    m_image_album_loading = Gdk::PixbufAnimation::create_from_file( Glib::build_filename( DATA_DIR, "images" G_DIR_SEPARATOR_S "album-cover-loading.gif" )) ;

		    m_image_album_loading_iter = m_image_album_loading->get_iter( NULL ) ;
                }

                ~Column ()
                {
                }

                void
                set_width (int width)
                {
                    m_width = width;
                }

                int
                get_width ()
                {
                    return m_width;
                }

                void
                set_column (int column)
                {
                    m_column = column;
                }

                int
                get_column ()
                {
                    return m_column;
                }

		Cairo::RefPtr<Cairo::ImageSurface>
		render_icon(
                      const Album_sp                        album
		    , Gtk::Widget&			    widget
		    , RTViewMode			    rt_viewmode
		)
		{
                    using boost::get;

		    Cairo::RefPtr<Cairo::ImageSurface> s = Cairo::ImageSurface::create( Cairo::FORMAT_ARGB32, 68, 68 ) ;
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
			, 64
			, 64
			, rounding 
		    ) ;
		    cairo->fill() ;

		    if( album->coverart )
		    {
			cairo->set_source_rgba(
			      0
			    , 0
			    , 0
			    , 0.70
			) ;

			RoundedRectangle(
			      cairo
			    , 2
			    , 2
			    , 64
			    , 64
			    , rounding 
			) ;
			cairo->set_line_width( (!album->type.empty()) ? 1.25 : 0.75 ) ;
			cairo->stroke() ;
		    }

		    // Render release type if not album or unknown
		    ReleaseType rt = get_rt( album->type ) ;

		    if( !rt_viewmode == RT_VIEW_NONE && !album->type.empty() && rt != RT_OTHER /*&& rt != RT_ALBUM*/ )
		    {
		    	std::string release_type = rt_string[rt] ;

			const int text_size_px = 10 ;
			const int text_size_pt = static_cast<int> ((text_size_px * 72) / Util::screen_get_y_resolution (Gdk::Screen::get_default ())) ;

			int width, height;

			Pango::FontDescription font_desc =  widget.get_style_context()->get_font() ;
			font_desc.set_size( text_size_pt * PANGO_SCALE ) ;
			font_desc.set_weight( Pango::WEIGHT_BOLD ) ;

			Glib::RefPtr<Pango::Context> ctx = widget.get_pango_context() ;
			Glib::RefPtr<Pango::Layout> layout = Pango::Layout::create( ctx ) ;

			layout->set_font_description( font_desc ) ;
			layout->set_text( release_type ) ;
			layout->get_pixel_size( width, height ) ;

			cairo->save() ;

			if( !(width && width > 0) || !(height && height > 0))
			    return s ;

			Cairo::RefPtr<Cairo::ImageSurface> s2 = Cairo::ImageSurface::create( Cairo::FORMAT_ARGB32, width, height ) ;
			Cairo::RefPtr<Cairo::Context> c2 = Cairo::Context::create( s2 ) ;

			if( s2 && c2 )
			{
				c2->set_operator( Cairo::OPERATOR_CLEAR ) ;
				c2->paint() ;
				c2->set_operator( Cairo::OPERATOR_OVER ) ;
				c2->move_to( 0, 0 ) ;
				c2->set_source_rgba(
				      1.
				    , 1.
				    , 1.
				    , 1.
				) ;
				pango_cairo_show_layout(c2->cobj(), layout->gobj()) ;

				if( rt_viewmode == RT_VIEW_RANDOM )
				{
				    boost::mt19937 rng ;
				    boost::uniform_int<> rtmap (1,2) ;

				    rt_viewmode = RTViewMode( rtmap(rng)) ;
				}

				double x, y ;

				switch( rt_viewmode )
				{
				    case RT_VIEW_NONE:
				    case RT_VIEW_RANDOM:
					break ;

				    case RT_VIEW_BOTTOM:
				    {
					/// RECTANGULAR; AT BOTTOM
					if( album->coverart )
					{
					    cairo->rectangle(
						  2+0
						, 2+50
						, 64
						, 14
					    ) ;
					    cairo->clip() ;
					}

					RoundedRectangle(
					      cairo
					    , 2	  + 0
					    , 2   + (album->coverart?46:50)
					    , 64
					    , 18  - (album->coverart?0:4)
					    , rounding 
					) ;

					cairo->set_operator( Cairo::OPERATOR_OVER ) ;
					cairo->set_source_rgba( 0., 0., 0., 0.60 ) ;
					cairo->fill() ;

					x = 2+((64 - width)/2) ;
					y = 2+50+((12 - height)/2) + 1 ;

					break ;
				    }

				    case RT_VIEW_STRIPE:
				    {
					/// STRIPE ACROSS
					double degrees = -45 ;
					double w = width+6 ;
					double h = (w/2) * std::tan( (M_PI*std::abs(degrees)) / 180. ) ;

					RoundedRectangle(
					      cairo
					    , 2  + 1
					    , 2  + 1
					    , 64 - 2
					    , 64 - 2
					    , rounding 
					) ;
					cairo->clip() ;

					cairo->rotate_degrees( degrees ) ;
					cairo->rectangle(
					      2  - w*1.5
					    , 2  + h + 3
					    , 64 + w*1.5
					    , height - 1
					) ;
					cairo->set_operator( Cairo::OPERATOR_ATOP ) ;
					cairo->set_source_rgba( 0., 0., 0., 0.65 ) ;
					cairo->fill() ;

					x = (2-width/2.) - 3; y = 2+h+1 ;

					break ;
				    }

				    default:
					break ;
				}

				cairo->rectangle(
				      x
				    , y
				    , width
				    , height
				) ;
				cairo->set_operator( Cairo::OPERATOR_OVER ) ;
				cairo->set_source( s2, x, y ) ;
				cairo->fill() ;

				cairo->restore() ;
			}
		    }

		    return s ;
		}

		void
		render_text_shadow(	
		      Glib::RefPtr<Pango::Layout>	   layout
		    , guint w
		    , guint h
		    , guint x
		    , guint y
		    , const Cairo::RefPtr<Cairo::Context>& cairo
		)
		{
		    int width, height ;
		    layout->get_pixel_size( width, height ) ;

		    Cairo::RefPtr<Cairo::ImageSurface> s = Cairo::ImageSurface::create( Cairo::FORMAT_ARGB32, w+30, h+30 ) ;
		    Cairo::RefPtr<Cairo::Context> c2 = Cairo::Context::create( s ) ;

		    c2->set_operator( Cairo::OPERATOR_CLEAR ) ;
		    c2->paint() ;

		    c2->set_operator( Cairo::OPERATOR_OVER ) ;
		    c2->set_source_rgba(
			      0. 
			    , 0. 
			    , 0.
			    , 0.45
		    ) ;
		    c2->move_to(
			       1.5
			     , 1.5
		    ) ;
		    pango_cairo_show_layout(
			  c2->cobj()
			, layout->gobj()
		    ) ;

		    Util::cairo_image_surface_blur( s, 1. ) ; 
		    cairo->move_to(
			  x 
			, y 
		    ) ;
		    cairo->set_source( s, x, y ) ;
		    cairo->rectangle( x, y, width+4, height+4) ;
		    cairo->fill() ;
		}

		void
		render_rect_shadow(	
		      guint x
		    , guint y
		    , guint w
		    , guint h
		    , double r
		    , const Cairo::RefPtr<Cairo::Context>& cairo
		)
		{
		    Cairo::RefPtr<Cairo::ImageSurface> s = Cairo::ImageSurface::create( Cairo::FORMAT_ARGB32, w+30,h+30 ) ; 
		    Cairo::RefPtr<Cairo::Context> c2 = Cairo::Context::create( s ) ;

		    c2->set_operator( Cairo::OPERATOR_CLEAR ) ;
		    c2->paint() ;

		    c2->set_operator( Cairo::OPERATOR_OVER ) ;
		    c2->set_source_rgba(
			      0 
			    , 0 
			    , 0 
			    , 0.1
		    ) ;
		    RoundedRectangle( c2, 4, 5, w, h, rounding ) ;
		    c2->fill() ;
		    Util::cairo_image_surface_blur( s, 1 ) ;
		    cairo->move_to(
			  x 
			, y 
		    ) ;
		    cairo->set_source( s, x, y ) ;
		    cairo->rectangle( x, y, w+6, h+6) ;
		    cairo->fill() ;
		}

                void
                render(
                      const Cairo::RefPtr<Cairo::Context>&  cairo
                    , Album_sp				    album
                    , Gtk::Widget&                          widget // FIXME: Do we still need this for the Pango style context in Gtk3?
                    , guint				    row
                    , guint				    xpos
                    , int                                   ypos
                    , guint				    row_height
                    , ThemeColor	                    color
                    , guint				    model_mapping_size
		    , guint				    model_size
		    , bool				    selected
		    , TCVector_sp&			    album_constraints
                )
                {
                    using boost::get;

		    if( !album )
			return ;

                    GdkRectangle r ;
                    r.y = ypos + 4 ;

		    cairo->set_operator( Cairo::OPERATOR_ATOP ) ;

                    if( row > 0 )
                    {
               		r.x = 5 ;

			if( !album->caching )
			{
			    if( !album->surface_cache ) 
				    album->surface_cache = render_icon( album, widget, m_rt_viewmode ) ;

			    if( album->coverart && selected )
			    {
				render_rect_shadow( r.x, r.y, 64, 64, 4., cairo ) ;
			    }

			    cairo->set_source( album->surface_cache, r.x, r.y ) ;
			    cairo->rectangle( r.x, r.y , 68, 68 ) ;
			    cairo->fill() ;
			}
			else
			{
			    Glib::RefPtr<Gdk::Pixbuf> pb = m_image_album_loading_iter->get_pixbuf() ;
			    Gdk::Cairo::set_source_pixbuf( cairo, pb, r.x+24, r.y+24 ) ;
			    cairo->rectangle( r.x+24, r.y+24, 20, 20 ) ;
			    cairo->fill() ;
			}
                    }

		    Gdk::RGBA c1, c2 ;
		    double h,s,b ;

		    Util::color_to_hsb( color, h, s, b ) ;
		    s *= 0.45 ;
		    c1 = Util::color_from_hsb( h, s, b ) ;

		    Util::color_to_hsb( color, h, s, b ) ;
		    s *= 0.95 ;
		    c2 = Util::color_from_hsb( h, s, b ) ;
		    c2.set_alpha( 1. ) ;

                    enum { L1, L2, L3, N_LS } ;

                    const int text_size_px[N_LS] = { 15, 17, 12 } ;
                    const int text_size_pt[N_LS] = {   static_cast<int> ((text_size_px[L1] * 72)
                                                            / Util::screen_get_y_resolution (Gdk::Screen::get_default ()))
                                                     , static_cast<int> ((text_size_px[L2] * 72)
                                                            / Util::screen_get_y_resolution (Gdk::Screen::get_default ()))
                                                     , static_cast<int> ((text_size_px[L3] * 72)
                                                            / Util::screen_get_y_resolution (Gdk::Screen::get_default ())) } ;

                    int width, height;

                    Pango::FontDescription font_desc[N_LS] ;

                    Glib::RefPtr<Pango::Layout> layout[N_LS] = { Glib::wrap( pango_cairo_create_layout( cairo->cobj() )),
                                                                 Glib::wrap( pango_cairo_create_layout( cairo->cobj() )),
                                                                 Glib::wrap( pango_cairo_create_layout( cairo->cobj() )) } ;

                    font_desc[L1] = widget.get_style_context()->get_font() ;
                    font_desc[L1].set_size( text_size_pt[L1] * PANGO_SCALE ) ;
                    font_desc[L1].set_weight( Pango::WEIGHT_BOLD ) ;
		    font_desc[L1].set_stretch( Pango::STRETCH_EXTRA_CONDENSED ) ;

                    layout[L1]->set_font_description( font_desc[L1] ) ;
                    layout[L1]->set_ellipsize( Pango::ELLIPSIZE_END ) ;
                    layout[L1]->set_width(( m_width - 90 ) * PANGO_SCALE ) ;

                    if( row > 0 )
                    {
			xpos += 74 ; 
			guint yoff = 0 ;

			font_desc[L2] = widget.get_style_context()->get_font() ;
			font_desc[L2].set_size( text_size_pt[L2] * PANGO_SCALE ) ;
			font_desc[L2].set_weight( Pango::WEIGHT_BOLD ) ;
			font_desc[L2].set_stretch( Pango::STRETCH_EXTRA_CONDENSED ) ;

			font_desc[L3] = widget.get_style_context()->get_font() ;
			font_desc[L3].set_size( text_size_pt[L3] * PANGO_SCALE ) ;

			layout[L2]->set_font_description( font_desc[L2] ) ;
			layout[L2]->set_ellipsize( Pango::ELLIPSIZE_END ) ;
			layout[L2]->set_width(( m_width - 90 ) * PANGO_SCALE ) ;

			layout[L3]->set_font_description( font_desc[L3] ) ;
			layout[L3]->set_ellipsize( Pango::ELLIPSIZE_END ) ;
			layout[L3]->set_width(( m_width - 90 ) * PANGO_SCALE ) ;

			layout[L1]->set_width(( m_width - 90 ) * PANGO_SCALE ) ;

			/* ARTIST */
			layout[L1]->set_text( album->album_artist ) ;
			layout[L1]->get_pixel_size( width, height ) ;

			if( selected )
			{
			    render_text_shadow( layout[L1], m_width-90, height, xpos+8, r.y+yoff, cairo ) ;
			}

			cairo->move_to(
			      xpos + 8
			    , r.y + yoff
			) ;

			Gdk::Cairo::set_source_rgba(cairo, Util::make_rgba(c1,.8)) ;
			pango_cairo_show_layout(cairo->cobj(), layout[L1]->gobj()) ;

			/* ALBUM */
			yoff = 17 ;

			layout[L2]->set_text( album->album )  ;
			layout[L2]->get_pixel_size( width, height ) ;

			if( selected )
			{
			    render_text_shadow( layout[L2], m_width-90, height, xpos+8, r.y+yoff, cairo ) ;
			}

			cairo->move_to(
			      xpos + 8
			    , r.y + yoff
			) ;

			Gdk::Cairo::set_source_rgba(cairo, color) ;
			pango_cairo_show_layout(cairo->cobj(), layout[L2]->gobj()) ;

			/* YEAR + LABEL */
			guint sx = xpos + 8 ;

			if( !album->year.empty() )
			{
			    font_desc[L3].set_weight( Pango::WEIGHT_BOLD ) ;
			    layout[L3]->set_font_description( font_desc[L3] ) ;

			    layout[L3]->set_text(album->year.substr(0,4)) ;
			    layout[L3]->get_pixel_size( width, height ) ;

			    if( selected )
			    {
				render_text_shadow( layout[L3], 50, height, sx, r.y+row_height-height-9, cairo) ; 
			    }

			    cairo->move_to(
				  sx
				, r.y + row_height - height - 9
			    ) ;

			    Gdk::Cairo::set_source_rgba(cairo, c2) ;
			    pango_cairo_show_layout(cairo->cobj(), layout[L3]->gobj()) ;

			    sx += width + 3 ;
			}

			if( !album->label.empty() )
			{
			    font_desc[L3].set_weight( Pango::WEIGHT_NORMAL ) ;
			    layout[L3]->set_font_description( font_desc[L3] ) ;

			    layout[L3]->set_text( album->label ) ;
			    layout[L3]->set_width((m_width-sx) * PANGO_SCALE ) ;
			    layout[L3]->set_ellipsize( Pango::ELLIPSIZE_NONE ) ;

			    layout[L3]->get_pixel_size( width, height ) ;

			    if( selected )
			    {
				render_text_shadow( layout[L3], m_width-108, height, sx, r.y+row_height-height-9, cairo) ; 
			    }

			    cairo->move_to(
				  sx
				, r.y + row_height - height - 9 
			    ) ;

			    Gdk::Cairo::set_source_rgba(cairo, c2) ;
			    pango_cairo_show_layout(cairo->cobj(), layout[L3]->gobj()) ;
			}

			/* DISC TIME AND TRACK COUNT */
			if( m_show_release_label )
			{
			    font_desc[L3].set_weight( Pango::WEIGHT_NORMAL ) ;
			    layout[L3]->set_font_description( font_desc[L3] ) ;

			    layout[L3]->set_width((m_width-108)*PANGO_SCALE) ; 
			    layout[L3]->set_ellipsize( Pango::ELLIPSIZE_NONE ) ;

			    guint tm = 0 ;

			    std::string out, tmp, timestr ;

			    if( album_constraints )
			    {
				tm = ((*album_constraints)[album->album_id]).Time ;
				guint totaltracks = ((*album_constraints)[album->album_id]).Count ;

				if( totaltracks != album->track_count )
				    tmp = ((boost::format("<b>%u</b> / <b>%u</b> %s") % totaltracks % album->track_count % ((album->track_count>1) ? "Tracks" : "Track")).str()) ;
				else
				    tmp = ((boost::format("<b>%u</b> %s") % totaltracks % ((totaltracks>1) ? "Tracks" : "Track")).str()) ;
			    }
			    else
			    {
				tm = album->total_time ;
				guint totaltracks = album->track_count ;

				tmp = ((boost::format("<b>%u</b> %s") % totaltracks % ((totaltracks>1) ? "Tracks" : "Track")).str()) ;
			    }

			    if( album->discs_count > 1 && !album_constraints )
			    { 
				out += ((boost::format("<b>%u</b> Discs, ") % album->discs_count).str()) ;
			    }

			    out += tmp ;

			    /* Playtime */
			    guint hrs = tm / 3600 ;
			    guint min = (tm-hrs*3600) / 60 ; 
			    guint sec = tm % 60 ;
			    timestr = ((boost::format("<b>%02u:%02u:%02u</b>") % hrs % min % sec).str()) ;
			    layout[L3]->set_markup( timestr ) ;
			    layout[L3]->get_pixel_size( width, height ) ;
			    if( selected )
			    {
				render_text_shadow( layout[L3], m_width-108, height, xpos+8+5, r.y+row_height-height-25, cairo) ; 
			    }
			    cairo->move_to(
				  xpos+8+5 
				, r.y+row_height-height-25 
			    ) ;
			    Gdk::Cairo::set_source_rgba(cairo, c2) ;
			    pango_cairo_show_layout(cairo->cobj(), layout[L3]->gobj()) ;

			    Gdk::Cairo::set_source_rgba(cairo, Util::make_rgba(c2.get_red(),c2.get_green(),c2.get_blue(),0.10)) ;
			    RoundedRectangle( cairo, xpos+8, r.y+row_height-height-25, 51, 12, 2.) ; 
			    cairo->fill() ;

			    xpos += width + 14 ;				

			    /* Discs, Tracks */
			    layout[L3]->set_markup( out ) ;
			    layout[L3]->get_pixel_size( width, height ) ;
			    if( selected )
			    {
				render_text_shadow( layout[L3], m_width-108, height, xpos+8, r.y+row_height-height-25, cairo) ; 
			    }
			    cairo->move_to(
				  xpos+8 
				, r.y+row_height-height-25 
			    ) ;
			    Gdk::Cairo::set_source_rgba(cairo, c2) ;
			    pango_cairo_show_layout(cairo->cobj(), layout[L3]->gobj()) ;
			}
		    }
		    else /* ALBUM COUNT ONLY */
		    {
			font_desc[L1] = widget.get_style_context()->get_font() ;
			font_desc[L1].set_size( text_size_pt[L1] * PANGO_SCALE * 1.5 ) ;
			font_desc[L1].set_weight( Pango::WEIGHT_BOLD ) ;

			layout[L1]->set_font_description( font_desc[L1] ) ;
			layout[L1]->set_ellipsize( Pango::ELLIPSIZE_END ) ;
			layout[L1]->set_width( m_width * PANGO_SCALE ) ;

			if( model_mapping_size != model_size )
			{
			    layout[L1]->set_text( (boost::format(("%u %s")) % model_mapping_size % ( (model_mapping_size > 1) ? ("Albums") : ("Album"))).str()) ;
			}
			else
			{
			    layout[L1]->set_text( _("All Albums")) ;
			}

			layout[L1]->get_pixel_size( width, height ) ;

			if( selected )
			{
			    render_text_shadow( layout[L1], m_width, height, xpos+(m_width-width)/2, r.y+(row_height-height)/2, cairo ) ;
			}

			cairo->move_to(
			      xpos+(m_width-width)/2
			    , r.y+(row_height-height)/2
			) ;

			Gdk::Cairo::set_source_rgba(cairo, c2) ;
			pango_cairo_show_layout(cairo->cobj(), layout[L1]->gobj()) ;
                    }
                }
        };

        typedef boost::shared_ptr<Column>       Column_sp_t ;
        typedef std::vector<Column_sp_t>        Column_sp_t_vector_t ;
        typedef sigc::signal<void>              Signal_void ;

        class Class
        : public Gtk::DrawingArea, public Gtk::Scrollable
        {
            public:

                DataModelFilter_sp_t                m_model ;

            private:

                boost::optional<boost::tuple<Model_t::iterator, guint, guint> >  m_selection ;

		PropAdjustment	    property_vadj_, property_hadj_ ;
		PropScrollPolicy    property_vsp_ , property_hsp_ ;

		enum SelDatum
		{
		      S_ITERATOR
		    , S_ID
		    , S_INDEX
		} ;

                Column_sp_t_vector_t                m_columns ;

                Signal_void                         m_SIGNAL_selection_changed ;
                Signal_void                         m_SIGNAL_find_accepted ;
                Signal_void                         m_SIGNAL_start_playback ;

                Interval<guint>			    m_ModelExtents ;
		Minus<int>			    ModelCount ;

		ViewMetrics_type		    ViewMetrics ;

                Gtk::Entry                        * m_SearchEntry ;
                Gtk::Window                       * m_SearchWindow ;

                sigc::connection                    m_search_changed_conn ;
                bool                                m_search_active ;

		std::set<guint>			    m_caching ;
		sigc::connection		    m_redraw_spinner_conn ;

                Glib::RefPtr<Gtk::UIManager>	    m_refUIManager ;
                Glib::RefPtr<Gtk::ActionGroup>	    m_refActionGroup ;
                Gtk::Menu*			    m_pMenuPopup ;

		sigc::connection 		    conn_vadj ;

		boost::shared_ptr<IYoukiThemeEngine> m_theme ; 

	    public:

                typedef sigc::signal<void, const std::string&> SignalMBID ;
                typedef sigc::signal<void, guint>	       SignalID ;

                SignalMBID _signal_0 ;
                SignalMBID _signal_1 ;
                SignalID   _signal_2 ;

	    private:

                void
                initialize_metrics ()
                {
		    ViewMetrics.set_base__row_height(
			  77
		    ) ;
                }

                void
                on_vadj_value_changed ()
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

                inline guint
                get_upper_row()
                {
		    return ViewMetrics.ViewPort.upper() ;
                }

            protected:

                bool
                key_press_event(GdkEventKey* event)
                {
                    if( event->is_modifier )
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
                                return true ;

			    case GDK_KEY_Page_Up:
			    case GDK_KEY_Page_Down:
			    case GDK_KEY_Home:
			    case GDK_KEY_End:
				error_bell() ;
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
                    int d = 0 ;

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
                                guint origin = boost::get<S_INDEX>(m_selection.get()) ;

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
                            select_index( 0 ) ;
                            scroll_to_index( 0 ) ;

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
                                guint origin = boost::get<S_INDEX>(m_selection.get()) ;

                                if( ViewMetrics.ViewPort( origin ))
                                {
                                    d = std::min<int>( origin+step, ModelCount(m_model->size())) ;

                                    select_index( d ) ;
				    scroll_to_index( std::min<int>( std::max<int>(0, d+ViewMetrics.ViewPort.size()-(ViewMetrics.ViewPort.size()/2)), ModelCount(m_model->size()))) ;

/*
				    if( d >= ViewMetrics.ViewPort ) 
				    {
					guint pn = (ViewMetrics.ViewPort.upper()+1+(d-ViewMetrics.ViewPort.lower())) * ViewMetrics.RowHeight - ViewMetrics.Excess ; 
					vadj_value_set( std::min<int>( vadj_upper(), pn )) ; 
				    }
*/
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
                focus_entry(
		    bool in 
		)
                {
		    gtk_widget_realize(GTK_WIDGET(m_SearchEntry->gobj())) ;

                    GdkEvent *event = gdk_event_new (GDK_FOCUS_CHANGE);

                    event->focus_change.type   = GDK_FOCUS_CHANGE;
                    event->focus_change.window = GDK_WINDOW(g_object_ref((*m_SearchEntry).get_window()->gobj())) ;
                    event->focus_change.in     = in;

                    (*m_SearchEntry).send_focus_change( event ) ;

                    gdk_event_free( event ) ;
                }

		void
		on_size_allocate( Gtk::Allocation& a )
		{
		    a.set_x(0) ;
		    a.set_y(0) ;
		    Gtk::DrawingArea::on_size_allocate( a ) ;
		    queue_draw() ;
		}

                bool
                on_button_press_event( GdkEventButton* event )
                {
                    using boost::get;

		    cancel_search() ;
		    grab_focus() ;

		    if( event->button == 1 && event->type == GDK_2BUTTON_PRESS ) 
                    {
			    m_SIGNAL_start_playback.emit() ;
			    return true ;
		    }

		    double ymod = fmod( vadj_value(), ViewMetrics.RowHeight ) ;

		    guint d = (vadj_value() + event->y) / ViewMetrics.RowHeight ;

		    if( !m_selection || (m_selection && get<S_INDEX>(m_selection.get()) != d))
		    {
			if( m_ModelExtents( d ))
			{
			    select_index( d ) ;
			}

			if( ymod != 0 )
			{
			    if( d == ViewMetrics.ViewPort.upper() ) 
			    {
				vadj_value_set( std::max<int>(0, ViewMetrics.ViewPortPx.upper() - ymod + 1)) ;
			    }
			    else if( d == ViewMetrics.ViewPort.lower())
			    {
				vadj_value_set( std::min<int>(vadj_upper(), ViewMetrics.ViewPortPx.upper() + (ViewMetrics.RowHeight - ymod) - ViewMetrics.Excess )) ;
			    }
			}
		    }

                    if( event->button == 3 )
                    {
                        m_pMenuPopup->popup(event->button, event->time) ;
                    }

                    return true ;
                }

                void
                configure_vadj(
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

                bool
                on_configure_event(
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
                on_draw(
		    const Cairo::RefPtr<Cairo::Context>& cairo 
		)	
                {
                    const ThemeColor& c_text		= m_theme->get_color( THEME_COLOR_TEXT ) ;
                    const ThemeColor& c_text_sel	= m_theme->get_color( THEME_COLOR_TEXT_SELECTED ) ;

                    const ThemeColor& c_base_rules_hint = m_theme->get_color( THEME_COLOR_BASE_ALTERNATE ) ;
		    const ThemeColor& c_treelines	= m_theme->get_color( THEME_COLOR_TREELINES ) ;

		    std::valarray<double> dashes ( 2 ) ;
		    dashes[0] = 1. ; 
	            dashes[1] = 2. ;



                    guint d       = ViewMetrics.ViewPort.upper() ; 
                    guint max_d   = Limiter<guint>( Limiter<guint>::ABS_ABS, 0, m_model->size(), ViewMetrics.ViewPort.size() + 2 ) ;

                    int ypos   = 0 ;
                    int offset = ViewMetrics.ViewPortPx.upper() - (d*ViewMetrics.RowHeight) ;

                    
		    if( offset )
                    {
                        ypos -= offset ;
			max_d += 1 ;
                    }

		    /* Let's see if we can save some rendering */	
    
		    double clip_x1, clip_y1, clip_x2, clip_y2 ;
		
		    cairo->get_clip_extents( clip_x1, clip_y1, clip_x2, clip_y2 ) ;

		    if( clip_y1 > 0 && clip_y2 == ViewMetrics.ViewPortPx.lower() )
		    {
			guint d_clip = clip_y1 / ViewMetrics.RowHeight ;
			ypos += d_clip * ViewMetrics.RowHeight ;
			d += d_clip ;
			max_d -= (d_clip-1) ;
		    }
/*
		    else
		    if( clip_y1 == 0 && clip_y2 < ViewMetrics.ViewPortPx.lower() )
		    {
			guint d_clip = clip_y2 / ViewMetrics.RowHeight ;
			max_d = d_clip+1 ;
		    }
 */

		    /* Now let's render the actual data */

		    cairo->set_operator( Cairo::OPERATOR_OVER ) ;

		    RowRowMapping_t::const_iterator iter = m_model->iter( d ) ;
		    MPX::CairoCorners::CORNERS c = MPX::CairoCorners::CORNERS(0) ;

		    for( guint n = 0 ; n < max_d && m_ModelExtents(d+n) ; ++n )
		    {
			guint di = d+n ;
			guint xpos = 0 ;

			int selected = m_selection && boost::get<S_INDEX>(m_selection.get()) == d+n ;

                        if( selected ) /* Selection */
                        {
                            GdkRectangle r ;

                            r.x         = 0 ; 
                            r.y         = ypos ; 
                            r.width     = get_allocation().get_width() ;
                            r.height    = ViewMetrics.RowHeight ;

                            m_theme->draw_selection_rectangle(
                                  cairo
                                , r
                                , has_focus()
				, 0
				, c
                            ) ;
                        }
			else
			if( di % 2 ) /* Rules Hint */
			{
			    GdkRectangle r ;

			    r.x         = 0 ;
			    r.y         = ypos ;
			    r.width     = get_allocation().get_width() ;
			    r.height    = ViewMetrics.RowHeight ;

			    RoundedRectangle(cairo, r.x, r.y, r.width, r.height, 0, c) ;
			    Gdk::Cairo::set_source_rgba(cairo, c_base_rules_hint);
			    cairo->fill() ;
			}

			m_columns[0]->render(
			      cairo
			    , **iter
			    , *this
			    , di 
			    , xpos
			    , ypos
			    , ViewMetrics.RowHeight 
			    , selected ? c_text_sel : c_text
			    , ModelCount(m_model->size())
			    , ModelCount(m_model->m_realmodel->size())
			    , selected
			    , m_model->m_constraints_albums
			) ;

			ypos += ViewMetrics.RowHeight ;
			++ iter ;
		    }

		    guint rend_offset = 0 ;

		    if( d == 0 )
		    {
			rend_offset = ViewMetrics.RowHeight - ViewMetrics.ViewPortPx.upper() ;	
		    }

		    cairo->save() ;
		    cairo->set_antialias( Cairo::ANTIALIAS_NONE ) ;
		    cairo->set_line_width(
			  1. 
		    ) ;
		    cairo->move_to(
			  78
			, rend_offset 
		    ) ; 
		    cairo->line_to(
			  78
			, std::min<int>( vadj_upper()-vadj_value()+ViewMetrics.RowHeight, get_allocation().get_height()) - rend_offset
		    ) ;
		    cairo->set_dash(
			  dashes
			, 0
		    ) ;
		    cairo->set_source_rgba(
			  c_treelines.get_red()
			, c_treelines.get_green()
			, c_treelines.get_blue()
			, c_treelines.get_alpha() * 0.8
		    ) ;
		    cairo->stroke() ;
		    cairo->restore() ;

                    return true;
                }

		double
		vadj_value()
		{
		    if(  property_vadjustment().get_value() )
			return property_vadjustment().get_value()->get_value() ;

		    return 0 ;
		}

		double
		vadj_upper()
		{
		    if(  property_vadjustment().get_value() )
			return property_vadjustment().get_value()->get_upper() ;

		    return 0 ;
		}

		void
		vadj_value_set( double v_ )
		{
		    if(  property_vadjustment().get_value() )
			return property_vadjustment().get_value()->set_value( v_ ) ;
		}

                void
                on_model_changed(
                      guint d
                )
		{
		    configure_vadj(
			  m_model->size() * ViewMetrics.RowHeight
			, ViewMetrics.ViewPortPx.size() 
			, ViewMetrics.RowHeight 
		    ) ;

		    m_ModelExtents = Interval<guint>(
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
			select_index( d, true ) ;
		    }
                }

		void
		on_vadj_prop_changed()
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
		invalidate_covers()
		{
		    for( Model_t::iterator i = m_model->m_realmodel->begin() ; i != m_model->m_realmodel->end() ; ++i )
		    {
			(*i)->surface_cache.clear() ;
		    }
		}

		public:

		void
		set_show_release_label( bool show )
		{
		    m_columns[0]->m_show_release_label = show ;
		    queue_draw() ;
		}

		void
		set_rt_viewmode( RTViewMode mode )
		{
		    m_columns[0]->m_rt_viewmode = mode ;
		    invalidate_covers() ;
		    queue_draw() ;
		}

                void
                select_id(
                    boost::optional<guint> id
                )
                {
                    using boost::get;

                    if( id )
                    {
                        const guint& real_id = id.get() ;

			guint d = 0 ;

                        for( RowRowMapping_t::iterator i = m_model->m_mapping.begin(); i != m_model->m_mapping.end(); ++i )
                        {
                            if( real_id == (**i)->album_id )
                            {
                                select_index( d ) ;
                                return ;
                            }

			    ++d ;
                        }
                    }
                }

                void
                scroll_to_index(
                      guint d
                )
                {
                        if( m_model->m_mapping.size() < ViewMetrics.ViewPort.size() ) 
                        {
			    vadj_value_set(0) ;
                        }
                        else
                        {
                            Limiter<guint> d2 (
                                  Limiter<guint>::ABS_ABS
                                , 0
                                , (m_model->size() * ViewMetrics.RowHeight) - ViewMetrics.ViewPort.size()
                                , d * ViewMetrics.RowHeight 
                            ) ;

			    vadj_value_set(d2) ;
                        }
                }

                void
                select_index(
                      guint   d
                    , bool    quiet = false
                )
                {
                    if( m_ModelExtents( d ))
                    {
                        const guint& id = (*m_model->m_mapping[d])->album_id ;

                        m_selection = boost::make_tuple( m_model->m_mapping[d], id, d ) ;

                        if( !quiet )
                        {
                            m_SIGNAL_selection_changed.emit() ;
                        }

                        queue_draw();
                    }
                }

                Signal_void&
                signal_selection_changed()
                {
                    return m_SIGNAL_selection_changed ;
                }

                Signal_void&
                signal_find_accepted()
                {
                    return m_SIGNAL_find_accepted ;
                }

                Signal_void&
                signal_start_playback()
                {
                    return m_SIGNAL_start_playback ;
                }

                boost::optional<guint>
                get_selected_id()
                {
		    boost::optional<guint> id ;

                    if( m_selection )
                    {
			id = boost::get<S_ID>(m_selection.get()) ;
                    }

                    return id ;
                }

                boost::optional<guint>
                get_selected_index()
                {
		    boost::optional<guint> idx ;

                    if( m_selection )
                    {
                        idx = boost::get<S_INDEX>(m_selection.get()) ;
                    }

                    return idx ; 
                }

                boost::optional<guint>
                get_selected()
                {
                    if( m_selection )
                    {
                        const guint& sel_id = boost::get<S_ID>(m_selection.get()) ;

                        if( sel_id != -1 )
                        {
                            return boost::optional<guint>( sel_id ) ;
                        }
                    }

                    return boost::optional<guint>() ;
                }

                void
                set_model(DataModelFilter_sp_t model)
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

                    on_model_changed( 0 ) ;
                }

                void
                append_column(
                      Column_sp_t   column
                )
                {
                    m_columns.push_back(column);
                }

            protected:

                void
                find_next_match()
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
                        Glib::ustring match = Glib::ustring((**i)->album).casefold() ;

                        if( match.length() && match.substr( 0, text.length()) == text.substr( 0, text.length()))
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
                find_prev_match()
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
                        Glib::ustring match = Glib::ustring((**i)->album).casefold() ;

                        if( match.length() && match.substr( 0, text.length()) == text.substr( 0, text.length()))
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
                on_search_entry_changed()
                {
                    using boost::get ;

                    Glib::ustring text = m_SearchEntry->get_text().casefold() ;

                    if( text.empty() )
                    {
                        return ;
                    }

                    RowRowMapping_t::iterator i = m_model->m_mapping.begin();
                    ++i ;

		    guint d = std::distance( m_model->m_mapping.begin(), i ) ;

                    for( ; i != m_model->m_mapping.end(); ++i )
                    {
                        Glib::ustring match = Glib::ustring((**i)->album).casefold() ;

                        if( match.length() && match.substr( 0, text.length()) == text.substr( 0, text.length()))
                        {
                            scroll_to_index( std::max<int>(0,d-get_page_size()/2)) ;
                            select_index( d ) ;
			    m_SearchEntry->unset_color() ;
                            return ;
                        }

			++d ;
                    }

		    m_SearchEntry->override_color(Util::make_rgba(1.,0.,0.,1.)) ;
                }

                void
                on_search_entry_activated()
                {
                    cancel_search() ;
                    m_SIGNAL_find_accepted.emit() ;
                }

                bool
                on_search_window_focus_out(
                      GdkEventFocus* G_GNUC_UNUSED
                )
                {
                    cancel_search() ;
                    return false ;
                }

                void
                on_show_only_this_album()
                {
                    if( m_selection )
                    {
                        Album_sp album = *(boost::get<S_ITERATOR>(m_selection.get())) ;
                        _signal_0.emit( album->mbid ) ;
                    }
                }

                void
                on_show_only_this_artist()
                {
                    if( m_selection )
                    {
                        Album_sp album = *(boost::get<S_ITERATOR>(m_selection.get())) ;
                        _signal_1.emit( album->mbid_artist ) ;
                    }
                }

                void
                on_refetch_album_cover()
                {
                    if( m_selection )
                    {
                        Album_sp album = *(boost::get<S_ITERATOR>(m_selection.get())) ;
			album->caching = true ;
			m_caching.insert( album->album_id ) ;
                        _signal_2.emit( album->album_id ) ;
			queue_draw() ;

			if( !m_redraw_spinner_conn )
			{
			     m_redraw_spinner_conn = Glib::signal_timeout().connect( sigc::mem_fun( *this, &Class::handle_redraw ), 100 ) ;
			}
                    }
                }

		bool
		handle_redraw()
		{
		    m_columns[0]->m_image_album_loading_iter->advance() ;
		    queue_draw() ;
		    while(gtk_events_pending()) gtk_main_iteration() ;
		    if(m_caching.empty()) m_redraw_spinner_conn.disconnect() ;
		    return !m_caching.empty() ;
		}

		void
		handle_cover_updated( guint id )
		{
		    m_caching.erase( id ) ;
		}

            public:

                void
                cancel_search()
                {
		    focus_entry( false ) ;
		    m_SearchWindow->hide() ;
		    m_search_changed_conn.block () ;
		    m_SearchEntry->set_text("") ;
		    m_search_changed_conn.unblock () ;
		    m_search_active = false ;
                }

            protected:

                virtual void
                on_realize()
                {
                    Gtk::DrawingArea::on_realize() ;
                    initialize_metrics();
                    queue_resize();
                }

            public:

                inline guint
                get_page_size(
                )
                {
		    return ViewMetrics.ViewPort.size() ;
                }

                void
                clear_selection(
                )
                {
                    m_selection.reset() ;
                    queue_draw() ;
                }

                void
                clear_selection_quiet(
                )
                {
                    m_selection.reset() ;
                }

                SignalMBID&
                signal_only_this_album_mbid()
                {
                    return _signal_0 ;
                }

                SignalMBID&
                signal_only_this_artist_mbid()
                {
                    return _signal_1 ;
                }

                SignalID&
                signal_refetch_cover()
                {
                    return _signal_2 ;
                }

                Class ()
			: ObjectBase( "YoukiViewAlbums" )

			, property_vadj_(*this, "vadjustment", RPAdj(0))
			, property_hadj_(*this, "hadjustment", RPAdj(0))

			, property_vsp_(*this, "vscroll-policy", Gtk::SCROLL_NATURAL )
			, property_hsp_(*this, "hscroll-policy", Gtk::SCROLL_NATURAL )

			, m_search_active( false )
                {
		    property_vadjustment().signal_changed().connect( sigc::mem_fun( *this, &Class::on_vadj_prop_changed )) ;

		    set_double_buffered(true) ;
                    set_can_focus(true);

		    ModelCount = Minus<int>( 1 ) ;

                    m_theme = services->get<IYoukiThemeEngine>("mpx-service-theme") ;
                    const ThemeColor& c = m_theme->get_color(THEME_COLOR_BASE) ;
                    override_background_color(c, Gtk::STATE_FLAG_NORMAL) ;

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

                    signal_key_press_event().connect(
                        sigc::mem_fun(
                              *this
                            , &Class::key_press_event
                    ), true ) ;

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

                virtual ~Class ()
                {
                }
        };
}
}
}

#endif // _YOUKI_ALBUM_LIST_HH
