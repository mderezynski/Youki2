//  MPX
//  Copyright (C) 2003-2007 MPX Development
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

#include "config.h"

#include <utility>
#include <iostream>

#include <glibmm.h>
#include <glib/gi18n.h>
#include <gtk/gtkstock.h>
#include <gst/gst.h>
#include <gtkmm.h>
#include <libglademm.h>

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#ifdef HAVE_ALSA
#  define ALSA_PCM_NEW_HW_PARAMS_API
#  define ALSA_PCM_NEW_SW_PARAMS_API

#  include <alsa/global.h>
#  include <alsa/asoundlib.h>
#  include <alsa/pcm_plugin.h>
#  include <alsa/control.h>
#endif

#include "mpx/mpx-main.hh"
#include "mpx/mpx-stock.hh"
#include "mpx/util-string.hh"
#include "mpx/widgets/widgetloader.hh"

#include "mpx/i-youki-play.hh"
#include "mpx/i-youki-preferences.hh"

#include "prefs-audio.hh"

using namespace Glib;
using namespace Gtk;

namespace MPX
{
    namespace
    {
        //// VARIOUS TOGGLE BUTTONS 
        struct DomainKeyPair
        {
            char const* domain;
            char const* key;
            char const* widget;
	    bool connect ;
        };

        DomainKeyPair buttons[] =
        {
            {     "audio"
                , "accurate-seek"
                , "cb-accurate-seek"
		, false
            },
        };

        class SignalBlocker
        {
            public:

                SignalBlocker (sigc::connection & connection)
                    : m_conn(connection)
                {
                    m_conn.block ();
                }

                ~SignalBlocker ()
                {
                    m_conn.unblock();
                }

            private:
                sigc::connection & m_conn;
        };

        struct AudioSystem
        {
            char const* description;
            char const* name;
            gint        tab;
            Sink        sink;
        };

        AudioSystem audiosystems[] =
        {
            {
                "Automatic Choice",
                "autoaudiosink",
                0,
                SINK_AUTO
            },

            {
                "GNOME Configured Audio Output",
                "gconfaudiosink",
                1,
                SINK_GCONF
            },

            #ifdef HAVE_ALSA
            {
                "ALSA Audio Output",
                "alsasink",
                2,
                SINK_ALSA
            },
            #endif               //HAVE_ALSA

            {
                "OSS",
                "osssink",
                3,
                SINK_OSS
            },

            #ifdef HAVE_SUN
            {
                "Sun/Solaris Audio",
                "sunaudiosink",
                4,
                SINK_SUNAUDIO
            },
            #endif               // HAVE_SUN

            {
                "ESD",
                "esdsink",
                5,
                SINK_ESD
            },

            #ifdef HAVE_HAL
            {
                "HAL",
                "halaudiosink",
                6,
                SINK_HAL
            },
            #endif               // HAVE_HAL

            {
                "PulseAudio",
                "pulsesink",
                7,
                SINK_PULSEAUDIO
            },

            {
                "JACK",
                "jackaudiosink",
                8,
                SINK_JACKSINK
            },
        } ;

        bool
        test_element(
              const std::string& name
        )
        {
            GstElementFactory* factory = gst_element_factory_find (name.c_str ());

            bool exists = ( factory != NULL ) ;

            if( factory )
            {
                gst_object_unref( factory ) ;
            }

            return exists;
        }
    } // namespace

    using namespace Gnome::Glade;

    PrefsAudio*
        PrefsAudio::create(
              gint64 id
        )
    {
        return new PrefsAudio(
                Gnome::Glade::Xml::create(
                    build_filename(
                          DATA_DIR
                        , "glade" G_DIR_SEPARATOR_S "cppmod-prefs-audio.glade"
                ))
                , id);
    }

    PrefsAudio::~PrefsAudio ()
    {
    }


