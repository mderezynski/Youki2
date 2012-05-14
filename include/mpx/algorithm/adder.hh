#ifndef MPX_ALGORITHM_ADDER_HH
#define MPX_ALGORITHM_ADDER_HH

namespace MPX	
{
namespace Algorithm
{
	template <typename T>
	struct Adder
	{
	    T& a ;
	    T& b ;

	    T  sum ;

	    Adder( T& a_, T& b_ ):a(a_),b(b_){}

	    operator const T () const
	    {
		return a+b ;
	    }

	    const T operator() () const
	    {
		return a+b ;
	    }
	} ;
}
}

#endif // MPX_ALGORITHM_ADDER_HH


