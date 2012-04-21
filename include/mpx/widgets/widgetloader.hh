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

#include <libglademm/xml.h>
#include <glade/glade-xml.h>

namespace Gnome {
namespace Glade {


/*
* WidgetLoader template class
*/
template <class T_WIDGET> class WidgetLoader : public T_WIDGET {
public:
	/*
	* constructor takes a RefPtr<Xml> to the glade data
	* and the glade name of the widget (of type T_WIDGET) 
	* to load.
	*/
	WidgetLoader(const Glib::RefPtr<Xml>& refxml, const Glib::ustring& widgetname) : 
		/*
		* with help of get_widget the underlying
		* T_WIDGET class is initialized 
		*/
		T_WIDGET( get_widget(refxml, widgetname))
    ,   m_Xml(refxml)
	{}
protected:
    Glib::RefPtr<Gnome::Glade::Xml> m_Xml;
private:
	typedef typename T_WIDGET::BaseObjectType widget_type;

	/*
	* static helper function
	* returns pointer to GtkWidget object
	* (if widgetname exists in glade xml)
	*/
	static widget_type *get_widget(const Glib::RefPtr<Xml>& _refxml, 
			const Glib::ustring& widgetname) 
	{
		//get pointer from glade 
		widget_type *base_widget = (widget_type *)glade_xml_get_widget(
				_refxml->gobj(), widgetname.c_str() );
		if (!base_widget) 
			throw XmlError("Base widget \"" + widgetname +
					"not found in glade file \"" +
					_refxml->get_filename() + "\".");
		//check reference count
		Glib::ObjectBase *object_base = Glib::ObjectBase::_get_current_wrapper( 
				(GObject*) base_widget );
		if (object_base) {
			throw XmlError("oject already exists.");
		} else {
			Glib::RefPtr<Xml> refThis(_refxml);
			//increase reference count
			refThis->reference(); 
		}
		return base_widget;
	};
};

}
}

#endif
