# Check for locales support

AC_DEFUN([CHECK_FOR_LOCALES_SUPPORT], [
    AC_CHECK_HEADER([locale.h], [], [AC_MSG_ERROR([Header file locale.h is not found])])
    AC_CHECK_HEADER([ctype.h], [], [AC_MSG_ERROR([Header file ctype.h is not found])])
    AC_CHECK_FUNC([setlocale], [], [AC_MSG_ERROR([Function setlocale() is not available])])
])
