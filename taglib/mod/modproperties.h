/***************************************************************************
    copyright            : (C) 2005 by Lukas Lalinsky
    email                : lalinsky@gmail.com
 ***************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it  under the terms of the GNU Lesser General Public License version  *
 *   2.1 as published by the Free Software Foundation.                     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  *
 *   USA                                                                   *
 ***************************************************************************/

#ifndef TAGLIB_MODPROPERTIES_H
#define TAGLIB_MODPROPERTIES_H

#include <audioproperties.h>
#include <tstring.h>

namespace TagLib
{
  namespace MOD
  {
    class File;  
    class Properties : public AudioProperties
    {
      friend class MOD::File;
      public:
      
        /*!
         * Create an instance of MOD::Properties.
         */
        Properties();
        
        /*!
         * Destroys this MOD::Properties instance.
         */
        virtual ~Properties();

        // Reimplementations.
        virtual int length() const;


        virtual int bitrate() const
        { return 0; }

        virtual int sampleRate() const
        { return 0; }

        virtual int channels() const
        { return 0; }


      private:
    
        void set (int length);
        int length_;

    };
  }
}

#endif 
