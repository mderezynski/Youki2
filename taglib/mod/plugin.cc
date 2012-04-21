#include "config.h"

#include "resolver.h"
#include "modfile.h"
#include "modtag.h"

extern "C" int  _plugin_has_accessors;
int _plugin_has_accessors = 1;

extern "C" int  _plugin_version;
int _plugin_version = PLUGIN_VERSION;

extern "C" const char ** _mimetypes ()
{
  static const char * _types[] =
  {
    "audio/x-mod",
    NULL
  };
  return _types;
}

using namespace MPX;
using namespace TagLib;

extern "C" bool _get (std::string const& filename, Track & track)  
{
  MOD::File opfile (filename.c_str());  

  track[ATTRIBUTE_TITLE] = opfile.MODTag()->title().to8Bit(true);
  track[ATTRIBUTE_GENRE] = opfile.MODTag()->genre().to8Bit(true);

  return true;
}
static void _plugin_init (void) __attribute__((constructor));

static void _plugin_init (void)
{
  TagLib::FileRef::addFileTypeResolver(new MODFileTypeResolver);
}
