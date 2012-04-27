//  MPX
//  Copyright (C) 2010 MPX development.
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
//  The MPX project hereby grants permission for non-GPL compatible GStreamer
//  plugins to be used and distributed together with GStreamer and MPX. This
//  permission is above and beyond the permissions granted by the GPL license
//  MPX is covered by.

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <boost/format.hpp>
#include <glibmm.h>
#include <glibmm/i18n.h>

#include "mpx/mpx-main.hh"
#include "mpx/mpx-plugin.hh"

#include "plugin-manager-gui.hh"

using namespace Glib;
using namespace Gtk;

namespace MPX
{
	class PluginTreeView
          : public WidgetLoader<Gtk::TreeView>
	{
            typedef std::map<guint, Gtk::TreeIter> IdIterMap_t;
    
			class ColumnsT : public Gtk::TreeModelColumnRecord
			{
				public:

					Gtk::TreeModelColumn<bool>				Active;
					Gtk::TreeModelColumn<bool>				HasGUI;
					Gtk::TreeModelColumn<bool>				CanActivate;
					Gtk::TreeModelColumn< Glib::RefPtr<Gdk::Pixbuf> >	Pixbuf;
					Gtk::TreeModelColumn<std::string>		Desc;
					Gtk::TreeModelColumn<std::string>		Authors;
					Gtk::TreeModelColumn<std::string>		Copyright;
					Gtk::TreeModelColumn<std::string>		Website;
					Gtk::TreeModelColumn<guint>			Id;
					Gtk::TreeModelColumn<std::string>		Name;
	
				ColumnsT ()
				{
					add(Active);
					add(HasGUI);
					add(CanActivate);
					add(Pixbuf);
					add(Desc);
					add(Authors);
					add(Copyright);
					add(Website);
					add(Id);
					add(Name);
				};
			};

            Gtk::Window                   & m_GUI;

			Glib::RefPtr<Gtk::ListStore>    Store;
			ColumnsT                        Columns;
            Gtk::CellRendererPixbuf       * m_pRendererPixbuf;

            IdIterMap_t                     m_IdIterMap;

		public:

			PluginTreeView(
                  const Glib::RefPtr<Gtk::Builder>& builder
                , PluginManager&                    manager
                , Gtk::Window&                      gui
            )

            : WidgetLoader<Gtk::TreeView>(builder, "treeview")
            , m_GUI(gui)

			{
				Store = Gtk::ListStore::create(Columns);

				Glib::RefPtr<Gtk::IconTheme> Theme = Gtk::IconTheme::get_default();

				append_column_editable(_("Active"), Columns.Active);
				append_column(_("Plugin"), Columns.Name);

				Gtk::TreeViewColumn* pColumn = get_column(0);

                m_pRendererPixbuf = Gtk::manage(new Gtk::CellRendererPixbuf());
                pColumn->pack_start(*m_pRendererPixbuf, true);
                pColumn->add_attribute(m_pRendererPixbuf->property_pixbuf(), Columns.Pixbuf);

				std::vector<Gtk::CellRenderer*> renderers = pColumn->get_cell_renderers();

				Gtk::CellRendererToggle * cell_toggle =	dynamic_cast<Gtk::CellRendererToggle*>(renderers[0]);

				cell_toggle->signal_toggled().connect(
                    sigc::mem_fun(
                        *this,
                        &PluginTreeView::on_cell_toggled
                    )
                );

                pColumn->set_cell_data_func(
                    *renderers[0],
                    sigc::mem_fun(
                        *this,
                        &PluginTreeView::cell_data_func_active
                    )
                );

				PluginHoldMap_t const& map = services->get<PluginManager>("mpx-service-plugins")->get_map();

				for( PluginHoldMap_t::const_iterator i = map.begin(); i != map.end(); ++i )
				{
                    if( !i->second->get_hidden() )
                    {
                            TreeIter iter = Store->append();

                            (*iter)[Columns.Id]             = i->second->get_id();
                            (*iter)[Columns.Name]           = i->second->get_name();
                            (*iter)[Columns.Desc]           = i->second->get_desc();
                            (*iter)[Columns.Authors]        = i->second->get_authors();
                            (*iter)[Columns.Copyright]      = i->second->get_copyright();
                            (*iter)[Columns.Website]        = i->second->get_website();
                            (*iter)[Columns.Active]         = i->second->get_active();
                            (*iter)[Columns.HasGUI]         = i->second->get_has_gui();
                            (*iter)[Columns.CanActivate]    = i->second->get_can_activate();

                            if(!i->second->get_can_activate())
                            {
                                (*iter)[Columns.Pixbuf] = Theme->load_icon("emblem-readonly", 16, Gtk::ICON_LOOKUP_NO_SVG);
                            }

                            m_IdIterMap.insert(std::make_pair(i->second->get_id(), iter));
                    }
				}

                Store->set_default_sort_func( sigc::mem_fun( *this, &PluginTreeView::plugin_sort_func ));
                Store->set_sort_column(-1, Gtk::SORT_ASCENDING);
				set_model (Store);

                manager.signal_plugin_show_gui().connect(
                    sigc::mem_fun(
                        *this,
                        &PluginTreeView::on_plugin_show_gui
                ));
            
                manager.signal_plugin_activated().connect(
                    sigc::mem_fun(
                        *this,
                        &PluginTreeView::on_plugin_activated
                ));

                manager.signal_plugin_deactivated().connect(
                    sigc::mem_fun(
                        *this,
                        &PluginTreeView::on_plugin_deactivated
                ));
			}

            void
            on_plugin_show_gui(guint id)
            {
                TreeIter iter = m_IdIterMap.find(id)->second;
                m_GUI.present();
                get_selection()->select( iter );
            }

            void
            on_plugin_activated(guint id)
            {
                TreeIter iter = m_IdIterMap.find(id)->second;
				(*iter)[Columns.Active] = true;
            }

            void
            on_plugin_deactivated(guint id)
            {
                TreeIter iter = m_IdIterMap.find(id)->second;
				(*iter)[Columns.Active] = false;
            }

            int
            plugin_sort_func(const TreeIter& iter_a, const TreeIter& iter_b)
            {
                Glib::ustring a = std::string((*iter_a)[Columns.Name]);
                Glib::ustring b = std::string((*iter_b)[Columns.Name]);

                return a.compare(b);
            }

            void
            cell_data_func_active (CellRenderer * basecell, TreeIter const& iter)
            {
                CellRendererToggle & cell = *(dynamic_cast<CellRendererToggle*>(basecell));

                if(!bool((*iter)[Columns.CanActivate]))
                {
                    cell.property_visible() = false;
                    m_pRendererPixbuf->property_visible() = true;
                    return;
                }

                cell.property_visible() = true;
                m_pRendererPixbuf->property_visible() = false;
                cell.property_active() = (*iter)[Columns.Active];
            }

			void
			on_cell_toggled (const Glib::ustring& p_string)
			{
				TreePath path (p_string);
				TreeIter iter = Store->get_iter(path);
        
                if(!bool((*iter)[Columns.CanActivate]))
                    return;

				guint id = (*iter)[Columns.Id];
				bool active = (*iter)[Columns.Active];

				if(active)
					services->get<PluginManager>("mpx-service-plugins")->deactivate(id);
				else
					services->get<PluginManager>("mpx-service-plugins")->activate(id);
			}

			Gtk::Widget *
			get_gui (const TreeModel::TreeModel::iterator& iter)
			{
				guint id = (*iter)[Columns.Id];

				if((*iter)[Columns.HasGUI])
					return services->get<PluginManager>("mpx-service-plugins")->get_gui(id);
				else
					return NULL;
			}

			bool
			is_active(const TreeModel::iterator& iter) const
			{
				return (*iter)[Columns.Active];
			}

			bool
			get_has_gui(const TreeModel::iterator& iter) const
			{
				return (*iter)[Columns.HasGUI];
			}

			const std::string
			get_desc(const TreeModel::iterator& iter) const
			{
				return (*iter)[Columns.Desc];
			}

			const std::string
			get_authors(const TreeModel::iterator& iter) const
			{
				return (*iter)[Columns.Authors];
			}

			const std::string
			get_copyright(const TreeModel::iterator& iter) const
			{
				return (*iter)[Columns.Copyright];
			}

			const std::string
			get_website(const TreeModel::iterator& iter) const
			{
				return (*iter)[Columns.Website];
			}

			~PluginTreeView ()
			{
			}
	};
}

