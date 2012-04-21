#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif //HAVE_CONFIG_H

#include <fileref.h>
// Plugin-specific include
#include "resolver.h"

extern "C" int  _plugin_has_accessors;
int _plugin_has_accessors = 1;

extern "C" int  _plugin_version;
int _plugin_version = PLUGIN_VERSION;

extern "C" const char ** _mimetypes ()
{
  static const char * _types[] =
  {
    "audio/x-sid",
    NULL
  };
  return _types;
}

static void _plugin_init (void) __attribute__((constructor));

static void _plugin_init (void)
{
  TagLib::FileRef::addFileTypeResolver(new SIDFileTypeResolver);
}
