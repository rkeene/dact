AC_DEFUN(DC_PROG_GCC, [
  AC_SUBST(DEPEND)

  if test -n "$GCC"; then
    CFLAGS="$CFLAGS -Wall"
    DEPEND="Makefile.dep"
  fi
])

AC_DEFUN(DC_CHK_URANDOM, [
  if test "$cross_compiling" = "no"; then
    AC_MSG_CHECKING(to see if you have /dev/urandom)
    if test -r "/dev/urandom"; then
      AC_MSG_RESULT(yes)
      AC_DEFINE([HAVE_DEV_URANDOM], [], [Have random device?])
    else
      AC_MSG_RESULT(no)
    fi
  fi
])

AC_DEFUN(DC_CHK_MODULE, [
  AC_SUBST(ALGO)
  AC_SUBST(MODLIBS)

  AC_CHECK_LIB($1, main, [
    ALGO="$2 $ALGO"
    MODLIBS="-l$1 $MODLIBS"
    AC_DEFINE($3, [], [Something])
  ])
])

AC_DEFUN(DC_CHK_MODULE_VAR, [
  AC_SUBST(ALGO)
  AC_SUBST($4)

  AC_CHECK_LIB($1, main, [
    ALGO="$2 $ALGO"
    $4="-l$1 $(eval echo $(echo \$$4))"
    AC_DEFINE($3, [], [Also something])
  ])
])

AC_DEFUN(DC_DO_WIN32, [
  AC_CHECK_HEADERS(windows.h windowsx.h winsock2.h)
  AC_CHECK_LIB(wsock32, main, [
    AC_DEFINE(HAVE_LIBWSOCK32, [], [Have libwsock32])
      LIBS="${LIBS} -lwsock32"
  ])
])

AC_DEFUN(DC_DO_NETSET, [
  AC_SEARCH_LIBS(socket, socket nsl ws2_32 wsock32, AC_DEFINE(HAVE_SOCKET, [], [Have socket()]))
  AC_SEARCH_LIBS(gethostbyname, nsl ws2_32 wsock32, AC_DEFINE(HAVE_GETHOSTBYNAME, [], [Have gethostbyname()]))
  AC_SEARCH_LIBS(inet_aton, xnet ws2_32 wsock32, AC_DEFINE(HAVE_INET_ATON, [], [Have inet_aton()]), AC_SEARCH_LIBS(inet_addr, nsl ws2_32 wsock32, AC_DEFINE(HAVE_INET_ADDR, [], [Have inet_addr()])))
])


AC_DEFUN(DC_ASK_CHKVERS, [
  AC_ARG_ENABLE(chkvers, [  --disable-chkvers       Completely disable the ability to check for new versions], [
    case "$enableval" in
      no)
      ;;
      *)
  AC_DEFINE(CHECK_VERSION, [], [Check for new versions over the network?])
      ;;
    esac
  ], AC_DEFINE(CHECK_VERSION, [], [Check for new versions over the network?]))
])

AC_DEFUN(DC_ASK_DEBIAN, [
  AC_ARG_ENABLE(debianupgrade, [  --enable-debianupgrade  Use the Debian upgrade procedure instead of the DACT one], [
    case "$enableval" in
      no)
      ;;
      *)
  		AC_DEFINE(DACT_DEBIAN_UPGRADE_PROC, [], [Use the Debian upgrade procedure instead of DACTs internal one])
      ;;
    esac
  ])
])

AC_DEFUN(DC_ASK_NETWORK, [
  AC_ARG_ENABLE(network, [  --disable-network       Disable DACT's network activity completely], [
    case "$enableval" in
      no)
	AC_DEFINE(NO_NETWORK, [], [Disable all network support])
      ;;
      *)
	DC_DO_NETSET
      ;;
    esac
  ], DC_DO_NETSET )
])

AC_DEFUN(DC_DO_DEBUG, [
  ALGO="$ALGO \$(DEBUGALGO)"
  AC_DEFINE(DEBUG, [1], [Debug])
])

