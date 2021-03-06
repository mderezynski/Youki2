#include "config.h"

#include <vector>
#include <future>
#include <ratio>
#include <ctime>
#include <unordered_map>

#include <glibmm/ustring.h>
#include <gtk/gtk.h>

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/ref.hpp>
#include <boost/algorithm/string.hpp>

#include <sigx/sigx.h>

#include "mpx/mpx-types.hh"
#include "mpx/mpx-uri.hh"

#include "mpx/util-string.hh"

#include "mpx/algorithm/aque.hh"

#include "mpx/xml/xmltoc++.hh"

#include "xmlcpp/xsd-topalbums-2.0.hxx"
#include "xmlcpp/xsd-topartists-2.0.hxx"
#include "xmlcpp/xsd-artist-similar-2.0.hxx"
#include "xmlcpp/xsd-artist-toptracks-2.0.hxx"
#include "xmlcpp/xsd-tag-toptracks-2.0.hxx"

#include "mpx/mpx-main.hh"
#include "src/library.hh"

namespace
{
    using namespace MPX ;

    boost::optional<time_t>
    parse_date(
	  const std::string& value
    )
    {
	MPX::StrV v ;
	boost::algorithm::split( v, value, boost::algorithm::is_any_of(".")) ;

	if( v.size() == 3 )
	{
	    GDate * date = g_date_new() ;

	    g_date_set_parse(date, value.c_str()) ;

	    if(g_date_valid(date))
	    {
		struct tm tm ;
		g_date_to_struct_tm(date, &tm) ;
		time_t epoch = mktime(&tm) ;
		return epoch ;
	    }
	}

	return boost::optional<time_t>() ;
    }

    MPX::OVariant
    _lastfm_tag_topalbums(
	  const std::string& value
    )
    {
	MPX::StrS s ;

	try{
	    MPX::URI u ((boost::format( "http://ws.audioscrobbler.com/2.0/?method=tag.gettopalbums&tag=%s&api_key=37cd50ae88b85b764b72bb4fe4041fe4" ) % value).str(), true ) ;
	    typedef MPX::XmlInstance<lfm_tagtopalbums::lfm> Instance ;
	
	    Instance* Xml = new Instance(Glib::ustring( u )) ;

	    for( auto& album : Xml->xml().topalbums().album() )
	    {
		s.insert( album.mbid() ) ; 
	    }

	    delete Xml ;
	}
	catch(std::runtime_error& cxe){
	    g_message("%s: RuntimeError: %s", G_STRLOC, cxe.what()) ;
	}

	return MPX::OVariant(s) ;
    }

    MPX::OVariant
    _lastfm_tag_toptracks(
	  const std::string& value
    )
    {
	MPX::StrS s ;

	try{
	    MPX::URI u ((boost::format( "http://ws.audioscrobbler.com/2.0/?method=tag.gettoptracks&tag=%s&api_key=37cd50ae88b85b764b72bb4fe4041fe4" ) % value).str(), true ) ;

	    typedef MPX::XmlInstance<lfm_top_tracks_for_tag::lfm> Instance ;
	    Instance* Xml = new Instance(Glib::ustring( u )) ;

	    for( auto& track : Xml->xml().toptracks().track() )
	    {
		s.insert( track.name() ) ;
	    }
	}
	catch(std::runtime_error& cxe){
	    g_message("%s: RuntimeError: %s", G_STRLOC, cxe.what()) ;
	}

	return MPX::OVariant(s) ;
    }

    MPX::OVariant
    _lastfm_artist_toptracks(
	  const std::string& value
    )
    {
	MPX::StrS s ;

	try{
	    MPX::URI u ((boost::format( "http://ws.audioscrobbler.com/2.0/?method=artist.gettoptracks&artist=%s&api_key=37cd50ae88b85b764b72bb4fe4041fe4" ) % value).str(), true ) ;

	    typedef MPX::XmlInstance<lfm_top_tracks_for_artist::lfm> Instance ;
	    Instance* Xml = new Instance(Glib::ustring( u )) ;

	    guint c = 0 ;

	    for( auto& track : Xml->xml().toptracks().track() )
	    {
		s.insert( track.name() ) ;
		++c ;

		if( c == 5 )
		    break ;
	    }
	}
	catch(std::runtime_error& cxe){
	    g_message("%s: RuntimeError: %s", G_STRLOC, cxe.what()) ;
	}

	return MPX::OVariant(s) ;
    }

