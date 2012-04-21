#ifndef MCS_H
#define MCS_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifndef HAVE_BMP
#  include <mcs/config.h>
#endif

#include <mcs/types.h>
#include <mcs/subscriber.h>
#include <mcs/key.h>

#ifndef HAVE_BMP

#if (1 == MCS_HAVE_XML)
#  include <mcs/mcs.h>
#endif

#if (1 == MCS_HAVE_GTK)
# include <mcs/gtk-bind.h>

#endif

#else

#ifdef HAVE_XML
#  include <mcs/mcs.h>
#endif
#ifdef HAVE_GTK
#  include <mcs/gtk-bind.h>
#endif

#endif

#endif // MCS_H
