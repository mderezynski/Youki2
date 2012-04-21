//  MPX
//  Copyright (C) 2005-2007 MPX development.
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License Version 2
//  as published by the Free Software Foundation.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
//  --
//
//  The MPX project hereby grants permission for non-GPL compatible GStreamer
//  plugins to be used and distributed together with GStreamer and MPX. This
//  permission is above and beyond the permissions granted by the GPL license
//  MPX is covered by.

#ifndef MPX_TASK_DIALOG_HH
#define MPX_TASK_DIALOG_HH

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif //HAVE_CONFIG_H

#include <map>

#include <glibmm/main.h>
#include <glibmm/ustring.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/window.h>
#include <sigc++/signal.h>

#include <boost/shared_ptr.hpp>

namespace MPX
{
  class TaskButton
    : public Gtk::Button
  {
    private:

      Gtk::HBox   m_hbox;
      Gtk::VBox   m_vbox;
      Gtk::Label  m_l1;
      Gtk::Label  m_l2;
      Gtk::Image  m_image;
      int         id;

    protected:

      virtual bool
      on_focus_in_event (GdkEventFocus* event);

      virtual bool
      on_focus_out_event (GdkEventFocus* event);

    public:

      TaskButton (Glib::ustring const& title,
                  Glib::ustring const& description,
                  Gtk::StockID         stock,
                  int                  response);

      virtual ~TaskButton ();
  };

  class TaskDialog
    : public Gtk::Window
  {
      public:

        typedef sigc::signal<void, int> SignalResponse;

        TaskDialog (Gtk::Widget        * parent,
                    Glib::ustring const& window_title,
                    Glib::ustring const& title,
                    Gtk::MessageType     type,
                    Glib::ustring const& desc = "");
        virtual ~TaskDialog ();

        void  add_button (Glib::ustring const& title,
                          Glib::ustring const& description,
                          Gtk::StockID         stock,
                          int                  response);

        void  set_default_response (int response);
        int   run ();

        SignalResponse&
        signal_response ()
        { return signal_response_; }

      protected:

        virtual bool on_delete_event (GdkEventAny *event);

      private:

        SignalResponse signal_response_;
        void emit_response (int response);
       
        typedef boost::shared_ptr<TaskButton>   TaskButton_ptr;
        typedef std::map<int, TaskButton_ptr>   TaskButton_map;
        typedef TaskButton_map::value_type      TaskButton_map_pair;

        TaskButton_map      m_buttons;
        Gtk::Image          m_image;
        Gtk::Label          m_main_text;
        Gtk::Label          m_sub_text;
        Gtk::HBox           m_hbox_main;
        Gtk::VBox           m_vbox_main;
        Gtk::VBox           m_vbox_buttons;
        int                 m_last_response;

        Glib::RefPtr <Glib::MainLoop> m_mainloop;
  };
} // namespace MPX
#endif //!MPX_TASK_DIALOG_HH
