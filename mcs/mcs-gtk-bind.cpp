
#include <iostream>
#include <gtkmm.h>

#include "mcs/types.h"
#include "mcs/key.h"
#include "mcs/mcs.h"
#include "mcs/gtk-bind.h"

namespace Mcs
{
  Bind::Bind (Config* mcs)
  : m_rebind_locks (N_REBIND_LOCKS, false)
  , mcs (mcs)
  {}

  Bind::~Bind () {}

  // Rebound
  void
  Bind::rebind_range (MCS_CB_DEFAULT_SIGNATURE,
                      Gtk::Range& range)
  {
    if (m_rebind_locks[RL_RANGE])
      return;

    RebindLock (*this, RL_RANGE);
    range.set_value (boost::get<int> (value));
  }

  void
  Bind::rebind_range_float (MCS_CB_DEFAULT_SIGNATURE,
                            Gtk::Range& range)
  {
    if (m_rebind_locks[RL_RANGE_FLOAT])
      return;

    RebindLock (*this, RL_RANGE_FLOAT);
    range.set_value (boost::get<double> (value));
  }

  void
  Bind::rebind_spin_button (MCS_CB_DEFAULT_SIGNATURE,
                            Gtk::SpinButton& spin_button)
  {
    if (m_rebind_locks[RL_SPINBUTTON])
      return;

    RebindLock (*this, RL_SPINBUTTON);
    spin_button.set_value (boost::get<int> (value));
  }

  void
  Bind::rebind_entry (MCS_CB_DEFAULT_SIGNATURE,
                      Gtk::Entry& entry)
  {
    if (m_rebind_locks[RL_ENTRY])
      return;

    RebindLock (*this, RL_ENTRY);
    entry.set_text (boost::get<std::string> (value));
  }

  void
  Bind::rebind_cbox_entry (MCS_CB_DEFAULT_SIGNATURE,
                           Gtk::ComboBoxEntry& cbox_entry)
  {
    if (m_rebind_locks[RL_CBOX_ENTRY])
      return;

    RebindLock (*this, RL_CBOX_ENTRY);
    cbox_entry.get_entry()->set_text (boost::get<std::string> (value));
  }

  void
  Bind::rebind_cbox (MCS_CB_DEFAULT_SIGNATURE,
                     Gtk::ComboBox& cbox)
  {
    if (m_rebind_locks[RL_CBOX])
      return;

    RebindLock (*this, RL_CBOX);
    cbox.set_active (boost::get<int> (value));
  }

  void
  Bind::rebind_toggle_button (MCS_CB_DEFAULT_SIGNATURE,
                              Gtk::ToggleButton& toggle_button)
  {
    if (m_rebind_locks[RL_TOGGLE_BUTTON])
      return;

    RebindLock (*this, RL_TOGGLE_BUTTON);
    toggle_button.set_active (boost::get<bool> (value));
  }

  //Gtk::Range
  void
  Bind::range_value_changed (Gtk::Range&        range,
                             std::string const& domain,
                             std::string const& key)
  {
    if (m_rebind_locks[RL_RANGE])
      return;

    RebindLock (*this, RL_RANGE);
    KeyVariant key_value (int (range.get_value ()));
    mcs->key_set (domain, key, key_value);
  }

  void
  Bind::bind_range (Gtk::Range&        range,
                    std::string const& domain,
                    std::string const& key)
  {
    range.signal_value_changed ().connect (sigc::bind (sigc::mem_fun (this, &Bind::range_value_changed),
                                                       sigc::ref (range), domain, key));
    range.set_value (double (mcs->key_get<int> (domain, key)));

    mcs->subscribe (domain, key,
                    sigc::bind (sigc::mem_fun (*this, &Bind::rebind_range),
                                sigc::ref (range)));
  }

