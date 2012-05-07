#include <vector>
#include <glibmm.h>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include "mpx/mpx-types.hh"
#include "mpx/util-string.hh"
#include "mpx/algorithm/aque.hh"
#include "mpx/mpx-uri.hh"

#include "mpx/xml/xmltoc++.hh"

#include "xmlcpp/xsd-topalbums-2.0.hxx"
#include "xmlcpp/xsd-topartists-2.0.hxx"
#include "xmlcpp/xsd-artist-similar-2.0.hxx"
#include "xmlcpp/xsd-artist-toptracks-2.0.hxx"

namespace
{
    MPX::OVariant
    _lastfm_artist_toptracks(
	  const std::string& value
    )
    {
	MPX::OVariant v ;
	MPX::StrS s ;

	try{
	    MPX::URI u ((boost::format( "http://ws.audioscrobbler.com/2.0/?method=artist.gettoptracks&artist=%s&api_key=37cd50ae88b85b764b72bb4fe4041fe4" ) % Glib::Markup::escape_text(value)).str(), true ) ;

	    MPX::XmlInstance<lfm_artisttoptracks::lfm> *Xml = new MPX::XmlInstance<lfm_artisttoptracks::lfm>(Glib::ustring(u)) ;

	    for( lfm_artisttoptracks::toptracks::track_sequence::const_iterator

		    i  = Xml->xml().toptracks().track().begin() ; 
		    i != Xml->xml().toptracks().track().end() ;

		    ++i
	    )
	    {
		s.insert((*i).mbid()) ;
	    }

	    delete Xml ;
	}
	catch(...) {
	}

	v = s ;
	return v ;
    }

