/* gtkstatusicon.c:
 *
 * Copyright (C) 2003 Sun Microsystems, Inc.
 * Copyright (C) 2005 Hans Breuer <hans@breuer.org>
 * Copyright (C) 2005 Novell, Inc.
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
 *
 * Authors:
 *	Mark McLoughlin <mark@skynet.ie>
 *	Hans Breuer <hans@breuer.org>
 *	Tor Lillqvist <tml@novell.com>
 */

#include <config.h>
#include <string.h>

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gtk/gtkprivate.h>

#include "widget-marshalers.h"
#include "youki-status-icon.h"
#include "youki-tray-icon.h"

#define BLINK_TIMEOUT 500

enum
{
  PROP_0,
  PROP_PIXBUF,
  PROP_FILE,
  PROP_STOCK,
  PROP_ICON_NAME,
  PROP_STORAGE_TYPE,
  PROP_SIZE,
  PROP_VISIBLE,
  PROP_BLINKING
};

enum 
{
  ACTIVATE_SIGNAL,
  CLICK_SIGNAL,
  SCROLL_UP_SIGNAL,
  SCROLL_DOWN_SIGNAL,
  POPUP_MENU_SIGNAL,
  SIZE_CHANGED_SIGNAL,
  LAST_SIGNAL
};

struct _YoukiStatusIconPrivate
{
  GtkWidget    *tray_icon;
  GtkWidget    *image;

  gint          size;

  gint          image_width;
  gint          image_height;

  GtkImageType  storage_type;
  
  gboolean      pressed;

  union
    {
      GdkPixbuf *pixbuf;
      gchar     *stock_id;
      gchar     *icon_name;
    } image_data;

  GdkPixbuf    *blank_icon;
  guint         blinking_timeout;

  guint         blinking : 1;
  guint         blink_off : 1;
  guint         visible : 1;
};

static void     youki_status_icon_finalize         (GObject        *object);
static void     youki_status_icon_set_property     (GObject        *object,
                                                  guint           prop_id,
                                                  const GValue   *value,
                                                  GParamSpec     *pspec);
static void     youki_status_icon_get_property     (GObject        *object,
                                                  guint           prop_id,
                                                  GValue         *value,
                                                  GParamSpec     *pspec);

static void     youki_status_icon_size_allocate    (YoukiStatusIcon  *status_icon,
                                                  GtkAllocation  *allocation);
static gboolean youki_status_icon_button_press     (YoukiStatusIcon  *status_icon,
                                                  GdkEventButton *event);
static gboolean youki_status_icon_button_release   (YoukiStatusIcon  *status_icon,
                                    						  GdkEventButton *event);
static gboolean youki_status_icon_scroll           (YoukiStatusIcon  *status_icon,
                                    						  GdkEventScroll *event);
static void     youki_status_icon_disable_blinking (YoukiStatusIcon  *status_icon);
static void     youki_status_icon_reset_image_data (YoukiStatusIcon  *status_icon);
					   

