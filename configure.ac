AC_PREREQ([2.69])
AC_INIT([volans], [1.0-dev], [http://github.org/halderen/volans/])

AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_MACRO_DIR([m4])
ACX_PREFIXHACK

AC_USE_SYSTEM_EXTENSIONS

AC_CONFIG_SRCDIR([opendnssec.c])
AC_CONFIG_HEADERS([config.h])

AC_PROG_CC

dnl CHECK_COMPILER_FLAG(std=c99, [C99FLAG="-std=c99"])
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h])
AC_CHECK_FUNCS([strrchr])
ACX_CUNIT
ACX_PEDANTIC
ACX_STRICT
AX_C___ATTRIBUTE__
AM_PROG_CC_C_O

AM_CONDITIONAL([WITH_CUNIT], [test "${with_cunit}" != "no" -a -n "$CUNIT_LIBS"])
AX_LIB_MYSQL()

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
