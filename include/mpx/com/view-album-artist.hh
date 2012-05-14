#ifndef YOUKI_VIEW_ARTISTS__HH
#define YOUKI_VIEW_ARTISTS__HH

#include <gtkmm.h>
#include <cairomm/cairomm.h>
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/optional.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include "mpx/algorithm/aque.hh"
#include "mpx/algorithm/interval.hh"
#include "mpx/algorithm/limiter.hh"
#include "mpx/algorithm/minus.hh"
#include "mpx/algorithm/adder.hh"

#include "mpx/aux/glibaddons.hh"

#include "mpx/mpx-types.hh"

#include "mpx/util-string.hh"
#include "mpx/util-graphics.hh"

#include "mpx/widgets/cairo-extensions.hh"
#include "mpx/widgets/cairo-blur.hh"

#include "glib-marshalers.h"

#include "mpx/mpx-main.hh"
#include "mpx/i-youki-theme-engine.hh"

#include "mpx/com/indexed-list.hh"

using boost::get ;

namespace
{
    typedef Glib::RefPtr<Gtk::Adjustment>		    RPAdj ;
    typedef Glib::Property<RPAdj>			    PropAdjustment ;
    typedef Glib::Property<Gtk::ScrollablePolicy>	    PropScrollPolicy ;
}

namespace MPX
{
namespace View
{
namespace Artist
{
        namespace
        {
            const double rounding = 1. ;
        }

        typedef boost::tuple<std::string, boost::optional<guint> > Row_t ;

        typedef IndexedList<Row_t>                         Model_t ;
        typedef boost::shared_ptr<Model_t>                 Model_sp ;
        typedef std::map<guint, Model_t::iterator>         IdIterMap_t ;

        typedef std::vector<Model_t::iterator>             RowRowMapping_t ;

        typedef sigc::signal<void>                         Signal_0 ;
        typedef sigc::signal<void, guint>		   Signal_1 ;
	typedef sigc::signal<void, guint>		   Signal_1a ;

        struct OrderFunc
        : public std::binary_function<Row_t, Row_t, bool>
        {
            bool operator() (
                  const Row_t& a
                , const Row_t& b
            ) const
            {
		if( !get<1>(a))
		    return true ;

		if( !get<1>(b))
		    return false ;

                return boost::get<0>(a) < boost::get<0>(b) ;
            }
        } ;

        struct DataModel
        : public sigc::trackable
        {
                Model_sp                      m_realmodel ;
                IdIterMap_t                     m_iter_map ;
                guint				m_upper_bound ;
                Signal_1                        m_changed ;

                DataModel()
                    : m_upper_bound( 0 )
                {
                    m_realmodel = Model_sp( new Model_t ) ;
                }

                DataModel(
                    Model_sp model
                )
                : m_upper_bound( 0 )
                {
                    m_realmodel = model;
                }

                virtual void
                clear()
                {
                    m_realmodel->clear () ;
                    m_iter_map.clear() ;
                    m_upper_bound = 0 ;
                }

                virtual Signal_1&
                signal_changed()
                {
                    return m_changed ;
                }

                virtual bool
                is_set()
                {
                    return bool(m_realmodel);
                }

                virtual guint
                size()
                {
                    return m_realmodel->size();
                }

                virtual Row_t&
                row(guint row)
                {
                    return (*m_realmodel)[row];
                }

                virtual void
                set_current_row(
                    guint row
                )
                {
                    m_upper_bound = row ;
                }

                virtual void
                append_artist(
                      const std::string&        artist
                    , const boost::optional<guint>& artist_id = boost::optional<guint>()
                )
                {
                    Row_t row ( artist, artist_id ) ;
                    m_realmodel->push_back(row);

		    if( artist_id )
		    {
			Model_t::iterator i = m_realmodel->end();
			std::advance( i, -1 );
			m_iter_map.insert(std::make_pair(artist_id.get(), i));
		    }
                }

                virtual void
                insert_artist(
                      const std::string&	artist_name
                    , const boost::optional<guint>& artist_id = boost::optional<guint>()
                )
                {
                    static OrderFunc order ;

                    Row_t row(
                          artist_name
                        , artist_id
                    ) ;

                    Model_t::iterator i = m_realmodel->insert(
                          std::lower_bound(
                                m_realmodel->begin()
                              , m_realmodel->end()
                              , row
                              , order
                          )
                        , row
                    ) ;
	    
		    if( artist_id )
                        m_iter_map.insert( std::make_pair( artist_id.get(), i ));
                }
        };

