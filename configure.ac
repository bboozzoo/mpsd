#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(mpsd, 0.0.1, maciek.borzecki@gmail.com)
AC_CONFIG_SRCDIR([scan.h])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CC
AM_PROG_LEX

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h string.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_HEADER_TIME
AC_TYPE_UINT32_T

# Checks for library functions.
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([gethostbyname inet_ntoa memset select socket strdup strerror])

# Extra config options
AC_ARG_ENABLE(dynamic-modules, [  --enable-dynamic-modules], [use_dyn=yes], [use_dyn=no])
if test "$use_dynamic" = "yes"; then
    AC_DEFINE(CORE_LOAD_DYNAMIC, 1, [support for dynamically loadable modules])
else
    AC_DEFINE(CORE_LOAD_STATIC, 1, [load modules statically])
fi

modules=""
# Extra features
AC_ARG_WITH(tftp, [  --with-tftp], [use_tftp=yes])
if test "$use_tftp" = "yes"; then
    AC_DEFINE(MODULE_TFTP, 1, [enable TFTP module])
    modules="$modules tftp"
fi
AC_ARG_WITH(ftp, [  --with-ftp], [use_tftp=yes])
if test "$use_ftp" = "yes"; then
    AC_DEFINE(MODULE_FTP, 1, [enable FTP module])
    modules="$modules ftp"
fi

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

AC_MSG_NOTICE( )
AC_MSG_NOTICE(enabled modules: $modules)
AC_MSG_NOTICE( )
