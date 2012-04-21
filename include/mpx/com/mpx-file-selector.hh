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

#ifndef MPX_FILE_SELECTOR_HH 
#define MPX_FILE_SELECTOR_HH

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif // HAVE_CONFIG_H

#include <boost/optional.hpp>
#include <giomm.h>
#include <gtkmm.h>
#include <libglademm/xml.h>
#include <sigx/sigx.h>
#include "mpx/widgets/widgetloader.hh"

namespace MPX
{
    typedef std::vector<std::string>        PathFragments;
    typedef PathFragments::const_iterator   PathPosition;
    typedef PathFragments::size_type        PathSize; 

    class PathModel
    {
        public:

            typedef sigc::signal<void, PathPosition const&>     SignalMoved;
            typedef sigc::signal<void, PathSize>                SignalCut;
            typedef sigc::signal<void>                          SignalSet;
            typedef sigc::signal<void>                          SignalAppend;

#include "mpx/mpx-exception.hh"

            EXCEPTION(NoSuchPathError)

        protected:

            struct Signals_t
            {
                SignalMoved         Moved;
                SignalCut           Cut;
                SignalSet           Set;
                SignalAppend        Append; 
            };

            Signals_t           Signals;
            PathFragments       m_Path;
            PathPositionVal     m_Position;

            virtual void
            fragmentize_path(
                const std::string&
            );

        public:
       
        // SIGNALS

            SignalMoved&
            signal_moved()
            {
                    return Signals.Moved;
            }

            SignalCut&
            signal_cut()
            {
                    return Signals.Cut;
            }

            SignalSet&
            signal_set()
            {
                    return Signals.Set;
            }

            SignalAppend&
            signal_append()
            {
                    return Signals.Append;
            }

        // API
                        
            PathModel ();
            virtual ~PathModel ();

            virtual PathSize 
            get_pos(
            );

            virtual const PathFragments&
            get_path(
            );

            virtual void
            set_path(
                const std::string&
            );

            virtual void
            append_path(
                const std::string&
            );

            virtual void
            cut_path(
            );

            virtual PathPosition 
            up(
            );

            virtual PathPosition 
            down(
            );
    };
}
#endif