AC_DEFUN(DC_ASK_DEBUG, [
  AC_ARG_ENABLE(debug, [  --enable-debug          Enable developmental code], [
    if test "$enable_version" != "no"; then
      DC_DO_DEBUG
    fi
  ], [
    case "`uname -n`" in
      rkeene | unleaded | *.saurik.com )
	DC_DO_DEBUG
      ;;
    esac
  ])
])

AC_DEFUN(DC_NO_MODULE, [
  DEFAULT="static"
  MODS="# "
])

AC_DEFUN(DC_GET_SHARED, [
  AC_SUBST(SHARED)

  OLD_LDFLAGS="$LDFLAGS"
  SHARED=""
  if test -z "$SHARED"; then
    LDFLAGS="$OLD_LDFLAGS -shared -Wl,-G,-z,textoff -rdynamic -fPIC"
    AC_TRY_LINK([#include <stdio.h>
int unrestst(void);], [ printf("okay\n"); unrestst(); return(0); ], [ SHARED="-shared -Wl,-G,-z,textoff -fPIC"; ADD_LDFLAGS="-rdynamic" ], SHARED="", SHARED="");
  fi
  if test -z "$SHARED"; then
    LDFLAGS="$OLD_LDFLAGS -shared -Wl,-G,-z,textoff -rdynamic"
    AC_TRY_LINK([#include <stdio.h>
int unrestst(void);], [ printf("okay\n"); unrestst(); return(0); ], [ SHARED="-shared -Wl,-G,-z,textoff"; ADD_LDFLAGS="-rdynamic" ], SHARED="", SHARED="");
  fi
  if test -z "$SHARED"; then
    LDFLAGS="$OLD_LDFLAGS -shared -Wl,-G,-z,textoff -fPIC"
    AC_TRY_LINK([#include <stdio.h>
int unrestst(void);], [ printf("okay\n"); unrestst(); return(0); ], [ SHARED="-shared -Wl,-G,-z,textoff -fPIC"; ADD_LDFLAGS="" ], SHARED="", SHARED="");
  fi
  if test -z "$SHARED"; then
    LDFLAGS="$OLD_LDFLAGS -shared -Wl,-G,-z,textoff"
    AC_TRY_LINK([#include <stdio.h>
int unrestst(void);], [ printf("okay\n"); unrestst(); return(0); ], [ SHARED="-shared -Wl,-G,-z,textoff"; ADD_LDFLAGS="" ], SHARED="", SHARED="");
  fi
  if test -z "$SHARED"; then
    LDFLAGS="$OLD_LDFLAGS -Wl,-G,-z,textoff -rdynamic -fPIC"
    AC_TRY_LINK([#include <stdio.h>
int unrestst(void);], [ printf("okay\n"); unrestst(); return(0); ], [ SHARED="-Wl,-G,-z,textoff -fPIC"; ADD_LDFLAGS="-rdynamic" ], SHARED="", SHARED="");
  fi
  if test -z "$SHARED"; then
    LDFLAGS="$OLD_LDFLAGS -Wl,-G,-z,textoff -rdynamic"
    AC_TRY_LINK([#include <stdio.h>
int unrestst(void);], [ printf("okay\n"); unrestst(); return(0); ], [ SHARED="-Wl,-G,-z,textoff"; ADD_LDFLAGS="-rdynamic" ], SHARED="", SHARED="");
  fi
  if test -z "$SHARED"; then
    LDFLAGS="$OLD_LDFLAGS -Wl,-G,-z,textoff -fPIC"
    AC_TRY_LINK([#include <stdio.h>
int unrestst(void);], [ printf("okay\n"); unrestst(); return(0); ], [ SHARED="-Wl,-G,-z,textoff -fPIC"; ADD_LDFLAGS="" ], SHARED="", SHARED="");
  fi
  if test -z "$SHARED"; then
    LDFLAGS="$OLD_LDFLAGS -Wl,-G,-z,textoff"
    AC_TRY_LINK([#include <stdio.h>
int unrestst(void);], [ printf("okay\n"); unrestst(); return(0); ], [ SHARED="-Wl,-G,-z,textoff"; ADD_LDFLAGS="" ], SHARED="", SHARED="");
  fi
  if test -z "$SHARED"; then
    LDFLAGS="$LDFLAGS -shared -rdynamic -fPIC"
    AC_TRY_LINK([#include <stdio.h>
int unrestst(void);], [ printf("okay\n"); unrestst(); return(0); ], [ SHARED="-shared -fPIC"; ADD_LDFLAGS="-rdynamic" ], SHARED="", SHARED="");
  fi
  if test -z "$SHARED"; then
    LDFLAGS="$OLD_LDFLAGS -shared -fPIC"
    AC_TRY_LINK([#include <stdio.h>
int unrestst(void);], [ printf("okay\n"); unrestst(); return(0); ], [ SHARED="-shared -fPIC"; ADD_LDFLAGS="" ], SHARED="", SHARED="");
  fi
  if test -z "$SHARED"; then
    LDFLAGS="$OLD_LDFLAGS -shared"
    AC_TRY_LINK([#include <stdio.h>
int unrestst(void);], [ printf("okay\n"); unrestst(); return(0); ], [ SHARED="-shared"; ADD_LDFLAGS="" ], SHARED="", SHARED="");
  fi
  LDFLAGS="$ADD_LDFLAGS $OLD_LDFLAGS"

  
])