    MPX::OVariant
    _lastfm_artist_similar(
	  const std::string& value
    )
    {
	MPX::StrS s ;

	s.insert(value) ;

	try{
	    MPX::URI u ((boost::format( "http://ws.audioscrobbler.com/2.0/?method=artist.getsimilar&artist=%s&api_key=37cd50ae88b85b764b72bb4fe4041fe4" ) % value).str(), true) ;
	    typedef MPX::XmlInstance<lfm_similarartists::lfm> Instance ; 

	    Instance* Xml = new Instance(Glib::ustring( u )) ;

	    guint c = 0 ;

	    for( auto i : Xml->xml().similarartists().artist())
	    {
		s.insert(i.name()) ;
		++c ;
		if(c==10) break ;
	    }

	    delete Xml ;
	}
	catch(std::runtime_error& cxe){
	    g_message("%s: RuntimeError: %s", G_STRLOC, cxe.what()) ;
	}

	return MPX::OVariant(s) ;
    }

    MPX::OVariant
    _lastfm_artist_similar_mbid(
	  const std::string& value
    )
    {
	MPX::StrS s ;
    
	s.insert(value) ;

	try{
	    MPX::URI u ((boost::format( "http://ws.audioscrobbler.com/2.0/?method=artist.getsimilar&mbid=%s&api_key=37cd50ae88b85b764b72bb4fe4041fe4" ) % value).str(), true) ;
	    typedef MPX::XmlInstance<lfm_similarartists::lfm> Instance ; 

	    Instance* Xml = new Instance(Glib::ustring( u )) ;

	    guint c = 0 ;

	    for( auto i : Xml->xml().similarartists().artist())
	    {
		s.insert(i.mbid()) ;
		++c ;
		if(c==10) break ;
	    }

	    delete Xml ;
	}
	catch(std::runtime_error& cxe){
	    g_message("%s: RuntimeError: %s", G_STRLOC, cxe.what()) ;
	}

	return MPX::OVariant(s) ;
    }

    MPX::OVariant
    _duplicates(
	  const std::string& value
    )
    {
	g_message("value: %s", value.c_str()) ;

	auto lib = services->get<MPX::Library>("mpx-service-library");

	MPX::SQL::RowV v ;
	MPX::StrS s ;

	lib->getSQL( v, mprintf("SELECT %s FROM (SELECT %s, count(*) AS c FROM track_view GROUP BY %s) X WHERE c > 1",value.c_str(),value.c_str(),value.c_str())) ; 

	for( auto r : v )
	{
	    const std::string& rval = boost::get<std::string>(r[value]) ; 
	    s.insert(rval) ; 

	    g_message("rval: %s", rval.c_str()) ;
	}

	return MPX::OVariant(s) ;
    } 
}

namespace
{
    using namespace MPX ;
    using namespace MPX::AQE ;

