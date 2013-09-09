dnl Process this file with autoconf to produce a configure script.

AC_INIT(libconfig, 1.4.9, hyperrealm@gmail.com, libconfig,
	[http://www.hyperrealm.com/main.php?s=libconfig])
#AC_CANONICAL_TARGET

AC_MSG_NOTICE([libconfig - made with pride in California])

AC_PROG_MAKE_SET

AC_DEFINE_UNQUOTED(TARGET, "${target}", [Configured target name.])

# Enable GNU extensions.
AC_GNU_SOURCE

dnl Checks for programs.
AC_PROG_CC

AC_PROG_LEX
if test Z"$LEX" != Zflex; then
cat <<EOF
*******************************************************************
You do not seem to have flex. While flex is not required to build
libconfig, you may need it to regenerate the lexer if you change
the scanner.l file. Flex 2.5.33 or newer is recommended.
*******************************************************************
EOF
fi

AC_PROG_YACC
if test Z"$YACC" != "Zbison -y"; then
cat <<EOF
*******************************************************************
You do not seem to have bison. While bison is not required to build
libconfig, you may need it to regenerate the parser if you change
the grammar.y file.
*******************************************************************
EOF
fi

AC_MSG_CHECKING([for compiler switch to enable full C/C++ warnings]);

dnl Checks for libraries

dnl Enable warnings, if we can determine an appropriate switch...
case "${CC}" in

gcc)
	warn_c_sw="-Wall -Wshadow -Wextra -Wdeclaration-after-statement -Wno-unused-parameter";;
*)
	warn_c_sw="";;
esac

if test -n "${warn_c_sw}";
then
	CFLAGS="${CFLAGS} ${warn_c_sw}";
	AC_MSG_RESULT([${warn_c_sw}])
else
	AC_MSG_RESULT([(cannot determine)])
fi;


dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(unistd.h stdint.h xlocale.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

dnl Checks for functions

AC_CHECK_FUNCS([newlocale uselocale freelocale])

dnl Checks for library functions.

AC_OUTPUT(
	doc/Makefile
	gen/Makefile
	)
