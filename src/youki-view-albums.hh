#ifndef YOUKI_VIEW_ALBUMS__HH
#define YOUKI_VIEW_ALBUMS__HH

#include <gtkmm.h>
#include <glibmm/i18n.h>
#include <cairomm/cairomm.h>
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/interprocess/containers/stable_vector.hpp>
#include <sigx/sigx.h>

#include "mpx/mpx-types.hh"
#include "mpx/mpx-main.hh"

#include "mpx/algorithm/limiter.hh"
#include "mpx/algorithm/interval.hh"
#include "mpx/algorithm/minus.hh"
#include "mpx/algorithm/range.hh"
#include "mpx/algorithm/adder.hh"

#include "mpx/com/viewmetrics.hh"
#include "mpx/i-youki-theme-engine.hh"

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
namespace Albums
{
        struct Album
        {
            Cairo::RefPtr<Cairo::ImageSurface>      coverart ;
            Cairo::RefPtr<Cairo::ImageSurface>      surface_cache ;
            Cairo::RefPtr<Cairo::ImageSurface>      surface_cache_blur ;
	    boost::optional<Gdk::RGBA>		    rgba ;
            boost::optional<guint>		    album_id ;
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
            guint				    total_time ;

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

        typedef boost::shared_ptr<Album> Album_sp ;
        typedef boost::container::stable_vector<Album_sp> Model_t ;
        typedef boost::shared_ptr<Model_t> Model_sp ;
        typedef std::map<guint, Model_t::iterator> IdIterMap_t ;
        typedef boost::container::stable_vector<Model_t::iterator>RowRowMapping_t ;


	bool operator==( const Album_sp& a, const Album_sp& b ) ;
	bool operator!=( const Album_sp& a, const Album_sp& b ) ;

	typedef sigc::signal<void>		    Signal_0 ;
	typedef sigc::signal<void, guint>	    Signal_1 ;

