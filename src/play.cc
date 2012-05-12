//  MPX - The Dumb Music Player
//  Copyright (C) 2010 MPX development team.
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License Version 2
//  as published by the Free Software Foundation.
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

#include "config.h"

#include <iostream>
#include <cstring>
#include <glibmm.h>
#include <glib/gi18n.h>

#include <gst/gst.h>
#include <gst/gstelement.h>

#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/optional.hpp>

#include "mpx/mpx-main.hh"
#include "mpx/mpx-uri.hh"

#include "play.hh"

#define MPX_GST_BUFFER_TIME   ((guint) 50000)
#define MPX_GST_PLAY_TIMEOUT  ((guint) 3000000000)

using namespace Glib ;

namespace
{
        static boost::format band_f ("band%d") ;

        char const* NAME_CONVERT    = N_("Raw Audio Format Converter") ;
        char const* NAME_VOLUME     = N_("Volume Adjustment") ;
        char const* NAME_RESAMPLE   = N_("Resampler") ;

        char const* m_pipeline_names[] =
        {
                "(none)",
                "http",
                "httpmad",
                "mmsx",
                "file",
                "cdda"
        } ;


        gboolean
                drop_data(
                      GstPad*           G_GNUC_UNUSED 
                    , GstMiniObject*    G_GNUC_UNUSED 
                    , gpointer          G_GNUC_UNUSED 
                )
                {
                        return FALSE ;
                }

        char const*
                nullify_string(
                      const std::string& in
                )
                {
                        return( in.size() ? in.c_str() : NULL ) ;
                }

        bool
        test_element(
              const std::string& name
        )
        {
            GstElementFactory* factory = gst_element_factory_find (name.c_str ());

            bool exists = ( factory != NULL ) ;

            if( factory )
            {
                gst_object_unref( factory ) ;
            }

            return exists;
        }
}

namespace MPX
{
        Play::Play()
        : ObjectBase              ("MPXPlaybackEngine")
        , Service::Base           ("mpx-service-play")
        , m_playback_bin          (0)
        , property_stream_        (*this, "stream", "")
        , property_stream_type_   (*this, "stream-type", "")
        , property_volume_        (*this, "volume", 50)
        , property_status_        (*this, "playstatus", PLAYSTATUS_STOPPED)
        , property_sane_          (*this, "sane", false)
        , property_position_      (*this, "position", 0)
        , property_duration_      (*this, "duration", 0)
        {
                m_message_queue = g_async_queue_new() ;

                property_volume().signal_changed().connect(
                        sigc::mem_fun(
                              *this
                            , &Play::on_volume_changed
                )) ;

                property_stream().signal_changed().connect(
                        sigc::mem_fun(
                              *this
                            , &Play::on_stream_changed
                )) ;

                m_bin[BIN_FILE]     = 0 ;
                m_bin[BIN_HTTP]     = 0 ;
                m_bin[BIN_MMSX]     = 0 ;
                m_bin[BIN_CDDA]     = 0 ;
                m_bin[BIN_OUTPUT]   = 0 ;

                m_pipeline = 0 ;

                reset () ;
        }

        //dtor
        Play::~Play ()
        {
                g_async_queue_unref (m_message_queue) ;
                stop_stream () ;
                destroy_bins () ;
        }

        inline GstElement*
                Play::control_pipe () const
                {
                    return m_pipeline ; 
                }

