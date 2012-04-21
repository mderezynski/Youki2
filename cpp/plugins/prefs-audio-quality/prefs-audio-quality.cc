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

#include <glibmm/i18n.h>
#include <gtkmm.h>

#include "mpx/mpx-main.hh"
#include "mpx/widgets/widgetloader.hh"

#include "prefs-audio-quality.hh"
#include "mpx/i-youki-preferences.hh"

using namespace Glib ;
using namespace Gtk ;

namespace MPX
{
    namespace
    {
       struct FileFormatInfo
        {
            std::string     MIME;
            std::string     Name;
            std::string     Description;
        } ;

        FileFormatInfo formats[] =
        {
            {
                "audio/x-flac",
                "FLAC",
                "FLAC (Free Lossless Audio Codec)"
            },

            {
                "audio/x-ape",
                "APE",
                "Monkey's Audio"
            },

            {
                "audio/x-vorbis+ogg",
                "OGG",
                "Ogg Vorbis"
            },

            {
                "audio/x-musepack",
                "MPC",
                "Musepack"
            },

            {
                "audio/mp4",
                "MP4",
                "AAC/MP4 (Advanced Audio Codec/MPEG 4 Audio)"
            },

            {
                "audio/mpeg",
                "MP3",
                "MPEG 1 Layer III Audio"
            },

            {
                "audio/x-ms-wma",
                "WMA",
                "Microsoft Windows Media Audio"
            }
        } ;

    }                            // namespace

    using namespace Gnome::Glade;

    typedef sigc::signal<void, int, bool> SignalColumnState;

    class FileFormatPrioritiesView
    : public WidgetLoader<Gtk::TreeView>
    {
        public:

            struct Signals_t
            {
                SignalColumnState ColumnState;
            };

            Signals_t Signals;

            class Columns_t : public Gtk::TreeModelColumnRecord
            {
                public:

                    Gtk::TreeModelColumn<std::string>   MIME;
                    Gtk::TreeModelColumn<std::string>   Name;
                    Gtk::TreeModelColumn<std::string>   Description;

                    Columns_t ()
                    {
                        add(MIME);
                        add(Name);
                        add(Description);
                    };
            };

            Columns_t                       Columns;
            Glib::RefPtr<Gtk::ListStore>    Store;

            FileFormatPrioritiesView (const Glib::RefPtr<Gnome::Glade::Xml>& xml)
                : WidgetLoader<Gtk::TreeView>(xml, "preferences-treeview-fileformatprefs")
            {
                Store = Gtk::ListStore::create(Columns);
                set_model(Store);

                append_column(_("Name"), Columns.Name);
                append_column(_("Description"), Columns.Description);
                append_column(_("MIME Type"), Columns.MIME);

                typedef std::map<std::string, FileFormatInfo> map_t;
                map_t m;

                for( unsigned n = 0; n < G_N_ELEMENTS(formats); ++n )
                {
                    m.insert( std::make_pair( formats[n].MIME, formats[n] ));
                }

                for( unsigned n = 0; n < G_N_ELEMENTS(formats); ++n )
                {
                    TreeIter iter = Store->append();

                    std::string mime = mcs->key_get<std::string>("Preferences-FileFormatPriorities", (boost::format ("Format%d") % n).str());

                    const FileFormatInfo& info = m.find( mime )->second;

                    (*iter)[Columns.MIME]           = info.MIME ;
                    (*iter)[Columns.Name]           = info.Name ;
                    (*iter)[Columns.Description]    = info.Description ;
                }

                Store->signal_row_deleted().connect(
                    sigc::mem_fun(
                        *this,
                        &FileFormatPrioritiesView::on_row_deleted
                ));
            };

            void
                update_configuration ()
            {
                using namespace Gtk;

                TreeNodeChildren const& children = Store->children();
                for( TreeNodeChildren::const_iterator i = children.begin(); i != children.end(); ++i )
                {
                    int n = std::distance( children.begin(), i );
                    mcs->key_set<std::string>("Preferences-FileFormatPriorities", (boost::format ("Format%d") % n).str(), (*i)[Columns.MIME]);
                }
            }

            virtual void
                on_row_deleted(
                  const TreeModel::Path&        G_GNUC_UNUSED
            )
            {
                update_configuration ();
            }
    };

    //// PrefsAudioQuality
    PrefsAudioQuality*
        PrefsAudioQuality::create(
              gint64 id
        )
    {
        return new PrefsAudioQuality (Gnome::Glade::Xml::create (build_filename(DATA_DIR, "glade" G_DIR_SEPARATOR_S "cppmod-prefs-audio-quality.glade")), id );
    }

    PrefsAudioQuality::~PrefsAudioQuality ()
    {
    }

    PrefsAudioQuality::PrefsAudioQuality(
          const Glib::RefPtr<Gnome::Glade::Xml>&    xml
        , gint64                                    id    
    )
        : Gnome::Glade::WidgetLoader<Gtk::VBox>(xml, "cppmod-prefs-audio-quality")
        , PluginHolderBase()
    {
        show() ;

        m_Name = "PreferencesModule AUDIO_QUALITY" ;
        m_Description = "This plugin provides audio quality preferences" ;
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
            , _("Audio Quality")
        ) ;

        // File Format Priorities
        m_Fmts_PrioritiesView = new FileFormatPrioritiesView(m_Xml);
        m_Xml->get_widget("cb-prioritize-by-filetype", m_Fmts_PrioritizeByFileType);
        m_Xml->get_widget("cb-prioritize-by-bitrate", m_Fmts_PrioritizeByBitrate);

        m_Fmts_PrioritizeByFileType->signal_toggled().connect(
            sigc::compose(
            sigc::mem_fun(*m_Fmts_PrioritiesView, &Gtk::Widget::set_sensitive),
            sigc::mem_fun(*m_Fmts_PrioritizeByFileType, &Gtk::ToggleButton::get_active)
            ));

        mcs_bind->bind_toggle_button(
            *m_Fmts_PrioritizeByFileType
            , "Preferences-FileFormatPriorities"
            , "prioritize-by-filetype"
            );

        mcs_bind->bind_toggle_button(
            *m_Fmts_PrioritizeByBitrate
            , "Preferences-FileFormatPriorities"
            , "prioritize-by-bitrate"
            );

        m_Fmts_PrioritiesView->set_sensitive( mcs->key_get<bool>("Preferences-FileFormatPriorities", "prioritize-by-filetype" ));
    }
}  // namespace MPX
