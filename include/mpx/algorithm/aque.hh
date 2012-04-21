#ifndef MPX_AQE_HH
#define MPX_AQE_HH

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>
#include "mpx/mpx-sql.hh"
#include "mpx/mpx-types.hh"
#include "mpx/util-string.hh"
#include <vector>
#include <glib.h>

namespace MPX
{
namespace AQE
{
    enum MatchType_t
    {
          MT_UNDEFINED
        , MT_EQUAL
        , MT_NOT_EQUAL
        , MT_EQUAL_BEGIN
        , MT_GREATER_THAN
        , MT_LESSER_THAN
        , MT_GREATER_THAN_OR_EQUAL
        , MT_LESSER_THAN_OR_EQUAL
        , MT_FUZZY_EQUAL
    };

    struct Constraint_t
    {
        int                     TargetAttr ;
        MPX::OVariant           TargetValue ;
        MatchType_t             MatchType ;
        bool                    InverseMatch ;

        Constraint_t ()
        : InverseMatch( false )
        {
        }
    };

    typedef std::vector<Constraint_t> Constraints_t;

    bool operator == (const Constraint_t& a, const Constraint_t& b ) ;
    bool operator < (const Constraint_t& a, const Constraint_t& b ) ;

    void
    parse_advanced_query(
          Constraints_t&        /*OUT: constraints*/
        , const std::string&    /*IN:  text*/
        , StrV&
    ) ;

    template <typename T>
    bool
    determine_match(
          const Constraint_t&   /*IN: constraints*/
        , const MPX::Track_sp&  /*IN: track*/
    ) ;

    template <>
    bool
    determine_match<std::string>(
          const Constraint_t&   /*IN: constraints*/
        , const MPX::Track_sp&  /*IN: track*/
    ) ;

    template <>
    bool
    determine_match<StrS>(
          const Constraint_t&   /*IN: constraints*/
        , const MPX::Track_sp&  /*IN: track*/
    ) ;

    bool
    match_track(
          const Constraints_t&
        , const MPX::Track_sp&
    ) ;
}
}

#endif