    MPX::OVariant
    _lastfm_artist_similar(
	  const std::string& value
    )
    {
	MPX::OVariant v ;
	MPX::StrS s ;

	try{
	    MPX::URI u ((boost::format( "http://ws.audioscrobbler.com/2.0/?method=artist.getsimilar&artist=%s&api_key=37cd50ae88b85b764b72bb4fe4041fe4" ) % Glib::Markup::escape_text(value)).str(), true ) ;
	    MPX::XmlInstance<lfm_similarartists::lfm> * Xml = new MPX::XmlInstance<lfm_similarartists::lfm>( Glib::ustring( u ) );

	    for( lfm_similarartists::similarartists::artist_sequence::const_iterator i = Xml->xml().similarartists().artist().begin(); i != Xml->xml().similarartists().artist().end(); ++i )
	    {
		s.insert( (*i).mbid() ) ;
	    }

	    delete Xml ;
	}
	catch(...) {
	}

	v = s ;
	return v ;
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

/*
            if( attribute == "lastfm-artist-toptracks" )
            {
                c.TargetAttr = ATTRIBUTE_MB_TRACK_ID ;
		c.Locality = CONSTRAINT_LOCALITY_NETWORK ;

		c.GetValue = sigc::ptr_fun( &_lastfm_artist_toptracks ) ;
                c.TargetValue = c.GetValue() ; 

                constraints.push_back(c) ;
            }
            else
*/
            if( attribute == "lastfm-artist-similar" )
            {
                c.TargetAttr = ATTRIBUTE_MB_ALBUM_ARTIST_ID ;
		c.Locality = CONSTRAINT_LOCALITY_NETWORK ;
		c.SourceValue = value ;
		c.GetValue = sigc::ptr_fun( &_lastfm_artist_similar ) ;	

                constraints.push_back(c) ;
            }
/*
            else
            if( attribute == "lastfm-tag-topartists" )
            {
                c.TargetAttr = ATTRIBUTE_MB_ALBUM_ARTIST_ID ;
		c.Locality = CONSTRAINT_LOCALITY_NETWORK ;

                StrS s ;

                try{
                    URI u ( (boost::format( "http://ws.audioscrobbler.com/2.0/?method=tag.gettopartists&tag=%s&api_key=37cd50ae88b85b764b72bb4fe4041fe4" ) % Glib::Markup::escape_text(value)).str(), true ) ;
                    MPX::XmlInstance<lfm_tagtopartists::lfm> * Xml = new MPX::XmlInstance<lfm_tagtopartists::lfm>( Glib::ustring( u ) );

                    for( lfm_tagtopartists::topartists::artist_sequence::const_iterator i = Xml->xml().topartists().artist().begin(); i != Xml->xml().topartists().artist().end(); ++i )
                    {
                        s.insert( (*i).mbid() ) ;
                    }

                    delete Xml ;
                }
                catch( ... ) {
                        g_message("Exception!");
                }

                c.TargetValue = s ;
                constraints.push_back(c) ;
            }
            else
            if( attribute == "lastfm-tag-topalbums" )
            {
                c.TargetAttr = ATTRIBUTE_MB_ALBUM_ID ;
		c.Locality = CONSTRAINT_LOCALITY_NETWORK ;

                StrS s ;

                try{
                    URI u ( (boost::format( "http://ws.audioscrobbler.com/2.0/?method=tag.gettopalbums&tag=%s&api_key=37cd50ae88b85b764b72bb4fe4041fe4" ) % value).str(), true ) ;
                    MPX::XmlInstance<lfm_tagtopalbums::lfm> * Xml = new MPX::XmlInstance<lfm_tagtopalbums::lfm>( Glib::ustring( u ) );

                    for( lfm_tagtopalbums::topalbums::album_sequence::const_iterator i = Xml->xml().topalbums().album().begin(); i != Xml->xml().topalbums().album().end(); ++i )
                    {
                        s.insert( (*i).mbid() ) ;
                    }

                    delete Xml ;
                }
                catch( ... ) {
                        g_message("Exception!");
                }

                c.TargetValue = s ;
                constraints.push_back(c) ;
            }
*/
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
            if( attribute == "bitrate" )
            {
                try{
                        c.TargetValue = (unsigned int)(boost::lexical_cast<int>(value)) ;
                        c.TargetAttr = ATTRIBUTE_BITRATE ;
                        c.MatchType = type ;

                        constraints.push_back(c) ;
                } catch( boost::bad_lexical_cast ) {
                }
            }
            else
            if( attribute == "year" )
            {
                try{
                        c.TargetValue = (unsigned int)(boost::lexical_cast<int>(value)) ;
                        c.TargetAttr = ATTRIBUTE_DATE ;
                        c.MatchType = type ;

                        constraints.push_back(c) ;
                } catch( boost::bad_lexical_cast ) {
                }
            }
            else
            if( attribute == "quality" )
            {
                c.TargetValue = (unsigned int)(boost::lexical_cast<int>(value)) ;
                c.TargetAttr = ATTRIBUTE_QUALITY ;

                constraints.push_back(c) ;
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
	for( Constraints_t::iterator i = constraints.begin() ; i != constraints.end() ; ++i )
	{
	    Constraint_t& c = *i ;

	    if( c.Locality == CONSTRAINT_LOCALITY_NETWORK )
	    {
		if( !c.GetValue )
		{
		    g_message("%s: Have constraint with LOCALITY_NETWORK but GetValue slot is empty!", G_STRLOC) ;
		    continue ;
		}

		c.TargetValue = c.GetValue( c.SourceValue ) ;
	    } 
	}	
    }

    void
    parse_advanced_query(
          Constraints_t&            constraints
        , const std::string&        text
        , StrV&                     non_attr_strings
    )
    {
        enum KeyValEnum
        {
              KEY
            , VAL
        } ;

        Glib::ustring line ;
        Glib::ustring kv[2] ;

        MatchType_t type ;

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
            if( *i == '"' || *i == '\'')
            {
                have_quot = true ;

                if( rt == READ_ATTR )
                {
                    // we interpret this as the start of the attribute and use MT_EQUAL

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
                    return ;
                }
                else
                {
                    kv[VAL] = line ;
                    line.clear() ;
                    done_reading_pair = true ; 
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
    }

    template <typename T>
    bool
    determine_match(
          const Constraint_t&   c
        , const MPX::Track_sp&  t
    )
    {
        const MPX::Track& track = *(t.get()) ;

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
        const MPX::Track& track = *(t.get()) ;

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
          const Constraints_t&  c
        , const MPX::Track_sp&  t
    )
    {
        const MPX::Track& track = *(t.get()) ;

        for( Constraints_t::const_iterator i = c.begin(); i != c.end(); ++i )
        {
            const Constraint_t& c = *i ;

            if( !track.has( c.TargetAttr ))
            {
                return false ;
            }
        
            bool truthvalue = false ; 

            if( c.TargetValue.get().which() == VT_STRSET )
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
