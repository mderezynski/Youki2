//  MPX - The Dumb Music Player
//  Copyright (C) 2005-2007 MPX development team.
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

#ifndef MPX_AUDIO_HH
#define MPX_AUDIO_HH

#include "config.h"

#include <string>

#ifndef MPX_PLUGIN_BUILD
#include <glibmm/ustring.h>
#include <glibmm/object.h>
#include <glibmm/property.h>
#include <glibmm/propertyproxy.h>
#include <gtkmm.h>
#include <sigc++/signal.h>
#include <boost/variant.hpp>
#include <boost/optional.hpp>
#include <gst/gst.h>
#include <gst/gstelement.h>
#include <gst/interfaces/mixer.h>
#include <gst/interfaces/mixertrack.h>
#include <gst/interfaces/mixeroptions.h>
#endif //!MPX_PLUGIN_BUILD

#include "mpx/mpx-uri.hh"

namespace MPX
{
  class AudioException
  : public std::exception
  {
    public:

        AudioException(
            const std::string& message_arg = std::string()
        ) 
        : message( message_arg )
        {
        }

        virtual ~AudioException() throw()
        {
        }

        const char*
        what() const throw()
        {
            return message.c_str();
        }

    private:

        std::string message ;
  } ;

#define AUDIOEXCEPTION(EXCEPTION_NAME)                                              \
    class EXCEPTION_NAME                                                            \
    : public AudioException                                                         \
    {                                                                               \
      public:                                                                       \
        EXCEPTION_NAME(                                                             \
            const std::string& message = std::string()                              \
        ) : AudioException( message )                                               \
        {                                                                           \
        }                                                                           \
    };

  namespace Audio
  {
    bool
    typefind (std::string const& uri, std::string & type);

#ifndef MPX_PLUGIN_BUILD

    Glib::RefPtr<Gdk::Pixbuf>
    get_inline_image (std::string const& uri); 

    /** Determine whether a stream/file specified by uri is a playable file or not 
     *
     * @param uri The URI to determine whether a stream is an audio file or not 
     *
     */
    bool
    is_audio_file (std::string const& uri);

    std::string
    get_ext_for_type (std::string const& type);

    bool
    test_element (std::string const& name);

    enum Sink
    {
      SINK_ALSA,
      SINK_GCONF,
      SINK_OSS,
      SINK_SUNAUDIO,
      SINK_ESD,
      SINK_HAL,
      SINK_PULSEAUDIO,
      SINK_JACKSINK,
      SINK_AUTO,
    };

#include "mpx/exception.hh"

    EXCEPTION (PipelineError)
    EXCEPTION (InvalidUriError)

    /** Elements enum
     */
    enum Elements
    {
        SOURCE,
        DECODER,
        TEE,
        QUEUE,
        CONVERT,
        RESAMPLE,
        VOLUME,
        PUID,
        SINK,
        LAME,
        FLACENC,
        VORBISENC,
        OGGMUX,

        N_ELEMENTS
    };

    /** An element consists of a a 'name' (std::string) and a
     *  list of attributes (@link MPX::Audio::Element::Attrs@endlink)
     */
    struct Element
    {
      class Attr;

      /** std::vector typedef of an Attr
       */
      typedef std::vector<Attr> Attrs;

      std::string name;
      Attrs       attrs;

      /** Default ctor
       */
      Element () {}

      /** Ctor that takes the name of the element
       */
      Element (std::string const& name)
        : name (name)
      {}

      /** Ctor that takes the name and a reference to a vector
       *  of @link MPX::Audio::Element::Attrs@endlink
       */
      Element (std::string const& name,
               Attrs const&       attrs)
        : name  (name),
          attrs (attrs)
      {}

      /** Adds an attribute to the list of element attributes
       * @param attribute An @link MPX::Audio::Element::Attr@endlink
       * @returns void
       */
      void
      add (Attr const& attr)
      {
        attrs.push_back (attr);
      }
    };

    /** An attribute holds a @link MPX::Audio::Element::Attr::Value@endlink,
     *  and a name (std::string)
     */
    struct Element::Attr
    {
      /** boost::variant type for bool, int, double and string values
       */
      typedef boost::variant<bool, int, double, std::string> Value;

      std::string name;
      Value       value;

      /** Default ctor
       */
      Attr () {}

      /** Ctor taking the value, type and the name
       */
      Attr (std::string const& name,
            Value const&       value)
        : name  (name)
        , value (value)
      {}
    };

    /** Current state of the audio processing unit
     */
    enum ProcessorState
    {
      STATE_STOPPED,
      STATE_PAUSED,
      STATE_RUNNING,
    };

    /** Stream audioproperties
      *
      */
    struct StreamProperties
    {
      unsigned int bitrate;
      unsigned int samplerate;
      std::string  title;