        void
                Play::for_each_tag(
                      GstTagList const* list
                    , gchar const*      tag
                    , gpointer          data
                )
                {
                        Play & play = *(static_cast<Play*>( data )) ;

                        int count = gst_tag_list_get_tag_size( list, tag )  ;

                        for( int i = 0; i < count; ++i )
                        {
                                //// TITLE
                                if( !std::strcmp( tag, GST_TAG_TITLE  ))
                                {
                                        if( play.m_pipeline_id == PIPELINE_HTTP ||
                                            play.m_pipeline_id == PIPELINE_MMSX
                                        )
                                        {
                                            Glib::ScopedPtr<char> w ;
                                            if( gst_tag_list_get_string_index (list, tag, i, w.addr( ))) ;
                                            {
                                                std::string title (w.get()); 

                                                if( !play.m_metadata.m_title || (play.m_metadata.m_title && (play.m_metadata.m_title.get( ) != title)))
                                                {
                                                    play.m_metadata.m_title = title ;
                                                    play.signal_metadata_.emit(FIELD_TITLE) ;
                                                }
                                            }
                                        }
                                }
                                else
                                //// ALBUM
                                if( !std::strcmp( tag, GST_TAG_ALBUM  ))
                                {
                                        if( play.m_pipeline_id == PIPELINE_HTTP ||
                                            play.m_pipeline_id == PIPELINE_MMSX
                                        )
                                        {
                                            Glib::ScopedPtr<char> w ;

                                            if( gst_tag_list_get_string_index (list, tag, i, w.addr( ))) ;
                                            {
                                                std::string album (w.get()); 

                                                if( !play.m_metadata.m_album || (play.m_metadata.m_album && (play.m_metadata.m_album.get( ) != album)))
                                                {
                                                    play.m_metadata.m_album = album ;
                                                    play.signal_metadata_.emit(FIELD_ALBUM) ;
                                                }
                                            }
                                        }
                                }
                                else
                                //// IMAGE
                                if( !std::strcmp( tag, GST_TAG_IMAGE  )) 
                                {
                                        GstBuffer* image = gst_value_get_buffer (gst_tag_list_get_value_index (list, tag, i)) ;
                                        if( image )
                                        {
                                            RefPtr<Gdk::PixbufLoader> loader (Gdk::PixbufLoader::create ()) ;
                                            try{
                                                loader->write (GST_BUFFER_DATA (image), GST_BUFFER_SIZE (image)) ;
                                                loader->close () ;
                                                RefPtr<Gdk::Pixbuf> img (loader->get_pixbuf()) ;
                                                if( img )
                                                {
                                                    play.m_metadata.m_image = img->copy() ;
                                                    play.signal_metadata_.emit(FIELD_IMAGE) ;
                                                }
                                            }
                                            catch (...) {} // pixbufloader is a whacky thing
                                        }
                                }
                                else
                                //// BITRATE
                                if( !std::strcmp( tag, GST_TAG_BITRATE  )) 
                                {
                                        guint bitrate = 0 ;
                                        if( gst_tag_list_get_uint_index (list, tag, i, &bitrate ))
                                        {
                                            play.m_metadata.m_audio_bitrate = bitrate; 
                                            play.signal_metadata_.emit(FIELD_AUDIO_BITRATE) ;
                                        }
                                }
                                else
                                //// AUDIO CODEC
                                if( !std::strcmp( tag, GST_TAG_AUDIO_CODEC )) 
                                {
                                        Glib::ScopedPtr<char> w ;
                                        if( gst_tag_list_get_string_index (list, tag, i, w.addr( ))) ;
                                        {
                                            play.m_metadata.m_audio_codec = std::string (w.get()); 
                                            play.signal_metadata_.emit (FIELD_AUDIO_CODEC) ;
                                        }
                                }
                                else
                                //// AUDIO CODEC
                                if( !std::strcmp( tag, GST_TAG_VIDEO_CODEC  )) 
                                {
                                        Glib::ScopedPtr<char> w ;
                                        if( gst_tag_list_get_string_index (list, tag, i, w.addr( ))) ;
                                        {
                                            play.m_metadata.m_video_codec = std::string (w.get()); 
                                            play.signal_metadata_.emit (FIELD_VIDEO_CODEC) ;
                                        }
                                }
                        }
                }

        bool
                Play::tick ()
                {
                        if( GST_STATE( control_pipe() ) == GST_STATE_PLAYING && m_conn_stream_position )
                        {
                                GstFormat   format  = GST_FORMAT_TIME  ;
                                gint64      nsec    = 0 ;

                                gst_element_query_position( control_pipe(), &format, &nsec )  ;
                                signal_position_.emit( nsec / GST_SECOND )  ;

                                return true ;
                        }

                        return false ;
                }

        void
                Play::stop_stream ()
                {
                    if( control_pipe() )
                    {
                        gst_element_set_state(
                              control_pipe ()
                            , GST_STATE_NULL
                        )  ;

                        gst_element_get_state(
                              control_pipe()
                            , NULL
                            , NULL
                            , GST_CLOCK_TIME_NONE
                        ) ; 

                        property_status_ = PLAYSTATUS_STOPPED ;
                    }
                }

        void
                Play::readify_stream ()
                {
                    if( control_pipe() )
                    {
                        GstState state ;

                        gst_element_set_state(
                              control_pipe ()
                            , GST_STATE_READY
                        )  ;

                        gst_element_get_state(
                              control_pipe()
                            , &state
                            , NULL
                            , GST_CLOCK_TIME_NONE
                        ) ; 

                        if( state == GST_STATE_READY )
                        {
                            gst_pipeline_set_new_stream_time(
                                  GST_PIPELINE(control_pipe())
                                , 0
                            ) ;

                            property_status_ = PLAYSTATUS_WAITING ;

                            return ;
                        }
                    }
                    
                    //stop_stream() ;
                }

        void
                Play::play_stream ()
                {
                    GstStateChangeReturn statechange = gst_element_set_state( control_pipe (), GST_STATE_PLAYING )  ;

                    if( statechange != GST_STATE_CHANGE_FAILURE )
                    {
                        GstState state  ;

                        gst_element_get_state(
                              control_pipe()
                            , &state
                            , NULL
                            , GST_CLOCK_TIME_NONE
                        ) ; 

                        if( state == GST_STATE_PLAYING )
                        {
                            property_status_ = PLAYSTATUS_PLAYING  ;
                            return  ;
                        }
                    }

                    //stop_stream()  ;
                }

        void
                Play::pause_stream ()
                {
                    if( GST_STATE (control_pipe() ) == GST_STATE_PAUSED )
                    {
                        GstStateChangeReturn G_GNUC_UNUSED statechange = gst_element_set_state( control_pipe (), GST_STATE_PLAYING )  ;
                        property_status_ = PLAYSTATUS_PLAYING ;
                        return ; 
                    }
                    else
                    if( GST_STATE (control_pipe() ) == GST_STATE_PLAYING )
                    {
                        GstStateChangeReturn G_GNUC_UNUSED statechange = gst_element_set_state( control_pipe (), GST_STATE_PAUSED )  ;
                        property_status_ = PLAYSTATUS_PAUSED ;
                        return ; 
                    }
                }

