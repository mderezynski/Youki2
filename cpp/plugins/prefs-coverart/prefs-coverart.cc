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

#include <glibmm.h>
#include <glibmm/i18n.h>
#include <gtkmm.h>

#include <boost/format.hpp>

#include "mpx/mpx-main.hh"
#include "mpx/widgets/widgetloader.hh"

#include "prefs-coverart.hh"
#include "mpx/i-youki-preferences.hh"

using namespace Glib;
using namespace Gtk;

namespace MPX
{
    namespace
    {
        char const* sources[] =
        {
            N_("Local files (folder.jpg, cover.jpg, etc.)"),
            N_("MusicBrainz Advanced Relations"),
            N_("Amazon ASIN"),
            N_("Amazon Search API"),
            N_("Inline Covers (File Metadata)"),
            N_("LastFM Cover Search")
        };
    }  // namespace

    using namespace Gnome::Glade;

    typedef sigc::signal<void, int, bool> SignalColumnState;

    class CoverArtSourceView : public WidgetLoader<Gtk::TreeView>
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

                    Gtk::TreeModelColumn<Glib::ustring> Name;
                    Gtk::TreeModelColumn<int>           ID;
                    Gtk::TreeModelColumn<bool>          Active;

                    Columns_t ()
                    {
                        add(Name);
                        add(ID);
                        add(Active);
                    };
            };

            Columns_t                       Columns;
            Glib::RefPtr<Gtk::ListStore>    Store;

            CoverArtSourceView (const Glib::RefPtr<Gnome::Glade::Xml>& xml)
                : WidgetLoader<Gtk::TreeView>(xml, "preferences-treeview-coverartsources")
            {
                Store = Gtk::ListStore::create(Columns);
                set_model(Store);

                TreeViewColumn *col = manage( new TreeViewColumn(_("Active")));
                CellRendererToggle *cell1 = manage( new CellRendererToggle );
                cell1->property_xalign() = 0.5;
                cell1->signal_toggled().connect(
                    sigc::mem_fun(
                    *this,
                    &CoverArtSourceView::on_cell_toggled
                    ));
                col->pack_start(*cell1, true);
                col->add_attribute(*cell1, "active", Columns.Active);
                append_column(*col);

                append_column(_("Column"), Columns.Name);

                for( unsigned i = 0; i < G_N_ELEMENTS(sources); ++i )
                {
                    TreeIter iter = Store->append();

                    int source = mcs->key_get<int>("Preferences-CoverArtSources", (boost::format ("Source%d") % i).str());

                    (*iter)[Columns.Name]   = _(sources[source]);
                    (*iter)[Columns.ID]     = source;
                    (*iter)[Columns.Active] = mcs->key_get<bool>("Preferences-CoverArtSources", (boost::format ("SourceActive%d") % i).str());
                }

                Store->signal_row_deleted().connect(
                    sigc::mem_fun(
                        *this,
                        &CoverArtSourceView::on_row_deleted
                ));
            };

            void
                update_configuration ()
            {
                using namespace Gtk;

                TreeNodeChildren const& children = Store->children();

                for(TreeNodeChildren::const_iterator i = children.begin(); i != children.end(); ++i)
                {
                    int n = std::distance(children.begin(), i);

                    mcs->key_set<int>("Preferences-CoverArtSources", (boost::format ("Source%d") % n).str(), (*i)[Columns.ID]);
                    mcs->key_set<bool>("Preferences-CoverArtSources", (boost::format ("SourceActive%d") % n).str(), (*i)[Columns.Active]);
                }
            }

            void
                on_cell_toggled(Glib::ustring const& path)
            {
                TreeIter iter = Store->get_iter(path);
                (*iter)[Columns.Active] = !bool((*iter)[Columns.Active]);
                Signals.ColumnState.emit((*iter)[Columns.ID], (*iter)[Columns.Active]); 

                update_configuration ();
            }

            virtual void
                on_row_deleted(
                      const TreeModel::Path&        G_GNUC_UNUSED
            )
            {
                update_configuration ();
            }
    };

    //// PrefsCoverart
    PrefsCoverart*
        PrefsCoverart::create(gint64 id)
    {
        return new PrefsCoverart (Gnome::Glade::Xml::create (build_filename(DATA_DIR, "glade" G_DIR_SEPARATOR_S "cppmod-prefs-coverart.glade")), id );
    }

    PrefsCoverart::~PrefsCoverart ()
    {
    }

    PrefsCoverart::PrefsCoverart(
          const Glib::RefPtr<Gnome::Glade::Xml>&    xml
        , gint64                                    id
    )
        : Gnome::Glade::WidgetLoader<Gtk::VBox>(xml, "cppmod-prefs-coverart")
        , PluginHolderBase()
    {
        m_Name = "PreferencesModule COVERART" ;
        m_Description = "This plugin provides coverart preferences" ;
        m_Authors = "M. Derezynski" ;
        m_Copyright = "(C) 2012 Youki" ;
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
            , _("Coverart Sources")
        ) ;

        m_Covers_CoverArtSources = new CoverArtSourceView(m_Xml);

        show_all() ;
    }
}  // namespace MPX
