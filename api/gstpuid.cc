/*
 * gstpuid.c:
 * 
 * Copyright (C) 2006-2007 M. Derezynski based on:
 *
 * gstidentity.c:
 *
 * GStreamer
 * Copyright (C) 1999,2000 Erik Walthinsen <omega@cse.ogi.edu>
 *                    2000 Wim Taymans <wtay@chello.be>
 *                    2005-2007 Wim Taymans <wim@fluendo.com>
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */


#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "mpx/aux/gstpuid.h"

#include <stdlib.h>
#include <string.h>
#include <ofa1/ofa.h>
#include <glib.h>
#include <glib/gi18n.h> 
#include <gst/gstmarshal.h>
#include "mpx/mpx-minisoup.hh"

using namespace MPX;

static GstStaticPadTemplate sinktemplate = GST_STATIC_PAD_TEMPLATE ("sink",
                                                                    GST_PAD_SINK,
                                                                    GST_PAD_ALWAYS, 
                                                                    GST_STATIC_CAPS ("audio/x-raw-int, "
                                                                                     "rate = (int) [ 1, MAX ], "
                                                                                     "channels = (int) [ 1, 2 ], "
                                                                                     "endianness = (int) BYTE_ORDER, "
                                                                                     "width = (int) { 16 }, "
                                                                                     "depth = (int) { 16 }, "
                                                                                     "signed = (boolean) true"));

static GstStaticPadTemplate srctemplate  = GST_STATIC_PAD_TEMPLATE ("src",
                                                                    GST_PAD_SRC,
                                                                    GST_PAD_ALWAYS,
                                                                    GST_STATIC_CAPS_ANY);
                                                                   
GST_DEBUG_CATEGORY_STATIC (gst_puid_debug);
#define GST_CAT_DEFAULT gst_puid_debug

static const GstElementDetails gst_puid_details =
GST_ELEMENT_DETAILS ("PUID", "MusicIP PUID Fingerprinting element", "MusicIP's fingerprinting using libofa", "Milosz Derezynski <internalerror@gmail.com>");
                    
#define DEFAULT_SLEEP_TIME              0
#define DEFAULT_SYNC                    FALSE
#define DEFAULT_CHECK_PERFECT           FALSE

enum
{
  PROP_0,
  PROP_SYNC,
  PROP_CHECK_PERFECT,
  PROP_MUSICDNS_ID,
  PROP_XMLDOC,
  PROP_ARTIST,
  PROP_TITLE,
  PROP_PUID,
};


#define _do_init(bla) \
    GST_DEBUG_CATEGORY_INIT (gst_puid_debug, "puid", 0, "puid element");

GST_BOILERPLATE_FULL (GstPUID, gst_puid, GstBaseTransform, GST_TYPE_BASE_TRANSFORM, _do_init);

static void             gst_puid_finalize     (GObject * object);
static void             gst_puid_set_property (GObject * object, guint prop_id, const GValue * value, GParamSpec * pspec);
static void             gst_puid_get_property (GObject * object, guint prop_id, GValue * value, GParamSpec * pspec);
static GstFlowReturn    gst_puid_transform_ip (GstBaseTransform * trans, GstBuffer * buf);
static gboolean         gst_puid_start        (GstBaseTransform * trans);
static gboolean         gst_puid_stop         (GstBaseTransform * trans);

static void
gst_puid_base_init (gpointer g_class)
{
  GstElementClass *gstelement_class = GST_ELEMENT_CLASS (g_class);

  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&srctemplate));

  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&sinktemplate));

  gst_element_class_set_details (gstelement_class, &gst_puid_details);
}

