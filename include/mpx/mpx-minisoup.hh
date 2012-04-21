//  MPX
//  Copyright (C) 2005-2007 MPX Project
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

#ifndef MPX_MINISOUP_HH
#define MPX_MINISOUP_HH

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif //HAVE_CONFIG_H
#include <libsoup/soup.h>
#include <glib.h>
#include <glibmm.h>
#include <iostream>
#include <fstream>

namespace MPX
{
  namespace Soup
  {
    typedef sigc::signal<void, char const* /* data */, guint /* size */, guint /* HTTP status code */> SigRequestCallback;
    typedef sigc::slot<void, char const* /* data */, guint /* size */, guint /* HTTP status code */> RequestSlot;

    typedef sigc::signal<void, double>              SigProgress;
    typedef sigc::signal<void, std::string const&>  SigFileDone;
    typedef sigc::signal<void, std::string const&>  SigFileAborted;


    class RequestSync;
    typedef Glib::RefPtr<RequestSync> RequestSyncRefP;

    class Request;
    typedef Glib::RefPtr<Request> RequestRefP;

    class RequestFile;
    typedef Glib::RefPtr<RequestFile> RequestFileRefP;


    class RequestSync
      : public Glib::Object
    {
        struct SignalT
        {
          SigProgress Progress;
        };

        SignalT Signals;

      public:

        static RequestSyncRefP create (std::string const& url, bool post = false);
        ~RequestSync ();

        void  add_header (std::string const& name,
                          std::string const& value); 
        void  add_request (std::string const& type,
                           std::string const& request);
        guint run ();
        std::string get_data ();
        void get_data (std::string & data);

        char const*
        get_data_raw ();
      
        guint
        get_data_size ();

        SigProgress & progress () { return Signals.Progress; }

      private:

        RequestSync (std::string const& url, bool post = false);

        std::string m_url;
        bool m_post;

        std::streamsize m_size;
        std::streamsize m_read;

        SoupSession * m_session;
        SoupMessage * m_message;

        static void
        restarted (SoupMessage* /*message*/, gpointer /*data*/);

        static void
        got_chunk (SoupMessage* /*message*/, gpointer /*data*/);

        static void
        got_content_length (SoupMessage* /*message*/, gpointer /*data*/);
    };


    class Request
      : public Glib::Object
    {
        protected:

            struct SignalsT
            {
              SigRequestCallback Callback;
            };

            SignalsT Signals;

        public:

            static RequestRefP create (std::string const& url, bool post = false);
            virtual ~Request ();
        
            virtual
            void  add_header (std::string const& name,
                              std::string const& value); 

            virtual
            void  add_request (std::string const& type,
                              std::string const& request);

            virtual
            void  run();

            virtual
            void  cancel();

            virtual
            guint status();

            virtual
            guint message_status();

            virtual
            char const*
            get_data_raw ();
         
            virtual 
            guint
            get_data_size ();

            SigRequestCallback & request_callback() { return Signals.Callback; }

        protected:

            Request (std::string const& url, bool post = false);

            bool        m_post;
            std::string m_url;

            SoupSession * m_session;
            SoupMessage * m_message;

            bool          m_block_reply;
            Glib::Mutex   m_message_lock;

            static void
            restarted (SoupMessage* /*message*/, gpointer /*data*/);

            static void
            got_answer (SoupMessage* /*message*/, gpointer /*data*/);

            static void
            got_body (SoupMessage* /*message*/, gpointer /*data*/);
    };


    class RequestFile
      : public Glib::Object
    {
        struct SignalsT
        {
          SigProgress    Progress;
          SigFileDone    Done;
          SigFileAborted Aborted;
        };
      
        SignalsT Signals;

      public:

        static RequestFileRefP create (std::string const& url, std::string const& filename); // XXX: Use MPX::FileIO object instead
        ~RequestFile ();

        void  add_header (std::string const& name,
                          std::string const& value); 
        void  run ();
        void  cancel ();

        SigProgress&
        file_progress ()  
        { return Signals.Progress; }

        SigFileDone&
        file_done()
        { return Signals.Done; }

        SigFileAborted&
        file_aborted()
        { return Signals.Aborted; }

      private:

        void  OpenFile ();
        void  CloseFile ();
        void  RemoveFile ();

        RequestFile (std::string const& url, std::string const& filename); 

        std::string m_url;
        std::string m_filename;

        std::ofstream m_file; 
        std::streamsize m_size;
        std::streamsize m_read;

        SoupSession * m_session;
        SoupMessage * m_message;

        bool          m_block_reply;
        Glib::Mutex   m_message_lock;

        static void
        restarted (SoupMessage* /*message*/, gpointer /*data*/);

        static void
        got_answer (SoupMessage* /*message*/, gpointer /*data*/);
                   
        static void
        got_chunk (SoupMessage* /*message*/, gpointer /*data*/);

        static void
        got_body (SoupMessage* /*message*/, gpointer /*data*/);

        static void
        got_content_length (SoupMessage* /*message*/, gpointer /*data*/);
    };
  };
}

#endif //!MPX_MINISOUP_HH
