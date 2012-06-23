#ifndef MPX_ID_HISTORY__HH
#define MPX_ID_HISTORY__HH

#include <vector>

namespace MPX
{
    class IdHistory
    {
	private:

		typedef std::vector<guint>  History ;
		typedef History::iterator   HistoryPosition ;

		History			    mHistory ;
		HistoryPosition		    mHistoryCurrent ;
		HistoryPosition		    mHistoryFirst ;
		HistoryPosition		    mHistoryLast ;

	public:

		IdHistory()
		{
		    clear() ;
		}

		virtual ~IdHistory()
		{
		}

		void
		print()
		{
		    g_print("\n\n") ;
		    for( History::iterator i = mHistory.begin() ; i != mHistory.end() ; ++i )
		    {
			g_message("%u%c", *i, (i == mHistoryCurrent) ? '*':' ') ;
		    }
		}

		bool
		empty()
		{
		    return mHistory.empty() ;
		}

		void
		clear()
		{
		    mHistory.clear() ;
		    mHistoryFirst = mHistory.begin() ;
		    mHistoryLast = mHistory.end() ;
		    mHistoryCurrent = mHistoryFirst ;
		}

		void
		rewind()
		{
		    mHistoryFirst = mHistory.begin() ;
		}

		void
		prepend(guint id)
		{
		    auto i = mHistory.begin() ;
		    mHistory.insert(i, id) ;

		    mHistoryCurrent = mHistory.begin() ;
		    mHistoryLast = mHistory.end()-1 ; 
		    mHistoryFirst = mHistory.begin() ;
		}

		void
		append(guint id)
		{
		    if( mHistoryCurrent < mHistoryLast )
		    {
			mHistory.erase(mHistoryCurrent+1, mHistory.end()) ;
		    }

		    mHistory.push_back(id) ;

		    mHistoryCurrent = mHistory.end() - 1 ;
		    mHistoryLast = mHistoryCurrent ;
		    mHistoryFirst = mHistory.begin() ;
		}

		guint
		current()
		{
		    return *mHistoryCurrent ;
		}

		guint
		go_back()
		{
		    if( mHistoryCurrent > mHistoryFirst )
		    {
			--mHistoryCurrent ;
		    }

		    return *mHistoryCurrent ;
		}

		guint
		go_ffwd()
		{
		    if( mHistoryCurrent < mHistoryLast )
		    {
			++mHistoryCurrent ;
		    }

		    return *mHistoryCurrent ;
		}

		bool
		has_prev()
		{
		    return !empty() && (mHistoryCurrent > mHistoryFirst) ;
		}

		bool
		has_ffwd()
		{
		    return !empty() && (mHistoryCurrent < mHistoryLast) ;
		}
    } ;
}

#endif
