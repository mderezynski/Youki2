// take over from sigc++
#include <sigc++config.h>


// Detect common platforms
#if defined(_WIN32)
  // Win32 compilers have a lot of variation
  #if defined(_MSC_VER)
    #define SIGX_MSC
    #define SIGX_WIN32
    #define SIGX_DLL
  #elif defined(__CYGWIN__)
  #elif defined(__MINGW32__)
    #define SIGX_WIN32
  #else
    //The Tru64 compiler complains about this "unrecognized preprocessing directive", but it should never get this far anyway.
    //#warning "libsigc++ config: Unknown win32 architecture (send me gcc --dumpspecs or equiv)"
  #endif
#else
#endif /* _WIN32 */



#ifdef SIGX_MSC
  // C++ exception specification ignored except to indicate a function is not __declspec(nothrow)
  #pragma warning(disable: 4290)
#endif

#ifdef SIGX_DLL
  #if defined(SIGX_BUILD) && defined(_WINDLL)
    #define SIGX_API __declspec(dllexport)
  #elif !defined(SIGX_BUILD)
    #define SIGX_API __declspec(dllimport)
  #else
    #define SIGX_API
  #endif /* SIGX_BUILD - _WINDLL */
#else
  #define SIGX_API
#endif /* SIGX_DLL */
