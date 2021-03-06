#include "mpx/widgets/timed-confirmation.hh"
#include "config.h"
#include <gtkmm.h>
#include <boost/format.hpp>
#include <string>

namespace MPX
{
    namespace
    {
        const std::string ui_path = DATA_DIR G_DIR_SEPARATOR_S "ui" G_DIR_SEPARATOR_S "timed-confirmation.ui";
    }

    TimedConfirmation::TimedConfirmation(
        Glib::ustring const& text,
        int                  seconds
    )
        : WidgetLoader<Gtk::Dialog>(Gtk::Builder::create_from_file(ui_path), "dialog")
        , m_Seconds(seconds)
    {
        m_Button_Cancel = add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
        m_Button_OK = add_button((boost::format("OK (%d)") % seconds).str(), Gtk::RESPONSE_OK);

        m_Button_Cancel->signal_clicked().connect(
            sigc::bind(
                sigc::mem_fun(
                    *this,
                    &TimedConfirmation::button_clicked
                ),
                Gtk::RESPONSE_CANCEL
        ));

        m_Button_OK->signal_clicked().connect(
            sigc::bind(
                sigc::mem_fun(
                    *this,
                    &TimedConfirmation::button_clicked
                ),
                Gtk::RESPONSE_OK
        ));
    }

    TimedConfirmation::~TimedConfirmation()
    {
    }

    int
    TimedConfirmation::run (Glib::ustring const& markup)
    {
        get_vbox()->set_border_width(24);
        get_vbox()->set_size_request(-1,250);

        Gtk::Label *label = 0;
        m_Builder->get_widget("label", label);

        label->set_markup( markup );
        get_vbox()->pack_start( *label, true, true );
        label->property_wrap() = true;
        label->property_wrap_mode() = Pango::WRAP_WORD;
        label->show_all();

        Gtk::Window::present();

        m_TimeoutConnection = Glib::signal_timeout().connect(
            sigc::mem_fun(
                *this,
                &TimedConfirmation::handler 
            ),
            1000
        );

        m_MainLoop = Glib::MainLoop::create();
        GDK_THREADS_LEAVE ();
        m_MainLoop->run();
        GDK_THREADS_ENTER ();
    
        return m_Response;    
    }

    bool
    TimedConfirmation::handler()
    {
        m_Seconds--;
        m_Button_OK->set_label((boost::format ("OK (%d)") % m_Seconds).str());

        if( m_Seconds == 0 )
        {
            m_MainLoop->quit();
            return false;
        }

        return true;
    }

    void
    TimedConfirmation::button_clicked (int response)
    {
        m_TimeoutConnection.disconnect();

        m_Response = response; 
        m_MainLoop->quit();
    }
}