  //Gtk::Range (storing a double)
  void
  Bind::range_value_changed_float (Gtk::Range&        range,
                                   std::string const& domain,
                                   std::string const& key)
  {
    if (m_rebind_locks[RL_RANGE_FLOAT])
      return;

    RebindLock (*this, RL_RANGE_FLOAT);
    KeyVariant key_value (range.get_value ());
    mcs->key_set (domain, key, key_value);
  }
  void
  Bind::bind_range_float (Gtk::Range&        range,
                          std::string const& domain,
                          std::string const& key)
  {
    range.signal_value_changed ().connect (sigc::bind (sigc::mem_fun (this, &Bind::range_value_changed_float),
                                                       sigc::ref (range), domain, key));
    range.set_value (mcs->key_get <double> (domain, key));

    mcs->subscribe (domain, key,
                    sigc::bind (sigc::mem_fun (this, &Bind::rebind_range_float),
                                sigc::ref (range)));
  }

  //Gtk::SpinButton
  void
  Bind::spin_button_changed_cb (Gtk::SpinButton&   spin_button,
                                std::string const& domain,
                                std::string const& key)
  {
    if (m_rebind_locks[RL_SPINBUTTON])
      return;

    RebindLock (*this, RL_SPINBUTTON);
    KeyVariant key_value = spin_button.get_value_as_int ();
    mcs->key_set (domain, key, key_value);
  }
  void
  Bind::bind_spin_button (Gtk::SpinButton&   spin_button,
                          std::string const& domain,
                          std::string const& key)
  {
    spin_button.signal_value_changed().connect
      (sigc::bind (sigc::mem_fun (this, &Bind::spin_button_changed_cb),
                   sigc::ref (spin_button), domain, key));

    spin_button.set_value (double (mcs->key_get<int> (domain, key)));

    mcs->subscribe (domain, key,
                    sigc::bind (sigc::mem_fun (this, &Bind::rebind_spin_button),
                                sigc::ref (spin_button)));
  }

  //Gtk::Entry
  void
  Bind::entry_changed_cb (Gtk::Entry&        entry,
                          std::string const& domain,
                          std::string const& key)
  {
    if (m_rebind_locks[RL_ENTRY])
      return;

    RebindLock (*this, RL_ENTRY);
    KeyVariant key_value = std::string (entry.get_text ());
    mcs->key_set (domain, key, key_value);
  }
  void
  Bind::bind_entry (Gtk::Entry&        entry,
                    std::string const& domain,
                    std::string const& key)
  {
    entry.signal_changed ().connect (sigc::bind (sigc::mem_fun (this, &Bind::entry_changed_cb),
                                                 sigc::ref (entry), domain, key));
    entry.set_text (mcs->key_get<std::string> (domain, key));

#if 0
    mcs->subscribe ("MCSBIND_INTERNAL_bind_entry", domain, key,
                    sigc::bind (sigc::mem_fun (this, &Bind::rebind_entry),
                                sigc::ref (entry)));
#endif
  }

  //Gtk::ToggleAction
  void
  Bind::action_toggled_cb (Glib::RefPtr<Gtk::ToggleAction> const& toggle_action,
                           std::string const&                     domain,
                           std::string const&                     key)
  {
    KeyVariant key_value = toggle_action->get_active ();
    mcs->key_set (domain, key, key_value);
  }

  void
  Bind::bind_toggle_action (Glib::RefPtr<Gtk::ToggleAction> const& toggle_action,
                            std::string const&	                   domain,
                            std::string const&                     key)
  {
    toggle_action->signal_toggled ().connect
      (sigc::bind (sigc::mem_fun (this, &Bind::action_toggled_cb),
                   toggle_action, domain, key));
    toggle_action->set_active (mcs->key_get<bool> (domain, key));
  }

  //Gtk::ToggleButton
  void
  Bind::button_toggled_cb (Gtk::ToggleButton& toggle_button,
                           std::string const& domain,
                           std::string const& key)
  {
    KeyVariant key_value = toggle_button.get_active ();
    mcs->key_set (domain, key, key_value);
  }
  void
  Bind::bind_toggle_button (Gtk::ToggleButton& toggle_button,
                            std::string const& domain,
                            std::string const& key)
  {
    toggle_button.signal_toggled ().connect
      (sigc::bind (sigc::mem_fun (this, &Bind::button_toggled_cb),
                   sigc::ref (toggle_button), domain, key));
    toggle_button.set_active (mcs->key_get<bool> (domain, key));
    mcs->subscribe (domain, key,
                    sigc::bind (sigc::mem_fun (this, &Bind::rebind_toggle_button),
                                sigc::ref (toggle_button)));
  }

