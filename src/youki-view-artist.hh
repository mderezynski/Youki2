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
#include <boost/interprocess/containers/stable_vector.hpp>
#include <unordered_map>
#include <sigx/sigx.h>

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

#include "mpx/mpx-artist-images.hh"

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
        typedef boost::tuple<std::string, boost::optional<guint>, std::string, Cairo::RefPtr<Cairo::ImageSurface>, Cairo::RefPtr<Cairo::ImageSurface>> Row_t ;

	enum class RowDatum : unsigned int
	{
	      R_ARTIST
	    , R_ID
	    , R_MBID
	    , R_ICON
	    , R_ICON_DESATURATED
	} ;

        typedef boost::container::stable_vector<Row_t>Model_t ;

        typedef boost::shared_ptr<Model_t> Model_sp ;

        typedef std::map<guint, Model_t::iterator> IdIterMap_t ;

        typedef std::vector<Model_t::iterator> RowRowMapping_t ;

        typedef std::unordered_map<std::string, Model_t::iterator> MBIDIterMap_t;

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

	class Class ;
	class DataModelFilter ;
	class DataModel
        : public sigc::trackable
	, public sigx::glib_auto_dispatchable
        {
		friend class Class ;
		friend class DataModelFilter ;

	    protected:

                Model_sp	    m_datamodel ;
                IdIterMap_t         m_iter_map ;
		MBIDIterMap_t	    m_mbid_map ;
                guint		    m_upper_bound ;

                Signal_1	    m_SIGNAL__changed ;

		ArtistImages	  * m_ArtistImages ;

		Glib::RefPtr<Gdk::Pixbuf>	
		get_icon(
		    const std::string& mbid
		) ;

		void
		handle_got_artist_image(
		      const std::string&
		    , Glib::RefPtr<Gdk::Pixbuf>
		) ;

	    public:

                DataModel() ;

                DataModel(
                    Model_sp model
                ) ;

                virtual Signal_1&
                signal_changed()
		{
		    return m_SIGNAL__changed ;
		}

                virtual void
                clear() ;

                virtual bool
                is_set() ;

                virtual guint
                size() ;

                virtual const Row_t&
                row(guint) ;

                virtual void
                set_current_row(
                    guint
                ) ;

                virtual void
                append_artist(
                      const std::string&	    artist
		    , const std::string&	    artist_mbid
                    , const boost::optional<guint>& artist_id = boost::optional<guint>()
                ) ;

                virtual void
                insert_artist(
                      const std::string&	    artist
		    , const std::string&	    artist_mbid
                    , const boost::optional<guint>& artist_id = boost::optional<guint>()
                ) ;
        } ;

        typedef boost::shared_ptr<DataModel> DataModel_sp;

        class DataModelFilter : public DataModel
        {
		friend class Class ;

	    protected:

                typedef std::vector<guint>              IdVector_t ;
                typedef boost::shared_ptr<IdVector_t>   IdVector_sp ;

                RowRowMapping_t        m_mapping ;
                IdVector_sp            m_constraints_artist ;

	    public:

		RowRowMapping_t::const_iterator
		iter(guint idx) ;

                DataModelFilter(DataModel_sp model) ;

                virtual ~DataModelFilter()
                {}

                virtual void
                set_constraints_artist(
                    const IdVector_sp& constraint
                ) ;

                virtual void
                clear_constraints_artist(
                ) ;

                virtual void
                clear() ;

                virtual guint
                size() ;

                virtual const Row_t&
                row(
		    guint
                ) ;

                virtual void
                append_artist(
                      const std::string&	    artist
		    , const std::string&	    artist_mbid
                    , const boost::optional<guint>& artist_id = boost::optional<guint>()
                ) ;

                virtual void
                insert_artist(
                      const std::string&	    artist
		    , const std::string&	    artist_mbid
                    , const boost::optional<guint>& artist_id = boost::optional<guint>()
                ) ;

                virtual void
                erase_artist(
                      guint artist_id
                ) ;

                void
                regen_mapping(
                ) ;

                void
                regen_mapping_iterative(
                ) ;

                void
                update_count(
                ) ;
        } ;

        typedef boost::shared_ptr<DataModelFilter> DataModelFilter_sp;

        class Column
        {
	    private:

                guint				    m_width ;
                guint				    m_column ;
                Pango::Alignment		    m_alignment ;
		double				    m_rounding ;

                Cairo::RefPtr<Cairo::ImageSurface>  m_image_disc ;
                Cairo::RefPtr<Cairo::ImageSurface>  m_rect_shadow ;

		Cairo::RefPtr<Cairo::ImageSurface>
		render_rect_shadow(	
		      guint w
		    , guint h
		) ; 

            public:

                Column() ;

		virtual ~Column()
                {}

                void
                set_rounding(double) ;

                double
                get_rounding() ;

                void
                set_width(guint) ;

                guint
                get_width() ;

                void
                set_column(guint) ;

                guint
                get_column() ;

                void
                set_alignment(
                    Pango::Alignment
                ) ;

                Pango::Alignment
                get_alignment(
                ) ;

		void
		render_header(
		      const Cairo::RefPtr<Cairo::Context>&  cairo
		    , Gtk::Widget&			    widget
		    , int				    xpos
		    , int				    ypos
		    , const ThemeColor&			    color
		) ;

                void
                render(
                      const Cairo::RefPtr<Cairo::Context>&  cairo
                    , const Row_t&			    r
                    , Gtk::Widget&			    widget
                    , int				    d
                    , int				    xpos
                    , int				    ypos
                    , int				    row_height
                    , bool				    selected
                    , const ThemeColor&			    color
                ) ; 
        } ;

        typedef boost::shared_ptr<Column>   Column_sp ;
        typedef std::vector<Column_sp>	    Column_SP_vector_t ;

        class Class 
	: public Gtk::DrawingArea, public Gtk::Scrollable
        {
	    public:

                DataModelFilter_sp		    m_model ;

	    private:

		PropAdjustment			    property_vadj_, property_hadj_ ;
		PropScrollPolicy		    property_vsp_ , property_hsp_ ;

                guint                               m_height__row ;
                guint                               m_height__headers ;
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

		Cairo::RefPtr<Cairo::ImageSurface>  m_background ;

                void
                initialize_metrics() ;

		void
		on_vadj_prop_changed() ;

                void
                on_vadj_value_changed() ;

                inline guint
                get_page_size(
                ) ;

                inline guint
                get_upper_row(
		) ;

                void
                focus_entry(
		    bool in 
		) ;

            protected:

                bool
                on_key_press_event(
		    GdkEventKey*
                ) ;

		void
		on_size_allocate(
		    Gtk::Allocation&
		) ;

                bool
                on_button_press_event(
		    GdkEventButton*
		) ;

                void
                configure_vadj(
                      guint   upper
                    , guint   page_size
                    , guint   step_increment
                ) ;

                bool
                on_configure_event(
                    GdkEventConfigure* event
                ) ;

		void
		render_header_background(
		      const Cairo::RefPtr<Cairo::Context>&  cairo
		    , guint mw
		    , guint mh
		    , const Gdk::RGBA& c_base
		    , const Gdk::RGBA& c_treelines
		) ;

                bool
                on_draw(
		    const Cairo::RefPtr<Cairo::Context>& cairo 
		) ;

		double
		vadj_value() ;

		double
		vadj_upper() ;

		void
		vadj_value_set( double v_ ) ;

		void
		on_model_changed(
		    guint index
		) ;

            public:

                void
                select_id(
                    boost::optional<guint> id
                ) ;

                void
                select_index(
                      guint d
                ) ;

                void
                scroll_to_index(
                      guint d
                ) ;

                boost::optional<guint>
                get_selected_id() ;

                boost::optional<guint>
                get_selected_index() ;

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
                get_selected() ;

                void
                clear_selection(
                ) ;

                void
                set_model(
                      DataModelFilter_sp
                ) ;

		void
		set_rounding(
		      double
		) ;

                void
                append_column(
                      Column_sp
                ) ;

                void
                column_set_collapsed(
                      guint
                    , bool
                ) ;

                void
                column_set_fixed(
                      guint
                    , bool 
                    , guint = 0
                ) ; 

            protected:

                void
                find_next_match() ;

                void
                find_prev_match() ;

                void
                on_search_entry_changed() ;

                void
                on_search_entry_activated() ;

                bool
                on_search_window_focus_out(
                      GdkEventFocus* G_GNUC_UNUSED
                ) ;

            public:

                void
                cancel_search() ;

            protected:

                virtual void
                on_realize() ;

                bool
                query_tooltip(
                      int                                   tooltip_x
                    , int                                   tooltip_y
                    , bool                                  keypress
                    , const Glib::RefPtr<Gtk::Tooltip>&     tooltip
                ) ;

            public:

                Class() ;

                virtual ~Class ()
                {
                }
        };
}
}
}

#endif