        void
                Play::on_volume_changed ()
                {
                    GstElement * volume = gst_bin_get_by_name(
                          GST_BIN( m_bin[BIN_OUTPUT] )
                        , NAME_VOLUME
                    )  ;

                    g_object_set(
                          G_OBJECT( volume )
                        , "volume"
                        , property_volume().get_value()/100.
                        , NULL
                    )  ;

                    gst_object_unref( volume )  ;
                }

        void
                Play::pipeline_configure (PipelineId id)
                {
                    if( m_pipeline_id != id )
                    {
                            if( m_playback_bin )
                            {
                                gst_element_set_state(
                                      m_playback_bin
                                    , GST_STATE_NULL
                                )  ;

                                gst_element_get_state(
                                      m_playback_bin
                                    , NULL
                                    , NULL
                                    , GST_CLOCK_TIME_NONE
                                ) ; 

                                gst_element_set_state(
                                      m_bin[BIN_OUTPUT]
                                    , GST_STATE_NULL
                                )  ;

                                gst_element_get_state(
                                      m_bin[BIN_OUTPUT]
                                    , NULL
                                    , NULL
                                    , GST_CLOCK_TIME_NONE
                                ) ; 

                                gst_element_unlink(
                                      m_playback_bin
                                    , m_bin[BIN_OUTPUT]
                                )  ;

                                gst_bin_remove_many(
                                      GST_BIN( m_pipeline )
                                    , m_playback_bin
                                    , m_bin[BIN_OUTPUT]
                                    , NULL
                                )  ;
                            }

                            m_pipeline_id = id ;
                            m_playback_bin   = m_bin[BinId(id)]  ;

                            gst_element_set_state(
                                  m_pipeline
                                , GST_STATE_NULL
                            )  ;

                            gst_element_get_state(
                                  m_pipeline
                                , NULL
                                , NULL
                                , GST_CLOCK_TIME_NONE
                            ) ; 

                            gst_element_set_name(
                                  m_pipeline
                                , m_pipeline_names[id]
                            )  ;

                            gst_bin_add_many(
                                  GST_BIN( m_pipeline )
                                , m_playback_bin 
                                , m_bin[BIN_OUTPUT]
                                , NULL
                            )  ;

                            gst_element_link(
                                  m_playback_bin 
                                , m_bin[BIN_OUTPUT]
                            )  ;
                    }
                }

        void
                Play::on_stream_changed ()
                {
                    if( property_sane().get_value() == false )
                        return ;

                    URI uri ;

                    try{
                        uri = URI( property_stream().get_value() )  ;
                    }
                    catch (...)
                    {
			//stop_stream (); 
                        return ;
                    }

                    switch( uri.get_protocol() )
                    {
                            case URI::PROTOCOL_FILE:
                            {
                                            pipeline_configure (PIPELINE_FILE) ;

                                            GstElement * src = gst_bin_get_by_name(
                                                                      GST_BIN(m_bin[BIN_FILE])
                                                                    , "src"
                                            )  ;

                                            g_object_set(
                                                  src
                                                , "location"
                                                , property_stream().get_value().c_str()
                                                , NULL
                                            )  ;

                                            gst_object_unref( src ) ;

                            }
                            break ;

                            case URI::PROTOCOL_MMS:
                            case URI::PROTOCOL_MMSU:
                            case URI::PROTOCOL_MMST:
                            {
                                            pipeline_configure( PIPELINE_MMSX )  ;

                                            GstElement * src = gst_bin_get_by_name(
                                                                      GST_BIN(m_bin[BIN_MMSX])
                                                                    , "src"
                                            )  ;

                                            g_object_set(
                                                  src
                                                , "location"
                                                , property_stream().get_value().c_str()
                                                , NULL
                                            )  ;

                                            gst_object_unref( src ) ;

                            }
                            break ;

                            case URI::PROTOCOL_HTTP:
                            {
                                            pipeline_configure( PIPELINE_HTTP )  ;

                                            GstElement * src = gst_bin_get_by_name(
                                                                      GST_BIN(m_playback_bin)
                                                                    , "src"
                                            )  ;

                                            g_object_set(
                                                  src
                                                , "location"
                                                , property_stream().get_value().c_str()
                                                , "prebuffer"
                                                , TRUE
                                                , NULL
                                            )  ;

                                            gst_object_unref( src ) ;

                            }
                            break ;

                            case URI::PROTOCOL_CDDA:
/*
                            {
                                            unsigned int track = static_cast<unsigned int>( std::atoi( uri.path.c_str()) + 1 ) ;

                                            pipeline_configure( PIPELINE_CDDA )  ;

                                            GstElement * src = gst_bin_get_by_name(
                                                                      GST_BIN(m_playback_bin)
                                                                    , "src"
                                            )  ;

                                            g_object_set(
                                                  src
                                                , "track"
                                                , track 
                                                , NULL
                                            )  ;

                                            gst_object_unref( src ) ;

                            }
*/
                            break ;

                            case URI::PROTOCOL_ITPC:
                            case URI::PROTOCOL_FTP:
                            case URI::PROTOCOL_QUERY:
                            case URI::PROTOCOL_TRACK:
                            case URI::PROTOCOL_LASTFM:
                            case URI::PROTOCOL_UNKNOWN:
                            {
                                // No playback of these URI types 
                            }
                            break ;
                    }
                }

