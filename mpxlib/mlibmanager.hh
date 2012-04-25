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
//  The MPX project hereby grants permission for non-GPL compatible GStreamer
//  plugins to be used and distributed together with GStreamer and MPX. This
//  permission is above and beyond the permissions granted by the GPL license
//  MPX is covered by.

#ifndef MPX_MLIBMANAGER_HH
#define MPX_MLIBMANAGER_HH

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif // HAVE_CONFIG_H

// FIXME: Must be here for some damn reason
#include <giomm.h>

#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/filechooserwidget.h>
#include <gtkmm/filefilter.h>
#include <gtkmm/window.h>
#include <sigx/sigx.h>

#include "library-mlibman.hh"

#ifdef HAVE_HAL
#include "libhal++/hal++.hh"
#include "mpx/i-youki-hal.hh"
#endif // HAVE_HAL

#include "mpx/widgets/widgetloader.hh"
#include "mpx/mpx-main.hh"
#include "mpx/mpx-services.hh"

#include "mpx-mlibman-dbus.hh"

namespace MPX
{
    enum PathTestResult
    {
          IS_PRESENT
        , RELOCATED
        , DELETED
        , IGNORED
    } ;

    struct FileStatsColumns : public Gtk::TreeModel::ColumnRecord
    {
        Gtk::TreeModelColumn<std::string>       Type;
        Gtk::TreeModelColumn<guint>            Count;
        Gtk::TreeModelColumn<std::string>       HighBitrate;
        Gtk::TreeModelColumn<std::string>       LowBitrate;

        FileStatsColumns ()
        {
            add(Type);
            add(Count);
            add(HighBitrate);
            add(LowBitrate);
        }
    };

