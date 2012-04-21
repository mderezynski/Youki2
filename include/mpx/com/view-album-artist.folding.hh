#ifndef YOUKI_VIEW_ARTISTS__HH
#define YOUKI_VIEW_ARTISTS__HH

#include <gtkmm.h>
#include <gtk/gtktreeview.h>
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
            const double rounding = 4. ; 
        }

        typedef boost::tuple<std::string, gint64>           Row_t ;

        typedef IndexedList<Row_t>                          Model_t ;
        typedef boost::shared_ptr<Model_t>                  Model_sp_t ;
        typedef std::map<gint64, Model_t::iterator>         IdIterMap_t ;

        typedef std::vector<Model_t::iterator>              RowRowMapping_t ;

        typedef sigc::signal<void>                          Signal_0 ;
        typedef sigc::signal<void, std::size_t, bool>       Signal_1 ;

        struct OrderFunc
        : public std::binary_function<Row_t, Row_t, bool>
        {
            bool operator() (
                  const Row_t& a
                , const Row_t& b
            ) const 
            {
                int64_t id[2] = { get<1>(a), get<1>(b) } ;

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
                std::size_t                     m_top_row ;
                boost::optional<gint64>         m_selected ;
                boost::optional<std::size_t>    m_selected_row ;
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
                signal_changed ()
                {
                    return m_changed ;
                }

                virtual bool
                is_set ()
                {
                    return bool(m_realmodel);
                }

                virtual std::size_t
                size ()
                {
                    return m_realmodel->size();
                }

                virtual Row_t&
                row (std::size_t row)
                {
                    return (*m_realmodel)[row];
                }

                virtual void
                set_current_row(
                    std::size_t row
                )
                {
                    m_top_row = row ;
                }

                virtual void
                set_selected(
                    const boost::optional<gint64>& row = boost::optional<gint64>()
                )
                {
                    m_selected = row ;
                }

                virtual boost::optional<std::size_t>
                get_selected_row(
                )
                {
                    return m_selected_row ;
                }

                virtual void
                append_artist(
                      const std::string&        artist
                    , gint64                    artist_id
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
                    , gint64                                                                            artist_id
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
                typedef std::vector<gint64>             IdVector_t ;
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
                    m_changed.emit( m_top_row, true ) ;
                } 

                virtual std::size_t 
                size()
                {
                    return m_mapping.size();
                }

                virtual Row_t&
                row(
                      std::size_t row
                )
                {
                    return *(m_mapping[row]);
                }

                virtual RowRowMapping_t::const_iterator 
                row_iter(
                      std::size_t row
                )
                {
                    RowRowMapping_t::const_iterator i  = m_mapping.begin() ; 
                    std::advance( i, row ) ;
                    return i ;
                }

                virtual void
                append_artist(
                      const std::string&        artist
                    , gint64                    artist_id
                )
                {
                    DataModel::append_artist( artist, artist_id ) ;
                }
                
                virtual void
                insert_artist(
                      const std::string&        artist_name
                    , gint64                    artist_id
                )
                {
                    DataModel::insert_artist(
                          artist_name
                        , artist_id
                    ) ;
                }

                virtual void
                erase_artist(
                      gint64                    artist_id
                )
                {
                    IdIterMap_t::iterator i = m_iter_map.find( artist_id ) ;

                    if( i != m_iter_map.end() ) 
                    {
                        Model_t::iterator iter = (*i).second ; 
                        m_realmodel->erase( iter ) ;
                    }
                }

                void
                regen_mapping(
                )
                {
                    using boost::get;

                    if( m_realmodel->size() == 0 )
                    {
                        return ;
                    }

                    RowRowMapping_t new_mapping;
                    new_mapping.reserve( m_realmodel->size() ) ;

                    boost::optional<gint64> id_top ;
                    boost::optional<gint64> id_sel ; 

                    if( m_top_row < m_mapping.size() )
                    {
                        id_top = get<1>(row( m_top_row )) ;
                    }

                    if( m_selected )
                    {
                        id_sel = m_selected ;
                    }

                    m_selected.reset() ;
                    m_selected_row.reset() ;
                    m_top_row = 0 ;

                    Model_t::iterator i = m_realmodel->begin() ; 
                    new_mapping.push_back( i++ ) ;

                    IdVector_t * constraints_artist = m_constraints_artist.get() ;

                    for( ; i != m_realmodel->end(); ++i )
                    {
                        gint64 id_row = get<1>( *i ) ;

                        if( !constraints_artist || (*m_constraints_artist)[id_row] )
                        {
                            if( id_top && id_row == id_top.get() )
                            {
                                m_top_row = new_mapping.size() ;
                            }

                            if( id_sel && id_row == id_sel.get() )
                            {
                                m_selected = id_sel ; 
                                m_selected_row = new_mapping.size() ;
                            }

                            new_mapping.push_back( i ) ;
                        }
                    }

                    if( m_selected_row )
                    {
                        m_top_row = m_selected_row.get() ;
                    }

                    std::swap( m_mapping, new_mapping ) ;
                    update_count() ;
                    m_changed.emit( m_top_row, m_mapping.size() != new_mapping.size() );
                }

                void
                update_count(
                )
                {
                    std::size_t model_size = m_mapping.size() - 1 ; // -1, because of our dummy album

                    Row_t& row = **m_mapping.begin() ;

		    if( model_size != (m_realmodel->size() - 1) )
		    {
                    	get<0>(row) = (boost::format(_("<b>%lld Release %s</b>")) % model_size % ((model_size > 1) ? _("Artists") : _("Artist"))).str() ;
		    }
		    else
		    {
                    	get<0>(row) = _("<b>All Release Artists</b>") ; 
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
                      Cairo::RefPtr<Cairo::Context>     cairo
                    , const Row_t&                      datarow
                    , Gtk::Widget&                      widget
                    , int                               row
                    , int                               xpos
                    , int                               ypos
                    , int                               rowheight
                    , bool                              selected
                    , const ThemeColor&                 color
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
                          (m_width - 8) * PANGO_SCALE
                    ) ;

                    layout->set_alignment( m_alignment ) ;

		    //// FIXME: Get rid of this

                    if( row == 0 )
                    {
                        layout->set_markup( s ) ;
                    }
                    else
                    {
                        layout->set_text( s ) ; 
                    }

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
				  7
				, 3
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

                    cairo->set_source_rgba(
                          color.r
                        , color.g
                        , color.b
                        , color.a
                    ) ;

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
        typedef sigc::signal<void>              Signal_void ;

        class Class : public Gtk::DrawingArea
        {
                int                                 m_height__row ;
                int                                 m_height__current_viewport ;

                DataModelFilter_sp_t                m_model ;
                Column_SP_vector_t                  m_columns ;

                PropAdj                             m_prop_vadj ;
                PropAdj                             m_prop_hadj ;

                boost::optional<boost::tuple<Model_t::iterator, gint64, std::size_t> > m_selection ;

                std::set<int>                       m_collapsed ;
                std::set<int>                       m_fixed ;
                int                                 m_fixed_total_width ;
        
                Signal_void                         m_SIGNAL_selection_changed ;
                Signal_void                         m_SIGNAL_find_accepted ;

                Gtk::Entry                        * m_SearchEntry ;
                Gtk::Window                       * m_SearchWindow ;

                sigc::connection                    m_search_changed_conn ;
                bool                                m_search_active ;

                void
                initialize_metrics ()
                {
                    PangoContext *context = gtk_widget_get_pango_context (GTK_WIDGET (gobj()));

                    PangoFontMetrics *metrics = pango_context_get_metrics(
                          context
                        , GTK_WIDGET (gobj())->style->font_desc
                        , pango_context_get_language (context)
                    ) ;

                    m_height__row =
                        (pango_font_metrics_get_ascent (metrics)/PANGO_SCALE) + 
                        (pango_font_metrics_get_descent (metrics)/PANGO_SCALE) + 5 ;
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

                inline std::size_t
                get_page_size(
                )
                {
                    if( m_height__current_viewport && m_height__row )
                        return m_height__current_viewport / m_height__row ; 
                    else
                        return 0 ;
                }

                inline std::size_t
                get_upper_row ()
                {
                    if( m_prop_vadj.get_value() )
                        return m_prop_vadj.get_value()->get_value() ; 
                    else
                        return 0 ;
                }

                inline std::size_t
                get_lower_row ()
                {
                    return get_upper_row() + get_page_size() ; 
                }

                inline bool
                get_row_is_visible (int row)
                {
                    std::size_t up = get_upper_row() ;

                    Interval<std::size_t> i (
                          Interval<std::size_t>::IN_IN
                        , up 
                        , up + (get_page_size())
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
                            case GDK_Up:
                            case GDK_KP_Up:
                                find_prev_match() ;
                                return true ;

                            case GDK_Down:
                            case GDK_KP_Down:
                                find_next_match() ;
                                return true ;

                            case GDK_Escape:
                                cancel_search() ;
                                return true ;

                            case GDK_Tab:
                                cancel_search() ;
                                return false ; 
        
                            default: ;
                        }

                        GdkEvent *new_event = gdk_event_copy( (GdkEvent*)(event) ) ;
                        g_object_unref( ((GdkEventKey*)new_event)->window ) ;
                        ((GdkEventKey *) new_event)->window = GDK_WINDOW(g_object_ref(G_OBJECT(GTK_WIDGET(m_SearchWindow->gobj())->window))) ;
                        gtk_widget_event(GTK_WIDGET(m_SearchEntry->gobj()), new_event) ;
                        gdk_event_free(new_event) ;

                        return true ;
                    }

                    Limiter<int64_t> row ;
                    Interval<std::size_t> i ;
                    int64_t origin = m_selection ? boost::get<2>(m_selection.get()) : 0 ;

                    switch( event->keyval )
                    {
                        case GDK_Up:
                        case GDK_KP_Up:
                        case GDK_Page_Up:
                        {

                            if( origin == 0 ) 
                                break ;

                            if( !m_selection ) 
                            {
                                select_row( get_upper_row() ) ;
                                break ;
                            }

                            std::size_t step ;

                            if( event->keyval == GDK_Page_Up )
                            {
                                step = get_page_size() ; 
                            }
                            else
                            {
                                step = 1 ; 
                            }
                            
                            row = Limiter<int64_t> ( 
                                  Limiter<int64_t>::ABS_ABS
                                , 0
                                , m_model->size()-1 
                                , origin - step
                            ) ;

                            select_row( row ) ;

                            i = Interval<std::size_t> (
                                  Interval<std::size_t>::IN_EX
                                , 0
                                , get_upper_row()
                            ) ;

                            if( i.in( row )) 
                            {
                                scroll_to_row( row ) ; 
                            }

                            return true;
                        }

                        case GDK_Home:
                        {
                            select_row( 0 ) ;
                            scroll_to_row( 0 ) ;

                            return true ;
                        }

                        case GDK_End:
                        {
                            select_row( m_model->size() - 1 ) ;
                            scroll_to_row( m_model->size() - get_page_size() ) ;

                            return true ;
                        }

                        case GDK_Down:
                        case GDK_KP_Down:
                        case GDK_Page_Down:
                        {
                            if( origin == (m_model->size()-1) )
                                break ;

                            if( !m_selection ) 
                            {
                                select_row( get_upper_row() ) ;
                                break ;
                            }

                            std::size_t step ;

                            if( event->keyval == GDK_Page_Down )
                            {
                                step = get_page_size() ;
                            }
                            else
                            {
                                step = 1 ;
                            }

                            row = Limiter<int64_t> ( 
                                  Limiter<int64_t>::ABS_ABS
                                , 0 
                                , m_model->size() - 1 
                                , origin + step 
                            ) ;

                            select_row( row ) ;

                            i = Interval<std::size_t> (
                                  Interval<std::size_t>::IN_EX
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
                                int x, y, x_root, y_root ;

                                dynamic_cast<Gtk::Window*>(get_toplevel())->get_position( x_root, y_root ) ;

                                x = x_root + get_allocation().get_x() ;
                                y = y_root + get_allocation().get_y() + get_allocation().get_height() ;

                                m_SearchWindow->set_size_request( get_allocation().get_width(), -1 ) ;
                                m_SearchWindow->move( x, y ) ;
                                m_SearchWindow->show() ;

                                send_focus_change( *m_SearchEntry, true ) ;

                                GdkEvent *new_event = gdk_event_copy( (GdkEvent*)(event) ) ;
                                g_object_unref( ((GdkEventKey*)new_event)->window ) ;
                                gtk_widget_realize( GTK_WIDGET(m_SearchWindow->gobj()) ) ;
                                ((GdkEventKey *) new_event)->window = GDK_WINDOW(g_object_ref(G_OBJECT(GTK_WIDGET(m_SearchWindow->gobj())->window))) ;
                                gtk_widget_event(GTK_WIDGET(m_SearchEntry->gobj()), new_event) ;
                                gdk_event_free(new_event) ;

                                m_search_active = true ;
        
                                return false ;
                            }
                    }

                    return false ;
                }

                void
                send_focus_change(
                      Gtk::Widget&  w
                    , bool          in
                    )
                {
                    GtkWidget * widget = w.gobj() ;

                    GdkEvent *fevent = gdk_event_new (GDK_FOCUS_CHANGE);

                    g_object_ref (widget);

                   if( in )
                      GTK_WIDGET_SET_FLAGS( widget, GTK_HAS_FOCUS ) ;
                    else
                      GTK_WIDGET_UNSET_FLAGS( widget, GTK_HAS_FOCUS ) ;

                    fevent->focus_change.type   = GDK_FOCUS_CHANGE;
                    fevent->focus_change.window = GDK_WINDOW(g_object_ref( widget->window )) ;
                    fevent->focus_change.in     = in;

                    gtk_widget_event( widget, fevent ) ;

                    g_object_notify(
                          G_OBJECT (widget)
                        , "has-focus"
                    ) ;

                    g_object_unref( widget ) ;
                    gdk_event_free( fevent ) ;
                }

                bool
                on_button_press_event (GdkEventButton * event)
                {
                    using boost::get;

                    if( event->type == GDK_BUTTON_PRESS )
                    {
                        cancel_search() ;
                        grab_focus() ;

                        std::size_t row = get_upper_row() + ( event->y / m_height__row ) ;

                        if( row < m_model->size() )
                        {
                            select_row( row ) ;
                        }
                    }
                
                    return true;
                }

                bool
                on_button_release_event (GdkEventButton * event)
                {
                    return true ;
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
                      std::size_t   upper
                    , std::size_t   page_size
                    , std::size_t   step_increment
                )
                {
                    if( m_prop_vadj.get_value() )
                    {
                        m_prop_vadj.get_value()->set_upper( upper ) ; 
                        m_prop_vadj.get_value()->set_page_size( page_size ) ; 
                        m_prop_vadj.get_value()->set_step_increment( step_increment ) ; 
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
                              m_model->size()
                            , get_page_size()
                            , 1 
                        ) ;
                    }

                    double column_width = (double(event->width) - m_fixed_total_width - (40*m_collapsed.size()) ) / double(m_columns.size()-m_collapsed.size()-m_fixed.size());

                    for( std::size_t n = 0 ; n < m_columns.size() ; ++n )
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
                on_expose_event (GdkEventExpose *event)
                {
                    const Gtk::Allocation& a = get_allocation();

                    boost::shared_ptr<IYoukiThemeEngine> theme = services->get<IYoukiThemeEngine>("mpx-service-theme") ;

                    Cairo::RefPtr<Cairo::Context> cairo = get_window()->create_cairo_context(); 
    
                    const std::size_t inner_pad  = 2 ;
                    cairo->rectangle(
                          0
                        , 0
                        , a.get_width()
                        , a.get_height() - inner_pad
                    ) ;
                    cairo->clip() ;

                    cairo->set_operator( Cairo::OPERATOR_OVER ) ;

                    const ThemeColor& c_base_rules_hint = theme->get_color( THEME_COLOR_BASE_ALTERNATE ) ;
                    const ThemeColor& c_text            = theme->get_color( THEME_COLOR_TEXT ) ;
                    const ThemeColor& c_text_sel        = theme->get_color( THEME_COLOR_TEXT_SELECTED ) ;

                    std::size_t row = get_upper_row() ; 

                    std::size_t cnt = Limiter<std::size_t>(
                                            Limiter<std::size_t>::ABS_ABS
                                          , 0
                                          , m_model->size()
                                          , get_page_size()
                                      ) + 1 ;

                    std::size_t ypos = inner_pad ;
                    std::size_t xpos = 0 ;

                    RowRowMapping_t::const_iterator row_iter = m_model->row_iter( row ) ;

                    while( row < m_model->size() && cnt )
                    {
                        xpos = 0 ;

                        if( !(row % 2) ) 
                        {
                            GdkRectangle r ;

                            r.x       = inner_pad ;
                            r.y       = ypos ;
                            r.width   = a.get_width() - 2 * inner_pad ; 
                            r.height  = m_height__row ; 

                            RoundedRectangle(
                                  cairo
                                , r.x
                                , r.y
                                , r.width
                                , r.height
                                , rounding
                            ) ;

                            cairo->set_source_rgba(
                                  c_base_rules_hint.r 
                                , c_base_rules_hint.g 
                                , c_base_rules_hint.b 
                                , c_base_rules_hint.a 
                            ) ;

                            cairo->fill() ;
                        }

                        bool iter_is_selected = ( m_selection && boost::get<1>(m_selection.get()) == get<1>(**row_iter)) ;

                        if( iter_is_selected ) 
                        {
                            GdkRectangle r ;

                            r.x         = inner_pad ;
                            r.y         = ypos ;
                            r.width     = a.get_width() - 2*inner_pad ;  
                            r.height    = m_height__row ; 

                            theme->draw_selection_rectangle(
                                  cairo
                                , r
                                , has_focus()
                            ) ;
                        }

                        for( Column_SP_vector_t::const_iterator i = m_columns.begin(); i != m_columns.end(); ++i )
                        {
                            (*i)->render(
                                  cairo
                                , **row_iter 
                                , *this
                                , row
                                , xpos
                                , ypos
                                , m_height__row
                                , iter_is_selected
                                , iter_is_selected ? c_text_sel : c_text
                            ) ;

                            xpos += (*i)->get_width();
                        }

                        ypos += m_height__row;
                        row ++;
                        cnt --;
                        row_iter ++ ;
                    }

                    return true;
                }

                void
                on_model_changed(
                      std::size_t       position
                    , bool              size_changed
                )
                {
                    if( size_changed ) 
                    {
                        configure_vadj(
                              m_model->size()
                            , get_page_size()
                            , 1 
                        ) ;
                    }

                    select_row( position, true ) ; 
                    scroll_to_row( position ) ;

                    queue_draw() ;
                }

                static gboolean
                list_view_set_adjustments(
                    GtkWidget*obj,
                    GtkAdjustment*hadj,
                    GtkAdjustment*vadj, 
                    gpointer data
                )
                {
                    if( vadj )
                    {
                            g_object_set(G_OBJECT(obj), "vadjustment", vadj, NULL); 
                            g_object_set(G_OBJECT(obj), "hadjustment", hadj, NULL);

                            Class & view = *(reinterpret_cast<Class*>(data));

                            view.m_prop_vadj.get_value()->signal_value_changed().connect(
                                sigc::mem_fun(
                                    view,
                                    &Class::on_vadj_value_changed
                            ));
                    }

                    return TRUE;
                }

            public:

                void
                select_id(
                    boost::optional<gint64> id
                )
                {
                    using boost::get;

                    if( id )
                    {
                        const gint64& real_id = id.get() ;

                        for( RowRowMapping_t::iterator i = m_model->m_mapping.begin(); i != m_model->m_mapping.end(); ++i )
                        {
                            if( real_id == get<1>(**i))
                            {
                                std::size_t n = std::distance( m_model->m_mapping.begin(), i ) ;
                                select_row( n ) ;
                                scroll_to_row( n ) ;
                                return ;
                            }
                        }
                    }
                
                    clear_selection() ;
                }

                void
                scroll_to_row(
                      std::size_t row
                )
                {
                    if( m_height__current_viewport && m_height__row && m_prop_vadj.get_value() )
                    {
                        if( m_model->m_mapping.size() < get_page_size() )
                        {
                            m_prop_vadj.get_value()->set_value( 0 ) ;
                        }
                        else
                        {
                            Limiter<std::size_t> d (
                                  Limiter<std::size_t>::ABS_ABS
                                , 0
                                , m_model->m_mapping.size() - get_page_size()
                                , row
                            ) ;

                            m_prop_vadj.get_value()->set_value( d ) ;
                        }
                    }
                }

                void
                select_row(
                      std::size_t   row
                    , bool          quiet = false
                )
                {
                    if( row < m_model->size() )
                    {
                        const gint64& id = get<1>(*m_model->m_mapping[row]) ;

                        m_model->set_selected( id ) ;
                        m_selection = boost::make_tuple( m_model->m_mapping[row], id, row ) ;

                        if( !quiet )
                        {
                            m_SIGNAL_selection_changed.emit() ;
                            queue_draw();
                        }
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

                boost::optional<gint64>
                get_selected()
                {
                    if( m_selection )
                    {
                            const gint64& sel_id = boost::get<1>(m_selection.get()) ;

                            if( sel_id != -1 )
                            {
                                return boost::optional<gint64>( sel_id ) ;
                            }
                    }

                    return boost::optional<gint64>() ;
                }
    
                void
                clear_selection(
                      bool quiet = true
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

                    on_model_changed( 0, true ) ;
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

                    for( ; i != m_model->m_mapping.end(); ++i )
                    {
                        Glib::ustring match = Glib::ustring(get<0>(**i)).casefold() ;

                        if( match.length() && match.substr( 0, text.length()) == text.substr( 0, text.length()) )
                        {
                            std::size_t d = std::distance( m_model->m_mapping.begin(), i ) ; 
                            scroll_to_row( d ) ;
                            select_row( d ) ;
                            return ;
                        }
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

                    for( ; i >= m_model->m_mapping.begin(); --i )
                    {
                        Glib::ustring match = Glib::ustring(get<0>(**i)).casefold() ;

                        if( match.length() && match.substr( 0, text.length()) == text.substr( 0, text.length()) )
                        {
                            std::size_t d = std::distance( m_model->m_mapping.begin(), i ) ; 
                            scroll_to_row( d ) ;
                            select_row( d ) ;
                            return ;
                        }
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

                    for( ; i != m_model->m_mapping.end(); ++i )
                    {
                        Glib::ustring match = Glib::ustring(get<0>(**i)).casefold() ;

                        if( match.length() && match.substr( 0, text.length()) == text.substr( 0, text.length()) )
                        {
                            std::size_t d = std::distance( m_model->m_mapping.begin(), i ) ; 
                            scroll_to_row( d ) ; 
                            select_row( d ) ;
                            return ;
                        }
                    }

                    clear_selection( false ) ;
                    scroll_to_row( 0 ) ;
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
                    if( !m_search_active )
                        return ;

                    send_focus_change( *m_SearchEntry, false ) ;

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
                        , m_prop_vadj( *this, "vadjustment", (Gtk::Adjustment*)( 0 ))
                        , m_prop_hadj( *this, "hadjustment", (Gtk::Adjustment*)( 0 ))
                        , m_fixed_total_width( 0 )
                        , m_search_active( false )

                {
                    boost::shared_ptr<IYoukiThemeEngine> theme = services->get<IYoukiThemeEngine>("mpx-service-theme") ;
                    const ThemeColor& c = theme->get_color( THEME_COLOR_BASE ) ;
                    Gdk::Color cgdk ;
                    cgdk.set_rgb_p( c.r, c.g, c.b ) ; 
                    modify_bg( Gtk::STATE_NORMAL, cgdk ) ;
                    modify_base( Gtk::STATE_NORMAL, cgdk ) ;

                    set_flags(Gtk::CAN_FOCUS);
                    add_events(Gdk::EventMask( GDK_KEY_PRESS_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_LEAVE_NOTIFY_MASK | GDK_BUTTON_PRESS_MASK ));

                    ((GtkWidgetClass*)(G_OBJECT_GET_CLASS(G_OBJECT(gobj()))))->set_scroll_adjustments_signal = 
                            g_signal_new ("set_scroll_adjustments",
                                      G_OBJECT_CLASS_TYPE (G_OBJECT_CLASS (G_OBJECT_GET_CLASS(G_OBJECT(gobj())))),
                                      GSignalFlags (G_SIGNAL_RUN_FIRST),
                                      0,
                                      NULL, NULL,
                                      g_cclosure_user_marshal_VOID__OBJECT_OBJECT, G_TYPE_NONE, 2, GTK_TYPE_ADJUSTMENT, GTK_TYPE_ADJUSTMENT);

                    g_signal_connect(G_OBJECT(gobj()), "set_scroll_adjustments", G_CALLBACK(list_view_set_adjustments), this);

                    m_SearchEntry = Gtk::manage( new Gtk::Entry ) ;
                    gtk_widget_realize( GTK_WIDGET(m_SearchEntry->gobj() )) ;
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
