AC_DEFUN(DC_DO_NETSET, [
  AC_SEARCH_LIBS(socket, socket nsl)
  AC_SEARCH_LIBS(gethostbyname, nsl)
  AC_SEARCH_LIBS(inet_aton, xnet, AC_DEFINE(HAVE_INET_ATON), AC_SEARCH_LIBS(inet_addr, nsl))
])
