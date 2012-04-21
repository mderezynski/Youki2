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

#ifndef MPX_ARTIST_IMAGES_HH
#define MPX_ARTIST_IMAGES_HH

#ifndef HAVE_CONFIG_H
#include <config.h>
#endif

#include "mpx/mpx-main.hh"
#include "mpx/mpx-minisoup.hh"
#include "mpx/mpx-network.hh"
#include "mpx/mpx-services.hh"
#include "mpx/mpx-sql.hh"

#include <string>
#include <map>
#include <set>

#include <boost/shared_ptr.hpp>
#include <cairomm/cairomm.h>
#include <glibmm/ustring.h>
#include <gdkmm/pixbuf.h> // bleh!
#include <sigx/sigx.h>

namespace MPX
{
    typedef std::map <std::string, Glib::RefPtr<Gdk::Pixbuf> >                      PixbufCache_t ;

    typedef sigc::signal<void, const std::string&, Glib::RefPtr<Gdk::Pixbuf> >      SignalGotArtistImage_t ; 
    typedef sigx::signal_f<SignalGotArtistImage_t>                                  SignalGotArtistImage_x ;

    class ArtistImages 
    : public Service::Base
    , public sigx::glib_threadable
    {
        public:

            SignalGotArtistImage_x    signal_got_artist_image ;

            ArtistImages ();
            virtual ~ArtistImages() {}

            sigx::request_f<>
            recache_images ;

            sigx::request_f<const std::string&>
            get_image ;

        protected:

            struct ThreadData;
            Glib::Private<ThreadData> m_ThreadData;

            // auto_threadable

            virtual void
            on_startup () ;

            virtual void
            on_cleanup () ;

            // requests

            void
            on_recache_images () ;

            void
            on_get_image(
                  const std::string&
            ) ;

            // internal api

            Glib::RefPtr<Gdk::Pixbuf>
            get_image_by_mbid(
                const std::string&
            );

            PixbufCache_t     m_pixbuf_cache;
            std::string       m_base_path;
            SQL::SQLDB      * m_SQL;
    };
}

#endif