    void
    add_constraint(
          Constraints_t&        constraints
        , const std::string&    attribute
        , const std::string&    value
        , MatchType_t           type
        , bool                  inverse_match
    )
    {
        if( type != MT_UNDEFINED )
        {
            Constraint_t c ;

            c.MatchType = type ;
            c.InverseMatch = inverse_match ;

            if( attribute == "top-tracks-for-tag" )
            {
                c.TargetAttr = ATTRIBUTE_TITLE ;
		c.Processing = CONSTRAINT_PROCESSING_ASYNC ;
		c.SourceValue = value ;
		c.GetValue = sigc::ptr_fun( &_lastfm_tag_toptracks ) ;	

                constraints.push_back(c) ;
            }
	    else
            if( attribute == "top-albums-for-tag" )
            {
                c.TargetAttr = ATTRIBUTE_MB_ALBUM_ID ;
		c.Processing = CONSTRAINT_PROCESSING_ASYNC ;
		c.SourceValue = value ;
		c.GetValue = sigc::ptr_fun( &_lastfm_tag_topalbums ) ;	

                constraints.push_back(c) ;
            }
            else
            if( attribute == "top-tracks-for-artist" )
            {
                c.TargetAttr = ATTRIBUTE_TITLE ;
		c.Processing = CONSTRAINT_PROCESSING_ASYNC ;
		c.SourceValue = value ;
		c.GetValue = sigc::ptr_fun( &_lastfm_artist_toptracks ) ;	
                constraints.push_back(c) ;
            }
	    else
	    if( attribute == "duplicates" )
	    {
		auto lib = services->get<MPX::Library>("mpx-service-library");

		MPX::SQL::RowV v ;
		MPX::StrS s ;

		lib->getSQL( v, mprintf("SELECT %s FROM (SELECT %s, count(*) AS c FROM track_view GROUP BY %s) X WHERE c > 1",value.c_str(),value.c_str(),value.c_str())) ; 

		for( auto r : v )
		{
		    const std::string& rval = boost::get<std::string>(r[value]) ; 
		    s.insert(rval) ; 
		}

		std::unordered_map<std::string, int> att_m ; 

		att_m.insert(std::make_pair("title",ATTRIBUTE_TITLE)) ;
		att_m.insert(std::make_pair("album",ATTRIBUTE_ALBUM)) ;
		att_m.insert(std::make_pair("artist",ATTRIBUTE_ARTIST)) ;

		auto i = att_m.find(value) ;

		if( i != std::end(att_m))
		{
/*
		    c.TargetAttr = i->second ;
		    c.Processing = CONSTRAINT_PROCESSING_ASYNC ;
		    c.SourceValue = value ;
		    c.GetValue = sigc::ptr_fun( &_duplicates ) ;	
*/
		    c.TargetAttr    = i->second ;
		    c.TargetValue   = s ;

		    constraints.push_back(c) ;
		}
	    }
            else
            if( attribute == "artists-similar-to" )
            {
                c.TargetAttr = ATTRIBUTE_ALBUM_ARTIST ;
		c.Processing = CONSTRAINT_PROCESSING_ASYNC ;
		c.SourceValue = value ;
		c.GetValue = sigc::ptr_fun( &_lastfm_artist_similar ) ;	

                constraints.push_back(c) ;
            }
            else
            if( attribute == "mbid-artists-similar-to" )
            {
                c.TargetAttr = ATTRIBUTE_MB_ALBUM_ARTIST_ID ;
		c.Processing = CONSTRAINT_PROCESSING_ASYNC ;
		c.SourceValue = value ;
		c.GetValue = sigc::ptr_fun( &_lastfm_artist_similar_mbid ) ;	

                constraints.push_back(c) ;
            }
            else
            if( attribute == "musicip-puid" )
            {
                c.TargetAttr = ATTRIBUTE_MUSICIP_PUID ;
                c.TargetValue = value ;

                constraints.push_back(c) ;
            }
            else
            if( attribute == "album-mbid" )
            {
                c.TargetAttr = ATTRIBUTE_MB_ALBUM_ID ;
                c.TargetValue = value ;

                constraints.push_back(c) ;
            }
            else
            if( attribute == "album-artist-mbid" )
            {
                c.TargetAttr = ATTRIBUTE_MB_ALBUM_ARTIST_ID ;
                c.TargetValue = value ;

                constraints.push_back(c) ;
            }
            else
            if( attribute == "artist-mbid" )
            {
                c.TargetAttr = ATTRIBUTE_MB_ARTIST_ID ;
                c.TargetValue = value ;

                constraints.push_back(c) ;
            }
            else
            if( attribute == "country" )
            {
                c.TargetAttr = ATTRIBUTE_MB_RELEASE_COUNTRY ;
                c.TargetValue = value ;

                constraints.push_back(c) ;
            }
            else
            if( attribute == "type" )
            {
                c.TargetAttr = ATTRIBUTE_MB_RELEASE_TYPE ;
                c.TargetValue = value ;

                constraints.push_back(c) ;
            }
	    else
            if( attribute == "location" )
            {
                c.TargetAttr = ATTRIBUTE_LOCATION ;
                c.TargetValue = value ;

                constraints.push_back(c) ;
            }
            else
            if( attribute == "bitrate" )
            {
                try{
                        c.TargetValue = (unsigned int)(boost::lexical_cast<int>(value)) ;
                        c.TargetAttr = ATTRIBUTE_BITRATE ;

                        constraints.push_back(c) ;
                } catch( boost::bad_lexical_cast ) {
                }
            }
            else
            if( attribute == "track" )
            {
                try{
                        c.TargetValue = (unsigned int)(boost::lexical_cast<int>(value)) ;
                        c.TargetAttr = ATTRIBUTE_TRACK ;

                        constraints.push_back(c) ;
                } catch( boost::bad_lexical_cast ) {
                }
            }
            else
            if( attribute == "time" )
            {
                try{
			StrV v ;
			boost::algorithm::split( v, value, boost::algorithm::is_any_of(":")) ;

			guint h = 0, m = 0, s = 0 ;

			if( v.size() == 3 )
			{
			    h = (unsigned int)(boost::lexical_cast<int>(v[0])) ;
			    m = (unsigned int)(boost::lexical_cast<int>(v[1])) ;
			    s = (unsigned int)(boost::lexical_cast<int>(v[2])) ;
			}
			else
			if( v.size() == 2 )
			{
			    m = (unsigned int)(boost::lexical_cast<int>(v[0])) ;
			    s = (unsigned int)(boost::lexical_cast<int>(v[1])) ;
			}
			else
			if( v.size() == 1 )
			{
			    s = (unsigned int)(boost::lexical_cast<int>(v[0])) ;
			}

                        c.TargetValue = (h*3600) + (m*60) + s ; 
                        c.TargetAttr = ATTRIBUTE_TIME ;

                        constraints.push_back(c) ;
                } catch( boost::bad_lexical_cast ) {
                }
            }
            else
            if( attribute == "added-at" )
            {
                try{
			boost::optional<time_t> t_ = parse_date(value) ;

			if(t_)
			{
			    c.TargetValue = (guint)(*t_) ; 
			    c.TargetAttr = ATTRIBUTE_INSERT_DATE ;
			    constraints.push_back(c) ;
			}

                } catch( boost::bad_lexical_cast ) {
                }
            }
            else
            if( attribute == "last-played-at" )
            {
                try{
			boost::optional<time_t> t_ = parse_date(value) ;

			if(t_)
			{
			    c.TargetValue = (guint)(*t_) ; 
			    c.TargetAttr = ATTRIBUTE_PLAYDATE ;
			    constraints.push_back(c) ;
			}

                } catch( boost::bad_lexical_cast ) {
                }
            }
            else
            if( attribute == "year" )
            {
                c.TargetAttr = ATTRIBUTE_DATE ;

                try{
			StrV v ;
			boost::algorithm::split( v, value, boost::algorithm::is_any_of("-")) ;

			guint year1, year2 ; 

			if( v.size() == 2 )
			{
			    year1 = std::stoi(v[0]) ; 
			    year2 = std::stoi(v[1]) ; 

			    c.TargetValue = year1 ; 
			    c.MatchType = MT_GREATER_THAN_OR_EQUAL ;
			    constraints.push_back(c) ;

			    c.TargetValue = year2 ; 
			    c.MatchType = MT_LESSER_THAN_OR_EQUAL ;
			    constraints.push_back(c) ;
			}
			if( v.size() == 1 )
			{
			    year1 = std::stoi(v[0]) ;  

			    c.TargetValue = year1 ; 
			    c.MatchType = MT_EQUAL ;
			    constraints.push_back(c) ;
			}

                } catch( std::exception ) {
                }
            }
            else
            if( attribute == "quality" )
            {
		try{
			c.TargetValue = (unsigned int)(boost::lexical_cast<int>(value)) ;
	                c.TargetAttr = ATTRIBUTE_QUALITY ;

			constraints.push_back(c) ;
		} catch( boost::bad_lexical_cast ) {
		}
            }
            else
            if( attribute == "genre" )
            {
                c.TargetValue = value ; 
                c.TargetAttr = ATTRIBUTE_GENRE ;
                constraints.push_back(c) ;
            }
            else
            if( attribute == "label" )
            {
                c.TargetValue = value ; 
                c.TargetAttr = ATTRIBUTE_LABEL ;
                constraints.push_back(c) ;
            }
            else
            if( attribute == "artist" )
            {
                c.TargetValue = value ; 
                c.TargetAttr = ATTRIBUTE_ARTIST ;
                constraints.push_back(c) ;
            }
            else
            if( attribute == "album-artist" )
            {
                c.TargetValue = value ;
                c.TargetAttr = ATTRIBUTE_ALBUM_ARTIST ;
                constraints.push_back(c) ;
            }
            else
            if( attribute == "album" )
            {
                c.TargetValue = value ; 
                c.TargetAttr = ATTRIBUTE_ALBUM ;
                constraints.push_back(c) ;
            }
            else
            if( attribute == "title" )
            {
                c.TargetValue = value ; 
                c.TargetAttr = ATTRIBUTE_TITLE ;
                constraints.push_back(c) ;
            }
        }
    }
}

