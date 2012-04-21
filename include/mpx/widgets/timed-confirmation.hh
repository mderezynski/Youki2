#ifndef MPX_TIMED_CONFIRMATION_HH
#define MPX_TIMED_CONFIRMATION_HH

#include "mpx/widgets/widgetloader.hh"

#include <glibmm/main.h>
#include <glibmm/ustring.h>
#include <gtkmm/button.h>
#include <gtkmm/dialog.h>

namespace MPX
{
class TimedConfirmation
: public Gnome::Glade::WidgetLoader<Gtk::Dialog>
{
    public:

        TimedConfirmation (Glib::ustring const&, int);
        virtual ~TimedConfirmation ();

        int run (Glib::ustring const&);

    private:

        void
        button_clicked (int);

        bool
        handler ();

        int                          m_Response;
        int                          m_Seconds;
        Gtk::Button                * m_Button_OK;
        Gtk::Button                * m_Button_Cancel;
        sigc::connection             m_TimeoutConnection;
        Glib::RefPtr<Glib::MainLoop> m_MainLoop;
};
}

#endif
