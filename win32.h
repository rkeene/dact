#if !defined(_LOCAL_WIN32_H) && defined(__WIN32__)
#define _LOCAL_WIN32_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef __MINGW32__
     /* MingW32 specific stuff here. */
#  if defined(HAVE_WINSOCK2_H) && defined(HAVE_LIBWSOCK32)
     /* We have to override the detected configuration
        because it can't detect the network libraries. */
#    define HAVE_GETHOSTBYNAME 1
#    define HAVE_INET_ADDR 1
#    define HAVE_SOCKET 1
#  endif
#else
   /* MSVC++ configuration follows */
#  undef HAVE_UNISTD_H
#  define HAVE_STDLIB_H 1
#  define HAVE_WINDOWS_H 1
#  define HAVE_STDARG_H 1
#endif /* __MINGW32__ */

#ifdef HAVE_WINDOWS_H
#  include <windows.h>
#endif
#ifdef HAVE_WINSOCK2_H
#  include <winsock2.h>
#endif



#endif /* _LOCAL_WIN32_H */