namespace MPX
{
namespace AQE
{
    bool operator == (const Constraint_t& a, const Constraint_t& b )
    {
        return  (a.TargetAttr == b.TargetAttr) 
                    &&
                (a.TargetValue == b.TargetValue)
                    &&
                (a.MatchType == b.MatchType)
                    &&
                (a.InverseMatch == b.InverseMatch)
        ;
    }

    bool operator < (const Constraint_t& a, const Constraint_t& b )
    {
        return  (a.TargetAttr < b.TargetAttr) 
                    &&
                (a.TargetValue < b.TargetValue)
                    &&
                (a.MatchType < b.MatchType)
                    &&
                (a.InverseMatch < b.InverseMatch)
        ;
    }

    void
    process_constraints(
	  Constraints_t&	    constraints
    )
    {
	for( auto c : constraints ) 
	{
	    if( c.Processing == CONSTRAINT_PROCESSING_ASYNC )
	    {
		if( !c.GetValue )
		{
		    g_message("%s: Have constraint with PROCESSING_ASYNC but GetValue slot is empty!", G_STRLOC) ;
		    continue ;
		}

		auto handle = std::async(
		      std::launch::async
		    , [](Constraint_t& c){ c.TargetValue = c.GetValue(c.SourceValue); }
		    , boost::ref(c)
		) ;

		while(!handle.wait_for(std::chrono::duration<int, std::milli>(20)))
		{
		    while(gtk_events_pending()) gtk_main_iteration() ;
		}
	    } 
	}	
    }

