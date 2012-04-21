// (C) 2006 M. Derezynski

#ifndef TAGLIB_SIDFILE_H
#define TAGLIB_SIDFILE_H

#include <sidplay/sidtune.h>

#include <tfile.h>
#include <tag.h>
#include "sidproperties.h"
#include "sidtag.h"

namespace TagLib
{
  namespace SID
  {
    class File : public TagLib::File
    {
        
        friend class Attribute;

        public:

          File (const char *file, bool readProperties = true, Properties::ReadStyle propertiesStyle = Properties::Average);
          virtual ~File();
        
          /*!
           * Returns the TagLib::Tag for this file. 
           */
          virtual TagLib::Tag *tag() const;

          /*!
           * Returns the SID::Tag for this file. 
           */
          virtual Tag *SIDTag() const;

          /*!
           * Returns the SID::Properties for this file. 
           */
          virtual Properties *audioProperties() const;

          virtual bool save ()
          { return false; }

          virtual bool readOnly () const
          {
            return true;
          }
        
        private:

          void read (bool readProperties, Properties::ReadStyle /*propertiesStyle*/);
         
          sidTune *tune; 
          sidTuneInfo info;
    };
  }
}  

#endif
