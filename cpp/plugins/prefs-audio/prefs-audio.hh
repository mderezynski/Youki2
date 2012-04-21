//  MPX
//  Copyright (C) 2005-2007 MPX development.
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
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
//  The MPX project hereby grants permission for non GPL-compatible GStreamer
//  plugins to be used and distributed together with GStreamer and MPX. This
//  permission is above and beyond the permissions granted by the GPL license
//  MPX is covered by.

#ifndef MPX_PREFS_AUDIO_HH
#define MPX_PREFS_AUDIO_HH

#include "config.h"

#include <glibmm.h>
#include <gtkmm.h>
#include <string>
#include <set>
#include <vector>

#include "mpx/mpx-main.hh" 
#include "mpx/mpx-services.hh"
#include "mpx/widgets/widgetloader.hh"
#include "mpx/plugin-types.hh"

namespace MPX
{
    class CoverArtSourceView ;
    class FileFormatPrioritiesView ;

    enum Sink
    {
          SINK_ALSA
        , SINK_GCONF
        , SINK_OSS
        , SINK_SUNAUDIO
        , SINK_ESD
        , SINK_HAL
        , SINK_PULSEAUDIO
        , SINK_JACKSINK
        , SINK_AUTO
    };

    class PrefsAudio
    : public Gnome::Glade::WidgetLoader<Gtk::VBox>
    , public PluginHolderBase
    {
        public:

            PrefsAudio(
                  const Glib::RefPtr<Gnome::Glade::Xml>&
                , gint64
            ) ;

            static PrefsAudio*
            create(
                  gint64
            ) ;

            virtual
            ~PrefsAudio();

            virtual bool activate(
            )
            {
                return true ;
            }

            virtual bool deactivate(
            )
            {
                return true ;
            }

            virtual Gtk::Widget* get_gui(
            )
            {
                return 0 ;
            }

        private:

            class AudioSystemColumnRecord
            : public Gtk::TreeModel::ColumnRecord
            {
              public:

                Gtk::TreeModelColumn<Glib::ustring> description;
                Gtk::TreeModelColumn<std::string>   name;
                Gtk::TreeModelColumn<int>           tab;
                Gtk::TreeModelColumn<Sink>          sink;

                AudioSystemColumnRecord()
                {
                    add (description);
                    add (name);
                    add (tab);
                    add (sink);
                }
            };

            AudioSystemColumnRecord   audio_system_columns;
            std::vector<int>          audio_system_cbox_ids;

            Gtk::StockID
            get_plugin_stock(bool /*available*/);

            void
            setup_audio_widgets ();

            void
            setup_audio ();

            void
            audio_system_apply_set_sensitive ();
          
            void
            audio_system_apply_set_insensitive ();

            void
            audio_system_changed ();

            void
            audio_system_apply ();

            Gtk::Button                       * m_button_audio_system_apply;
            Gtk::Button                       * m_button_audio_system_reset;
            Gtk::ComboBox                     * m_cbox_audio_system;
            Gtk::HBox                         * m_warning_audio_system_changed;
            Gtk::Notebook                     * m_notebook_audio_system;
            Gtk::Notebook                     * m_notebook_preferences;
            std::set<std::string>               m_sinks;
            Glib::RefPtr<Gtk::ListStore>        m_list_store_audio_systems;

      #ifdef HAVE_ALSA
            // ALSA
            struct AlsaDevice
            {
                std::string   m_handle;
                int	          m_id_card;
                int	          m_id_device;
                std::string   m_name;

                AlsaDevice()
                {}

                AlsaDevice(
                      const std::string&  handle
                    , int                 id_card
                    , int                 id_device
                    , const std::string&  name
                )
                : m_handle    (handle),
                  m_id_card   (id_card),
                  m_id_device (id_device),
                  m_name      (name) {}
            };
            typedef std::vector <AlsaDevice> AlsaDevices;

          struct AlsaCard
            {
                std::string   m_handle;
                int           m_id_card;
                std::string   m_id;
                std::string   m_name;
                std::string   m_longname;
                std::string   m_driver;
                std::string   m_mixer;

                AlsaDevices   m_devices;

                AlsaCard () {}
                AlsaCard(
                      const std::string&  handle
                    , int                 id_card
                    , const std::string&  id
                    , const std::string&  name
                    , const std::string&  longname
                    , const std::string&  driver
                    , const std::string&  mixer
                    , AlsaDevices&        devices
                )
                : m_handle    (handle),
                  m_id_card   (id_card),
                  m_id        (id),
                  m_name      (name),
                  m_longname  (longname),
                  m_driver    (driver),
                  m_mixer     (mixer)
                {
                    std::swap( devices, m_devices ) ;
                }

                operator Glib::ustring()
                {
                    return Glib::ustring( (m_longname.size() ? m_longname : m_name) ) ;
                }
            } ;

            typedef std::vector<AlsaCard> AlsaCards;

            class AlsaCardColumnRecord
            : public Gtk::TreeModel::ColumnRecord
            {
                public:

                    Gtk::TreeModelColumn <Glib::ustring> name;
                    Gtk::TreeModelColumn <AlsaCard>      card;

                    AlsaCardColumnRecord ()
                    {
                        add (name);
                        add (card);
                    }
            };

            class AlsaDeviceColumnRecord
            : public Gtk::TreeModel::ColumnRecord
            {
                public:

                    Gtk::TreeModelColumn <Glib::ustring> name;
                    Gtk::TreeModelColumn <AlsaDevice>    device;

                    AlsaDeviceColumnRecord ()
                    {
                        add (name);
                        add (device);
                    }
            };

            AlsaCardColumnRecord                m_alsa_card_columns;
            AlsaDeviceColumnRecord              m_alsa_device_columns;
            Gtk::ComboBox                     * m_cbox_alsa_card;
            Gtk::ComboBox                     * m_cbox_alsa_device;
            Gtk::SpinButton                   * m_alsa_buffer_time;
            Gtk::Entry                        * m_alsa_device_string;
            Glib::RefPtr<Gtk::ListStore>        m_list_store_alsa_cards;
            Glib::RefPtr<Gtk::ListStore>        m_list_store_alsa_device;
            sigc::connection                    m_conn_alsa_card_changed;
            sigc::connection                    m_conn_alsa_device_changed;
            sigc::connection                    m_conn_alsa_device_string_changed;

            AlsaCards
            get_alsa_cards(
            ) ;

            void
            on_alsa_card_changed(
            ) ;

            void
            on_alsa_device_changed(
            ) ;

            void
            on_alsa_device_string_changed(
            ) ;
#endif //HAVE_ALSA

            // OSS
            Gtk::ComboBoxEntry                * m_oss_cbe_device;
            Gtk::SpinButton                   * m_oss_buffer_time;

            // ESD
            Gtk::Entry                        * m_esd_host;
            Gtk::SpinButton                   * m_esd_buffer_time;

            // PulseAudio
            Gtk::Entry                        * m_pulse_server;
            Gtk::Entry                        * m_pulse_device;
            Gtk::SpinButton                   * m_pulse_buffer_time;

            // Jack
            Gtk::Entry                        * m_jack_server;
            Gtk::SpinButton                   * m_jack_buffer_time;

#ifdef HAVE_SUN
            Gtk::ComboBoxEntry                * m_sun_cbe_device;
            Gtk::SpinButton                   * m_sun_buffer_time;
#endif // HAVE_SUN

#ifdef HAVE_HAL
            Gtk::Entry                        * m_halaudio_udi;
#endif // HAVE_SUN

    }; // class PrefsAudio
} // namespace MPX

extern "C" MPX::PluginHolderBase*
get_instance(gint64 id)
{
    return MPX::PrefsAudio::create( id ) ;
}

extern "C" bool
del_instance(MPX::PluginHolderBase* b)
{
    delete dynamic_cast<MPX::PrefsAudio*>(b) ;
    return true ;
}

#endif // MPX_PREFS_AUDIO_HH
