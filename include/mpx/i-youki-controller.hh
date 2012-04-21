#ifndef _YOUKI_CONTROLLER_BASE__HH
#define _YOUKI_CONTROLLER_BASE__HH

#include <gtkmm.h>
#include <string>

#include "mpx/mpx-types.hh"

namespace MPX
{
    class IYoukiController
    : public Glib::Object
    {
        public: 

            IYoukiController() {} ;
            virtual ~IYoukiController () {} ;

            virtual Gtk::Window*
            get_widget () = 0 ; 

            virtual void
            queue_next_track(
                guint
            ) = 0 ;

            virtual int
            get_status(
            ) = 0 ;

            virtual MPX::Track&
            get_metadata(
            ) = 0 ;

            virtual MPX::Track&
            get_metadata_previous(
            ) = 0 ;

            virtual void
            API_pause_toggle(
            ) = 0 ;

            virtual void
            API_next(
            ) = 0 ;

            virtual void
            API_prev(
            ) = 0 ;

            virtual void
            API_stop(
            ) = 0 ;

            virtual void        
            add_info_widget(
                  Gtk::Widget*
                , const std::string&
            ) = 0 ;

            virtual void
            remove_info_widget(
                  Gtk::Widget*
            ) = 0 ; 
    } ;
}

#endif // YOUKI_CONTROLLER_BASE__HH
