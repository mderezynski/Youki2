#ifndef YOUKI_VIEW_TRACKS_HH
#define YOUKI_VIEW_TRACKS_HH

#include <gtkmm.h>
#include <cairomm/cairomm.h>
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/optional.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <cmath>
#include <deque>
#include <algorithm>
#include <boost/unordered_set.hpp>
#include <sigx/sigx.h>

#include "mpx/util-string.hh"
#include "mpx/util-graphics.hh"

#include "mpx/mpx-types.hh"
#include "mpx/mpx-main.hh"
#include "mpx/mpx-covers.hh"

#include "mpx/algorithm/aque.hh"
#include "mpx/algorithm/ntree.hh"
#include "mpx/algorithm/interval.hh"
#include "mpx/algorithm/limiter.hh"
#include "mpx/algorithm/vector_compare.hh"

#include "mpx/com/indexed-list.hh"
#include "mpx/aux/glibaddons.hh"

#include "mpx/widgets/cairo-extensions.hh"
#include "mpx/widgets/cairo-blur.hh"

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
namespace Tracks
{
        std::string
        RowGetArtistName(
              const MPX::SQL::Row&   r
        )
        {
            std::string name ;

            if( r.count( "album_artist" )) 
            {
                Glib::ustring in_utf8 = boost::get<std::string>(r.find("album_artist")->second) ; 
                gunichar c = in_utf8[0] ;

                if( g_unichar_get_script( c ) != G_UNICODE_SCRIPT_LATIN && r.count("album_artist_sortname") ) 
                {
                        std::string in = boost::get<std::string>( r.find("album_artist_sortname")->second ) ; 

                        boost::iterator_range <std::string::iterator> match1 = boost::find_nth( in, ", ", 0 ) ;
                        boost::iterator_range <std::string::iterator> match2 = boost::find_nth( in, ", ", 1 ) ;

                        if( !match1.empty() && match2.empty() ) 
                        {
                            name = std::string (match1.end(), in.end()) + " " + std::string (in.begin(), match1.begin());
                        }
                        else
                        {
                            name = in ;
                        }

                        return name ;
                }

                name = in_utf8 ;
            }

            return name ;
        }

        namespace
        {
            const double rounding = 1. ; 
        }

        typedef boost::tuple<std::string, std::string, std::string, guint, Track_sp, guint, guint, std::string, std::string, guint>  Row_t ;

	bool operator==(const Row_t& a, const Row_t& b)
	{
	    return boost::get<3>(a) == boost::get<3>(b) ;
	}

	bool operator==( Row_t& a, Row_t& b )
	{
	    return boost::get<3>(a) == boost::get<3>(b) ;
	}

	bool operator!=(const Row_t& a, const Row_t& b)
	{
	    return boost::get<3>(a) == boost::get<3>(b) ;
	}

	bool operator!=( Row_t& a, Row_t& b )
	{
	    return boost::get<3>(a) == boost::get<3>(b) ;
	}

        typedef std::vector<Row_t>			Model_t ;
        typedef boost::shared_ptr<Model_t>		Model_sp_t ;
        typedef sigc::signal<void, guint, bool>	Signal1 ;

        struct Model_t_iterator_equal
        : std::binary_function<Model_t::iterator, Model_t::iterator, bool>
        {
            bool operator()(
                  const Model_t::iterator& a
                , const Model_t::iterator& b
            ) const
            {
                return a == b ;
            }

            bool operator()(
                  const Model_t::const_iterator& a
                , const Model_t::const_iterator& b
            ) const
            {
                return a == b ;
            }

            bool operator()(
                  const Model_t::const_iterator& a
                , const Model_t::iterator& b
            ) const
            {
                return a == b ;
            }

            bool operator()(
                  const Model_t::iterator& a
                , const Model_t::const_iterator& b
            ) const
            {
                return a == b ;
            }
        } ;

        struct Model_t_iterator_hash
        {
            guint operator()( const Model_t::iterator& i ) const
            {
                return GPOINTER_TO_INT(&(*i)) ;
            }

            guint operator()( Model_t::iterator& i ) const
            {
                return GPOINTER_TO_INT(&(*i)) ;
            }

            guint operator()( const Model_t::const_iterator& i ) const
            {
                return GPOINTER_TO_INT(&(*i)) ;
            }

            guint operator()( Model_t::const_iterator& i ) const
            {
                return GPOINTER_TO_INT(&(*i)) ;
            }
        } ;

        struct OrderFunc
        : public std::binary_function<Row_t, Row_t, bool>
        {
            bool operator() (
                  const Row_t&  a
                , const Row_t&  b
            )
            {
		Track_sp t_a = get<4>( a ) ;
		Track_sp t_b = get<4>( b ) ;

                const std::string&  order_artist_a = get<7>( a ) ; 
                const std::string&  order_artist_b = get<7>( b ) ; 

                const std::string&  order_album_a  = get<2>( a ) ; 
                const std::string&  order_album_b  = get<2>( b ) ; 

                const std::string&  order_date_a   = get<8>( a ) ; 
                const std::string&  order_date_b   = get<8>( b ) ; 

                guint order_track [2] = {
                      get<5>( a )
                    , get<5>( b )
                } ;

                if( order_artist_a < order_artist_b)
                    return true ;

                if( order_artist_b < order_artist_a)
                    return false ;

                if( order_date_a < order_date_b )
                    return true ;

                if( order_date_b < order_date_a )
                    return false ;

                if( order_track[0] < order_track[1] )
                    return true ;

                if( order_track[1] < order_track[0] )
                    return false ;

                if( order_album_a < order_album_b )
                    return true ;

                if( order_album_b < order_album_a )
                    return false ;

		if( t_a && t_b && t_a->has(ATTRIBUTE_DISCNR) && t_b->has(ATTRIBUTE_DISCNR))
		{
			guint discnr_a = boost::get<guint>((*t_a)[ATTRIBUTE_DISCNR].get()) ;	
			guint discnr_b = boost::get<guint>((*t_a)[ATTRIBUTE_DISCNR].get()) ;	

	                if( discnr_a < discnr_b )
        	            return true ;

                	if( discnr_b < discnr_a )
	                    return false ;
		}

                return false ;
            }
        };

        struct FindIdFunc
        : public std::binary_function<guint, Model_t::iterator, bool>
        {
            bool operator() (
		  const long long int& id
                , const Model_t::const_iterator& i
            ) const
            {
		return(boost::get<3>(*i) < id) ;
            }
	};


        struct DataModel
        : public sigc::trackable 
        {
		typedef std::vector<Model_t::size_type>		ModelIdxVec_t ;
		typedef std::vector<ModelIdxVec_t>		AlbumTrackMapping_t ;

                Signal1         m_SIGNAL__changed;

                Model_sp_t      m_realmodel;
                guint		m_upper_bound ;

		AlbumTrackMapping_t m_album_track_mapping ;

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
                    m_realmodel->clear () ;
                    m_upper_bound = 0 ;
                } 

                virtual Signal1&
                signal_changed()
                {
                    return m_SIGNAL__changed ;
                }

                virtual bool
                is_set ()
                {
                    return bool(m_realmodel) ;
                }

                virtual guint
                size ()
                {
                    return m_realmodel->size() ;
                }

                inline virtual const Row_t&
                row(guint row)
                {
                    return (*m_realmodel)[row] ;
                }

                virtual void
                set_current_row(
                    guint row
                )
                {
                    m_upper_bound = row ;
                }

                virtual void
                append_track(
                      SQL::Row&             r
                    , const MPX::Track_sp&  track
                )
                {
                    using boost::get ;

                    Row_t row (
                              r.count("title") ? get<std::string>(r["title"]) : ""
                            , Util::row_get_artist_name( r )
                            , r.count("album") ? get<std::string>(r["album"]) : ""
                            , get<guint>(r["id"])
                            , track 
                            , r.count("track") ? get<guint>(r["track"]) : 0
                            , r.count("mpx_album_artist_id") ? get<guint>(r["mpx_album_artist_id"]) : 0
                            , Util::row_get_album_artist_name( r ) 
                            , r.count("mb_release_date") ? get<std::string>(r["mb_release_date"]) : ""
                            , r.count("time") ? get<guint>(r["time"]) : 0
                    ) ;

                    m_realmodel->push_back(row) ;

                    Model_t::iterator i = m_realmodel->end() ;
                    std::advance( i, -1 ) ;

		    guint album_id = get<guint>(r["mpx_album_id"]) ;
		    ModelIdxVec_t& vy = m_album_track_mapping[album_id] ;
		    vy.push_back( m_realmodel->size() - 1 ) ; 
                }

