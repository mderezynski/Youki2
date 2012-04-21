#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif //HAVE_CONFIG_H

#include <fileref.h>

// Plugin-specific include
#include "resolver.h"

static void _plugin_init (void) __attribute__((constructor));

static void _plugin_init (void)
{
  TagLib::FileRef::addFileTypeResolver(new AudibleFileTypeResolver);
}