    bool
    parse_advanced_query(
          Constraints_t&            constraints
        , const std::string&        text
        , StrV&                     non_attr_strings
    )
    {
	bool async = false ;

        enum KeyValEnum
        {
              KEY
            , VAL
        } ;

        Glib::ustring line ;
        Glib::ustring kv[2] ;

        MatchType_t type = MT_EQUAL ; 

        bool inverse = false ;
        bool done_reading_pair  = false ;
        bool have_op = false ;
        bool have_quot = false ;
        
        enum ReadType_t
        {
              READ_ATTR
            , READ_VAL
        } ;

        ReadType_t rt = READ_ATTR ;

        Glib::ustring text_utf8 ( text ) ;
        Glib::ustring::iterator i = text_utf8.begin() ;

        while( i != text_utf8.end() )
        {
            if( *i == '<' )
            {
                type = MT_LESSER_THAN ;
                rt = READ_VAL ;
                have_op = true ;
            }
            else
            if( *i == '>' )
            {
                type = MT_GREATER_THAN ;
                rt = READ_VAL ;
                have_op = true ;
            }
            else
            if( *i == '=' )
            {
                type = MT_EQUAL ;
                rt = READ_VAL ;
                have_op = true ;
            }
            else
            if( *i == '~' )
            {
                type = MT_FUZZY_EQUAL ;
                rt = READ_VAL ;
                have_op = true ;
            }
            else
            if( *i == '%' )
            {
                type = MT_EQUAL_BEGIN ;
                rt = READ_VAL ;
                have_op = true ;
            }
            else
            if( *i == '!' )
            {
                if( rt == READ_ATTR && !kv[KEY].empty() )
                {
                    inverse = true ;
                }
                else
                {
                    line += *i ;
                }
            }
            else
            if( *i == '"' ) 
            {
                have_quot = true ;

                if( rt == READ_ATTR )
                {
                    // we interpret this as the start of the attribute and use MT_FUZZY_EQUAL

                    type = MT_FUZZY_EQUAL ;
                    kv[KEY] = line ;
                    rt = READ_VAL ;
                }

                line.clear() ;
                ++i ;

                while( i != text_utf8.end() && *i != '"' )
                {
                    line += *i ; 
                    ++i ;
                }

                if( i == text_utf8.end() )
                {
                    return async ;
                }
                else
                {
		    if( have_op )
		    {
			kv[VAL] = line ;
			line.clear() ;
			done_reading_pair = true ; 
		    }
		    else
		    {
			non_attr_strings.push_back(line) ;
		    }
                }
            }
            else
            if( *i == ' ' ) 
            {
                if( rt == READ_ATTR )
                {
                    if( !line.empty() )
                    {
                        kv[KEY] = line ;
                        line.clear() ;
                    }
                }
                else
                {
                    if( !line.empty() )
                    {
                        kv[VAL] = line ;
                        line.clear() ;

                        done_reading_pair = true ;
                    }
                }
            }
            else
            {
                if( line.empty() && !kv[KEY].empty() && rt == READ_ATTR )
                {
                    non_attr_strings.push_back( kv[KEY] ) ;
                    kv[KEY].clear() ;
                }

                line += *i ;
            }

            if( i != text_utf8.end() )
            {
                ++i ;
            }

            if( i == text_utf8.end() && !done_reading_pair && (!have_op || !have_quot) )
            {
                if( rt == READ_ATTR )
                {
                    if( !line.empty() )
                    {
                        non_attr_strings.push_back( line ) ;
                        line.clear() ;
                    }
                    else
                    if( !kv[KEY].empty() )
                    {
                        non_attr_strings.push_back( kv[KEY] ) ;
                        kv[KEY].clear() ;
                    }
                }
                else
                {
                    kv[VAL] = line ;
                    line.clear() ;

                    done_reading_pair = true ;
                }
            }
    
            if( done_reading_pair )
            {
                if( !kv[KEY].empty() && !kv[VAL].empty() )
                {
                    add_constraint(
                          constraints
                        , kv[KEY]
                        , kv[VAL]
                        , type
                        , inverse
                    ) ;

		    if( constraints.size() && constraints[constraints.size()-1].Processing == CONSTRAINT_PROCESSING_ASYNC )
		    {
			async = true ;
		    }

                    kv[KEY].clear() ;
                    kv[VAL].clear() ;

                    line.clear() ;

                    done_reading_pair = false ;
                    inverse = false ;
                    have_op = false ;
                    have_quot = false ;

                    type = MT_UNDEFINED ;
                    rt = READ_ATTR ;
                }
            }
        }

	return async ;
    }