                void
                erase_track(guint id)
                {
                    for( Model_t::iterator i = m_realmodel->begin(); i != m_realmodel->end(); ++i )
                    {
                        guint model_id = get<3>( *i ) ;

                        if( model_id == id )
                        {
                            m_realmodel->erase( i ) ;
                            return ;
                        }
                    }
                }
        };

        typedef boost::shared_ptr<DataModel> DataModel_sp_t;

        struct DataModelFilter
        : public DataModel
        {
                typedef std::vector<guint>		    IdVector_t ;
                typedef boost::shared_ptr<IdVector_t>	    IdVector_sp ;

                typedef std::set<Model_t::const_iterator>	ModelIteratorSet_t ;
                typedef boost::shared_ptr<ModelIteratorSet_t>	ModelIteratorSet_sp ;
                typedef std::vector<ModelIteratorSet_sp>	IntersectVector_t ;
                typedef std::vector<Model_t::const_iterator>	RowRowMapping_t ;
		typedef boost::shared_ptr<RowRowMapping_t>	RowRowMapping_sp ;

                guint  m_max_size_constraints_artist ;
                guint  m_max_size_constraints_albums ;

                typedef std::map<std::string, ModelIteratorSet_sp>  FragmentCache_t ;

                struct IntersectSort
                    : std::binary_function<ModelIteratorSet_sp, ModelIteratorSet_sp, bool>
                {
                        bool operator() (
                              const ModelIteratorSet_sp&  a
                            , const ModelIteratorSet_sp&  b
                        )
                        {
                            return a->size() < b->size() ; 
                        }
                } ;

                RowRowMapping_sp            m_mapping ;
                RowRowMapping_sp            m_mapping_unfiltered ;
                RowRowMapping_sp            m_mapping_identity ;
                FragmentCache_t             m_fragment_cache ;
                std::string                 m_current_filter, m_current_filter_noaque ;
                StrV                        m_frags ;
                AQE::Constraints_t          m_constraints_ext ;
                AQE::Constraints_t          m_constraints_aqe ;
		boost::optional<guint>	    m_constraint_single_artist ;
		boost::optional<guint>	    m_constraint_single_album ;
                boost::optional<guint>      m_id_currently_playing ;
                boost::optional<guint>      m_row_currently_playing_in_mapping ;
                IdVector_sp                 m_constraints_artist ;
                TCVector_sp                 m_constraints_albums ;
                bool                        m_cache_enabled ;

                DataModelFilter( DataModel_sp_t& model )

                    : DataModel( model->m_realmodel )
                    , m_max_size_constraints_artist( 0 )
                    , m_max_size_constraints_albums( 0 )
                    , m_cache_enabled(true)

                {
                    regen_mapping() ;
                }

                virtual ~DataModelFilter()
                {
                }

		void
		shuffle()
		{
			std::random_shuffle( m_mapping->begin(), m_mapping->end() ) ;
                    	m_SIGNAL__changed.emit( 0, false ) ;
		}

                void
                set_sizes(
                      guint    max_size_artist
                    , guint    max_size_albums
                )
                {
                    m_max_size_constraints_artist = max_size_artist + 1 ;
                    m_max_size_constraints_albums = max_size_albums + 1 ;

	            if( m_constraints_albums )
		    	m_constraints_albums->resize( max_size_albums ) ;

		    if( m_constraints_artist )
	                m_constraints_artist->resize( max_size_artist ) ;

		    m_album_track_mapping.resize( m_max_size_constraints_albums ) ;
                }

                void
                get_sizes(
                      guint&   max_size_artist
                    , guint&   max_size_albums
                )
                {
                    max_size_artist = m_max_size_constraints_artist ; 
                    max_size_albums = m_max_size_constraints_albums ; 
                }

                virtual void
                clear()
                {
                    DataModel::clear () ;
                    m_mapping->clear() ;
                    m_mapping_unfiltered->clear() ;
                    clear_active_track() ;
		    m_upper_bound = 0 ;
                    m_SIGNAL__changed.emit( 0, true ) ;
                } 

                void
                clear_active_track()
                {
		    m_id_currently_playing.reset() ;
		    m_row_currently_playing_in_mapping.reset() ;
                }

                boost::optional<guint>
                get_active_track()
                {
                    return m_row_currently_playing_in_mapping ;
                }

                void
                set_active_id(guint id)
                {
		    m_row_currently_playing_in_mapping.reset() ;

                    m_id_currently_playing = id ;
                    scan_for_currently_playing() ;
                }

                void
                clear_fragment_cache()
                {
                    m_fragment_cache.clear() ;
                }

                void
                disable_fragment_cache()
                {
                    m_cache_enabled = false ;
                }

                void
                enable_fragment_cache()
                {
                    m_cache_enabled = true ;
                }

                void
                add_synthetic_constraint(
                    const AQE::Constraint_t& c
                )
                {
                    m_constraints_ext.push_back( c ) ;    
                    regen_mapping() ;
                }

                void
                add_synthetic_constraint_quiet(
                    const AQE::Constraint_t& c
                )
                {
                    m_constraints_ext.push_back( c ) ;    
                }

                virtual void
                clear_synthetic_constraints(
                )
                {
                    m_constraints_ext.clear() ;
                    regen_mapping() ;
                }

                virtual void
                clear_synthetic_constraints_quiet(
                )
                {
                    m_constraints_ext.clear() ;
                }

		virtual void
		clear_single_artist_constraint_quiet()
		{
		    m_constraint_single_artist.reset() ;
		}

		virtual void
		set_constraint_single_artist( guint id )
		{
		    m_constraint_single_artist = id ;
		}

		virtual void
		clear_single_album_constraint_quiet()
		{
		    m_constraint_single_album.reset() ;
		}

		virtual void
		set_constraint_single_album( guint id )
		{
		    m_constraint_single_album = id ;
		}

                virtual guint 
                size()
                {
                    return m_mapping ? m_mapping->size() : 0 ;
                }

                virtual const Row_t&
                row (guint row)
                {
                    return *((*m_mapping)[row]);
                }

                void
                swap( guint p1, guint p2 )
                {
                    std::swap( (*m_mapping)[p1], (*m_mapping)[p2] ) ;

                    m_row_currently_playing_in_mapping.reset() ;
                    scan_for_currently_playing() ;

                    m_SIGNAL__changed.emit( m_upper_bound, false ) ;
                }

                void
                erase( guint p )
                {
                    RowRowMapping_t::iterator i = m_mapping->begin() ;

                    std::advance( i, p ) ;
                    m_mapping->erase( i ) ;

                    m_row_currently_playing_in_mapping.reset() ;
                    scan_for_currently_playing() ;

                    m_SIGNAL__changed.emit( m_upper_bound, true ) ;
                }

                virtual void
                append_track(SQL::Row& r, const MPX::Track_sp& track)
                {
                    DataModel::append_track(r, track);
                }

                void
                erase_track(guint id)
                {
                    DataModel::erase_track( id );
                }

                virtual void
                insert_track(
                      SQL::Row&             r
                    , const MPX::Track_sp&  track
                )
                {
                    const std::string&                    title             = get<std::string>(r["title"]) ;
                    const std::string&                    artist            = Util::row_get_artist_name( r ) ;
                    const std::string&                    album             = get<std::string>(r["album"]) ; 
	
                    std::string release_date ;
		    if( r.count("mb_release_date"))
			release_date = get<std::string>(r["mb_release_date"]) ;

                    guint                                id                = get<guint>(r["id"]) ;
                    guint                                track_n           = get<guint>(r["track"]) ;
                    guint                                time              = get<guint>(r["time"]) ;
                    guint                                id_artist         = get<guint>(r["mpx_album_artist_id"]) ;

                    std::string order_artist ;
		    if( r.count("album_artist_sortname"))
                    	order_artist = get<std::string>(r["album_artist_sortname"]); 

                    static OrderFunc order ;

                    Row_t row(
                          title
                        , artist
                        , album
                        , id
                        , track
                        , track_n 
                        , id_artist
                        , order_artist
                        , release_date
                        , time
                    ) ; 

                    m_realmodel->insert(
                          std::upper_bound(
                              m_realmodel->begin()
                            , m_realmodel->end()
                            , row
                            , order
                          )
                        , row
                    ) ;
                }
 
                virtual void
                set_filter(
                    const std::string& text
                )
                { 
                    using boost::get ;
                    using boost::algorithm::split;
                    using boost::algorithm::is_any_of;
                    using boost::algorithm::find_first;

                    AQE::Constraints_t aqe = m_constraints_aqe ;

                    m_constraints_aqe.clear() ;
                    m_frags.clear() ;

                    AQE::parse_advanced_query( m_constraints_aqe, text, m_frags ) ;

                    bool aqe_diff = m_constraints_aqe != aqe ;

                    if( !aqe_diff && ( text.substr( 0, text.size() - 1 ) == m_current_filter ) )
                    {
                        m_current_filter = text ;
			m_current_filter_noaque = Util::stdstrjoin( m_frags, " " ) ;
                        regen_mapping_iterative() ;
                    }
                    else
                    {
                        m_current_filter = text ;
			m_current_filter_noaque = Util::stdstrjoin( m_frags, " " ) ;
                        //Util::window_set_busy( * dynamic_cast<Gtk::Window*>(m_widget->get_toplevel()) ) ;
                        regen_mapping() ;
                        //Util::window_set_idle( * dynamic_cast<Gtk::Window*>(m_widget->get_toplevel()) ) ;
                    }
                }

                void
                scan(
                      const boost::optional<guint>& id
                )
                {
			scan_for_currently_playing() ;
			scan_for_upper_bound( id ) ;
                }

                void
                scan_for_currently_playing(
                )
                {
                    if( m_id_currently_playing )
                    {
#if 0
			RowRowMapping_t::const_iterator it, first, last, begin = m_mapping->begin() ;

			guint count, step ;

			first = m_mapping->begin() ;
			last = m_mapping->end() ;

			count = std::distance( first, last ) ; 

			while( count > 0 )
			{
			    it = first ;
			    step = count / 2 ;

			    std::advance( it, step ) ;

			    if(!(m_id_currently_playing.get() < boost::get<3>(**it)))
			    {
				first = ++it ;
				count -= step + 1 ;
			    } else count = step ;

			}

			if( first != m_mapping->end() )
			{
			    m_row_currently_playing_in_mapping = std::distance(first,begin) - 1 ;
			    return ;
			}
#endif
			guint d = 0 ;

			for( RowRowMapping_t::const_iterator i = m_mapping->begin() ; i != m_mapping->end() ; ++i )
			{
			    if( m_id_currently_playing.get() == boost::get<3>(**i))
			    {
				m_row_currently_playing_in_mapping = d ;
				return ;
			    }
			    ++d ;
			}
                    }

		    m_row_currently_playing_in_mapping.reset() ;
                }

                void
                scan_for_upper_bound(
                      const boost::optional<guint>& id
                )
                {
                    if( id )
                    {
#if 0
			RowRowMapping_t::const_iterator it, first, last, begin = m_mapping->begin() ;

			guint count, step ;

			first = m_mapping->begin() ;
			last = m_mapping->end() ;

			count = std::distance( first, last ) ; 

			while( count > 0 )
			{
			    it = first ;
			    step = count / 2 ;

			    std::advance( it, step ) ;

			    if(!(id.get() < boost::get<3>(**it)))
			    {
				first = ++it ;
				count -= step + 1 ;
			    } else count = step ;

			}

			if( first != m_mapping->end() )
			{
			    m_upper_bound = std::distance(begin, first) - 1 ;
			    return ;
			}
#endif
			guint d = 0 ;

			for( RowRowMapping_t::const_iterator i = m_mapping->begin() ; i != m_mapping->end() ; ++i )
			{
			    if( id.get() == boost::get<3>(**i))
			    {
				m_upper_bound = d ;
				return ;
			    }
			    ++d ;
			}

                    }

		    m_upper_bound = 0 ;
                }

                virtual void
                cache_current_fragments(
                )
                {
                    using boost::get;
                    using boost::algorithm::split;
                    using boost::algorithm::is_any_of;
                    using boost::algorithm::find_first;

                    if( !m_cache_enabled )
                    {
                        return ;
                    }

                    if( m_frags.empty() )
                    {
                        return ;
                    }

                    std::vector<std::string> vec( 3 ) ;

                    for( guint n = 0 ; n < m_frags.size(); ++n )
                    {
                        if( m_frags[n].empty() )
                        {
                            continue ;
                        }

                        if( m_fragment_cache.count( m_frags[n] )) 
                        {
                            continue ;
                        }
                        else
                        {
                            ModelIteratorSet_sp model_iterator_set ( new ModelIteratorSet_t ) ;

                            for( Model_t::const_iterator i = m_realmodel->begin(); i != m_realmodel->end(); ++i ) // determine all the matches
                            {
                                const Row_t& row = *i;

                                vec[0] = Glib::ustring(boost::get<0>(row)).lowercase() ;
                                vec[1] = Glib::ustring(boost::get<1>(row)).lowercase() ;
                                vec[2] = Glib::ustring(boost::get<2>(row)).lowercase() ;

                                if( Util::match_vec( m_frags[n], vec) )
                                {
                                    model_iterator_set->insert( i ) ; 
                                }
                            }
    
                            m_fragment_cache.insert( std::make_pair( m_frags[n], model_iterator_set )) ;
                        }
                    }
                }

                virtual void
                create_identity_mapping(
                )
                {
                    m_mapping_identity = RowRowMapping_sp( new RowRowMapping_t ) ;
                    m_mapping_identity->reserve( m_realmodel->size() ) ;

                    for( Model_t::iterator i = m_realmodel->begin(); i != m_realmodel->end(); ++i )
                    {
			m_mapping_identity->push_back( i ) ;
                    }
                }

                virtual void
                regen_mapping(
		    boost::optional<guint> scroll_id = boost::optional<guint>()
                )
                {
                    using boost::get;
                    using boost::algorithm::split;
                    using boost::algorithm::is_any_of;
                    using boost::algorithm::find_first;
	
                    RowRowMapping_sp new_mapping( new RowRowMapping_t ), new_mapping_unfiltered( new RowRowMapping_t ) ;

		    boost::optional<guint> id ;

		    if( scroll_id )
		    {
			id = scroll_id ;
		    }
		    else
		    if( m_mapping && m_upper_bound < m_mapping->size() )
		    {
			id = get<3>(row(m_upper_bound)) ;
		    }

                    m_upper_bound = 0 ;

		    if( m_constraint_single_album )
		    {
			AlbumTrackMapping_t::size_type n = m_constraint_single_album.get() ; 
			const ModelIdxVec_t& v = m_album_track_mapping[n] ;

                        new_mapping->reserve( m_realmodel->size() ) ;
                        new_mapping_unfiltered->reserve( m_realmodel->size() ) ;

			for( ModelIdxVec_t::const_iterator i = v.begin() ; i != v.end() ; ++i )
			{
			    Model_t::iterator i2 = m_realmodel->begin() ;
			    std::advance( i2, *i ) ;

			    new_mapping->push_back( i2 ) ; 
			    new_mapping_unfiltered->push_back( i2 ) ; 
			}
		    }
		    else
                    if( m_frags.empty() && (m_constraints_ext.empty() && m_constraints_aqe.empty()) )
                    {
                        m_constraints_albums.reset() ;
                        m_constraints_artist.reset() ;

			m_mapping = m_mapping_identity ;
			m_mapping_unfiltered = m_mapping_identity ;

			if( id )
			{
			    scan_for_upper_bound( id ) ;
			}

			m_SIGNAL__changed.emit( m_upper_bound, true ) ; 

			return ;
                    }
                    else
                    if( m_frags.empty() && !(m_constraints_ext.empty() && m_constraints_aqe.empty()) )
                    {
                        m_constraints_albums = TCVector_sp( new TCVector_t ) ; 
                        m_constraints_albums->resize( m_max_size_constraints_albums + 1 ) ;

                        m_constraints_artist = IdVector_sp( new IdVector_t ) ; 
                        m_constraints_artist->resize( m_max_size_constraints_artist + 1 ) ;

                        TCVector_t& constraints_albums = *(m_constraints_albums.get()) ;
                        IdVector_t& constraints_artist = *(m_constraints_artist.get()) ;

                        new_mapping->reserve( m_realmodel->size() ) ;
                        new_mapping_unfiltered->reserve( m_realmodel->size() ) ;

                        for( Model_t::const_iterator i = m_realmodel->begin(); i != m_realmodel->end(); ++i ) // determine all the matches
                        {
                            const Row_t& row = *i;
                            const MPX::Track_sp& t = get<4>(row);
                            const MPX::Track& track = *(t.get()) ;

                            if( !m_constraints_aqe.empty() && !AQE::match_track( m_constraints_aqe, t ))
                            {
                                continue ;
                            }

                            new_mapping_unfiltered->push_back( i ) ;

                            if( !m_constraints_ext.empty() && !AQE::match_track( m_constraints_ext, t ))
                            {                            
                                continue ;
                            }

                            guint id_album  = get<guint>(track[ATTRIBUTE_MPX_ALBUM_ID].get()) ;
                            guint id_artist = get<6>(*i) ;

			    TracksConstraint& tc = constraints_albums[id_album] ;

                            tc.Count ++ ; 
			    tc.Time += get<guint>(track[ATTRIBUTE_TIME].get()) ;

                            constraints_artist[id_artist] = constraints_artist[id_artist] + 1 ;

                            new_mapping->push_back( i ) ; 
                        }
                    }
                    else
                    {
                        IntersectVector_t intersect ;
                        intersect.reserve( m_frags.size() ) ; 

                        StrV vec( 3 ) ;

                        for( guint n = 0 ; n < m_frags.size(); ++n )
                        {
                            if( m_frags[n].empty() )
                            {
                                continue ;
                            }

                            if( m_cache_enabled ) 
                            {
                                FragmentCache_t::iterator cache_iter = m_fragment_cache.find( m_frags[n] ) ;

                                if( cache_iter != m_fragment_cache.end() )
                                {
                                    intersect.push_back( cache_iter->second ) ;
                                    continue ;
                                }
                            }

                            ModelIteratorSet_sp model_iterator_set( new ModelIteratorSet_t ) ;

                            for( Model_t::const_iterator i = m_realmodel->begin(); i != m_realmodel->end(); ++i )
                            {
                                const Row_t& row = *i;

                                vec[0] = Glib::ustring(boost::get<0>(row)).lowercase() ;
                                vec[1] = Glib::ustring(boost::get<1>(row)).lowercase() ;
                                vec[2] = Glib::ustring(boost::get<2>(row)).lowercase() ;

                                if( Util::match_vec( m_frags[n], vec ))
                                {
                                    model_iterator_set->insert( i ) ; 
                                }
                            }

                            intersect.push_back( model_iterator_set ) ; 

                            if( m_cache_enabled && !m_fragment_cache.count( m_frags[n] ) && m_constraints_ext.empty() && m_constraints_aqe.empty() )
                            {
                                m_fragment_cache.insert( std::make_pair( m_frags[n], model_iterator_set )) ;
                            }
                        }

                        std::sort( intersect.begin(), intersect.end(), IntersectSort() ) ;
                        ModelIteratorSet_sp output( new ModelIteratorSet_t ) ; 

                        if( !intersect.empty() )
                        {
                            output = intersect[0] ;

                            for( guint n = 1 ; n < intersect.size() ; ++n )
                            {
                                ModelIteratorSet_sp intersect_out( new ModelIteratorSet_t ) ;

                                std::set_intersection(

                                    // Set 1
                                      output->begin()
                                    , output->end()

                                    // Set 2 
                                    , intersect[n]->begin()
                                    , intersect[n]->end()

                                    // Result insertion 
                                    , std::inserter(
                                            *intersect_out.get()
                                           , intersect_out->end()
                                      )
                                ) ;

                                output = intersect_out ;
                            }
                        }

                        new_mapping->reserve( output->size() ) ;
                        new_mapping_unfiltered->reserve( output->size() ) ;

                        m_constraints_albums = TCVector_sp( new TCVector_t ) ; 
                        m_constraints_albums->resize( m_max_size_constraints_albums + 1 ) ;

                        m_constraints_artist = IdVector_sp( new IdVector_t ) ; 
                        m_constraints_artist->resize( m_max_size_constraints_artist + 1 ) ;

                        TCVector_t& constraints_albums = *(m_constraints_albums.get()) ;
                        IdVector_t& constraints_artist = *(m_constraints_artist.get()) ;

                        for( ModelIteratorSet_t::iterator i = output->begin() ; i != output->end(); ++i )
                        {
                            const MPX::Track_sp& t = get<4>(**i);
                            const MPX::Track& track = *(t.get()) ;

                            if( !m_constraints_aqe.empty() && !AQE::match_track( m_constraints_aqe, t ))
                            {
                                continue ;
                            }

                            new_mapping_unfiltered->push_back( *i ) ;

                            if( !m_constraints_ext.empty() && !AQE::match_track( m_constraints_ext, t ))
                            {
                                continue ;
                            }

                            guint id_album  = get<guint>(track[ATTRIBUTE_MPX_ALBUM_ID].get()) ;
                            guint id_artist = get<6>(**i) ;

			    TracksConstraint& tc = constraints_albums[id_album] ;

                            tc.Count ++ ; 
			    tc.Time += get<guint>(track[ATTRIBUTE_TIME].get()) ;

                            constraints_artist[id_artist] = constraints_artist[id_artist] + 1 ;

                            new_mapping->push_back( *i ) ;
                        }
                    }

		    if( !m_mapping || !vector_compare( *m_mapping, *new_mapping ))
		    {
                        m_mapping = new_mapping ;
	                m_mapping_unfiltered = new_mapping_unfiltered ;

			if( id )
			{
			    scan_for_upper_bound( id ) ;
			}

			m_SIGNAL__changed.emit( m_upper_bound, true ) ; 
		    }
                }

                void
                regen_mapping_iterative(
		    boost::optional<guint> scroll_id = boost::optional<guint>()
                )
                {
                    using boost::get;
                    using boost::algorithm::split;
                    using boost::algorithm::is_any_of;
                    using boost::algorithm::find_first;

                    RowRowMapping_sp new_mapping( new RowRowMapping_t ), new_mapping_unfiltered( new RowRowMapping_t ) ;

		    boost::optional<guint> id ;

		    if( scroll_id )
		    {
			id = scroll_id ;
		    }
		    if( m_mapping && m_upper_bound < m_mapping->size() )
		    {
			id = get<3>(row(m_upper_bound)) ;
		    }

                    m_upper_bound = 0 ;

		    if( m_constraint_single_album ) // FIXME: This is not iterative
		    {
			AlbumTrackMapping_t::size_type n = m_constraint_single_album.get() ; 
			const ModelIdxVec_t& v = m_album_track_mapping[n] ;

                        new_mapping->reserve( m_realmodel->size() ) ;
                        new_mapping_unfiltered->reserve( m_realmodel->size() ) ;

			for( ModelIdxVec_t::const_iterator i = v.begin() ; i != v.end() ; ++i )
			{
			    Model_t::iterator i2 = m_realmodel->begin() ;
			    std::advance( i2, *i ) ;

			    new_mapping->push_back( i2 ) ; 
			    new_mapping_unfiltered->push_back( i2 ) ; 
			}
		    }
		    else
                    if( m_frags.empty() && (m_constraints_ext.empty() && m_constraints_aqe.empty()) )
                    {
                        m_constraints_albums.reset() ;
                        m_constraints_artist.reset() ;

                        new_mapping->reserve( m_mapping_unfiltered->size() ) ;
                        new_mapping_unfiltered->reserve( m_mapping_unfiltered->size() ) ;

                        for( RowRowMapping_t::iterator i = m_mapping_unfiltered->begin(); i != m_mapping_unfiltered->end(); ++i )
                        {
                            new_mapping->push_back( *i ) ;
                            new_mapping_unfiltered->push_back( *i ) ;
                        }
                    }
                    else
                    if( m_frags.empty() && !(m_constraints_ext.empty() && m_constraints_aqe.empty()) )
                    {
                        m_constraints_albums = TCVector_sp( new TCVector_t ) ; 
                        m_constraints_albums->resize( m_max_size_constraints_albums + 1 ) ;

                        m_constraints_artist = IdVector_sp( new IdVector_t ) ; 
                        m_constraints_artist->resize( m_max_size_constraints_artist + 1 ) ;

                        IdVector_t& constraints_artist = *(m_constraints_artist.get()) ;
                        TCVector_t& constraints_albums = *(m_constraints_albums.get()) ;

                        new_mapping->reserve( m_mapping_unfiltered->size() ) ;
                        new_mapping_unfiltered->reserve( m_mapping_unfiltered->size() ) ;

                        for( RowRowMapping_t::const_iterator i = m_mapping_unfiltered->begin(); i != m_mapping_unfiltered->end(); ++i )
                        {
                            const Row_t& row = **i ;

                            const MPX::Track_sp& t = get<4>(row);
                            const MPX::Track& track = *(t.get()) ;

                            if( !m_constraints_aqe.empty() && !AQE::match_track( m_constraints_aqe, t ))
                            {
                                continue ;
                            }

                            new_mapping_unfiltered->push_back( *i ) ;

                            if( !m_constraints_ext.empty() && !AQE::match_track( m_constraints_ext, t )) 
                            {
                                continue ;
                            }

                            guint id_album  = get<guint>(track[ATTRIBUTE_MPX_ALBUM_ID].get()) ;
                            guint id_artist = get<6>(**i) ;

			    TracksConstraint& tc = constraints_albums[id_album] ;

                            tc.Count ++ ; 
			    tc.Time += get<guint>(track[ATTRIBUTE_TIME].get()) ;

                            constraints_artist[id_artist] = constraints_artist[id_artist] + 1 ;

                            new_mapping->push_back( *i ) ; 
                        }
                    }
                    else
                    {
                        IntersectVector_t intersect ; 
                        intersect.reserve( m_frags.size() ) ;

                        StrV vec( 3 ) ;

                        for( guint n = 0 ; n < m_frags.size(); ++n )
                        {
                            if( m_frags[n].empty() ) 
                            {
                                continue ;
                            }

                            if( m_cache_enabled ) 
                            {
                                FragmentCache_t::iterator cache_iter = m_fragment_cache.find( m_frags[n] ) ;

                                if( cache_iter != m_fragment_cache.end() )
                                {
                                    intersect.push_back( cache_iter->second ) ;
                                    continue ;
                                }
                            }

                            ModelIteratorSet_sp model_iterator_set ( new ModelIteratorSet_t ) ;

                            for( RowRowMapping_t::const_iterator i = m_mapping_unfiltered->begin(); i != m_mapping_unfiltered->end(); ++i )
                            {
                                const Row_t& row = **i ;

                                vec[0] = Glib::ustring(boost::get<0>(row)).lowercase() ;
                                vec[1] = Glib::ustring(boost::get<1>(row)).lowercase() ;
                                vec[2] = Glib::ustring(boost::get<2>(row)).lowercase() ;

                                if( Util::match_vec( m_frags[n], vec ))
                                {
                                    model_iterator_set->insert( *i ) ; 
                                }
                            }

                            intersect.push_back( model_iterator_set ) ; 

                            // We cannot cache results which are preconstrainted i.e. through ext or aqe constraints.
                            // If there is more than 1 frag then the result set, since we go through a previous projection,
                            // the controller mapping (m_mapping_unfiltered) is already constrainted through the preceding frags
                            // i.e. in previous calls to regen_mapping_iterative m_mapping_unfiltered has already become a
                            // projection of the base model through the previous frags

                            if(
                                m_cache_enabled
                                        &&
                                m_frags.size() == 1
                                        &&
                                m_constraints_ext.empty()
                                        &&
                                m_constraints_aqe.empty()
                            )
                            {
                                m_fragment_cache.insert( std::make_pair( m_frags[n], model_iterator_set )) ;
                            }
                        }

                        std::sort( intersect.begin(), intersect.end(), IntersectSort() ) ;
                        ModelIteratorSet_sp output( new ModelIteratorSet_t ) ; 

                        if( !intersect.empty() )
                        {
                            output = intersect[0] ;

                            for( guint n = 1 ; n < intersect.size() ; ++n )
                            {
                                ModelIteratorSet_sp intersect_out( new ModelIteratorSet_t ) ;

                                std::set_intersection(

                                    // Set 1
                                      output->begin()
                                    , output->end()

                                    // Set 2 
                                    , intersect[n]->begin()
                                    , intersect[n]->end()

                                    // Result insertion 
                                    , std::inserter(
                                            *intersect_out.get()
                                           , intersect_out->end()
                                      )
                                ) ;

                                output = intersect_out ;
                            }
                        }

                        new_mapping->reserve( m_realmodel->size() ) ;
                        new_mapping_unfiltered->reserve( m_realmodel->size() ) ; 

                        m_constraints_albums = TCVector_sp( new TCVector_t ) ; 
                        m_constraints_albums->resize( m_max_size_constraints_albums + 1 ) ;

                        m_constraints_artist = IdVector_sp( new IdVector_t ) ; 
                        m_constraints_artist->resize( m_max_size_constraints_artist + 1 ) ;

                        IdVector_t& constraints_artist = *(m_constraints_artist.get()) ;
                        TCVector_t& constraints_albums = *(m_constraints_albums.get()) ;

                        for( ModelIteratorSet_t::iterator i = output->begin() ; i != output->end(); ++i )
                        {
                            const MPX::Track_sp& t = get<4>(**i);
                            const MPX::Track& track = *(t.get()) ;

                            if( !m_constraints_aqe.empty() && !AQE::match_track( m_constraints_aqe, t ))
                            {
                                continue ;
                            }

                            new_mapping_unfiltered->push_back( *i ) ;

                            if( !m_constraints_ext.empty() && !AQE::match_track( m_constraints_ext, t ))
                            {
                                continue ;
                            }

                            guint id_album  = get<guint>(track[ATTRIBUTE_MPX_ALBUM_ID].get()) ;
                            guint id_artist = get<6>(**i) ;

			    TracksConstraint& tc = constraints_albums[id_album] ;

                            tc.Count ++ ; 
			    tc.Time += get<guint>(track[ATTRIBUTE_TIME].get()) ;

                            constraints_artist[id_artist] = constraints_artist[id_artist] + 1 ;

                            new_mapping->push_back( *i ) ;
                        }
                    }

		    if( !m_mapping || !vector_compare( *m_mapping, *new_mapping ))
		    {
                        m_mapping = new_mapping ;
	                m_mapping_unfiltered = new_mapping_unfiltered ;

			if( id )
			{
			    scan_for_upper_bound( id ) ;
			}

			m_SIGNAL__changed.emit( m_upper_bound, true ) ; 
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
                    if( m_width != width )
                    {
                        m_width = width; 
                    }
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
                render_header(
                      const Cairo::RefPtr<Cairo::Context>&  cairo
                    , Gtk::Widget&			    widget
                    , int				    xpos
                    , int				    ypos
                    , int				    rowheight
                    , int				    column
                    , const ThemeColor&			    color
                )
                {
                    using boost::get;

		    cairo->save() ;
                    cairo->rectangle(
                          xpos
                        , ypos
                        , m_width
                        , rowheight
                    ) ;

                    cairo->clip() ;

                    cairo->move_to(
                          xpos + 6
                        , ypos + 4
                    ) ;

                    cairo->set_operator(Cairo::OPERATOR_OVER);
                    cairo->set_source_rgba(color.get_red(), color.get_green(), color.get_blue(), color.get_alpha() * 0.8);

                    Glib::RefPtr<Pango::Layout> layout = widget.create_pango_layout(m_title) ;

                    layout->set_ellipsize(
                          Pango::ELLIPSIZE_END
                    ) ;

                    layout->set_width(
                          (m_width-12)*PANGO_SCALE
                    ) ;

                    layout->set_alignment(
                          m_alignment
                    ) ;

                    pango_cairo_show_layout(
                          cairo->cobj()
                        , layout->gobj()
                    ) ;

                    cairo->reset_clip() ;
		    cairo->restore() ;
                }

                void
                render(
                      const Cairo::RefPtr<Cairo::Context>&  cairo
                    , Gtk::Widget&			    widget
                    , const Row_t&			    datarow
                    , int				    row
                    , int				    xpos
                    , int				    ypos
                    , int				    rowheight
                    , const ThemeColor&			    color
                    , double				    alpha
		    , bool				    highlight
		    , const std::string&		    matches
		    , bool				    selected
                )
                {
		      using boost::get ;

		      cairo->save() ;
		      cairo->set_operator(Cairo::OPERATOR_OVER) ;
		      cairo->rectangle(
			    xpos
			  , ypos
			  , m_width
			  , rowheight
		      ) ;
		      cairo->clip();

		      std::string str ;

		      switch( m_column )
		      {
			    case 0:
				str = get<0>(datarow);
				break;
			    case 1:
				str = get<1>(datarow);
				break;
			    case 2:
				str = get<2>(datarow);
				break;
			    case 3:
				str = get<3>(datarow);
				break;
			    case 5:
				str = boost::lexical_cast<std::string>(get<5>(datarow)) ;
				break;
			    case 9:
				{
				  guint time_ = get<9>(datarow) ;
				  str = ((boost::format("%02d:%02d") % (time_/60) % (time_ % 60)).str()) ;
				}
				break;
		      }

		      Glib::RefPtr<Pango::Layout> layout; 

		      if( !matches.empty() && highlight )
		      {
			std::string highlighted = Util::text_match_highlight( str, matches, "#ffd0d0" ) ;
			layout = widget.create_pango_layout("");
			layout->set_markup( highlighted ) ;
		      }
		      else
		      {
			layout = widget.create_pango_layout( str ) ;
		      }

		      layout->set_ellipsize(
			    Pango::ELLIPSIZE_END
		      ) ;

		      layout->set_width(
			    (m_width - 12) * PANGO_SCALE
		      ) ;

		      layout->set_alignment(
			    m_alignment
		      ) ;

		      if( selected )
  		      {
			    Util::render_text_shadow( layout, xpos+6,ypos+3, cairo, 2, 0.55 ) ;
		      }

		      cairo->set_source_rgba(color.get_red(), color.get_green(), color.get_blue(), color.get_alpha() * alpha);
		      cairo->move_to(
			      xpos + 6
			    , ypos + 3
		      ) ;
		      pango_cairo_show_layout(
			    cairo->cobj()
			  , layout->gobj()
		      ) ;

		      cairo->reset_clip();
		      cairo->restore() ;
                  }
        };

        typedef boost::shared_ptr<Column>               Column_sp_t ;
        typedef std::vector<Column_sp_t>                Columns ;

        typedef sigc::signal<void, MPX::Track_sp, bool> SignalTrackActivated ;
        typedef sigc::signal<void>                      SignalVAdjChanged ;
        typedef sigc::signal<void>                      SignalFindAccepted ;
        typedef sigc::signal<void, const std::string&>  SignalFindPropagate ;

        class Class
        : public Gtk::DrawingArea, public Gtk::Scrollable
        {
            public:

                DataModelFilter_sp_t                m_model ;
		std::deque<std::pair<guint,guint> > m_motion_queue ;

            private:

		PropAdjustment			    property_vadj_, property_hadj_ ;
		PropScrollPolicy		    property_vsp_ , property_hsp_ ;

                int                                 m_height__row ;
                int                                 m_height__headers ;
                int                                 m_height__current_viewport ;

                Interval<guint>			    m_ModelExtents ;
		Interval<guint>			    m_Current_Viewport_I ;

                Columns                             m_columns ;

                boost::optional<boost::tuple<Model_t::const_iterator, guint> >  m_selection ;
    
		enum SelValue
		{
		      S_ITER
		    , S_INDEX
		} ;

                boost::optional<guint>		    m_row__button_press ;

                std::set<guint>                     m_columns__collapsed ;
                std::set<guint>                     m_columns__fixed ;
                guint                               m_columns__fixed_total_width ;
        
                Gtk::Entry                        * m_SearchEntry ;
                Gtk::Window                       * m_SearchWindow ;
                Gtk::HBox                         * m_SearchHBox ;
                Gtk::Button                       * m_SearchButton ;

                sigc::connection                    m_search_changed_conn ; 
                bool                                m_search_active ;

		bool				    m_highlight_matches ;
		bool				    m_play_on_single_tap ;

		int				    vadj_value_old ;

                Glib::RefPtr<Gtk::UIManager>	    m_refUIManager ;
                Glib::RefPtr<Gtk::ActionGroup>	    m_refActionGroup ;
                Gtk::Menu*			    m_pMenuPopup ;

                typedef sigc::signal<void, const std::string&> SignalMBID ;

                SignalMBID _signal_0 ; 
                SignalMBID _signal_1 ; 

                SignalTrackActivated                m_SIGNAL_track_activated ;
                SignalVAdjChanged                   m_SIGNAL_vadj_changed ;
                SignalFindAccepted                  m_SIGNAL_find_accepted ;
                SignalFindPropagate                 m_SIGNAL_find_propagate ;

                void
                initialize_metrics ()
                {
                    Glib::RefPtr<Pango::Context> context = get_pango_context ();
                    Pango::FontMetrics metrics = context->get_metrics(get_style_context()->get_font(), context->get_language());

                    m_height__row = (metrics.get_ascent ()/PANGO_SCALE) +
                                    (metrics.get_descent ()/PANGO_SCALE) + 5 ;

                    const int header_pad = 5 ;

                    m_height__headers = m_height__row + header_pad ;
                }

                void
                on_vadj_value_changed ()
                {
                    if( m_model->size() )
                    {
			m_Current_Viewport_I = Interval<guint> (
			      Interval<guint>::IN_EX
			    , get_upper_row()
			    , get_upper_row() + get_page_size()
			) ;

                        m_model->set_current_row( get_upper_row() ) ;
			queue_draw() ;	
                        m_SIGNAL_vadj_changed.emit() ;
                    }
                }

            protected:

                virtual bool
                on_focus_in_event(GdkEventFocus* G_GNUC_UNUSED)
                {
		    if( m_selection )
		    {
			guint idx = boost::get<S_INDEX>(m_selection.get()) ;

			if( m_Current_Viewport_I( idx ))
			    return true ;
		    }

		    select_index( get_upper_row() ) ;

                    return true ;
                }

                bool
                key_press_event (GdkEventKey * event)
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

                            case GDK_KEY_Return:
                            case GDK_KEY_KP_Enter:
                            case GDK_KEY_ISO_Enter:
                            case GDK_KEY_3270_Enter:
                                cancel_search() ;
                                goto continue_matching ;

                            default: ;
                        }

                        GdkEvent *copy_event = gdk_event_copy( (GdkEvent*)(event) ) ;
                        //g_object_unref( ((GdkEventKey*)copy_event)->window ) ;
                        ((GdkEventKey *) copy_event)->window = m_SearchEntry->get_window()->gobj();

                        m_SearchEntry->event(copy_event) ;
                        //gdk_event_free(copy_event) ;

                        return true ;
                    }

                    continue_matching:

                    int step = 0 ; 
                    int origin = m_selection ? boost::get<1>(m_selection.get()) : 0 ;

                    switch( event->keyval )
                    {
/*
                        case GDK_KEY_Delete:
                        {
                            if( m_selection )
                            {
                                guint p = origin ;
				clear_selection_quiet() ;
                                m_model->erase( p ) ;
                            }
                            return true ;

                        }
*/

                        case GDK_KEY_Return:
                        case GDK_KEY_KP_Enter:
                        case GDK_KEY_ISO_Enter:
                        case GDK_KEY_3270_Enter:
                        {
                            if( m_search_active )
                            {
                                cancel_search() ;
                            }

                            if( m_selection )
                            {
                                using boost::get;

                                MPX::Track_sp track = get<4>(*(get<0>(m_selection.get()))) ;
                                m_SIGNAL_track_activated.emit( track, !(event->state & GDK_CONTROL_MASK) ) ;

                                if( event->state & GDK_CONTROL_MASK )
                                {
                                    clear_selection() ;
                                }
                            }

                            return true;
                        }

                        case GDK_KEY_Up:
                        case GDK_KEY_KP_Up:
                        case GDK_KEY_Page_Up:
                        {
                            if( event->keyval == GDK_KEY_Page_Up )
                            {
                                step = get_page_size() ;
                            }
                            else
                            {
                                step = 1 ;
                            }

                            if( event->state & GDK_SHIFT_MASK )
                            {
                                if( m_ModelExtents( origin - step ))
                                {
                                    m_model->swap( origin, origin-step ) ;
                                    m_selection = boost::make_tuple((*m_model->m_mapping)[origin-step], origin-step) ;
                                }

                                return true ;
                            }

                            if( !m_selection || !get_row_is_visible( origin ))
                            {
                                select_index( get_upper_row() ) ;
                            }
                            else
                            {
                                guint row = std::max<int>( 0, origin-step ) ;

                                if( row < get_upper_row() ) 
                                {
				    if( step == 1 )
					scroll_to_index( get_upper_row() - 1 ) ;
				    else
					scroll_to_index( row ) ;
                                }
    
                                select_index( row ) ;
                            }

                            return true;
                        }

                        case GDK_KEY_Home:
                        {
                            select_index( 0 ) ;
                            scroll_to_index( 0 ) ;

                            return true ;
                        }

                        case GDK_KEY_End:
                        {
                            select_index( m_model->size() - 1 ) ;
                            scroll_to_index( m_model->size() - get_page_size() ) ;

                            return true ;
                        }

                        case GDK_KEY_Down:
                        case GDK_KEY_KP_Down:
                        case GDK_KEY_Page_Down:
                        {
                            if( event->keyval == GDK_KEY_Page_Down )
                            {
                                step = get_page_size() ; 
                            }
                            else
                            {
                                step = 1 ;
                            }

                            if( event->state & GDK_SHIFT_MASK )
                            {
                                if( m_ModelExtents( origin + step ))
                                {
                                    m_model->swap( origin, origin+step ) ;
                                    m_selection = boost::make_tuple((*m_model->m_mapping)[origin+step], origin+step) ;
                                }

                                return true ;
                            }

                            if( !m_selection || !get_row_is_visible( origin ))
                            {
                                select_index( get_upper_row() ) ;
                            }
                            else
                            {
                                guint row = std::min<guint>( origin+step, m_model->size()-1 ) ;

                                if( row >= get_lower_row())
                                {
				    if( step == 1 )
                                        scroll_to_index( get_upper_row() + 1 ) ;
				    else
					scroll_to_index( row ) ;
                                }

                                select_index( row ) ;
                            }

                            return true;
                        }

                        default:

                            if( !m_search_active )
                            {
                                int x, y ;

				get_window()->get_origin( x, y ) ;
				y += get_allocation().get_height() ; 
				x += m_columns[0]->get_width() ;

                                gtk_widget_realize(GTK_WIDGET(m_SearchWindow->gobj()));

                                m_SearchEntry->set_size_request( m_columns[1]->get_width(), - 1 ) ;
                                m_SearchWindow->move( x, y ) ;
                                m_SearchWindow->show() ;

                                focus_entry(true) ;

                                GdkEvent *copy_event = gdk_event_copy( (GdkEvent*)(event) ) ;
                                //g_object_unref( ((GdkEventKey*)copy_event)->window ) ;
                                ((GdkEventKey *) copy_event)->window = m_SearchWindow->get_window()->gobj() ;
                                m_SearchEntry->event( copy_event ) ;
                                //gdk_event_free( copy_event ) ;

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
                on_button_press_event(
		    GdkEventButton* event
		)
                {
                    using boost::get ;

                    guint x_orig = event->x ;

		    Limiter<guint> d ( 
			  Limiter<guint>::ABS_ABS
			, 0
			, m_model->size() - 1
			, get_upper_row() + (event->y-m_height__headers) / m_height__row
		    ) ;

		    cancel_search() ;
		    grab_focus() ;
		    select_index( d ) ;
		    m_row__button_press = d ;

                    if(event->type == GDK_BUTTON_PRESS && event->button == 1)
                    {
                        if( event->y < m_height__headers ) 
                        {
                            guint p = 16 ;

                            for( guint n = 0; n < m_columns.size(); ++n )
                            {
                                int w = m_columns[n]->get_width() ;

                                if( (x_orig >= p) && (x_orig <= p + w) && !m_columns__fixed.count(n) )
                                {
                                    column_set_collapsed( n, !m_columns__collapsed.count(n)) ;
                                    break ;
                                }

                                p += w ;
                            }

                            return true ;
                        }
			else
			if( m_play_on_single_tap )
			{	
			    goto play_single_tap ;
			}
                    }
		    else
                    if(event->type == GDK_2BUTTON_PRESS && event->button == 1)
                    {
                        if( event->y > m_height__headers )
			{
			    play_single_tap:

			    Interval<guint> I (
				  Interval<guint>::IN_EX
				, 0
				, m_model->size()
			    ) ;

			    if( I( d )) 
			    {
				MPX::Track_sp track = get<4>(m_model->row(d)) ;
				m_SIGNAL_track_activated.emit( track, true ) ;
			    }
			}
                    }
		    else 
                    if(event->type == GDK_BUTTON_PRESS && event->button == 3)
                    {
			m_row__button_press.reset() ;
                        m_pMenuPopup->popup(event->button, event->time) ;                            
                    }

                    return true ;
                }

                bool
                on_button_release_event (GdkEventButton * event)
                {
		    process_motion_queue() ;
                    m_row__button_press.reset() ; 
                    return true ;
                }

                bool
                on_motion_notify_event(
                    GdkEventMotion* event
                )
                {
/*
		    if( !m_motion_queue.empty() )
		    {
			process_motion_queue() ;
		    }

                    int x_orig, y_orig;

                    GdkModifierType state;

                    if( event->is_hint )
                    {
                        gdk_window_get_device_position( event->window, event->device, &x_orig, &y_orig, &state ) ;
                    }
                    else
                    {
                        x_orig = int( event->x ) ;
                        y_orig = int( event->y ) ;
                        state  = GdkModifierType( event->state ) ;
                    }

                    guint row = get_upper_row() + ( y_orig - m_height__headers ) / m_height__row ;

                    if( m_row__button_press && row != m_row__button_press.get() ) 
                    {
			if( m_ModelExtents( row )) 
			{
			    m_motion_queue.push_back(std::make_pair( row, m_row__button_press.get())) ;
			    m_row__button_press = row ;
			}
                    }

                    return true ;
*/

		    return false ;
                }

		void
		process_motion_queue()
		{
		    guint last ;

		    while( !m_motion_queue.empty())
		    {
			std::pair<guint,guint> p = m_motion_queue.front() ;
			m_motion_queue.pop_front() ;

			m_model->swap( p.first, p.second ) ;
			last = p.first ;
		    }

		    select_index( last ) ; 
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
                    const double column_width_collapsed = 40. ;

                    m_height__current_viewport = event->height - m_height__headers ;

		    m_Current_Viewport_I = Interval<guint> (
			  Interval<guint>::IN_EX
			, get_upper_row()
			, get_upper_row() + get_page_size()
		    ) ;

                    configure_vadj(
                          m_model->size()
                        , get_page_size()
                        , 8 
                    ) ;

                    int width = event->width - 16 ;

                    double column_width_calculated = (double(width) - double(m_columns__fixed_total_width) - double(column_width_collapsed*double(m_columns__collapsed.size()))) / (m_columns.size() - m_columns__collapsed.size() - m_columns__fixed.size()) ;

                    for( guint n = 0; n < m_columns.size(); ++n )
                    {
                        if( !m_columns__fixed.count( n ) )
                        {
                            m_columns[n]->set_width(m_columns__collapsed.count( n ) ? column_width_collapsed : column_width_calculated ) ; 
                        }
                    }

                    return false;
                }

                inline bool
                compare_id_to_optional(
                      const Row_t&                     row
                    , const boost::optional<guint>&    id
                )
                {
                    return( id && id.get() == boost::get<3>( row )) ;
                }

                template <typename T>
                inline bool
                compare_val_to_optional(
                      const T&                          val
                    , const boost::optional<T>&         cmp
                )
                {
                    return( cmp && cmp.get() == val ) ;
                }

                bool
                on_draw(
		    const Cairo::RefPtr<Cairo::Context>& cairo
		)
                {
                    boost::shared_ptr<IYoukiThemeEngine> theme = services->get<IYoukiThemeEngine>("mpx-service-theme") ;

                    const ThemeColor& c_text        = theme->get_color( THEME_COLOR_TEXT ) ;
                    const ThemeColor& c_text_sel    = theme->get_color( THEME_COLOR_TEXT_SELECTED ) ;
                    const ThemeColor& c_rules_hint  = theme->get_color( THEME_COLOR_BASE_ALTERNATE ) ;
		    const ThemeColor& c_treelines   = theme->get_color( THEME_COLOR_TREELINES ) ;
		    const ThemeColor& c_base	    = theme->get_color( THEME_COLOR_BASE ) ;

		    std::valarray<double> dashes ( 2 ) ;
		    dashes[0] = 1. ; 
	            dashes[1] = 2. ;



		    Gdk::Cairo::set_source_rgba(cairo, c_base);
		    cairo->paint() ;

		    // RENDER HEADER BACKGROUND
		    cairo->save() ;
		    cairo->set_operator( Cairo::OPERATOR_OVER ) ;
		    cairo->rectangle(
			  0
			, 0
			, get_allocated_width()
			, m_height__headers
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

		    Cairo::RefPtr<Cairo::LinearGradient> gr = Cairo::LinearGradient::create( get_allocated_width()/2., 1, get_allocated_width() / 2., m_height__headers - 2 ) ;
		    gr->add_color_stop_rgba( 0., c1.get_red(), c1.get_green(), c1.get_blue(), 1. ) ;
		    gr->add_color_stop_rgba( .35, c1.get_red(), c1.get_green(), c1.get_blue(), 1. ) ;
		    gr->add_color_stop_rgba( 1., c2.get_red(), c2.get_green(), c2.get_blue(), 1. ) ;

		    cairo->set_source( gr ) ;
		    cairo->fill() ;
		    cairo->restore() ;

		    cairo->save() ;
		    cairo->set_antialias( Cairo::ANTIALIAS_NONE ) ;
		    cairo->set_line_width( 1. ) ;
		    cairo->set_source_rgba( c_treelines.get_red(), c_treelines.get_green(), c_treelines.get_blue(), 0.6 ) ;
		    cairo->move_to( 0, m_height__headers ) ;
		    cairo->line_to( get_allocated_width(), m_height__headers ) ;
		    cairo->stroke() ;
		    cairo->restore() ;

		    /// Variables mostly for viewport vertical and horizontal iteration 
                    guint upper_row   = get_upper_row() ;

                    guint row_limit   = Limiter<guint>(
				            Limiter<guint>::ABS_ABS
					  , 0
					  , m_model->size()
					  , get_page_size() + 1
					) ;

                    guint xpos        = 0 ;

		    for( Columns::iterator i = m_columns.begin(); i != m_columns.end(); ++i )
		    {
			(*i)->render_header(
				cairo
			      , *this
			      , xpos
			      , 0
			      , m_height__headers
			      , std::distance( m_columns.begin(), i )
			      , c_text
			) ;

			xpos += (*i)->get_width() ; 
		    }

		    // RULES HINT
		    {
			GdkRectangle rect ;

			rect.x       = 0 ;
			rect.width   = get_allocated_width() ; 
			rect.height  = m_height__row ; 

			Gdk::Cairo::set_source_rgba(cairo, c_rules_hint);

			for( guint n = 0 ; n < row_limit ; ++n ) 
			{
			    if( n % 2 )
			    {
				rect.y = m_height__headers + (n*m_height__row) ;

				RoundedRectangle(
				      cairo
				    , rect.x
				    , rect.y
				    , rect.width
				    , rect.height
				    , rounding
				    , MPX::CairoCorners::CORNERS(0)
				) ;

				cairo->fill() ;
			    }
			}
		    }

		    // SELECTION
		    boost::optional<guint> d_sel ;

		    if( m_selection )
		    {
			d_sel = boost::get<1>(m_selection.get()) ; 
		    }

		    // Selection Rectangle, if any
		    if( d_sel && m_Current_Viewport_I(d_sel.get()))
		    {
			GdkRectangle rect ;

			rect.x         = 0 ; 
			rect.width     = get_allocated_width() ;
			rect.height    = m_height__row ; 

			rect.y = m_height__headers + ((d_sel.get() - upper_row)*m_height__row) ;

			theme->draw_selection_rectangle(
			      cairo
			    , rect
			    , has_focus()
			    , rounding
			    , MPX::CairoCorners::CORNERS(0)
			) ;
		    }

		    // ROW DATA
		    for( guint n = 0 ; n < row_limit && m_ModelExtents( n + upper_row ) ; ++n ) 
		    {
			xpos = 0 ;

			const Row_t& r = m_model->row( n + upper_row ) ;

			// RENDER "playing" ARROW
			if( compare_id_to_optional( r, m_model->m_id_currently_playing )) 
			{
			    const guint x = 4, y = m_height__headers + n*m_height__row + 2 ;

			    cairo->save() ;
			    cairo->set_line_join( Cairo::LINE_JOIN_ROUND ) ;
			    cairo->set_line_cap( Cairo::LINE_CAP_ROUND ) ;
			    cairo->move_to( x+4, y+3 ) ; 
			    cairo->line_to( x+13, y+9 ) ; 
			    cairo->line_to( x+4, y+15 ) ;
			    cairo->close_path() ;

			    if( m_selection && boost::get<1>(m_selection.get()) == n + upper_row )
			    { 
				cairo->set_source_rgba( 1., 1., 1., 0.9 ) ;	
			    }
			    else
			    {
				cairo->set_source_rgba( 0.2, 0.2, 0.2, 0.9 ) ;
			    }
		    
			    cairo->fill() ;	
			    cairo->restore() ;
			}

			for( Columns::const_iterator i = m_columns.begin(); i != m_columns.end(); ++i )
			{
			    (*i)->render(
				  cairo
				, *this
				, r 
				, n + upper_row
				, xpos
				, m_height__headers + (n*m_height__row) - 1
				, m_height__row
				, compare_val_to_optional( n + upper_row, d_sel ) ? c_text_sel : c_text
				, 1.0
				, m_highlight_matches
				, m_model->m_current_filter_noaque
				, compare_val_to_optional( n + upper_row, d_sel )
			    ) ;

			    xpos += (*i)->get_width() ; 
			}
		    }

		    // TREELINES
		    {
			Columns::iterator i2 = m_columns.end() ;
			std::advance( i2, -1 ) ;	

			std::vector<guint> xpos_v ;

			guint xpos = 0 ;

			for( Columns::const_iterator i = m_columns.begin() ; i != i2; ++i )
			{
			    xpos += (*i)->get_width() ; // adjust us to the column's end
			    xpos_v.push_back( xpos ) ; 
			}

			std::vector<guint>::iterator ix = xpos_v.end() ;
			std::advance( ix, -1 ) ;

			for( std::vector<guint>::iterator i = xpos_v.begin() ; i != xpos_v.end() ; ++i ) 
			{
			    guint xpos = *i ; 

			    cairo->save() ;
			    cairo->set_antialias( Cairo::ANTIALIAS_NONE ) ;
			    cairo->set_line_width(
				  1. 
			    ) ;
			    cairo->move_to(
				  xpos
				, m_height__headers + 1 
			    ) ; 
			    cairo->line_to(
				  xpos
				, get_allocated_height() + m_height__headers
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

                            cairo->save() ;
                            cairo->set_antialias( Cairo::ANTIALIAS_NONE ) ;
                            cairo->set_line_width(
                                  1.
                            ) ;
                            cairo->move_to(
                                  xpos
                                , 0
                            ) ;
                            cairo->line_to(
                                  xpos
                                , m_height__headers - 1
                            ) ;
                            cairo->set_source_rgba(
                                  c_treelines.get_red()
                                , c_treelines.get_green()
                                , c_treelines.get_blue()
                                , 0.4
                            ) ;

                            cairo->stroke() ;
                            cairo->restore();
		        }
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
                      guint   position
                    , bool    size_changed
                )
                {
                    if( size_changed ) 
                    {
                        m_ModelExtents = Interval<guint> (
			      Interval<guint>::IN_EX
			    , 0
			    , m_model->size()
                        ) ;

                        configure_vadj(
                              m_model->size()
                            , get_page_size()
                            , 8
                        ) ;
                    }

                    scroll_to_index( position ) ;

		    if( m_selection )	
		    {
		    	MPX::View::Tracks::Model_t::const_iterator i = boost::get<0>( m_selection.get() ) ;
			MPX::View::Tracks::DataModelFilter::RowRowMapping_t::const_iterator i_mapping = std::find( m_model->m_mapping->begin(), m_model->m_mapping->end(), i ) ;

			if( i_mapping != m_model->m_mapping->end() )
			{
				MPX::View::Tracks::DataModelFilter::RowRowMapping_t::const_iterator i_mapping_const_begin = m_model->m_mapping->begin() ; 

                        	m_selection = boost::make_tuple( *i_mapping, std::distance( i_mapping_const_begin, i_mapping)) ;
			}
			else
				clear_selection() ;
		    }

		    queue_draw() ;
                }

                bool
                query_tooltip(
                      int                                   tooltip_x
                    , int                                   tooltip_y
                    , bool                                  keypress
                    , const Glib::RefPtr<Gtk::Tooltip>&     tooltip
                )
                {
                    guint row = (double( tooltip_y ) - m_height__headers) / double(m_height__row) ;

                    MPX::Track_sp t = boost::get<4>( m_model->row(row) ) ;
                    const MPX::Track& track = *(t.get()) ;

                    boost::shared_ptr<Covers> covers = services->get<Covers>("mpx-service-covers") ;
                    Glib::RefPtr<Gdk::Pixbuf> cover ;

                    const std::string& mbid = boost::get<std::string>( track[ATTRIBUTE_MB_ALBUM_ID].get() ) ;

                    Gtk::Image * image = Gtk::manage( new Gtk::Image ) ;

                    if( covers->fetch(
                          mbid
                        , cover
                    ))
                    {   
                        image->set( cover ) ;
                        tooltip->set_custom( *image ) ;
                        return true ;
                    }

                    return false ;
                }

            public:

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
                get_upper_row(
                )
                {
                    if( property_vadjustment().get_value() )
                        return property_vadjustment().get_value()->get_value() ;
                    else
                        return 0 ;
                }

                inline guint
                get_lower_row(
                )
                {
                    return property_vadjustment().get_value()->get_value() + get_page_size() ;
                }

                inline bool
                get_row_is_visible(
                      guint   row
                )
                {
                    guint up = get_upper_row() ;

                    Interval<guint> I (
                          Interval<guint>::IN_IN
                        , up 
                        , up + get_page_size()
                    ) ;
            
                    return I( row ) ;
                }

                void
                set_highlight(bool highlight)
                {
                    m_highlight_matches = highlight;
                    queue_draw ();
                }

		void
		set_play_on_single_tap(bool tap)
		{
		    m_play_on_single_tap = tap ;
		}

                void
                set_model(DataModelFilter_sp_t model)
                {
                    if( m_model )
                    {
                        boost::optional<guint> active_track = m_model->m_id_currently_playing ;

                        m_model = model;
                        m_model->m_id_currently_playing = active_track ;
                        m_model->scan_for_currently_playing() ;
                    }
                    else
                    {
                        m_model = model;
                    }

                    m_model->signal_changed().connect(
                        sigc::mem_fun(
                            *this,
                            &Class::on_model_changed
                    ));

                    on_model_changed( 0, true ) ;
                }

                void
                append_column(
                      Column_sp_t   column
                )
                {
                    m_columns.push_back(column) ;
                }

                SignalTrackActivated&
                signal_track_activated()
                {
                    return m_SIGNAL_track_activated ;
                }

                SignalVAdjChanged&
                signal_vadj_changed()
                {
                    return m_SIGNAL_vadj_changed ;
                }

                SignalFindAccepted&
                signal_find_accepted()
                {
                    return m_SIGNAL_find_accepted ;
                }

                SignalFindPropagate&
                signal_find_propagate()
                {
                    return m_SIGNAL_find_propagate ;
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
                    }
                    else
                    {
                        m_columns__collapsed.erase( column ) ;
                        queue_resize () ;
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
                    }
                    else
                    {
                        m_columns__fixed.erase( column ) ;
                        m_columns__fixed_total_width -= m_columns[column]->get_width() ; 
                        queue_resize () ;
                    }
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

                void
                scroll_to_id(
                      guint id
                )
                {
		    guint row = 0 ;

                    for( DataModelFilter::RowRowMapping_t::iterator i = m_model->m_mapping->begin() ; i != m_model->m_mapping->end(); ++i )
                    {
                        if( boost::get<3>(**i) == id )
                        {
                            Limiter<guint> d ( 
                                  Limiter<guint>::ABS_ABS
                                , 0
                                , m_model->m_mapping->size() - get_page_size()
                                , row 
                            ) ;

                            vadj_value_set( d ) ; 
                            break ;
                        }

			++ row ;
                    } 
                }

                void
                scroll_to_index(
                      guint row
                )
                {
                    if( m_height__current_viewport && m_height__row && m_model )
                    {
                        Limiter<guint> d ( 
                              Limiter<guint>::ABS_ABS
                            , 0
                            , m_model->m_mapping->size() - get_page_size()
                            , row 
                        ) ;

                        if( m_model->m_mapping->size() < get_page_size()) 
                            vadj_value_set( 0 ) ; 
                        else
                        if( row > (m_model->m_mapping->size() - get_page_size()) )
                            vadj_value_set( m_model->m_mapping->size() - get_page_size() ) ; 
                        else
                            vadj_value_set( d ) ; 
                    }
                }

                void
                select_index(
                      guint d
                )
                {
                    Interval<guint> I (
                          Interval<guint>::IN_EX
                        , 0
                        , m_model->size()
                    ) ;

                    if( I( d ))
                    {
                        m_selection = boost::make_tuple((*m_model->m_mapping)[d], d) ;
                        queue_draw() ;
                    }
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

            protected:

                void
                find_next_match()
                {
                    using boost::get ;

                    Glib::ustring text = m_SearchEntry->get_text().casefold() ;

		    bool numeric = false ;
		    guint nr ;

		    try{
			nr = boost::lexical_cast<int>( text ) ;
			numeric = true ;
		    } catch(...) {}

                    DataModelFilter::RowRowMapping_t::iterator i = m_model->m_mapping->begin(); 

                    if( m_selection )
                    {
                        std::advance( i, get<1>(m_selection.get()) ) ;
                        ++i ;
                    }

		    guint d = std::distance( m_model->m_mapping->begin(), i ) ;

                    for( ; i != m_model->m_mapping->end(); ++i )
                    {
			if( numeric && nr == get<5>(**i )) 
			{
			    scroll_to_index( std::max<int>( 0, d-get_page_size()/2)) ;
			    select_index( d ) ;
			    return ;
			}

                        Glib::ustring match = Glib::ustring(get<0>(**i)).casefold() ;

                        if( match.length() && match.substr( 0, text.length()) == text.substr( 0, text.length()) )
                        {
                            scroll_to_index( std::max<int>( 0, d-get_page_size()/2)) ;
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

		    bool numeric = false ;
		    guint nr ;

		    try{
			nr = boost::lexical_cast<int>( text ) ;
			numeric = true ;
		    } catch(...) {}

                    DataModelFilter::RowRowMapping_t::iterator i = m_model->m_mapping->begin(); 

                    if( m_selection )
                    {
                        std::advance( i, get<1>(m_selection.get()) ) ;
                        --i ; 
                    }

		    guint d = std::distance( m_model->m_mapping->begin(), i ) ;

                    for( ; i >= m_model->m_mapping->begin(); --i )
                    {
			if( numeric && nr == get<5>(**i )) 
			{
			    scroll_to_index( std::max<int>( 0, d-get_page_size()/2)) ;
			    select_index( d ) ;
			    return ;
			}

                        Glib::ustring match = Glib::ustring(get<0>(**i)).casefold() ;

                        if( match.length() && match.substr( 0, text.length()) == text.substr( 0, text.length()) )
                        {
                            scroll_to_index( std::max<int>( 0, d-get_page_size()/2)) ;
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
			scroll_to_index(0) ;
			select_index(0) ;
			m_SearchEntry->unset_color() ;
			return ;
		    }

		    bool numeric = false ;
		    guint nr ;

		    try{
			nr = boost::lexical_cast<int>( text ) ;
			numeric = true ;
		    } catch(...) {}

                    DataModelFilter::RowRowMapping_t::iterator i = m_model->m_mapping->begin(); 

		    guint d = 0 ; 

                    for( ; i != m_model->m_mapping->end(); ++i )
                    {
			if( numeric && nr == get<5>(**i )) 
			{
			    scroll_to_index( std::max<int>( 0, d-get_page_size()/2)) ;
			    select_index( d ) ;
			    m_SearchEntry->unset_color() ;
			    return ;
			}

                        Glib::ustring match = Glib::ustring(get<0>(**i)).casefold() ;

                        if( match.length() && match.substr( 0, text.length()) == text.substr( 0, text.length()) )
                        {
                            scroll_to_index( std::max<int>( 0, d-get_page_size()/2)) ;
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
                on_search_button_clicked(
                )
                {
                    std::string text = m_SearchEntry->get_text() ;
                    cancel_search() ;

                    m_SIGNAL_find_propagate.emit( text ) ;
                }

                void
                on_show_only_this_album() 
                {
                    if( m_selection )
                    {
                        const Row_t& row = *(boost::get<0>(m_selection.get())) ;
                        const MPX::Track_sp& t = get<4>(row);
                        const MPX::Track& track = *(t.get()) ;

                        _signal_0.emit( get<std::string>(track[ATTRIBUTE_MB_ALBUM_ID].get()));
                    }
                }

                void
                on_show_only_this_artist() 
                {
                    if( m_selection )
                    {
                        const Row_t& row = *(boost::get<0>(m_selection.get())) ;
                        const MPX::Track_sp& t = get<4>(row);
                        const MPX::Track& track = *(t.get()) ;

                        _signal_1.emit( get<std::string>(track[ATTRIBUTE_MB_ALBUM_ARTIST_ID].get()));
                    }
                }

		void
		on_add_track_to_queue()
		{	
		    if( m_selection )
		    {
			MPX::Track_sp track = get<4>(*(get<0>(m_selection.get()))) ;
			m_SIGNAL_track_activated.emit( track, false ) ;
			clear_selection() ;
		    }
		}

                void
                on_shuffle_tracklist() 
                {
		    m_model->shuffle() ;
                }

            public:

                void
                cancel_search()
                {
                    if( m_search_active )
		    {
			focus_entry(false) ;
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

		void
		on_vadj_prop_changed()
		{
		    if( !property_vadjustment().get_value())
			return ;

		    property_vadjustment().get_value()->signal_value_changed().connect(
			sigc::mem_fun(
			    *this,
			    &Class::on_vadj_value_changed
		    ));

                    configure_vadj(
                          m_model->size()
                        , get_page_size()
                        , 8
                    ) ;
		}

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

                Class()

                        : ObjectBase( "YoukiClassTracks" )

			, property_vadj_(*this, "vadjustment", RPAdj(0))
			, property_hadj_(*this, "hadjustment", RPAdj(0))

			, property_vsp_(*this, "vscroll-policy", Gtk::SCROLL_NATURAL )
			, property_hsp_(*this, "hscroll-policy", Gtk::SCROLL_NATURAL )

                        , m_columns__fixed_total_width(0)
                        , m_search_active(false)
                        , m_highlight_matches(false)
			, m_play_on_single_tap(false)
			, vadj_value_old(0)

                {
		    property_vadjustment().signal_changed().connect( sigc::mem_fun( *this, &Class::on_vadj_prop_changed )) ;

                    boost::shared_ptr<IYoukiThemeEngine> theme = services->get<IYoukiThemeEngine>("mpx-service-theme") ;
                    const ThemeColor& c = theme->get_color( THEME_COLOR_BASE ) ;

                    Gdk::RGBA bg1 ;
                    bg1.set_rgba( c.get_red(), c.get_green(), c.get_blue() ) ;
                    override_background_color( bg1, Gtk::STATE_FLAG_NORMAL ) ;

                    set_can_focus(true);

                    add_events(Gdk::EventMask(GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK | GDK_BUTTON_PRESS_MASK | GDK_EXPOSURE_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_LEAVE_NOTIFY_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_BUTTON_MOTION_MASK | GDK_SCROLL_MASK ));

                    /*
                    signal_query_tooltip().connect(
                        sigc::mem_fun(
                              *this
                            , &Class::query_tooltip
                    )) ;

                    set_has_tooltip(true) ;
                    */

                    m_SearchEntry = Gtk::manage( new Gtk::Entry ) ;
                    m_SearchEntry->show() ;

                    m_SearchHBox = Gtk::manage( new Gtk::HBox ) ;
                    m_SearchButton = Gtk::manage( new Gtk::Button ) ;
                    m_SearchButton->signal_clicked().connect(
                        sigc::mem_fun(
                              *this
                            , &Class::on_search_button_clicked
                    )) ;
		    m_SearchButton->set_tooltip_text(_("Click this button to apply this as a search.")) ;

                    Gtk::Image * img = Gtk::manage( new Gtk::Image ) ;
                    img->set( Gtk::Stock::FIND, Gtk::ICON_SIZE_MENU ) ;
                    img->show() ;
    
                    m_SearchButton->set_image( *img ) ;

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
                    m_SearchWindow->set_decorated(false) ;
                    m_SearchWindow->set_border_width(4) ;

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

                    signal_key_press_event().connect(
                            sigc::mem_fun(
                                  *this
                                , &Class::key_press_event
                    ), true ) ;

                    m_SearchHBox->pack_start( *m_SearchEntry, true, true, 0 ) ;
                    m_SearchHBox->pack_start( *m_SearchButton, true, true, 0 ) ;

                    m_SearchWindow->add( *m_SearchHBox ) ;
                    m_SearchHBox->show_all() ;

                    m_refActionGroup = Gtk::ActionGroup::create() ;
                    m_refActionGroup->add( Gtk::Action::create("ContextMenu", "Context Menu")) ;

                    m_refActionGroup->add( Gtk::Action::create("ContextShowAlbum", "Show this Album"),
                        sigc::mem_fun(*this, &Class::on_show_only_this_album)) ;
                    m_refActionGroup->add( Gtk::Action::create("ContextShowArtist", "Show this Artist"),
                        sigc::mem_fun(*this, &Class::on_show_only_this_artist)) ;
                    m_refActionGroup->add( Gtk::Action::create("ContextRandomShuffle", "Shuffle Tracklist"),
                        sigc::mem_fun(*this, &Class::on_shuffle_tracklist)) ;
                    m_refActionGroup->add( Gtk::Action::create("ContextAddToQueue", "Add to Queue"),
                        sigc::mem_fun(*this, &Class::on_add_track_to_queue)) ;
 
                    m_refUIManager = Gtk::UIManager::create() ;
                    m_refUIManager->insert_action_group(m_refActionGroup) ;

                    std::string ui_info =
                    "<ui>"
                    "   <popup name='PopupMenu'>"
                    "       <menuitem action='ContextAddToQueue'/>"
                    "       <separator/>"
                    "       <menuitem action='ContextShowAlbum'/>"
                    "       <menuitem action='ContextShowArtist'/>"
                    "       <separator/>"
                    "       <menuitem action='ContextRandomShuffle'/>"
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

#endif // _YOUKI_TRACK_LIST_HH