  //Gtk::FileChooser
  void
  Bind::fc_current_folder_set (Gtk::FileChooser&  filechooser,
                               std::string const& domain,
                               std::string const& key)
  {
    KeyVariant key_value = std::string (Glib::filename_from_uri (filechooser.get_current_folder_uri ()));
    mcs->key_set (domain, key, key_value);
  }
  void
  Bind::bind_filechooser (Gtk::FileChooser&  filechooser,
                          std::string const& domain,
                          std::string const& key)
  {
    filechooser.signal_current_folder_changed ().connect
      (sigc::bind (sigc::mem_fun (this, &Bind::fc_current_folder_set),
                   sigc::ref (filechooser), domain, key));
    filechooser.set_current_folder_uri (Glib::filename_to_uri (mcs->key_get<std::string> (domain, key)));
  }

  //Gtk::FontButton
  void
  Bind::font_set_cb (Gtk::FontButton&   font_button,
                     std::string const& domain,
                     std::string const& key)
  {
    KeyVariant key_value = std::string (font_button.get_font_name ());
    mcs->key_set (domain, key, key_value);
  }

  void
  Bind::bind_font_button (Gtk::FontButton&   font_button,
                          std::string const& domain,
                          std::string const& key)
  {
    font_button.signal_font_set ().connect (sigc::bind (sigc::mem_fun (this, &Bind::font_set_cb),
                                                        sigc::ref (font_button), domain, key));
    font_button.property_font_name () = mcs->key_get<std::string> (domain, key);
  }

  //Gtk::ComboBoxEntry
  void
  Bind::cbox_entry_changed_cb (Gtk::ComboBoxEntry& cbox_entry,
                               std::string const&  domain,
                               std::string const&  key)
  {
    if (m_rebind_locks[RL_CBOX_ENTRY])
      return;

    RebindLock (*this, RL_CBOX_ENTRY);
    KeyVariant key_value = std::string (cbox_entry.get_entry ()->get_text ());
    mcs->key_set (domain, key, key_value);
  }

  void
  Bind::bind_cbox_entry (Gtk::ComboBoxEntry& cbox_entry,
                         std::string const&  domain,
                         std::string const&  key)
  {
    cbox_entry.get_entry ()->set_text (mcs->key_get<std::string> (domain, key));
    cbox_entry.signal_changed ().connect (sigc::bind (sigc::mem_fun (this, &Bind::cbox_entry_changed_cb),
                                                      sigc::ref (cbox_entry), domain, key));

    mcs->subscribe (domain, key,
                    sigc::bind (sigc::mem_fun (this, &Bind::rebind_cbox_entry),
                                sigc::ref (cbox_entry)));
  }

  //Gtk::ComboBox
  void
  Bind::cbox_changed_cb (Gtk::ComboBox&     cbox,
                         std::string const& domain,
                         std::string const& key)
  {
    if (m_rebind_locks[RL_CBOX])
      return;

    RebindLock (*this, RL_CBOX);
    KeyVariant key_value = int (cbox.get_active_row_number ());
    mcs->key_set (domain, key, key_value);
  }

  void
  Bind::bind_cbox (Gtk::ComboBox&     cbox,
                   std::string const& domain,
                   std::string const& key)
  {
    cbox.signal_changed ().connect (sigc::bind (sigc::mem_fun (this, &Bind::cbox_changed_cb),
                                                sigc::ref (cbox), domain, key));
    cbox.set_active (mcs->key_get<int> (domain, key));
    mcs->subscribe (domain, key,
                    sigc::bind (sigc::mem_fun (this, &Bind::rebind_cbox),
                                sigc::ref (cbox)));
  }

} // Mcs
