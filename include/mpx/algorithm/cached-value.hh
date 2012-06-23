#ifndef MPX_ALGO__CACHED_VALUE
#define MPX_ALGO__CACHED_VALUE

#include <boost/optional.hpp>
#include <sigc++/sigc++.h>

namespace MPX
{
    template <typename T>
    class CachedValue
    {
	protected:

	    boost::optional<T> mValue ;

	public:

	    typedef sigc::slot<T> GetSlot ;

	protected:

	    GetSlot mSlot ; 

	public:

	    CachedValue() = delete ;

	    CachedValue(const GetSlot& slot)
	    {
		mSlot = slot ; 
	    }

	    const& T
	    get()
	    {
		if(!mValue)
		    update() ;

		return mValue ;
	    }
	   
	    void 
	    update()
	    {
		mValue = mSlot() ;
	    }
    } ;
}

#endif