        void
                Play::switch_stream_real(
                      const std::string& stream
                    , const std::string& type
                )
                {
                    m_metadata.reset() ;

                    property_stream_type_ = type ;
                    readify_stream (); 

                    property_stream_ = stream ;
                    play_stream () ;

                    signal_stream_switched_.emit() ;
                }

        void
                Play::switch_stream(
                      const std::string& stream
                    , const std::string& type
                )
                {
                    Message message ;
                    message.stream = stream ;
                    message.type = type ;
                    message.id = 1 ;
                    push_message (message) ;
                }       

        void
                Play::request_status(
                    PlayStatus status
                )
                {
                    Message message ;
                    message.status = status;
                    message.id = 0 ;
                    push_message (message) ;
                } 

        void
                Play::request_status_real(
                    PlayStatus status
                )
                {
                    switch( status )
                    {
                        case PLAYSTATUS_PAUSED:
                        {
                                pause_stream ()  ;
                        }
                        break ;

                        case PLAYSTATUS_PLAYING:
                        {
                                play_stream ()  ;
                        }
                        break ;

                        case PLAYSTATUS_WAITING:
                        {
                                m_metadata.reset() ;
                                property_stream_type_ = std::string() ;
                                readify_stream (); 
                        }
                        break ;

                        case PLAYSTATUS_STOPPED:
                        {
                                // FIXME: remove the need for this somehow

                                if( BinId( m_pipeline_id ) == BIN_HTTP ) 
                                {
                                    g_object_set(
                                        G_OBJECT(gst_bin_get_by_name (GST_BIN (m_bin[BIN_HTTP]), "src"))
                                      , "abort"
                                      , TRUE
                                      , NULL
                                    ) ; 
                                }

                                stop_stream () ;

                                m_metadata.reset() ;
                                property_stream_type_ = std::string() ;
                        }
                        break ;

                        default:
                        {
                                g_log(
                                      G_LOG_DOMAIN
                                    , G_LOG_LEVEL_WARNING
                                    , "%s: request for unsupported playstatus: %d"
                                    , G_STRFUNC
                                    , int( status )
                                )  ;
                        }
                        break ;
                    }
                }

        void
                Play::seek(
                    guint position
                )
                {
                    m_conn_stream_position.disconnect () ;

                    guint position_cur = property_position().get_value() ;

                    GstSeekFlags flags = GST_SEEK_FLAG_FLUSH ;

                    if( m_accurate_seek )
                    {
                        flags = GstSeekFlags( flags | GST_SEEK_FLAG_ACCURATE ) ;
                    }

                    if( !gst_element_seek(

                              control_pipe()

                            , 1.0

                            , GST_FORMAT_TIME

                            , flags

                            , GST_SEEK_TYPE_SET
                            , position * GST_SECOND

                            , GST_SEEK_TYPE_NONE
                            , GST_CLOCK_TIME_NONE
                    ))
                    {
                        g_print ("Seek failed!\n") ;
                    }

                    signal_seek_.emit( property_position().get_value() - position_cur ) ;
                }

        void
                Play::destroy_bins ()
                {
                    for( int n = 0; n < N_BINS; ++n )
                    {
                        if( m_bin[n] )
                        {
                            gst_element_set_state(
                                  m_bin[n]
                                , GST_STATE_NULL
                            )  ;

                            gst_element_get_state(
                                  m_bin[n]
                                , NULL
                                , NULL
                                , GST_CLOCK_TIME_NONE
                            ) ; 

                            gst_object_unref( m_bin[n] )  ;

                            m_bin[n] = 0 ;
                        }
                    }
                }

        void
                Play::link_pad(
                      GstElement* element
                    , GstPad*     pad
                    , gboolean    last
                    , gpointer    data
                )
                {
                    GstPad * pad2 = gst_element_get_static_pad (GST_ELEMENT (data), "sink"); 

                    char *pad1_name = gst_pad_get_name(pad) ;
                    char *pad2_name = gst_pad_get_name(pad2) ;

                    GstObject * parent1 = gst_pad_get_parent(pad); 
                    GstObject * parent2 = gst_pad_get_parent(pad2); 

                    char *pad1_parent_name = gst_object_get_name(parent1) ;
                    char *pad2_parent_name = gst_object_get_name(parent2) ;

                    gst_object_unref(parent1) ;
                    gst_object_unref(parent2) ;

                    g_free(pad1_name) ;
                    g_free(pad2_name) ;
                    g_free(pad1_parent_name) ;
                    g_free(pad2_parent_name) ;

                    switch( gst_pad_link (pad, pad2 ))
                    {
                            case GST_PAD_LINK_OK: 
                                    break ;

                            case GST_PAD_LINK_WRONG_HIERARCHY:
                                    g_log (G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "%s: Trying to link pads %p and %p with non common ancestry.", G_STRFUNC, pad, pad2); 
                                    break ;

                            case GST_PAD_LINK_WAS_LINKED: 
                                    g_log (G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "%s: Pad %p was already linked", G_STRFUNC, pad); 
                                    break ;

                            case GST_PAD_LINK_WRONG_DIRECTION:
                                    g_log (G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "%s: Pad %p is being linked into the wrong direction", G_STRFUNC, pad); 
                                    break ;

                            case GST_PAD_LINK_NOFORMAT: 
                                    g_log (G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "%s: Pads %p and %p have no common format", G_STRFUNC, pad, pad2); 
                                    break ;

                            case GST_PAD_LINK_NOSCHED: 
                                    g_log (G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "%s: Pads %p and %p can not cooperate in scheduling", G_STRFUNC, pad, pad2); 
                                    break ;

                            case GST_PAD_LINK_REFUSED:
                                    g_log (G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "%s: Pad %p refused the link", G_STRFUNC, pad); 
                                    break ;
                    }
                    gst_object_unref (pad2) ;
                }

