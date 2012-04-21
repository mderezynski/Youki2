#ifndef _YOUKI_NTREE_HH
#define _YOUKI_NTREE_HH

#include <boost/shared_ptr.hpp>

namespace MPX
{
    template <typename T>
    struct NTree
    {
        struct                           Node ;

        typedef Node*                    Node_SP_t ;
        typedef std::vector<Node_SP_t >  Children_t ;

        struct Node
        { 
            public:

                    Node ()
                    {
                    }


                    Node_SP_t               Parent ;
                    Children_t              Children ;
                    T                       Data ;

                    Node(
                        const T&            data
                    )
                    : Data( data )
                    {
                    }

                    void
                    append(
                        Node_SP_t           node
                    )
                    {
                        node->Parent = this ;
                        Children.push_back( node ) ; 
                    }
        } ;

        Node_SP_t Root ;

        NTree()
        {
            Root = new Node ; 
        }

        virtual ~NTree()
        {
            delete_node( Root ) ;
        }

        void
        delete_node(
            Node_SP_t node
        )
        {
            for( std::size_t n = 0 ; n < node->Children.size() ; ++n ) 
            {
                delete_node( node->Children[n] ) ;
            }

            delete node ;
        }
    } ;
}

#endif // _YOUKI_NTREE_HH
