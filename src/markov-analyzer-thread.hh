//
// markov-analyzer-thread
//
// Authors:
//     Milosz Derezynski <milosz@backtrace.info>
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
#ifndef MPX_MARKOV_ANALYZER_HH
#define MPX_MARKOV_ANALYZER_HH
#include <glibmm.h>
#include "library.hh"
#include "mpx/mpx-services.hh"
#include "mpx/mpx-types.hh"
#include <deque>

namespace MPX
{
	class MarkovAnalyzer : public Service::Base
	{
        public:	

            MarkovAnalyzer () ;
            ~MarkovAnalyzer () ;

            void append (MPX::Track const& track) ;

        protected:

            bool process_idle();

            void
            process_tracks(
                MPX::Track const& track1,
                MPX::Track const& track2
            );

        private:
    
            typedef std::deque<MPX::Track>  TrackQueue_t;

            TrackQueue_t                    m_trackQueue;
            sigc::connection                m_idleConnection;
            Glib::Mutex                     m_queueLock;
	};
}

#endif
