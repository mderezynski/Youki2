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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif //HAVE_CONFIG_H

#include <gtk/gtk.h>
#include <gtkmm/alignment.h>
#include <gtkmm/stock.h>
#include <sigc++/bind.h>

#include "mpx/widgets/task-dialog.hh"

namespace MPX
{
  bool
  TaskButton::on_focus_in_event (GdkEventFocus* event)
  {
    set_relief (Gtk::RELIEF_NORMAL);
    Gtk::Widget::on_focus_in_event (event);
    return false;
  }

  bool
  TaskButton::on_focus_out_event (GdkEventFocus* event)
  {
    set_relief (Gtk::RELIEF_NONE);
    Gtk::Widget::on_focus_out_event (event);
    return false;
  }

  TaskButton::TaskButton (  Glib::ustring const&  title,
                            Glib::ustring const&  description,
                            Gtk::StockID          stock,
                            int                   response)
      : id (response)
  {
    m_hbox.set_homogeneous (false);
    m_vbox.set_homogeneous (false);
    m_hbox.set_spacing (0);
    m_vbox.set_spacing (0);

    m_image.set (Gtk::StockID (stock), Gtk::ICON_SIZE_SMALL_TOOLBAR);
    m_image.set_alignment ( 0.5, 0 );

    m_hbox.pack_start (m_image, false, false, 4);
    m_hbox.pack_start (m_vbox, true, true, 0);
    m_hbox.set_border_width (3);

    m_l1.set_markup( "<big>" + title + "</big>" );
    m_l1.set_alignment( 0, 0.5 );
    m_vbox.pack_start( m_l1, false, false, 0 );

    m_l2.set_markup ( description );
    m_l2.set_alignment( 0, 0.5 );
    m_vbox.pack_start( m_l2, false, false, 0 );

    //Gtk::Container::set_border_width (10);
    set_relief (Gtk::RELIEF_NONE);

    add (m_hbox);
  };

  TaskButton::~TaskButton ()
  {
  }

  /////////////////////////////////////////////////////////////////

  TaskDialog::~TaskDialog ()
  {
  }

  TaskDialog::TaskDialog(

        Gtk::Widget        * parent,
        const Glib::ustring& window_title,
        const Glib::ustring& title,
        Gtk::MessageType     type,
        const Glib::ustring& desc

  )

  : Gtk::Window     (Gtk::WINDOW_TOPLEVEL)
  , m_last_response (0)
  , m_mainloop      (0)

