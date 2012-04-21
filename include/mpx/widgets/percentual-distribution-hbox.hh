#ifndef _YOUKI_PD_HBOX__HH
#define _YOUKI_PD_HBOX__HH

#include "config.h"
#include <gtkmm.h>

namespace MPX
{
    class PercentualDistributionHBox
    : public Gtk::HBox
    {
        protected:

            std::vector<double> m_widths ;

        public:

            PercentualDistributionHBox () {} 
    
            virtual ~PercentualDistributionHBox () {}

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

                GList * children = GTK_BOX(gobj())->GSEAL(children) ;

                if( g_list_length(children) == 0 )
                    return ;

                Gtk::Allocation alloc_child = alloc ;

                double overall_alloc_width = alloc.get_width() ;
                overall_alloc_width -= get_spacing() * (g_list_length(children)-1) ;

                for( int n = 0 ; children ; children = children->next )
                {
                    GtkBoxChild * child = static_cast<GtkBoxChild*>(children->data) ;
                    Gtk::Widget * widget = Glib::wrap( child->widget, false ) ;

                    double width_t = overall_alloc_width * m_widths[n] ;

                    alloc_child.set_width( width_t ) ;
                    widget->size_allocate( alloc_child ) ;

                    n++ ;
                    alloc_child.set_x( alloc_child.get_x() + width_t + get_spacing() ) ;
                }
            }
    };
}

#endif // _YOUKI_PD_BOX__HH
