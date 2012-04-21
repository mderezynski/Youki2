//  MPX
//  Copyright (C) 2005-2007 MPX development.
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
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

#ifndef MPX_PLUGIN_TYPES_HH
#define MPX_PLUGIN_TYPES_HH

#include <config.h>
#include <boost/shared_ptr.hpp>
#include <glib/gtypes.h>
#include <gtkmm/widget.h>

#include <string>

namespace MPX
{
	struct PluginHolderBase
	{
		protected:

			std::string		m_Name ;
			std::string		m_Description ;
			std::string		m_Authors ;
			std::string		m_Copyright ;
			int				m_IAge ;
			std::string		m_Website ;
			bool			m_Active ;
            bool            m_HasGUI ;
            bool            m_CanActivate ;
            bool            m_Hidden ;
			guint			m_Id ;

		public:

            virtual bool
            activate(
            ) = 0;

            virtual bool
            deactivate(
            ) = 0;

            virtual Gtk::Widget*
            get_gui(
            ) = 0;

			virtual std::string const&
			get_name ()			const	{ return m_Name; }
	
			virtual std::string	const&
			get_desc ()			const	{ return m_Description; }

			virtual std::string const&
			get_authors ()		const	{ return m_Authors; }
	
			virtual std::string const&
			get_copyright ()	const	{ return m_Copyright; }

			virtual std::string const&
			get_website ()		const	{ return m_Website; }
	
			virtual bool
			get_active ()		const	{ return m_Active; }

            virtual bool
            get_has_gui ()      const   { return m_HasGUI; }

            virtual bool
            get_can_activate () const   { return m_CanActivate; }

            virtual bool
            get_hidden ()       const   { return m_Hidden; }

			virtual guint
			get_id ()			const	{ return m_Id; }

		friend class PluginManager;
        friend class PluginActivate;
	};

//	typedef boost::shared_ptr<PluginHolderBase> PluginHolderRefP_t ;
	typedef PluginHolderBase* PluginHolderRefP_t ;

    class MethodInvocationError : public std::exception
    {
        public:

            MethodInvocationError(
                const std::string& _message = std::string()
            )
            : message(_message)
            {}

            virtual
            ~MethodInvocationError() throw()
            {}

            virtual const char*
            what() const throw()
            {
                return message.c_str();
            }

       private:

            std::string message;
    };
}

#endif