    PrefsAudio::PrefsAudio(
          const Glib::RefPtr<Gnome::Glade::Xml>&    xml
        , gint64                                    id
    )
        : Gnome::Glade::WidgetLoader<Gtk::VBox>(xml, "cppmod-prefs-audio")
        , PluginHolderBase()
    {
        show() ;

        m_Name = "IPreferencesModule AUDIO" ;
        m_Description = "This plugin provides audio preferences" ;
        m_Authors = "M. Derezynski" ;
        m_Copyright = "(C) 2009 MPX Project" ;
        m_IAge = 0 ;
        m_Website = "http://redmine.sivashs.org/projects/mpx" ;
        m_Active = false ;
        m_HasGUI = false ;
        m_CanActivate = false ;
        m_Hidden = true ;
        m_Id = id ;

        boost::shared_ptr<IPreferences> p = services->get<IPreferences>("mpx-service-preferences") ;

        p->add_page(
              this
            , _("Audio Output")
        ) ;

        // Audio

        m_Xml->get_widget ("cbox_audio_system", m_cbox_audio_system);

#ifdef HAVE_ALSA
        m_Xml->get_widget ("cbox_alsa_card", m_cbox_alsa_card);
        m_Xml->get_widget ("cbox_alsa_device", m_cbox_alsa_device);
        m_Xml->get_widget ("alsa_buffer_time", m_alsa_buffer_time);
        m_Xml->get_widget ("alsa_device_string", m_alsa_device_string);
#endif //// ALSA

#ifdef HAVE_SUN
        m_Xml->get_widget ("sun_cbe_device",  m_sun_cbe_device);
        m_Xml->get_widget ("sun_buffer_time", m_sun_buffer_time);
#endif //// SUN

        m_Xml->get_widget ("oss_cbe_device", m_oss_cbe_device);
        m_Xml->get_widget ("oss_buffer_time", m_oss_buffer_time);

        m_Xml->get_widget ("esd_host", m_esd_host);
        m_Xml->get_widget ("esd_buffer_time", m_esd_buffer_time);

        m_Xml->get_widget ("pulse_server", m_pulse_server);
        m_Xml->get_widget ("pulse_device", m_pulse_device);
        m_Xml->get_widget ("pulse_buffer_time", m_pulse_buffer_time);

        m_Xml->get_widget ("jack_server", m_jack_server);
        m_Xml->get_widget ("jack_buffer_time", m_jack_buffer_time);

        m_Xml->get_widget ("notebook_audio_system", m_notebook_audio_system);
        m_Xml->get_widget ("audio-system-apply-changes", m_button_audio_system_apply);
        m_Xml->get_widget ("audio-system-reset-changes", m_button_audio_system_reset);
        m_Xml->get_widget ("audio-system-changed-warning", m_warning_audio_system_changed);

        for (unsigned n = 0; n < G_N_ELEMENTS( buttons ); ++n)
        {
            ToggleButton* button = dynamic_cast<ToggleButton*>( m_Xml->get_widget( buttons[n].widget ));

            if( button )
            {
                mcs_bind->bind_toggle_button(
                      *button
                    , buttons[n].domain
                    , buttons[n].key
                ) ;

		if( buttons[n].connect )
		{
			button->signal_toggled().connect(
			    sigc::mem_fun(
				  *this
				, &PrefsAudio::audio_system_apply_set_sensitive
			));
		}
            }
            else
            {
                g_warning( "%s: Widget '%s' not found in Glade::Xml", G_STRLOC, buttons[n].widget );
            }
        }

        setup_audio_widgets ();
        setup_audio ();
    }

    void
        PrefsAudio::audio_system_changed ()
    {
        m_notebook_audio_system->set_current_page ((*m_cbox_audio_system->get_active ())[audio_system_columns.tab]);
    }

    void
        PrefsAudio::audio_system_apply_set_insensitive ()
    {
        m_warning_audio_system_changed->set_sensitive(false);
        m_button_audio_system_apply->set_sensitive(false);
        m_button_audio_system_reset->set_sensitive(true);
    }

