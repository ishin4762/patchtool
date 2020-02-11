#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([patchtool], [0.1])
LT_INIT
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([1.9])

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lbz2':
AC_CHECK_LIB([bz2], [BZ2_bzReadOpen])

AC_CHECK_HEADERS([fcntl.h limits.h stddef.h stdint.h stdlib.h string.h unistd.h])

# Check for windows.
build_linux=no
build_windows=no
build_mac=no

# Detect the target system
case "${host_os}" in
 linux*)
   build_linux=yes
 ;;
 cygwin*|mingw*)
   build_windows=yes
 ;;
 darwin*)
   build_mac=yes
 ;;
 *)
   AC_MSG_ERROR(["OS $host_os is not supported"])
 ;;
esac

# Pass the conditionals to automake
AM_CONDITIONAL([LINUX], [test "$build_linux" = "yes"])
AM_CONDITIONAL([WINDOWS], [test "$build_windows" = "yes"])
AM_CONDITIONAL([OSX], [test "$build_mac" = "yes"])


# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT64_T
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset])

AC_CONFIG_FILES([Makefile
                 lib/Makefile
                 lib/bsdiff/Makefile
                 lib/patchfile/Makefile
                 patchgen/Makefile
                 patchapply/Makefile])
AC_OUTPUT