        typedef boost::shared_ptr<DataModel> DataModel_sp;

        struct DataModelFilter : public DataModel
        {
                typedef std::vector<guint>              IdVector_t ;
                typedef boost::shared_ptr<IdVector_t>   IdVector_sp ;

                RowRowMapping_t        m_mapping ;
                IdVector_sp            m_constraints_artist ;

                DataModelFilter(DataModel_sp & model)
                : DataModel( model->m_realmodel )
                {
                    regen_mapping() ;
                }

                virtual ~DataModelFilter()
                {
                }

                virtual void
                set_constraints_artist(
                    const IdVector_sp& constraint
                )
                {
                    if( constraint != m_constraints_artist )
                    {
                        m_constraints_artist = constraint ;
                        regen_mapping() ;
                    }
                }

                virtual void
                clear_constraints_artist(
                )
                {
                    m_constraints_artist.reset() ;
                }

                virtual void
                clear()
                {
                    DataModel::clear() ;
                    m_mapping.clear() ;
		    m_upper_bound = 0 ;
                    m_changed.emit( m_upper_bound ) ;
                }

                virtual guint
                size()
                {
                    return m_mapping.size();
                }

                virtual Row_t&
                row(
                      guint row
                )
                {
                    return *(m_mapping[row]);
                }

                virtual RowRowMapping_t::const_iterator
                row_iter(
                      guint row
                )
                {
                    RowRowMapping_t::const_iterator i  = m_mapping.begin() ;
                    std::advance( i, row ) ;
                    return i ;
                }

                virtual void
                append_artist(
                      const std::string&        artist
                    , const boost::optional<guint>& artist_id = boost::optional<guint>()
                )
                {
                    DataModel::append_artist( artist, artist_id ) ;
                }

                virtual void
                insert_artist(
                      const std::string&        artist_name
                    , const boost::optional<guint>& artist_id = boost::optional<guint>()
                )
                {
                    DataModel::insert_artist(
                          artist_name
                        , artist_id
                    ) ;
                }

                virtual void
                erase_artist(
                      guint                    artist_id
                )
                {
                    IdIterMap_t::iterator i = m_iter_map.find( artist_id ) ;

                    if( i != m_iter_map.end() )
                    {
                        Model_t::iterator iter = (*i).second ;
			m_iter_map.erase( i ) ;
                        m_realmodel->erase( iter ) ;
                    }
                }

                void
                regen_mapping(
                )
                {
                    using boost::get;

                    if( m_realmodel->empty() )
                    {
                        return ;
                    }

                    RowRowMapping_t new_mapping;
                    new_mapping.reserve( m_realmodel->size() ) ;

                    m_upper_bound = 0 ;

                    IdVector_t * constraints_artist = m_constraints_artist.get() ;

                    Model_t::iterator i = m_realmodel->begin() ;
                    new_mapping.push_back( i++ ) ;
		    guint n = std::distance( m_realmodel->begin(), i ) ;

                    for( ; i != m_realmodel->end(); ++i )
                    {
                        boost::optional<guint> id_row = get<1>( *i ) ;

                        if( !constraints_artist || (*m_constraints_artist)[id_row.get()] )
                        {
                            new_mapping.push_back( i ) ;
                        }

			++n ;
                    }

                    std::swap( m_mapping, new_mapping ) ;
                    update_count() ;
                    m_changed.emit(0) ;
                }

                void
                update_count(
                )
                {
                    guint model_size = m_mapping.size() - 1 ; // -1, because of our dummy album

                    Row_t& row = **m_mapping.begin() ;

		    if( model_size != (m_realmodel->size() - 1) )
		    {
                    	get<0>(row) = (boost::format(_("%u %s")) % model_size % ((model_size > 1) ? _("Artists") : _("Artist"))).str() ;
		    }
		    else
		    {
                    	get<0>(row) = _("All Artists") ;
		    }
                }
        };

        typedef boost::shared_ptr<DataModelFilter> DataModelFilter_sp;

        class Column
        {
                int                 m_width ;
                int                 m_column ;
                std::string         m_title ;
                Pango::Alignment    m_alignment ;

