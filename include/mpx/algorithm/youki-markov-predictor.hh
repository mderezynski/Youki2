#ifndef _MPX_MARKOV_PREDICTOR__HH
#define _MPX_MARKOV_PREDICTOR__HH

#include <glibmm.h>
#include <glib.h>
#include <boost/lexical_cast.hpp>

#include "mpx/algorithm/ntree.hh"
#include "mpx/mpx-main.hh"

#include "library.hh"

namespace MPX
{
    struct GlyphData
    {
        double      Intensity ;
        gunichar    Char ;
    } ;

    class MarkovTypingPredictor
    {
        private:

                NTree<GlyphData>      m_tree ;

                Library             * m_library ;

        public:

                MarkovTypingPredictor()
                {
                    m_library = services->get<Library>("mpx-service-library").get() ;

/*
                    bool exists = m_library->get_sql_db()->table_exists( "markov_predictor_node_chain" ) ;

                    m_library->execSQL( "CREATE TABLE IF NOT EXISTS markov_predictor_node_chain (id INTEGER PRIMARY KEY AUTOINCREMENT, parent INTEGER DEFAULT NULL)" ) ;
                    m_library->execSQL( "CREATE TABLE IF NOT EXISTS markov_predictor_node_data  (id INTEGER, char STRING, intensity INTEGER)" ) ;

                    if( !exists )
*/
                    {
                        SQL::RowV v ;
                        m_library->getSQL( v, "SELECT album_artist FROM album_artist ORDER BY album_artist" ) ; 

                        for( SQL::RowV::iterator i = v.begin(); i != v.end(); ++i )
                        {
                            process_string(
                                boost::get<std::string>((*i)["album_artist"])
                            ) ;
                        }

                        v.clear() ;
                        m_library->getSQL( v, "SELECT album_artist FROM album JOIN album_artist ON album.album_artist_j = album_artist.id WHERE album_playscore > 0.1" ) ; 

                        for( SQL::RowV::iterator i = v.begin(); i != v.end(); ++i )
                        {
                            int inc = 1 ;

                            const Variant& vr = (*i)["album_playscore"] ;

                            if( vr.which() == 0 )
                                inc = boost::get<gint64>( vr ) ;
                            else
                                inc = boost::get<double>( vr ) ;

                            process_string(
                                  boost::get<std::string>((*i)["album_artist"])
                                , inc 
                            ) ;
                        }
                    }
/*
                    else
                    {
                        restore_node( m_tree.Root, 1 ) ;
                    }
*/
                }

                void
                restore_node(
                      const NTree<GlyphData>::Node_SP_t&    node 
                    , int64_t                               parent 
                )
                {
                    SQL::RowV v ;
                    m_library->getSQL(v, (
                        boost::format("SELECT * FROM markov_predictor_node_chain WHERE parent = '%u'")
                            % parent
                    ).str()) ; // get all the node's children

                    if( v.empty() )
                        return ;

                    for( SQL::RowV::iterator i = v.begin(); i != v.end(); ++i ) 
                    {
                        int64_t id = boost::get<gint64>((*i)["id"]) ;

                        SQL::RowV v2 ;
                        m_library->getSQL(v2, (
                            boost::format("SELECT char, intensity FROM markov_predictor_node_data WHERE id = '%u'")
                                % id
                        ).str()) ; // get all the node's children

                        NTree<GlyphData>::Node_SP_t node_new = new NTree<GlyphData>::Node ;

                        gunichar c ;    

                        if( v2[0]["char"].which() == 0 )
                        {
                            c = Glib::ustring(boost::lexical_cast<std::string>(boost::get<gint64>(v2[0]["char"])))[0] ;
                        }
                        else
                        {
                            c = Glib::ustring(boost::get<std::string>(v2[0]["char"]))[0] ;
                        }

                        node_new->Data.Char = c ; 
                        node_new->Data.Intensity = boost::get<gint64>(v2[0]["intensity"]) ;
                        node->append( node_new ) ;
                        restore_node( node_new, id ) ;
                    }
                }