  {
    set_title (window_title);

    if (parent)
    {
      if (dynamic_cast <Gtk::Window*> (parent))
      {
        set_transient_for (*(dynamic_cast <Gtk::Window*> (parent)));
      }
      else
      {
        set_screen (parent->get_screen());
      }
    }

    Gtk::Alignment* alignment_outer = Gtk::manage (new Gtk::Alignment);
    alignment_outer->set (0.5, 0.5, 1., 1.);
    alignment_outer->set_padding ( 12, 0, 8, 0);

    Gtk::Alignment* alignment_main = Gtk::manage (new Gtk::Alignment);
    alignment_main->set (0.5, 0.5, 1., 1.);
    alignment_main->set_padding ( 0, 0, 12, 12);

    m_hbox_main.set_homogeneous (false);
    m_hbox_main.set_spacing (8);

    alignment_outer->add (m_hbox_main);
    m_hbox_main.pack_start (m_image, false, false, 0);
    m_hbox_main.pack_start (*alignment_main, false, false, 0);

    add (*alignment_outer);

    switch (type)
    {
    case Gtk::MESSAGE_INFO:
      m_image.set (Gtk::Stock::DIALOG_INFO, Gtk::ICON_SIZE_DIALOG);
      break;

    case Gtk::MESSAGE_WARNING:
      m_image.set (Gtk::Stock::DIALOG_WARNING, Gtk::ICON_SIZE_DIALOG);
      break;

    case Gtk::MESSAGE_ERROR:
      m_image.set (Gtk::Stock::DIALOG_ERROR, Gtk::ICON_SIZE_DIALOG);
      break;

    case Gtk::MESSAGE_QUESTION:
      m_image.set (Gtk::Stock::DIALOG_QUESTION, Gtk::ICON_SIZE_DIALOG);
      break;

  //// gtkmm 2.10 only:
  //  case Gtk::MESSAGE_OTHER:
  //// FIXME: What to set here?
  //    break;

    default:
      break;
    }

    m_image.set_alignment ( 0.5, 0. );

    m_vbox_main.set_homogeneous (false);
    m_vbox_main.set_spacing (8);
    alignment_main->add (m_vbox_main);

    m_main_text.set_markup ("<b><big>" + title + "</big></b>");
    m_main_text.set_alignment ( 0, 0.5 );
    m_vbox_main.pack_start (m_main_text, false, false, 0);

    if (desc.length())
    {
      m_sub_text.set_line_wrap ();
      m_sub_text.set_justify (Gtk::JUSTIFY_FILL);
      m_sub_text.set_markup ("<small>" + desc + "</small>");
      m_sub_text.set_alignment ( 0, 0.5 );
      m_vbox_main.pack_start (m_sub_text, false, false, 0);
    }

    m_vbox_buttons.set_homogeneous (false);
    m_vbox_buttons.set_spacing (12);

    Gtk::Alignment *alignment_m_buttons = Gtk::manage (new Gtk::Alignment);
    alignment_m_buttons->set (0.5, 0.5, 1., 1.);
    alignment_m_buttons->set_padding ( 20, 24, 12, 0);
    alignment_m_buttons->add (m_vbox_buttons);

    m_vbox_main.pack_start (*alignment_m_buttons, false, false, 0);

    set_default_size (700, -1);
    set_resizable (false);
    set_position (Gtk::WIN_POS_CENTER);
  }

  void
  TaskDialog::set_default_response (int response)
  {
    for (TaskButton_map::iterator b = m_buttons.begin(), e = m_buttons.end(); b != e; ++b)
    {
      b->second->property_can_default() = false;
    }

    for (TaskButton_map::iterator b = m_buttons.begin(), e = m_buttons.end(); b != e; ++b)
    {
      if (b->first == response)
      {
        b->second->property_can_default() = true;
        b->second->property_has_default() = true;
        b->second->grab_default ();
        b->second->grab_focus ();
        break;
      }
    }
  }

  void
  TaskDialog::emit_response (int response)
  {
    signal_response_.emit (response);
    m_last_response = response;

    if (m_mainloop)
    {
      m_mainloop->quit ();
    }
  }

  bool
  TaskDialog::on_delete_event (GdkEventAny* event)
  {
    signal_response_.emit (Gtk::RESPONSE_DELETE_EVENT);
    m_last_response = Gtk::RESPONSE_DELETE_EVENT;

    if (m_mainloop)
    {
      m_mainloop->quit ();
    }

    return false;
  }

  int
  TaskDialog::run ()
  {
    show_all ();
    present ();
    m_mainloop = Glib::MainLoop::create ();

    GDK_THREADS_LEAVE ();
    m_mainloop->run();
    GDK_THREADS_ENTER ();

    return m_last_response;
  }

  void
  TaskDialog::add_button (Glib::ustring const& title,
                          Glib::ustring const& description,
                          Gtk::StockID         stock,
                          int                  response)
  {
    TaskButton_ptr b (new TaskButton (title, description, stock, response));
    b->show_all ();
    m_vbox_buttons.pack_start ((*(b.get())), false, false, 0);
    m_buttons.insert (std::make_pair (response, b));
    b->signal_clicked().connect (sigc::bind (sigc::mem_fun (this, &TaskDialog::emit_response), response));
  }
}