      StreamProperties () : bitrate (0), samplerate (0), title ("") {}
    };

    /** sigc signal typedef for the stream position
     */
    typedef sigc::signal<void, int> SignalPosition;

    /** sigc signal typedef signalizing end-of-stream
     */
    typedef sigc::signal<void> SignalEos;

    /** sigc signal typedef signalizing error
     */
    typedef sigc::signal<void, Glib::ustring const&> SignalError;

    /** sigc signal typedef for stream properties
     */
    typedef sigc::signal<void, StreamProperties&> SignalStreamProperties;

    /** This is the base class for the audio processors
      *
      */
    class ProcessorBase
      : public Glib::Object
    {
      public:

        ProcessorBase ();

        virtual
        ~ProcessorBase ();

        /** PropertyProxy to get or set the stream time.
         *  Setting the stream time equals to a seek.
         */
        virtual Glib::PropertyProxy<unsigned int>
        prop_stream_time ();

        /** PropertyProxy to get or set the time interval in millisecond
         *  at which to report the current stream time via SignalPosition
         */
        virtual Glib::PropertyProxy<unsigned int>
        prop_stream_time_report_interval ();

        /** ProcessorState PropertyProxy. Allows to read the current processor
         *  state, or set it which equals to one of ::stop (),
         *  ::run (), or ::pause ()
         */
        virtual Glib::PropertyProxy<ProcessorState>
        prop_state ();

        virtual Glib::PropertyProxy<unsigned int>
        prop_length();

        /** Volume, Range 0-100
         *
         */
        virtual Glib::PropertyProxy<int>
        prop_volume();

        virtual SignalPosition&
        signal_position();

        virtual SignalEos&
        signal_eos();

        virtual SignalStreamProperties&
        signal_stream_properties ();

        virtual SignalError&
        signal_error ();

        /** Starts the processor
         */
        GstStateChangeReturn
        run ();

        /** Stops the processor
         */
        GstStateChangeReturn
        stop ();

        /** Puts the processor into pause mode
         */
        GstStateChangeReturn
        pause ();

        /** Taps into the processor's pipeline
         *
         * @returns The 'tee' element of the pipeline to connect a Processor_Source_* to
         *
         */
        GstElement*
        tap ();

      protected:

        virtual bool
        verify_pipeline ();

        GstElement* pipeline;

        Glib::Property<unsigned int>    prop_stream_time_;
        Glib::Property<unsigned int>    prop_stream_time_report_interval_;
        Glib::Property<unsigned int>    prop_length_;
        Glib::Property<int>             prop_volume_;
        Glib::Property<ProcessorState>  prop_state_;

        SignalPosition                  signal_position_;
        SignalEos                       signal_eos_;
        SignalStreamProperties          signal_stream_properties_;
        SignalError                     signal_error_;

        sigc::connection                conn_position;
        sigc::connection                conn_state;

        StreamProperties                stream_properties;

        /** Creates a pipeline as specified by the current source and sink elements
         */
        virtual void
        create_pipeline () = 0;

        /** Preforms a position query on the current pipeline and emits the current
         *  stream position via @link SignalPosition@endlink
         */
        bool
        emit_stream_position ();

        /** Stops emission of the current stream position
         */
        void
        position_send_stop ();

        /** Starts emission of the current stream position in intervals specified
         *  by prop_stream_time_report_interval()
         */
        void
        position_send_start ();

        /** Handler of state changes of the processor's state property
         */
        void
        prop_state_changed ();

        /** Handler of state changes of the processor's volume property
         */
        void
        prop_volume_changed ();

        /** Handler of changes to the interval for sending the stream's position
         */
        void
        position_send_interval_change ();

        static void
        link_pad (GstElement* element,
                  GstPad*     pad,
                  gboolean    last,
                  gpointer    data);

        static gboolean
        bus_watch (GstBus*     bus,
                   GstMessage* message,
                   gpointer    data);

        static gboolean
        foreach_structure (GQuark        field_id,
                           GValue const* value,
                           gpointer      data);
    };


    /** This class can be used to play a stream
     *
     */
    class ProcessorURISink
      : public ProcessorBase
    {
      public:

        ProcessorURISink ();

        virtual
        ~ProcessorURISink ();

        /** Will set the uri to the ProcessorURISink
         * @param uri The uri to use. The processor will automatically adapt to the protocol of
         *            the URI to construct an appropriate pipeline
         *
         * @param sink The @link MPX::Audio::Element::Element@endlink to specify the sink
         */
        void
        set_uri (Glib::ustring const& uri,
                 const Element&       sink);

      private:

        virtual void
        create_pipeline ();

        URI::Protocol current_protocol;
        Glib::ustring stream;

        Element source;
        Element sink;
    };

