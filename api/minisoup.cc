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
#include "config.h"
#include <cstring>
#include <glib/gstdio.h>
#include <glibmm/i18n.h>
#include <boost/format.hpp>

#include "mpx/mpx-minisoup.hh"
using namespace Glib;

namespace MPX
{
  namespace Soup
  {
    /////////////////////////////////////
    // Synchronous Request             //
    /////////////////////////////////////

    RequestSync::RequestSync (std::string const& url, bool post)
    : m_url   (url)
    , m_post  (post)
    {
      m_session = soup_session_sync_new ();
      m_message = soup_message_new (m_post ? "POST" : "GET", m_url.c_str());
    }
    
    Glib::RefPtr<RequestSync>
    RequestSync::create (std::string const& url, bool post)
    {
      return Glib::RefPtr<RequestSync>(new RequestSync (url, post));  
    }

    guint
    RequestSync::run ()
    {
      //g_signal_connect (G_OBJECT (m_message), "got-chunk", G_CALLBACK (got_chunk), this);
      //g_signal_connect (G_OBJECT (m_message), "restarted", G_CALLBACK (restarted), this);
      soup_message_add_header_handler (m_message, "got-headers", "content-length", 
                                       GCallback (got_content_length), this);
      return soup_session_send_message (m_session, m_message);
    }
  
    RequestSync::~RequestSync ()
    {
      g_object_unref (m_session);
    }


    std::string
    RequestSync::get_data ()
    {
      std::string buffer;
      buffer.append (m_message->response_body->data, m_message->response_body->length);
      return buffer;
    }

    void
    RequestSync::get_data (std::string & buffer)
    {
      buffer.append (m_message->response_body->data, m_message->response_body->length);
    }

    char const*
    RequestSync::get_data_raw ()
    {
      return m_message->response_body->data;
    }

    guint
    RequestSync::get_data_size ()
    {
      return m_message->response_body->length;
    }

    void
    RequestSync::add_header (std::string const& name,
                             std::string const& value) 
    {
      soup_message_headers_append (m_message->request_headers, name.c_str(), value.c_str());   
    }

    void
    RequestSync::add_request (std::string const& type,
                              std::string const& request)
    {
      soup_message_set_request (m_message, type.c_str(), SOUP_MEMORY_COPY /* we can't rely on the std::string not being destroyed */,
      g_strdup (const_cast<char*>(request.c_str())), strlen (request.c_str()));
      m_post = true;
    }


    void
    RequestSync::restarted (SoupMessage* message, gpointer data)
    {
      RequestSync & request = (*(reinterpret_cast<RequestSync*>(data)));

      request.m_read = 0;
      request.m_size = 0;
    }

    void
    RequestSync::got_chunk (SoupMessage* message, gpointer data)
    {
      RequestSync & request = (*(reinterpret_cast<RequestSync*>(data)));

      request.m_read += message->response_body->length;
      double percent = (double (request.m_read) / double (request.m_size));
      if(percent >= 0. && percent <= 1.)
      {
        request.Signals.Progress.emit (percent);
      }
    }

    void
    RequestSync::got_content_length (SoupMessage* message, gpointer data)
    {
      RequestSync & request = (*(reinterpret_cast<RequestSync*>(data)));
      request.m_size = g_ascii_strtoull (soup_message_headers_get (message->response_headers, "content-length"), NULL, 10);
    }


    /////////////////////////////////////
    // Default Request                 //
    /////////////////////////////////////

    Request::Request (std::string const& url, bool post)
    : m_post        (post)
    , m_url         (url)
    , m_block_reply (false)
    {
      m_session = soup_session_async_new ();
      m_message = soup_message_new (m_post ? "POST" : "GET", m_url.c_str());
    }

    Glib::RefPtr<Request>
    Request::create (std::string const& url, bool post)
    {
      g_message("Soup Request with URL: %s", url.c_str());
      return Glib::RefPtr<Request>(new Request (url, post));  
    }

    Request::~Request ()
    {
      m_message_lock.lock ();
      m_block_reply = true;
      m_message_lock.unlock ();

      cancel ();
      g_object_unref (m_session);
    }

    guint
    Request::status ()
    {
      return m_message->status_code;
    }

    guint
    Request::message_status ()
    {
      return m_message->status_code;
    }

    char const*
    Request::get_data_raw ()
    {
      return m_message->response_body->data;
    }

    guint
    Request::get_data_size ()
    {
      return m_message->response_body->length;
    }

    void
    Request::run ()
    {
      g_signal_connect (G_OBJECT (m_message), "restarted", G_CALLBACK (restarted), this);
      g_signal_connect (G_OBJECT (m_message), "got-body", G_CALLBACK (got_body), this);
      soup_session_queue_message (m_session, m_message, SoupSessionCallback (got_answer), this);
    }

    void
    Request::cancel ()
    {
      if (G_IS_OBJECT(m_session))
      {
            soup_session_abort (m_session);
            //soup_message_set_status (m_message, SOUP_STATUS_CANCELLED);
            //soup_session_cancel_message (m_session, m_message);
            //m_message = 0;
      }
    }


    void
    Request::add_header (std::string const& name,
                         std::string const& value) 
    {
      soup_message_headers_append (m_message->request_headers, name.c_str(), value.c_str());   
    }

