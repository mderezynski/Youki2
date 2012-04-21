//  BMPx - The Dumb Music IPlayer
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

#ifndef _YOUKI_PLAY__INTERFACE__HH
#define _YOUKI_PLAY__INTERFACE__HH

#include "config.h"

#include <string>
#include <vector>

#include <glibmm.h>
#include <sigc++/sigc++.h>
#include <boost/optional.hpp>
#include <gdkmm.h>

#include "mpx/aux/glibaddons.hh"

struct _GstClock ;
typedef _GstClock GstClock ;

namespace MPX
{
        const int SPECT_BANDS = 64 ;
        typedef std::vector<float> Spectrum ;

        enum GstMetadataField
        {
            FIELD_TITLE,
            FIELD_ALBUM,
            FIELD_IMAGE,
            FIELD_AUDIO_BITRATE,
            FIELD_AUDIO_CODEC,
            FIELD_VIDEO_CODEC,
        };

        struct GstMetadata
        {
            boost::optional<std::string>                m_title;
            boost::optional<std::string>                m_album;
            boost::optional<Glib::RefPtr<Gdk::Pixbuf> > m_image;
            boost::optional<unsigned int>               m_audio_bitrate;
            boost::optional<std::string>                m_audio_codec;
            boost::optional<std::string>                m_video_codec;

            void reset ()
            {
                m_title.reset ();
                m_album.reset ();
                m_image.reset ();
                m_audio_bitrate.reset ();
                m_audio_codec.reset ();
                m_video_codec.reset ();
                m_audio_bitrate.reset ();
            }
        };

        /** IPlayback Engine
         *
         * MPX::IPlay is the playback engine of BMP. It is based on GStreamer 0.10
         * using a rather simple design. (http://www.gstreamer.net)
         *
         */

        typedef sigc::signal<void>                          SignalEos ;
        typedef sigc::signal<void, guint>                   SignalSeek ;
        typedef sigc::signal<void, guint>                   SignalPosition ;
        typedef sigc::signal<void, double>                  SignalBuffering ;
        typedef sigc::signal<void>                          SignalStreamSwitched;
        typedef sigc::signal<void, Spectrum>                SignalSpectrum ;
        typedef sigc::signal<void, GstMetadataField>        SignalMetadata ;
        typedef sigc::signal<void
                , const Glib::ustring&   /* element name */
                , const Glib::ustring&   /* location     */
                , const Glib::ustring&   /* debug string */> SignalError ;

        class IPlay
        : public Glib::Object
        {
                public:

                        IPlay () {} ;
                        virtual ~IPlay () {} ;

                        virtual ProxyOf<PropString>::ReadWrite
                        property_stream () = 0 ;

                        virtual ProxyOf<PropString>::ReadWrite
                        property_stream_type() = 0 ;

                        virtual ProxyOf<PropInt>::ReadWrite
                        property_volume() = 0 ;

                        virtual ProxyOf<PropInt>::ReadOnly
                        property_status() const = 0 ;

                        virtual ProxyOf<PropBool>::ReadOnly
                        property_sane() const = 0 ; 

                        virtual ProxyOf<PropInt>::ReadOnly
                        property_position() const = 0 ;

                        virtual ProxyOf<PropInt>::ReadOnly
                        property_duration() const = 0 ;

                        /** Signal emitted on error state 
                         *
                         */
                        virtual SignalError&
                                signal_error() = 0 ;

                        /** Signal emitted on spectrum data 
                         *
                         */
                        virtual SignalSpectrum&
                                signal_spectrum() = 0 ;

                       /** Signal emitted on end of stream
                         *
                         */
                        virtual SignalEos&  
                                signal_eos() = 0 ;

                        /** Signal emitted on a successful seek event
                         *
                         */
                        virtual SignalSeek&
                                signal_seek() = 0 ;

                        /** Signal emitted on stream position change
                         *
                         */
                        virtual SignalPosition&
                                signal_position() = 0 ;

                        /** Signal emitted when prebuffering live stream data 
                         *
                         */
                        virtual SignalBuffering&
                                signal_buffering() = 0 ;

                        /** Signal on new metadata 
                         *
                         */
                        virtual SignalMetadata&
                                signal_metadata() = 0 ;

                        /** Signal sent after stream switch is complete 
                         *
                         */
                        virtual SignalStreamSwitched&
                                signal_stream_switched() = 0 ;


                        virtual const GstMetadata&
                        get_metadata () = 0 ;

                        virtual void
                        reset(
                        ) = 0 ;

                        virtual GstClock*
                        get_clock() = 0 ;
        };
}

#endif // _YOUKI_PLAY__INTERFACE__HH

