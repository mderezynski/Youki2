
#ifndef HAVE_BMP

#include <mcs/config.h>
#if (1 != MCS_HAVE_GTK)
#  error "This MCS installation was built without GTK+ binding system support!"
#endif

#endif

#ifndef MCS_BIND_H
#define MCS_BIND_H

#include <gtkmm.h>
#include <mcs/base.h>
#include <vector>
#include <string>

namespace Mcs
{
  class Bind
  {
    private:

      enum RebindLockId
      {
        RL_RANGE,
        RL_RANGE_FLOAT,
        RL_SPINBUTTON,
        RL_CBOX,
        RL_CBOX_ENTRY,
        RL_ENTRY,
        RL_TOGGLE_BUTTON,

        N_REBIND_LOCKS
      };

      std::vector<bool> m_rebind_locks;

      class RebindLock
      {
        public:

          RebindLock (Bind&        bind,
                      RebindLockId lock_id)
            : m_bind  (bind)
            , m_id    (lock_id)
          {
            m_bind.m_rebind_locks[m_id] = true;
          }

          ~RebindLock ()
          {
            m_bind.m_rebind_locks[m_id] = false;
          }

        private:

          Bind & m_bind;
          RebindLockId m_id;
      };

      void
      rebind_range (MCS_CB_DEFAULT_SIGNATURE,
                    Gtk::Range& range);

      void
      rebind_range_float (MCS_CB_DEFAULT_SIGNATURE,
                          Gtk::Range& range);

      void
      rebind_spin_button (MCS_CB_DEFAULT_SIGNATURE,
                          Gtk::SpinButton& button);

      void
      rebind_entry (MCS_CB_DEFAULT_SIGNATURE,
                    Gtk::Entry& entry);

      void
      rebind_cbox (MCS_CB_DEFAULT_SIGNATURE,
                   Gtk::ComboBox& cbox);

      void
      rebind_cbox_entry (MCS_CB_DEFAULT_SIGNATURE,
                         Gtk::ComboBoxEntry& cbox_entry);

      void
      rebind_toggle_button (MCS_CB_DEFAULT_SIGNATURE,
                         Gtk::ToggleButton& toggle_button);

      friend class RebindLock;

    public:

      Bind (Mcs::Config * mcs);
      ~Bind ();

      void
      bind_toggle_action  (Glib::RefPtr<Gtk::ToggleAction> const& toggle_action,
                           std::string const&                     domain,
                           std::string const&                     key);

      void
      bind_spin_button  (Gtk::SpinButton&   spin_button,
                         std::string const& domain,
                         std::string const& key);

      void
      bind_entry          (Gtk::Entry&        entry,
                           std::string const& domain,
                           std::string const& key);
      void
      bind_toggle_button  (Gtk::ToggleButton& button,
                           std::string const& domain,
                           std::string const& key);

      void
      bind_font_button    (Gtk::FontButton&   font_button,
                           std::string const& domain,
                           std::string const& key);

      void
      bind_filechooser    (Gtk::FileChooser&  filechooser,
                           std::string const& domain,
                           std::string const& key);


      void
      bind_cbox_entry     (Gtk::ComboBoxEntry& cbox_entry,
                           std::string const&  domain,
                           std::string const&  key);

      void
      bind_cbox           (Gtk::ComboBox&     cbox,
                           std::string const& domain,
                           std::string const& key);

      void
      bind_range          (Gtk::Range&        range,
                           std::string const& domain,
                           std::string const& key);

      void
      bind_range_float    (Gtk::Range&        range,
                           std::string const& domain,
                           std::string const& key);

    private:

      Mcs::Config *mcs;

      void
      action_toggled_cb       (Glib::RefPtr<Gtk::ToggleAction> const& toggle_action,
                               std::string const&                     domain,
                               std::string const&                     key);

      void
      spin_button_changed_cb  (Gtk::SpinButton&   spin_button,
                               std::string const& domain,
                               std::string const& key);

      void
      entry_changed_cb        (Gtk::Entry&        entry,
                               std::string const& domain,
                               std::string const& key);
      void
      button_toggled_cb       (Gtk::ToggleButton& toggle_button,
                               std::string const& domain,
                               std::string const& key);
      void
      font_set_cb             (Gtk::FontButton&   font_button,
                               std::string const& domain,
                               std::string const& key);

      void
      fc_current_folder_set   (Gtk::FileChooser&  filechooser,
                               std::string const& domain,
                               std::string const& key);

      void
      cbox_entry_changed_cb   (Gtk::ComboBoxEntry& cbox_entry,
                               std::string const&  domain,
                               std::string const&  key);

      void
      cbox_changed_cb         (Gtk::ComboBox&     cbox,
                               std::string const& domain,
                               std::string const& key);

      void
      range_value_changed     (Gtk::Range&        range,
                               std::string const& domain,
                               std::string const& key);

      void
      range_value_changed_float (Gtk::Range&        range,
                                 std::string const& domain,
                                 std::string const& key);

  }; // class Bind

} // namespace Mcs

#endif // MCS_BIND_H
