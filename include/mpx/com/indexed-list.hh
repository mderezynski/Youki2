#ifndef MPX_INDEXED_LIST__HH
#define MPX_INDEXED_LIST__HH

#include <list>

namespace MPX
{
        template <typename T>
        struct IndexedList : public std::list<T>
        {
            typedef typename IndexedList<T>::iterator Iter ;

            const T& 
            operator[] (std::size_t n) const
            {
                Iter i = IndexedList::begin() ;
                std::advance( i, n ) ;
                return *i ;
            }

            T& 
            operator[] (std::size_t n)
            {
                Iter i = IndexedList::begin() ;
                std::advance( i, n ) ;
                return *i ;
            }
        } ;
}

#endif // MPX_INDEXED_LIST__HH