        bool
                Play::clock_idle_handler ()
                {
                    // signal_spectrum_.emit( m_spectrum ) ;
                    return false ;
                }
    
        gboolean
                Play::clock_callback(
                    GstClock*       clock,
                    GstClockTime    time,
                    GstClockID      id,
                    gpointer        data
                )
                {
/*
                    Play & play = *(static_cast<Play*>( data )) ;

                    Glib::signal_idle().connect(
                            sigc::mem_fun(
                                    play,
                                    &Play::clock_idle_handler
                    )) ;
*/

                    // play.signal_spectrum_.emit( play.m_spectrum ) ;

                    return FALSE ;
                }

        void 
                Play::bus_watch (GstBus*     bus,
                                GstMessage* message,
                                gpointer    data)
                {
                    Play & play = *(static_cast<Play*>( data ))  ;

                    static GstState old_state = GstState (0), new_state = GstState (0), pending_state = GstState (0)  ;

                    switch( GST_MESSAGE_TYPE( message ))
                    {
                            case GST_MESSAGE_APPLICATION:
                            {
                                GstStructure const* s = gst_message_get_structure (message) ;

                                if( std::string (gst_structure_get_name (s )) == "buffering")
                                {
                                    double size ;
                                    gst_structure_get_double (s, "size", &size) ;
                                    play.signal_buffering_.emit(size) ;
                                }
                                else
                                if( std::string (gst_structure_get_name (s )) == "buffering-done")
                                {
                                    //// FIXME display somewhere maybe 
                                }
                            }
                            break ;

                            case GST_MESSAGE_ELEMENT:
                            {
                            }
                            break ;

                            case GST_MESSAGE_TAG:
                            {
                                GstTagList *list = 0 ;
                                gst_message_parse_tag (message, &list) ;

                                if( list )
                                {
                                    gst_tag_list_foreach (list, GstTagForeachFunc (for_each_tag), &play) ;
                                    gst_tag_list_free (list) ;
                                }

                            }
                            break ;

                            case GST_MESSAGE_STATE_CHANGED:
                            {
                                gst_message_parse_state_changed( message, &old_state, &new_state, &pending_state )  ;

                                if( old_state != GST_STATE_PLAYING && new_state == GST_STATE_PLAYING )
                                {
                                    if( !play.m_conn_stream_position.connected() )
                                    {
                                        play.m_conn_stream_position = signal_timeout().connect(sigc::mem_fun (play, &Play::tick), 500) ;
                                    }
                                }
                            }
                            break ;

                            case GST_MESSAGE_ERROR:
                            {
                                g_message("%s: GST Error, details follow.", G_STRLOC) ;
                                GError * error = NULL ;
                                Glib::ScopedPtr<char> debug ;
                                gst_message_parse_error(message, &error, debug.addr()) ;
                                g_message("%s: Error message: '%s'", G_STRLOC, error->message) ;
                                g_message("%s: Debug........: '%s'", G_STRLOC, debug.get()) ;

				play.signal_error_.emit("Playback Engine", error->message, debug.get()) ; 

				if( error->domain == GST_STREAM_ERROR ) 
				{
				    g_message("%s: Continuing to next stream", G_STRLOC) ;

				    gst_element_set_state(
					  play.m_playback_bin
					, GST_STATE_NULL
				    )  ;

				    gst_element_get_state(
					  play.m_playback_bin
					, NULL
					, NULL
					, GST_CLOCK_TIME_NONE
				    ) ; 

				    play.signal_eos_.emit() ;
				}
                            }
                            break ;

                            case GST_MESSAGE_EOS:
                            {
                                play.signal_eos_.emit() ;
                            }
                            break ;

                            default: break ;
                    }
                }

        void
                Play::queue_underrun (GstElement* element,
                                gpointer    data)
                {
                    unsigned int level ;

                    g_object_get(
                          G_OBJECT( element )
                        , "current-level-bytes"
                        , &level
                        , NULL
                    ) ;
                }

        void
                Play::eq_band_changed (MCS_CB_DEFAULT_SIGNATURE, unsigned int band)

                {
                }

