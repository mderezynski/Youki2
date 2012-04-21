/* gtkstatusicon.h:
 *
 * Copyright (C) 2003 Sun Microsystems, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Authors:
 *      Mark McLoughlin <mark@skynet.ie>
 */

#ifndef YOUKI_STATUS_ICON_H
#define YOUKI_STATUS_ICON_H

#include <gtk/gtkimage.h>
#include <gtk/gtkmenu.h>
#include "youki-tray-icon.h"

G_BEGIN_DECLS

#define YOUKI_TYPE_STATUS_ICON         (youki_status_icon_get_type ())
#define YOUKI_STATUS_ICON(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), YOUKI_TYPE_STATUS_ICON, YoukiStatusIcon))
#define YOUKI_STATUS_ICON_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST ((k), YOUKI_TYPE_STATUS_ICON, YoukiStatusIconClass))
#define YOUKI_IS_STATUS_ICON(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), YOUKI_TYPE_STATUS_ICON))
#define YOUKI_IS_STATUS_ICON_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), YOUKI_TYPE_STATUS_ICON))
#define YOUKI_STATUS_ICON_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), YOUKI_TYPE_STATUS_ICON, YoukiStatusIconClass))

typedef struct _YoukiStatusIcon	     YoukiStatusIcon;
typedef struct _YoukiStatusIconClass   YoukiStatusIconClass;
typedef struct _YoukiStatusIconPrivate YoukiStatusIconPrivate;

struct _YoukiStatusIcon
{
  GObject               parent_instance;

  YoukiStatusIconPrivate *priv;
};

struct _YoukiStatusIconClass
{
  GObjectClass parent_class;

  void     (* scroll_up)    (YoukiStatusIcon *status_icon);
  void     (* scroll_down)  (YoukiStatusIcon *status_icon);
  void     (* click)        (YoukiStatusIcon *status_icon);
  void     (* activate)     (YoukiStatusIcon *status_icon);
  void     (* popup_menu)   (YoukiStatusIcon *status_icon,
	                  		     guint          button,
                  			     guint32        activate_time);
  gboolean (* size_changed) (YoukiStatusIcon *status_icon,
                  			     gint           size);

  void (*__gtk_reserved1);
  void (*__gtk_reserved2);
  void (*__gtk_reserved3);
  void (*__gtk_reserved4);
  void (*__gtk_reserved5);
  void (*__gtk_reserved6);  
};

GType                 youki_status_icon_get_type           (void) G_GNUC_CONST;

YoukiStatusIcon        *youki_status_icon_new                (void);
YoukiStatusIcon        *youki_status_icon_new_from_pixbuf    (GdkPixbuf          *pixbuf);
YoukiStatusIcon        *youki_status_icon_new_from_file      (const gchar        *filename);
YoukiStatusIcon        *youki_status_icon_new_from_stock     (const gchar        *stock_id);
YoukiStatusIcon        *youki_status_icon_new_from_icon_name (const gchar        *icon_name);

void                  youki_status_icon_set_from_pixbuf    (YoukiStatusIcon      *status_icon,
                                                          GdkPixbuf          *pixbuf);
void                  youki_status_icon_set_from_file      (YoukiStatusIcon      *status_icon,
                                                          const gchar        *filename);
void                  youki_status_icon_set_from_stock     (YoukiStatusIcon      *status_icon,
                                                          const gchar        *stock_id);
void                  youki_status_icon_set_from_icon_name (YoukiStatusIcon      *status_icon,
                                                          const gchar        *icon_name);

GtkImageType          youki_status_icon_get_storage_type   (YoukiStatusIcon      *status_icon);

GdkPixbuf            *youki_status_icon_get_pixbuf         (YoukiStatusIcon      *status_icon);
G_CONST_RETURN gchar *youki_status_icon_get_stock          (YoukiStatusIcon      *status_icon);
G_CONST_RETURN gchar *youki_status_icon_get_icon_name      (YoukiStatusIcon      *status_icon);

gint                  youki_status_icon_get_size           (YoukiStatusIcon      *status_icon);

void                  youki_status_icon_set_visible        (YoukiStatusIcon      *status_icon,
                                                          gboolean            visible);
gboolean              youki_status_icon_get_visible        (YoukiStatusIcon      *status_icon);

void                  youki_status_icon_set_blinking       (YoukiStatusIcon      *status_icon,
                                                          gboolean            blinking);
gboolean              youki_status_icon_get_blinking       (YoukiStatusIcon      *status_icon);

gboolean              youki_status_icon_is_embedded        (YoukiStatusIcon      *status_icon);

void                  youki_status_icon_position_menu      (GtkMenu            *menu,
                                                          gint               *x,
                                                          gint               *y,
                                                          gboolean           *push_in,
                                                          gpointer            user_data);

gboolean              youki_status_icon_get_geometry       (YoukiStatusIcon      *status_icon,
                                                          GdkScreen         **screen,
                                                          GdkRectangle       *area,
                                                          GtkOrientation     *orientation);

GtkWidget*            youki_status_icon_get_tray_icon      (YoukiStatusIcon      *status_icon);


G_END_DECLS

#endif /* __YOUKI_STATUS_ICON_H__ */