    class MLibManager
    : public info::backtrace::Youki::MLibMan_adaptor
    , public DBus::IntrospectableAdaptor
    , public DBus::ObjectAdaptor
    , public WidgetLoader<Gtk::Window>
    , public sigx::glib_auto_dispatchable
    , public Service::Base
    {
            FileStatsColumns                    m_FileStats_Columns ;
            Glib::RefPtr<Gtk::ListStore>        m_FileStats_Store ;
            Gtk::TreeView                     * m_FileStats_View ;

            void
            update_filestats ();

        public:

            static MLibManager* create(
                DBus::Connection&
            ) ;

            virtual ~MLibManager(
            ) ;

            void
            push_message(
                const std::string&
            ) ;

            virtual bool
            on_delete_event(
                GdkEventAny*
            ) ;

            virtual void
            present () ;

            virtual void
            hide () ;

        private:

            MLibManager(
                  const Glib::RefPtr<Gtk::Builder>&
                , DBus::Connection&
            ) ;

            void
            scan_start(
            ) ;

            void
            scan_end(
            ) ;

            void
            scan_summary(
                const ScanSummary&
            ) ;

            void
            new_album(
                  guint
                , const std::string&
                , const std::string&
                , const std::string&
                , const std::string&
                , const std::string&
            ) ;

            void
            new_artist(
                  const guint&
            ) ;

            void
            new_track(
                  const guint&
            ) ;

            void
            entity_updated(
                  const guint&
                , int
            ) ;

            void
            entity_deleted(
                  const guint&
                , int
            ) ;

            void
            rescan_volumes() ;

#ifdef HAVE_HAL
            void
            clear_volumes() ;

            void
            populate_volumes() ;

            void
            build_fstree(
                const std::string&
            ) ;

            int
            fstree_sort(
                const Gtk::TreeIter&,
                const Gtk::TreeIter&
            ) ;

            void
            append_path(
                const std::string&,
                Gtk::TreeIter&
            ) ;

            void
            prescan_path(
                const std::string&,
                Gtk::TreeIter& iter
            ) ;

            bool
            has_active_parent(
                Gtk::TreeIter&
            ) ;

            PathTestResult
            path_test(
                  const std::string&
                , const std::string&
                , const std::string&
            ) ;

            void
            on_fstree_row_expanded(
                const Gtk::TreeIter&,
                const Gtk::TreePath&
            ) ;

            void
            on_path_toggled(
                const Glib::ustring&
            ) ;

	    void
	    handle_pause() ;

            struct VolumeColumnsT : public Gtk::TreeModel::ColumnRecord
            {
                Gtk::TreeModelColumn<std::string>                   Name ;
                Gtk::TreeModelColumn<std::string>                   Mountpoint ;
                Gtk::TreeModelColumn<Hal::RefPtr<Hal::Volume> >     Volume ;
                Gtk::TreeModelColumn<Glib::RefPtr<Gio::Volume> >    GioVolume ;

                VolumeColumnsT ()
                {
                    add (Name) ;
                    add (Mountpoint) ;
                    add (Volume) ;
                    add (GioVolume) ;
                } ;
            } ;
	    VolumeColumnsT VolumeColumns ;
            Glib::RefPtr<Gtk::ListStore> VolumeStore ;

            Gtk::TreeView * m_FSTree ;

            struct FSTreeColumnsT : public Gtk::TreeModel::ColumnRecord
            {
                Gtk::TreeModelColumn<std::string>     SegName ;
                Gtk::TreeModelColumn<std::string>     FullPath ;
                Gtk::TreeModelColumn<bool>            WasExpanded ;

                FSTreeColumnsT ()
                {
                    add (SegName) ;
                    add (FullPath) ;
                    add (WasExpanded) ;
                } ;
            } ;
            FSTreeColumnsT FSTreeColumns ;
            Glib::RefPtr<Gtk::TreeStore> FSTreeStore ;

            void
            cell_data_func_active(
                  Gtk::CellRenderer*
                , const Gtk::TreeIter&
            ) ;

            void
            cell_data_func_text(
                  Gtk::CellRenderer*
                , const Gtk::TreeIter&
            ) ;

            bool
            slot_select(
                  const Glib::RefPtr<Gtk::TreeModel>&
                , const Gtk::TreePath&
                , bool
            ) ;

            void
            recreate_path_frags () ;

            typedef std::set<std::string>       StrSetT ;
            typedef std::vector<std::string>    PathFrags ;
            typedef std::vector<PathFrags>      PathFragsV ;

            StrSetT     m_ManagedPaths ;
            PathFragsV  m_ManagedPathFrags ;
#endif // HAVE_HAL

            Gtk::Button     * m_Close ;

#ifdef HAVE_HAL
            Gtk::Button     * m_Rescan ;
            Gtk::Button     * m_DeepRescan ;
            Gtk::Button     * m_Vacuum ;
#endif // HAVE_HAL

            Gtk::Notebook   * m_notebook;
            Gtk::Statusbar  * m_Statusbar ;
            Gtk::Widget     * m_VboxInner ;
	        Gtk::Label	    * m_RescanTimeoutLabel ;
            Gtk::Button     * m_ButtonScan ;
	        Gtk::ToggleButton * m_ButtonPauseRescan ;

            Glib::Timer                     m_RescanTimer ;

            Glib::RefPtr<Gtk::TextBuffer>   m_TextBufferDetails ;
            Glib::RefPtr<Gtk::UIManager>    m_UIManager ;
            Glib::RefPtr<Gtk::ActionGroup>  m_Actions ;

            Gtk::HBox       * m_InnerdialogHBox ;
            Gtk::Label      * m_InnerdialogLabel ;
            Gtk::Button     * m_InnerdialogYes ;
            Gtk::Button     * m_InnerdialogCancel ;

            Glib::RefPtr<Glib::MainLoop>    m_InnerdialogMainloop ;
            int                             m_InnerdialogResponse ;

            boost::shared_ptr<IHAL>         m_HAL ;

            void
            innerdialog_response(int) ;

            void
            on_mlib_remove_dupes() ;

            void
            on_mlib_rescan_library() ;

            void
            on_mlib_vacuum_library() ;

#ifdef HAVE_HAL
            void
            on_volume_rescan_volume() ;

            void
            on_volume_vacuum_volume() ;
#endif // HAVE_HAL

            void
            on_update_statistics();

            // MCS Callbacks

#ifdef HAVE_HAL
            void
            on_library_use_hal_changed (MCS_CB_DEFAULT_SIGNATURE) ;
#endif // HAVE_HAL

            void
            on_library_rescan_in_intervals_changed (MCS_CB_DEFAULT_SIGNATURE) ;

            bool
            on_rescan_timeout() ;

            // GIO import stuff

            void
            on_import_folder();

            void
            on_import_share();

            void
            mount_ready_callback (Glib::RefPtr<Gio::AsyncResult>&);

            void
            unmount_ready_callback (Glib::RefPtr<Gio::AsyncResult>&);

            void
            ask_password_cb (const Glib::ustring& message,
                             const Glib::ustring& default_user,
                             const Glib::ustring& default_domain,
                             Gio::AskPasswordFlags flags);

            Glib::RefPtr<Gio::File>             m_MountFile;
            Glib::RefPtr<Gio::MountOperation>   m_MountOperation;
            Glib::ustring                       m_Share
                                              , m_ShareName;

        //// DBUS

            virtual void
            ShowWindow () ;

            virtual void
            Exit () ;

            virtual void
            Start() ;
    } ;
}
#endif
