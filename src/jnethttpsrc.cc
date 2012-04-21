/* vim: set sw=2: -*- Mode: C; tab-width: 2; indent-tabs-mode: t; c-basic-offset: 2; c-indent-level: 2 -*- */
/* GStreamer
 * Copyright (C) <2005> Edgard Lima <edgard.lima@indt.org.br>
 * Copyright (C) <2010> Milosz Derezynski <internalerror@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more 
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif //HAVE_CONFIG_H

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <glibmm.h>

#include "jnetlib/connection.h"
#include "jnetlib/httpget.h"
#include "jnetlib/util.h"
#include "jnethttpsrc.h"
#include "mpx/mpx-uri.hh"

using namespace Glib;

#define HTTP_DEFAULT_HOST        "localhost"
#define HTTP_DEFAULT_PORT        80
#define HTTPS_DEFAULT_PORT       443

GST_DEBUG_CATEGORY_STATIC (jnlhttpsrc_debug);
#define GST_CAT_DEFAULT jnlhttpsrc_debug

#define BUFFER_SIZE           (2U * 1024U)
#define BUFFER_SIZE_FIRST     (32U * 1024U)

#define RET_NO_DATA_READ              -2
#define RET_UNABLE_TO_RETRY           -3
#define RET_CONTENT_MAX_SIZE_REACHED  -4


static const GstElementDetails gst_jnethttpsrc_details =
GST_ELEMENT_DETAILS ("HTTP client source",
    "Source/Network",
    "Receive data as a client over the network via HTTP using JNET",
    "Edgard Lima <edgard.lima@indt.org.br>, Milosz Derezynski <internalerror@gmail.com>");

static GstStaticPadTemplate srctemplate = GST_STATIC_PAD_TEMPLATE ("src", GST_PAD_SRC, GST_PAD_ALWAYS, GST_STATIC_CAPS_ANY);

static gboolean
get_abort (MPXJNLHttpSrc * src)
{
  g_mutex_lock (src->abort_lock);
  bool abort = src->abort;
  g_mutex_unlock (src->abort_lock);
  return abort;
}

static void
set_abort (MPXJNLHttpSrc * src, bool abort)
{
  g_mutex_lock (src->abort_lock);
  src->abort = abort; 
  g_mutex_unlock (src->abort_lock);
}

enum
{
  PROP_0,
  PROP_PREBUFFER,
  PROP_LOCATION,
  PROP_CUSTOMHEADER,
  PROP_URI,
  PROP_USER_AGENT,
  PROP_IRADIO_MODE,
  PROP_ABORT,
};

static void           gst_jnethttpsrc_finalize (GObject * gobject);
static GstFlowReturn  gst_jnethttpsrc_create (GstPushSrc * psrc, GstBuffer ** outbuf);
static gboolean       gst_jnethttpsrc_start (GstBaseSrc * bsrc);
static gboolean       gst_jnethttpsrc_stop (GstBaseSrc * bsrc);
static gboolean       gst_jnethttpsrc_get_size (GstBaseSrc * bsrc, guint64 * size);
static void           gst_jnethttpsrc_set_property (GObject * object, guint prop_id, const GValue * value, GParamSpec * pspec);
static void           gst_jnethttpsrc_get_property (GObject * object, guint prop_id, GValue * value, GParamSpec * pspec);
static void           gst_jnethttpsrc_uri_handler_init (gpointer g_iface, gpointer iface_data);
static gboolean       gst_jnethttpsrc_check_get_range (GstBaseSrc * bsrc);
 
namespace
{
  void
  _urihandler_init (GType type)
  {
    static const GInterfaceInfo urihandler_info = {
      gst_jnethttpsrc_uri_handler_init,
      NULL,
      NULL
    };

    g_type_add_interface_static (type, GST_TYPE_URI_HANDLER, &urihandler_info);
    GST_DEBUG_CATEGORY_INIT (jnlhttpsrc_debug, "jnethttpsrc", 0, "MPX JNET HTTP src");
  }
}

    
GST_BOILERPLATE_FULL (MPXJNLHttpSrc, gst_jnethttpsrc, GstPushSrc, GST_TYPE_PUSH_SRC, _urihandler_init)

static void
gst_jnethttpsrc_base_init (gpointer g_class)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (g_class);
  gst_element_class_add_pad_template (element_class, gst_static_pad_template_get (&srctemplate));
  gst_element_class_set_details (element_class, &gst_jnethttpsrc_details);
}

static void
gst_jnethttpsrc_class_init (MPXJNLHttpSrcClass * klass)
{
  GObjectClass    *gobject_class;
  GstBaseSrcClass *gstbasesrc_class;
  GstPushSrcClass *gstpush_src_class;

  gobject_class = (GObjectClass *) klass;
  gstbasesrc_class = (GstBaseSrcClass *) klass;
  gstpush_src_class = (GstPushSrcClass *) klass;

  gobject_class->set_property = gst_jnethttpsrc_set_property;
  gobject_class->get_property = gst_jnethttpsrc_get_property;
  gobject_class->finalize = gst_jnethttpsrc_finalize;

  g_object_class_install_property
      (gobject_class, PROP_LOCATION,
      g_param_spec_string ("location", "Location",
          "The location. In the form:"
          "\n\t\t\thttp://a.com/file.txt - default port '80' "
          "\n\t\t\thttp://a.com:80/file.txt "
          "\n\t\t\ta.com/file.txt - defualt scheme 'HTTP' "
          "\n\t\t\thttps://a.com/file.txt - default port '443' "
          "\n\t\t\thttp:///file.txt - default host '" HTTP_DEFAULT_HOST "'",
          "", GParamFlags (G_PARAM_READWRITE)));

  g_object_class_install_property
      (gobject_class, PROP_CUSTOMHEADER,
      g_param_spec_string ("customheader", "Custom HTTP header",
          "Custom HTTP header",
          "", GParamFlags (G_PARAM_READWRITE)));

  g_object_class_install_property
      (gobject_class, PROP_URI,
      g_param_spec_string ("uri", "Uri",
          "The location in form of a URI (deprecated; use location)",
          "", GParamFlags (G_PARAM_READWRITE)));

  g_object_class_install_property
      (gobject_class, PROP_USER_AGENT,
      g_param_spec_string ("user-agent", "User-Agent",
          "The User-Agent used for connection.",
          "jnlhttpsrc", GParamFlags (G_PARAM_READWRITE)));

  g_object_class_install_property
      (gobject_class, PROP_ABORT,
      g_param_spec_boolean ("abort", "abort",
          "abort streaming",
          FALSE, GParamFlags (G_PARAM_READWRITE)));

  g_object_class_install_property
      (gobject_class, PROP_IRADIO_MODE,
      g_param_spec_boolean ("iradio-mode", "iradio-mode",
          "Enable internet radio mode (extraction of shoutcast/icecast metadata)",
          FALSE, GParamFlags (G_PARAM_READWRITE)));

  g_object_class_install_property
      (gobject_class, PROP_PREBUFFER,
      g_param_spec_boolean ("prebuffer", "prebuffer",
          "Resets prebuffer mode",
          TRUE, GParamFlags (G_PARAM_READWRITE)));

  gstbasesrc_class->start = gst_jnethttpsrc_start;
  gstbasesrc_class->check_get_range = gst_jnethttpsrc_check_get_range;
  gstbasesrc_class->stop = gst_jnethttpsrc_stop;
  gstbasesrc_class->get_size = gst_jnethttpsrc_get_size;
  gstpush_src_class->create = gst_jnethttpsrc_create;

  GST_DEBUG_CATEGORY_INIT (jnlhttpsrc_debug, "jnethttpsrc", 0, "JNET HTTP Client Source");
     
}

static void
gst_jnethttpsrc_init (MPXJNLHttpSrc * jnlhttpsrc, MPXJNLHttpSrcClass * g_class)
{
  jnlhttpsrc->get = 0;
  jnlhttpsrc->content_size = -1;
  jnlhttpsrc->content_read = 0;
  jnlhttpsrc->iradio_mode = FALSE;
  jnlhttpsrc->icy_caps = NULL;
  jnlhttpsrc->icy_metaint = 0;
  jnlhttpsrc->first_buffering = TRUE;
  jnlhttpsrc->eos = FALSE;
  jnlhttpsrc->abort = FALSE;
  jnlhttpsrc->abort_lock = g_mutex_new();
  jnlhttpsrc->uri = NULL;
  jnlhttpsrc->customheader = NULL;
}

static void
gst_jnethttpsrc_finalize (GObject * gobject)
{
  MPXJNLHttpSrc *jnlhttpsrc = MPX_JNETHTTP_SRC (gobject);

  delete jnlhttpsrc->get;
  jnlhttpsrc->get = 0;

  if (jnlhttpsrc->icy_caps)
  {
    gst_caps_unref (jnlhttpsrc->icy_caps);
    jnlhttpsrc->icy_caps = NULL;
  }

  g_mutex_free (jnlhttpsrc->abort_lock);

  G_OBJECT_CLASS (parent_class)->finalize (gobject);
}

namespace
{
  int
  request_dispatch (MPXJNLHttpSrc * src, GstBuffer ** outbuf)
  {
    int   read        = 0;
    int   sizetoread  = src->first_buffering ? BUFFER_SIZE_FIRST : BUFFER_SIZE; 
    int   dest_size   = sizetoread;

    if (G_UNLIKELY(src->first_buffering))
    {
          double percentage = 0.; 
          GstStructure *structure = gst_structure_new ("buffering", "size", G_TYPE_DOUBLE, percentage, NULL);
          GstMessage *message = gst_message_new_application (GST_OBJECT (src), structure);
          gst_element_post_message (GST_ELEMENT (src), message);
    }

    ssize_t len = 0; 

    while ((!get_abort (src)) && (sizetoread > 0))
    {
          int result = src->get->run();
          len = src->get->get_bytes (reinterpret_cast<char*> (GST_BUFFER_DATA (*outbuf)) + read, sizetoread);


          if (result == -1 || result == 1)
          {
                read = RET_CONTENT_MAX_SIZE_REACHED;
                return read;
          }


          if (len > 0)
          {
                if (G_UNLIKELY(src->first_buffering))
                {
                      double percentage = double(read)/double(BUFFER_SIZE_FIRST);
                      GstStructure *structure = gst_structure_new ("buffering", "size", G_TYPE_DOUBLE, percentage, NULL);
                      GstMessage *message = gst_message_new_application (GST_OBJECT (src), structure);
                      gst_element_post_message (GST_ELEMENT (src), message);
                }

                read += len;
                sizetoread -= len;
          }
    }

    if (get_abort (src))
    {
          return RET_CONTENT_MAX_SIZE_REACHED;
    }

    GST_BUFFER_SIZE (*outbuf) = read;

    if (read < dest_size)
    {
          g_log (G_LOG_DOMAIN, G_LOG_LEVEL_INFO, "%s: Read only %d bytes, wanted %d", G_STRFUNC, read, dest_size);
    }

    src->content_read += read; 
    if (G_UNLIKELY(src->content_size != (-1)) && (src->content_read > src->content_size))
    {
          return RET_CONTENT_MAX_SIZE_REACHED;
    }

    if (G_UNLIKELY(src->first_buffering))
    {
          double percentage = 1.; 

          GstStructure *structure = 0; 
          GstMessage *message = 0; 

          structure = gst_structure_new ("buffering", "size", G_TYPE_DOUBLE, percentage, NULL);
          message = gst_message_new_application (GST_OBJECT (src), structure);
          gst_element_post_message (GST_ELEMENT (src), message);

          structure = gst_structure_new ("buffering-done", "done", G_TYPE_BOOLEAN, TRUE, NULL);
          message = gst_message_new_application (GST_OBJECT (src), structure);
          gst_element_post_message (GST_ELEMENT (src), message);
          src->first_buffering = FALSE;
    }

    return read;
  }
}

static GstFlowReturn
gst_jnethttpsrc_create (GstPushSrc * psrc, GstBuffer ** outbuf)
{
  MPXJNLHttpSrc *src;
  GstFlowReturn ret;
  int read;

  src = MPX_JNETHTTP_SRC (psrc);

  if (G_UNLIKELY (src->eos))
  {
        goto eos;
  }
   
  ret = gst_pad_alloc_buffer (GST_BASE_SRC_PAD (GST_BASE_SRC (psrc)),
                              GST_BUFFER_OFFSET_NONE,
                              src->first_buffering ? BUFFER_SIZE_FIRST : BUFFER_SIZE, 
                              src->icy_caps ? src->icy_caps :
                              GST_PAD_CAPS (GST_BASE_SRC_PAD (GST_BASE_SRC (psrc))), outbuf);

  if (G_UNLIKELY (ret != GST_FLOW_OK))
  {
        return ret;
  }

  read = request_dispatch (src, outbuf);

  if (G_UNLIKELY (read < 0))
  {
        switch (read)
        {
              case RET_NO_DATA_READ:
                goto read_error;
                break;

              case RET_UNABLE_TO_RETRY:
                goto read_error;
                break;
      
              case RET_CONTENT_MAX_SIZE_REACHED:
                goto eos;
                break;
        }
  }
  else
  {
        return ret;
  }

  eos:
  {
        src->eos = TRUE;
        GST_DEBUG_OBJECT (src, "EOS reached");
        return GST_FLOW_UNEXPECTED;
  }

  read_error:
  {
        return GST_FLOW_ERROR;
  }

}

static int
send_request_and_redirect (MPXJNLHttpSrc * src)
{
  int result;

  src->get = new JNL_HTTPGet();
  src->get->addheader ("User-Agent: Last.fm Client 1.4.2.58240 (X11)");
  src->get->addheader ("icy-metadata: 1");
  src->get->addheader ("Accept: audio/mpeg, */*");
  if(src->customheader)
  {
    g_message("%s: Adding custom header: '%s'", G_STRLOC, src->customheader);
    src->get->addheader(src->customheader);
  }

  src->get->connect(src->uri, 1);

  Glib::Timer timer;

  while (1)
  {
    if (timer.elapsed() > 5)
    {
      return RET_NO_DATA_READ;
    }

    result = src->get->run(); 

    if (result < 0)
    {
      return RET_NO_DATA_READ;
    } 

    if (src->get->get_status() == 2) 
    {
      break;
    }
  }

  return result;
}