    void
        PrefsAudio::audio_system_apply_set_sensitive ()
    {
#ifdef HAVE_ALSA
        if( m_notebook_audio_system->get_current_page() == 2 )
        {
            if( m_cbox_alsa_device->get_active_row_number () == -1 && m_cbox_alsa_device->is_sensitive() )
            {
                m_button_audio_system_reset->set_sensitive (true);
                return;          // user must choose a device when there is also a device to choose
            }
        }
#endif
        m_warning_audio_system_changed->set_sensitive (true);
        m_button_audio_system_apply->set_sensitive (true);
        m_button_audio_system_reset->set_sensitive (true);
    }

    void
        PrefsAudio::audio_system_apply ()
    {
        TreeModel::iterator iter (m_cbox_audio_system->get_active ());
        Sink sink = (*iter)[audio_system_columns.sink];
        std::string name = (*iter)[audio_system_columns.name];
        mcs->key_set<std::string> ("audio", "sink", name);

        switch (sink)
        {
#ifdef HAVE_ALSA
            case SINK_ALSA:
            {
                std::string device = m_alsa_device_string->get_text();
                mcs->key_set<std::string>( "audio", "device-alsa", device );
                break;
            }
#endif //// ALSA
            default: ;
        }

        m_warning_audio_system_changed->set_sensitive(false);
        m_button_audio_system_apply->set_sensitive(false);
        m_button_audio_system_reset->set_sensitive(false);

        services->get<IPlay>("mpx-service-play")->reset();
    }

    #ifdef HAVE_ALSA
    void
        PrefsAudio::on_alsa_device_string_changed ()
    {
        std::string alsa_device = m_alsa_device_string->get_text();

        if( alsa_device.size()>=2 && alsa_device.substr(0,2) == "hw" )
        {
            SignalBlocker B1 ( m_conn_alsa_card_changed );
            SignalBlocker B2 ( m_conn_alsa_device_changed );

            std::vector<std::string> subs;

            using namespace boost::algorithm;
            split (subs, alsa_device, is_any_of (":,"));

            if (subs.size()==3 && subs[0] == "hw")
            {
                int card (atoi(subs[1].c_str()));
                int device (atoi(subs[2].c_str()));

                Glib::RefPtr<Gtk::TreeModel> model = m_cbox_alsa_card->get_model();
                for( Gtk::TreeNodeChildren::const_iterator iter = ++(model->children().begin()); iter != model->children().end(); ++iter)
                {
                    int id = AlsaCard ((*iter)[m_alsa_card_columns.card]).m_id_card;
                    if( id == card )
                    {
                        m_cbox_alsa_card->set_active( iter );

                        m_list_store_alsa_device->clear ();
                        AlsaCard const& card = (*m_cbox_alsa_card->get_active ())[m_alsa_card_columns.card];
                        for (AlsaDevices::const_iterator i = card.m_devices.begin () ; i != card.m_devices.end() ; ++i)
                        {
                            TreeModel::iterator iter (m_list_store_alsa_device->append ());
                            (*iter)[m_alsa_device_columns.name]   = i->m_name;
                            (*iter)[m_alsa_device_columns.device] = *i;
                        }

                        if( subs[2].empty() )
                        {
                            m_cbox_alsa_device->set_active( -1 );
                            m_cbox_alsa_device->set_sensitive( !card.m_devices.empty() );
                            if( !card.m_devices.empty() )
                            {
                                audio_system_apply_set_insensitive();
                            }
                            return;
                        }

                        Glib::RefPtr<Gtk::TreeModel> model_device = m_cbox_alsa_device->get_model();
                        for( Gtk::TreeNodeChildren::const_iterator iter2 = model_device->children().begin(); iter2 != model_device->children().end(); ++iter2)
                        {
                            int id = AlsaDevice ((*iter2)[m_alsa_device_columns.device]).m_id_device;
                            if( id == device )
                            {
                                m_cbox_alsa_device->set_active( iter2 );
                                m_cbox_alsa_device->set_sensitive( true );
                                return;
                            }
                        }
                        m_cbox_alsa_device->set_active (-1);
                        return;
                    }
                }
                m_cbox_alsa_card->set_active (-1);
            }
        }
        else if( alsa_device == "default" )
        {
            m_cbox_alsa_card->set_active (0);
        }
        else
        {
            m_cbox_alsa_card->set_active (-1);
            m_cbox_alsa_device->set_active (-1);
            audio_system_apply_set_sensitive();
        }
    }