        void
                Play::create_bins ()
                {
                    // Output Bin
                    {
                            std::string sink = mcs->key_get <std::string> ("audio", "sink") ;
                            GstElement* sink_element = gst_element_factory_make (sink.c_str (), "sink") ;

                            if( sink == "autoaudiosink" )
                            {
                                    /* nothing to do for it */
                            }
                            else
                            if( sink == "gconfaudiosink" )
                            {
                                    g_object_set (G_OBJECT (sink_element), "profile", int (1) /* music/video */, NULL) ;
                            }
                            else
                            if( sink == "osssink" )
                            {
                                    g_object_set (G_OBJECT (sink_element), "device",
                                                    nullify_string (mcs->key_get <std::string> ("audio", "device-oss")),
                                                    NULL) ;
                                    g_object_set (G_OBJECT (sink_element), "buffer-time",
                                                    guint64(mcs->key_get <int> ("audio", "oss-buffer-time")), NULL) ;
                            }
                            else
                            if( sink == "esdsink" )
                            {
                                    g_object_set (G_OBJECT (sink_element), "host",
                                                    nullify_string (mcs->key_get <std::string> ("audio", "device-esd")),
                                                    NULL) ;
                                    g_object_set (G_OBJECT (sink_element), "buffer-time",
                                                    guint64(mcs->key_get <int> ("audio", "esd-buffer-time")),
                                                    NULL) ;
                            }
                            else
                            if( sink == "pulsesink" )
                            {
                                    g_object_set (G_OBJECT (sink_element), "server",
                                                    nullify_string (mcs->key_get <std::string> ("audio", "pulse-server")),
                                                    NULL) ;
                                    g_object_set (G_OBJECT (sink_element), "device",
                                                    nullify_string (mcs->key_get <std::string> ("audio", "pulse-device")),
                                                    NULL) ;
                                    g_object_set (G_OBJECT (sink_element), "buffer-time",
                                                    guint64(mcs->key_get <int> ("audio", "pulse-buffer-time")),
                                                    NULL) ;
                            }
                            else
                            if( sink == "jackaudiosink" )
                            {
                                    g_object_set (G_OBJECT (sink_element), "server",
                                                    nullify_string (mcs->key_get <std::string> ("audio", "jack-server")),
                                                    NULL) ;
                                    g_object_set (G_OBJECT (sink_element), "buffer-time",
                                                    guint64(mcs->key_get <int> ("audio", "jack-buffer-time")),
                                                    NULL) ;
                            }
#ifdef HAVE_ALSA
                            else
                            if( sink == "alsasink" )
                            {
                                    g_object_set (G_OBJECT (sink_element), "device",
                                                    nullify_string (mcs->key_get <std::string> ("audio", "device-alsa")),
                                                    NULL) ;
                                    g_object_set (G_OBJECT (sink_element), "buffer-time",
                                                    guint64(mcs->key_get <int> ("audio", "alsa-buffer-time")),
                                                    NULL) ;
                            }
#endif //HAVE_ALSA
#ifdef HAVE_SUN
                            else
                            if( sink == "sunaudiosink" )
                            {
                                    g_object_set (G_OBJECT (sink_element), "device",
                                                    nullify_string (mcs->key_get <std::string> ("audio", "device-sun")),
                                                    NULL) ;
                                    g_object_set (G_OBJECT (sink_element), "buffer-time",
                                                    guint64(mcs->key_get <int> ("audio", "sun-buffer-time")),
                                                    NULL) ;
                            }
#endif //HAVE_SUN
#ifdef HAVE_HAL
                            else
                            if( sink == "halaudiosink" )
                            {
                                    g_object_set (G_OBJECT (sink_element), "udi",
                                                    nullify_string (mcs->key_get <std::string> ("audio", "hal-udi")),
                                                    NULL) ;
                            }
#endif //HAVE_HAL

                            m_bin[BIN_OUTPUT]     = gst_bin_new ("output") ;
                            GstElement* convert   = gst_element_factory_make ("audioconvert", (NAME_CONVERT)) ;
                            GstElement* resample  = gst_element_factory_make ("audioresample", (NAME_RESAMPLE)) ;
                            GstElement* volume    = gst_element_factory_make ("volume", (NAME_VOLUME)) ;

			    gst_bin_add_many( GST_BIN (m_bin[BIN_OUTPUT]), convert, resample, volume, sink_element, NULL) ;
			    gst_element_link_many (convert, resample, volume, sink_element, NULL) ;

                            GstPad * pad = gst_element_get_static_pad (convert, "sink") ;
                            gst_element_add_pad (m_bin[BIN_OUTPUT], gst_ghost_pad_new ("sink", pad)) ;
                            gst_object_unref (pad) ;

                            gst_object_ref (m_bin[BIN_OUTPUT]) ;

                            property_volume() = mcs->key_get <int> ("mpx", "volume") ;
                            property_sane_ = true ;
                    }


                    ////////////////// FILE BIN
                    {
                            GstElement* source    = gst_element_factory_make ("giosrc", "src") ;
                            GstElement* decoder   = gst_element_factory_make ("decodebin", "Decoder File"); 
                            GstElement* identity  = gst_element_factory_make ("identity", "Identity File"); 

                            if( source && decoder && identity )
                            {
                                    m_bin[BIN_FILE] = gst_bin_new ("bin-file") ;

                                    gst_bin_add_many(
                                          GST_BIN(m_bin[BIN_FILE])
                                        , source
                                        , decoder
                                        , identity
                                        , NULL
                                    )  ;

                                    gst_element_link_many(
                                          source
                                        , decoder
                                        , NULL
                                    )  ;

                                    g_signal_connect(
                                          G_OBJECT (decoder)
                                        , "new-decoded-pad"
                                        , G_CALLBACK(Play::link_pad)
                                        , identity
                                    )  ;

                                    GstPad * pad = gst_element_get_static_pad (identity, "src") ;
                                    gst_element_add_pad (m_bin[BIN_FILE], gst_ghost_pad_new ("src", pad)) ;
                                    gst_object_unref (pad) ;

                                    gst_object_ref (m_bin[BIN_FILE]) ;
                            }
                    }


                    ////////////////// HTTP BIN
                    {
                            GstElement  * source    = gst_element_factory_make ("jnethttpsrc", "src") ;
                            GstElement  * decoder   = gst_element_factory_make ("decodebin", "Decoder HTTP"); 
                            GstElement  * identity  = gst_element_factory_make ("identity", "Identity HTTP"); 

                            if( source && decoder && identity )
                            {
                                    m_bin[BIN_HTTP] = gst_bin_new ("bin-http") ;
                                    gst_bin_add_many (GST_BIN (m_bin[BIN_HTTP]), source, decoder, identity, NULL) ;
                                    gst_element_link_many (source, decoder, NULL) ;

                                    g_signal_connect (G_OBJECT (decoder),
                                                    "new-decoded-pad",
                                                    G_CALLBACK (Play::link_pad),
                                                    identity) ;
                                    g_object_set (G_OBJECT (source),
                                                    "iradio-mode", TRUE, 
                                                    NULL) ;

                                    GstPad * pad = gst_element_get_static_pad (identity, "src") ;
                                    gst_element_add_pad (m_bin[BIN_HTTP], gst_ghost_pad_new ("src", pad)) ;
                                    gst_object_unref (pad) ;

                                    gst_object_ref (m_bin[BIN_HTTP]) ;
                            }
                    }

                    ////////////////// MMS* BIN
                    {
                            GstElement * source   = gst_element_factory_make ("mmssrc", "src") ;
                            GstElement * decoder  = gst_element_factory_make ("decodebin", "Decoder MMSX"); 
                            GstElement * identity = gst_element_factory_make ("identity", "Identity MMSX"); 

                            if( source && decoder && identity )
                            {
                                    m_bin[BIN_MMSX] = gst_bin_new ("bin-mmsx") ;

                                    gst_bin_add_many (GST_BIN (m_bin[BIN_MMSX]), source, decoder, identity, NULL) ;
                                    gst_element_link_many (source, decoder, NULL) ;

                                    g_signal_connect (G_OBJECT (decoder),
                                                    "new-decoded-pad",
                                                    G_CALLBACK (Play::link_pad),
                                                    identity) ;

                                    GstPad * pad = gst_element_get_static_pad (identity, "src") ;
                                    gst_element_add_pad (m_bin[BIN_MMSX], gst_ghost_pad_new ("src", pad)) ;
                                    gst_object_unref (pad) ;

                                    gst_object_ref (m_bin[BIN_MMSX]) ;
                            }
                    }

/*
                    ////////////////// CDDA BIN
                    {
#if defined (HAVE_CDPARANOIA)
                            GstElement * source = gst_element_factory_make ("cdparanoiasrc", "src") ;
#elif defined (HAVE_CDIO)
                            GstElement * source = gst_element_factory_make ("cdiocddasrc", "src") ;
#endif

                            if( source )
                            {
                                    m_bin[BIN_CDDA] = gst_bin_new ("bin-cdda") ;
                                    gst_bin_add_many (GST_BIN (m_bin[BIN_CDDA]), source, NULL) ;

                                    GstPad * pad = gst_element_get_static_pad (source, "src") ;
                                    gst_element_add_pad (m_bin[BIN_CDDA], gst_ghost_pad_new ("src", pad)) ;
                                    gst_object_unref (pad) ;

                                    gst_object_ref (m_bin[BIN_CDDA]) ;
                            }
                    }
*/

                    m_pipeline      = gst_pipeline_new ("YoukiPipeline") ;
                    m_pipeline_id   = PIPELINE_NONE ; // not configured yet

                    GstBus * bus = gst_pipeline_get_bus( GST_PIPELINE( m_pipeline )) ;
                    gst_bus_add_signal_watch( bus ) ;
                    g_signal_connect(
                          G_OBJECT( bus )
                        , "message"
                        , GCallback( Play::bus_watch )
                        , this
                    ) ;
                    gst_object_unref( bus ) ;
                }