static gboolean
gst_jnethttpsrc_check_get_range (GstBaseSrc * bsrc)
{
  return FALSE;
}

static gboolean
gst_jnethttpsrc_start (GstBaseSrc * bsrc)
{
  MPXJNLHttpSrc *src = MPX_JNETHTTP_SRC (bsrc);

  int result = send_request_and_redirect (src);

  if (result != 0) 
  {
        GST_ELEMENT_ERROR (src, LIBRARY, INIT, (NULL), ("Could not begin request (%d)", result));
        return FALSE;
  }

  src->content_size = src->get->content_length();

  if (src->iradio_mode)
  {
        char *str_value;
        gint  int_value;

        str_value = src->get->getheader ("icy-metaint");
        if (str_value)
        {
              if (sscanf (str_value, "%d", &int_value) == 1)
              {
                    if (src->icy_caps)
                    {
                          gst_caps_unref (src->icy_caps);
                          src->icy_caps = NULL;
                    }
                    src->icy_metaint = int_value;
                    src->icy_caps = gst_caps_new_simple ("application/x-icy", "metadata-interval", G_TYPE_INT, src->icy_metaint, NULL);
              }
        }

        GstTagList * taglist = gst_tag_list_new ();

        str_value = src->get->getheader ("icy-bitrate");
        if (str_value)
        {
              gst_tag_list_add (taglist, GST_TAG_MERGE_REPLACE, GST_TAG_BITRATE, guint (g_ascii_strtoull (str_value, NULL, 10)), NULL);
        }

        str_value = src->get->getheader ("icy-br");
        if (str_value)
        {
              gst_tag_list_add (taglist, GST_TAG_MERGE_REPLACE, GST_TAG_BITRATE, guint (g_ascii_strtoull (str_value, NULL, 10)), NULL);
        }

        str_value = src->get->getheader ("icy-genre");
        if (str_value)
        {
              gst_tag_list_add (taglist, GST_TAG_MERGE_REPLACE, GST_TAG_ALBUM, str_value, NULL);
        }

        if (!gst_tag_list_is_empty (taglist))
              gst_element_found_tags_for_pad (GST_ELEMENT (src), GST_BASE_SRC_PAD (src), taglist);
        else
              gst_tag_list_free (taglist);
  }

  return TRUE;
}

