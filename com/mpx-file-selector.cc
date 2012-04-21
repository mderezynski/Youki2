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
//
#include "mpx/com/mpx-file-selector.hh"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif // HAVE_CONFIG_H

#include <boost/algorithm/string.hpp>
#include <giomm.h>
#include <gtkmm.h>
#include <libglademm/xml.h>
#include <sigx/sigx.h>

using namespace boost::algorithm;

namespace MPX
{
                PathModel::PathModel ()
                {
                }

                PathModel::~PathModel ()
                {
                }

        PathSize 
                PathModel::get_pos(
                )
                {
                    return m_Position;
                }

        const PathFragments&
                PathModel::get_path(
                )
                {
                    return m_Path;
                }

        void
                PathModel::fragmentize_path(
                    const std::string& path
                )
                {
                    m_Path = PathFragments();
                    split( m_Path, path, is_any_of(std::string(1,G_DIR_SEPARATOR_S)));
                }

        void
                PathModel::set_path(
                    const std::string& path
                )
                {
                    fragmentize_path( path );
                    m_Position = std::distance( m_Path.begin(), m_Path.begin() );

                    Signals.Set.emit();
                }

        void
                PathModel::append_path(
                    const std::string& path
                )
                {
                    m_Path.push_back( path );

                    Signals.Append.emit();
                }

        void
                PathModel::cut_path(
                )
                {
                    PathPosition p = m_Path.begin():
                    std::advance( p, m_Position );
                    m_Path.erase( m_Path.begin(), p ); 
                    
                    Signals.Cut.emit( m_Path.size() );
                }

        PathPosition 
                PathModel::up(
                             )
                {
                }

        PathPosition 
                PathModel::down(
                               )
                {
                }
}