    void
        PrefsAudio::on_alsa_device_changed ()
    {
        int row = m_cbox_alsa_device->get_active_row_number ();

        if( row == -1 )
        {
            return;
        }

        TreeModel::iterator iter (m_cbox_alsa_device->get_active ());
        if( iter )
        {
            SignalBlocker B1 ( m_conn_alsa_device_string_changed );
            m_alsa_device_string->set_text( (AlsaDevice ((*iter)[m_alsa_device_columns.device]).m_handle));
        }
    }

    void
        PrefsAudio::on_alsa_card_changed ()
    {
        m_list_store_alsa_device->clear ();

        int row = m_cbox_alsa_card->get_active_row_number ();

        if( row == -1 )
        {
            return;
        }

        AlsaCard const& card = (*m_cbox_alsa_card->get_active ())[m_alsa_card_columns.card];
        for (AlsaDevices::const_iterator i = card.m_devices.begin () ; i != card.m_devices.end() ; ++i)
        {
            TreeModel::iterator iter (m_list_store_alsa_device->append ());
            (*iter)[m_alsa_device_columns.name]   = i->m_name;
            (*iter)[m_alsa_device_columns.device] = *i;
        }

        if (row == 0 || card.m_devices.empty())
        {
            m_cbox_alsa_device->set_active(-1);
            m_cbox_alsa_device->set_sensitive(false);

            SignalBlocker B1 ( m_conn_alsa_device_string_changed );
            m_alsa_device_string->set_text("default");
            return;
        }

        SignalBlocker B1 ( m_conn_alsa_device_string_changed );

        m_cbox_alsa_device->set_active(0);
        m_cbox_alsa_device->set_sensitive(true);

        TreeModel::iterator iter (m_cbox_alsa_device->get_active ());

        if( iter )
        {
            m_alsa_device_string->set_text((AlsaDevice ((*iter)[m_alsa_device_columns.device]).m_handle));
        }
    }