        void
                Play::reset ()
                {
                    if( m_pipeline )
                    {
                            if( property_status().get_value() != PLAYSTATUS_STOPPED )
                            {
                                stop_stream () ;
                            }

                            if( m_playback_bin )
                            {
                                gst_element_set_state(
                                      m_playback_bin
                                    , GST_STATE_NULL
                                )  ;

                                gst_element_get_state(
                                      m_playback_bin
                                    , NULL
                                    , NULL
                                    , GST_CLOCK_TIME_NONE
                                ) ; 

                                gst_element_set_state(
                                      m_bin[BIN_OUTPUT]
                                    , GST_STATE_NULL
                                )  ;

                                gst_element_get_state(
                                      m_bin[BIN_OUTPUT]
                                    , NULL
                                    , NULL
                                    , GST_CLOCK_TIME_NONE
                                ) ; 

                                gst_element_unlink(
                                      m_playback_bin
                                    , m_bin[BIN_OUTPUT]
                                )  ;

                                gst_bin_remove_many(
                                      GST_BIN( m_pipeline )
                                    , m_playback_bin
                                    , m_bin[BIN_OUTPUT]
                                    , NULL
                                )  ;
                            }

                            gst_object_unref( m_pipeline )  ;

                            m_pipeline     = 0  ;
                            m_playback_bin = 0  ;
                    }

                    destroy_bins()  ;
                    create_bins()  ;

                    m_accurate_seek = mcs->key_get<bool>( "audio","accurate-seek" ) ;

                    property_status_ = PLAYSTATUS_STOPPED ;
                }