namespace MPX
{
		PluginManagerGUI::~PluginManagerGUI ()
		{
            hide();
			delete m_PluginTreeView;
		}

		PluginManagerGUI::PluginManagerGUI(
            const Glib::RefPtr<Gtk::Builder>& builder
        )
        : WidgetLoader<Gtk::Window>(builder, "window")
        , Service::Base("mpx-service-plugins-gui")
		{
		    m_PluginTreeView = new PluginTreeView(builder, *(services->get<PluginManager>("mpx-service-plugins").get()), *this);
			m_PluginTreeView->get_selection()->signal_changed().connect(
                    sigc::mem_fun(
                        *this,
                        &PluginManagerGUI::on_selection_changed
            ));

			builder->get_widget("overview", m_Overview);
			builder->get_widget("options", m_Options);
			builder->get_widget("error", m_Error);
			builder->get_widget("notebook", m_Notebook);
			m_Notebook->get_nth_page(1)->hide();

            Gtk::Button* close_button = 0;
            builder->get_widget("close", close_button);
			close_button->signal_clicked().connect(sigc::mem_fun(*this, &PluginManagerGUI::hide));

			builder->get_widget("traceback", m_Button_Traceback);
			m_Button_Traceback->signal_clicked().connect(
                    sigc::mem_fun(
                        *this,
                        &PluginManagerGUI::show_dialog
            ));

			if(services->get<PluginManager>("mpx-service-plugins")->get_traceback_count())
			{
				set_error_text();
				m_Button_Traceback->set_sensitive();
			}

            services->get<PluginManager>("mpx-service-plugins")->signal_traceback().connect(
                sigc::mem_fun(
                    *this,
                    &PluginManagerGUI::check_traceback
            ));

            /*- Setup Window Geometry -----------------------------------------*/ 

            gtk_widget_realize(GTK_WIDGET(gobj()));
        
            resize(
               mcs->key_get<int>("mpx","window-plugins-w"),
               mcs->key_get<int>("mpx","window-plugins-h")
            );

            move(
                mcs->key_get<int>("mpx","window-plugins-x"),
                mcs->key_get<int>("mpx","window-plugins-y")
            );
		}

