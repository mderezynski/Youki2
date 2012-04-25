#ifndef MPX_ROUNDED_LAYOUT
#define MPX_ROUNDED_LAYOUT

#include <gtkmm.h>
#include "mpx/widgets/widgetloader.hh"
#include "mpx/widgets/cairo-extensions.hh"

namespace MPX
{
    class RoundedLayout : public WidgetLoader<Gtk::DrawingArea>
    {
        private:

                Glib::ustring           m_text ;
                CairoCorners::CORNERS   m_corners ;
                double                  m_radius ;

        protected:

                void
                on_size_request (Gtk::Requisition*);

                virtual bool
                on_expose_event (GdkEventExpose*);

        public:

                RoundedLayout(
                      const Glib::RefPtr<Gtk::Builder>& builder
                    , const std::string&                     name
                    , CairoCorners::CORNERS = CairoCorners::ALL
                    , double = 2.
                ) ;

                virtual ~RoundedLayout() ;

                void
                set_text(Glib::ustring const&);
    };
}


#endif