            public:

                Column (std::string const& title)
                : m_width( 0 )
                , m_column( 0 )
                , m_title( title )
                , m_alignment( Pango::ALIGN_LEFT )
                {
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

                void
                set_title (std::string const& title)
                {
                    m_title = title;
                }

                std::string const&
                get_title ()
                {
                    return m_title;
                }

                void
                set_alignment(
                    Pango::Alignment align
                )
                {
                    m_alignment = align ;
                }


                Pango::Alignment
                get_alignment(
                )
                {
                    return m_alignment ;
                }

                void
                render(
                      const Cairo::RefPtr<Cairo::Context>   cairo
                    , const Row_t&			    r
                    , Gtk::Widget&			    widget
                    , int				    d
                    , int				    xpos
                    , int				    ypos
                    , int				    rowheight
                    , bool				    selected
                    , const ThemeColor&			    color
                )
                {
                    using boost::get;

                    Glib::RefPtr<Pango::Layout> l = widget.create_pango_layout(get<0>(r)) ;

                    l->set_ellipsize(
                          Pango::ELLIPSIZE_END
                    ) ;
                    l->set_width(
                          (m_width - 8) * PANGO_SCALE
                    ) ;

                    l->set_alignment( m_alignment ) ;



                    if( d == 0 )
                    {
			Pango::Attribute attr = Pango::Attribute::create_attr_weight( Pango::WEIGHT_BOLD ) ;
			Pango::AttrList list ;
			list.insert( attr ) ;
			l->set_attributes( list ) ;
                    }

                    cairo->save() ;
		    cairo->set_operator( Cairo::OPERATOR_OVER ) ;

		    if( selected )
		    {
			Util::render_text_shadow( l, xpos+2, ypos+2, cairo ) ;
		    }

	            Gdk::Cairo::set_source_rgba(cairo, color);
                    cairo->move_to(
                          xpos + 2
                        , ypos + 2
                    ) ;
                    pango_cairo_show_layout(
                          cairo->cobj()
                        , l->gobj()
                    ) ;
                    cairo->restore();
                }
        };

        typedef boost::shared_ptr<Column>       Column_sp ;
        typedef std::vector<Column_sp>        Column_SP_vector_t ;
        typedef sigc::signal<void>              Signal_0 ;