    /** This class uses GstPUID to determine a file PUID
     *
     */
    class ProcessorPUID
      : public ProcessorBase
    {
      public:

        ProcessorPUID ();

        virtual
        ~ProcessorPUID ();

        /** Will set the uri to the ProcessorPUID
         * @param uri The uri to use
         */
        void
        set_uri (Glib::ustring const& uri);

        boost::optional<Glib::ustring>
        get_puid () const;

        boost::optional<Glib::ustring>
        get_artist () const;

        boost::optional<Glib::ustring>
        get_title () const;

      private:

        Glib::ustring m_stream;

        boost::optional<Glib::ustring> m_puid;
        boost::optional<Glib::ustring> m_artist;
        boost::optional<Glib::ustring> m_title;

        void
        stream_eos ();

        virtual void
        create_pipeline ();

    };

    //////// File Transcoding

    enum ConversionType
    {
      CONV_OGG_VORBIS,
      CONV_MPEG1_L3,
    };

    class ProcessorFileTranscode
      : public ProcessorBase
    {
      protected:

        virtual void create_pipeline ();

      public:

        ProcessorFileTranscode (std::string const&  filename_src,
                                std::string const&  filename_dest,
                                ConversionType type);

        ProcessorFileTranscode (ConversionType type);
        virtual ~ProcessorFileTranscode () {}

      protected:

        std::string  m_filename_src, m_filename_dest;
        int m_quality;
        ConversionType m_type;
    };

    ///////////////////
    
    class ProcessorTranscode
      : public ProcessorBase
    {
      public:

        ProcessorTranscode ()
          : Glib::ObjectBase (typeid (this)),
            m_track   (1),
            m_quality (1)
          {}

        ProcessorTranscode (std::string const&  filename,
                            unsigned int        track,
                            std::string const&  device,
                            int                 quality)
          : Glib::ObjectBase (typeid (this)),
            m_filename  (filename),
            m_track     (track),
            m_device    (device),
            m_quality   (quality)
          {}

        virtual ~ProcessorTranscode () {}

      protected:

        std::string  m_filename;
        unsigned int m_track;
        std::string  m_device;
        int          m_quality;

    };

    ///////////////////
    
    class ProcessorFactory;
    /** This class is a processor to rip an audiocd track to an mp3 file
     *
     */
    class ProcessorCDDA_MP3
      : public ProcessorTranscode
    {
      public:
        /** ProcessorCDDA_MP3 ctor
         * @param path Destination path for the file
         * @param track CDDA Track Number
         */
        ProcessorCDDA_MP3 (std::string const&  filename,
                           unsigned int        track,
                           std::string const&  device,
                           int                 quality);

        virtual
        ~ProcessorCDDA_MP3 ();

      private:

        friend class ProcessorFactory;
        ProcessorCDDA_MP3 ();
        virtual void
        create_pipeline ();
    };

    /** This class is a processor to rip an audiocd track to an FLAC file
     *
     */
    class ProcessorCDDA_FLAC
      : public ProcessorTranscode
    {
      public:

        /** ProcessorCDDA_FLAC ctor
         * @param path Destination path for the file
         * @param track CDDA Track Number
         */
        ProcessorCDDA_FLAC (std::string const&  filename,
                            unsigned int        track,
                            std::string const&  device,
                            int                 quality);

        virtual
        ~ProcessorCDDA_FLAC ();

      private:

        friend class ProcessorFactory;
        ProcessorCDDA_FLAC ();
        virtual void
        create_pipeline ();
    };

    /** This class is a processor to rip an audiocd track to an Ogg Vorbis file
     *
     */
    class ProcessorCDDA_Vorbis
      : public ProcessorTranscode
    {
      public:

        /** ProcessorCDDA_Vorbis ctor
         * @param path Destination path for the file
         * @param track CDDA Track Number
         */
        ProcessorCDDA_Vorbis (std::string const& filename,
                              unsigned int       track,
                              std::string const& device,
                              int                quality);

        virtual
        ~ProcessorCDDA_Vorbis ();

      private:

        friend class ProcessorFactory;
        ProcessorCDDA_Vorbis ();
        virtual void
        create_pipeline ();
    };

    enum ProcessorType
    {
      PROC_MP3,
      PROC_VORBIS,
      PROC_FLAC,
      N_PROCS
    };

    class ProcessorFactory
    {
      private:

        ProcessorFactory () {}
      
      public:
  
        ~ProcessorFactory () {}

        static ProcessorBase*
        get_processor (ProcessorType      type,
                       std::string const& filename,
                       unsigned int       track,
                       std::string const& device,
                       int                quality);

        static bool
        test_processor (ProcessorType type);
    };
#endif // !MPX_PLUGIN_BUILD

  } // Audio namespace
} // MPX namespace

#endif //!MPX_AUDIO_HH
