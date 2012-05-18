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

#include "mpx/mpx-main.hh"
#include "mpx/mpx-plugin.hh"

#include "mpx/i-youki-play.hh"
#include "mpx/i-youki-controller.hh"

#include "cppmod-scrobbler.hh"

#include "log.h"

using namespace Glib;
using namespace Gtk;

namespace MPX
{
    const char* ui_path = DATA_DIR G_DIR_SEPARATOR_S "ui" G_DIR_SEPARATOR_S "cppmod-scrobbler.ui";

    //// CPPModLastFmScrobbler
    CPPModLastFmScrobbler*
        CPPModLastFmScrobbler::create(guint id)
    {
        return new CPPModLastFmScrobbler (Gtk::Builder::create_from_file (ui_path), id );
    }

    CPPModLastFmScrobbler::~CPPModLastFmScrobbler ()
    {
	m_LastFmScrobbler->finish() ;
        delete m_LastFmScrobbler ;
    }

    CPPModLastFmScrobbler::CPPModLastFmScrobbler(
          const Glib::RefPtr<Gtk::Builder>& builder
        , guint                             id
    )
        : WidgetLoader<Gtk::VBox>(builder, "cppmod-scrobbler")
        , PluginHolderBase()
    {
        m_Name = "Last.fm Scrobbler" ;
        m_Description = "This plugin allows you to scrobbler tracks to Last.fm" ;
        m_Authors = "Dirk Vanden Boer, M. Derezynski" ;
        m_Copyright = "(C) Dirk Vanden Boer, (C) 2009 MPX Project" ;
        m_IAge = 0 ;
        m_Website = "http://redmine.sivashs.org/wiki/mpx" ;
        m_Active = false ;
        m_HasGUI = true ;
        m_CanActivate = true ;
        m_Hidden = false ;
        m_Id = id ;

        builder->get_widget("lastfm-entry-username", m_Entry_Username ) ;
        builder->get_widget("lastfm-entry-password", m_Entry_Password ) ;

        mcs->domain_register( "lastfm" ) ;

        mcs->key_register( "lastfm", "username", std::string() ) ;
        mcs->key_register( "lastfm", "password", std::string() ) ;

        mcs_bind->bind_entry(
              *m_Entry_Username
            , "lastfm"
            , "username"
        ) ;

        mcs_bind->bind_entry(
              *m_Entry_Password
            , "lastfm"
            , "password"
        ) ;

        m_Entry_Username->signal_changed().connect(
                sigc::mem_fun(
                      *this
                    , &CPPModLastFmScrobbler::on_entry_changed
        )) ;

        m_Entry_Password->signal_changed().connect(
                sigc::mem_fun(
                      *this
                    , &CPPModLastFmScrobbler::on_entry_changed
        )) ;

        m_Log = new TextViewLog( builder ) ;

        m_LastFmScrobbler = new LastFmScrobbler( true, *m_Log ) ;
	m_LastFmScrobbler->run() ;

        boost::shared_ptr<IYoukiController> p1 = services->get<IYoukiController>("mpx-service-controller") ;

        GObject * obj = G_OBJECT(p1->gobj()) ;

        g_signal_connect(
              obj
            , "track-out"
            , GCallback(on_controller_track_out)
            , this 
        ) ;

        g_signal_connect(
              obj
            , "track-new"
            , GCallback(on_controller_track_new)
            , this 
        ) ;

        g_signal_connect(
              obj
            , "track-cancelled"
            , GCallback(on_controller_track_cancelled)
            , this 
        ) ;

        boost::shared_ptr<IPlay> p2 = services->get<IPlay>("mpx-service-play") ;

        p2->property_status().signal_changed().connect(
	    sigc::mem_fun(
		  *this
                 , &CPPModLastFmScrobbler::on_play_status_changed
        )) ;

	p2->signal_seek().connect(
	    sigc::mem_fun(
		  *this
	    , &CPPModLastFmScrobbler::on_play_seek
    )) ;

    show_all() ;
}

Gtk::Widget*
CPPModLastFmScrobbler::get_gui()
{
    return this ;
}

bool
CPPModLastFmScrobbler::activate()
{
    logger::info( *m_Log, "Scrobbling turned ON" ) ;

    m_Active = true ;

    m_LastFmScrobbler->set_enabled( true ) ;

    m_LastFmScrobbler->set_credentials(
	  mcs->key_get<std::string>( "lastfm", "username" )
	, mcs->key_get<std::string>( "lastfm", "password" )
    ) ;

    return true ;
}

bool
CPPModLastFmScrobbler::deactivate()
{
    logger::info( *m_Log, "Scrobbling turned OFF" ) ;

    m_LastFmScrobbler->set_enabled( false ) ;
    m_Active = false ;

    return true ;
}

void
CPPModLastFmScrobbler::on_controller_track_out(
      GObject *     controller
    , gpointer      data
)
{
    CPPModLastFmScrobbler & obj = *reinterpret_cast<CPPModLastFmScrobbler*>(data) ;

    if( !obj.m_Active )
	return ;

    try{
	logger::info( *(obj.m_Log), "Calling finishedPlaying()" ) ;
	obj.m_LastFmScrobbler->finishedPlaying() ;
    } catch( std::runtime_error )
    {
	logger::info( *(obj.m_Log), "Exception in: finishedPlaying()" ) ;
    }
}

void
CPPModLastFmScrobbler::on_controller_track_new(
      GObject *     controller
    , gpointer      data
)
{
    CPPModLastFmScrobbler & obj = *reinterpret_cast<CPPModLastFmScrobbler*>(data) ;

    if( !obj.m_Active )
    {
	logger::info( *(obj.m_Log), "Plugin not active" ) ;
	return ;
    }

    boost::shared_ptr<IYoukiController> p = services->get<IYoukiController>("mpx-service-controller") ;

    try{
	MPX::Track & t = p->get_metadata() ; 

	const std::string&  artist  =   boost::get<std::string>(t[ATTRIBUTE_ARTIST].get()) ;
	const std::string&  title   =   boost::get<std::string>(t[ATTRIBUTE_TITLE].get()) ;
	const std::string&  album   =   boost::get<std::string>(t[ATTRIBUTE_ALBUM].get()) ;
	const guint&        time_    =   boost::get<guint>(t[ATTRIBUTE_TIME].get()) ;
	const guint&        tracknr  =   boost::get<guint>(t[ATTRIBUTE_TRACK].get()) ;

	SubmissionInfo info ( artist, title, time(NULL) ) ;
	info.setTrackLength( int(time_) ) ;
	info.setTrackNr( int(tracknr) ) ;
	info.setAlbum( album ) ; 

	if( t.has(ATTRIBUTE_MB_TRACK_ID) )
	{
	    info.setMusicBrainzId( boost::get<std::string>(t[ATTRIBUTE_MB_TRACK_ID].get())) ;
	}

	logger::info( *(obj.m_Log), "Started playing; setting info" ) ;
	obj.m_LastFmScrobbler->startedPlaying( info ) ;

    } catch( std::runtime_error )
    {
	logger::info( *(obj.m_Log), "Exception in: setting info" ) ;
    }
}

void
CPPModLastFmScrobbler::on_controller_track_cancelled(
      GObject *     controller
    , gpointer      data
)
{
    CPPModLastFmScrobbler & obj = *reinterpret_cast<CPPModLastFmScrobbler*>(data) ;

    if( !obj.m_Active )
	return ;

    try{
	logger::info( *(obj.m_Log), "Calling finishedPlaying()" ) ;
	obj.m_LastFmScrobbler->finishedPlaying() ;
    } catch( std::runtime_error )
    {
	logger::info( *(obj.m_Log), "Exception in: finishedPlaying()" ) ;
    }
}

void
CPPModLastFmScrobbler::on_play_seek(
	int diff
    )
    {
	m_LastFmScrobbler->play_seek( diff ) ;
    }

void
CPPModLastFmScrobbler::on_play_status_changed(
    )
    {
        boost::shared_ptr<IPlay> p = services->get<IPlay>("mpx-service-play") ;

        if( p->property_status().get_value() == PLAYSTATUS_PAUSED )
        {
            m_LastFmScrobbler->pausePlaying( true ) ;
        }
        else
        if( p->property_status().get_value() == PLAYSTATUS_PLAYING )
        {
            m_LastFmScrobbler->pausePlaying( false ) ;
        }
	else
        if( p->property_status().get_value() == PLAYSTATUS_STOPPED )
        {
            m_LastFmScrobbler->pausePlaying( false ) ;
	    on_controller_track_out( nullptr, this ) ;
        }
    }

    void
    CPPModLastFmScrobbler::on_entry_changed(
    )
    {
        if( m_Active )
        {
            boost::shared_ptr<PluginManager> p = services->get<PluginManager>("mpx-service-plugins") ;
            p->deactivate( m_Id ) ;
        }
    }

}  // namespace MPX