        ///////////////////////////////////////////////
        /// Object Properties
        ///////////////////////////////////////////////

        ProxyOf<PropString>::ReadWrite
                Play::property_stream_type()
                {
                        return ProxyOf<PropString>::ReadWrite(
                            this
                          , "stream-type"
                        ) ;
                }

        ProxyOf<PropString>::ReadWrite
                Play::property_stream()
                {
                        return ProxyOf<PropString>::ReadWrite(
                            this
                          , "stream"
                        ) ;
                }

        ProxyOf<PropInt>::ReadWrite
                Play::property_volume()
                {
                        return ProxyOf<PropInt>::ReadWrite(
                              this
                            , "volume"
                        ) ;
                }

        // RO Proxies //

        ProxyOf<PropInt>::ReadOnly
                Play::property_status() const
                {
                        return ProxyOf<PropInt>::ReadOnly(
                              this
                            , "playstatus"
                        ) ;
                }

        ProxyOf<PropBool>::ReadOnly
                Play::property_sane() const
                {
                        return ProxyOf<PropBool>::ReadOnly(
                              this
                            , "sane"
                        ) ;
                }

        ProxyOf<PropInt>::ReadOnly
                Play::property_position() const
                {
                        GstFormat format (GST_FORMAT_TIME) ;
                        gint64 position = 0 ;

                        gst_element_query_position( GST_ELEMENT( control_pipe()), &format, &position ) ;

                        g_object_set( G_OBJECT( gobj()), "position", (position/GST_SECOND), NULL ) ;

                        return ProxyOf<PropInt>::ReadOnly(
                              this
                            , "position"
                        ) ;
                }

        ProxyOf<PropInt>::ReadOnly
                Play::property_duration() const
                {
                        GstFormat format (GST_FORMAT_TIME) ;
                        gint64 duration = 0 ;

                        gst_element_query_duration( GST_ELEMENT( control_pipe()), &format, &duration ) ;

                        g_object_set( G_OBJECT( gobj()), "duration", (duration/GST_SECOND), NULL ) ;

                        return ProxyOf<PropInt>::ReadOnly(
                              this
                            , "duration"
                        ) ;
                }

        /* Signals --------------------------------------------------------------------*/

        SignalSpectrum &
                Play::signal_spectrum ()
                {
                        return signal_spectrum_ ;
                }

        SignalEos &
                Play::signal_eos ()
                {
                        return signal_eos_ ;
                }

        SignalSeek &
                Play::signal_seek ()
                {
                        return signal_seek_ ;
                }

        SignalPosition &
                Play::signal_position ()
                {
                        return signal_position_ ;
                }

        SignalBuffering &
                Play::signal_buffering ()
                {
                        return signal_buffering_ ;
                }

        SignalError &
                Play::signal_error ()
                {
                        return signal_error_ ;
                }

        SignalMetadata &
                Play::signal_metadata ()
                {
                        return signal_metadata_ ;
                }

        SignalStreamSwitched &
                Play::signal_stream_switched ()
                {
                        return signal_stream_switched_ ;
                }

        /*--------------------------------------------------------*/

        GstMetadata const&
                Play::get_metadata ()
                {
                        return m_metadata ;
                }

        void
                Play::process_queue ()
                {
                        while( g_async_queue_length( m_message_queue ) > 0 )
                        {
                                gpointer data = g_async_queue_pop( m_message_queue ) ;

                                Message * message = reinterpret_cast<Message*>(data) ;

                                switch( message->id )
                                {
                                        case 0: /* FIXME: Enums */
                                                request_status_real( message->status ) ;
                                                break ;

                                        case 1:
                                                switch_stream_real( message->stream, message->type ) ;
                                                break ;
                                }

                                delete message ;
                        }
                }

        void
                Play::push_message(
                      const Message& message
                )
                {
                        g_async_queue_push (m_message_queue, (gpointer)(new Message (message))) ;
                        process_queue () ;
                }
}