        class Class 
	: public Gtk::DrawingArea, public Gtk::Scrollable
        {
	    public:

                DataModelFilter_sp                m_model ;

	    private:

		PropAdjustment			    property_vadj_, property_hadj_ ;
		PropScrollPolicy		    property_vsp_ , property_hsp_ ;

                guint                               m_height__row ;
                guint                               m_height__current_viewport ;

		Interval<guint>			    ModelExtents ;
		Interval<guint>			    m_Viewport_I ;
		Minus<int>			    ModelCount ;

                boost::optional<boost::tuple<Model_t::iterator, boost::optional<guint>, guint> > m_selection ;

		enum SelDatum
		{
		      S_ITERATOR
		    , S_ID
		    , S_INDEX
		} ;

                Column_SP_vector_t                  m_columns ;

                std::set<int>                       m_columns__collapsed ;
                std::set<int>                       m_columns__fixed ;
                int                                 m_columns__fixed_total_width ;

                Signal_0			    m_SIGNAL_selection_changed ;
                Signal_0			    m_SIGNAL_find_accepted ;
                Signal_0			    m_SIGNAL_start_playback ;

                Gtk::Entry                        * m_SearchEntry ;
                Gtk::Window                       * m_SearchWindow ;

                sigc::connection                    m_search_changed_conn ;
                bool                                m_search_active ;

                void
                initialize_metrics ()
                {
                    Glib::RefPtr<Pango::Context> context = get_pango_context();

                    Pango::FontMetrics metrics = context->get_metrics(
			                              get_style_context ()->get_font()
			                            , context->get_language()
                    ) ;

                    m_height__row =
                        (metrics.get_ascent() / PANGO_SCALE) +
                        (metrics.get_descent() / PANGO_SCALE) + 5 ;
                }

		void
		on_vadj_prop_changed()
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
                on_vadj_value_changed ()
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
                get_page_size(
                )
                {
                    if( m_height__current_viewport && m_height__row )
                        return m_height__current_viewport / m_height__row ;
                    else
                        return 0 ;
                }

                inline guint
                get_upper_row ()
                {
                    if( property_vadjustment().get_value() )
                        return property_vadjustment().get_value()->get_value() / m_height__row ;

		    return 0 ;
                }

            protected:

                bool
                key_press_event(
                      GdkEventKey* event
                )
                {
                    if( !m_model->size() )
                        return false ;

                    if( m_search_active )
                    {
                        switch( event->keyval )
                        {
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
                                    scroll_to_index( 0 ) ;
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
                focus_entry(
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

		    if( event->button == 1 ) 
                    {
			if( event->type == GDK_2BUTTON_PRESS )
			{
			    m_SIGNAL_start_playback.emit() ;
			}
			else
			{
			    double ymod = fmod( vadj_value(), m_height__row ) ;

			    guint d = (vadj_value() + event->y) / m_height__row ;

			    if( m_selection && get<S_INDEX>(m_selection.get()) == d )
			    {
				return false ;
			    }

			    if( ModelExtents( d ))
			    {
				select_index( d ) ;
			    }

			    if( ymod != 0 )
			    {
				if( d == get_upper_row() ) 
				{
				    vadj_value_set( std::max<int>(0, vadj_value() - ymod + 1)) ;
				}
				else if( d >= (get_upper_row()+get_page_size()-1))
				{
				    double Excess = get_allocation().get_height() - (get_page_size()*m_height__row) ;
				    vadj_value_set( std::min<int>(vadj_upper(), vadj_value() + (m_height__row - ymod) - Excess )) ;
				}
			    }
			}
                    }

                    return true ;
                }

                bool
                on_button_release_event (GdkEventButton * event)
                {
                    return false ;
                }

                bool
                on_leave_notify_event(
                    GdkEventCrossing* G_GNUC_UNUSED
                )
                {
                    queue_draw () ;

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
                        property_vadjustment().get_value()->set_page_increment( step_increment ) ;
                    }
                }

                bool
                on_configure_event(
                    GdkEventConfigure* event
                )
                {
                    m_height__current_viewport = event->height ;

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

                bool
                on_draw(
		    const Cairo::RefPtr<Cairo::Context>& cairo 
		)
                {
                    boost::shared_ptr<IYoukiThemeEngine> theme = services->get<IYoukiThemeEngine>("mpx-service-theme") ;

                    const ThemeColor& c_base_rules_hint = theme->get_color( THEME_COLOR_BASE_ALTERNATE ) ;
                    const ThemeColor& c_text            = theme->get_color( THEME_COLOR_TEXT ) ;
                    const ThemeColor& c_text_sel        = theme->get_color( THEME_COLOR_TEXT_SELECTED ) ;

                    guint d	= get_upper_row() ;
                    guint d_max = Limiter<guint>(
                                            Limiter<guint>::ABS_ABS
                                          , 0
                                          , m_model->size()
                                          , get_page_size() + 2
                                  ) ;
                    guint xpos = 0 ;
                    gint  ypos = 0 ;

                    int offset  = vadj_value() - (d*m_height__row) ;
		    ypos -= offset ? offset : 0 ;

                    guint n = 0 ;
		    Algorithm::Adder<guint> d_cur( d, n ) ;

		    GdkRectangle rr ;
		    rr.x = 0 ;
		    rr.width = get_allocated_width() ;
		    rr.height = m_height__row ;

                    while( n < d_max && ModelExtents(d_cur)) 
                    {
                        xpos = 0 ;

                        RowRowMapping_t::const_iterator iter = m_model->row_iter( d_cur ) ;

                        int is_selected = (m_selection && get<S_ID>(m_selection.get()) == get<S_ID>(**iter)) ;

                        if( is_selected )
                        {
                            rr.y = ypos ;

                            theme->draw_selection_rectangle(
                                  cairo
                                , rr
                                , has_focus()
                                , rounding
                                , MPX::CairoCorners::CORNERS(0)
                            ) ;
                        }
                        else if(d_cur%2)
                        {
			    rr.y = ypos ;

                            cairo->rectangle(
                                  rr.x
                                , rr.y
                                , rr.width
                                , rr.height
                            ) ;

                            Gdk::Cairo::set_source_rgba(cairo, c_base_rules_hint);
                            cairo->fill() ;
                        }

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

		    get_window()->process_all_updates() ;

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
		guint position
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

                scroll_to_index( position ) ;
                select_index( position ) ;
            }

            public:

                void
                select_id(
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
                select_index(
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
                scroll_to_index(
                      guint d
                )
                {
                    if( m_height__current_viewport && m_height__row )
                    {
                        if( m_model->m_mapping.size() < get_page_size() )
                        {
			    vadj_value_set( 0 ) ;
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
                get_selected_id()
                {
		    return get<S_ID>(m_selection.get()) ;
                }

                boost::optional<guint>
                get_selected_index()
                {
                    boost::optional<guint> idx ;

                    if( m_selection )
                    {
                        idx = get<S_INDEX>(m_selection.get()) ;
                    }

                    return idx ;
                }

                Signal_0&
                signal_selection_changed()
                {
                    return m_SIGNAL_selection_changed ;
                }

                Signal_0&
                signal_find_accepted()
                {
                    return m_SIGNAL_find_accepted ;
                }

                Signal_0&
                signal_start_playback()
                {
                    return m_SIGNAL_start_playback ;
                }

                boost::optional<guint>
                get_selected()
                {
		    boost::optional<guint> v_ ; 

                    if( m_selection )
                    {
                            v_ = boost::get<S_ID>(m_selection.get()) ;
                    }

                    return v_ ; 
                }

                void
                clear_selection(
                )
                {
		    m_selection.reset() ;
                }

                void
                set_model(
                      DataModelFilter_sp  model
                )
                {
                    m_model = model;

                    m_model->signal_changed().connect(
                        sigc::mem_fun(
                            *this,
                            &Class::on_model_changed
                    ));

                    on_model_changed( 0 ) ;
                    queue_resize() ;
                }

                void
                append_column(
                      Column_sp   column
                )
                {
                    m_columns.push_back(column);
                }

                void
                column_set_collapsed(
                      int       column
                    , bool      collapsed
                )
                {
                    if( collapsed )
                    {
                        m_columns__collapsed.insert( column ) ;
                        queue_resize () ;
                        queue_draw () ;
                    }
                    else
                    {
                        m_columns__collapsed.erase( column ) ;
                        queue_resize () ;
                        queue_draw () ;
                    }
                }

                void
                column_set_fixed(
                      int       column
                    , bool      fixed
                    , int       width = 0
                )
                {
                    if( fixed )
                    {
                        m_columns__fixed.insert( column ) ;
                        m_columns__fixed_total_width += width ;
                        m_columns[column]->set_width( width ) ;
                        queue_resize () ;
                        queue_draw () ;
                    }
                    else
                    {
                        m_columns__fixed.erase( column ) ;
                        m_columns__fixed_total_width -= m_columns[column]->get_width() ;
                        queue_resize () ;
                        queue_draw () ;
                    }
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

                Class ()

                        : ObjectBase( "YoukiViewArtists" )

			, property_vadj_(*this, "vadjustment", RPAdj(0))
			, property_hadj_(*this, "hadjustment", RPAdj(0))

			, property_vsp_(*this, "vscroll-policy", Gtk::SCROLL_NATURAL )
			, property_hsp_(*this, "hscroll-policy", Gtk::SCROLL_NATURAL )

                        , m_columns__fixed_total_width( 0 )
                        , m_search_active( false )

                {
		    property_vadjustment().signal_changed().connect( sigc::mem_fun( *this, &Class::on_vadj_prop_changed )) ;

		    ModelCount = Minus<int>( -1 ) ;

                    boost::shared_ptr<IYoukiThemeEngine> theme = services->get<IYoukiThemeEngine>("mpx-service-theme") ;
                    const ThemeColor& c = theme->get_color( THEME_COLOR_BASE ) ;

                    Gdk::RGBA cgdk ;
                    cgdk.set_rgba( c.get_red(), c.get_green(), c.get_blue(), 1.0 ) ;
                    override_background_color ( cgdk, Gtk::STATE_FLAG_NORMAL ) ;

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

                    signal_key_press_event().connect(
                          sigc::mem_fun(
                              *this
                            , &Class::key_press_event
                    ), true ) ;
                }

                virtual ~Class ()
                {
                }
        };
}
}
}

#endif
