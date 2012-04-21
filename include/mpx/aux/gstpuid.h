/* GStreamer
 * Copyright (C) 1999,2000 Erik Walthinsen <omega@cse.ogi.edu>
 *                    2000 Wim Taymans <wtay@chello.be>
 *
 *
 * gstpuid.h:
 *
 *
 * gstidentity.h:
 *
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


#ifndef __GST_PUID_H__
#define __GST_PUID_H__


#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>

G_BEGIN_DECLS


#define GST_TYPE_PUID \
  (gst_puid_get_type())
#define GST_PUID(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_PUID,GstPUID))
#define GST_PUID_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_PUID,GstPUIDClass))
#define GST_IS_PUID(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_PUID))
#define GST_IS_PUID_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_PUID))

typedef struct _GstPUID GstPUID;
typedef struct _GstPUIDClass GstPUIDClass;

/**
 * GstPUID:
 *
 * Opaque #GstPUID data structure
 */
struct _GstPUID
{
  GstBaseTransform 	 element;

  /*< private >*/
  GstClockID     clock_id;
  gboolean 	     sync;
  gboolean 	     check_perfect;
  GstClockTime   prev_timestamp;
  GstClockTime   prev_duration;
  guint64        prev_offset_end;
  guint64        offset;
  gchar         *musicdns_id;
  gboolean       got_caps;
  gint           rate;
  gint           chans;
  char          *data;
  guint          datasize;
  char          *fingerprint;
  gboolean       record;
  gboolean       sign;
  int            width;
  int            depth;
  char          *xmldoc;

  char          *artist;
  char          *title;
  char          *puid;

  int            markupstate;
};

struct _GstPUIDClass
{
  GstBaseTransformClass parent_class;
};

GType gst_puid_get_type(void);

G_END_DECLS

#endif /* __GST_PUID_H__ */