    template <typename T>
    bool
    determine_match(
          const Constraint_t&   c
        , const MPX::Track_sp&  t
    )
    {
        const MPX::Track& track = *t ; 

        g_return_val_if_fail(track.has(c.TargetAttr), false) ;

        bool truthvalue = false ;

        try{
          switch( c.MatchType )
          {
              case MT_EQUAL:
                  truthvalue = boost::get<T>(track[c.TargetAttr].get()) == boost::get<T>(c.TargetValue.get()) ;
                  break  ;

              case MT_NOT_EQUAL:
                  truthvalue = boost::get<T>(track[c.TargetAttr].get()) != boost::get<T>(c.TargetValue.get()) ;
                  break  ;

              case MT_GREATER_THAN:
                  truthvalue = boost::get<T>(track[c.TargetAttr].get())  > boost::get<T>(c.TargetValue.get()) ;
                  break  ;

              case MT_LESSER_THAN:
                  truthvalue = boost::get<T>(track[c.TargetAttr].get())  < boost::get<T>(c.TargetValue.get()) ;
                  break  ;

              case MT_GREATER_THAN_OR_EQUAL:
                  truthvalue = boost::get<T>(track[c.TargetAttr].get()) >= boost::get<T>(c.TargetValue.get()) ;
                  break  ;

              case MT_LESSER_THAN_OR_EQUAL:
                  truthvalue = boost::get<T>(track[c.TargetAttr].get()) <= boost::get<T>(c.TargetValue.get()) ;
                  break  ;

              default:
                  truthvalue = false  ;
                  break  ;
          }
        } catch( boost::bad_get& cxe )
        {
            g_message("%s: boost::bad_get encountered! %s", G_STRLOC, cxe.what() ) ;
        }

        return (c.InverseMatch) ? !truthvalue : truthvalue ;
    }

