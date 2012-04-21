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
#ifndef MPX_UTIL_GRAPHICS_HH
#define MPX_UTIL_GRAPHICS_HH
#include "config.h"
#include <string>
#include <glibmm/ustring.h>
#include <gtk/gtk.h>
#include <cairomm/context.h>
#include <gdkmm/color.h>
#include <gdkmm/colormap.h>
#include <gdkmm/drawable.h>
#include <gdkmm/pixbuf.h>
#include <gdkmm/screen.h>
#include <gtkmm/uimanager.h>
#include <gtkmm/widget.h>
#include <gtkmm/window.h>

namespace MPX
{
  namespace Util
  {
    Glib::RefPtr<Gdk::Pixbuf>
    get_image_from_uri(
          const std::string&
    ) ;

    void
    window_set_busy(
          GtkWindow*
    ) ;

    void
    window_set_idle(
          GtkWindow*
    ) ;

    void
    window_set_busy(
          Gtk::Window&
    ) ;

    void
    window_set_idle(
          Gtk::Window&
    ) ;

    double
    screen_get_resolution(
          Glib::RefPtr<Gdk::Screen>
    ) ;

    double
    screen_get_x_resolution(
          Glib::RefPtr<Gdk::Screen>
    ) ;

    double
    screen_get_y_resolution(
          Glib::RefPtr<Gdk::Screen>
    ) ;

    void
    color_to_rgba(
          const Gdk::Color&
        , double& /*r*/
        , double& /*g*/
        , double& /*b*/
        , double& /*a*/
    ) ;

    Gdk::Color
    color_adjust_brightness(
          const Gdk::Color&
        , double /*brightness*/
    ) ;

    Gdk::Color
    color_shade(
          const Gdk::Color&
        , double /*ratio*/
    ) ;

    Gdk::Color
    color_from_hsb(
          double /*hue*/
        , double /*saturation*/
        , double /*brightness*/
    ) ;

    void
    color_to_hsb(
          const Gdk::Color&
        , double & /*hue*/
        , double & /*saturation*/
        , double & /*brightness*/
    );


    Cairo::RefPtr<Cairo::ImageSurface>
    cairo_image_surface_from_pixbuf(
          Glib::RefPtr<Gdk::Pixbuf>
    ) ;

    Glib::RefPtr<Gdk::Pixbuf>
    cairo_image_surface_to_pixbuf(
          Cairo::RefPtr<Cairo::ImageSurface>
    ) ;

    void
    cairo_rounded_rect(
          Cairo::RefPtr<Cairo::Context>
         , double /*x*/
         , double /*y*/
         , double /*width*/
         , double /*height*/
         , double /*radius*/
    ) ;

    Cairo::RefPtr<Cairo::ImageSurface>
    cairo_image_surface_scale(
          Cairo::RefPtr<Cairo::ImageSurface>    /*source*/
        , double                                /*width*/
        , double                                /*height*/
    ) ;

    Cairo::RefPtr<Cairo::ImageSurface>
    cairo_image_surface_round(
          Cairo::RefPtr<Cairo::ImageSurface>    /*source*/
        , double                                /*radius*/
    ) ;

    void 
    cairo_image_surface_border(
          Cairo::RefPtr<Cairo::ImageSurface>    /*source*/
        , double                                /*width*/
        , double                                r = 0.
        , double                                g = 0.
        , double                                b = 0.
        , double                                a = 1.
    ) ;

    void 
    cairo_image_surface_rounded_border(
          Cairo::RefPtr<Cairo::ImageSurface>    /*source*/
        , double                                /*width*/
        , double                                /*radius*/
        , double                                r = 0.
        , double                                g = 0.
        , double                                b = 0.
        , double                                a = 1.
    ) ;

    Cairo::RefPtr<Cairo::ImageSurface>
    cairo_image_surface_overlay(
          Cairo::RefPtr<Cairo::ImageSurface>    /*source*/
        , Cairo::RefPtr<Cairo::ImageSurface>    /*overlay*/
        , double                                /*x*/
        , double                                /*y*/
        , double                                alpha = 1.
    ) ; 

    void
    draw_cairo_image(
          Cairo::RefPtr<Cairo::Context>         /*cairo context*/
        , Cairo::RefPtr<Cairo::ImageSurface>    /*image*/
        , double                                /*x*/
        , double                                /*y*/
        , double                                /*alpha*/
    ) ;

    Gdk::Color
    get_mean_color_for_pixbuf(
          Glib::RefPtr<Gdk::Pixbuf>
    ) ;
  } // Util
} // MPX

#endif // MPX_UTIL_GRAPHICS_HH
