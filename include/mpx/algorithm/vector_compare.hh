#ifndef MPX_VECTOR_COMPARE_HH
#define MPX_VECTOR_COMPARE_HH

#include <vector>

namespace MPX
{
    template <typename T>
    bool
    vector_compare(const std::vector<T>& a, const std::vector<T>& b)
    {
	if( a.size() != b.size())
	    return false ;

	typename std::vector<T>::const_iterator i_a = a.begin() ;
	typename std::vector<T>::const_iterator i_b = b.begin() ;

	while( i_a != a.end() && i_b != b.end() )
	{
	    if( *i_a != *i_b )
	    {
		return false ;
	    }

	    ++i_a ;
	    ++i_b ;
	}

	return true ;
    }
}

#endif
