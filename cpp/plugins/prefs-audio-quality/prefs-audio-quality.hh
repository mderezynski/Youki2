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

#ifndef MPX_PREFS_AUDIO_QUALITY_HH
#define MPX_PREFS_AUDIO_QUALITY_HH

#include "config.h"

#include <string>
#include <set>
#include <vector>
#include <glibmm.h>
#include <gtkmm.h>

#include "mpx/mpx-services.hh"
#include "mpx/widgets/widgetloader.hh"
#include "mpx/plugin-types.hh"

namespace MPX
{
  class FileFormatPrioritiesView;

  /** PrefsAudioQuality dialog
   *
   * MPX::PrefsAudioQuality is a complex dialog for adjusting run time parameters
   * of MPX trough the GUI instead of having to manipulate the configuration
   * file.
   */
  class PrefsAudioQuality
  : public Gnome::Glade::WidgetLoader<Gtk::VBox>
  , public PluginHolderBase 
  {
    public:

        PrefsAudioQuality(
              const Glib::RefPtr<Gnome::Glade::Xml>&
            , gint64
        ) ;

        static PrefsAudioQuality*
        create(
              gint64
        ) ;

        virtual
        ~PrefsAudioQuality(
        ) ;

        virtual bool
        activate()
        {
            return true ;
        }

        virtual bool
        deactivate()
        {
            return true ;
        }

        virtual Gtk::Widget*
        get_gui()
        {
            return 0 ;
        }

    private:

        // File Formats
        FileFormatPrioritiesView    * m_Fmts_PrioritiesView ;
        Gtk::CheckButton            * m_Fmts_PrioritizeByFileType ;
        Gtk::CheckButton            * m_Fmts_PrioritizeByBitrate ;

  } ; // class PrefsAudioQuality
} // namespace MPX

extern "C" MPX::PluginHolderBase*
get_instance(gint64 id)
{
    return MPX::PrefsAudioQuality::create( id ) ;
}

extern "C" bool
del_instance(MPX::PluginHolderBase* b)
{
    delete dynamic_cast<MPX::PrefsAudioQuality*>(b) ;
    return true ;
}

#endif // MPX_PREFS_AUDIO_QUALITY_HH
