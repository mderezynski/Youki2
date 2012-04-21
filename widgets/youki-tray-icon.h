/* gtktrayicon.h
 * Copyright (C) 2002 Anders Carlsson <andersca@gnu.org>
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
 */

#ifndef YOUKI_TRAY_ICON_H
#define YOUKI_TRAY_ICON_H

#include <gtk/gtkplug.h>

G_BEGIN_DECLS

#define YOUKI_TYPE_TRAY_ICON		(youki_tray_icon_get_type ())
#define YOUKI_TRAY_ICON(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), YOUKI_TYPE_TRAY_ICON, YoukiTrayIcon))
#define YOUKI_TRAY_ICON_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), YOUKI_TYPE_TRAY_ICON, YoukiTrayIconClass))
#define YOUKI_IS_TRAY_ICON(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), YOUKI_TYPE_TRAY_ICON))
#define YOUKI_IS_TRAY_ICON_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), YOUKI_TYPE_TRAY_ICON))
#define YOUKI_TRAY_ICON_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), YOUKI_TYPE_TRAY_ICON, YoukiTrayIconClass))
	
typedef struct _YoukiTrayIcon	   YoukiTrayIcon;
typedef struct _YoukiTrayIconPrivate YoukiTrayIconPrivate;
typedef struct _YoukiTrayIconClass   YoukiTrayIconClass;

struct _YoukiTrayIcon
{
  GtkPlug parent_instance;

  YoukiTrayIconPrivate *priv;
};

struct _YoukiTrayIconClass
{
  GtkPlugClass parent_class;

  void (*destroyed) (YoukiTrayIcon *tray);

  void (*__gtk_reserved1);
  void (*__gtk_reserved2);
  void (*__gtk_reserved3);
  void (*__gtk_reserved4);
  void (*__gtk_reserved5);
  void (*__gtk_reserved6);
};

GType          youki_tray_icon_get_type         (void) G_GNUC_CONST;

YoukiTrayIcon   *_youki_tray_icon_new_for_screen  (GdkScreen   *screen,
					       const gchar *name);

YoukiTrayIcon   *_youki_tray_icon_new             (const gchar *name);

guint          _youki_tray_icon_send_message    (YoukiTrayIcon *icon,
					       gint         timeout,
					       const gchar *message,
					       gint         len);
void           _youki_tray_icon_cancel_message  (YoukiTrayIcon *icon,
					       guint        id);

GtkOrientation _youki_tray_icon_get_orientation (YoukiTrayIcon *icon);
					    
G_END_DECLS

#endif /* __YOUKI_TRAY_ICON_H__ */