		PluginManagerGUI*
		PluginManagerGUI::create ()
		{
			const std::string path = DATA_DIR G_DIR_SEPARATOR_S "ui" G_DIR_SEPARATOR_S "pluginmanager.ui";
			PluginManagerGUI * p = new PluginManagerGUI(Gtk::Builder::create_from_file (path));
			return p;
		}

		bool
		PluginManagerGUI::on_delete_event (GdkEventAny* G_GNUC_UNUSED)
		{
            hide();
			return true;
		}

        void
        PluginManagerGUI::hide ()
        {
            Gtk::Window::get_position( Mcs::Key::adaptor<int>(mcs->key("mpx", "window-plugins-x")), Mcs::Key::adaptor<int>(mcs->key("mpx", "window-plugins-y")));
            Gtk::Window::get_size( Mcs::Key::adaptor<int>(mcs->key("mpx", "window-plugins-w")), Mcs::Key::adaptor<int>(mcs->key("mpx", "window-plugins-h")));
            Gtk::Widget::hide();
        }

        void
        PluginManagerGUI::present ()
        {
            resize(
               mcs->key_get<int>("mpx","window-plugins-w"),
               mcs->key_get<int>("mpx","window-plugins-h")
            );

            move(
                mcs->key_get<int>("mpx","window-plugins-x"),
                mcs->key_get<int>("mpx","window-plugins-y")
            );

            Gtk::Window::present();
        }

		void
		PluginManagerGUI::on_selection_changed()
		{
			Glib::RefPtr<TreeSelection> sel = m_PluginTreeView->get_selection();
			TreeModel::iterator iter = sel->get_selected();
			m_Overview->set_markup( (boost::format(_("%1%\n\n<b>Authors:</b> %2%\n<b>Copyright:</b> %3%\n<b>Website:</b> %4%"))
										% Glib::Markup::escape_text(m_PluginTreeView->get_desc(iter)).c_str()
										% Glib::Markup::escape_text(m_PluginTreeView->get_authors(iter)).c_str()
										% Glib::Markup::escape_text(m_PluginTreeView->get_copyright(iter)).c_str()
										% Glib::Markup::escape_text(m_PluginTreeView->get_website(iter)).c_str() ).str() );

			if(m_PluginTreeView->get_has_gui(iter))
			{
				m_Notebook->get_nth_page(1)->show();
                Gtk::Widget* child = m_Options->get_child();
				if(child)
				{
					m_Options->remove();
				}
				Gtk::Widget * widget = m_PluginTreeView->get_gui(iter);
				if(widget != 0)
				{
                    g_object_ref(G_OBJECT(widget->gobj())) ;
                    widget->unparent() ;
					m_Options->add(*widget);
                    g_object_unref(G_OBJECT(widget->gobj())) ;
					widget->show();
				}
                m_Notebook->set_current_page(1);
			}
			else
            {
				m_Notebook->get_nth_page(1)->hide();
                m_Notebook->set_current_page(0);
            }
		}

        void
        PluginManagerGUI::check_traceback()
        {
			if(services->get<PluginManager>("mpx-service-plugins")->get_traceback_count())
			{
				set_error_text();
				m_Button_Traceback->set_sensitive();
			}
			else
			{
				m_Error->set_markup(" ");
				m_Button_Traceback->set_sensitive(false);
			}
        }
        
		void
		PluginManagerGUI::show_dialog()
		{
			Gtk::MessageDialog dialog ((boost::format(_("Failed to %1%: %2%")) % services->get<PluginManager>("mpx-service-plugins")->get_last_traceback().get_method() % services->get<PluginManager>("mpx-service-plugins")->get_last_traceback().get_name()).str(), false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE);
			dialog.set_title (_("Plugin traceback - MPX"));
			dialog.set_secondary_text (services->get<PluginManager>("mpx-service-plugins")->pull_last_traceback().get_traceback());
			dialog.run();

			if(services->get<PluginManager>("mpx-service-plugins")->get_traceback_count())
            {
				set_error_text();
            }
			else
			{
				m_Error->set_markup(" ");
				m_Button_Traceback->set_sensitive(false);
			}
		}

		void
		PluginManagerGUI::set_error_text()
		{
			std::string text = (boost::format(_("<b>Failed to %1%: %2%</b>")) % services->get<PluginManager>("mpx-service-plugins")->get_last_traceback().get_method() % services->get<PluginManager>("mpx-service-plugins")->get_last_traceback().get_name()).str();

			unsigned int n = services->get<PluginManager>("mpx-service-plugins")->get_traceback_count();
			if(n > 1)
				text += (boost::format(_(" (%d more errors)")) % (n-1)).str();

			m_Error->set_markup(text);
		}

}