AC_DEFUN(DC_ASK_MODULE, [
  AC_SUBST(DEFAULT)
  AC_SUBST(MODS)
  AC_SUBST(NOMODS)

  DC_CHK_DLOPEN
  AC_MSG_CHECKING(to use modules)
  AC_ARG_ENABLE(modules, [  --disable-modules       Disable use of modules], [
    case "$enableval" in
      no)
	AC_MSG_RESULT(no)
	DC_NO_MODULE
      ;;
      *)
	DC_GET_SHARED
	if test "$HAVEDLOPEN" = yes -a -n "$SHARED"; then
	  AC_MSG_RESULT(yes)
	  AC_DEFINE(USE_MODULES, [], [Enable use of dynamically loadable modules?])
          NOMODS="#"
	  DEFAULT="module"
	else
	  AC_MSG_RESULT(can't)
	  DC_NO_MODULE
	fi
      ;;
    esac
  ], [
	DC_GET_SHARED
	if test "$HAVEDLOPEN" = yes -a -n "$SHARED"; then
	  AC_MSG_RESULT(yes)
	  AC_DEFINE(USE_MODULES, [Enable use of dynamically loadable modules?])
          NOMODS="#"
	  DEFAULT="module"
	else
	  AC_MSG_RESULT(can't)
	  DC_NO_MODULE
	fi
  ])
])

AC_DEFUN(DC_DO_TYPE, [
	if test -z "$ac_cv_sizeof_long"; then
		AC_C_INLINE
		AC_CHECK_SIZEOF(long, 4)
		AC_CHECK_SIZEOF(int, 4)
		AC_CHECK_SIZEOF(short, 2)
	fi
	FOUND=0
	for dc_cv_loop in \$ac_cv_sizeof_int \$ac_cv_sizeof_long \$ac_cv_sizeof_short; do
		dc_cv_size=`eval echo $dc_cv_loop`
		dc_cv_name=`echo $dc_cv_loop | sed s/\\\$ac_cv_sizeof_//`
		if test "$dc_cv_size" = "$3"; then
			if test "$dc_cv_name" = "int"; then 
				AC_CHECK_TYPE($1, $2 int)
			fi
			if test "$dc_cv_name" = "long"; then
				AC_CHECK_TYPE($1, $2 long)
			fi
			if test "$dc_cv_name" = "short"; then
				AC_CHECK_TYPE($1, $2 short)
			fi
			FOUND=1
			break
		fi
	done
])

AC_DEFUN(DC_CHK_DLOPEN, [
	AC_CHECK_FUNC(dlopen, [
		AC_DEFINE(HAVE_DLOPEN, [1], [Have dlopen()?])
		HAVEDLOPEN=yes
	])
	if test "$ac_cv_func_dlopen" = "no"; then
		AC_CHECK_LIB(dl, dlopen, [
			AC_DEFINE(HAVE_DLOPEN, [1], [Have dlopen()?])
			ALLMODLIBS=-ldl
			AC_SUBST(ALLMODLIBS)
			HAVEDLOPEN=yes
		])
	fi
])
