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

#ifndef TAGLIB_SIDPROPERTIES_H
#define TAGLIB_SIDPROPERTIES_H

#include <audioproperties.h>
#include <tstring.h>

namespace TagLib
{
  namespace SID
  {
    class File;  
    class Properties : public AudioProperties
    {
      friend class SID::File;
      public:
      
        /*!
         * Create an instance of SID::Properties.
         */
        Properties();
        
        /*!
         * Destroys this SID::Properties instance.
         */
        virtual ~Properties();

        // Reimplementations.

        virtual int length() const;
        virtual int bitrate() const;
        virtual int sampleRate() const;
        virtual int channels() const;

      private:
    
        void set (int length, int bitrate, int sampleRate, int channels);

        int length_;
        int bitrate_;
        int sampleRate_;
        int channels_;

    };
  }
}

#endif 
