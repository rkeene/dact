AC_DEFUN(DC_PROG_GCC, [
  AC_SUBST(DEPEND)

  if test -n "$GCC"; then
    CFLAGS="$CFLAGS -Wall"
    DEPEND="Makefile.dep"
  fi
])

AC_DEFUN(DC_DO_NETSET, [
  AC_SEARCH_LIBS(socket, socket nsl)
  AC_SEARCH_LIBS(gethostbyname, nsl)
  AC_SEARCH_LIBS(inet_aton, xnet, AC_DEFINE(HAVE_INET_ATON), AC_SEARCH_LIBS(inet_addr, nsl))
])

AC_DEFUN(DC_ASK_NETWORK, [
  AC_ARG_ENABLE(network, [  --disable-network], [
    case "$enableval" in
      no)
	AC_DEFINE(NO_NETWORK)
      ;;
      *)
	DC_DO_NETSET
      ;;
    esac
  ], DC_DO_NETSET )
])

AC_DEFUN(DC_DO_DEBUG, [
  LIBOBJS="$LIBOBJS"
  ALGO="$ALGO"
  AC_DEFINE(DEBUG)
])

AC_DEFUN(DC_ASK_DEBUG, [
  AC_ARG_ENABLE(debug, [  --enable-debug], [
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
