//  BMPx - The Dumb Music Player
//  Copyright (C) 2005-2007 BMPx development team.
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
//  The BMPx project hereby grants permission for non-GPL compatible GStreamer
//  plugins to be used and distributed together with GStreamer and BMPx. This
//  permission is above and beyond the permissions granted by the GPL license
//  BMPx is covered by.

#ifndef MPX_PLAY_HH
#define MPX_PLAY_HH

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif //HAVE_CONFIG_H

#include <map>
#include <string>
#include <vector>

#include <glibmm/object.h>
#include <glibmm/property.h>
#include <glibmm/propertyproxy.h>
#include <glibmm/ustring.h>
#include <sigc++/signal.h>
#include <sigc++/connection.h>
#include <boost/optional.hpp>

#include <gst/gst.h>
#include <gst/gstelement.h>
#include <gst/interfaces/mixer.h>
#include <gst/interfaces/mixertrack.h>
#include <gst/interfaces/mixeroptions.h>

#include <sigc++/sigc++.h>
#include <glibmm/ustring.h>
#include <glibmm/refptr.h>
#include <gdkmm/pixbuf.h>
#include <gst/gst.h>

#include "mpx/mpx-main.hh"
#include "mpx/mpx-services.hh"
#include "mpx/mpx-types.hh"
#include "mpx/mpx-uri.hh"
#include "mpx/aux/glibaddons.hh"

#include "mpx/i-youki-play.hh"

namespace MPX
{
        /** Playback Engine
         *
         * MPX::Play is the playback engine of BMP. It is based on GStreamer 0.10
         * using a rather simple design. (http://www.gstreamer.net)
         *
         */

        struct Message
    	{
	    	int         id ;

    		PlayStatus  status ;
	    	std::string stream ;
		    std::string type ;

    		Message()
            : id( -1 )
            {}
    	} ;
    
        class Play
        : public IPlay
        , public Service::Base
        {
                private:

                        enum PipelineId
                        {
                              PIPELINE_NONE     = 0
                            , PIPELINE_HTTP
                            , PIPELINE_MMSX
                            , PIPELINE_FILE
                            , PIPELINE_CDDA
                        };

                        enum BinId
                        {
                              BIN_OUTPUT        = 0
                            , BIN_HTTP
                            , BIN_MMSX
                            , BIN_FILE
                            , BIN_CDDA
                            , N_BINS
                        };

                        PipelineId          m_pipeline_id ;
                        GstElement        * m_pipeline ;
                        GstElement        * m_equalizer ;
                        GstElement        * m_playback_bin ;
                        GstElement        * m_bin[N_BINS] ;
                        GAsyncQueue       * m_message_queue ;
                        GstMetadata         m_metadata ;
                        Spectrum            m_spectrum
                                          , m_spectrum_zero ;
                        sigc::connection    m_conn_stream_position ;
                        bool                m_accurate_seek ;

                private:

                        //// MESSAGE QUEUE

                        void
                        process_queue () ;

                        void
                        push_message(
                              const Message&
                        ) ;

                public:

                        Play ();
                        ~Play ();

                        ProxyOf<PropString>::ReadWrite
                        property_stream ();

                        ProxyOf<PropString>::ReadWrite
                        property_stream_type ();

                        ProxyOf<PropInt>::ReadWrite
                        property_volume ();

                        ProxyOf<PropInt>::ReadOnly
                        property_status() const;

                        ProxyOf<PropBool>::ReadOnly
                        property_sane() const; 

                        ProxyOf<PropInt>::ReadOnly
                        property_position() const; 

                        ProxyOf<PropInt>::ReadOnly
                        property_duration() const; 

                        /** Signal emitted on error state 
                         *
                         */
                        SignalError&
                                signal_error();

                        /** Signal emitted on spectrum data 
                         *
                         */
                        SignalSpectrum&
                                signal_spectrum();

                        /** Signal emitted on end of stream
                         *
                         */
                        SignalEos&  
                                signal_eos();

                        /** Signal emitted on a successful seek event
                         *
                         */
                        SignalSeek&
                                signal_seek();

                        /** Signal emitted on stream position change
                         *
                         */
                        SignalPosition&
                                signal_position();

                        /** Signal emitted when prebuffering live stream data 
                         *
                         */
                        SignalBuffering&
                                signal_buffering();

                        /** Signal on new metadata 
                         *
                         */
                        SignalMetadata&
                                signal_metadata();

                        /** Signal sent after stream switch is complete 
                         *
                         */
                        SignalStreamSwitched&
                                signal_stream_switched();


                        const GstMetadata&
                        get_metadata ();
    
                        GstClock*
                        get_clock()
                        {
                            return gst_pipeline_get_clock(GST_PIPELINE( m_pipeline )) ;
                        }

                        void
                        request_status(
                              PlayStatus
                        ) ;

                        void
                        switch_stream(
                              const std::string&
                            , const std::string& = std::string()
                        ) ;

                        void
                        seek(
                              guint
                        ) ;

                        void
                        reset(
                        ) ;

                private:

                        SignalMetadata          signal_metadata_;
                        SignalSeek              signal_seek_;
                        SignalBuffering         signal_buffering_;
                        SignalSpectrum          signal_spectrum_;
                        SignalPosition          signal_position_;
                        SignalEos               signal_eos_;
                        SignalError             signal_error_;
                        SignalStreamSwitched    signal_stream_switched_;

                        void
                        eq_band_changed(
                              MCS_CB_DEFAULT_SIGNATURE
                            , unsigned int band
                        ) ;

                        static void
                        queue_underrun(
                              GstElement*
                            , gpointer 
                        ) ;

                        static void
                        link_pad(
                              GstElement*
                            , GstPad*
                            , gboolean
                            , gpointer
                        ) ;

                        static void
                        bus_watch(
                              GstBus*
                            , GstMessage*
                            , gpointer
                        ) ;

                        static gboolean
                        foreach_structure(
                              GQuark
                            , const GValue*
                            , gpointer
                        ) ;

                        static void
                        for_each_tag(
                              const GstTagList*
                            , const gchar*
                            , gpointer
                        ) ;

                        // Properties

                        PropString          property_stream_;
                        PropString          property_stream_type_;
                        PropInt             property_volume_;
                        PropInt             property_status_;
                        PropBool            property_sane_;
                        PropInt             property_position_;
                        PropInt             property_duration_;

                        void
                        destroy_bins(
                        ) ;

                        void
                        create_bins(
                        ) ;

                        void
                        stop_stream(
                        ) ;

                        void
                        play_stream(
                        ) ;

                        void
                        pause_stream(
                        ) ;

                        void
                        readify_stream(
                        ) ;

                        bool
                        tick(
                        ) ;

                        bool
                        clock_idle_handler(
                        ) ;

                        void
                        on_stream_changed(
                        ) ;

                        void
                        on_volume_changed(
                        ) ;

                        void
                        pipeline_configure(
                              PipelineId
                        ) ;

                        void
                        request_status_real(
                              PlayStatus
                        ) ;

                        void
                        switch_stream_real(
                              const std::string&
                            , const std::string& = std::string()
                        ) ;

                        static gboolean
                        clock_callback(
                              GstClock*
                            , GstClockTime 
                            , GstClockID
                            , gpointer
                        ) ;

                        GstElement*
                        control_pipe() const ;
        };
}

#endif // MPX_PLAY_HH
