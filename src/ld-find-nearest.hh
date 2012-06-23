/*-
 | This source code is part of PATL (Practical Algorithm Template Library)
 | Released under the BSD License (http://www.opensource.org/licenses/bsd-license.php)
 | Copyright (c) 2005, 2007..2009, Roman S. Klyujkov (uxnuxn AT gmail.com)
 |
 | Interactive demo of filtering words using Levenshtein distance
-*/

#include <uxn/patl/config.hpp>
#include <uxn/patl/trie_set.hpp>
#include <uxn/patl/levenshtein.hpp>
#include <uxn/patl/maxrep_iterator.hpp>
#include <uxn/patl/super_maxrep_iterator.hpp>
#include <uxn/patl/suffix_set.hpp>
#include <uxn/patl/partial.hpp>
#include <glibmm.h>

namespace patl = uxn::patl;

namespace MPX
{
    struct RankedString 
    {
	std::string Value ;
	guint	    Rank ;

	RankedString()
	: Rank(0)
	{}	

	RankedString(
	      std::string   value
	    , guint	    rank 
	)
	: Value(std::move(value))
	, Rank(rank)
	{}	
    
	RankedString(const RankedString& other)
	: Value(other.Value)
	, Rank(other.Rank)
	{}

	bool operator<(const RankedString& other)
	{
	    return Rank < other.Rank ; 
	}
    } ;

    typedef std::vector<RankedString> RankedStringSet_t ;

    class LDFindNearest
    {
	private:

	    typedef patl::trie_set<std::string> string_set ;
	    typedef string_set::const_iterator const_iter ;
            typedef patl::levenshtein_tp_distance<string_set, false> leven_dist ;
            typedef patl::hamming_distance<string_set, false> hamming_dist ;

	    string_set mDict ;

	public:	

	    LDFindNearest() 
	    {}

	    virtual ~LDFindNearest()
	    {}

	    void
	    clear_dictionary()
	    {
		mDict.clear() ;
	    }

	    void
	    load_dictionary(
		const std::vector<std::string>& v_
	    ) 
	    {
		string_set::iterator hint(mDict.end()) ;

		for( auto& s : v_ )
		{
		    hint = mDict.insert(hint, s) ;		    
		}
	    }

	    auto
	    find_nearest_match_leven_set(
		  guint max_distance
		, const std::string& match
	    ) -> RankedStringSet_t 
	    {
		leven_dist ld(mDict, max_distance, match) ;

		string_set::const_partimator<leven_dist>
		      beg(mDict.begin(ld))
		    , end(mDict.end(ld))
		    , it(beg)
		;

		RankedStringSet_t r ;

		for( ; it != end; ++it )
		{
		    if( it.decis().distance() <= max_distance )
		    {
			r.push_back(RankedString(*it, it.decis().distance())) ;
		    }
		}

		std::sort(r.begin(), r.end()) ;

		return r ;
	    }

	    auto
	    find_nearest_match_leven(
		  guint max_distance
		, const std::string& match
	    ) -> std::string
	    {
		leven_dist ld(mDict, max_distance, match) ;
		string_set::const_partimator<leven_dist>
		      beg(mDict.begin(ld))
		    , end(mDict.end(ld))
		    , it(beg)
		;
		guint cdist = max_distance + 1 ;
		std::string r ;
		for( ; it != end; ++it)
		{
		    if( it.decis().distance() < cdist )
		    {
			cdist = it.decis().distance() ;
			r = *it ;
		    }
		}
		return r ;
	    }

	    auto
	    find_nearest_match_hamm(
		  guint max_distance
		, const std::string& match
	    ) -> std::string
	    {
		hamming_dist ld(mDict, max_distance, match) ;
		string_set::const_partimator<hamming_dist>
		      beg(mDict.begin(ld))
		    , end(mDict.end(ld))
		    , it(beg)
		;
		guint cdist = max_distance + 1 ;
		std::string r ;
		for( ; it != end; ++it)
		{
		    if( it.decis().distance() < cdist )
		    {
			cdist = it.decis().distance() ;
			r = *it ;
		    }
		}
		return r ;
	    }

    } ;
}
