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

#ifndef MPX_CPPMOD_LASTFM_SCROBBLER_HH
#define MPX_CPPMOD_LASTFM_SCROBBLER_HH

#include "config.h"

#include <string>
#include <set>
#include <vector>
#include <glibmm.h>
#include <glibmm/i18n.h>
#include <gtkmm.h>

#include "mpx/widgets/widgetloader.hh"
#include "mpx/plugin-types.hh"

#include "lastfmscrobbler.h"
#include "i-log.hh"

#include <queue>

namespace MPX
{
    class TextViewLog
    : public Gnome::Glade::WidgetLoader<Gtk::TextView>
    , public ILog
    {
        private:

            Glib::Dispatcher  m_disp ;
            Glib::Mutex       m_lock ;
            
            std::queue<std::string> m_msgqueue ;
        
        public:

            TextViewLog(
                const Glib::RefPtr<Gnome::Glade::Xml>& xml
            )
            : Gnome::Glade::WidgetLoader<Gtk::TextView>( xml, "lastfm-log" )
            {
                 m_disp.connect( 
                      sigc::mem_fun(
                            *this
                          , &TextViewLog::on_dispatched
                 )) ;
            }

            void 
            on_dispatched()
            {
                m_lock.lock() ;
                Glib::RefPtr<Gtk::TextBuffer> buf = get_buffer() ;
                while( !m_msgqueue.empty() )
                {
                    buf->insert( buf->end(), m_msgqueue.front() ) ;
                    m_msgqueue.pop() ;
                }
                m_lock.unlock() ;
            }

            TextViewLog& 
            operator<< (const std::string& msg)
            {
                m_lock.lock() ;
                m_msgqueue.push( msg );
                m_lock.unlock() ;
                m_disp.emit() ;
                return *this ;
            }

            TextViewLog& 
            operator<< (const char* msg)
            {
                m_lock.lock() ;
                m_msgqueue.push( msg );
                m_lock.unlock() ;
                m_disp.emit() ;
                return *this ;
            }
    } ;

    class CPPModLastFmScrobbler
    : public Gnome::Glade::WidgetLoader<Gtk::VBox>
    , public PluginHolderBase 
    {
      public:

          CPPModLastFmScrobbler(
                const Glib::RefPtr<Gnome::Glade::Xml>&
              , guint
          ) ;

          static CPPModLastFmScrobbler*
          create(
                guint
          ) ;

          virtual
          ~CPPModLastFmScrobbler(
          ) ;

          virtual bool
          activate(
          ) ;

          virtual bool
          deactivate(
          ) ;

          virtual Gtk::Widget*
          get_gui(
          ) ;

      protected:

          static void
          on_controller_track_out(
                GObject*
              , gpointer
          ) ;

          static void
          on_controller_track_new(
                GObject*
              , gpointer
          ) ;

          static void
          on_controller_track_cancelled(
                GObject*
              , gpointer
          ) ;

          void
          on_play_status_changed(
          ) ;

          void
          on_entry_changed(
          ) ;

          LastFmScrobbler     * m_LastFmScrobbler ;

          Gtk::Entry          * m_Entry_Username ;
          Gtk::Entry          * m_Entry_Password ;

          TextViewLog         * m_Log ;

    } ; // class CPPModLastFmScrobbler
} // namespace MPX

extern "C" MPX::PluginHolderBase*
get_instance(guint id)
{
    return MPX::CPPModLastFmScrobbler::create( id ) ;
}

extern "C" bool
del_instance(MPX::PluginHolderBase* b)
{
    delete dynamic_cast<MPX::CPPModLastFmScrobbler*>(b) ;
    return true ;
}

#endif // MPX_CPPMOD_LASTFM_SCROBBLER?g_HH