    PrefsAudio::AlsaCards
        PrefsAudio::get_alsa_cards ()
    {
        AlsaCards cards;

        int id_card = -1;

        while( !snd_card_next( &id_card ) && ( id_card > -1 ))
        {
            snd_ctl_t* control = 0;

            if( !snd_ctl_open( &control, (boost::format ("hw:%d") % id_card).str().c_str(), SND_CTL_ASYNC ))
            {
                snd_ctl_card_info_t * card_info = 0;

                snd_ctl_card_info_malloc (&card_info);

                if( !snd_ctl_card_info( control, card_info ))
                {
                    std::string   card_handle   = snd_ctl_name (control);
                    std::string   id_card       = snd_ctl_card_info_get_id (card_info);
                    std::string   card_name     = snd_ctl_card_info_get_name (card_info);
                    std::string   card_longname = snd_ctl_card_info_get_longname (card_info);
                    std::string   card_driver   = snd_ctl_card_info_get_driver (card_info);
                    std::string   card_mixer    = snd_ctl_card_info_get_mixername (card_info);
                    int           card_id_card  = snd_ctl_card_info_get_card (card_info);

                    AlsaDevices   devices;

                    int id_device = -1;

                    while( !snd_ctl_pcm_next_device( control, &id_device ) && ( id_device > -1 ))
                    {
                        snd_pcm_info_t * pcm_info (0);
                        snd_pcm_info_malloc (&pcm_info);
                        snd_pcm_info_set_device (pcm_info, id_device);

                        if( !snd_ctl_pcm_info( control, pcm_info ) )
                        {
                            if( snd_pcm_info_get_stream( pcm_info ) == SND_PCM_STREAM_PLAYBACK )
                            {
                                std::string device_handle  = (boost::format ("%s,%d") % snd_ctl_name (control) % id_device).str();
                                std::string device_name    = snd_pcm_info_get_name (pcm_info);

                                devices.push_back(
                                    AlsaDevice(
                                        device_handle,
                                        card_id_card,
                                        id_device,
                                        device_name
                                ));
                            }
                        }

                        if( pcm_info )
                            snd_pcm_info_free( pcm_info ) ;
                    }

                    if (devices.size())
                    {
                        cards.push_back(
                            AlsaCard(
                                card_handle,
                                card_id_card,
                                id_card,
                                card_name,
                                card_longname,
                                card_driver,
                                card_mixer,
                                devices
                        ));
                    }
                }
                if( card_info )
                    snd_ctl_card_info_free( card_info ) ;
            }
            if( control )
                snd_ctl_close( control ) ;
        }
        return cards;
    }
#endif //// ALSA

#define PRESENT_SINK(n) ((m_sinks.find (n) != m_sinks.end()))
#define CURRENT_SINK(n) ((sink == n))
#define NONE_SINK (-1)

    Gtk::StockID
        PrefsAudio::get_plugin_stock (bool truth)
    {
        return (truth ? StockID(GTK_STOCK_YES) : StockID(GTK_STOCK_NO));
    }