static guint status_icon_signals [LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (YoukiStatusIcon, youki_status_icon, G_TYPE_OBJECT)

static void
youki_status_icon_class_init (YoukiStatusIconClass *class)
{
  GObjectClass *gobject_class = (GObjectClass *) class;

  gobject_class->finalize     = youki_status_icon_finalize;
  gobject_class->set_property = youki_status_icon_set_property;
  gobject_class->get_property = youki_status_icon_get_property;

  g_object_class_install_property (gobject_class,
				   PROP_PIXBUF,
				   g_param_spec_object ("pixbuf",
							_("Pixbuf"),
							_("A GdkPixbuf to display"),
							GDK_TYPE_PIXBUF,
							GTK_PARAM_READWRITE));

  g_object_class_install_property (gobject_class,
				   PROP_FILE,
				   g_param_spec_string ("file",
							_("Filename"),
							_("Filename to load and display"),
							NULL,
							GTK_PARAM_WRITABLE));

  g_object_class_install_property (gobject_class,
				   PROP_STOCK,
				   g_param_spec_string ("stock",
							_("Stock ID"),
							_("Stock ID for a stock image to display"),
							NULL,
							GTK_PARAM_READWRITE));
  
  g_object_class_install_property (gobject_class,
                                   PROP_ICON_NAME,
                                   g_param_spec_string ("icon-name",
                                                        _("Icon Name"),
                                                        _("The name of the icon from the icon theme"),
                                                        NULL,
                                                        GTK_PARAM_READWRITE));
  
  g_object_class_install_property (gobject_class,
				   PROP_STORAGE_TYPE,
				   g_param_spec_enum ("storage-type",
						      _("Storage type"),
						      _("The representation being used for image data"),
						      GTK_TYPE_IMAGE_TYPE,
						      GTK_IMAGE_EMPTY,
						      GTK_PARAM_READABLE));

  g_object_class_install_property (gobject_class,
				   PROP_SIZE,
				   g_param_spec_int ("size",
						     _("Size"),
						     _("The size of the icon"),
						     0,
						     G_MAXINT,
						     0,
						     GTK_PARAM_READABLE));

  g_object_class_install_property (gobject_class,
				   PROP_BLINKING,
				   g_param_spec_boolean ("blinking",
							 _("Blinking"),
							 _("Whether or not the status icon is blinking"),
							 FALSE,
							 GTK_PARAM_READWRITE));

  g_object_class_install_property (gobject_class,
				   PROP_VISIBLE,
				   g_param_spec_boolean ("visible",
							 _("Visible"),
							 _("Whether or not the status icon is visible"),
							 TRUE,
							 GTK_PARAM_READWRITE));

  /**
   * YoukiStatusIcon::activate:
   * @status_icon: the object which received the signal
   *
   * Gets emitted when the user activates the status icon. 
   * If and how status icons can activated is platform-dependent.
   *
   * Since: 2.10
   */
  status_icon_signals [ACTIVATE_SIGNAL] =
    g_signal_new (("activate"),
		  G_TYPE_FROM_CLASS (gobject_class),
		  G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		  G_STRUCT_OFFSET (YoukiStatusIconClass, activate),
		  NULL,
		  NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE,
		  0);

  /**
   * YoukiStatusIcon::click:
   * @status_icon: the object which received the signal
   *
   * Gets emitted when the user single-clicks the icon. 
   *
   * mderezynski
   *
   */
  status_icon_signals [CLICK_SIGNAL] =
    g_signal_new (("click"),
		  G_TYPE_FROM_CLASS (gobject_class),
		  G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		  G_STRUCT_OFFSET (YoukiStatusIconClass, click),
		  NULL,
		  NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE,
		  0);

  /**
   * YoukiStatusIcon::scroll-up:
   * @status_icon: the object which received the signal
   *
   * Gets emitted when the user does a mouse-wheel scroll-up 
   *
   * mderezynski
   *
   */
  status_icon_signals [SCROLL_UP_SIGNAL] =
    g_signal_new (("scroll-up"),
		  G_TYPE_FROM_CLASS (gobject_class),
		  G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		  G_STRUCT_OFFSET (YoukiStatusIconClass, scroll_down),
		  NULL,
		  NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE,
		  0);

  /**
   * YoukiStatusIcon::scroll-down:
   * @status_icon: the object which received the signal
   *
   * Gets emitted when the user does a mouse-wheel scroll-down 
   *
   * mderezynski
   *
   */
  status_icon_signals [SCROLL_DOWN_SIGNAL] =
    g_signal_new (("scroll-down"),
		  G_TYPE_FROM_CLASS (gobject_class),
		  G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		  G_STRUCT_OFFSET (YoukiStatusIconClass, scroll_down),
		  NULL,
		  NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE,
		  0);

  /**
   * YoukiStatusIcon::popup-menu:
   * @status_icon: the object which received the signal
   * @button: the button that was pressed, or 0 if the 
   *   signal is not emitted in response to a button press event
   * @activate_time: the timestamp of the event that
   *   triggered the signal emission
   *
   * Gets emitted when the user brings up the context menu
   * of the status icon. Whether status icons can have context 
   * menus and how these are activated is platform-dependent.
   *
   * The @button and @activate_timeout parameters should be 
   * passed as the last to arguments to gtk_menu_popup().
   *
   * Since: 2.10
   */
  status_icon_signals [POPUP_MENU_SIGNAL] =
    g_signal_new (("popup-menu"),
		  G_TYPE_FROM_CLASS (gobject_class),
		  G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		  G_STRUCT_OFFSET (YoukiStatusIconClass, popup_menu),
		  NULL,
		  NULL,
		  g_cclosure_user_marshal_VOID__UINT_UINT, 
		  G_TYPE_NONE,
		  2,
		  G_TYPE_UINT,
		  G_TYPE_UINT);

  /**
   * YoukiStatusIcon::size-changed:
   * @status_icon: the object which received the signal
   * @size: the new size
   *
   * Gets emitted when the size available for the image
   * changes, e.g. because the notification area got resized.
   *
   * Since: 2.10
   */
  status_icon_signals [SIZE_CHANGED_SIGNAL] =
    g_signal_new (("size-changed"),
		  G_TYPE_FROM_CLASS (gobject_class),
		  G_SIGNAL_RUN_FIRST,
		  G_STRUCT_OFFSET (YoukiStatusIconClass, size_changed),
		  NULL,
		  NULL,
		  g_cclosure_marshal_VOID__INT,
		  G_TYPE_NONE,
		  1,
		  G_TYPE_INT);

    g_type_class_add_private (class, sizeof (YoukiStatusIconPrivate));
}

static void
youki_status_icon_init (YoukiStatusIcon *status_icon)
{
  YoukiStatusIconPrivate *priv;

  priv = G_TYPE_INSTANCE_GET_PRIVATE (status_icon, YOUKI_TYPE_STATUS_ICON,
				      YoukiStatusIconPrivate);
  status_icon->priv = priv;
  
  priv->storage_type = GTK_IMAGE_EMPTY;
  priv->visible      = TRUE;

  priv->size         = 0;
  priv->image_width  = 0;
  priv->image_height = 0;

  priv->tray_icon = GTK_WIDGET (_youki_tray_icon_new (NULL));

  gtk_widget_add_events (GTK_WIDGET (priv->tray_icon),
			 GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_SCROLL_MASK);

  g_signal_connect_swapped (status_icon->priv->tray_icon, "button-press-event",
			    G_CALLBACK (youki_status_icon_button_press), status_icon);

  g_signal_connect_swapped (status_icon->priv->tray_icon, "button-release-event",
			    G_CALLBACK (youki_status_icon_button_release), status_icon);

  g_signal_connect_swapped (status_icon->priv->tray_icon, "scroll-event",
			    G_CALLBACK (youki_status_icon_scroll), status_icon);

  priv->image = gtk_image_new ();

  g_signal_connect_swapped (priv->image, "size-allocate",
			    G_CALLBACK (youki_status_icon_size_allocate), status_icon);

  gtk_container_add (GTK_CONTAINER (priv->tray_icon), priv->image);

  gtk_widget_show (priv->image);
  gtk_widget_show (priv->tray_icon);
}

static void
youki_status_icon_finalize (GObject *object)
{
  YoukiStatusIcon *status_icon = YOUKI_STATUS_ICON (object);
  YoukiStatusIconPrivate *priv = status_icon->priv;

  youki_status_icon_disable_blinking (status_icon);
  
  youki_status_icon_reset_image_data (status_icon);

  if (priv->blank_icon)
    g_object_unref (priv->blank_icon);
  priv->blank_icon = NULL;

  gtk_widget_destroy (priv->tray_icon);

  G_OBJECT_CLASS (youki_status_icon_parent_class)->finalize (object);
}

static void
youki_status_icon_set_property (GObject      *object,
			      guint         prop_id,
			      const GValue *value,
			      GParamSpec   *pspec)
{
  YoukiStatusIcon *status_icon = YOUKI_STATUS_ICON (object);

  switch (prop_id)
    {
    case PROP_PIXBUF:
      youki_status_icon_set_from_pixbuf (status_icon, g_value_get_object (value));
      break;
    case PROP_FILE:
      youki_status_icon_set_from_file (status_icon, g_value_get_string (value));
      break;
    case PROP_STOCK:
      youki_status_icon_set_from_stock (status_icon, g_value_get_string (value));
      break;
    case PROP_ICON_NAME:
      youki_status_icon_set_from_icon_name (status_icon, g_value_get_string (value));
      break;
    case PROP_BLINKING:
      youki_status_icon_set_blinking (status_icon, g_value_get_boolean (value));
      break;
    case PROP_VISIBLE:
      youki_status_icon_set_visible (status_icon, g_value_get_boolean (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
youki_status_icon_get_property (GObject    *object,
			      guint       prop_id,
			      GValue     *value,
			      GParamSpec *pspec)
{
  YoukiStatusIcon *status_icon = YOUKI_STATUS_ICON (object);
  YoukiStatusIconPrivate *priv = status_icon->priv;

  /* The "getter" functions whine if you try to get the wrong
   * storage type. This function is instead robust against that,
   * so that GUI builders don't have to jump through hoops
   * to avoid g_warning
   */

  switch (prop_id)
    {
    case PROP_PIXBUF:
      if (priv->storage_type != GTK_IMAGE_PIXBUF)
	g_value_set_object (value, NULL);
      else
	g_value_set_object (value, youki_status_icon_get_pixbuf (status_icon));
      break;
    case PROP_STOCK:
      if (priv->storage_type != GTK_IMAGE_STOCK)
	g_value_set_string (value, NULL);
      else
	g_value_set_string (value, youki_status_icon_get_stock (status_icon));
      break;
    case PROP_ICON_NAME:
      if (priv->storage_type != GTK_IMAGE_ICON_NAME)
	g_value_set_string (value, NULL);
      else
	g_value_set_string (value, youki_status_icon_get_icon_name (status_icon));
      break;
    case PROP_STORAGE_TYPE:
      g_value_set_enum (value, youki_status_icon_get_storage_type (status_icon));
      break;
    case PROP_SIZE:
      g_value_set_int (value, youki_status_icon_get_size (status_icon));
      break;
    case PROP_BLINKING:
      g_value_set_boolean (value, youki_status_icon_get_blinking (status_icon));
      break;
    case PROP_VISIBLE:
      g_value_set_boolean (value, youki_status_icon_get_visible (status_icon));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

/**
 * youki_status_icon_new:
 * 
 * Creates an empty status icon object.
 * 
 * Return value: a new #YoukiStatusIcon
 *
 * Since: 2.10
 **/
YoukiStatusIcon *
youki_status_icon_new (void)
{
  return g_object_new (YOUKI_TYPE_STATUS_ICON, NULL);
}

/**
 * youki_status_icon_new_from_pixbuf:
 * @pixbuf: a #GdkPixbuf
 * 
 * Creates a status icon displaying @pixbuf. 
 *
 * The image will be scaled down to fit in the available 
 * space in the notification area, if necessary.
 * 
 * Return value: a new #YoukiStatusIcon
 *
 * Since: 2.10
 **/
YoukiStatusIcon *
youki_status_icon_new_from_pixbuf (GdkPixbuf *pixbuf)
{
  return g_object_new (YOUKI_TYPE_STATUS_ICON,
		       "pixbuf", pixbuf,
		       NULL);
}

/**
 * youki_status_icon_new_from_file:
 * @filename: a filename
 * 
 * Creates a status icon displaying the file @filename. 
 *
 * The image will be scaled down to fit in the available 
 * space in the notification area, if necessary.
 * 
 * Return value: a new #YoukiStatusIcon
 *
 * Since: 2.10
 **/
YoukiStatusIcon *
youki_status_icon_new_from_file (const gchar *filename)
{
  return g_object_new (YOUKI_TYPE_STATUS_ICON,
		       "file", filename,
		       NULL);
}

/**
 * youki_status_icon_new_from_stock:
 * @stock_id: a stock icon id
 * 
 * Creates a status icon displaying a stock icon. Sample stock icon
 * names are #GTK_STOCK_OPEN, #GTK_STOCK_QUIT. You can register your 
 * own stock icon names, see gtk_icon_factory_add_default() and 
 * gtk_icon_factory_add(). 
 *
 * Return value: a new #YoukiStatusIcon
 *
 * Since: 2.10
 **/
YoukiStatusIcon *
youki_status_icon_new_from_stock (const gchar *stock_id)
{
  return g_object_new (YOUKI_TYPE_STATUS_ICON,
		       "stock", stock_id,
		       NULL);
}

/**
 * youki_status_icon_new_from_icon_name:
 * @icon_name: an icon name
 * 
 * Creates a status icon displaying an icon from the current icon theme.
 * If the current icon theme is changed, the icon will be updated 
 * appropriately.
 * 
 * Return value: a new #YoukiStatusIcon
 *
 * Since: 2.10
 **/
YoukiStatusIcon *
youki_status_icon_new_from_icon_name (const gchar *icon_name)
{
  return g_object_new (YOUKI_TYPE_STATUS_ICON,
		       "icon-name", icon_name,
		       NULL);
}

static void
emit_click_signal (YoukiStatusIcon *status_icon)
{
  g_signal_emit (status_icon,
		 status_icon_signals [CLICK_SIGNAL], 0);
}

static void
emit_scroll_up_signal (YoukiStatusIcon *status_icon)
{
  g_signal_emit (status_icon,
		 status_icon_signals [SCROLL_UP_SIGNAL], 0);
}

static void
emit_scroll_down_signal (YoukiStatusIcon *status_icon)
{
  g_signal_emit (status_icon,
		 status_icon_signals [SCROLL_DOWN_SIGNAL], 0);
}


static void
emit_popup_menu_signal (YoukiStatusIcon *status_icon,
			guint          button,
			guint32        activate_time)
{
  g_signal_emit (status_icon,
		 status_icon_signals [POPUP_MENU_SIGNAL], 0,
		 button,
		 activate_time);
}

static gboolean
emit_size_changed_signal (YoukiStatusIcon *status_icon,
			  gint           size)
{
  gboolean handled = FALSE;
  
  g_signal_emit (status_icon,
		 status_icon_signals [SIZE_CHANGED_SIGNAL], 0,
		 size,
		 &handled);

  return handled;
}

static GdkPixbuf *
youki_status_icon_blank_icon (YoukiStatusIcon *status_icon)
{
  YoukiStatusIconPrivate *priv = status_icon->priv;

  if (priv->blank_icon)
    {
      gint width, height;

      width  = gdk_pixbuf_get_width (priv->blank_icon);
      height = gdk_pixbuf_get_height (priv->blank_icon);


      if (width == priv->image_width && height == priv->image_height)
	return priv->blank_icon;
      else
	{
	  g_object_unref (priv->blank_icon);
	  priv->blank_icon = NULL;
	}
    }

  priv->blank_icon = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8,
				     priv->image_width, 
				     priv->image_height);
  if (priv->blank_icon)
    gdk_pixbuf_fill (priv->blank_icon, 0);

  return priv->blank_icon;
}

static GtkIconSize
find_icon_size (GtkWidget *widget, 
		gint       pixel_size)
{
  GdkScreen *screen;
  GtkSettings *settings;
  GtkIconSize s, size;
  gint w, h, d, dist;

  screen = gtk_widget_get_screen (widget);

  if (!screen)
    return GTK_ICON_SIZE_MENU;

  settings = gtk_settings_get_for_screen (screen);
  
  dist = G_MAXINT;
  size = GTK_ICON_SIZE_MENU;

  for (s = GTK_ICON_SIZE_MENU; s < GTK_ICON_SIZE_DIALOG; s++)
    {
      if (gtk_icon_size_lookup_for_settings (settings, s, &w, &h) &&
	  w <= pixel_size && h <= pixel_size)
	{
	  d = MAX (pixel_size - w, pixel_size - h);
	  if (d < dist)
	    {
	      dist = d;
	      size = s;
	    }
	}
    }
  
  return size;
}

static void
youki_status_icon_update_image (YoukiStatusIcon *status_icon)
{
  YoukiStatusIconPrivate *priv = status_icon->priv;

  if (priv->blink_off)
    {
      gtk_image_set_from_pixbuf (GTK_IMAGE (priv->image),
				 youki_status_icon_blank_icon (status_icon));
      return;
    }

  switch (priv->storage_type)
    {
    case GTK_IMAGE_PIXBUF:
      {
	GdkPixbuf *pixbuf;

	pixbuf = priv->image_data.pixbuf;

	if (pixbuf)
	  {
	    GdkPixbuf *scaled;
	    gint size;
	    gint width;
	    gint height;

	    size = priv->size;

	    width  = gdk_pixbuf_get_width  (pixbuf);
	    height = gdk_pixbuf_get_height (pixbuf);

	    if (width > size || height > size)
	      scaled = gdk_pixbuf_scale_simple (pixbuf,
						MIN (size, width),
						MIN (size, height),
						GDK_INTERP_BILINEAR);
	    else
	      scaled = g_object_ref (pixbuf);

	    gtk_image_set_from_pixbuf (GTK_IMAGE (priv->image), scaled);
	    g_object_unref (scaled);
	  }
	else
	  {
	    gtk_image_set_from_pixbuf (GTK_IMAGE (priv->image), NULL);
	  }
      }
      break;

    case GTK_IMAGE_STOCK:
      {
	GtkIconSize size = find_icon_size (priv->image, priv->size);
	gtk_image_set_from_stock (GTK_IMAGE (priv->image),
				  priv->image_data.stock_id,
				  size);
      }
      break;
      
    case GTK_IMAGE_ICON_NAME:
      {
	GtkIconSize size = find_icon_size (priv->image, priv->size);
	gtk_image_set_from_icon_name (GTK_IMAGE (priv->image),
				      priv->image_data.icon_name,
				      size);
      }
      break;
      
    case GTK_IMAGE_EMPTY:
      gtk_image_set_from_pixbuf (GTK_IMAGE (priv->image), NULL);
      break;
    default:
      g_assert_not_reached ();
      break;
    }
}

static void
youki_status_icon_size_allocate (YoukiStatusIcon *status_icon,
			       GtkAllocation *allocation)
{
  YoukiStatusIconPrivate *priv = status_icon->priv;
  GtkOrientation orientation;
  gint size;

  orientation = _youki_tray_icon_get_orientation (YOUKI_TRAY_ICON (priv->tray_icon));

  if (orientation == GTK_ORIENTATION_HORIZONTAL)
    size = allocation->height;
  else
    size = allocation->width;

  priv->image_width = allocation->width - GTK_MISC (priv->image)->xpad * 2;
  priv->image_height = allocation->height - GTK_MISC (priv->image)->ypad * 2;

  if (priv->size != size)
    {
      priv->size = size;

      g_object_notify (G_OBJECT (status_icon), "size");

      if (!emit_size_changed_signal (status_icon, size))
	youki_status_icon_update_image (status_icon);
    }
}

static gboolean
youki_status_icon_scroll (YoukiStatusIcon  *status_icon,
		       GdkEventScroll *event)
{

  switch (event->direction)
    {
	    case GDK_SCROLL_UP:
	      {
		emit_scroll_up_signal (status_icon);
		break;
	      }

	    case GDK_SCROLL_DOWN:
	      {
		emit_scroll_down_signal (status_icon);
		break;
	      }

	    default: break;
    }

  return FALSE;
}

static gboolean
youki_status_icon_button_release (YoukiStatusIcon  *status_icon,
			        GdkEventButton *event)
{
    status_icon->priv->pressed = FALSE;
    return FALSE;
}

static gboolean
youki_status_icon_button_press (YoukiStatusIcon  *status_icon,
			      GdkEventButton *event)
{
  status_icon->priv->pressed = TRUE;

  if (event->button == 1 && event->type == GDK_BUTTON_PRESS)
    {
      emit_click_signal (status_icon);
    }
  else
  if (event->button == 3 && event->type == GDK_BUTTON_PRESS)
    {
      emit_popup_menu_signal (status_icon, event->button, event->time); 
    }

  return FALSE;
}



static void
youki_status_icon_reset_image_data (YoukiStatusIcon *status_icon)
{
  YoukiStatusIconPrivate *priv = status_icon->priv;

  switch (priv->storage_type)
  {
    case GTK_IMAGE_PIXBUF:
      if (priv->image_data.pixbuf)
	g_object_unref (priv->image_data.pixbuf);
      priv->image_data.pixbuf = NULL;
      g_object_notify (G_OBJECT (status_icon), "pixbuf");
      break;

    case GTK_IMAGE_STOCK:
      g_free (priv->image_data.stock_id);
      priv->image_data.stock_id = NULL;

      g_object_notify (G_OBJECT (status_icon), "stock");
      break;
      
    case GTK_IMAGE_ICON_NAME:
      g_free (priv->image_data.icon_name);
      priv->image_data.icon_name = NULL;

      g_object_notify (G_OBJECT (status_icon), "icon-name");
      break;
      
    case GTK_IMAGE_EMPTY:
      break;
    default:
      g_assert_not_reached ();
      break;
  }

  priv->storage_type = GTK_IMAGE_EMPTY;
  g_object_notify (G_OBJECT (status_icon), "storage-type");
}

static void
youki_status_icon_set_image (YoukiStatusIcon *status_icon,
    			   GtkImageType   storage_type,
			   gpointer       data)
{
  YoukiStatusIconPrivate *priv = status_icon->priv;

  g_object_freeze_notify (G_OBJECT (status_icon));

  youki_status_icon_reset_image_data (status_icon);

  priv->storage_type = storage_type;
  g_object_notify (G_OBJECT (status_icon), "storage-type");

  switch (storage_type) 
    {
    case GTK_IMAGE_PIXBUF:
      priv->image_data.pixbuf = (GdkPixbuf *)data;
      g_object_notify (G_OBJECT (status_icon), "pixbuf");
      break;
    case GTK_IMAGE_STOCK:
      priv->image_data.stock_id = g_strdup ((const gchar *)data);
      g_object_notify (G_OBJECT (status_icon), "stock");
      break;
    case GTK_IMAGE_ICON_NAME:
      priv->image_data.icon_name = g_strdup ((const gchar *)data);
      g_object_notify (G_OBJECT (status_icon), "icon-name");
      break;
    default:
      g_warning ("Image type %d not handled by YoukiStatusIcon", storage_type);
    }

  g_object_thaw_notify (G_OBJECT (status_icon));

  youki_status_icon_update_image (status_icon);
}

/**
 * youki_status_icon_set_from_pixbuf:
 * @status_icon: a #YoukiStatusIcon
 * @pixbuf: a #GdkPixbuf or %NULL
 * 
 * Makes @status_icon display @pixbuf. 
 * See youki_status_icon_new_from_pixbuf() for details.
 *
 * Since: 2.10
 **/
void
youki_status_icon_set_from_pixbuf (YoukiStatusIcon *status_icon,
				 GdkPixbuf     *pixbuf)
{
  g_return_if_fail (YOUKI_IS_STATUS_ICON (status_icon));
  g_return_if_fail (pixbuf == NULL || GDK_IS_PIXBUF (pixbuf));

  if (pixbuf)
    g_object_ref (pixbuf);

  youki_status_icon_set_image (status_icon, GTK_IMAGE_PIXBUF,
      			     (gpointer) pixbuf);
}

/**
 * youki_status_icon_set_from_file:
 * @status_icon: a #YoukiStatusIcon
 * @filename: a filename
 * 
 * Makes @status_icon display the file @filename.
 * See youki_status_icon_new_from_file() for details.
 *
 * Since: 2.10 
 **/
void
youki_status_icon_set_from_file (YoukiStatusIcon *status_icon,
 			       const gchar   *filename)
{
  GdkPixbuf *pixbuf;
  
  g_return_if_fail (YOUKI_IS_STATUS_ICON (status_icon));
  g_return_if_fail (filename != NULL);
  
  pixbuf = gdk_pixbuf_new_from_file (filename, NULL);
  
  youki_status_icon_set_from_pixbuf (status_icon, pixbuf);
  
  if (pixbuf)
    g_object_unref (pixbuf);
}

/**
 * youki_status_icon_set_from_stock:
 * @status_icon: a #YoukiStatusIcon
 * @stock_id: a stock icon id
 * 
 * Makes @status_icon display the stock icon with the id @stock_id.
 * See youki_status_icon_new_from_stock() for details.
 *
 * Since: 2.10 
 **/
void
youki_status_icon_set_from_stock (YoukiStatusIcon *status_icon,
				const gchar   *stock_id)
{
  g_return_if_fail (YOUKI_IS_STATUS_ICON (status_icon));
  g_return_if_fail (stock_id != NULL);

  youki_status_icon_set_image (status_icon, GTK_IMAGE_STOCK,
      			     (gpointer) stock_id);
}

/**
 * youki_status_icon_set_from_icon_name:
 * @status_icon: a #YoukiStatusIcon
 * @icon_name: an icon name
 * 
 * Makes @status_icon display the icon named @icon_name from the 
 * current icon theme.
 * See youki_status_icon_new_from_icon_name() for details.
 *
 * Since: 2.10 
 **/
void
youki_status_icon_set_from_icon_name (YoukiStatusIcon *status_icon,
				    const gchar   *icon_name)
{
  g_return_if_fail (YOUKI_IS_STATUS_ICON (status_icon));
  g_return_if_fail (icon_name != NULL);

  youki_status_icon_set_image (status_icon, GTK_IMAGE_ICON_NAME,
      			     (gpointer) icon_name);
}

/**
 * youki_status_icon_get_storage_type:
 * @status_icon: a #YoukiStatusIcon
 * 
 * Gets the type of representation being used by the #YoukiStatusIcon
 * to store image data. If the #YoukiStatusIcon has no image data,
 * the return value will be %GTK_IMAGE_EMPTY. 
 * 
 * Return value: the image representation being used
 *
 * Since: 2.10
 **/
GtkImageType
youki_status_icon_get_storage_type (YoukiStatusIcon *status_icon)
{
  g_return_val_if_fail (YOUKI_IS_STATUS_ICON (status_icon), GTK_IMAGE_EMPTY);

  return status_icon->priv->storage_type;
}
/**
 * youki_status_icon_get_pixbuf:
 * @status_icon: a #YoukiStatusIcon
 * 
 * Gets the #GdkPixbuf being displayed by the #YoukiStatusIcon.
 * The storage type of the status icon must be %GTK_IMAGE_EMPTY or
 * %GTK_IMAGE_PIXBUF (see youki_status_icon_get_storage_type()).
 * The caller of this function does not own a reference to the
 * returned pixbuf.
 * 
 * Return value: the displayed pixbuf, or %NULL if the image is empty.
 *
 * Since: 2.10
 **/
GdkPixbuf *
youki_status_icon_get_pixbuf (YoukiStatusIcon *status_icon)
{
  YoukiStatusIconPrivate *priv;

  g_return_val_if_fail (YOUKI_IS_STATUS_ICON (status_icon), NULL);

  priv = status_icon->priv;

  g_return_val_if_fail (priv->storage_type == GTK_IMAGE_PIXBUF ||
			priv->storage_type == GTK_IMAGE_EMPTY, NULL);

  if (priv->storage_type == GTK_IMAGE_EMPTY)
    priv->image_data.pixbuf = NULL;

  return priv->image_data.pixbuf;
}

/**
 * youki_status_icon_get_stock:
 * @status_icon: a #YoukiStatusIcon
 * 
 * Gets the id of the stock icon being displayed by the #YoukiStatusIcon.
 * The storage type of the status icon must be %GTK_IMAGE_EMPTY or
 * %GTK_IMAGE_STOCK (see youki_status_icon_get_storage_type()).
 * The returned string is owned by the #YoukiStatusIcon and should not
 * be freed or modified.
 * 
 * Return value: stock id of the displayed stock icon,
 *   or %NULL if the image is empty.
 *
 * Since: 2.10
 **/
G_CONST_RETURN gchar *
youki_status_icon_get_stock (YoukiStatusIcon *status_icon)
{
  YoukiStatusIconPrivate *priv;

  g_return_val_if_fail (YOUKI_IS_STATUS_ICON (status_icon), NULL);

  priv = status_icon->priv;

  g_return_val_if_fail (priv->storage_type == GTK_IMAGE_STOCK ||
			priv->storage_type == GTK_IMAGE_EMPTY, NULL);
  
  if (priv->storage_type == GTK_IMAGE_EMPTY)
    priv->image_data.stock_id = NULL;

  return priv->image_data.stock_id;
}

/**
 * youki_status_icon_get_icon_name:
 * @status_icon: a #YoukiStatusIcon
 * 
 * Gets the name of the icon being displayed by the #YoukiStatusIcon.
 * The storage type of the status icon must be %GTK_IMAGE_EMPTY or
 * %GTK_IMAGE_ICON_NAME (see youki_status_icon_get_storage_type()).
 * The returned string is owned by the #YoukiStatusIcon and should not
 * be freed or modified.
 * 
 * Return value: name of the displayed icon, or %NULL if the image is empty.
 *
 * Since: 2.10
 **/
G_CONST_RETURN gchar *
youki_status_icon_get_icon_name (YoukiStatusIcon *status_icon)
{
  YoukiStatusIconPrivate *priv;
  
  g_return_val_if_fail (YOUKI_IS_STATUS_ICON (status_icon), NULL);

  priv = status_icon->priv;

  g_return_val_if_fail (priv->storage_type == GTK_IMAGE_ICON_NAME ||
			priv->storage_type == GTK_IMAGE_EMPTY, NULL);

  if (priv->storage_type == GTK_IMAGE_EMPTY)
    priv->image_data.icon_name = NULL;

  return priv->image_data.icon_name;
}

/**
 * youki_status_icon_get_size:
 * @status_icon: a #YoukiStatusIcon
 * 
 * Gets the size in pixels that is available for the image. 
 * Stock icons and named icons adapt their size automatically
 * if the size of the notification area changes. For other
 * storage types, the size-changed signal can be used to
 * react to size changes.
 * 
 * Return value: the size that is available for the image
 *
 * Since: 2.10
 **/
gint
youki_status_icon_get_size (YoukiStatusIcon *status_icon)
{
  g_return_val_if_fail (YOUKI_IS_STATUS_ICON (status_icon), 0);

  return status_icon->priv->size;
}

static gboolean
youki_status_icon_blinker (YoukiStatusIcon *status_icon)
{
  YoukiStatusIconPrivate *priv = status_icon->priv;
  
  priv->blink_off = !priv->blink_off;

  youki_status_icon_update_image (status_icon);

  return TRUE;
}

static void
youki_status_icon_enable_blinking (YoukiStatusIcon *status_icon)
{
  YoukiStatusIconPrivate *priv = status_icon->priv;
  
  if (!priv->blinking_timeout)
    {
      youki_status_icon_blinker (status_icon);

      priv->blinking_timeout =
	g_timeout_add (BLINK_TIMEOUT, 
		       (GSourceFunc) youki_status_icon_blinker, 
		       status_icon);
    }
}

static void
youki_status_icon_disable_blinking (YoukiStatusIcon *status_icon)
{
  YoukiStatusIconPrivate *priv = status_icon->priv;
  
  if (priv->blinking_timeout)
    {
      g_source_remove (priv->blinking_timeout);
      priv->blinking_timeout = 0;
      priv->blink_off = FALSE;

      youki_status_icon_update_image (status_icon);
    }
}

/**
 * youki_status_icon_set_visible:
 * @status_icon: a #YoukiStatusIcon
 * @visible: %TRUE to show the status icon, %FALSE to hide it
 * 
 * Shows or hides a status icon.
 *
 * Since: 2.10
 **/
void
youki_status_icon_set_visible (YoukiStatusIcon *status_icon,
			     gboolean       visible)
{
  YoukiStatusIconPrivate *priv;

  g_return_if_fail (YOUKI_IS_STATUS_ICON (status_icon));

  priv = status_icon->priv;

  visible = visible != FALSE;

  if (priv->visible != visible)
    {
      priv->visible = visible;

      if (visible)
	gtk_widget_show (priv->tray_icon);
      else
	gtk_widget_hide (priv->tray_icon);
      g_object_notify (G_OBJECT (status_icon), "visible");
    }
}

/**
 * youki_status_icon_get_visible:
 * @status_icon: a #YoukiStatusIcon
 * 
 * Returns whether the status icon is visible or not. 
 * Note that being visible does not guarantee that 
 * the user can actually see the icon, see also 
 * youki_status_icon_is_embedded().
 * 
 * Return value: %TRUE if the status icon is visible
 *
 * Since: 2.10
 **/
gboolean
youki_status_icon_get_visible (YoukiStatusIcon *status_icon)
{
  g_return_val_if_fail (YOUKI_IS_STATUS_ICON (status_icon), FALSE);

  return status_icon->priv->visible;
}

/**
 * youki_status_icon_set_blinking:
 * @status_icon: a #YoukiStatusIcon
 * @blinking: %TRUE to turn blinking on, %FALSE to turn it off
 * 
 * Makes the status icon start or stop blinking. 
 * Note that blinking user interface elements may be problematic
 * for some users, and thus may be turned off, in which case
 * this setting has no effect.
 *
 * Since: 2.10
 **/
void
youki_status_icon_set_blinking (YoukiStatusIcon *status_icon,
			      gboolean       blinking)
{
  YoukiStatusIconPrivate *priv;

  g_return_if_fail (YOUKI_IS_STATUS_ICON (status_icon));

  priv = status_icon->priv;

  blinking = blinking != FALSE;

  if (priv->blinking != blinking)
    {
      priv->blinking = blinking;

      if (blinking)
	youki_status_icon_enable_blinking (status_icon);
      else
	youki_status_icon_disable_blinking (status_icon);

      g_object_notify (G_OBJECT (status_icon), "blinking");
    }
}

/**
 * youki_status_icon_get_blinking:
 * @status_icon: a #YoukiStatusIcon
 * 
 * Returns whether the icon is blinking, see 
 * youki_status_icon_set_blinking().
 * 
 * Return value: %TRUE if the icon is blinking
 *
 * Since: 2.10
 **/
gboolean
youki_status_icon_get_blinking (YoukiStatusIcon *status_icon)
{
  g_return_val_if_fail (YOUKI_IS_STATUS_ICON (status_icon), FALSE);

  return status_icon->priv->blinking;
}

/**
 * youki_status_icon_is_embedded:
 * @status_icon: a #YoukiStatusIcon
 * 
 * Returns whether the status icon is embedded in a notification
 * area. 
 * 
 * Return value: %TRUE if the status icon is embedded in
 *   a notification area.
 *
 * Since: 2.10
 **/
gboolean
youki_status_icon_is_embedded (YoukiStatusIcon *status_icon)
{
  GtkPlug *plug;

  g_return_val_if_fail (YOUKI_IS_STATUS_ICON (status_icon), FALSE);

  plug = GTK_PLUG (status_icon->priv->tray_icon);

  if (plug->socket_window)
    return TRUE;
  else
    return FALSE;
}

/**
 * youki_status_icon_position_menu:
 * @menu: the #GtkMenu
 * @x: return location for the x position
 * @y: return location for the y position
 * @push_in: return location for whether the menu should be pushed in 
 *     to be completely inside the screen instead of just clamped to the 
 *     size to the screen.
 * @user_data: the status icon to position the menu on
 *
 * Menu positioning function to use with gtk_menu_popup()
 * to position @menu aligned to the status icon @user_data.
 * 
 * Since: 2.10
 **/
void
youki_status_icon_position_menu (GtkMenu  *menu,
			       gint     *x,
			       gint     *y,
			       gboolean *push_in,
			       gpointer  user_data)
{
  YoukiStatusIcon *status_icon;
  YoukiStatusIconPrivate *priv;
  YoukiTrayIcon *tray_icon;
  GtkWidget *widget;
  GdkScreen *screen;
  GtkTextDirection direction;
  GtkRequisition menu_req;
  GdkRectangle monitor;
  gint monitor_num, height, width, xoffset, yoffset;
  
  g_return_if_fail (GTK_IS_MENU (menu));
  g_return_if_fail (YOUKI_IS_STATUS_ICON (user_data));

  status_icon = YOUKI_STATUS_ICON (user_data);
  priv = status_icon->priv;
  tray_icon = YOUKI_TRAY_ICON (priv->tray_icon);
  widget = priv->tray_icon;

  direction = gtk_widget_get_direction (widget);

  screen = gtk_widget_get_screen (widget);
  gtk_menu_set_screen (menu, screen);

  monitor_num = gdk_screen_get_monitor_at_window (screen, widget->window);
  if (monitor_num < 0)
    monitor_num = 0;
  gtk_menu_set_monitor (menu, monitor_num);

  gdk_screen_get_monitor_geometry (screen, monitor_num, &monitor);

  gdk_window_get_origin (widget->window, x, y);
  
  gtk_widget_size_request (GTK_WIDGET (menu), &menu_req);

  if (_youki_tray_icon_get_orientation (tray_icon) == GTK_ORIENTATION_VERTICAL)
    {
      width = 0;
      height = widget->allocation.height;
      xoffset = widget->allocation.width;
      yoffset = 0;
    }
  else
    {
      width = widget->allocation.width;
      height = 0;
      xoffset = 0;
      yoffset = widget->allocation.height;
    }

  if (direction == GTK_TEXT_DIR_RTL)
    {
      if ((*x - (menu_req.width - width)) >= monitor.x)
        *x -= menu_req.width - width;
      else if ((*x + xoffset + menu_req.width) < (monitor.x + monitor.width))
        *x += xoffset;
      else if ((monitor.x + monitor.width - (*x + xoffset)) < *x)
        *x -= menu_req.width - width;
      else
        *x += xoffset;
    }
  else
    {
      if ((*x + xoffset + menu_req.width) < (monitor.x + monitor.width))
        *x += xoffset;
      else if ((*x - (menu_req.width - width)) >= monitor.x)
        *x -= menu_req.width - width;
      else if ((monitor.x + monitor.width - (*x + xoffset)) > *x)
        *x += xoffset;
      else 
        *x -= menu_req.width - width;
    }

  if ((*y + yoffset + menu_req.height) < (monitor.y + monitor.height))
    *y += yoffset;
  else if ((*y - (menu_req.height - height)) >= monitor.y)
    *y -= menu_req.height - height;
  else if (monitor.y + monitor.height - (*y + yoffset) > *y)
    *y += yoffset;
  else 
    *y -= menu_req.height - height;

  *push_in = FALSE;
}

/**
 * youki_status_icon_get_geometry:
 * @status_icon: a #YoukiStatusIcon
 * @screen: return location for the screen, or %NULL if the
 *          information is not needed 
 * @area: return location for the area occupied by the status 
 *        icon, or %NULL
 * @orientation: return location for the orientation of the panel 
 *    in which the status icon is embedded, or %NULL. A panel 
 *    at the top or bottom of the screen is horizontal, a panel 
 *    at the left or right is vertical.
 *
 * Obtains information about the location of the status icon
 * on screen. This information can be used to e.g. position 
 * popups like notification bubbles. 
 *
 * See youki_status_icon_position_menu() for a more convenient 
 * alternative for positioning menus.
 *
 * Note that some platforms do not allow GTK+ to provide 
 * this information, and even on platforms that do allow it,
 * the information is not reliable unless the status icon
 * is embedded in a notification area, see
 * youki_status_icon_is_embedded().
 *
 * Return value: %TRUE if the location information has 
 *               been filled in
 *
 * Since: 2.10
 */
gboolean  
youki_status_icon_get_geometry (YoukiStatusIcon    *status_icon,
			      GdkScreen       **screen,
			      GdkRectangle     *area,
			      GtkOrientation   *orientation)
{
  GtkWidget *widget;
  YoukiStatusIconPrivate *priv;
  gint x, y;

  g_return_val_if_fail (YOUKI_IS_STATUS_ICON (status_icon), FALSE);

  priv = status_icon->priv;
  widget = priv->tray_icon;

  if (screen)
    *screen = gtk_widget_get_screen (widget);

  if (area)
    {
      gdk_window_get_origin (widget->window, &x, &y);
      area->x = x;
      area->y = y;
      area->width = widget->allocation.width;
      area->height = widget->allocation.height;
    }

  if (orientation)
    *orientation = _youki_tray_icon_get_orientation (YOUKI_TRAY_ICON (widget));

  return TRUE;
}

GtkWidget*
youki_status_icon_get_tray_icon       (YoukiStatusIcon      *status_icon)
{
  YoukiStatusIconPrivate *priv = status_icon->priv;
  return priv->tray_icon;
}