static void
gst_puid_finalize (GObject * object)
{
  GstPUID *puid;

  puid = GST_PUID (object);

  g_free (puid->xmldoc);
  g_free (puid->artist);
  g_free (puid->title);
  g_free (puid->puid);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gst_puid_class_init (GstPUIDClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;
  GstBaseTransformClass *gstbasetrans_class;

  gobject_class       = G_OBJECT_CLASS (klass);
  gstelement_class    = GST_ELEMENT_CLASS (klass);
  gstbasetrans_class  = GST_BASE_TRANSFORM_CLASS (klass);

  gobject_class->set_property = GST_DEBUG_FUNCPTR (gst_puid_set_property);
  gobject_class->get_property = GST_DEBUG_FUNCPTR (gst_puid_get_property);

  g_object_class_install_property (gobject_class, PROP_SYNC,
      g_param_spec_boolean ("sync", "Synchronize",
                            "Synchronize to pipeline clock",
                            DEFAULT_SYNC,
                            GParamFlags (G_PARAM_READWRITE)));

  g_object_class_install_property (gobject_class, PROP_CHECK_PERFECT,
      g_param_spec_boolean ("check-perfect", "Check For Perfect Stream",
                            "Verify that the stream is time- and data-contiguous",
                            DEFAULT_CHECK_PERFECT,
                            GParamFlags (G_PARAM_READWRITE)));

  g_object_class_install_property (gobject_class, PROP_MUSICDNS_ID,
      g_param_spec_string ("musicdns-id", "MusicDNS ID (needed for use with libofa)",
                           "MusicDNS ID (needed for use with libofa) http://www.musicdns.org",
                           NULL,
                           GParamFlags (G_PARAM_READWRITE)));

  g_object_class_install_property (gobject_class, PROP_XMLDOC,
      g_param_spec_string ("xmldoc", "Resulting XML Document after PUID finding",
                           "Resulting XML Document after PUID finding",
                           NULL,
                           GParamFlags (G_PARAM_READABLE)));

  g_object_class_install_property (gobject_class, PROP_ARTIST,
      g_param_spec_string ("artist", "Resulting Artist name after PUID finding",
                           "Resulting Artist name after PUID finding",
                           NULL,
                           GParamFlags (G_PARAM_READABLE)));

  g_object_class_install_property (gobject_class, PROP_TITLE,
      g_param_spec_string ("title", "Resulting Title after PUID finding",
                           "Resulting Title after PUID finding",
                            NULL,
                            GParamFlags (G_PARAM_READABLE)));

  g_object_class_install_property (gobject_class, PROP_PUID,
      g_param_spec_string ("puid", "Resulting PUID",
                           "Resulting PUID",
                            NULL,
                            GParamFlags (G_PARAM_READABLE)));

  gobject_class->finalize = GST_DEBUG_FUNCPTR (gst_puid_finalize);

  gstbasetrans_class->transform_ip = GST_DEBUG_FUNCPTR (gst_puid_transform_ip);
  gstbasetrans_class->start = GST_DEBUG_FUNCPTR (gst_puid_start);
  gstbasetrans_class->stop = GST_DEBUG_FUNCPTR (gst_puid_stop);
}

/* XML Document Parser */
enum
{
  E_NONE      = 0,
  E_TRACK     = 1 << 0,
  E_TITLE     = 1 << 1,
  E_ARTIST    = 1 << 2,
  E_NAME      = 1 << 3,
  E_PUID_LIST = 1 << 4,
  E_PUID      = 1 << 5
};

static void
element_end (GMarkupParseContext  * context,
                     const char           * element_name,
                     gpointer               data,
                     GError              ** error)
{
  GstPUID* puid = GST_PUID (data);

  if (!strcmp (element_name, "track")) {
      puid->markupstate &= ~E_TRACK;      
  } else if (!strcmp (element_name, "title")) {
      puid->markupstate &= ~E_TITLE;      
  } else if (!strcmp (element_name, "name")) {
      puid->markupstate &= ~E_NAME;      
  } else if (!strcmp (element_name, "puid-list")) {
      puid->markupstate &= ~E_PUID_LIST;      
  } else if (!strcmp (element_name, "puid")) {
      puid->markupstate &= ~E_PUID;      
  }
}

static void
pcdata (GMarkupParseContext *context,
              const gchar         *text,
              gsize                text_len,
              gpointer             user_data,
              GError             **error)
{
  GstPUID *puid = GST_PUID (user_data);

  if (puid->markupstate & E_TRACK) 
    {
      if (puid->markupstate & E_NAME)
        {
          puid->artist = g_strdup (text);
          return;
        }

      if (puid->markupstate & E_TITLE)
        {
          puid->title = g_strdup (text);
          return;
        }

    }
}

static void
element_start (GMarkupParseContext* context,
                       const char*          element_name,
                       const char**         attribute_names,
                       const char**         attribute_values,
                       gpointer             data,
                       GError**             error)
{
  GstPUID* puid = GST_PUID (data);

  if (!strcmp (element_name, "track")) {
      puid->markupstate |= E_TRACK;      
  } else if (!strcmp (element_name, "title")) {
      puid->markupstate |= E_TITLE;      
  } else if (!strcmp (element_name, "name")) {
      puid->markupstate |= E_NAME;      
  } else if (!strcmp (element_name, "puid-list")) {
      puid->markupstate |= E_PUID_LIST;      
  } else if (!strcmp (element_name, "puid")) {
      puid->markupstate |= E_PUID;      
      guint n = 0;
      while (attribute_names[n])
      {
        if (!strcmp(attribute_names[n], "id"))
        {
          puid->puid = g_strdup (attribute_values[n]);
          break;
        }
        n++;
      }
  }
}

namespace
{
  const GMarkupParser parser = 
  {
    element_start,
    element_end,
    pcdata,
    NULL,
    NULL, 
  };
}

static void
gst_puid_init (GstPUID * puid, GstPUIDClass * g_class)
{
  gst_base_transform_set_passthrough (GST_BASE_TRANSFORM (puid), TRUE);

  puid->sync = DEFAULT_SYNC;
  puid->check_perfect = DEFAULT_CHECK_PERFECT;
  puid->musicdns_id = NULL;
  puid->chans = 0;
  puid->rate = 0;
  puid->width = 0;
  puid->depth = 0;
  puid->sign  = 0;
  puid->got_caps = FALSE;
  puid->data = NULL;
  puid->datasize = 0;
  puid->fingerprint = 0;
  puid->record = TRUE;
  puid->artist = NULL;
  puid->title = NULL;
  puid->puid = NULL;
}

static void
gst_puid_check_perfect (GstPUID * puid, GstBuffer * buf)
{
  GstClockTime timestamp;

  timestamp = GST_BUFFER_TIMESTAMP (buf);

  /* see if we need to do perfect stream checking */
  /* invalid timestamp drops us out of check.  FIXME: maybe warn ? */
  if (timestamp != GST_CLOCK_TIME_NONE) {
    /* check if we had a previous buffer to compare to */
    if (puid->prev_timestamp != GST_CLOCK_TIME_NONE) {
      guint64 offset;

      if (puid->prev_timestamp + puid->prev_duration != timestamp) {
        GST_WARNING_OBJECT (puid,
            "Buffer not time-contiguous with previous one: " "prev ts %"
            GST_TIME_FORMAT ", prev dur %" GST_TIME_FORMAT ", new ts %"
            GST_TIME_FORMAT, GST_TIME_ARGS (puid->prev_timestamp),
            GST_TIME_ARGS (puid->prev_duration), GST_TIME_ARGS (timestamp));
      }

      offset = GST_BUFFER_OFFSET (buf);
      if (puid->prev_offset_end != offset) {
        GST_WARNING_OBJECT (puid,
            "Buffer not data-contiguous with previous one: "
            "prev offset_end %" G_GINT64_FORMAT ", new offset %"
            G_GINT64_FORMAT, puid->prev_offset_end, offset);
      }
    }
    /* update prev values */
    puid->prev_timestamp = timestamp;
    puid->prev_duration = GST_BUFFER_DURATION (buf);
    puid->prev_offset_end = GST_BUFFER_OFFSET_END (buf);
  }
}

static GstFlowReturn
gst_puid_transform_ip (GstBaseTransform * trans, GstBuffer * buf)
{
  GstFlowReturn ret = GST_FLOW_OK;
  GstPUID *puid = GST_PUID (trans);
  GstClockTime runtimestamp = G_GINT64_CONSTANT (0);

  if (!puid->got_caps)
  {
      GstPad *pad = gst_element_get_pad (GST_ELEMENT (puid), "src");
      GstCaps *caps;
      if (pad)
        {
            caps = gst_pad_get_negotiated_caps (pad);
            if (caps)
            {
              GstStructure *structure;
              structure = gst_caps_get_structure (caps, 0);
              
              if (gst_structure_get_int (structure, "channels", &puid->chans) &&
                    gst_structure_get_int (structure, "rate", &puid->rate) && gst_structure_get_boolean (structure, "signed", &puid->sign)
                  && gst_structure_get_int (structure, "width", &puid->width) && gst_structure_get_int (structure, "depth", &puid->depth)) 
                {
                    if ((puid->chans != 0) && (puid->rate != 0)) puid->got_caps = TRUE;
                }
            }
        }
  }

  if (puid->check_perfect)
    gst_puid_check_perfect (puid, buf);

  if (puid->record && buf && buf->caps)
  {
      GST_OBJECT_LOCK(puid);
      GstStructure *str = gst_caps_get_structure (buf->caps, 0);
      if (str)
        {
           const char *name = gst_structure_get_name (str);
          if (name && !strcmp (name, "audio/x-raw-int"))
          {
            puid->data = (char*)realloc (puid->data, puid->datasize+buf->size);
            memcpy (puid->data+puid->datasize, buf->data, buf->size);
            puid->datasize += buf->size;
          }
        }
      GST_OBJECT_UNLOCK(puid);
  }

  guint seconds = buf->timestamp / GST_SECOND;
  if ((seconds == 135) && !puid->fingerprint)
  {
      GST_OBJECT_LOCK(puid);

      puid->fingerprint = g_strdup (ofa_create_print ((unsigned char*)puid->data,
                                                      (G_BYTE_ORDER == G_LITTLE_ENDIAN) ? 0 : 1,
                                                       puid->datasize,
                                                       puid->rate,
                                                      (puid->chans == 2) ? 1 : 0));

      g_free (puid->data);

      puid->data = NULL;
      puid->datasize = 0;
      puid->record = FALSE;

      GST_OBJECT_UNLOCK(puid);
      GstQuery *query;
      GstFormat format = GST_FORMAT_TIME;
      gint64 duration_ns = 0;
      gint duration = 0;
      query = gst_query_new_duration (format);
      gst_element_query (GST_ELEMENT (gst_object_get_parent (GST_OBJECT_CAST (puid))), query);
      gst_query_parse_duration (query, &format, &duration_ns);
      duration = duration_ns / 1000000;
      gst_query_unref (query);
      GST_OBJECT_LOCK(puid);

      const char *unknown = "unknown";

      const char *request_format = 
                    "cid=%s&"       // Client ID
                    "cvr=%s&"       // Client Version
                    "fpt=%s&"       // Fingerprint
                    "rmd=%d&"       // m = 1: return metadata; m = 0: only return id 
                    "brt=%d&"       // bitrate (kbps)
                    "fmt=%s&"       // File extension (e.g. mp3, ogg, flac)
                    "dur=%ld&"      // Length of track (milliseconds)
                    "art=%s&"       // Artist name. If there is none, send "unknown"
                    "ttl=%s&"       // Track title. If there is none, send "unknown"
                    "alb=%s&"       // Album name. If there is none, send "unknown"
                    "tnm=%d&"       // Track number in album. If there is none, send "0"
                    "gnr=%s&"       // Genre. If there is none, send "unknown"
                    "yrr=%s&"       // Year. If there is none, send "0"
                    "enc=%s"        // Encoding. e = true: ISO-8859-15; e = false: UTF-8 (default). Optional.
                    "\r\n";

      puid->puid = g_strdup_printf (request_format, 
                                         puid->musicdns_id,
                                         "gst_puid_0_01",
                                         puid->fingerprint,
                                         1,
                                         0,
                                         "",
                                         duration, 
                                         unknown,
                                         unknown,
                                         unknown,
                                         0,  
                                         unknown,
                                         "",
                                         "");

#if 0
      Soup::RequestSyncRefP request = Soup::RequestSync::create ("http://ofa.musicdns.org/ofa/1/track/", true);
      request->add_request ("application/x-www-form-urlencoded", post_data);
      guint code = request->run ();

      if (code == 200)
      {
              std::string response = request->get_data ();
              puid->xmldoc = g_strdup (response.c_str());
          
              GMarkupParseContext * ctx = g_markup_parse_context_new (&parser, GMarkupParseFlags (0), puid, NULL);
              g_markup_parse_context_parse (ctx, puid->xmldoc, strlen (puid->xmldoc), NULL);
              g_markup_parse_context_free (ctx); 

      }
#endif

      GST_OBJECT_UNLOCK(puid);
      GstEvent *event = gst_event_new_eos ();
      gst_element_send_event (GST_ELEMENT (puid), event);

      goto out;
    }

    if ((puid->sync) && (trans->segment.format == GST_FORMAT_TIME))
    {
      GstClock *clock;

      GST_OBJECT_LOCK (puid);

      if ((clock = GST_ELEMENT (puid)->clock))
        {
          GstClockReturn cret;
          GstClockTime timestamp;

          timestamp = runtimestamp + GST_ELEMENT (puid)->base_time;

          /* save id if we need to unlock */
          /* FIXME: actually unlock this somewhere in the state changes */
          puid->clock_id = gst_clock_new_single_shot_id (clock, timestamp);
          GST_OBJECT_UNLOCK (puid);

          cret = gst_clock_id_wait (puid->clock_id, NULL);

          GST_OBJECT_LOCK (puid);

          if (puid->clock_id)
            {
              gst_clock_id_unref (puid->clock_id);
              puid->clock_id = NULL;
            }

          if (cret == GST_CLOCK_UNSCHEDULED)
            {
              ret = GST_FLOW_UNEXPECTED;
            }
    }
    GST_OBJECT_UNLOCK (puid);
  }
  
  out:

  puid->offset += GST_BUFFER_SIZE (buf);
  return ret;
}

static void
gst_puid_set_property (GObject * object, guint prop_id, const GValue * value, GParamSpec * pspec)
{
  GstPUID *puid;

  puid = GST_PUID (object);

  switch (prop_id)
  {
    case PROP_SYNC:
      puid->sync = g_value_get_boolean (value);
      break;
    case PROP_CHECK_PERFECT:
      puid->check_perfect = g_value_get_boolean (value);
      break;
    case PROP_MUSICDNS_ID:
      puid->musicdns_id = g_value_dup_string (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_puid_get_property (GObject * object, guint prop_id, GValue * value, GParamSpec * pspec)
{
  GstPUID *puid;

  puid = GST_PUID (object);

  switch (prop_id)
  {
    case PROP_ARTIST:
      g_value_set_string (value, puid->artist);
      break;

    case PROP_TITLE:
      g_value_set_string (value, puid->title);
      break;

    case PROP_PUID:
      g_value_set_string (value, puid->puid);
      break;

    case PROP_XMLDOC:
      g_value_set_string (value, puid->xmldoc);
      break;

    case PROP_SYNC:
      g_value_set_boolean (value, puid->sync);
      break;

    case PROP_CHECK_PERFECT:
      g_value_set_boolean (value, puid->check_perfect);
      break;

    case PROP_MUSICDNS_ID:
      g_value_set_string (value, puid->musicdns_id);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }

}

static gboolean
gst_puid_start (GstBaseTransform * trans)
{
  GstPUID *puid;

  puid = GST_PUID (trans);

  puid->offset = 0;
  puid->prev_timestamp = GST_CLOCK_TIME_NONE;
  puid->prev_duration = GST_CLOCK_TIME_NONE;
  puid->prev_offset_end = -1;

  puid->chans = 0;
  puid->rate  = 0;
  puid->sign  = 0;
  puid->width = 0;
  puid->depth = 0;

  if (puid->artist)
    g_free (puid->artist);

  if (puid->title)
    g_free (puid->title);

  if (puid->puid)
    g_free (puid->puid);

  puid->puid = NULL;
  puid->title = NULL;
  puid->artist = NULL;
  puid->got_caps = FALSE;
  
  GST_OBJECT_LOCK (puid);
  g_free (puid->data);
  puid->datasize = 0;
  puid->record = TRUE;
  GST_OBJECT_UNLOCK (puid);

  return TRUE;
}

static gboolean
gst_puid_stop (GstBaseTransform * trans)
{
  return TRUE;
}

static gboolean
plugin_init (GstPlugin * plugin)
{
  if (!gst_element_register (plugin, "puid", GST_RANK_PRIMARY, gst_puid_get_type ())) return FALSE;
  return TRUE;
}

GST_PLUGIN_DEFINE_STATIC (GST_VERSION_MAJOR,
                          GST_VERSION_MINOR,
                          "puid",
                          "Calculates MusicIP PUID from audio files and retrieves metadata",
                          plugin_init, PACKAGE_VERSION, "LGPL", "BMP Project", "Milosz Derezynski <internalerror@gmail.com>")
