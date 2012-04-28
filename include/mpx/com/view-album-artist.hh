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

#include "mpx/aux/glibaddons.hh"

#include "mpx/mpx-types.hh"

#include "mpx/util-string.hh"
#include "mpx/util-graphics.hh"

#include "mpx/widgets/cairo-extensions.hh"
#include "mpx/widgets/cairo-blur.h"

#include "glib-marshalers.h"

#include "mpx/mpx-main.hh"
#include "mpx/i-youki-theme-engine.hh"

#include "mpx/com/indexed-list.hh"

using boost::get ;

typedef Glib::Property<Gtk::Adjustment*> PropAdj;

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

        typedef boost::tuple<std::string, guint>           Row_t ;

        typedef IndexedList<Row_t>                         Model_t ;
        typedef boost::shared_ptr<Model_t>                 Model_sp_t ;
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
                gint id[2] = { get<1>(a), get<1>(b) } ;

                if( id[0] == -1 )
                {
                    return true ;
                }

                if( id[1] == -1 )
                {
                    return false ;
                }

                return boost::get<0>(a) < boost::get<0>(b) ;
            }
        } ;

        struct DataModel
        : public sigc::trackable
        {
                Model_sp_t                      m_realmodel ;
                IdIterMap_t                     m_iter_map ;
                guint                     m_top_row ;
                boost::optional<guint>          m_selected ;
                boost::optional<guint>    m_selected_row ;
                Signal_1                        m_changed ;

                DataModel()
                    : m_top_row( 0 )
                {
                    m_realmodel = Model_sp_t( new Model_t ) ;
                }

                DataModel(
                    Model_sp_t model
                )
                : m_top_row( 0 )
                {
                    m_realmodel = model;
                }

                virtual void
                clear()
                {
                    m_realmodel->clear () ;
                    m_iter_map.clear() ;
                    m_top_row = 0 ;
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
                    m_top_row = row ;
                }

                virtual void
                set_selected(
                    const boost::optional<guint>& row = boost::optional<guint>()
                )
                {
                    m_selected = row ;
                }

                virtual boost::optional<guint>
                get_selected_row(
                )
                {
                    return m_selected_row ;
                }

                virtual void
                append_artist(
                      const std::string&        artist
                    , guint                    artist_id
                )
                {
                    Row_t row ( artist, artist_id ) ;
                    m_realmodel->push_back(row);

                    Model_t::iterator i = m_realmodel->end();
                    std::advance( i, -1 );
                    m_iter_map.insert(std::make_pair(artist_id, i));
                }

                virtual void
                insert_artist(
                      const std::string&                                                                artist_name
                    , guint                                                                            artist_id
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

                    m_iter_map.insert( std::make_pair( artist_id, i ));
                }
        };

        typedef boost::shared_ptr<DataModel> DataModel_sp_t;

        struct DataModelFilter : public DataModel
        {
                typedef std::vector<guint>              IdVector_t ;
                typedef boost::shared_ptr<IdVector_t>   IdVector_sp ;

                RowRowMapping_t        m_mapping ;
                IdVector_sp            m_constraints_artist ;

                DataModelFilter(DataModel_sp_t & model)
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
		    m_top_row = 0 ;
                    m_changed.emit( m_top_row ) ;
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
                    , guint                    artist_id
                )
                {
                    DataModel::append_artist( artist, artist_id ) ;
                }

                virtual void
                insert_artist(
                      const std::string&        artist_name
                    , guint                    artist_id
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

		    boost::optional<guint> id_sel ;

		    if( m_selected )
		    {
			id_sel = m_selected ;
		    }

                    m_selected.reset() ;
                    m_selected_row.reset() ;

                    m_top_row = 0 ;

                    IdVector_t * constraints_artist = m_constraints_artist.get() ;

                    Model_t::iterator i = m_realmodel->begin() ;
                    new_mapping.push_back( i++ ) ;
		    guint n = std::distance( m_realmodel->begin(), i ) ;

                    for( ; i != m_realmodel->end(); ++i )
                    {
                        guint id_row = get<1>( *i ) ;

			if( id_sel && id_sel.get() == id_row )
			{
			    m_selected_row = n ;
			}

                        if( !constraints_artist || (*m_constraints_artist)[id_row] )
                        {
                            new_mapping.push_back( i ) ;
                        }

			++n ;
                    }

		    guint d = 0 ;

		    if( m_selected_row )
		    {
			d = m_selected_row.get() ;
		    }

                    std::swap( m_mapping, new_mapping ) ;
                    update_count() ;
                    m_changed.emit( d ) ;
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

        typedef boost::shared_ptr<DataModelFilter> DataModelFilter_sp_t;

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
                    , const Row_t&			    datarow
                    , Gtk::Widget&			    widget
                    , int				    row
                    , int				    xpos
                    , int				    ypos
                    , int				    rowheight
                    , bool				    selected
                    , const ThemeColor&			    color
                )
                {
                    using boost::get;

                    cairo->save() ;

                    std::string s = get<0>(datarow) ;

                    Glib::RefPtr<Pango::Layout> layout = widget.create_pango_layout("") ;

                    layout->set_ellipsize(
                          Pango::ELLIPSIZE_END
                    ) ;

                    layout->set_width(
                          (m_width - 20) * PANGO_SCALE
                    ) ;

                    layout->set_alignment( m_alignment ) ;

                    if( row == 0 )
                    {
			Pango::Attribute attr = Pango::Attribute::create_attr_weight( Pango::WEIGHT_BOLD ) ;
			Pango::AttrList list ;
			list.insert( attr ) ;
			layout->set_attributes( list ) ;
                    }

                    layout->set_text( s ) ;

		    cairo->set_operator( Cairo::OPERATOR_OVER ) ;

		    if( selected )
		    {
			    Cairo::RefPtr<Cairo::ImageSurface> srf = Cairo::ImageSurface::create( Cairo::FORMAT_ARGB32, m_width, rowheight ) ;
			    Cairo::RefPtr<Cairo::Context> c = Cairo::Context::create( srf ) ;

			    c->set_operator( Cairo::OPERATOR_CLEAR) ;
			    c->paint() ;

			    c->set_operator( Cairo::OPERATOR_OVER ) ;

			    c->set_source_rgba(
				  0.
				, 0.
				, 0.
				, 0.40
			    ) ;
			    c->move_to(
				  8
				, 4
			    ) ;
			    pango_cairo_show_layout(
				  c->cobj()
				, layout->gobj()
			    ) ;

			    cairo_image_surface_blur( srf->cobj(), 1 ) ;

			    cairo->set_source( srf, xpos, ypos ) ;
			    cairo->rectangle( xpos, ypos, m_width, rowheight ) ;
			    cairo->set_operator( Cairo::OPERATOR_OVER ) ;
			    cairo->fill() ;
		    }

	            Gdk::Cairo::set_source_rgba(cairo, color);

                    cairo->move_to(
                          xpos + 6
                        , ypos + 2
                    ) ;

                    pango_cairo_show_layout(
                          cairo->cobj()
                        , layout->gobj()
                    ) ;

                    cairo->reset_clip();
                    cairo->restore();
                }
        };

        typedef boost::shared_ptr<Column>       Column_sp_t ;
        typedef std::vector<Column_sp_t>        Column_SP_vector_t ;
        typedef sigc::signal<void>              Signal_0 ;

        class Class 
	: public Gtk::DrawingArea, public Gtk::Scrollable
        {
	    public:

                DataModelFilter_sp_t                m_model ;

	    private:

		typedef Glib::RefPtr<Gtk::Adjustment>	RPAdj ;
		typedef Glib::Property<RPAdj>		PropAdjustment ;

		PropAdjustment	property_vadj_, property_hadj_ ;

                guint                                 m_height__row ;
                guint                                 m_height__current_viewport ;

                boost::optional<boost::tuple<Model_t::iterator, guint, guint> > m_selection ;

                Column_SP_vector_t                  m_columns ;

                std::set<int>                       m_collapsed ;
                std::set<int>                       m_fixed ;
                int                                 m_fixed_total_width ;

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
			  (m_model->size() * m_height__row) + 4
			, m_height__current_viewport
			, 3
		    ) ;
		}

                void
                on_vadj_value_changed ()
                {
                    if( m_model->m_mapping.size() )
                    {
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

                inline bool
                get_row_is_visible (int row)
                {
                    guint up = get_upper_row() ;

                    Interval<guint> i (
                          Interval<guint>::IN_IN
                        , up
                        , up + (m_height__current_viewport / m_height__row)
                    ) ;

                    return i.in( row ) ;
                }

            protected:

                bool
                key_press_event(
                      GdkEventKey* event
                )
                {
                    if( event->is_modifier )
                        return false ;

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

                            default: ;
                        }

                        GdkEvent *new_event = gdk_event_copy( (GdkEvent*)(event) ) ;
                        //g_object_unref( ((GdkEventKey*)new_event)->window ) ;
                        ((GdkEventKey *) new_event)->window = m_SearchWindow->get_window()->gobj();
                        m_SearchEntry->event (new_event) ;
                        //gdk_event_free(new_event) ;

                        return true ;
                    }

                    Limiter<guint> row ;
                    Interval<guint> i ;
                    guint origin = m_selection ? boost::get<2>(m_selection.get()) : 0 ;

                    switch( event->keyval )
                    {
                        case GDK_KEY_Up:
                        case GDK_KEY_KP_Up:
                        case GDK_KEY_Page_Up:
                        {

                            if( origin == 0 )
                                break ;

                            if( !m_selection )
                            {
                                select_row( get_upper_row() ) ;
                                break ;
                            }

                            guint step ;

                            if( event->keyval == GDK_KEY_Page_Up )
                            {
                                step = get_page_size() ;
                            }
                            else
                            {
                                step = 1 ;
                            }

                            row = Limiter<guint> (
                                  Limiter<guint>::ABS_ABS
                                , 0
                                , m_model->size()-1
                                , origin - step
                            ) ;

                            select_row( row ) ;

                            i = Interval<guint> (
                                  Interval<guint>::IN_EX
                                , 0
                                , get_upper_row()
                            ) ;

                            if( i.in( row ))
                            {
                                scroll_to_row( row ) ;
                            }

                            return true;
                        }

                        case GDK_KEY_Home:
                        {
                            scroll_to_row( 0 ) ;
                            select_row( 0 ) ;

                            return true ;
                        }

                        case GDK_KEY_End:
                        {
                            scroll_to_row( m_model->size() - get_page_size() ) ;
                            select_row( m_model->size() - 1 ) ;

                            return true ;
                        }

                        case GDK_KEY_Down:
                        case GDK_KEY_KP_Down:
                        case GDK_KEY_Page_Down:
                        {
                            if( origin == (m_model->size()-1) )
                                break ;

                            if( !m_selection )
                            {
                                select_row( get_upper_row() ) ;
                                break ;
                            }

                            guint step ;

                            if( event->keyval == GDK_KEY_Page_Down )
                            {
                                step = get_page_size() ;
                            }
                            else
                            {
                                step = 1 ;
                            }

                            row = Limiter<guint> (
                                  Limiter<guint>::ABS_ABS
                                , 0
                                , m_model->size() - 1
                                , origin + step
                            ) ;

                            select_row( row ) ;

                            i = Interval<guint> (
                                  Interval<guint>::IN_EX
                                , get_upper_row() + (get_page_size())
                                , m_model->size()
                            ) ;

                            if( i.in( row ))
                            {
                                scroll_to_row( row ) ;
                            }

                            return true ;
                        }

                        default:

                            if( !m_search_active )
                            {
                                gtk_widget_realize( GTK_WIDGET( m_SearchWindow->gobj() ) ); //m_SearchWindow->realize ();

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
                    GdkEvent *event = gdk_event_new (GDK_FOCUS_CHANGE);

                    event->focus_change.type   = GDK_FOCUS_CHANGE;
                    event->focus_change.window = (*m_SearchEntry).get_window()->gobj ();
                    event->focus_change.in     = in;

                    (*m_SearchEntry).send_focus_change( event ) ;
                    (*m_SearchEntry).property_has_focus() = in;

                    //gdk_event_free( event ) ;
                }


                bool
                on_button_press_event (GdkEventButton * event)
                {
                    using boost::get;

                    cancel_search() ;
                    grab_focus() ;

                    if( event->button == 1 && event->type == GDK_2BUTTON_PRESS )
                    {
                        m_SIGNAL_start_playback.emit() ;
                        return false ;
                    }

                    guint row  = vadj_value() / m_height__row ;
                    guint off  = m_height__row - (vadj_value() - (row*m_height__row)) ;

                    if( event->y > off || off == 0 )
                    {
                        guint row2 = row + (event->y + (off ? (m_height__row-off) : 0)) / m_height__row ;

                        if( m_selection && boost::get<2>(m_selection.get()) == row2 )
                            return true ;

                        if( row2 < m_model->size() )
                        {
                            if( row2 >= (row + m_height__current_viewport/m_height__row))
                            {
                            }
                            select_row( row2 ) ;
                        }

                    }
                    else
                    {
                        if( m_selection && boost::get<2>(m_selection.get()) == row )
                            return false ;

                        if(row < m_model->size())
                        {
                            select_row( row ) ;
                        }
                    }

                    return false ;
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
                        configure_vadj(
                              (m_model->size() * m_height__row) + 4
                            , m_height__current_viewport
                            , 3
                        ) ;
                    }

                    double column_width = (double(event->width) - m_fixed_total_width - (40*m_collapsed.size()) ) / double(m_columns.size()-m_collapsed.size()-m_fixed.size());

                    for( guint n = 0 ; n < m_columns.size() ; ++n )
                    {
                        if( m_fixed.count( n ) )
                        {
                            continue ;
                        }

                        if( m_collapsed.count( n ) )
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
                    const Gtk::Allocation& a = get_allocation();

                    boost::shared_ptr<IYoukiThemeEngine> theme = services->get<IYoukiThemeEngine>("mpx-service-theme") ;

                    const ThemeColor& c_base_rules_hint = theme->get_color( THEME_COLOR_BASE_ALTERNATE ) ;
                    const ThemeColor& c_text            = theme->get_color( THEME_COLOR_TEXT ) ;
                    const ThemeColor& c_text_sel        = theme->get_color( THEME_COLOR_TEXT_SELECTED ) ;
                    const ThemeColor& c_bg	= theme->get_color( THEME_COLOR_BACKGROUND ) ;
                    const ThemeColor& c_base	= theme->get_color( THEME_COLOR_BASE ) ;
                    const ThemeColor& c_outline	= theme->get_color( THEME_COLOR_ENTRY_OUTLINE ) ;

                    guint row = get_upper_row() ;

                    guint limit = Limiter<guint>(
                                            Limiter<guint>::ABS_ABS
                                          , 0
                                          , m_model->size()
                                          , get_page_size()
                                      ) + 2 ;

                    guint ypos = 2 ;
                    guint xpos = 0 ;

                    int offset  = vadj_value() - (row*m_height__row) ;

                    if( offset )
                    {
                        ypos -= offset ;
                    }

                    cairo->set_operator( Cairo::OPERATOR_SOURCE ) ;
                    Gdk::Cairo::set_source_rgba(cairo, c_bg);
                    cairo->paint() ;

                    cairo->save() ;
                    RoundedRectangle(
                                     cairo
                                     , 1
                                     , 1
                                     , a.get_width() - 2 
                                     , a.get_height() - 2 
                                     , rounding
                                     ) ;
		    Gdk::Cairo::set_source_rgba(cairo, c_outline) ;
                    cairo->set_line_width( 0.75 ) ;
                    cairo->stroke() ;
                    cairo->restore() ;

                    RoundedRectangle(
                          cairo
                        , 2
                        , 2
                        , a.get_width() - 4 
                        , a.get_height() - 4 
                        , rounding
                    ) ;
                    cairo->clip() ;

                    Gdk::Cairo::set_source_rgba(cairo, c_base) ;
                    cairo->paint() ;

                    cairo->set_operator( Cairo::OPERATOR_OVER ) ;

                    guint n = 0 ;

                    while( (row+n) < m_model->size() && n < limit )
                    {
                        RowRowMapping_t::const_iterator iter = m_model->row_iter( row+n ) ;

                        MPX::CairoCorners::CORNERS c = MPX::CairoCorners::CORNERS(0) ;

                        xpos = 0 ;

                        bool is_selected = (m_selection && boost::get<1>(m_selection.get()) == get<1>(**iter)) ;

                        if( is_selected )
                        {
                            GdkRectangle r ;

                            r.x         = 0 ;
                            r.y         = ypos ;
                            r.width     = a.get_width() ;
                            r.height    = m_height__row ;

                            theme->draw_selection_rectangle(
                                  cairo
                                , r
                                , has_focus()
                                , rounding
                                , c
                            ) ;
                        }
                        else if((row+n) % 2)
                        {
                            GdkRectangle r ;

                            r.x       = 0 ;
                            r.y       = ypos ;
                            r.width   = a.get_width() ;
                            r.height  = m_height__row ;

                            RoundedRectangle(
                                  cairo
                                , r.x
                                , r.y
                                , r.width
                                , r.height
                                , rounding
                                , c
                            ) ;

                            Gdk::Cairo::set_source_rgba(cairo, c_base_rules_hint);
                            cairo->fill() ;
                        }

                        for( Column_SP_vector_t::const_iterator i = m_columns.begin(); i != m_columns.end(); ++i )
                        {
                            (*i)->render(
                                  cairo
                                , **iter
                                ,  *this
                                , row+n
                                , xpos
                                , ypos
                                , m_height__row
                                , is_selected
                                , is_selected ? c_text_sel : c_text
                            ) ;

                            xpos += (*i)->get_width();
                        }

                        ypos += m_height__row;

                        ++n ;
                        ++iter;
                    }

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
                               (m_model->size() * m_height__row) + 4
                               , m_height__current_viewport
                               , 3
                               ) ;

                select_row( position ) ;
                scroll_to_row( position ) ;
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
                        const guint& real_id = id.get() ;

                        for( RowRowMapping_t::iterator i = m_model->m_mapping.begin(); i != m_model->m_mapping.end(); ++i )
                        {
                            if( real_id == get<1>(**i))
                            {
                                guint row = std::distance( m_model->m_mapping.begin(), i ) ;

                                select_row( row ) ;

				if( !get_row_is_visible( row ))
	                                scroll_to_row( row ) ;

                                return ;
                            }
                        }
                    }

                    clear_selection() ;
                }

                void
                scroll_to_row(
                      guint row
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
                            Limiter<guint> d (
                                  Limiter<guint>::ABS_ABS
                                , 0
                                , m_model->m_mapping.size() - get_page_size()
                                , row
                            ) ;

                            vadj_value_set( d * m_height__row ) ;
                        }
                    }
                }

                boost::optional<guint>
                get_selected_id()
                {
                    boost::optional<guint> id ;

                    if( m_selection )
                    {
                        id = get<1>(m_selection.get()) ;
                    }

                    return id ;
                }

                void
                select_row(
                      guint   row
                )
                {
                    if( row < m_model->size() )
                    {
                        const guint& id = get<1>(*m_model->m_mapping[row]) ;

                        m_model->set_selected( id ) ;
                        m_selection = boost::make_tuple( m_model->m_mapping[row], id, row ) ;
                        m_SIGNAL_selection_changed.emit() ;
                        queue_draw();
                    }
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
                    if( m_selection )
                    {
                            const guint& sel_id = boost::get<1>(m_selection.get()) ;

                            if( sel_id != -1 )
                            {
                                return boost::optional<guint>( sel_id ) ;
                            }
                    }

                    return boost::optional<guint>() ;
                }

                void
                clear_selection(
                )
                {
                    m_model->set_selected() ;
                    m_model->m_selected_row.reset() ;
                }

                void
                set_model(
                      DataModelFilter_sp_t  model
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
                      Column_sp_t   column
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
                        m_collapsed.insert( column ) ;
                        queue_resize () ;
                        queue_draw () ;
                    }
                    else
                    {
                        m_collapsed.erase( column ) ;
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
                        m_fixed.insert( column ) ;
                        m_fixed_total_width += width ;
                        m_columns[column]->set_width( width ) ;
                        queue_resize () ;
                        queue_draw () ;
                    }
                    else
                    {
                        m_fixed.erase( column ) ;
                        m_fixed_total_width -= m_columns[column]->get_width() ;
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
                        std::advance( i, get<2>(m_selection.get()) ) ;
                        ++i ;
                    }

		    guint d = std::distance( m_model->m_mapping.begin(), i ) ;

                    for( ; i != m_model->m_mapping.end(); ++i )
                    {
                        Glib::ustring match = Glib::ustring(get<0>(**i)).casefold() ;

                        if( match.length() && match.substr( 0, text.length()) == text.substr( 0, text.length()) )
                        {
                            scroll_to_row( std::max<int>(0, d-get_page_size()/2)) ;
                            select_row( d ) ;
                            break ;
                        }

			++d ;
                    }
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
                        std::advance( i, get<2>(m_selection.get()) ) ;
                        --i ;
                    }

		    guint d = std::distance( m_model->m_mapping.begin(), i ) ;

                    for( ; i >= m_model->m_mapping.begin(); --i )
                    {
                        Glib::ustring match = Glib::ustring(get<0>(**i)).casefold() ;

                        if( match.length() && match.substr( 0, text.length()) == text.substr( 0, text.length()) )
                        {
                            scroll_to_row( std::max<int>(0, d-get_page_size()/2)) ;
                            select_row( d ) ;
                            break ;
                        }

			--d ;
                    }
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
                            scroll_to_row( std::max<int>(0, d-get_page_size()/2)) ;
                            select_row( d ) ;
                            break ;
                        }

			++d ;
                    }
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
                    if( m_search_active )
		    {
			focus_entry( false ) ;
			m_SearchWindow->hide() ;
			m_search_changed_conn.block () ;
			m_SearchEntry->set_text("") ;
			m_search_changed_conn.unblock () ;
			m_search_active = false ;
		    }
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

                        , m_fixed_total_width( 0 )
                        , m_search_active( false )

                {
		    Scrollable::add_interface(G_OBJECT_TYPE(Glib::ObjectBase::gobj())) ;
		    property_vadjustment().signal_changed().connect( sigc::mem_fun( *this, &Class::on_vadj_prop_changed )) ;

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