    void
        PrefsAudio::setup_audio_widgets ()
    {
        audio_system_cbox_ids.resize(16);

        CellRendererText * cell;
        m_list_store_audio_systems = ListStore::create (audio_system_columns);
        cell = manage (new CellRendererText() );
        m_cbox_audio_system->clear ();
        m_cbox_audio_system->pack_start (*cell);
        m_cbox_audio_system->add_attribute (*cell, "text", 0);
        m_cbox_audio_system->set_model (m_list_store_audio_systems);

        std::string sink = mcs->key_get<std::string> ("audio", "sink");

        int idx = 0;

        for( unsigned n = 0; n < G_N_ELEMENTS (audiosystems); n++ )
        {
            if( test_element (audiosystems[n].name) )
            {
                audio_system_cbox_ids[n] = idx++;

                m_sinks.insert( audiosystems[n].name ) ;

                TreeIter iter = m_list_store_audio_systems->append ();

                (*iter)[audio_system_columns.description] = audiosystems[n].description;
                (*iter)[audio_system_columns.name] = audiosystems[n].name;
                (*iter)[audio_system_columns.tab] = audiosystems[n].tab;
                (*iter)[audio_system_columns.sink] = audiosystems[n].sink;
            }
        }

        #ifdef HAVE_ALSA
        if (PRESENT_SINK ("alsasink"))
        {
            m_list_store_alsa_cards = ListStore::create (m_alsa_card_columns);
            m_list_store_alsa_device = ListStore::create (m_alsa_device_columns);

            cell = manage (new CellRendererText () );
            m_cbox_alsa_card->clear ();
            m_cbox_alsa_card->pack_start (*cell);
            m_cbox_alsa_card->add_attribute (*cell, "text", 0);

            cell = manage (new CellRendererText () );
            m_cbox_alsa_device->clear ();
            m_cbox_alsa_device->pack_start (*cell);
            m_cbox_alsa_device->add_attribute (*cell, "text", 0);

            m_cbox_alsa_device->set_model (m_list_store_alsa_device);
            m_cbox_alsa_card->set_model (m_list_store_alsa_cards);

            m_conn_alsa_card_changed = m_cbox_alsa_card->signal_changed ().connect
                (sigc::mem_fun (*this, &PrefsAudio::on_alsa_card_changed));
            m_cbox_alsa_card->signal_changed().connect(
                sigc::mem_fun (*this, &PrefsAudio::audio_system_apply_set_sensitive
                ));

            m_conn_alsa_device_changed = m_cbox_alsa_device->signal_changed().connect
                (sigc::mem_fun (*this, &PrefsAudio::on_alsa_device_changed));
            m_cbox_alsa_device->signal_changed().connect
                (sigc::mem_fun (*this, &PrefsAudio::audio_system_apply_set_sensitive));

            mcs_bind->bind_spin_button (*m_alsa_buffer_time, "audio", "alsa-buffer-time");
            m_alsa_buffer_time->signal_value_changed().connect
                (sigc::mem_fun (*this, &PrefsAudio::audio_system_apply_set_sensitive));

            m_conn_alsa_device_string_changed = m_alsa_device_string->signal_changed().connect(
                sigc::mem_fun (*this, &PrefsAudio::on_alsa_device_string_changed
                ));
        }
        #endif                   //HAVE_ALSA

        #ifdef HAVE_SUN
        if (PRESENT_SINK("sunaudiosink"))
        {
            mcs_bind->bind_cbox_entry (*m_sun_cbe_device, "audio", "device-sun");
            mcs_bind->bind_spin_button (*m_sun_buffer_time, "audio", "sun-buffer-time");

            m_sun_buffer_time->signal_value_changed ().connect
                (sigc::mem_fun (*this, &PrefsAudio::audio_system_apply_set_sensitive));
            m_sun_cbe_device->signal_changed ().connect
                (sigc::mem_fun (*this, &PrefsAudio::audio_system_apply_set_sensitive));
        }
        #endif                   //HAVE_SUN

        if (PRESENT_SINK("osssink"))
        {
            mcs_bind->bind_cbox_entry (*m_oss_cbe_device, "audio", "device-oss");
            mcs_bind->bind_spin_button (*m_oss_buffer_time, "audio", "oss-buffer-time");

            m_oss_cbe_device->signal_changed().connect
                (sigc::mem_fun (*this, &PrefsAudio::audio_system_apply_set_sensitive));
            m_oss_buffer_time->signal_value_changed().connect
                (sigc::mem_fun (*this, &PrefsAudio::audio_system_apply_set_sensitive));
        }

        if (PRESENT_SINK("esdsink"))
        {
            mcs_bind->bind_entry (*m_esd_host, "audio", "device-esd");
            mcs_bind->bind_spin_button (*m_esd_buffer_time, "audio", "esd-buffer-time");

            m_esd_buffer_time->signal_value_changed ().connect
                (sigc::mem_fun (*this, &PrefsAudio::audio_system_apply_set_sensitive));
            m_esd_host->signal_changed ().connect
                (sigc::mem_fun (*this, &PrefsAudio::audio_system_apply_set_sensitive));
        }

        if (PRESENT_SINK("pulsesink"))
        {
            mcs_bind->bind_entry (*m_pulse_server, "audio", "pulse-server");
            mcs_bind->bind_entry (*m_pulse_device, "audio", "pulse-device");
            mcs_bind->bind_spin_button (*m_pulse_buffer_time, "audio", "pulse-buffer-time");

            m_pulse_buffer_time->signal_value_changed ().connect
                (sigc::mem_fun (*this, &PrefsAudio::audio_system_apply_set_sensitive));
            m_pulse_server->signal_changed ().connect
                (sigc::mem_fun (*this, &PrefsAudio::audio_system_apply_set_sensitive));
            m_pulse_device->signal_changed ().connect
                (sigc::mem_fun (*this, &PrefsAudio::audio_system_apply_set_sensitive));
        }

        if (PRESENT_SINK("jackaudiosink"))
        {
            mcs_bind->bind_entry (*m_jack_server, "audio", "jack-server");
            mcs_bind->bind_spin_button (*m_jack_buffer_time, "audio", "jack-buffer-time");

            m_jack_buffer_time->signal_value_changed ().connect
                (sigc::mem_fun (*this, &PrefsAudio::audio_system_apply_set_sensitive));
            m_jack_server->signal_changed ().connect
                (sigc::mem_fun (*this, &PrefsAudio::audio_system_apply_set_sensitive));
        }

        #ifdef HAVE_HAL
        if (PRESENT_SINK("halaudiosink"))
        {
            m_Xml->get_widget("halaudio_udi", m_halaudio_udi);
            mcs_bind->bind_entry (*m_halaudio_udi, "audio", "hal-udi");
            m_halaudio_udi->signal_changed ().connect
                (sigc::mem_fun (*this, &PrefsAudio::audio_system_apply_set_sensitive));
        }
        #endif                   // HAVE_HAL

        m_button_audio_system_apply->signal_clicked().connect
            (sigc::mem_fun (*this, &PrefsAudio::audio_system_apply));

        m_button_audio_system_reset->signal_clicked().connect
            (sigc::mem_fun (*this, &PrefsAudio::setup_audio));

        m_cbox_audio_system->signal_changed().connect
            (sigc::mem_fun (*this, &PrefsAudio::audio_system_changed));

        m_cbox_audio_system->signal_changed().connect
            (sigc::mem_fun (*this, &PrefsAudio::audio_system_apply_set_sensitive));
    }