    template <>
    bool
    determine_match<std::string>(
          const Constraint_t&   c
        , const MPX::Track_sp&  t
    )
    {
        const MPX::Track& track = *(t.get()) ;

        g_return_val_if_fail(track.has(c.TargetAttr), false) ;
        g_return_val_if_fail(bool(c.TargetValue), false) ;

        bool truthvalue = false ;

        try{
          switch( c.MatchType )
          {
              case MT_EQUAL:
                  truthvalue = boost::get<std::string>(track[c.TargetAttr].get()) == boost::get<std::string>(c.TargetValue.get()) ;
                  break  ;

              case MT_NOT_EQUAL:
                  truthvalue = boost::get<std::string>(track[c.TargetAttr].get()) != boost::get<std::string>(c.TargetValue.get()) ;
                  break  ;

              case MT_GREATER_THAN:
                  truthvalue = boost::get<std::string>(track[c.TargetAttr].get())  > boost::get<std::string>(c.TargetValue.get()) ;
                  break  ;

              case MT_LESSER_THAN:
                  truthvalue = boost::get<std::string>(track[c.TargetAttr].get())  < boost::get<std::string>(c.TargetValue.get()) ;
                  break  ;

              case MT_GREATER_THAN_OR_EQUAL:
                  truthvalue = boost::get<std::string>(track[c.TargetAttr].get()) >= boost::get<std::string>(c.TargetValue.get()) ;
                  break  ;

              case MT_LESSER_THAN_OR_EQUAL:
                  truthvalue = boost::get<std::string>(track[c.TargetAttr].get()) <= boost::get<std::string>(c.TargetValue.get()) ;
                  break  ;

              case MT_FUZZY_EQUAL:
                  truthvalue = Util::match_keys(boost::get<std::string>(track[c.TargetAttr].get()), boost::get<std::string>(c.TargetValue.get())) ;
                  break  ;

              case MT_EQUAL_BEGIN:

                  {
                      Glib::ustring m = Glib::ustring(boost::get<std::string>(track[c.TargetAttr].get())).lowercase() ;
                      Glib::ustring v = Glib::ustring(boost::get<std::string>(c.TargetValue.get())).lowercase() ;

                      truthvalue = (!m.empty() && !v.empty()) && m.substr( 0, v.length()) == v ;
                  }

                  break  ;
          
              case MT_UNDEFINED:
              default:
                  truthvalue = false  ;
                  break  ;
          }
        } catch( boost::bad_get& cxe )
        {
            g_message("%s: boost::bad_get encountered! %s, MatchType: %d, TargetAttr: %d", G_STRLOC, cxe.what(), int(c.MatchType), int(c.TargetAttr) ) ;
        }

        return (c.InverseMatch) ? !truthvalue : truthvalue ;
    }

    template <>
    bool
    determine_match<StrS>(
          const Constraint_t&   c
        , const MPX::Track_sp&  t
    )
    {
        const MPX::Track& track = *t ;

        g_return_val_if_fail(track.has(c.TargetAttr), false) ;

        bool truthvalue = false ;

        try{
          const StrS&         strset              = boost::get<StrS>(c.TargetValue.get()) ;
          const std::string&  track_target_val    = boost::get<std::string>(track[c.TargetAttr].get()) ;

          switch( c.MatchType )
          {
              case MT_EQUAL:
                  truthvalue = strset.count( track_target_val ) ; 
                  break  ;
         
              case MT_UNDEFINED:
              default:
                  break  ;
          }
        } catch( boost::bad_get& cxe )
        {
            g_message("%s: boost::bad_get encountered! %s", G_STRLOC, cxe.what() ) ;
        }

        return (c.InverseMatch) ? !truthvalue : truthvalue ;
    }

    bool
    match_track(
          const Constraints_t&  constraints
        , const MPX::Track_sp&  t
    )
    {
	if(!t)
	    return false ;

        const MPX::Track& track = *t ;

        for( auto c : constraints ) 
        {
            if( !track[c.TargetAttr])
            {
                return false ;
            }
        
            bool truthvalue = false ; 

            if( c.TargetValue && c.TargetValue.get().which() == VT_STRSET )
            {
                truthvalue = determine_match<StrS>( c, t ) ;
            }
            else
            if( c.TargetAttr >= ATTRIBUTE_TRACK )
            {
                truthvalue = determine_match<unsigned int>( c, t ) ;
            }
            else
            {
                truthvalue = determine_match<std::string>( c, t ) ;
            }

            if( !truthvalue )
            {
                return false ;
            }
        }

        return true ;
    }
}
}
