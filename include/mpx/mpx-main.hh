#ifndef MPX_MAIN_HH 
#define MPX_MAIN_HH

#include <mcs/mcs.h>
#include "mpx/mpx-services.hh"

namespace MPX
{
	extern Mcs::Config      * mcs;
    	extern Mcs::Bind        * mcs_bind;
    	extern Service::Manager * services;
}

#endif // MPX_MAIN_HH