                void
                store_node(
                      const NTree<GlyphData>::Node_SP_t&    node 
                    , int64_t                               parent 
                )
                {
                    for( NTree<GlyphData>::Children_t::iterator n = node->Children.begin(); n != node->Children.end(); ++n )
                    {
                        int64_t id = m_library->execSQL((
                            boost::format("INSERT INTO markov_predictor_node_chain (parent) VALUES ('%u')")
                                % parent
                        ).str()) ; 

                        m_library->execSQL(
                            mprintf("INSERT INTO markov_predictor_node_data (id, char, intensity) VALUES ('%u', '%q', '%d')"
                                , id
                                , Glib::ustring (1, (*n)->Data.Char).c_str()
                                , int((*n)->Data.Intensity)
                        )) ;

                        store_node( *n, id ) ; 
                    }
                }

                virtual ~MarkovTypingPredictor()
                {
/*
                    boost::shared_ptr<Library> lib = services->get<Library>("mpx-service-library") ;

                    m_library->execSQL( "DELETE FROM markov_predictor_node_chain" ) ;
                    m_library->execSQL( "DELETE FROM markov_predictor_node_data" ) ;

                    //// STORE ROOT NODE ////

                    m_library->execSQL((boost::format("INSERT INTO markov_predictor_node_chain (id, parent) VALUES ('%u', NULL)")
                            % int64_t(1) 
                    ).str()) ; 

                    //// STORE REST OF THE TREE ////

                    store_node( m_tree.Root, 1 ) ;
*/
                }

                void
                process_string(     
                      const Glib::ustring&  u
                    , int                   intensity_inc = 1
                )
                {
                    NTree<GlyphData>::Node_SP_t root = m_tree.Root ;
                    NTree<GlyphData>::Node_SP_t curr = root ;

                    for( Glib::ustring::const_iterator i = u.begin(); i != u.end(); ++i )
                    {
                        int found = 0 ;

                        for( NTree<GlyphData>::Children_t::iterator n = curr->Children.begin(); n != curr->Children.end(); ++n )
                        {       
                            GlyphData & data = (*n)->Data ;

                            if( g_unichar_tolower(data.Char) == g_unichar_tolower(*i) )
                            {
                                data.Intensity += intensity_inc ; 
                                curr = *n ;
                                found = 1 ; 
                                break ;
                            }
                        }

                        if( !found )
                        {
                            GlyphData new_glyph_data ;
                            new_glyph_data.Intensity = intensity_inc - 1 ;
                            new_glyph_data.Char = *i ; 
                            NTree<GlyphData>::Node_SP_t n =  new NTree<GlyphData>::Node( new_glyph_data ) ;
                            curr->append( n ) ; 
                            curr = n ;
                        }
                    }
                }
            
                Glib::ustring
                predict(     
                      const Glib::ustring&  u
                )
                {
                    Glib::ustring prediction ;

                    NTree<GlyphData>::Node_SP_t root = m_tree.Root ;
                    NTree<GlyphData>::Node_SP_t curr = root ;

                    for( Glib::ustring::const_iterator i = u.begin(); i != u.end(); ++i )
                    {
                        for( NTree<GlyphData>::Children_t::iterator n = curr->Children.begin(); n != curr->Children.end(); ++n )
                        {       
                            GlyphData & data = (*n)->Data ;

                            if( g_unichar_tolower(data.Char) == g_unichar_tolower(*i) )
                            {
                                curr = *n ;
                                goto continue_loop_in_case_of_found ;
                            }
                        }

                        // this string can not be predicted within the current tree, we reached the end of nodes, but not the end of text
                        return prediction ;

                        continue_loop_in_case_of_found: ;
                    }

                    while( curr && !curr->Children.empty() )
                    {
                        double Intensity_Max = 0 ;
                        NTree<GlyphData>::Node_SP_t Most_Intense_Node  = 0 ;

                        for( NTree<GlyphData>::Children_t::iterator n = curr->Children.begin(); n != curr->Children.end(); ++n )
                        {       
                            GlyphData & data = (*n)->Data ;

                            double Intensity = data.Intensity / curr->Children.size() ;
                           
                            if( Intensity > Intensity_Max ) 
                            {
                                Intensity_Max = Intensity ;
                                Most_Intense_Node = *n ;
                            }
                        }

                        if( Most_Intense_Node )
                        {
                            prediction += Most_Intense_Node->Data.Char ;
                            curr = Most_Intense_Node ;
                        }
                        else
                        if( !curr->Children.empty() )
                        {
                            curr = *(curr->Children.begin()) ;
                            prediction += curr->Data.Char ;
                        }
                    }

                    return prediction ;
                }
    } ;
}

#endif // YOUKI_MARKOV_PREDICTOR