    void
        PrefsAudio::setup_audio ()
    {
        m_notebook_audio_system->set_current_page( 0 );

        std::string sink = mcs->key_get<std::string> ("audio", "sink");
        int x = NONE_SINK;

        for (unsigned n = 0; n < G_N_ELEMENTS (audiosystems); n++)
        {
            if (PRESENT_SINK(audiosystems[n].name) && CURRENT_SINK(audiosystems[n].name))
            {
                x = n;
                break;
            }
        }

        if( x == NONE_SINK )
        {
            x = SINK_AUTO;       // we fallback to autoaudiosink if garbage or unavailable output is in the config file
        }

        m_cbox_audio_system->set_active (audio_system_cbox_ids[x]);

        if( x != NONE_SINK )
        {
            m_notebook_audio_system->set_current_page (audiosystems[x].tab);
        }

        #ifdef HAVE_ALSA
        if (PRESENT_SINK ("alsasink"))
        {
            AlsaCards cards (get_alsa_cards());

            m_list_store_alsa_cards->clear ();
            m_list_store_alsa_device->clear ();

            TreeModel::iterator iter = m_list_store_alsa_cards->append ();
            (*iter)[m_alsa_card_columns.name] = _("System Default");

            for (AlsaCards::iterator i = cards.begin (); i != cards.end (); ++i)
            {
                iter = m_list_store_alsa_cards->append ();
                (*iter)[m_alsa_card_columns.name] = ustring (*i);
                (*iter)[m_alsa_card_columns.card] = *i;
            }

            if (CURRENT_SINK ("alsasink"))
            {
                std::string alsa_device (mcs->key_get<std::string> ("audio", "device-alsa"));

                if( alsa_device.size()>=2 && alsa_device.substr(0,2) == "hw" )
                {
                    SignalBlocker B1 ( m_conn_alsa_card_changed );
                    SignalBlocker B2 ( m_conn_alsa_device_changed );

                    std::vector<std::string> subs;

                    using namespace boost::algorithm;
                    split (subs, alsa_device, is_any_of (":,"));

                    if (subs.size()==3 && subs[0] == "hw")
                    {
                        int card (atoi(subs[1].c_str()));
                        int device (atoi(subs[2].c_str()));

                        Glib::RefPtr<Gtk::TreeModel> model = m_cbox_alsa_card->get_model();
                        for( Gtk::TreeNodeChildren::const_iterator iter = ++(model->children().begin()); iter != model->children().end(); ++iter)
                        {
                            int id = AlsaCard ((*iter)[m_alsa_card_columns.card]).m_id_card;
                            if( id == card )
                            {
                                m_cbox_alsa_card->set_active( iter );

                                m_list_store_alsa_device->clear ();
                                AlsaCard const& card = (*m_cbox_alsa_card->get_active ())[m_alsa_card_columns.card];
                                for (AlsaDevices::const_iterator i = card.m_devices.begin () ; i != card.m_devices.end() ; ++i)
                                {
                                    TreeModel::iterator iter (m_list_store_alsa_device->append ());
                                    (*iter)[m_alsa_device_columns.name]   = i->m_name;
                                    (*iter)[m_alsa_device_columns.device] = *i;
                                }

                                if( subs[2].empty() )
                                {
                                    m_cbox_alsa_device->set_active( -1 );
                                    m_cbox_alsa_device->set_sensitive( !card.m_devices.empty() );
                                    if( !card.m_devices.empty() )
                                    {
                                        audio_system_apply_set_insensitive();
                                    }
                                    goto out1;
                                }

                                Glib::RefPtr<Gtk::TreeModel> model_device = m_cbox_alsa_device->get_model();
                                for( Gtk::TreeNodeChildren::const_iterator iter2 = model_device->children().begin(); iter2 != model_device->children().end(); ++iter2)
                                {
                                    int id = AlsaDevice ((*iter2)[m_alsa_device_columns.device]).m_id_device;
                                    if( id == device )
                                    {
                                        m_cbox_alsa_device->set_active( iter2 );
                                        m_cbox_alsa_device->set_sensitive( true );

                                        SignalBlocker B1 ( m_conn_alsa_device_string_changed );
                                        m_alsa_device_string->set_text(alsa_device);
                                        goto out1;
                                    }
                                }
                                m_cbox_alsa_device->set_active (-1);
                                goto out1;
                            }
                        }
                        m_cbox_alsa_card->set_active (-1);
                    }
                }
                else if( alsa_device == "default" )
                {
                    m_cbox_alsa_card->set_active (0);
                }
                else
                {
                    m_cbox_alsa_card->set_active (-1);
                    m_cbox_alsa_device->set_active (-1);
                    SignalBlocker B1 ( m_conn_alsa_device_string_changed );
                    m_alsa_device_string->set_text(alsa_device);
                }

                out1:
                ;
            }
        }
        #endif                   //HAVE_ALSA

        #ifdef HAVE_SUN
        if (PRESENT_SINK("sunaudiosink"))
        {
            mcs->key_push ("audio", "device-sun");
            mcs->key_push ("audio", "sun-buffer-time");
        }
        #endif                   //HAVE_SUN

        if (PRESENT_SINK("osssink"))
        {
            mcs->key_push ("audio", "device-oss");
            mcs->key_push ("audio", "oss-buffer-time");
        }

        if (PRESENT_SINK("esdsink"))
        {
            mcs->key_push ("audio", "device-esd");
            mcs->key_push ("audio", "esd-buffer-time");
        }

        if (PRESENT_SINK("pulsesink"))
        {
            mcs->key_push ("audio", "pulse-server");
            mcs->key_push ("audio", "pulse-device");
            mcs->key_push ("audio", "pulse-buffer-time");
        }

        if (PRESENT_SINK("jackaudiosink"))
        {
            mcs->key_push ("audio", "jack-server");
            mcs->key_push ("audio", "jack-buffer-time");
        }

        #ifdef HAVE_HAL
        if (PRESENT_SINK("halaudiosink"))
        {
            mcs->key_push ("audio", "hal-udi");
        }
        #endif                   // HAVE_HAL

        mcs->key_push( "audio","accurate-seek" ) ;

        m_warning_audio_system_changed->set_sensitive(false);

        m_button_audio_system_apply->set_sensitive(false);
        m_button_audio_system_reset->set_sensitive(false);
    }

} // namespace MPX
