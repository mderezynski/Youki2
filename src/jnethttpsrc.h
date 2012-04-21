/* vim: set sw=2: -*- Mode: C; tab-width: 2; indent-tabs-mode: t; c-basic-offset: 2; c-indent-level: 2 -*- */
/* GStreamer
 * Copyright (C) <2005> Edgard Lima <edgard.lima@indt.org.br>
 *               <2006-2007> Milosz Derezynski <internalerror@gmail.com>
 *
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

#ifndef __MPX_JNETHTTP_SRC_H__
#define __MPX_JNETHTTP_SRC_H__

#include <glibmm.h>
#include <gst/gst.h>
#include <gst/base/gstpushsrc.h>
#include <stdio.h>

#include "mpx/mpx-uri.hh"
#include "jnetlib/connection.h"
#include "jnetlib/httpget.h"
#include "jnetlib/util.h"

G_BEGIN_DECLS

#define GST_TYPE_JNETHTTP_SRC \
  (gst_jnethttpsrc_get_type())
#define MPX_JNETHTTP_SRC(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_JNETHTTP_SRC,MPXJNLHttpSrc))
#define MPX_JNETHTTP_SRC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_JNETHTTP_SRC,MPXJNLHttpSrcClass))
#define GST_IS_JNETHTTP_SRC(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_JNETHTTP_SRC))
#define GST_IS_JNETHTTP_SRC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_JNETHTTP_SRC))

typedef struct _MPXJNLHttpSrc MPXJNLHttpSrc;
typedef struct _MPXJNLHttpSrcClass MPXJNLHttpSrcClass;

struct _MPXJNLHttpSrc
{
  GstPushSrc element;

  JNL_HTTPGet * get;
  gchar * uri; 
  gchar * customheader;

  gint64 content_size;
  gint64 content_read;

  gboolean eos;
  gboolean abort;

  /* icecast/audiocast metadata extraction handling */
  gboolean iradio_mode;

  GstCaps *icy_caps;

  gint icy_metaint;

  /* buffering+lastfm */
  gboolean first_buffering;

  GMutex  * abort_lock;
};

struct _MPXJNLHttpSrcClass {
  GstPushSrcClass parent_class;
};

GType gst_jnethttpsrc_get_type (void);

G_END_DECLS

#endif /* __MPX_JNETHTTP_SRC_H__ */