    void
    Request::add_request (std::string const& type,
                          std::string const& request)
    {
      g_message("%s: Adding request: %s", G_STRLOC, request.c_str());
      soup_message_set_request (m_message, type.c_str(), SOUP_MEMORY_COPY /* we can't rely on the std::string not being destroyed */,
        g_strdup (const_cast<char*>(request.c_str())), strlen (request.c_str()));
      m_post = true;
    }


    void
    Request::restarted (SoupMessage* message, gpointer data)
    {
      /* nothing to do in case of async */
    }

    void
    Request::got_body (SoupMessage* message, gpointer data)
    {
      Request & request = (*(reinterpret_cast<Request*>(data)));

      request.m_message_lock.lock ();
      bool block = request.m_block_reply;
      request.m_message_lock.unlock ();

      if( block )
        return;

      request.Signals.Callback.emit(
                                      request.m_message->response_body->data, 
                                      request.m_message->response_body->length,
                                      request.m_message->status_code
                                    );
    }

    void
    Request::got_answer (SoupMessage* message, gpointer data)
    {
    }

    /////////////////////////////////////
    // File Download                   //
    /////////////////////////////////////

    RequestFile::RequestFile (std::string const& url, std::string const& filename) 
    : m_url           (url)
    , m_filename      (filename)
    , m_size          (0)
    , m_read          (0)
    {
      m_session = soup_session_async_new ();
      m_message = soup_message_new ("GET", m_url.c_str());
    }

    Glib::RefPtr<RequestFile>
    RequestFile::create (std::string const& url, std::string const& filename)
    {
      return Glib::RefPtr<RequestFile>(new RequestFile (url, filename));  
    }

    RequestFile::~RequestFile ()
    {
      if (G_IS_OBJECT(m_message)) 
      {
            cancel ();
            Signals.Aborted.emit ((boost::format (_("Download of file '%s' was cancelled prematurely!")) % filename_to_utf8 (m_filename).c_str()).str());
      }
      g_object_unref (m_session);
    }

    void
    RequestFile::CloseFile ()
    {
      if (m_file.is_open())
          m_file.close ();
    }

    void
    RequestFile::RemoveFile ()
    {
      g_unlink (m_filename.c_str());
    }

    void
    RequestFile::OpenFile ()
    {
      m_file.open (m_filename.c_str());
      if (!m_file.good())
      {
        cancel (); 
        Signals.Aborted.emit ((boost::format (_("Target file ('%s') couldn't be opened for writing")) % filename_to_utf8 (m_filename).c_str()).str());
      }
    }

    void
    RequestFile::run ()
    {
      g_signal_connect (G_OBJECT (m_message), "got-chunk", G_CALLBACK (got_chunk), this);
      g_signal_connect (G_OBJECT (m_message), "got-body", G_CALLBACK (got_body), this);
      g_signal_connect (G_OBJECT (m_message), "restarted", G_CALLBACK (restarted), this);
      soup_message_add_header_handler (m_message, "got-headers", "content-length", 
                                       GCallback (got_content_length), this);
      soup_session_queue_message (m_session, m_message, SoupSessionCallback (got_answer), this);
      OpenFile ();
    }

    void
    RequestFile::cancel ()
    {
      if (G_IS_OBJECT(m_message))
      {
            soup_message_set_status (m_message, SOUP_STATUS_CANCELLED);
            soup_session_cancel_message (m_session, m_message, 400);
            m_message = 0;
      }
      
      CloseFile ();
      RemoveFile ();
    }

    void
    RequestFile::add_header (std::string const& name,
                             std::string const& value) 
    {
      soup_message_headers_append (m_message->request_headers, name.c_str(), value.c_str());   
    }


    void
    RequestFile::restarted (SoupMessage* message, gpointer data)
    {
      RequestFile & request = (*(reinterpret_cast<RequestFile*>(data)));
      request.CloseFile ();
      request.OpenFile ();
    }

    void
    RequestFile::got_body (SoupMessage* message, gpointer data)
    {
      RequestFile & request = (*(reinterpret_cast<RequestFile*>(data)));
      
      if( request.m_message->status_code >= 400)
      {
        guint status_code = request.m_message->status_code;
        request.cancel (); 
        request.Signals.Aborted.emit ((boost::format (_("%u: %s")) % status_code % soup_status_get_phrase (status_code)).str());
        return;
      }

      if( request.m_message->status_code == SOUP_STATUS_CANCELLED)
        return;

      request.CloseFile ();
      request.Signals.Done.emit (request.m_filename);
    }

    void
    RequestFile::got_answer (SoupMessage* message, gpointer data)
    {
    }

    void
    RequestFile::got_chunk (SoupMessage* message, gpointer data)
    {
      RequestFile & request = (*(reinterpret_cast<RequestFile*>(data)));

      request.m_file.write (message->response_body->data, message->response_body->length);
      request.m_read += message->response_body->length;

      double percent = (double (request.m_read) / double (request.m_size));
      if(percent >= 0. && percent <= 1.)
      {
        request.Signals.Progress.emit (percent);
      }
    }

    void
    RequestFile::got_content_length (SoupMessage* message, gpointer data)
    {
      RequestFile & request = (*(reinterpret_cast<RequestFile*>(data)));
      request.m_size = g_ascii_strtoull (soup_message_headers_get (message->response_headers, "content-length"), NULL, 10);
    }
  }
}