static gboolean
gst_jnethttpsrc_get_size (GstBaseSrc * bsrc, guint64 * size)
{
  MPXJNLHttpSrc *src (MPX_JNETHTTP_SRC (bsrc));
  if (src->content_size == -1)
    return FALSE;

  *size = src->content_size;
  return TRUE;
}

static gboolean
gst_jnethttpsrc_stop (GstBaseSrc * bsrc)
{
  MPXJNLHttpSrc *src;
  src = MPX_JNETHTTP_SRC (bsrc);

  set_abort (src, true);

  src->first_buffering = TRUE;
  src->content_size = -1;
  src->content_read =  0;

  if (src->icy_caps)
  {
    gst_caps_unref (src->icy_caps);
    src->icy_caps = NULL;
  }

  delete src->get;
  src->get = 0;

  src->eos = FALSE;
  set_abort (src, false);

  return TRUE;
}

static void
gst_jnethttpsrc_set_property (GObject * object, guint prop_id, const GValue * value, GParamSpec * pspec)
{
  MPXJNLHttpSrc *jnlhttpsrc = MPX_JNETHTTP_SRC (object);

  switch (prop_id)
  {

    case PROP_ABORT:
    {
      jnlhttpsrc->abort = g_value_get_boolean (value);
      break;
    }

    case PROP_PREBUFFER:
    {
      jnlhttpsrc->first_buffering = g_value_get_boolean (value);
      break;
    }

    case PROP_CUSTOMHEADER:
    {
      if(jnlhttpsrc->customheader)
        g_free(jnlhttpsrc->customheader);

      jnlhttpsrc->customheader = g_value_dup_string(value);
      break;
    }

    case PROP_URI:
    case PROP_LOCATION:
    {
      if (!g_value_get_string (value))
      {
        GST_WARNING ("location property cannot be NULL");
        return;
      }

      if(jnlhttpsrc->uri)
        g_free(jnlhttpsrc->uri);

      jnlhttpsrc->uri = g_value_dup_string (value);
      break;
    }

    case PROP_USER_AGENT:
    {
      break;
    }

    case PROP_IRADIO_MODE:
    {
      jnlhttpsrc->iradio_mode = g_value_get_boolean (value);
      break;
    }

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_jnethttpsrc_get_property (GObject * object, guint prop_id, GValue * value, GParamSpec * pspec)
{
  MPXJNLHttpSrc *jnlhttpsrc = MPX_JNETHTTP_SRC (object);

  switch (prop_id)
  {
    case PROP_PREBUFFER:
    {
      g_value_set_boolean (value, jnlhttpsrc->first_buffering);
      break;
    }

    case PROP_CUSTOMHEADER:
    {
      g_value_set_string (value, jnlhttpsrc->customheader);
      break;
    }

    case PROP_URI:
    case PROP_LOCATION:
    {
      g_value_set_string (value, jnlhttpsrc->uri);
      break;
    }

    case PROP_USER_AGENT:
    {
      g_value_set_string (value, ""); 
      break;
    }

    case PROP_ABORT:
    {
      g_value_set_boolean (value, jnlhttpsrc->abort);
      break;
    }

    case PROP_IRADIO_MODE:
    {
      g_value_set_boolean (value, jnlhttpsrc->iradio_mode);
      break;
    }

    default:
    {
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
  }
}

static gboolean
plugin_init (GstPlugin * plugin)
{
  return gst_element_register (plugin, "jnethttpsrc", GST_RANK_NONE, GST_TYPE_JNETHTTP_SRC);
}

GST_PLUGIN_DEFINE_STATIC (GST_VERSION_MAJOR,
                          GST_VERSION_MINOR,
                          "jnethttpsrc",
                          "jnethttp client src",
                           plugin_init, PACKAGE_VERSION, "LGPL", "MPX", "http://mpx.backtrace.info")


/*** GSTURIHANDLER INTERFACE *************************************************/
static GstURIType gst_jnethttpsrc_uri_get_type (void)
{
  return GST_URI_SRC;
}
static gchar **
gst_jnethttpsrc_uri_get_protocols (void)
{
  static gchar *protocols[] = { "http", NULL };
  return protocols;
}

static const gchar *
gst_jnethttpsrc_uri_get_uri (GstURIHandler * handler)
{
  MPXJNLHttpSrc *src = MPX_JNETHTTP_SRC (handler);

  return src->uri;
}

static gboolean
gst_jnethttpsrc_uri_set_uri (GstURIHandler * handler, const gchar * uri)
{
  MPXJNLHttpSrc *src = MPX_JNETHTTP_SRC (handler);
  src->uri = g_strdup (uri);
  return TRUE;
}

static void
gst_jnethttpsrc_uri_handler_init (gpointer g_iface, gpointer iface_data)
{
  GstURIHandlerInterface *iface = (GstURIHandlerInterface *) g_iface;

  iface->get_type = gst_jnethttpsrc_uri_get_type;
  iface->get_protocols = gst_jnethttpsrc_uri_get_protocols;
  iface->get_uri = gst_jnethttpsrc_uri_get_uri;
  iface->set_uri = gst_jnethttpsrc_uri_set_uri;
}



