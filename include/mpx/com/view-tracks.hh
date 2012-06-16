#ifndef YOUKI_VIEW_TRACKS_HH
#define YOUKI_VIEW_TRACKS_HH

#include <gtkmm.h>
#include <cairomm/cairomm.h>
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/optional.hpp>
#include <boost/format.hpp>
#include <boost/ref.hpp>
#include <boost/unordered_set.hpp>
#include <boost/lexical_cast.hpp>

#include <sigx/sigx.h>

#include <cmath>
#include <deque>
#include <algorithm>

#include "mpx/util-string.hh"
#include "mpx/util-graphics.hh"

#include "mpx/mpx-types.hh"
#include "mpx/mpx-main.hh"
#include "mpx/mpx-network.hh"

#include "mpx/algorithm/aque.hh"
#include "mpx/algorithm/interval.hh"
#include "mpx/algorithm/limiter.hh"
#include "mpx/algorithm/vector_compare.hh"
#include "mpx/algorithm/adder.hh"

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

namespace SearchController
{
    enum class FilterMode
    {
	  ITERATIVE
	, NONITERATIVE
	, NONE
    } ;
}


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

	struct NewRow_t
	{
	    Track_sp	    TrackSp ;
	    guint	    ID ;

	    std::string	    Title ;
	    std::string	    Album ;
	    std::string	    Artist ;
	    std::string	    AlbumArtist ;
	    std::string	    ReleaseDate ;
	    guint	    Track ;	
	    guint	    Time ;
	} ;

	typedef boost::shared_ptr<NewRow_t> NewRow_sp ;

	enum class RowDatum
	{
	      R_TITLE
	    , R_ARTIST
	    , R_ALBUM
	    , R_ID
	    , R_TRACK_SP
	    , R_TRACK
	    , R_ALBUM_ARTIST
	    , R_MB_RELEASE_DATE
	    , R_TIME
	} ;


	bool operator==(const NewRow_t& a, const NewRow_t& b)
	{
	    return a.ID == b.ID ;
	}

	bool operator!=(const NewRow_t& a, const NewRow_t& b)
	{
	    return a.ID != b.ID ;
	}

	bool operator<(const NewRow_t& a, const NewRow_t& b)
	{
	    return a.ID < b.ID ; 
	}


	bool operator==(const NewRow_sp& a, const NewRow_sp& b)
	{
	    return a->ID == b->ID ;
	}

	bool operator!=(const NewRow_sp& a, const NewRow_sp& b)
	{
	    return a->ID != b->ID ;
	}

	bool operator<(const NewRow_sp& a, const NewRow_sp& b)
	{
	    return a->ID < b->ID ; 
	}

        typedef std::vector<NewRow_sp>			Model_t ;
        typedef boost::shared_ptr<Model_t>		Model_sp ;

	typedef sigc::signal<void>			Signal0 ;
        typedef sigc::signal<void, guint, bool>		Signal2 ;

	typedef std::vector<guint>		    IdVector_t ;
	typedef boost::shared_ptr<IdVector_t>	    IdVector_sp ;

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
        : public std::binary_function<NewRow_sp, NewRow_sp, bool>
        {
            bool operator() (
                  const NewRow_sp&  a
                , const NewRow_sp&  b
            )
            {
		if(! a && b )
		    return false ;

		Track_sp t_a = a->TrackSp ; 
		Track_sp t_b = b->TrackSp ; 

		if(! t_a && t_b )
		    return false ;

                const std::string&  order_artist_a = a->AlbumArtist ; 
                const std::string&  order_artist_b = b->AlbumArtist ; 

                const std::string&  order_album_a  = a->Album ; 
                const std::string&  order_album_b  = b->Album ; 

                const std::string&  order_date_a   = a->ReleaseDate ; 
                const std::string&  order_date_b   = b->ReleaseDate ; 

                guint order_track [2] = {
                      a->Track 
                    , b->Track 
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

        struct DataModel
        : public sigc::trackable 
        {
		typedef std::vector<Model_t::size_type>		ModelIdxVec_t ;
		typedef std::vector<ModelIdxVec_t>		AlbumTrackMapping_t ;

                Model_sp		m_realmodel;
                guint			m_upper_bound ;
                Signal2			m_SIGNAL__changed;
		AlbumTrackMapping_t	m_album_track_mapping ;

                DataModel()
                : m_upper_bound( 0 )
                {
                    m_realmodel = Model_sp(new Model_t); 
                }

                DataModel(Model_sp model)
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

                virtual Signal2&
                signal_changed()
                {
                    return m_SIGNAL__changed ;
                }

                virtual bool
                is_set()
                {
                    return bool(m_realmodel) ;
                }

                virtual guint
                size()
                {
                    return m_realmodel->size() ;
                }

                inline virtual const NewRow_sp&
                row(guint d)
                {
                    return (*m_realmodel)[d] ;
                }

                virtual void
                set_current_row(
                    guint d
                )
                {
                    m_upper_bound = d ;
                }

                virtual void
                append_track(
                      SQL::Row&             r
                    , const MPX::Track_sp&  track
                )
                {
                    using boost::get ;

		    NewRow_t * nr = new NewRow_t ;

		    nr->TrackSp = track ;
		    nr->ID = get<guint>(r["id"]) ;

		    nr->Title = r.count("title") ? get<std::string>(r["title"]) : "" ;
		    nr->Album = r.count("album") ? get<std::string>(r["album"]) : "" ;
		    nr->Artist = Util::row_get_artist_name( r ) ;
		    nr->AlbumArtist = Util::row_get_album_artist_name( r ) ;
		    nr->ReleaseDate = r.count("mb_release_date") ? get<std::string>(r["mb_release_date"]) : "" ;
		    nr->Time = r.count("time") ? get<guint>(r["time"]) : 0 ;
		    nr->Track = r.count("track") ? get<guint>(r["track"]) : 0 ;

                    m_realmodel->push_back(NewRow_sp(nr)) ;

/*
                    Model_t::iterator i = m_realmodel->end() ;
                    std::advance( i, -1 ) ;
		    guint album_id = get<guint>(r["mpx_album_id"]) ;
		    ModelIdxVec_t& vy = m_album_track_mapping[album_id] ;
		    vy.push_back( m_realmodel->size() - 1 ) ; 
*/
                }

                void
                erase_track(guint id)
                {
                    for( Model_t::iterator i = m_realmodel->begin() ; i != m_realmodel->end() ; ++i ) 
                    {
                        if( (*i)->ID == id )
                        {
                            m_realmodel->erase( i ) ;
                            return ;
                        }
                    }
                }
        };

        typedef boost::shared_ptr<DataModel> DataModel_sp;

        struct DataModelFilter
        : public DataModel
        {
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
                TCVector_sp                 m_constraints_artist ;
                TCVector_sp                 m_constraints_albums ;
		guint			    m_total_time ;
                bool                        m_cache_enabled ;

		Signal0			    m_SIGNAL__process_begin ;
		Signal0			    m_SIGNAL__process_end ;

		MPX::NM			    m_NM ;

                Signal0&
                signal_process_begin()
                {
                    return m_SIGNAL__process_begin ;
                }

                Signal0&
                signal_process_end()
                {
                    return m_SIGNAL__process_end ;
                }

                DataModelFilter( DataModel_sp& model )

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

		IdVector_sp
		get_id_vector()
		{
		    IdVector_t * v = new IdVector_t ;

		    for( auto& r : *m_mapping )
		    {
			v->push_back( (*r)->ID ) ;
		    }

		    return IdVector_sp(v) ;
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

                virtual const NewRow_sp&
                row(guint d)
                {
                    return *((*m_mapping)[d]);
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
                    const std::string& title  = get<std::string>(r["title"]) ;
                    const std::string& artist = Util::row_get_artist_name( r ) ;
                    const std::string& album  = get<std::string>(r["album"]) ; 
                    guint id         = get<guint>(r["id"]) ;
                    guint track_n    = get<guint>(r["track"]) ;
                    guint time       = get<guint>(r["time"]) ;


                    std::string order_artist ;
                    std::string release_date ;

		    if( r.count("album_artist_sortname"))
		    {
                    	order_artist = get<std::string>(r["album_artist_sortname"]); 
		    }

		    if( r.count("mb_release_date"))
		    {
			release_date = get<std::string>(r["mb_release_date"]) ;
		    }

                    static OrderFunc order ;

		    NewRow_t * nr = new NewRow_t ;
		    NewRow_sp nrsp (nr) ;

		    nr->TrackSp = track ;
		    nr->ID = id ; 

		    nr->Title = title ; 
		    nr->Album = album ; 
		    nr->Artist = artist ; 
		    nr->AlbumArtist = order_artist ; 
		    nr->ReleaseDate = release_date ; 
		    nr->Time = time ; 
		    nr->Track = track_n ;

                    m_realmodel->insert(
                          std::upper_bound(
                              m_realmodel->begin()
                            , m_realmodel->end()
                            , nrsp 
                            , order
                          )
                        , nrsp 
                    ) ;
                }
 
                virtual SearchController::FilterMode 
                process_filter(
                     const std::string&		text
		   , boost::optional<guint>	id
		   , std::string&		text_noaque
                )
                { 
		    const std::string m_old_filter = m_current_filter ;

                    AQE::Constraints_t aqe = m_constraints_aqe ;

                    m_constraints_aqe.clear() ;
                    m_frags.clear() ;

		    bool have_internet = m_NM.is_connected() ;

                    bool async = AQE::parse_advanced_query( m_constraints_aqe, text, m_frags ) ;

		    if( async && have_internet )    
		    {
			m_SIGNAL__process_begin.emit() ;
			AQE::process_constraints( m_constraints_aqe ) ;
			m_SIGNAL__process_end.emit() ;
		    }

                    bool aqe_diff = m_constraints_aqe != aqe ;

		    m_current_filter_noaque = Util::stdstrjoin( m_frags, " " ) ;
		    m_current_filter = text ;

		    text_noaque = m_current_filter_noaque ;
 
                    if( !aqe_diff && m_current_filter_noaque.empty() && (m_current_filter == m_old_filter)) 
		    {
			return SearchController::FilterMode::NONE ;	
		    }
                    if( !aqe_diff && !m_current_filter_noaque.empty() && (text.substr(0, text.size()-1) == m_old_filter))
                    {
			regen_mapping_iterative( id ) ;
			return SearchController::FilterMode::ITERATIVE ;
                    }
                    else
                    {
                        regen_mapping( id ) ;
			return SearchController::FilterMode::NONITERATIVE ;
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
			guint d = 0 ;
			for( auto& i : *m_mapping ) 
			{
			    if( m_id_currently_playing.get() == (*i)->ID ) 
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
			guint d = 0 ;
			for( auto& i : *m_mapping ) 
			{
			    if( id.get() == (*i)->ID )
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
			id = row(m_upper_bound)->ID ; 
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

                        m_constraints_artist = TCVector_sp( new TCVector_t ) ; 
                        m_constraints_artist->resize( m_max_size_constraints_artist + 1 ) ;

                        TCVector_t& constraints_albums = *m_constraints_albums ;
                        TCVector_t& constraints_artist = *m_constraints_artist ;

                        new_mapping->reserve( m_realmodel->size() ) ;
                        new_mapping_unfiltered->reserve( m_realmodel->size() ) ;

                        for( Model_t::iterator i = m_realmodel->begin() ; i != m_realmodel->end() ; ++i ) 
                        {
                            const MPX::Track_sp& t = (*i)->TrackSp ; 
                            const MPX::Track& track = *t ;

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
			    TracksConstraint& tc_alb = constraints_albums[id_album] ;
                            tc_alb.Count ++ ; 
			    tc_alb.Time += get<guint>(track[ATTRIBUTE_TIME].get()) ;

                            guint id_artist = get<guint>(track[ATTRIBUTE_MPX_ALBUM_ARTIST_ID].get()) ;
			    TracksConstraint& tc_art = constraints_artist[id_artist] ;
                            tc_art.Count ++ ; 
			    tc_art.Time += get<guint>(track[ATTRIBUTE_TIME].get()) ;

                            new_mapping->push_back( i ) ; 
                        }
                    }
                    else
                    {
                        IntersectVector_t intersect ;
                        intersect.reserve( m_frags.size() ) ; 

                        StrV vec (4) ;

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
                                const NewRow_sp r = *i;

                                vec[0] = Glib::ustring(r->Artist).lowercase() ;
                                vec[1] = Glib::ustring(r->Album).lowercase() ;
                                vec[2] = Glib::ustring(r->Title).lowercase() ;
                                vec[3] = Glib::ustring(r->AlbumArtist).lowercase() ;

                                if( Util::match_vec( m_frags[n], vec ))
                                {
                                    model_iterator_set->insert( i ) ; 
                                }
                            }

                            intersect.push_back( model_iterator_set ) ; 

                            if( m_cache_enabled && m_constraints_ext.empty() && m_constraints_aqe.empty())
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

                        m_constraints_artist = TCVector_sp( new TCVector_t ) ; 
                        m_constraints_artist->resize( m_max_size_constraints_artist + 1 ) ;

                        TCVector_t& constraints_albums = *m_constraints_albums ;
                        TCVector_t& constraints_artist = *m_constraints_artist ;

                        for( auto& i : *output ) 
                        {
                            const MPX::Track_sp& t = (*i)->TrackSp ; 
                            const MPX::Track& track = *t ;

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
			    TracksConstraint& tc_alb = constraints_albums[id_album] ;
                            tc_alb.Count ++ ; 
			    tc_alb.Time += get<guint>(track[ATTRIBUTE_TIME].get()) ;

                            guint id_artist = get<guint>(track[ATTRIBUTE_MPX_ALBUM_ARTIST_ID].get()) ;
			    TracksConstraint& tc_art = constraints_artist[id_artist] ;
                            tc_art.Count ++ ; 
			    tc_art.Time += get<guint>(track[ATTRIBUTE_TIME].get()) ;

                            new_mapping->push_back( i ) ;
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
			id = row(m_upper_bound)->ID ;
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

                        m_constraints_artist = TCVector_sp( new TCVector_t ) ; 
                        m_constraints_artist->resize( m_max_size_constraints_artist + 1 ) ;

                        TCVector_t& constraints_albums = *m_constraints_albums ;
                        TCVector_t& constraints_artist = *m_constraints_artist ;

                        new_mapping->reserve( m_mapping_unfiltered->size() ) ;
                        new_mapping_unfiltered->reserve( m_mapping_unfiltered->size() ) ;
	
                        for( auto& i : *m_mapping_unfiltered ) 
                        {
                            const MPX::Track_sp& t = (*i)->TrackSp ; 
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
			    TracksConstraint& tc_alb = constraints_albums[id_album] ;
                            tc_alb.Count ++ ; 
			    tc_alb.Time += get<guint>(track[ATTRIBUTE_TIME].get()) ;

                            guint id_artist = get<guint>(track[ATTRIBUTE_MPX_ALBUM_ARTIST_ID].get()) ;
			    TracksConstraint& tc_art = constraints_artist[id_artist] ;
                            tc_art.Count ++ ; 
			    tc_art.Time += get<guint>(track[ATTRIBUTE_TIME].get()) ;

                            new_mapping->push_back( i ) ; 
                        }
                    }
                    else
                    {
                        IntersectVector_t intersect ; 
                        intersect.reserve( m_frags.size() ) ;

                        StrV vec (4) ;

                        for( auto& f : m_frags ) 
                        {
                            if( f.empty() ) 
                            {
                                continue ;
                            }

                            if( m_cache_enabled ) 
                            {
                                FragmentCache_t::iterator cache_iter = m_fragment_cache.find( f ) ;

                                if( cache_iter != m_fragment_cache.end() )
                                {
                                    intersect.push_back( cache_iter->second ) ;
                                    continue ;
                                }
                            }

                            ModelIteratorSet_sp model_iterator_set ( new ModelIteratorSet_t ) ;

                            for( auto& i : *m_mapping_unfiltered ) 
                            {
                                const NewRow_sp& r = *i ;

                                vec[0] = Glib::ustring(r->Artist).lowercase() ;
                                vec[1] = Glib::ustring(r->Album).lowercase() ;
                                vec[2] = Glib::ustring(r->Title).lowercase() ;
                                vec[3] = Glib::ustring(r->AlbumArtist).lowercase() ;

                                if( Util::match_vec( f, vec ))
                                {
                                    model_iterator_set->insert( i ) ; 
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
                                m_fragment_cache.insert( std::make_pair( f, model_iterator_set )) ;
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

                        m_constraints_artist = TCVector_sp( new TCVector_t ) ; 
                        m_constraints_artist->resize( m_max_size_constraints_artist + 1 ) ;

                        TCVector_t& constraints_albums = *m_constraints_albums ;
                        TCVector_t& constraints_artist = *m_constraints_artist ;

                        for( ModelIteratorSet_t::iterator i = output->begin() ; i != output->end(); ++i )
                        {
                            const MPX::Track_sp& t = (**i)->TrackSp ; 
                            const MPX::Track& track = *t ;

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
			    TracksConstraint& tc_alb = constraints_albums[id_album] ;
                            tc_alb.Count ++ ; 
			    tc_alb.Time += get<guint>(track[ATTRIBUTE_TIME].get()) ;

                            guint id_artist = get<guint>(track[ATTRIBUTE_MPX_ALBUM_ARTIST_ID].get()) ;
			    TracksConstraint& tc_art = constraints_artist[id_artist] ;
                            tc_art.Count ++ ; 
			    tc_art.Time += get<guint>(track[ATTRIBUTE_TIME].get()) ;

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

		guint
		get_total_time()
		{
		    guint total = 0 ;

		    for( RowRowMapping_t::iterator i = (*m_mapping).begin() ; i != (*m_mapping).end() ; ++i ) 
		    {
			const MPX::Track_sp& t = (**i)->TrackSp ; 
			const MPX::Track& track = *t ;
			total += get<guint>(track[ATTRIBUTE_TIME].get()) ;
		    }

		    return total ;
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
                    cairo->move_to(
                          xpos + 6
                        , ypos + 4
                    ) ;

                    Glib::RefPtr<Pango::Layout> layout = widget.create_pango_layout(m_title) ;
                    layout->set_ellipsize(Pango::ELLIPSIZE_END) ;
                    layout->set_width( (m_width-12)*PANGO_SCALE ) ;
                    layout->set_alignment( m_alignment ) ;
                    Gdk::Cairo::set_source_rgba(cairo, Util::make_rgba(color,0.8));
		    layout->show_in_cairo_context( cairo ) ;
                }

                void
                render(
                      const Cairo::RefPtr<Cairo::Context>&  cairo
                    , Gtk::Widget&			    widget
                    , const NewRow_sp&			    r
                    , int				    xpos
                    , int				    ypos
                    , int				    rowheight
                    , const ThemeColor&			    color
		    , bool				    highlight
		    , const std::string&		    matches
		    , bool				    selected
                )
                {
		    using boost::get ;

		    std::string s ;

		    switch( m_column )
		    {
			  case 0:
			      s = r->Title ; 
			      break;
			  case 1:
			      s = r->Artist ; 
			      break;
			  case 2:
			      s = r->Album ; 
			      break;
			  case 5:
			      try{
				  s = boost::lexical_cast<std::string>(r->Track) ;
			      } catch(...) {}
			      break;
			  case 9:
			      s = ((boost::format("%02d:%02d") % (r->Time/60) % (r->Time%60)).str()) ;
			      break;
		    }

		    Glib::RefPtr<Pango::Layout> layout; 

		    if( !matches.empty() && highlight )
		    {
			layout = widget.create_pango_layout("");
			layout->set_markup(Util::text_match_highlight( s, matches, "#ff4040" )) ;
		    }
		    else
		    {
			layout = widget.create_pango_layout(s) ;
		    }

		    layout->set_ellipsize( Pango::ELLIPSIZE_END ) ;
		    layout->set_width((m_width - 12) * PANGO_SCALE ) ;
		    layout->set_alignment( m_alignment ) ;

		    if( selected )
		    {
			Util::render_text_shadow( layout, xpos+6,ypos+2, cairo, 2, 0.55 ) ;
		    }

		    cairo->move_to(
			    xpos + 6
			  , ypos + 2
		    ) ;

		    Gdk::Cairo::set_source_rgba( cairo, color ) ;
		    layout->show_in_cairo_context( cairo ) ;
		}
        };

        typedef boost::shared_ptr<Column>               Column_sp ;
        typedef std::vector<Column_sp>			Columns ;

        typedef sigc::signal<void, MPX::Track_sp, bool> SignalTrackActivated ;
        typedef sigc::signal<void>                      SignalVAdjChanged ;
        typedef sigc::signal<void>                      SignalFindAccepted ;
        typedef sigc::signal<void, const std::string&>  SignalFindPropagate ;
	typedef sigc::signal<void, const std::string&>	SignalMBID ;

        class Class
        : public Gtk::DrawingArea, public Gtk::Scrollable
        {
            public:

                DataModelFilter_sp                  m_model ;

            private:

		PropAdjustment			    property_vadj_, property_hadj_ ;
		PropScrollPolicy		    property_vsp_ , property_hsp_ ;

                guint                               m_height__row ;
                guint                               m_height__headers ;
                guint                               m_height__current_viewport ;

                Interval<guint>			    ModelExtents ;
		Interval<guint>			    ViewPort ;

                Columns                             m_columns ;

                boost::optional<boost::tuple<Model_t::const_iterator, guint> >  m_selection ;
    
		enum SelDatum
		{
		      S_ITERATOR
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

		boost::shared_ptr<IYoukiThemeEngine> m_theme ; 

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
			ViewPort = Interval<guint> (
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

		virtual void
		on_size_allocate( Gtk::Allocation& a )
		{
		    int x = a.get_x() ;
		    int y = a.get_y() ;

		    int height = get_parent()->get_allocation().get_height() ; 
		    y = 0 ;
		    x = 0 ;

		    a.set_x(x) ;
		    a.set_y(y) ;
		    a.set_height(height) ;

		    Gtk::DrawingArea::on_size_allocate(a) ;
		    queue_draw() ;
		}

                virtual bool
                on_focus_in_event(GdkEventFocus* G_GNUC_UNUSED)
                {
		    if( m_selection )
		    {
			guint idx = boost::get<S_INDEX>(m_selection.get()) ;

			if( ViewPort( idx ))
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

			    case GDK_KEY_BackSpace:
				if( m_SearchEntry->get_text().empty() )
				{
				    cancel_search() ;
				    return true ;
				}

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

                                MPX::Track_sp track = (*(get<0>(m_selection.get())))->TrackSp ;

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
                                step = get_page_size()/2 ;
                            }
                            else
                            {
                                step = 1 ;
                            }

                            if( event->state & GDK_SHIFT_MASK )
                            {
                                if( ModelExtents( origin - step ))
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
                                guint d = std::max<int>( 0, origin-step ) ;

                                if( d < get_upper_row() ) 
                                {
				    if( step == 1 )
					scroll_to_index( get_upper_row()-1 ) ;
				    else
					scroll_to_index( std::max<int>(0, d-get_page_size()/2)) ;
                                }
    
                                select_index( d ) ;
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
                                step = get_page_size()/2 ; 
                            }
                            else
                            {
                                step = 1 ;
                            }

                            if( event->state & GDK_SHIFT_MASK )
                            {
                                if( ModelExtents( origin + step ))
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
                                guint d = std::min<guint>( origin+step, m_model->size()-1 ) ;

                                if( d >= get_lower_row())
                                {
				    if( step == 1 )
                                        scroll_to_index( get_upper_row()+1 ) ;
				    else
					scroll_to_index( std::max<int>(0, d-get_page_size()/2)) ;
                                }

                                select_index(d) ;
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

                    //gdk_event_free( event ) ;
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
			    Interval<guint> I (
				  Interval<guint>::IN_EX
				, 0
				, m_model->size()
			    ) ;

			    if(I( d )) 
			    {
				MPX::Track_sp track = m_model->row(d)->TrackSp ;
				m_SIGNAL_track_activated.emit( track, true ) ;
			    }
			}
                    }
		    else
                    if(event->type == GDK_2BUTTON_PRESS && event->button == 1)
                    {
                        if( event->y > m_height__headers )
			{
			    Interval<guint> I (
				  Interval<guint>::IN_EX
				, 0
				, m_model->size()
			    ) ;

			    if(I( d ) && !m_play_on_single_tap) 
			    {
				MPX::Track_sp track = m_model->row(d)->TrackSp ;
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
			if( ModelExtents( row )) 
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
/*
		    guint last ;

		    while( !m_motion_queue.empty())
		    {
			std::pair<guint,guint> p = m_motion_queue.front() ;
			m_motion_queue.pop_front() ;

			m_model->swap( p.first, p.second ) ;
			last = p.first ;
		    }

		    select_index( last ) ; 
*/
		}

                void
                configure_vadj(
                      double upper
                    , double page_size
                )
                {
                    if( property_vadjustment().get_value() ) 
                    {
			if( m_height__current_viewport && m_height__row)
			{
			    page_size = (double(m_height__current_viewport)/double(m_height__row)) ;
			}

			property_vadjustment().get_value()->freeze_notify() ;	
                        property_vadjustment().get_value()->set_upper( (upper<page_size)?page_size:upper ) ; 
		        property_vadjustment().get_value()->set_page_size( page_size ) ; 
			property_vadjustment().get_value()->thaw_notify() ;
		    }
                }

                bool
                on_configure_event(
                    GdkEventConfigure* event
                )        
                {
		    Gtk::DrawingArea::on_configure_event(event) ;

                    const double column_width_collapsed = 40. ;

                    m_height__current_viewport = event->height - m_height__headers ;

		    ViewPort = Interval<guint> (
			  Interval<guint>::IN_EX
			, get_upper_row()
			, get_upper_row() + get_page_size()
		    ) ;

                    configure_vadj(
                          m_model->size()+1
                        , get_page_size()
                    ) ;

                    int width = event->width ;

                    double column_width_calculated = (double(width) - double(m_columns__fixed_total_width) - double(column_width_collapsed*double(m_columns__collapsed.size()))) / (m_columns.size() - m_columns__collapsed.size() - m_columns__fixed.size()) ;

                    for( guint n = 0; n < m_columns.size(); ++n )
                    {
			double w_eff = 0 ;

			if( n == 1 )
			    w_eff = (column_width_calculated*3) * 0.50 ;
			else
			if( n == 3 )
			    w_eff = (column_width_calculated*3) * 0.25 ;
			else
			if( n == 4 )
			    w_eff = (column_width_calculated*3) * 0.25 ;
	    
                        if( !m_columns__fixed.count( n ) )
                        {
                            m_columns[n]->set_width(m_columns__collapsed.count( n ) ? column_width_collapsed : w_eff ) ; 
                        }
                    }

                    return true ;
                }

                inline bool
                Compare(
                      const NewRow_sp&                 r
                    , const boost::optional<guint>&    id
                )
                {
                    return( id && id.get() == r->ID ) ;
                }

                template <typename T>
                inline bool
                Compare(
                      const T&                          val
                    , const boost::optional<T>&         cmp
                )
                {
                    return( cmp && cmp.get() == val ) ;
                }

		void
		render_now_playing_arrow(
		      const Cairo::RefPtr<Cairo::Context>&  cairo
		    , guint n
		    , bool  selected 
		)	
		{ 
		    const guint x = 0,
				y = m_height__headers + n*m_height__row ;

		    cairo->save() ;
		    cairo->translate(x,y) ;
		    cairo->set_line_join( Cairo::LINE_JOIN_ROUND ) ;
		    cairo->set_line_cap( Cairo::LINE_CAP_ROUND ) ;
		    cairo->move_to( 4, 4) ; 
		    cairo->line_to( 16, m_height__row/2. ) ; 
		    cairo->line_to( 4, m_height__row-4 ) ;
		    cairo->close_path() ;

		    if( selected )  
			cairo->set_source_rgba( 1.0, 1.0, 1.0, 0.9 ) ;	
		    else
			cairo->set_source_rgba( 0.1, 0.1, 0.1, 1.0 ) ;
	    
		    cairo->fill() ;	
		    cairo->restore() ;
		}

		void
		render_header_background(
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

		void
		render_rules_hint(
		      const Cairo::RefPtr<Cairo::Context>& cairo
		    , const Gdk::RGBA& c_rules_hint
		    , GdkRectangle& rr
		    , guint d_max
		)
		{
		    cairo->save() ;

		    Gdk::Cairo::set_source_rgba(cairo, c_rules_hint);

		    for( guint n = 0 ; n < d_max ; ++n ) 
		    {
			if( n % 2 )
			{
			    rr.y = m_height__headers + (n*m_height__row) ;

			    cairo->rectangle(
				  rr.x
				, rr.y
				, rr.width
				, rr.height
			    ) ;
			    cairo->fill() ;
			}
		    }
		    cairo->restore() ;
		}

		void
		render_headers(
		      const Cairo::RefPtr<Cairo::Context>& cairo
		    , const Gdk::RGBA& c_text
		)
		{
		    guint xpos = 0 ;
		    guint n = 0 ;

		    for( auto& c : m_columns ) 
		    {
			c->render_header(
				cairo
			      , *this
			      , xpos
			      , 0
			      , m_height__headers
			      , n 
			      , c_text
			) ;

			xpos += c->get_width() ; 
			++n ;
		    }
		}

		void
		render_treelines(
		      const Cairo::RefPtr<Cairo::Context>& cairo
		    , const Gdk::RGBA& c
		)
		{
		    const std::vector<double> dashes { 1, 1 } ;

		    Columns::iterator i2 = m_columns.end() ;
		    std::advance( i2, -1 ) ;	

		    std::vector<guint> xpos_v ;
		    guint xpos = 0 ;

		    for( Columns::const_iterator i = m_columns.begin() ; i != i2; ++i )
		    {
			xpos += (*i)->get_width() ; // adjust us to the column's end
			xpos_v.push_back( xpos ) ; 
		    }

		    cairo->save() ;
		    cairo->set_antialias( Cairo::ANTIALIAS_NONE ) ;
		    cairo->set_line_width(
			  1. 
		    ) ;

		    for( auto& xpos : xpos_v ) 
		    {
			cairo->move_to(
			      xpos
			    , 0
			) ;
			cairo->line_to(
			      xpos
			    , m_height__headers
			) ;
			Gdk::Cairo::set_source_rgba(cairo, Util::make_rgba(c,0.35)) ;
			cairo->stroke() ;

			cairo->set_dash( dashes, 0 ) ;
			cairo->move_to(
			      xpos
			    , m_height__headers
			) ; 
			cairo->line_to(
			      xpos
			    , get_allocated_height()
			) ;
			Gdk::Cairo::set_source_rgba(cairo, Util::make_rgba(c,0.35)) ;
			cairo->stroke() ;
			cairo->unset_dash() ;
		    }
		    cairo->restore() ;
		}

                bool
                on_draw(
		    const Cairo::RefPtr<Cairo::Context>& cairo
		)
                {
                    const ThemeColor& c_text        = m_theme->get_color( THEME_COLOR_TEXT ) ;
                    const ThemeColor& c_text_sel    = m_theme->get_color( THEME_COLOR_TEXT_SELECTED ) ;
                    const ThemeColor& c_rules_hint  = m_theme->get_color( THEME_COLOR_BASE_ALTERNATE ) ;
		    const ThemeColor& c_treelines   = m_theme->get_color( THEME_COLOR_TREELINES ) ;
		    const ThemeColor& c_bg	    = m_theme->get_color( THEME_COLOR_BACKGROUND ) ;

                    guint d       = get_upper_row() ;
                    guint d_max   = std::min<guint>( m_model->size(), get_page_size()+1 ) ;
                    guint xpos    = 0 ;

		    /* Row Rectangle for reusal */
		    GdkRectangle rr ;
		    rr.x       = 0 ;
		    rr.width   = get_allocated_width() ; 
		    rr.height  = m_height__row ; 

		    /* Header Background */ 
		    render_header_background(
			  cairo
			, get_allocated_width()
			, m_height__headers
			, c_bg
			, c_treelines
		    ) ;
    
		    /* Headers */
		    render_headers( cairo, c_text ) ;

		    /* Rules Hint */
		    render_rules_hint( cairo, c_rules_hint, rr, d_max ) ;

		    /* Determine Selected ID */
		    boost::optional<guint> d_sel = m_selection ? get<S_INDEX>(m_selection.get()) : boost::optional<guint>() ; 

		    /* Render Selection Rectangle */
		    if( d_sel && ViewPort(d_sel.get()))
		    {
			rr.y = m_height__headers + ((d_sel.get() - d)*m_height__row) ;

			m_theme->draw_selection_rectangle(
			      cairo
			    , rr
			    , has_focus()
			    , 0
			    , MPX::CairoCorners::CORNERS(0)
			) ;
		    }

		    /* Row Data */
		    guint n = 0 ;
		    Algorithm::Adder<guint> d_cur( n, d ) ;	

		    while( n < d_max && ModelExtents(d_cur)) 
		    {
			xpos = 0 ;

			const NewRow_sp& r = m_model->row( d_cur ) ;

			if( Compare( r, m_model->m_id_currently_playing )) 
			{
			    render_now_playing_arrow( cairo, n, Compare<guint>(d_cur,d_sel)) ;
			}

			for( auto& c : m_columns ) 
			{
			    c->render(
				  cairo
				, *this
				, r 
				, xpos
				, m_height__headers + (n*m_height__row)
				, m_height__row
				, Compare<guint>( d_cur, d_sel ) ? c_text_sel : c_text
				, m_highlight_matches
				, m_model->m_current_filter_noaque
				, Compare<guint>( d_cur, d_sel )
			    ) ;

			    xpos += c->get_width() ; 
			}

			++n ;
		    }

		    render_treelines( cairo, c_treelines ) ;

		    if( !is_sensitive() )
		    {
			cairo->rectangle(0,0,get_allocated_width(),get_allocated_height()) ;
			cairo->set_source_rgba(0,0,0,0.2) ;
			cairo->fill() ;
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
                      guint   position
                    , bool    size_changed
                )
                {
                    if( size_changed ) 
                    {
                        ModelExtents = Interval<guint> (
			      Interval<guint>::IN_EX
			    , 0
			    , m_model->size()
                        ) ;

                        configure_vadj(
                              m_model->size()+1
                            , get_page_size()
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

		    queue_resize() ;
                }

                bool
                query_tooltip(
                      int                                   tooltip_x
                    , int                                   tooltip_y
                    , bool                                  keypress
                    , const Glib::RefPtr<Gtk::Tooltip>&     tooltip
                )
                {
                    guint d = (double( tooltip_y ) - m_height__headers) / double(m_height__row) ;

		    if(!ModelExtents(d+get_upper_row())) return false ;

                    MPX::Track_sp t = m_model->row(d+get_upper_row())->TrackSp ;
                    const MPX::Track& track = *(t.get()) ;
		    tooltip->set_text( boost::get<std::string>(track[ATTRIBUTE_TITLE].get())) ;
                    return true ;
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
                set_model(DataModelFilter_sp model)
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
                      Column_sp   column
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
                    for( DataModelFilter::RowRowMapping_t::iterator i = m_model->m_mapping->begin() ; i != m_model->m_mapping->end() ; ++i )
                    {
                        if( (**i)->ID == id )
                        {
                            Limiter<guint> d ( 
                                  Limiter<guint>::ABS_ABS
                                , 0
                                , m_model->m_mapping->size() - get_page_size()
                                , std::distance( m_model->m_mapping->begin(), i ) 
                            ) ;

                            vadj_value_set( d ) ; 
                            break ;
                        }
                    } 
                }

                void
                scroll_to_index(
                      guint d
                )
                {
                    if( m_height__current_viewport && m_height__row && m_model )
                    {
                        Limiter<guint> d_lim ( 
                              Limiter<guint>::ABS_ABS
                            , 0
                            , m_model->m_mapping->size() - get_page_size()
                            , d 
                        ) ;

                        if( m_model->m_mapping->size() < get_page_size()) 
                            vadj_value_set( 0 ) ; 
                        else
                        if( d_lim > (m_model->m_mapping->size() - get_page_size()) )
                            vadj_value_set( m_model->m_mapping->size() - get_page_size() ) ; 
                        else
                            vadj_value_set( d_lim ) ; 
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
			if( numeric && nr == (**i)->Track ) 
			{
			    scroll_to_index( std::max<int>( 0, d-get_page_size()/2)) ;
			    select_index( d ) ;
			    return ;
			}

                        Glib::ustring match = Glib::ustring((**i)->Title).casefold() ;

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
			if( numeric && nr == (**i)->Track ) 
			{
			    scroll_to_index( std::max<int>( 0, d-get_page_size()/2)) ;
			    select_index( d ) ;
			    return ;
			}

                        Glib::ustring match = Glib::ustring((**i)->Title).casefold() ;

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
			if( numeric && nr == (**i)->Track ) 
			{
			    scroll_to_index( std::max<int>( 0, d-get_page_size()/2)) ;
			    select_index( d ) ;
			    m_SearchEntry->unset_color() ;
			    return ;
			}

                        Glib::ustring match = Glib::ustring((**i)->Title).casefold() ;

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
                        const NewRow_sp& r = *(boost::get<0>(m_selection.get())) ;
                        const MPX::Track_sp& t = r->TrackSp ; 
                        const MPX::Track& track = *(t.get()) ;

                        _signal_0.emit( get<std::string>(track[ATTRIBUTE_MB_ALBUM_ID].get()));
                    }
                }

                void
                on_show_only_this_artist() 
                {
                    if( m_selection )
                    {
                        const NewRow_sp& r = *(boost::get<0>(m_selection.get())) ;
                        const MPX::Track_sp& t = r->TrackSp ; 
                        const MPX::Track& track = *(t.get()) ;

                        _signal_1.emit( get<std::string>(track[ATTRIBUTE_MB_ALBUM_ARTIST_ID].get()));
                    }
                }

		void
		on_add_track_to_queue()
		{	
		    if( m_selection )
		    {
			MPX::Track_sp track = (*(get<S_ITERATOR>(m_selection.get())))->TrackSp ;
			m_SIGNAL_track_activated.emit( track, false ) ;
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
                          m_model->size()+1
                        , get_page_size()
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

			, m_height__row(0)
			, m_height__headers(0)
			, m_height__current_viewport(0)
                        , m_columns__fixed_total_width(0)
                        , m_search_active(false)
                        , m_highlight_matches(false)
			, m_play_on_single_tap(false)
			, vadj_value_old(0)

                {
		    property_vadjustment().signal_changed().connect( sigc::mem_fun( *this, &Class::on_vadj_prop_changed )) ;

		    m_theme = services->get<IYoukiThemeEngine>("mpx-service-theme") ;

                    set_can_focus(true);

                    add_events(Gdk::EventMask(GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK | GDK_BUTTON_PRESS_MASK | GDK_EXPOSURE_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_LEAVE_NOTIFY_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_BUTTON_MOTION_MASK | GDK_SCROLL_MASK ));

                    signal_query_tooltip().connect(
                        sigc::mem_fun(
                              *this
                            , &Class::query_tooltip
                    )) ;

                    set_has_tooltip(true) ;

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
