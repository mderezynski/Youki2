// (C) 2006 M. Derezynski

#ifndef TAGLIB_MODFILE_H
#define TAGLIB_MODFILE_H

#include <modplug.h>

#include <tfile.h>
#include <tag.h>
#include "modproperties.h"
#include "modtag.h"

namespace TagLib
{
  namespace MOD
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
           * Returns the MOD::Tag for this file. 
           */
          virtual Tag *MODTag() const;

          /*!
           * Returns the MOD::Properties for this file. 
           */
          virtual Properties *audioProperties() const;

          virtual bool save ()
          { return false; }

          virtual bool readOnly () const
          { return true; }
        
        private:

          void read (const char * file, bool readProperties, Properties::ReadStyle /*propertiesStyle*/);
    };
  }
}  

#endif
