//
// (C) 2007 DEREZYNSKI Milosz
//

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif //HAVE_CONFIG_H

#include <glibmm.h>

#include <taglib-gio.h>
#include <fileref.h>
#include <tfile.h>
#include <tag.h>

using namespace TagLib;
using namespace std;

#include "mpx/mpx-types.hh"
// we don't really need it, but just to make sure we won't fuck up declaration/definition 
#include "common.hh"

namespace MPX
{
    bool
    metadata_check_file (TagLib::File * file)
    {
      if (!file->isOpen())
        return false;

      if (!file->isValid())
        return false;

      return true;
    }

    void
    metadata_get_common (TagLib::File * file, Track & track)
    {
      Glib::ustring value;

      value = (file->tag()->artist().to8Bit (true));
      if (!value.empty() && value.validate())
          track [ ATTRIBUTE_ARTIST ] = value;

      value = (file->tag()->album().to8Bit (true));
      if (!value.empty() && value.validate())
          track [ ATTRIBUTE_ALBUM ] = value;

      value = (file->tag()->title().to8Bit (true));
      if (!value.empty() && value.validate())
          track [ ATTRIBUTE_TITLE ] = value; 

      value = (file->tag()->genre().to8Bit (true));
      if (!value.empty() && value.validate())
          track [ ATTRIBUTE_GENRE ] = value;

      value = (file->tag()->comment().to8Bit (true));
      if (!value.empty() && value.validate())
          track [ ATTRIBUTE_COMMENT ] = value;

      if (file->audioProperties())
      {
          track [ ATTRIBUTE_BITRATE ] = guint (file->audioProperties()->bitrate());
          track [ ATTRIBUTE_SAMPLERATE ] = guint (file->audioProperties()->sampleRate());
          track [ ATTRIBUTE_TIME ] = guint (file->audioProperties()->length());
      }
      else
      {
          track [ ATTRIBUTE_BITRATE ] = guint (0);
          track [ ATTRIBUTE_SAMPLERATE ] = guint (0);
          track [ ATTRIBUTE_TIME ] = guint (0);
      }

      track [ ATTRIBUTE_TRACK ] = guint (file->tag()->track());
      track [ ATTRIBUTE_DATE ] = guint (file->tag()->year());

    }

    void
    metadata_set_common (TagLib::File * file, Track & track)
    {
      TagLib::Tag * tag = file->tag();
      if (tag)
      {
        if (track[ATTRIBUTE_TITLE])
            tag->setTitle   (String (boost::get<std::string>(track[ATTRIBUTE_TITLE].get()).c_str(), String::UTF8));

        if (track[ATTRIBUTE_ARTIST])
            tag->setArtist  (String (boost::get<std::string>(track[ATTRIBUTE_ARTIST].get()).c_str(), String::UTF8));

        if (track[ATTRIBUTE_ALBUM])
            tag->setAlbum   (String (boost::get<std::string>(track[ATTRIBUTE_ALBUM].get()).c_str(), String::UTF8));

        if (track[ATTRIBUTE_COMMENT])
            tag->setComment (String (boost::get<std::string>(track[ATTRIBUTE_COMMENT].get()).c_str(), String::UTF8));

        if (track[ATTRIBUTE_GENRE])
            tag->setGenre   (String (boost::get<std::string>(track[ATTRIBUTE_GENRE].get()).c_str(), String::UTF8));

        if (track[ATTRIBUTE_DATE])
            tag->setYear    (boost::get<guint>(track[ATTRIBUTE_DATE].get()));

        if (track[ATTRIBUTE_TRACK])
            tag->setTrack   (boost::get<guint>(track[ATTRIBUTE_TRACK].get()));
      }
      else
      {
        try{
          g_critical ("%s: Couldn't get File's Tag: %s", G_STRLOC, Glib::filename_from_uri (boost::get<std::string>(track[ATTRIBUTE_LOCATION].get())).c_str());
        } catch (Glib::ConvertError & cxe) {}
      }
    }

} // end namespace MPX
