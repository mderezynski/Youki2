#ifndef _YOUKI_CLUTTER_WIDGET__HH
#define _YOUKI_CLUTTER_WIDGET__HH

#include <cluttermm.h>
#include <clutter-gtkmm.h>

namespace MPX
{
    class YoukiClutterWidget
    : public Clutter::Gtk::Embed
    {
        public:

            YoukiClutterWidget () ;
            virtual ~YoukiClutterWidget () {}

        protected:

            virtual void
            on_size_allocate(
                  Gtk::Allocation&
            ) ;

            virtual void
            on_show(
            ) ;

            virtual void
            redraw(
            ) = 0 ;
   };
}

#endif // _YOUKI_CLUTTER_WIDGET__HH
