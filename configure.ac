AC_INIT(dact, 0.9.0)
AC_CONFIG_HEADERS(config.h)

dnl Locate standard tools
AC_PROG_CC
AC_PROG_MAKE_SET
AC_PROG_INSTALL
AC_AIX
AC_LANG(C)

dnl Determine this platform
DC_CHK_OS_INFO

dnl Determine how to create shared objects on this platform
DC_GET_SHOBJFLAGS

dnl Determine how to create static archives on this platform
AC_CHECK_TOOL(AR, ar)
AC_CHECK_TOOL(RANLIB, ranlib)

dnl Determine how to strip executables and remove symbols
DC_FIND_STRIP_AND_REMOVESYMS(libdact.syms)

dnl Set version script, to limit the scope of symbols
DC_SETVERSIONSCRIPT(libdact.vers, libdact.syms)

dnl Upate LDFLAGS to include setting the run-time linker path to the same as our compile-time linker
DC_SYNC_RPATH

dnl If we updated LIBOBJS, update SHLIBOBJS -- must be last.
DC_SYNC_SHLIBOBJS

dnl Produce Makefile
AC_OUTPUT(Makefile libdact.syms)
