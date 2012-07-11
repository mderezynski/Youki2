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

#ifndef MPX_COVERS_STORES_HH
#define MPX_COVERS_STORES_HH

#include <string>
#include <unordered_map>
#include <glibmm.h>

#include "mpx/mpx-minisoup.hh"

namespace MPX
{
    enum CoverDownloadState
    {
          FETCH_STATE_NOT_FETCHED
        , FETCH_STATE_COVER_SAVED
    } ;

    struct RequestQualifier
    {
        std::string mbid ;
        std::string asin ; 
        std::string uri ;
        std::string artist ;
        std::string album ;
        guint	    id ;
    } ;

    class CoverStore
    {
	protected:

	    CoverDownloadState m_state ;

	public:

	    typedef sigc::signal<void, const std::string&, Glib::RefPtr<Gdk::Pixbuf> > SignalSaveCover ; 

	    SignalSaveCover m_SIGNAL ;

	    SignalSaveCover&
	    signal_save_cover()
	    {
		return m_SIGNAL ;
            }

	    CoverStore()
	    : m_state( FETCH_STATE_NOT_FETCHED )
	    {}

	    virtual void
	    load_artwork(const RequestQualifier&) = 0;

	    CoverDownloadState
	    get_state()
	    {
		return m_state ;
	    }
    };

    class RemoteStore : public CoverStore
    {
	public:

	    RemoteStore() {}

	    virtual void
	    fetch_image(const std::string&, const RequestQualifier&);

	    virtual void
	    load_artwork(const RequestQualifier&);

	    virtual std::string
	    get_url(const RequestQualifier&) = 0; 

	    virtual bool
	    can_load_artwork(const RequestQualifier&);

	    void
	    save_image(
		  char const*
		, guint
		, const RequestQualifier& 
	    );

	    virtual void
	    request_failed() ;

	protected:

	    void
	    data_cb(
		  const char*	data
		, guint		size
		, guint		httpcode
		, const RequestQualifier& rql
	    ) ; 

	    typedef std::unordered_map<guint, Soup::RequestRefP> RequestMap_t ;
    
	    RequestMap_t    m_requests ;
    };

   class LastFMCovers : public RemoteStore
    {
	public:

	    virtual void
	    load_artwork(const RequestQualifier&) ;

	protected:

	    void
	    data1_cb(
		  const char*	data
		, guint		size
		, guint		httpcode
		, const RequestQualifier& rql
	    ) ; 

	    virtual std::string
	    get_url(const RequestQualifier&);
    };

    class MusicBrainzCovers : public RemoteStore
    {
	public:

	    void
	    load_artwork(const RequestQualifier&);

	protected:

	    void
	    data1_cb(
		  const char*	data
		, guint		size
		, guint		httpcode
		, const RequestQualifier& rql
	    ) ; 


	    virtual std::string
	    get_url(const RequestQualifier&); 
    };

    class LocalCovers : public CoverStore
    {
	public:

	    void
            load_artwork(const RequestQualifier&);
    };

    class InlineCovers : public CoverStore
    {
	public:

	    void
	    load_artwork(const RequestQualifier&);
    };
}

#endif
