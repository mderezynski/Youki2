// (c) 2007 M. Derezynski

#include "mpx/playlistparser-xspf.hh"

#include <glib/gmacros.h>
#include <glib/gmessages.h>
#include <string>
#include "xmlcpp/xspf.hxx"
#include "mpx/xml/xmltoc++.hh"

namespace MPX
{
namespace PlaylistParser
{
	XSPF::XSPF ()
	{
	}

	XSPF::~XSPF ()
	{
	}

	bool
	XSPF::read (std::string const& uri, Track_v & v, bool G_GNUC_UNUSED)
	{
		try{
			MPX::XmlInstance<xspf::playlist> xspf (uri.c_str());
			if(xspf.xml().trackList().size())
			{
				xspf::trackList::track_sequence tracklist = (*xspf.xml().trackList().begin()).track();
				for(xspf::trackList::track_sequence::iterator i = tracklist.begin(); i != tracklist.end(); ++i)
				{
                    MPX::Track t;

                    t[ATTRIBUTE_LOCATION] = OVariant(*(i->location().begin()));
                    t[ATTRIBUTE_TITLE] = OVariant(*(i->title().begin()));
                    t[ATTRIBUTE_ARTIST] = OVariant(*(i->creator().begin()));
                    t[ATTRIBUTE_ALBUM] = OVariant(*(i->album().begin()));

                    gint64 time = *(i->duration().begin());
                    t[ATTRIBUTE_TIME] = OVariant(time);

                    gint64 track = *(i->trackNum().begin());
                    t[ATTRIBUTE_TRACK] = OVariant(track); 
            
                    v.push_back(t);
				}
			}
		} catch (...) {
			return false;
		}
		return true;
	}

	bool
	XSPF::write (std::string const& uri, Track_v const& list)
	{
		return false;
	}
}
}
