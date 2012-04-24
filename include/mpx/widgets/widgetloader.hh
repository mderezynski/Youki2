/*
* Copyright (C) 2007 André Gaul <gaul@web-yard.de>
* 
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*
*/

#ifndef __widgetloader_include__
#define __widgetloader_include__

#include <gtkmm/builder.h>
#include <stdexcept>

namespace MPX {

/*
* WidgetLoader template class
*/
template <class T_WIDGET> class WidgetLoader : public T_WIDGET {
public:
	/*
	* constructor takes a RefPtr<Builder> to the glade data
	* and the glade name of the widget (of type T_WIDGET) 
	* to load.
	*/
	WidgetLoader(const Glib::RefPtr<Gtk::Builder>& refbuilder, const Glib::ustring& widgetname) : 
		/*
		* with help of get_widget the underlying
		* T_WIDGET class is initialized 
		*/
		T_WIDGET( get_widget(refbuilder, widgetname))
    ,   m_Builder(refbuilder)
	{}
protected:
    Glib::RefPtr<Gtk::Builder> m_Builder;
private:
	typedef typename T_WIDGET::BaseObjectType widget_type;

	/*
	* static helper function
	* returns pointer to GtkWidget object
	* (if widgetname exists in glade xml)
	*/
	static widget_type *get_widget(const Glib::RefPtr<Gtk::Builder>& _refbuilder, 
			const Glib::ustring& widgetname) 
	{
		//get pointer from glade 
		widget_type *base_widget = (widget_type *)gtk_builder_get_object(
				_refbuilder->gobj(), widgetname.c_str() );
		if (!base_widget) 
			throw std::runtime_error("Base widget \"" + widgetname +
					"not found in builder.");
		//check reference count
		Glib::ObjectBase *object_base = Glib::ObjectBase::_get_current_wrapper( 
				(GObject*) base_widget );
		if (object_base) {
			throw std::runtime_error("Object already exists.");
		} else {
			Glib::RefPtr<Gtk::Builder> refThis(_refbuilder);
			//increase reference count
			refThis->reference(); 
		}
		return base_widget;
	};
};

} // MPX namespace

#endif
