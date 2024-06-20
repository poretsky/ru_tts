# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.69)

AC_INIT([ru-tts], [6.2.3])
AC_CONFIG_SRCDIR([src/ru_tts.c])
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_MACRO_DIRS([m4])
AM_INIT_AUTOMAKE([foreign])
LT_INIT

# Guess environment.
AC_CANONICAL_HOST
AC_CANONICAL_BUILD

# Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL

# Language choice.
AC_LANG([C])
AX_CHECK_VSCRIPT

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for required headers and libraries.
AC_CHECK_HEADERS([stdint.h stdlib.h unistd.h string.h stdio.h fcntl.h math.h limits.h sys/types.h sys/stat.h], [],
                 [AC_MSG_ERROR([Some system header files are not found])])
AC_SEARCH_LIBS([rint], [m], [], [AC_MSG_ERROR([The rint() function is unavailable])])

# Cooperation with the Rulex pronunciation dictionary
AC_ARG_WITH([dictionary],
            AS_HELP_STRING([--with-dictionary],
                           [whether to link with the rulex library to take advantage of the pronunciation dictionary
                            (by default the library is not linked in but dlopened at runtime).]))

AS_IF([test "x$with_dictionary" = "xyes"], [
    CHECK_FOR_LOCALES_SUPPORT
    AC_CHECK_HEADER([rulexdb.h], [], [AC_MSG_ERROR([Header file rulexdb.h is not found])])
    AC_SEARCH_LIBS([rulexdb_search], [rulexdb], [], [AC_MSG_ERROR([Broken or unavailable rulexdb library])])
], [test "x$with_dictionary" != "xno"], [
    CHECK_FOR_LOCALES_SUPPORT
    AC_CHECK_HEADER([dlfcn.h], [], [AC_MSG_ERROR([Header file dlfcn.h is not found])])
    AC_SEARCH_LIBS([dlopen], [dl], [], [AC_MSG_ERROR([The dlopen() function is unavailable])])
    AC_DEFINE([RULEX_DLL], ["librulexdb.so.0"], [RuLex DLL])
], [
    AC_DEFINE([WITHOUT_DICTIONARY], [], [Drop RuLex dictionary support])
])

# Output results.
AC_CONFIG_FILES([Makefile src/Makefile manpages/Makefile])
AC_OUTPUT