        struct OrderFunc
        : public std::binary_function<Album_sp, Album_sp, bool>
	{
	    bool operator() (const Album_sp& a,
			     const Album_sp& b)
	    {
		if( !a->album_id ) 
		    return true ;

		if( !b->album_id ) 
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
		Model_sp               m_base_model ;
		IdIterMap_t            m_iter_map ;

		Signal_0	       m_SIGNAL__redraw ;
		Signal_1	       m_SIGNAL__changed ;
		Signal_1	       m_SIGNAL__cover_updated ;

		guint		       m_upper_bound ;

		DataModel() ;
		DataModel(Model_sp model) ;

		virtual ~DataModel() {}

		virtual void
		clear() ;

		virtual Signal_1&
		signal_changed() ;

		virtual Signal_0&
		signal_redraw() ;

		virtual Signal_1&
		signal_cover_updated() ;

		virtual bool
		is_set() ;

		virtual guint
		size() ;

		virtual const Album_sp&
		row(guint) ;

		virtual void
		set_upper_bound(guint) ;

		virtual void
		append_album(const Album_sp) ;

		virtual void
		insert_album(const Album_sp) ;

		virtual void
		update_album(const Album_sp) ;
		   
		virtual void
 		erase_album(guint) ;

		virtual void
		update_album_cover_cancel(guint) ;

		virtual void
		update_album_cover(
		    guint
		  , Cairo::RefPtr<Cairo::ImageSurface>
		) ;
	} ;

        typedef boost::shared_ptr<DataModel> DataModel_sp;

        struct DataModelFilter
        : public DataModel
        {
            public:

                RowRowMapping_t   m_mapping ;
                TCVector_sp       m_constraints_albums ;
                TCVector_sp       m_constraints_artist ;

            public:

                DataModelFilter(
                      DataModel_sp model
                ) ;

                virtual
                ~DataModelFilter() {}

                virtual void
                set_constraints_albums(
                    TCVector_sp&
                ) ;

                virtual void
                clear_constraints_albums(
                ) ;

                virtual void
                set_constraints_artist(
                    TCVector_sp&
                ) ;

                virtual void
                clear_constraints_artist(
                ) ;

		virtual void
		clear_all_constraints_quiet(
		);

                virtual void
                clear(
		) ;

                virtual guint
                size(
		) ;

                virtual const Album_sp&
                row(
		    guint
                ) ;

                virtual RowRowMapping_t::const_iterator
                iter(
		    guint
                ) ;

                void
                swap(
                      guint   p1
                    , guint   p2
                ) ;

                virtual void
                append_album(
                    const Album_sp album
                ) ;

                virtual void
                insert_album(
                    const Album_sp
                ) ;

                virtual void
                update_album(
                    const Album_sp
                ) ;

                virtual void
                erase_album(
                      guint
                ) ;

                virtual void
                regen_mapping(
                ) ;

                virtual void
                regen_mapping_iterative(
                ) ;
        } ;

        typedef boost::shared_ptr<DataModelFilter> DataModelFilter_sp;

	class Class ;

        class Column
        {
	    protected:

		friend class Class ;

                guint	    m_width ;
                guint	    m_column ;
		bool	    m_show_additional_info ;
		double	    m_rounding ;

                Cairo::RefPtr<Cairo::ImageSurface>  m_image_disc ;
                Cairo::RefPtr<Cairo::ImageSurface>  m_image_new ;
                Cairo::RefPtr<Cairo::ImageSurface>  m_image_lensflare ;
                Cairo::RefPtr<Cairo::ImageSurface>  m_image_dotmask ;

		Glib::RefPtr<Gdk::PixbufAnimation>     m_image_album_loading ;
		Glib::RefPtr<Gdk::PixbufAnimationIter> m_image_album_loading_iter ;

	    public:

                Column(
                ) ;

                virtual ~Column () {}

                void
                set_width(guint) ;

                guint
                get_width() ;

                void
                set_column(guint column) ;

                guint
                get_column() ;

		void
		set_rounding(double) ;

		double	
		get_rounding() ;

	    protected:

		void	
		render_icon(
		    Album_sp
		) ;

		void
		render_rgba(
		    Album_sp
		) ;

	    public:

                void
                render(
                      const Cairo::RefPtr<Cairo::Context>&  cairo
                    , const Album_sp&			    album
                    , const Gtk::Widget&		    widget // FIXME: Do we still need this for the Pango style context in Gtk3?
                    , guint				    row
                    , int                                   ypos
                    , guint				    row_height
                    , const ThemeColor&			    color
                    , const ThemeColor&			    color_sel
                    , const ThemeColor&			    color_bg
                    , guint				    model_mapping_size
		    , guint				    model_size
		    , bool				    selected
		    , const TCVector_sp&		    album_constraints
                ) ;
        } ;

        typedef boost::shared_ptr<Column>     Column_sp ;
        typedef std::vector<Column_sp>        Column_sp_vector_t ;

        
class Class
        : public Gtk::DrawingArea
	, public Gtk::Scrollable
	, public sigx::glib_auto_dispatchable
        {
            public:

                DataModelFilter_sp          m_model ;

            private:

                typedef boost::optional<boost::tuple<Model_t::iterator, boost::optional<guint>, guint> >  Sel_t ;

                Sel_t m_selection ;

		PropAdjustment	    property_vadj, property_hadj ;
		PropScrollPolicy    property_vsp , property_hsp ;

		struct Selection
		{
		    enum sel
		    {
		      ITERATOR
		    , ID
		    , INDEX
		    };
		} ;

                Column_sp_vector_t	    m_columns ;

                Signal_0		    m_SIGNAL_selection_changed ;
                Signal_0		    m_SIGNAL_find_accepted ;
                Signal_0		    m_SIGNAL_start_playback ;

                Interval<guint>			    ModelExtents ;
		Minus<int>			    ModelCount ;

		ViewMetrics_type		    ViewMetrics ;

                Gtk::Entry                        * m_SearchEntry ;
                Gtk::Window                       * m_SearchWindow ;

                sigc::connection                    m_search_changed_conn ;
                bool                                m_search_active ;
	
		bool				    m_button_depressed ;
		double				    m_y_old ;

		std::set<guint>			    m_caching ;
		sigc::connection		    m_redraw_spinner_conn ;

                Glib::RefPtr<Gtk::UIManager>	    m_refUIManager ;
                Glib::RefPtr<Gtk::ActionGroup>	    m_refActionGroup ;
                Gtk::Menu*			    m_pMenuPopup ;

		sigc::connection 		    conn_vadj ;

		boost::shared_ptr<IYoukiThemeEngine> m_theme ; 
		Cairo::RefPtr<Cairo::ImageSurface>   m_background ;

	    public:

                typedef sigc::signal<void, const std::string&>	SignalMBID ;
                typedef sigc::signal<void, guint>		SignalID ; 

                SignalMBID _signal_0 ;
                SignalMBID _signal_1 ;
                SignalID   _signal_2 ;

            protected:

                void
                focus_entry(
		    bool in 
		) ;

                void
                configure_vadj(
                      guint   upper
                    , guint   page_size
                    , guint   step_increment
                ) ;

                void
                on_vadj_value_changed() ;

		void
 		on_vadj_prop_changed() ;

	    protected:    

		virtual bool
		on_motion_notify_event(
		      GdkEventMotion*
		) ;

		virtual bool
		on_focus_in_event(
		    GdkEventFocus*
		) ;

                virtual bool
                on_key_press_event(
		    GdkEventKey*
		) ;

                virtual bool
                on_button_release_event(
		    GdkEventButton*
		) ;

                virtual bool
                on_button_press_event(
		    GdkEventButton*
		) ;

		virtual void
		on_size_allocate(
		    Gtk::Allocation&
		) ;

                virtual bool
                on_configure_event(
                    GdkEventConfigure*
                ) ;

                virtual bool
                on_draw(
		    const Cairo::RefPtr<Cairo::Context>& cairo 
		) ;

	    protected:

                void
                initialize_metrics() ;

		double
		vadj_value() ;

		double
		vadj_upper() ;

		void
		vadj_value_set( double v_ ) ;

	    protected:

                void
                on_model_changed(
                      guint d
                ) ;

		void
		invalidate_covers() ;

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

                void
                on_show_only_this_album() ;

                void
                on_show_only_this_artist() ;

                void
                on_refetch_album_cover() ;

		void
		on_jump_to_selected() ;

		bool
		handle_redraw() ;

		void
		handle_cover_updated( guint id ) ;

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

                void
                clear_selection(
                ) ;

                void
                clear_selection_quiet(
                ) ;

                void
                select_id(
		      boost::optional<guint> id
                ) ;

                void
                scroll_to_index(
                      guint d
                ) ;

                void
                select_index(
                      guint d
                    , bool  quiet = false
                ) ;

                boost::optional<guint>
                get_selected_id() ;

                boost::optional<guint>
                get_selected_index() ;

                boost::optional<guint>
                get_selected() ;

		void
		set_show_additional_info(bool) ;

                void
                set_model(DataModelFilter_sp) ;

		void
		set_rounding(double) ;

                void
                append_column(
                      Column_sp   column
                ) ;

                Class() ;

                virtual ~Class() {}
        };
}
}
}

#endif // _YOUKI_ALBUM_LIST_HH
