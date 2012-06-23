#ifndef _YOUKI_PD_HBUTTONBOX__HH
#define _YOUKI_PD_HBUTTONBOX__HH

#include "config.h"
#include <gtkmm.h>

namespace MPX
{
    class PercentualDistributionHButtonBox
    : public Gtk::HButtonBox
    {
        protected:

            std::vector<double> m_widths ;

	    virtual void get_preferred_width_vfunc(int& min, int& nat)
	    {
		min = 3*36;
		nat = 3*36;
	    }

        public:

            PercentualDistributionHButtonBox () {} 
    
            virtual ~PercentualDistributionHButtonBox () {}

            void 
            add_percentage(
                  double p
            )
            {
                m_widths.push_back( p ) ;
                queue_resize() ;
            }

            void
            set_percentage(
                  std::vector<double>::size_type    n 
                , double                            p
            )
            {
                m_widths[n] = p ; 
                queue_resize() ;
            }

        protected:

            void
            on_size_allocate(
                  Gtk::Allocation&  alloc
            )
            {
                set_allocation( alloc ) ;

                std::vector<Gtk::Widget*> children = get_children();

                if( children.empty() )
                    return ;

                Gtk::Allocation alloc_child = alloc ;

                double overall_alloc_width = alloc.get_width() ;
                overall_alloc_width -= get_spacing() * (children.size()-1) ;

		g_message("Overall width: %f", overall_alloc_width);

                for( unsigned int n = 0 ; n < children.size () ; n++ )
                {
                    double width_t = overall_alloc_width * m_widths[n] ;

                    alloc_child.set_width( width_t ) ;
                    children[n]->size_allocate( alloc_child ) ;

                    alloc_child.set_x( alloc_child.get_x() + width_t + get_spacing() ) ;
                }
            }
    };
}

#endif // _YOUKI_PD_BOX__HH
