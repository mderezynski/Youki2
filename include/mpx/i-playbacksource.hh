//  MPX
//  Copyright (C) 2005-2007 MPX development.
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
#ifndef MPX_PLAYBACK_SOURCE_HH
#define MPX_PLAYBACK_SOURCE_HH
#include "config.h"
#include <glib/ghash.h>
#include <glibmm/ustring.h>
#include <gdkmm/pixbuf.h>
#include <gtkmm/widget.h>
#include <gtkmm/uimanager.h>
#include <sigc++/signal.h>
#include <boost/format.hpp>
#include <Python.h>
#include <boost/python.hpp>
#include <vector>

#define NO_IMPORT
#include <pygobject.h>

#include "mpx/mpx-types.hh"
#include "mpx/util-file.hh"

namespace MPX
{
	typedef std::vector<std::string> UriSchemes;

    struct Metadata
    : public Track
    {
        Glib::RefPtr<Gdk::Pixbuf> Image;

        Glib::RefPtr<Gdk::Pixbuf>
		get_image ()
		{
            return Image;
		}

		Metadata ()
		{
		}

		Metadata (Track const& track)
		: Track (track)
		{
		}
    };

    enum Flags
    {
      F_NONE                    = 0,
      F_ASYNC                   = 1 << 0,
      F_HANDLE_STREAMINFO       = 1 << 1,
      F_PHONY_NEXT              = 1 << 2,
      F_PHONY_PREV              = 1 << 3,
      F_HANDLE_LASTFM           = 1 << 5,
      F_HANDLE_LASTFM_ACTIONS   = 1 << 6,
      F_USES_REPEAT             = 1 << 7,
      F_USES_SHUFFLE            = 1 << 8,
    };

    enum Caps
    {
      C_NONE                    = 0,
      C_CAN_GO_NEXT             = 1 << 0,
      C_CAN_GO_PREV             = 1 << 1,
      C_CAN_PAUSE               = 1 << 2,
      C_CAN_PLAY                = 1 << 3,
      C_CAN_STOP                = 1 << 4, // this is really only used in the SourceController
      C_CAN_SEEK                = 1 << 5,
      C_CAN_RESTORE_CONTEXT     = 1 << 6, 
      C_CAN_PROVIDE_METADATA    = 1 << 7,
      C_CAN_BOOKMARK            = 1 << 8,
      C_PROVIDES_TIMING         = 1 << 9,
    };

    typedef std::pair<boost::optional<gint64>, gint64> ItemKey;
    typedef std::map<ItemKey, Flags>                   FlagsMap_t;
    typedef std::map<ItemKey, Caps>                    CapsMap_t; 

    class Player;
    class PlaybackSource
    {
        public:

            typedef sigc::signal<void, Caps>                    SignalCaps;
            typedef sigc::signal<void, Flags>                   SignalFlags;
            typedef sigc::signal<void, const Metadata&>         SignalTrackMetadata;
            typedef sigc::signal<void>                          SignalSegment;
            typedef sigc::signal<void>                          SignalPlaybackRequest;
            typedef sigc::signal<void>                          SignalStopRequest;
            typedef sigc::signal<void>                          SignalPlayAsync;
            typedef sigc::signal<void>                          SignalNextAsync;
            typedef sigc::signal<void>                          SignalPrevAsync;
            typedef sigc::signal<void>                          SignalNameChanged;
            typedef sigc::signal<void, gint64>                  SignalItems;

            PlaybackSource(
                const Glib::RefPtr<Gtk::UIManager>&,
                const std::string&,
                Caps = C_NONE,
                Flags = F_NONE
            );

            virtual ~PlaybackSource ();

            SignalCaps&
            signal_caps ();

            SignalFlags&
            signal_flags ();

            SignalTrackMetadata&
            signal_track_metadata ();

            SignalPlaybackRequest&
            signal_playback_request ();

            SignalStopRequest&
            signal_stop_request ();

            SignalSegment&
            signal_segment ();

            SignalNextAsync&
            signal_next_async ();

            SignalPrevAsync&
            signal_prev_async ();

            SignalPlayAsync&
            signal_play_async ();

            SignalNameChanged&
            signal_name_changed ();

            SignalItems&
            signal_items ();

        public:
            virtual std::string
            get_uri () = 0; 
        
            virtual std::string
            get_type ();

            virtual bool
            play () = 0;

            virtual bool
            go_next () = 0;

            virtual bool
            go_prev () = 0; 

            virtual void
            stop () = 0;
      
            virtual void
            play_async ();

            virtual void
            go_next_async ();
      
            virtual void
            go_prev_async (); 

            virtual void
            play_post () = 0;

            virtual void
            next_post (); 

            virtual void
            prev_post ();

            virtual void
            restore_context () = 0;

            virtual void
            skipped (); 

            virtual void
            segment (); 

            virtual void
            buffering_done (); 

            virtual void
            send_caps ();

            virtual void
            send_flags ();

            virtual void
            send_metadata () = 0;

            std::string
            get_name ();

            virtual PyObject*
            get_py_obj ();

            virtual std::string
            get_guid () = 0;

            virtual std::string
            get_class_guid () = 0;

            virtual Glib::RefPtr<Gdk::Pixbuf>
            get_icon () = 0;

            virtual Gtk::Widget*
            get_ui () = 0;

            virtual guint
            add_menu ();

            virtual UriSchemes 
            get_uri_schemes ();

            virtual void    
            process_uri_list (Util::FileList const&, bool play);

            virtual void
            drag_data (const Glib::RefPtr<Gdk::DragContext>&,
                       const Gtk::SelectionData&, guint, guint);

            void
            set_name(std::string const&);

            ItemKey const& //nonvirtual by design
            get_key ();

            virtual void
            post_install ();

            void
            add_cap (Caps);

            void
            rem_cap (Caps);

        private:

            friend class ::MPX::Player;

            void
            set_key (ItemKey const& key);

            ItemKey m_OwnKey;

        public:

            struct SignalsT
            {
              SignalCaps                      Caps;
              SignalFlags                     Flags;
              SignalTrackMetadata             Metadata;
              SignalSegment                   Segment;
              SignalPlaybackRequest           PlayRequest;
              SignalStopRequest               StopRequest;
              SignalPlayAsync                 PlayAsync;
              SignalNextAsync                 NextAsync;
              SignalPrevAsync                 PrevAsync;
              SignalNameChanged               NameChanged;
              SignalItems                     Items;
            };

            SignalsT        Signals;

        protected:

            Caps            m_Caps;
            Flags           m_Flags;
            std::string     m_Name;

    }; // end class PlaybackSource 

    inline Caps operator|(Caps lhs, Caps rhs)
      { return static_cast<Caps>(static_cast<unsigned>(lhs) | static_cast<unsigned>(rhs)); }

    inline Flags operator|(Flags lhs, Flags rhs)
      { return static_cast<Flags>(static_cast<unsigned>(lhs) | static_cast<unsigned>(rhs)); }

} // end namespace MPX 
  
#endif
