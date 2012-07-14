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

#ifndef MPX_COVERS_RM_STORES_HH
#define MPX_COVERS_RM_STORES_HH

#include <string>
#include <unordered_map>
#include <glibmm.h>

#include "mpx/mpx-minisoup.hh"

namespace MPX
{
namespace RM
{
    struct RequestQualifier
    {
        std::string mbid ;
        std::string asin ; 
        std::string uri ;
        std::string artist ;
        std::string album ;
        guint	    id ;
    } ;

    inline bool operator<(const RequestQualifier& a, const RequestQualifier& b)
    {
	return a.id < b.id ;
    }

    class CoverStore
    {
	public:

	    CoverStore()
	    {}

	    virtual Glib::RefPtr<Gdk::Pixbuf> 
	    load_artwork(const RequestQualifier&) = 0;
    };

    class RemoteStore : public CoverStore
    {
	public:

	    RemoteStore() {}

	    virtual Glib::RefPtr<Gdk::Pixbuf> 
	    fetch_image(const std::string&, const RequestQualifier&);

	    virtual Glib::RefPtr<Gdk::Pixbuf> 
	    load_artwork(const RequestQualifier&);

	    virtual bool
	    can_load_artwork(const RequestQualifier&);

	    virtual Glib::RefPtr<Gdk::Pixbuf> 
	    save_image(
		  char const*
		, guint
		, const RequestQualifier& 
	    );

	protected:

	    virtual std::string
	    get_url(const RequestQualifier&) = 0; 
    };

    class LastFMCovers : public RemoteStore
    {
	public:

	    virtual Glib::RefPtr<Gdk::Pixbuf>
	    load_artwork(const RequestQualifier&) ;

	protected:

	    virtual std::string
	    get_url(const RequestQualifier&);
    };

    class MusicBrainzCovers : public RemoteStore
    {
	public:

	    virtual Glib::RefPtr<Gdk::Pixbuf>
	    load_artwork(const RequestQualifier&);

	protected:

	    virtual std::string
	    get_url(const RequestQualifier&); 
    };

    class InlineCovers : public CoverStore
    {
	public:

	    virtual Glib::RefPtr<Gdk::Pixbuf>
	    load_artwork(const RequestQualifier&);
    };
}}

#endif
