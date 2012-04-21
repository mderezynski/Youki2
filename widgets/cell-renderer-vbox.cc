//  MPX
//  Copyright (C) 2005-2007 MPX Project 
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif //HAVE_CONFIG_H

#include <glibmm.h>
#include <gtkmm.h>
#include <algorithm>
#include "mpx/widgets/cell-renderer-vbox.hh"

using namespace Gtk;

namespace MPX
{
    CellRendererVBox::CellRendererVBox ()
        : ObjectBase              (typeid(CellRendererVBox))
        , property_renderer1_     (*this, "renderer1", 0)
        , property_renderer2_     (*this, "renderer2", 0)
    {
    }

    CellRendererVBox::~CellRendererVBox ()
    {
    }

    void
    CellRendererVBox::get_size_vfunc  (Gtk::Widget & widget,
                                       const Gdk::Rectangle * cell_area,
                                       int *   x_offset,
                                       int *   y_offset,
                                       int *   width,
                                       int *   height) const 
    {
		Gtk::CellRenderer * renderer1 = property_renderer1_.get_value();
		Gtk::CellRenderer * renderer2 = property_renderer2_.get_value();

		int x_offset1 = 0, y_offset1 = 0, width1 = 0, height1 = 0;
		int x_offset2 = 0, y_offset2 = 0, width2 = 0, height2 = 0;

		if(renderer1)	
			renderer1->get_size(widget, *cell_area, x_offset1, y_offset1, width1, height1);

		if(renderer2)	
			renderer2->get_size(widget, *cell_area, x_offset2, y_offset2, width2, height2);

		if(x_offset)
			*x_offset = x_offset1;

		if(y_offset)
			*y_offset = y_offset1;

		if(width)
			*width = std::max(width1,width2); 

		if(height)
		*height = height1+height2;
    }

    void
    CellRendererVBox::render_vfunc    (Glib::RefPtr<Gdk::Drawable> const& window,
                                       Gtk::Widget                      & widget,
                                       Gdk::Rectangle              const& background_area,
                                       Gdk::Rectangle              const& cell_area,
                                       Gdk::Rectangle              const& expose_area,
                                       Gtk::CellRendererState             flags)
    {
		Gtk::CellRenderer * renderer1 = property_renderer1().get_value();
		Gtk::CellRenderer * renderer2 = property_renderer2().get_value();

		int x_offset1 = 0, y_offset1 = 0, width1 = 0, height1 = 0;
		int x_offset2 = 0, y_offset2 = 0, width2 = 0, height2 = 0;

		if(renderer1)	
			renderer1->get_size(widget, cell_area, x_offset1, y_offset1, width1, height1);

		if(renderer2)	
			renderer2->get_size(widget, cell_area, x_offset2, y_offset2, width2, height2);

		if(renderer1)
		{
			Gdk::Rectangle bgarea_cell1 = background_area;
			Gdk::Rectangle cellarea_cell1 = cell_area;
			Gdk::Rectangle exposearea_cell1 = expose_area;

			bgarea_cell1.set_height(bgarea_cell1.get_height() - height2); 
			cellarea_cell1.set_height(cellarea_cell1.get_height() - height2); 
			exposearea_cell1.set_height(exposearea_cell1.get_height() - height2); 

			renderer1->render (Glib::RefPtr<Gdk::Window>::cast_static(window), widget, bgarea_cell1, cellarea_cell1, exposearea_cell1, flags); 
		}

		if(renderer2)
		{
			Gdk::Rectangle bgarea_cell2 = background_area;
			Gdk::Rectangle cellarea_cell2 = cell_area;
			Gdk::Rectangle exposearea_cell2 = expose_area;

			bgarea_cell2.set_height(bgarea_cell2.get_height() - height1); 
			cellarea_cell2.set_height(cellarea_cell2.get_height() - height1); 
			exposearea_cell2.set_height(exposearea_cell2.get_height() - height1); 

			bgarea_cell2.set_y(bgarea_cell2.get_y()+height1);
			cellarea_cell2.set_y(cellarea_cell2.get_y()+height1);
			exposearea_cell2.set_y(exposearea_cell2.get_y()+height1);

			renderer2->render (Glib::RefPtr<Gdk::Window>::cast_static(window), widget, bgarea_cell2, cellarea_cell2, exposearea_cell2, flags); 
		}
    }

    ///////////////////////////////////////////////
    /// Object Properties
    ///////////////////////////////////////////////

    ProxyOf<PropRenderer>::ReadWrite
    CellRendererVBox::property_renderer1()
    {
      return ProxyOf<PropRenderer>::ReadWrite (this, "renderer1");
    }

    ProxyOf<PropRenderer>::ReadWrite
    CellRendererVBox::property_renderer2()
    {
      return ProxyOf<PropRenderer>::ReadWrite (this, "renderer2");
    }
};
