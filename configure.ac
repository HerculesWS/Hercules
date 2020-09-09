#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# This file is part of Hercules.
# http://herc.ws - http://github.com/HerculesWS/Hercules
#
# Copyright (C) 2012-2023 Hercules Dev Team
# Copyright (C) Athena Dev Teams
#
# Hercules is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

AC_INIT(Hercules)
AC_REVISION([m4_esyscmd_s([type git >/dev/null 2>&1 && git describe --always 2>/dev/null || echo '(unknown version)'])])
AC_PREREQ([2.69])
AC_CONFIG_SRCDIR([src/common/cbasetypes.h])
AC_CONFIG_FILES([Makefile src/common/Makefile])
AC_CONFIG_FILES([3rdparty/mt19937ar/Makefile 3rdparty/libconfig/Makefile 3rdparty/libbacktrace/Makefile 3rdparty/libbacktrace/backtrace-supported.h])
AC_CONFIG_FILES([3rdparty/http-parser/Makefile 3rdparty/multipart-parser/Makefile 3rdparty/cJSON/Makefile])
AC_CONFIG_FILES([src/char/Makefile src/login/Makefile])
AC_CONFIG_FILES([src/map/Makefile src/plugins/Makefile])
AC_CONFIG_FILES([src/api/Makefile])
AC_CONFIG_FILES([src/test/Makefile])
AC_CONFIG_FILES([tools/HPMHookGen/Makefile])
AC_CONFIG_FILES([tools/doxygen/Makefile])

dnl AC_USE_SYSTEM_EXTENSIONS requires autoconf 2.60 or newer. Fall back to AC_GNU_SOURCE otherwise.
m4_ifdef([AC_USE_SYSTEM_EXTENSIONS],
	[AC_USE_SYSTEM_EXTENSIONS],
	[AC_GNU_SOURCE]
)

AC_MSG_CHECKING([host OS])
host_os="`uname`"
AC_MSG_RESULT([$host_os])

# Root-check
case $host_os in
CYGWIN*)
	;;
*)
	if type id >/dev/null 2>&1 && test -z "$CI"; then
		euid="`id -u`"
		if test "$euid" == "0"; then
			echo ""
			echo "********************************************************************************"
			echo '*                            W A R N I N G      /!\                            *'
			echo "********************************************************************************"
			echo "* It appears that you're planning to run Hercules with root privileges. That's *"
			echo "* not necessary, nor recommended, and it may open your machine to unnecessary  *"
			echo "* security risks.  You should never ever run software as root unless it        *"
			echo "* requires the extra privileges (which Hercules does not.)                     *"
			echo "*                                                                              *"
			echo "* More info:                                                                   *"
			echo "* http://www.tldp.org/HOWTO/Security-HOWTO/local-security.html                 *"
			echo "* https://wiki.debian.org/sudo                                                 *"
			echo "* http://wiki.centos.org/TipsAndTricks/BecomingRoot                            *"
			echo "* http://fedoraproject.org/wiki/Configuring_Sudo                               *"
			echo "* https://help.ubuntu.com/community/RootSudo                                   *"
			echo "* http://www.freebsdwiki.net/index.php/Root                                    *"
			echo "*                                                                              *"
			echo "* If your service provider forces (or encourages) you to run server software   *"
			echo "* as root, please complain to them. It is a very bad idea.                     *"
			echo "********************************************************************************"
			echo "Execution will be paused for 60 seconds... Press Ctrl-C now if you wish to stop."
			for j in 1 2 3 4 5 6; do
				for i in 1 2 3 4 5 6 7 8 9 10; do
					printf "\a. "
					sleep 1
				done
				echo ""
			done
			echo ""
			echo "Resuming as root. If anything breaks, you'll get to keep the pieces."
			sleep 2
		fi
	fi
	;;
esac

#
# Memory managers
#
AC_ARG_ENABLE(
	[manager],
	AS_HELP_STRING(
		[--enable-manager=ARG],
		[memory managers: no, builtin, memwatch, dmalloc, gcollect, bcheck (defaults to builtin)]
	),
	[
		enable_manager="$enableval"
		case $enableval in
			"no");;
			"builtin");;
			"memwatch");;
			"dmalloc");;
			"gcollect");;
			"bcheck");;
			*) AC_MSG_ERROR([[unknown memory manager '$enableval'... stopping]]);;
		esac
	],
	[enable_manager="builtin"]
)


#
# packetver
#
AC_ARG_ENABLE(
	[packetver],
	AS_HELP_STRING(
		[--enable-packetver=ARG],
		[Sets the PACKETVER define. (see src/common/mmo.h)]
	),
	[enable_packetver="$enableval"],
	[enable_packetver=""]
)

#
# cash shop preview patch
#
AC_ARG_ENABLE(
	[cashshop-preview-patch],
	AS_HELP_STRING(
		[--enable-cashshop-preview-patch],
		[Enable Nemo patch ExtendCashShopPreview.]
	),
	[enable_cashshop_preview_patch=1],
	[enable_cashshop_preview_patch=0]
)

#
# old cash shop preview patch
#
AC_ARG_ENABLE(
	[old-cashshop-preview-patch],
	AC_HELP_STRING(
		[--enable-old-cashshop-preview-patch],
		[Enable Nemo patch ExtendOldCashShopPreview.]
	),
	[enable_old_cashshop_preview_patch=1],
	[enable_old_cashshop_preview_patch=0]
)


#
# packetver-RE
#
AC_ARG_ENABLE(
	[packetver-re],
	AS_HELP_STRING(
		[--enable-packetver-re],
		[Sets or unsets the PACKETVER_RE define - see src/common/mmo.h (currently disabled by default)]
	),
	[
		enable_packetver_re="$enableval"
		case $enableval in
			"no");;
			"yes");;
			*) AC_MSG_ERROR([[invalid argument --enable-packetver-re=$enableval... stopping]]);;
		esac
	],
	[enable_packetver_re="no"]
)


#
# packetver-Zero
#
AC_ARG_ENABLE(
	[packetver-zero],
	AS_HELP_STRING(
		[--enable-packetver-zero],
		[Sets or unsets the PACKETVER_ZERO define - see src/common/mmo.h (currently disabled by default)]
	),
	[
		enable_packetver_zero="$enableval"
		case $enableval in
			"no");;
			"yes");;
			*) AC_MSG_ERROR([[invalid argument --enable-packetver-zero=$enableval... stopping]]);;
		esac
	],
	[enable_packetver_zero="no"]
)

#
# packetver-Sak
#
AC_ARG_ENABLE(
	[packetver-sak],
	AS_HELP_STRING(
		[--enable-packetver-sak],
		[Sets or unsets the PACKETVER_SAK define - see src/common/mmo.h (currently disabled by default)]
	),
	[
		enable_packetver_sak="$enableval"
		case $enableval in
			"no");;
			"yes");;
			*) AC_MSG_ERROR([[invalid argument --enable-packetver-sak=$enableval... stopping]]);;
		esac
	],
	[enable_packetver_sak="no"]
)

#
# packetver-Ad
#
AC_ARG_ENABLE(
	[packetver-ad],
	AS_HELP_STRING(
		[--enable-packetver-ad],
		[Sets or unsets the PACKETVER_AD define - see src/common/mmo.h (currently disabled by default)]
	),
	[
		enable_packetver_ad="$enableval"
		case $enableval in
			"no");;
			"yes");;
			*) AC_MSG_ERROR([[invalid argument --enable-packetver-ad=$enableval... stopping]]);;
		esac
	],
	[enable_packetver_ad="no"]
)


#
# Epoll
#
AC_ARG_ENABLE([epoll],
	[AS_HELP_STRING([--enable-epoll],[use epoll(4) on Linux])],
	[enable_epoll=$enableval],
	[enable_epoll=no]
)
if test x$enable_epoll = xno; then
	have_linux_epoll=no
else
	AC_MSG_CHECKING([for Linux epoll(4)])
	AC_LINK_IFELSE([AC_LANG_PROGRAM(
		[
		#ifndef __linux__
		#error This is not Linux
		#endif
		#include <sys/epoll.h>
		],
		[epoll_create1 (EPOLL_CLOEXEC);])],
		[have_linux_epoll=yes],
		[have_linux_epoll=no]
	)
	AC_MSG_RESULT([$have_linux_epoll])
fi
if test x$enable_epoll,$have_linux_epoll = xyes,no; then
    AC_MSG_ERROR([epoll support explicitly enabled but not available])
fi


#
# Obfuscation keys
#
AC_ARG_WITH(
	[key1],
	AS_HELP_STRING(
		[--with-key1@<:@=ARG@:>@],
		[Set the first obfuscation key (ignored unless the other two are also specified)]
	),
	[
		obfuscationkey1="$( expr "0x$withval" : '0*x*\(0x@<:@A-Fa-f0-9@:>@\{8\}\)' )"
		if ! expr "x$obfuscationkey1" : 'x0x@<:@A-Fa-f0-9@:>@\{8\}' >/dev/null 2>&1; then
			obfuscationkey1=""
		fi
	],
	[
		obfuscationkey1=""
	]
)
AC_ARG_WITH(
	[key2],
	AS_HELP_STRING(
		[--with-key2@<:@=ARG@:>@],
		[Set the second obfuscation key (ignored unless the other two are also specified)]
	),
	[
		obfuscationkey2="$( expr "0x$withval" : '0*x*\(0x@<:@A-Fa-f0-9@:>@\{8\}\)' )"
		if ! expr "x$obfuscationkey2" : 'x0x@<:@A-Fa-f0-9@:>@\{8\}' >/dev/null 2>&1; then
			obfuscationkey2=""
		fi
	],
	[
		obfuscationkey2=""
	]
)
AC_ARG_WITH(
	[key3],
	AS_HELP_STRING(
		[--with-key3@<:@=ARG@:>@],
		[Set the third obfuscation key (ignored unless the other two are also specified)]
	),
	[
		obfuscationkey3="$( expr "0x$withval" : '0*x*\(0x@<:@A-Fa-f0-9@:>@\{8\}\)' )"
		if ! expr "x$obfuscationkey3" : 'x0x@<:@A-Fa-f0-9@:>@\{8\}' >/dev/null 2>&1; then
			obfuscationkey3=""
		fi
	],
	[
		obfuscationkey3=""
	]
)


#
# debug
#
AC_ARG_ENABLE(
	[debug],
	AS_HELP_STRING(
		[--enable-debug@<:@=ARG@:>@],
		[
			Compiles extra debug code. (yes by default)
			(available options: yes, no, gdb)
		]
	),
	[
		enable_debug="$enableval"
		case $enableval in
			"no");;
			"yes");;
			"gdb");;
			*) AC_MSG_ERROR([[invalid argument --enable-debug=$enableval... stopping]]);;
		esac
	],
	[enable_debug="yes"]
)

#
# libbacktrace
#
AC_ARG_ENABLE(
	[libbacktrace],
	AS_HELP_STRING(
		[--enable-libbacktrace@<:@=ARG@:>@],
		[
			Compiles with libbacktrace. (no by default - experimental)
		]
	),
	[
		enable_libbacktrace="$enableval"
		case $enableval in
			"no");;
			"yes");;
			*) AC_MSG_ERROR([[invalid argument --enable-libbacktrace=$enableval... stopping]]);;
		esac
	],
	[
		case $host_os in
		Linux* )
			enable_libbacktrace="yes"
			;;
		*)
			enable_libbacktrace="no"
			;;
		esac
	]
)

#
# Buildbot
#
AC_ARG_ENABLE(
	[buildbot],
	AS_HELP_STRING(
		[--enable-buildbot@<:@=ARG@:>@],
		[(available options: yes, no)]
	),
	[
		enable_buildbot="$enableval"
		case $enableval in
			"no");;
			"yes");;
			*) AC_MSG_ERROR([[invalid argument --enable-buildbot=$enableval... stopping]]);;
		esac
	],
	[enable_buildbot="no"]
)

#
# RDTSC as Tick Source
#
AC_ARG_ENABLE(
	[rdtsc],
	AS_HELP_STRING(
		[--enable-rdtsc],
		[
			Uses rdtsc as timing source (disabled by default)
			Enable it when you've timing issues.

			(For example:  in conjunction with XEN or Other Virtualization mechanisms)
		
			Note:
				Please ensure that you've disabled dynamic CPU-Frequencys, such as power saving options.
				(On most modern Dedicated Servers cpufreq is preconfigured, see your distribution's
				 manual how to disable it).
				Furthermore, If your CPU has built-in CPU-Frequency scaling features (such as Intel's
				 SpeedStep(R)), do not enable this option. Recent CPUs (Intel Core or newer) guarantee
				 a fixed increment rate for their TSC, so it should be safe to use, but please doublecheck
				 the documentation of both your CPU and OS before enabling this option.
		]
	),
	[
		enable_rdtsc=1
	],
	[enable_rdtsc=0]
)

#
# Profiler
#
AC_ARG_ENABLE(
	[profiler],
	AS_HELP_STRING(
		[--enable-profiler=ARG],
		[Profilers: no, gprof (disabled by default)]
	),
	[
		enable_profiler="$enableval"
		case $enableval in
			"no");;
			"gprof");;
			*) AC_MSG_ERROR([[invalid argument --enable-profiler=$enableval... stopping]]);;
		esac
	],
	[enable_profiler="no"]
)


#
# 64bit
#
AC_ARG_ENABLE(
	[64bit],
	AS_HELP_STRING(
		[--disable-64bit],
		[
			Enforce 32bit output on x86_64 systems.
		]
	),
	[
		enable_64bit="$enableval"
		case $enableval in
			"no");;
			"yes");;
			*) AC_MSG_ERROR([[invalid argument --enable-64bit=$enableval... stopping]]);;
		esac
	],
	[enable_64bit="yes"]
)


#
# LTO
#
AC_ARG_ENABLE(
	[lto],
	AS_HELP_STRING(
		[--enable-lto],
		[
			Enables or Disables Linktime Code Optimization (LTO is disabled by default)
		]
	),
	[
		enable_lto="$enableval"
		case $enableval in
			"no");;
			"yes");;
			*) AC_MSG_ERROR([[invalid argument --enable-lto=$enableval... stopping]]);;
		esac
	],
	[enable_lto="no"]
)


#
# static linking
#
AC_ARG_ENABLE(
	[static],
	AS_HELP_STRING(
		[--enable-static],
		[
			Enables or Disables Statick Linking (STATIC is disabled by default)
		]
	),
	[
		enable_static="$enableval"
		case $enableval in
			"no");;
			"yes");;
			*) AC_MSG_ERROR([[invalid argument --enable-static=$enableval... stopping]]);;
		esac
	],
	[enable_static="no"]
)


#
# adress sanitize
#
AC_ARG_ENABLE(
	[sanitize],
	AS_HELP_STRING(
		[--enable-sanitize@<:@=ARG@:>@],
		[
			Enables sanitizer. (disabled by default)
			(available options: yes, no, full)
		]
	),
	[
		enable_sanitize="$enableval"
		case $enableval in
			"no");;
			"yes")
				CPPFLAGS="$CPPFLAGS -DSANITIZE"
				;;
			"full")
				CPPFLAGS="$CPPFLAGS -DSANITIZE"
				;;
			*) AC_MSG_ERROR([[invalid argument --enable-sanitize=$enableval... stopping]]);;
		esac
	],
	[enable_sanitize="no"]
)


#
# -Werror compiler flag
# It's not always possible (if at all) to pass -Werror in the CFLAGS, since
# autoconf often generates test-code that produces warnings.
# Reference: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=734104
#
AC_ARG_ENABLE(
	[Werror],
	AS_HELP_STRING(
		[--enable-Werror],
		[
			Enables -Werror in the compiler flags. (disabled by default)
		]
	),
	[
		enable_Werror="$enableval"
		case $enableval in
			"no");;
			"yes");;
			*) AC_MSG_ERROR([[invalid argument --enable-Werror=$enableval... stopping]]);;
		esac
	],
	[enable_Werror="no"]
)


#
# Renewal
#
AC_ARG_ENABLE(
	[renewal],
	AS_HELP_STRING(
		[--disable-renewal],
		[
			Disable Ragnarok Renewal support (override settings in src/config/renewal.h)
		]
	),
	[
		enable_renewal="$enableval"
		case $enableval in
			"no");;
			"yes");;
			*) AC_MSG_ERROR([[invalid argument --enable-renewal=$enableval... stopping]]);;
		esac
	],
	[enable_renewal="yes"]
)


#
# Optionally set the max number of network conenctions
# the core will be support
#
AC_ARG_WITH(
	[maxconn],
	AS_HELP_STRING(
		[--with-maxconn@<:@=ARG@:>@],
		[optionally set the maximum connections the core can handle (Without epoll enabled, default: 1024. With epol enabled: 3072)]
	),
	[
		if test "$withval" != "no";	 then
			if ! test "$withval" -ge 0 -o "$withval" -lt 0 2>&- ; then
				AC_MSG_ERROR([Invalid argument --with-maxconn=$withval ... stopping])
			else
				CPPFLAGS="$CPPFLAGS -DMAXCONN=$withval"
			fi
		fi
	],
	[
		AC_MSG_RESULT([Use default MAXCONN value.])
	]
)


#
# Optionally specify the path to mysql_config
#
AC_ARG_WITH(
	[mysql],
	AS_HELP_STRING(
		[--with-mysql@<:@=ARG@:>@],
		[optionally specify the path to the mysql_config executable]
	),
	[
		if test "$withval" != "no" ; then
			if test ! -x "$withval" ; then
				AC_MSG_ERROR([$withval is not an executable file])
			fi
			MYSQL_CONFIG_HOME="$withval"
		fi
	]
)


#
# Manual MYSQL_CFLAGS (optional)
#
AC_ARG_WITH(
	[MYSQL_CFLAGS],
	AS_HELP_STRING(
		[--with-MYSQL_CFLAGS=ARG],
		[specify MYSQL_CFLAGS manually (instead of using "mysql_config --include")]
	),
	[
		manual_MYSQL_CFLAGS="yes"
		MYSQL_CFLAGS="$withval"
	],
	[manual_MYSQL_CFLAGS="no"]
)


#
# Manual MYSQL_LIBS (optional)
#
AC_ARG_WITH(
	[MYSQL_LIBS],
	AS_HELP_STRING(
		[--with-MYSQL_LIBS=ARG],
		[specify MYSQL_LIBS manually (instead of using "mysql_config --libs")]
	),
	[
		manual_MYSQL_LIBS="yes"
		MYSQL_LIBS="$withval"
	],
	[manual_MYSQL_LIBS="no"]
)


###############################################################################
# Check for programs and types.
#



AC_PROG_MAKE_SET
AC_PROG_CC
AC_PROG_CPP
AC_PATH_PROG(AR, ar)
AC_LANG([C])


# -Wcast-qual  use for check wrong casts. It cant be added by default, because some casts must be wrong.
# -Wconversion warning: conversion to 'long unsigned int' from 'int' may change the sign of the result
# -Wfloat-equal comparing floating point with == or != is unsafe
# -Wunsuffixed-float-constants float issues
# -Wpedantic different pedantic checks. Not all can be fixed in nice way.
# -Wtraditional-conversion some conversion issues
# -Wunsafe-loop-optimizations possible optimisation issues
# -Wunused-parameter -Wunused-but-set-parameter unused parameters
# -Wunused-macros unused macro warnings, but it also enable unused parameters
# -Wstrict-prototypes show warning in grammar.y
# -wdiscarded-qualifiers show different const removal
# attributes suggestion
#  -Wsuggest-attribute=const -Wsuggest-attribute=format -Wsuggest-attribute=pure

# flags what can be used but now return no warnings:
#   -Wchkp -Wdisabled-optimization
#   -Werror-implicit-function-declaration -Wtrampolines
#   -Wjump-misses-init

AC_MSG_CHECKING([whether $CC supports -std=c11])
OLD_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -std=c11"
AC_COMPILE_IFELSE(
	[AC_LANG_SOURCE([int foo;])],
	[
		AC_MSG_RESULT([yes])
	],
	[
		AC_MSG_RESULT([no])

		AC_MSG_CHECKING([whether $CC supports -std=c1x])
		CFLAGS="$OLD_CFLAGS -std=c1x"
		AC_COMPILE_IFELSE(
			[AC_LANG_SOURCE([int foo;])],
			[
				AC_MSG_RESULT([yes])
			],
			[
				AC_MSG_RESULT([no, falling back to -std=c99])
				CFLAGS="$OLD_CFLAGS -std=c99"
			]
		)
	],
	[
		AC_MSG_RESULT([guessing no])
		CFLAGS="$OLD_CFLAGS -std=c99"
	]
)

CFLAGS="$CFLAGS -pipe -ffast-math -Wall -Wextra -Wno-sign-compare"
CPPFLAGS="$CPPFLAGS -I../common"
LDFLAGS="$LDFLAGS"

# Add /usr/local to the library and header search path, where appropriate
case `uname` in
	Darwin*|*BSD*)
		if test -d /usr/local/include ; then
			CPPFLAGS="$CPPFLAGS -I/usr/local/include"
		fi
		if test -d /usr/local/lib ; then
			LDFLAGS="$LDFLAGS -L/usr/local/lib"
		fi
	;;
esac

AC_C_BIGENDIAN(
	[AC_MSG_ERROR([[bigendian is not supported... stopping]])],
	,
	[AC_MSG_WARN([[unable to determine endianess, only little endian is supported]])]
)


if test "$enable_64bit" = "no" ; then
	AC_MSG_CHECKING([whether pointers can be stored in ints (old code)])
	pointers_fit_in_ints="no"
	AC_COMPILE_IFELSE(
		[AC_LANG_PROGRAM([[static int test_array[((long int)sizeof(int)) == ((long int)sizeof(void*)) ? 1 : -1];]])],
		[pointers_fit_in_ints="yes"],
		[]
	)
	if test "$pointers_fit_in_ints" = "no" ; then
		CFLAGS="$CFLAGS -m32"
		LDFLAGS="$LDFLAGS -m32"
		AC_COMPILE_IFELSE(
			[AC_LANG_PROGRAM([[static int test_array[((long int)sizeof(int)) == ((long int)sizeof(void *)) ? 1 : -1];]])],
			[pointers_fit_in_ints="yes (with -m32)"],
			[]
		)
	fi
	AC_MSG_RESULT($pointers_fit_in_ints)
	if test "$pointers_fit_in_ints" = "no" ; then
		AC_MSG_ERROR([pointers cannot be stored in ints, required for old code... stopping])
	fi
fi


# 
# check if we're producing 32bit code - so well produce binarys for at least i686 (speedup: cmovs, and cmpchg8 support)
#
AC_MSG_CHECKING([whether $CC produces 32bit code])
AC_RUN_IFELSE(
	[AC_LANG_SOURCE([
		int main(int argc, char **argv){
			if(sizeof(void*) == 4) return 0;
			else return 1;
		}
	])],
	[
		AC_MSG_RESULT([yes])
		AC_MSG_CHECKING([whether $CC supports -march=i686 (and we can run it)])
		OLD_CFLAGS="$CFLAGS"
		OLD_LDFLAGS="$LDFLAGS"
		CFLAGS="$CFLAGS -march=i686"
		LDFLAGS="$LDFLAGS -march=i686"
		AC_RUN_IFELSE(
			[AC_LANG_SOURCE([
				int main(int argc, char **argv) { return 0; }
			])],
			[
				AC_MSG_RESULT([yes])
			],
			[
				AC_MSG_RESULT([no])
				CFLAGS="$OLD_CFLAGS"
				LDFLAGS="$OLD_LDFLAGS"
			],
			[
				AC_MSG_RESULT([guessing no])
				CFLAGS="$OLD_CFLAGS"
				LDFLAGS="$OLD_LDFLAGS"
			]
		)
	],
	[
		AC_MSG_RESULT([no])
	],
	[
		AC_MSG_RESULT([guessing no])
	]
)


# 
# Check if CC supports __thread attribute (Thread Local Storage) 
# (Usually our OSX friends 're lacking support of it in older llvm versions ..)
# 
case "${host}" in
	*-*-mingw*)
		CPPFLAGS="$CPPFLAGS -DHAS_TLS"
		;;
	*)
		AC_MSG_CHECKING([whether $CC supports __thread specifier (TLS)])
		AC_RUN_IFELSE(
			[AC_LANG_SOURCE([
				__thread int g_Test = -1;
				int main(int argc, char **argv){
					g_Test = 0;
					return g_Test;
				}
			])],
			[
				AC_MSG_RESULT([yes])
				CPPFLAGS="$CPPFLAGS -DHAS_TLS"
			],
			[
				AC_MSG_RESULT([no])
			]
		)
		;;
esac


#
# Check if the linker supports/accepts -rdynamic
# Generally only needed by the ELF linker, in order to produce backtraces.
# On non-ELF platforms, some compilers (i.e. gcc < 5 and clang on OSX) are able to ignore it, others will error out.
#
OLD_LDFLAGS="$LDFLAGS"
LDFLAGS="$LDFLAGS -rdynamic"
AC_MSG_CHECKING([whether $CC accepts -rdynamic])
AC_LINK_IFELSE(
	[AC_LANG_SOURCE([
		int main(int argc, char **argv)
		{
			return 0;
		}
	])],
	[
		AC_MSG_RESULT([yes])
	],
	[
		AC_MSG_RESULT([no])
		LDFLAGS="$OLD_LDFLAGS"
	],
	[
		AC_MSG_RESULT([guessing no])
	]
)

#
# LTO Support test
#
if test "$enable_lto" != "no" ; then

	OLD_CFLAGS="$CFLAGS"
	CFLAGS="$CFLAGS -flto -ffat-lto-objects -Werror"

	OLD_LDFLAGS="$LDFLAGS"
	LDFLAGS="$LDFLAGS -flto -ffat-lto-objects"

	AC_MSG_CHECKING([whether $CC supports -flto -ffat-lto-objects])
	AC_RUN_IFELSE(
		[AC_LANG_SOURCE([
			int main(int argc, char **argv){
				(void)argc;
				(void)argv;
				return 0;
			}
		])],
		[
			AC_MSG_RESULT([yes])
			CFLAGS="$OLD_CFLAGS -flto -ffat-lto-objects"
			LDFLAGS="$OLD_LDFLAGS -flto -ffat-lto-objects"
		],
		[
			AC_MSG_RESULT([no])
			CFLAGS="$OLD_CFLAGS -flto"
			LDFLAGS="$OLD_LDFLAGS -flto"
			AC_MSG_CHECKING([whether $CC supports -flto])
			AC_RUN_IFELSE(
				[AC_LANG_SOURCE([
					int main(int argc, char **argv){
						(void)argc;
						(void)argv;
						return 0;
					}
				])],
				[
					AC_MSG_RESULT([yes])
				],
				[
					AC_MSG_RESULT([no])
					CFLAGS="$OLD_CFLAGS"
					LDFLAGS="$OLD_LDFLAGS"
				],
				[
					AC_MSG_RESULT([guessing no])
					CFLAGS="$OLD_CFLAGS"
					LDFLAGS="$OLD_LDFLAGS"
				]
			)
		],
		[
			AC_MSG_RESULT([guessing no])
			CFLAGS="$OLD_CFLAGS"
			LDFLAGS="$OLD_LDFLAGS"
		]
	)
fi

AC_DEFUN([AC_CHECK_SANITIZER_FLAG],
	[
		AC_MSG_CHECKING([whether $CC supports -fsanitize=$1])
		OLD_CFLAGS="$CFLAGS"
		CFLAGS="$CFLAGS -fsanitize=$1"
		OLD_LDFLAGS="$LDFLAGS"
		LDFLAGS="$LDFLAGS -fsanitize=$1"
		AC_COMPILE_IFELSE(
			[AC_LANG_SOURCE([int foo;])],
			[
				AC_MSG_RESULT([yes])
			],
			[
				AC_MSG_RESULT([no])

				AC_MSG_CHECKING([whether $CC requires -fsanitize-undefined-trap-on-error for -fsanitize=$1])
				CFLAGS="$CFLAGS -fsanitize-undefined-trap-on-error"
				AC_COMPILE_IFELSE(
					[AC_LANG_SOURCE([int foo;])],
					[
						AC_MSG_RESULT([yes])
					],
					[
						AC_MSG_RESULT([no])
						CFLAGS="$OLD_CFLAGS"
						LDFLAGS="$OLD_LDFLAGS"
					]
				)
			],
			[
				AC_MSG_RESULT([guessing no])
				CFLAGS="$OLD_CFLAGS"
				LDFLAGS="$OLD_LDFLAGS"
			]
		)
	]
)

AC_DEFUN([AC_CHECK_FLAG],
	[
		AC_MSG_CHECKING([whether $CC supports $1])
		OLD_CFLAGS="$CFLAGS"
		CFLAGS="$CFLAGS $1"
		OLD_LDFLAGS="$LDFLAGS"
		LDFLAGS="$LDFLAGS $1"
		AC_COMPILE_IFELSE(
			[AC_LANG_SOURCE([int foo;])],
			[
				AC_MSG_RESULT([yes])
			],
			[
				AC_MSG_RESULT([no])
				CFLAGS="$OLD_CFLAGS"
				LDFLAGS="$OLD_LDFLAGS"
			],
			[
				AC_MSG_RESULT([guessing no])
				CFLAGS="$OLD_CFLAGS"
				LDFLAGS="$OLD_LDFLAGS"
			]
		)
	]
)

#
# sanitize Support test
#
if test "$enable_sanitize" != "no" ; then
	AC_CHECK_SANITIZER_FLAG(address)
	if test "$enable_sanitize" == "full" ; then
		# skipped because server have multiple issues: -fsanitize=alignment
		AC_CHECK_SANITIZER_FLAG(shift)
		AC_CHECK_SANITIZER_FLAG(integer-divide-by-zero)
		AC_CHECK_SANITIZER_FLAG(unreachable)
		AC_CHECK_SANITIZER_FLAG(vla-bound)
		AC_CHECK_SANITIZER_FLAG(null)
		AC_CHECK_SANITIZER_FLAG(return)
		AC_CHECK_SANITIZER_FLAG(signed-integer-overflow)
		AC_CHECK_SANITIZER_FLAG(bounds)
		AC_CHECK_SANITIZER_FLAG(bounds-strict)
		AC_CHECK_SANITIZER_FLAG(object-size)
		AC_CHECK_SANITIZER_FLAG(float-divide-by-zero)
		AC_CHECK_SANITIZER_FLAG(float-cast-overflow)
		AC_CHECK_SANITIZER_FLAG(nonnull-attribute)
		AC_CHECK_SANITIZER_FLAG(returns-nonnull-attribute)
		AC_CHECK_SANITIZER_FLAG(bool)
		AC_CHECK_SANITIZER_FLAG(enum)
		AC_CHECK_SANITIZER_FLAG(vptr)
		AC_CHECK_SANITIZER_FLAG(address-use-after-scope)
		AC_CHECK_SANITIZER_FLAG(pointer-overflow)
		AC_CHECK_SANITIZER_FLAG(builtin)
		AC_CHECK_SANITIZER_FLAG(pointer-compare)
		AC_CHECK_SANITIZER_FLAG(pointer-subtract)
		AC_CHECK_SANITIZER_FLAG(shift-exponent)
		AC_CHECK_SANITIZER_FLAG(shift-base)
		AC_CHECK_SANITIZER_FLAG(sanitize-address-use-after-scope)
	fi
fi

AC_DEFUN([AC_CHECK_COMPILER_WFLAG],
	[
		AC_MSG_CHECKING([whether $CC supports -W$1])
		OLD_CFLAGS="$CFLAGS"
		CFLAGS="$CFLAGS -Werror -W$1"
		AC_COMPILE_IFELSE(
			[AC_LANG_SOURCE([int foo;])],
			[
				AC_MSG_RESULT([yes])
				CFLAGS="$OLD_CFLAGS -W$1"
				# Optionally, run a test
				if test "x$2" != "x"; then
					AC_MSG_CHECKING([whether $CC can actually use -W$1])
					CFLAGS="$OLD_CFLAGS -Werror -W$1"
					AC_COMPILE_IFELSE(
						[AC_LANG_SOURCE([$2])],
						[
							AC_MSG_RESULT([no])
							CFLAGS="$OLD_CFLAGS"
						]
						[
							AC_MSG_RESULT([yes])
							CFLAGS="$OLD_CFLAGS -W$1"
						],
					)
				fi
			],
			[
				AC_MSG_RESULT([no])
				CFLAGS="$OLD_CFLAGS"
			]
		)
	]
)

AC_DEFUN([AC_CHECK_COMPILER_WNOFLAG],
	[
		AC_MSG_CHECKING([whether $CC supports -Wno-$1])
		OLD_CFLAGS="$CFLAGS"
		CFLAGS="$CFLAGS -Werror -Wno-$1"
		AC_COMPILE_IFELSE(
			[AC_LANG_SOURCE([int foo;])],
			[
				# Recent versions of gcc don't fail if -Wno-foo is not recognized
				# (unless there are also other warnings), so we also check for -Wfoo
				# which always fails if not supported
				CFLAGS="$OLD_CFLAGS -Werror -W$1"
				AC_COMPILE_IFELSE(
					[AC_LANG_SOURCE([int foo;])],
					[
						AC_MSG_RESULT([yes])
						CFLAGS="$OLD_CFLAGS -Wno-$1"
						# Optionally, run a test
						if test "x$2" != "x"; then
							AC_MSG_CHECKING([whether $CC can actually use -Wno-$1])
							CFLAGS="$OLD_CFLAGS -Werror -W$1"
							AC_COMPILE_IFELSE(
								[AC_LANG_SOURCE([$2])],
								[
									AC_MSG_RESULT([not needed but enabled])
									CFLAGS="$OLD_CFLAGS"
								],
								[
									CFLAGS="$OLD_CFLAGS -Werror -Wno-$1"
									AC_COMPILE_IFELSE(
										[AC_LANG_SOURCE([$2])],
										[
											AC_MSG_RESULT([yes])
											CFLAGS="$OLD_CFLAGS -Wno-$1"
										],
										[
											AC_MSG_RESULT([no])
											CFLAGS="$OLD_CFLAGS"
										]
									)
								]
							)
						fi
					],
					[
						AC_MSG_RESULT([no])
						CFLAGS="$OLD_CFLAGS"
					]
				)
			],
			[
				AC_MSG_RESULT([no])
				CFLAGS="$OLD_CFLAGS"
			]
		)
	]
)

AC_CHECK_FLAG(-fsigned-char)
AC_CHECK_FLAG(-fcf-protection=none)

AC_CHECK_COMPILER_WNOFLAG(unused-parameter, [int foo(int bar) { return 0; }])
AC_CHECK_COMPILER_WNOFLAG(clobbered)
AC_CHECK_COMPILER_WFLAG(empty-body)
AC_CHECK_COMPILER_WFLAG(newline-eof)
AC_CHECK_COMPILER_WFLAG(int-conversion)
AC_CHECK_COMPILER_WFLAG(init-self)
AC_CHECK_COMPILER_WFLAG(pointer-arith)
AC_CHECK_COMPILER_WFLAG(switch-bool)
AC_CHECK_COMPILER_WFLAG(enum-conversion)
AC_CHECK_COMPILER_WFLAG(shorten-64-to-32)
AC_CHECK_COMPILER_WFLAG(constant-conversion)
AC_CHECK_COMPILER_WFLAG(bool-conversion)
AC_CHECK_COMPILER_WFLAG(format-security)
AC_CHECK_COMPILER_WFLAG(format)
AC_CHECK_COMPILER_WFLAG(format-signedness)
AC_CHECK_COMPILER_WFLAG(format-y2k)
AC_CHECK_COMPILER_WFLAG(format-overflow=2)
AC_CHECK_COMPILER_WFLAG(format-diag)
#AC_CHECK_COMPILER_WFLAG(format-truncation=2)
AC_CHECK_COMPILER_WFLAG(missing-include-dirs)
AC_CHECK_COMPILER_WFLAG(suggest-attribute=noreturn)
AC_CHECK_COMPILER_WFLAG(suggest-attribute=format)
#AC_CHECK_COMPILER_WFLAG(suggest-attribute=malloc)
AC_CHECK_COMPILER_WFLAG(undef)
#AC_CHECK_COMPILER_WFLAG(cast-align)
#AC_CHECK_COMPILER_WFLAG(logical-op) # some useless warnings
AC_CHECK_COMPILER_WFLAG(nested-externs)
AC_CHECK_COMPILER_WFLAG(old-style-definition)
AC_CHECK_COMPILER_WFLAG(overlength-strings)
AC_CHECK_COMPILER_WFLAG(redundant-decls)
#AC_CHECK_COMPILER_WFLAG(cast-qual)
AC_CHECK_COMPILER_WFLAG(misleading-indentation)
AC_CHECK_COMPILER_WFLAG(null-dereference)
AC_CHECK_COMPILER_WFLAG(vla)
AC_CHECK_COMPILER_WFLAG(vla-parameter)
AC_CHECK_COMPILER_WFLAG(switch-enum)
AC_CHECK_COMPILER_WFLAG(mismatched-dealloc)
AC_CHECK_COMPILER_WFLAG(sizeof-array-div)
AC_CHECK_COMPILER_WFLAG(stringop-overflow=4)
AC_CHECK_COMPILER_WFLAG(stringop-overread)
AC_CHECK_COMPILER_WFLAG(string-compare)
AC_CHECK_COMPILER_WFLAG(zero-length-bounds)
AC_CHECK_COMPILER_WFLAG(absolute-value)
AC_CHECK_COMPILER_WFLAG(shift-overflow=2)
#AC_CHECK_COMPILER_WFLAG(unreachable-code)
#AC_CHECK_COMPILER_WFLAG(unused-const-variable=2)
#AC_CHECK_COMPILER_WFLAG(write-strings)

#AC_CHECK_COMPILER_WNOFLAG(format-nonliteral)
AC_CHECK_COMPILER_WNOFLAG(switch)
AC_CHECK_COMPILER_WNOFLAG(missing-field-initializers)
#AC_CHECK_COMPILER_WNOFLAG(suggest-attribute=format)
AC_CHECK_COMPILER_WNOFLAG(incompatible-pointer-types-discards-qualifiers)
AC_CHECK_COMPILER_WNOFLAG(discarded-qualifiers)
# issue in clang-15 in libconfig
AC_CHECK_COMPILER_WNOFLAG(deprecated-non-prototype)
# issue in clang-15 in libconfig
AC_CHECK_COMPILER_WNOFLAG(unused-but-set-variable)

# Certain versions of gcc make -Wshadow completely useless by making it flood
# you with unnecessary warnings <https://lkml.org/lkml/2006/11/28/239>
# Let's check if we can really use it
SAVED_OLD_CFLAGS="$CFLAGS"
AC_CHECK_COMPILER_WFLAG(shadow)
if test "x$CFLAGS" != "x$SAVED_OLD_CFLAGS"; then
	AC_MSG_CHECKING([whether $CC can efficiently use -Wshadow])
	NEW_CFLAGS="$CFLAGS"
	CFLAGS="$CFLAGS -Werror -Wshadow"
	AC_COMPILE_IFELSE(
		[AC_LANG_SOURCE([
			int foo(void) {
				return 0;
			}
			int bar(void) {
				int foo = 0;
				return foo + 1;
			}
		])],
		[
			AC_MSG_RESULT([yes])
			CFLAGS="$NEW_CFLAGS"
		],
		[
			AC_MSG_RESULT([no])
			CFLAGS="$SAVED_OLD_CFLAGS"
		]
	)
fi

# Certain versions of gcc implement -Wformat-truncation in a naive way,
# disregarding the actual values that an integer can take in every code path
# and assuming a worst case of INT_MAX. That makes it not very useful as it
# produces meaningless warnings.
SAVED_OLD_CFLAGS="$CFLAGS"
AC_CHECK_COMPILER_WNOFLAG(format-truncation)
if test "x$CFLAGS" != "x$SAVED_OLD_CFLAGS"; then
	AC_MSG_CHECKING([whether $CC has a too naive -Wformat-truncation implementation])
	NEW_CFLAGS="$CFLAGS"
	CFLAGS="$SAVED_OLD_CFLAGS"
	CFLAGS="$CFLAGS -Werror"
	AC_COMPILE_IFELSE(
		[AC_LANG_SOURCE([
			#include <stdio.h>
			char foo(void) {
				[char buf[3];]
				for (int i = 0; i < 4; i++) {
					snprintf(buf, sizeof(buf), "a%d", i);
				}
				[return buf[1];]
			}
		])],
		[
			AC_MSG_RESULT([no])
			CFLAGS="$SAVED_OLD_CFLAGS"
		],
		[
			AC_MSG_RESULT([yes])
			CFLAGS="$NEW_CFLAGS"
		]
	)
fi

AC_MSG_CHECKING([whether $CC supports -fPIC])
OLD_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -fPIC"
AC_COMPILE_IFELSE(
	[AC_LANG_SOURCE([int foo;])],
	[
		AC_MSG_RESULT([yes])
		SOFLAGS="$SOFLAGS -fPIC"
	],
	[
		AC_MSG_RESULT([no])
	]
)
CFLAGS="$OLD_CFLAGS"

#
# option for shared objects
#
AC_MSG_CHECKING([how to make shared objects])
OLD_CFLAGS="$CFLAGS"
compiler_shared_objects=""
compiler_supports_shared_objects="no"
my_shared_test_flags="$CFLAGS $SOFLAGS"
# default
CFLAGS="$my_shared_test_flags -shared"
AC_LINK_IFELSE(
	[AC_LANG_SOURCE([
		int bar = 0;

		int foo(void)
		{
			return bar;
		}
	])],
	[
		compiler_shared_objects="-shared"
		compiler_supports_shared_objects="yes"
	]
)
# BeOS specific
CFLAGS="$my_shared_test_flags -nostart"
AC_LINK_IFELSE(
	[AC_LANG_SOURCE([
		int bar = 0;

		int foo(void)
		{
			return bar;
		}
	])],
	[
		compiler_shared_objects="-nostart"
		compiler_supports_shared_objects="yes"
	]
)
CFLAGS="$OLD_CFLAGS"
if test "$compiler_supports_shared_objects" = "no" ; then
	AC_MSG_RESULT([not supported])
	AC_MSG_NOTICE([compiler is unable to generate shared objects, disabled plugins (optional)])
	WITH_PLUGINS="no"
else
	AC_MSG_RESULT([$compiler_shared_objects $compiler_supports_shared_objects])
	SOFLAGS="$SOFLAGS $compiler_shared_objects"
	WITH_PLUGINS="yes"

	#
	# On certain platforms, undefined references on shared libraries won't be checked
	# unless explicitly required with the --no-undefined linker option
	#
	AC_MSG_CHECKING([whether $CC needs -Wl,--no-undefined to check for undefined references in shared objects])
	OLD_CFLAGS="$CFLAGS"
	CFLAGS="$SOFLAGS"
	AC_LINK_IFELSE(
		[AC_LANG_SOURCE([
			void foo(void) {
				foobar();
			}
		])],
		[
			CFLAGS="$SOFLAGS -Wl,--no-undefined"
			AC_LINK_IFELSE(
				[AC_LANG_SOURCE([
					int bar = 0;

					int foo(void) {
						return bar;
					}
				])],
				[
					AC_MSG_RESULT([yes])
					SOFLAGS="$SOFLAGS -Wl,--no-undefined"
				],
				[
					AC_MSG_RESULT([unsupported (undefined references check will be ignored)])
				]
			)
		],
		[
			AC_MSG_RESULT([no])
		]
	)
	CFLAGS="$OLD_CFLAGS"
fi
my_shared_test_flags=""
AC_SUBST([WITH_PLUGINS])
AC_SUBST([SOFLAGS])


#
# -O2 implies -fstrict-aliasing, but the code is not safe for that
#
AC_MSG_CHECKING([whether $CC supports -fno-strict-aliasing])
OLD_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -fno-strict-aliasing"
AC_COMPILE_IFELSE(
	[AC_LANG_SOURCE([int foo;])],
	[AC_MSG_RESULT([yes])],
	[
		AC_MSG_RESULT([no])
		CFLAGS="$OLD_CFLAGS"
	]
)


###############################################################################
# Check for libraries and header files.
#


#
# setrlimit - used to set the socket limit
#
AC_CHECK_FUNC([setrlimit],[CPPFLAGS="$CPPFLAGS -DHAVE_SETRLIMIT"])


#
# strnlen - string length with upper scan bound
#
AC_CHECK_FUNC([strnlen],[CPPFLAGS="$CPPFLAGS -DHAVE_STRNLEN"])

# libconfig
AC_CHECK_FUNCS([uselocale])
AC_CHECK_FUNCS([newlocale])
AC_CHECK_FUNCS([freelocale])
AC_CHECK_HEADERS([xlocale.h])

#
# Memory manager
#
case $enable_manager in
	"no")
		CPPFLAGS="$CPPFLAGS -DNO_MEMMGR"
		;;
	"builtin")
		# enabled by default
		;;
	"memwatch")
		CPPFLAGS="$CPPFLAGS -DMEMWATCH"
		AC_CHECK_HEADER([memwatch.h], , [AC_MSG_ERROR([memwatch header not found... stopping])])
		;;
	"dmalloc")
		CPPFLAGS="$CPPFLAGS -DDMALLOC -DDMALLOC_FUNC_CHECK"
		LIBS="$LIBS -ldmalloc"
		AC_CHECK_HEADER([dmalloc.h], , [AC_MSG_ERROR([dmalloc header not found... stopping])])
		;;
	"gcollect")
		CPPFLAGS="$CPPFLAGS -DGCOLLECT"
		LIBS="$LIBS -lgc"
		AC_CHECK_HEADER([gc.h], , [AC_MSG_ERROR([gcollect header not found... stopping])])
		;;
	"bcheck")
		CPPFLAGS="$CPPFLAGS -DBCHECK"
		;;
esac


#
# Packetver
#
if test -n "$enable_packetver" ; then
	CPPFLAGS="$CPPFLAGS -DPACKETVER=$enable_packetver"
fi

#
# Packetver-RE
#
case $enable_packetver_re in
	"yes")
		CPPFLAGS="$CPPFLAGS -DENABLE_PACKETVER_RE"
		;;
	"no")
		# default value
		;;
esac

#
# Packetver-Zero
#
case $enable_packetver_zero in
	"yes")
		CPPFLAGS="$CPPFLAGS -DENABLE_PACKETVER_ZERO"
		;;
	"no")
		# default value
		;;
esac

#
# Packetver-Sak
#
case $enable_packetver_sak in
	"yes")
		CPPFLAGS="$CPPFLAGS -DENABLE_PACKETVER_SAK"
		;;
	"no")
		# default value
		;;
esac

#
# Packetver-ad
#
case $enable_packetver_ad in
	"yes")
		CPPFLAGS="$CPPFLAGS -DENABLE_PACKETVER_AD"
		;;
	"no")
		# default value
		;;
esac

#
# cash shop preview patch
#
case $enable_cashshop_preview_patch in
	0)
		#default value
		;;
	1)
		CPPFLAGS="$CPPFLAGS -DENABLE_CASHSHOP_PREVIEW_PATCH"
		;;
esac

#
# old cash shop preview patch
#
case $enable_old_cashshop_preview_patch in
	0)
		#default value
		;;
	1)
		CPPFLAGS="$CPPFLAGS -DENABLE_OLD_CASHSHOP_PREVIEW_PATCH"
		;;
esac

#
# Epoll
#
case $have_linux_epoll in
	"yes")
		CPPFLAGS="$CPPFLAGS -DSOCKET_EPOLL"
		;;
	"no")
		# default value
		;;
esac

#
# Obfuscation keys
#
if test -n "$obfuscationkey1" -a -n "$obfuscationkey2" -a -n "$obfuscationkey3"; then
	CPPFLAGS="$CPPFLAGS -DOBFUSCATIONKEY1=$obfuscationkey1 -DOBFUSCATIONKEY2=$obfuscationkey2 -DOBFUSCATIONKEY3=$obfuscationkey3"
fi

#
# Debug
#
case $enable_debug in
	"no")
		# default value
#		CFLAGS="$CFLAGS -Wno-unused -Wno-parentheses"
		;;
	"yes")
		CFLAGS="$CFLAGS -g"
		CPPFLAGS="$CPPFLAGS -DDEBUG"
		AC_CHECK_FLAG(-fno-omit-frame-pointer)
		;;
	"gdb")
		CFLAGS="$CFLAGS -ggdb"
		CPPFLAGS="$CPPFLAGS -DDEBUG"
		AC_CHECK_FLAG(-fno-omit-frame-pointer)
		;;
esac

#
# libbacktrace
#
if test "$enable_libbacktrace" = "no" ; then
	USE_LIBBACKTRACE="no"
else
	CPPFLAGS="$CPPFLAGS -DHAVE_LIBBACKTRACE"
	USE_LIBBACKTRACE="yes"

	# libbacktrace checks
	AC_CHECK_FLAG(-funwind-tables)

	AC_PROG_AWK
	case "$AWK" in
		"")
			AC_MSG_ERROR([Libbacktrace requires awk to build])
			;;
	esac

	backtrace_supported=yes
	AC_CHECK_HEADER([unwind.h],
		[AC_CHECK_FUNC([_Unwind_Backtrace],
				[BACKTRACE_FILE="backtrace.lo simple.lo"],
				[
					BACKTRACE_FILE="nounwind.lo"
					backtrace_supported=no
				]
		)],
		[
			BACKTRACE_FILE="nounwind.lo"
			backtrace_supported=no
		]
	)
	AC_SUBST(BACKTRACE_FILE)

	AC_DEFINE(HAVE_GETIPINFO, 1, [Define if _Unwind_GetIPInfo is available.])

	AC_CACHE_CHECK([__sync extensions],
		[libbacktrace_cv_sys_sync],
		[
			if test -n "${with_target_subdir}"; then
				case "${host}" in
					hppa*-*-hpux*)
						libbacktrace_cv_sys_sync=no
						;;
					*)
						libbacktrace_cv_sys_sync=yes
						;;
				esac
			else
				AC_LINK_IFELSE(
					[
						AC_LANG_PROGRAM([int i;],
							[
								__sync_bool_compare_and_swap(&i, i, i);
								__sync_lock_test_and_set(&i, 1);
								__sync_lock_release(&i);
							]
						)
					],
					[libbacktrace_cv_sys_sync=yes],
					[libbacktrace_cv_sys_sync=no]
				)
			fi
		]
	)
	BACKTRACE_SUPPORTS_THREADS=0
	if test "$libbacktrace_cv_sys_sync" = "yes"; then
		BACKTRACE_SUPPORTS_THREADS=1
		AC_DEFINE([HAVE_SYNC_FUNCTIONS], 1, [Define to 1 if you have the __sync functions])
	fi
	AC_SUBST(BACKTRACE_SUPPORTS_THREADS)

	# Test for __atomic support.
	AC_CACHE_CHECK([__atomic extensions],
		[libbacktrace_cv_sys_atomic],
		[
			if test -n "${with_target_subdir}"; then
				libbacktrace_cv_sys_atomic=yes
			else
				AC_LINK_IFELSE(
					[
						AC_LANG_PROGRAM([int i;],
							[
								__atomic_load_n(&i, __ATOMIC_ACQUIRE);
								__atomic_store_n(&i, 1, __ATOMIC_RELEASE);
							]
						)
					],
					[libbacktrace_cv_sys_atomic=yes],
					[libbacktrace_cv_sys_atomic=no]
				)
			fi
		]
	)
	if test "$libbacktrace_cv_sys_atomic" = "yes"; then
		AC_DEFINE([HAVE_ATOMIC_FUNCTIONS], 1, [Define to 1 if you have the __atomic functions])
	fi

	# The library needs to be able to read the executable itself.  Compile
	# a file to determine the executable format.  The awk script
	# filetype.awk prints out the file type.
	AC_CACHE_CHECK([output filetype],
		[libbacktrace_cv_sys_filetype],
		[
			filetype=
			AC_COMPILE_IFELSE(
				[AC_LANG_PROGRAM([int i;], [int j;])],
				[filetype=`${AWK} -f $srcdir/3rdparty/libbacktrace/filetype.awk conftest.$ac_objext`],
				[AC_MSG_FAILURE([compiler failed])]
			)
			libbacktrace_cv_sys_filetype=$filetype
		]
	)

	# Match the file type to decide what files to compile.
	LIBBACKTRACE_FORMAT_FILE=
	backtrace_supports_data=yes
	case "$libbacktrace_cv_sys_filetype" in
		elf*)
			LIBBACKTRACE_FORMAT_FILE="elf.o"
			;;
		pecoff)
			LIBBACKTRACE_FORMAT_FILE="pecoff.o"
			backtrace_supports_data=no
			;;
		xcoff*)
			LIBBACKTRACE_FORMAT_FILE="xcoff.o"
			backtrace_supports_data=no
			;;
		macho*)
			LIBBACKTRACE_FORMAT_FILE="macho.o"
			backtrace_supports_data=no
			;;
		*)
			AC_MSG_WARN([could not determine output file type])
			LIBBACKTRACE_FORMAT_FILE="unknown.o"
			backtrace_supported=no
			;;
	esac
	AC_SUBST(LIBBACKTRACE_FORMAT_FILE)

	# ELF defines.
	elfsize=
	case "$libbacktrace_cv_sys_filetype" in
		elf32)
			elfsize=32
			;;
		elf64)
			elfsize=64
			;;
		*)
			elfsize=unused
	esac
	AC_DEFINE_UNQUOTED([BACKTRACE_ELF_SIZE], [$elfsize], [ELF size: 32 or 64])

	# XCOFF defines.
	xcoffsize=
	case "$libbacktrace_cv_sys_filetype" in
		xcoff32)
			xcoffsize=32
			;;
		xcoff64)
			xcoffsize=64
			;;
		*)
			xcoffsize=unused
	esac
	AC_DEFINE_UNQUOTED([BACKTRACE_XCOFF_SIZE], [$xcoffsize], [XCOFF size: 32 or 64])

	BACKTRACE_SUPPORTED=0
	if test "$backtrace_supported" = "yes"; then
		BACKTRACE_SUPPORTED=1
	fi
	AC_SUBST(BACKTRACE_SUPPORTED)

	BACKTRACE_SUPPORTS_DATA=0
	if test "$backtrace_supports_data" = "yes"; then
		BACKTRACE_SUPPORTS_DATA=1
	fi
	AC_SUBST(BACKTRACE_SUPPORTS_DATA)

	AC_CHECK_FUNC(mmap, [have_mmap=yes], [have_mmap=no])

	VIEW_FILE=mmapio.lo
	ALLOC_FILE=mmap.lo

	AC_SUBST(VIEW_FILE)
	AC_SUBST(ALLOC_FILE)

	BACKTRACE_USES_MALLOC=0
	if test "$ALLOC_FILE" = "alloc.lo"; then
		BACKTRACE_USES_MALLOC=1
	fi
	AC_SUBST(BACKTRACE_USES_MALLOC)

	AC_CHECK_FUNC([dl_iterate_phdr], [have_dl_iterate_phdr=yes], [have_dl_iterate_phdr=no])
	if test "$have_dl_iterate_phdr" = "yes"; then
		AC_DEFINE(HAVE_DL_ITERATE_PHDR, 1, [Define if dl_iterate_phdr is available.])
	fi

	# Check for the fcntl function.
	if test -n "${with_target_subdir}"; then
		case "${host}" in
			*-*-mingw*)
				have_fcntl=no
				;;
			spu-*-*)
				have_fcntl=no
				;;
			*)
				have_fcntl=yes
				;;
		esac
	else
		AC_CHECK_FUNC(fcntl, [have_fcntl=yes], [have_fcntl=no])
	fi
	if test "$have_fcntl" = "yes"; then
		AC_DEFINE([HAVE_FCNTL], 1, [Define to 1 if you have the fcntl function])
	fi

	AC_CHECK_DECLS(strnlen)
	AC_CHECK_FUNCS(lstat readlink)

	AC_CHECK_FUNC(getexecname, [have_getexecname=yes], [have_getexecname=no])
	if test "$have_getexecname" = "yes"; then
		AC_DEFINE(HAVE_GETEXECNAME, 1, [Define if getexecname is available.])
	fi
fi
AC_SUBST(USE_LIBBACKTRACE)

#
# Buildbot
#
case $enable_buildbot in
	"no")
		# default value
		;;
	"yes")
		CPPFLAGS="$CPPFLAGS -DBUILDBOT"
		;;
esac

#
# RDTSC
#
case $enable_rdtsc in
	0)
		#default value
		;;
	1)
		CPPFLAGS="$CPPFLAGS -DENABLE_RDTSC"
		;;
esac


#
# Profiler
#
case $enable_profiler in
	"no")
		# default value
		;;
	"gprof")
		CFLAGS="$CFLAGS -pg"
		LDFLAGS="$LDFLAGS -pg"
		;;
esac

#
# Renewal
#
case $enable_renewal in
	"no")
		CPPFLAGS="$CPPFLAGS -DDISABLE_RENEWAL"
		;;
	"yes")
		# default value
		;;
esac

AC_CHECK_FLAG(-fno-var-tracking)

#
# Host specific stuff
#
fd_setsize=""
DLLEXT=".so"
case $host_os in
Solaris* )
	LIBS="$LIBS -lsocket -lnsl -ldl"
	;;
Linux* )
	LIBS="$LIBS -ldl"
	;;
FreeBSD*)
	CPPFLAGS="$CPPFLAGS -D__FREEBSD__ -fvisibility=hidden"
	;;
NetBSD*)
	CPPFLAGS="$CPPFLAGS -D__NETBSD__"
	;;
CYGWIN*)
	CPPFLAGS="$CPPFLAGS -DFD_SETSIZE=4096 -DCYGWIN"
	fd_setsize="done"
	DLLEXT=".dll"
	;;
Darwin*)
	DLLEXT=".dylib"
	;;
esac
AC_SUBST([DLLEXT])

AC_MSG_CHECKING([for MinGW])
if test -n "`$CC --version 2>/dev/null | grep -i mingw`" ; then
	AC_MSG_RESULT([yes])
	CPPFLAGS="$CPPFLAGS -DMINGW -DLIBCONFIG_EXPORTS"
	if test -z "$fd_setsize" ; then
		CPPFLAGS="$CPPFLAGS -DFD_SETSIZE=4096"
	fi
	LIBS="$LIBS -lws2_32"
else
	AC_MSG_RESULT([no])
fi



#
# zlib library (required)
#
AC_SEARCH_LIBS([inflateEnd], [z], [],[AC_MSG_ERROR([zlib library not found or incompatible... stopping])])
AC_CHECK_HEADER([zlib.h], [], [AC_MSG_ERROR([zlib header not found, please specify the correct path with... stopping])])


#
# math library (required)
#
AC_SEARCH_LIBS([sqrt], [m], [], [AC_MSG_ERROR([math library not found... stopping])])


#
# clock_gettime (optional, rt on Debian)
#
AC_SEARCH_LIBS([clock_gettime], [rt])


#
# CLOCK_MONOTONIC clock for clock_gettime
# Normally defines _POSIX_TIMERS > 0 and _POSIX_MONOTONIC_CLOCK (for posix
# compliant systems) and __FreeBSD_cc_version >= 500005 (for FreeBSD
# >= 5.1.0, which does not have the posix defines (ref. r11983)) would be
# checked but some systems define them even when they do not support it
# (ref. bugreport:1003).
#
if test "$ac_cv_search_clock_gettime" != "no" ; then
	AC_MSG_CHECKING([whether CLOCK_MONOTONIC is supported and works])
	AC_RUN_IFELSE(
		[AC_LANG_SOURCE([
			#include <sys/time.h>
			#include <time.h>
			#include <unistd.h>
			
			int main(int argc, char** argv)
			{
				struct timespec tval;
				return clock_gettime(CLOCK_MONOTONIC, &tval);
			}
		])],
		[
			AC_MSG_RESULT([yes])
			CPPFLAGS="$CPPFLAGS -DHAVE_MONOTONIC_CLOCK"
		],
		[
			# either it failed to compile (CLOCK_MONOTONIC undefined)
			# or clock_gettime has returned a non-zero value
			AC_MSG_RESULT([no])
		],
		[
			AC_MSG_RESULT([guessing no])
		]
	)
fi


#
# pthread
#
case "${host}" in
	*-*-mingw*)
		;;
	*)
		AC_SEARCH_LIBS([pthread_create], [pthread], [], [AC_MSG_ERROR([pthread library not found or incompatible])])
		AC_SEARCH_LIBS([pthread_sigmask], [pthread], [], [AC_MSG_ERROR([pthread library not found or incompatible])])
		AC_SEARCH_LIBS([pthread_attr_init], [pthread], [], [AC_MSG_ERROR([pthread library not found or incompatible])])
		AC_SEARCH_LIBS([pthread_attr_setstacksize], [pthread], [], [AC_MSG_ERROR([pthread library not found or incompatible])])
		AC_SEARCH_LIBS([pthread_attr_destroy], [pthread], [], [AC_MSG_ERROR([pthread library not found or incompatible])])
		AC_SEARCH_LIBS([pthread_cancel], [pthread], [], [AC_MSG_ERROR([pthread library not found or incompatible])])
		AC_SEARCH_LIBS([pthread_join], [pthread], [], [AC_MSG_ERROR([pthread library not found or incompatible])])
		;;
esac

# execinfo (backtrace)
AC_CHECK_HEADERS([execinfo.h],
	[
		AC_SEARCH_LIBS([backtrace],
			[execinfo],
			[
				CFLAGS="$CFLAGS -DHAVE_EXECINFO"
			],
			[
			]
		)
	],
	[
	]
)

#
# MySQL library
#

if test -z "$MYSQL_CONFIG_HOME"; then
	AC_PATH_PROG([MYSQL_CONFIG_HOME], [mysql_config], [no])
fi

if test "$MYSQL_CONFIG_HOME" != "no" ; then
	MYSQL_VERSION="`$MYSQL_CONFIG_HOME --version`"
	if test "$manual_MYSQL_CFLAGS" = "no" ; then
		MYSQL_CFLAGS="`$MYSQL_CONFIG_HOME --include`"
	fi
	if test "$manual_MYSQL_LIBS" = "no" ; then
		MYSQL_LIBS="`$MYSQL_CONFIG_HOME --libs`"
	fi
else
	MYSQL_VERSION="unknown"
fi

MYSQL_OLD_LIBS="$LIBS" ; LIBS="$LIBS $MYSQL_LIBS"
MYSQL_OLD_CPPFLAGS="$CPPFLAGS" ; CPPFLAGS="$CPPFLAGS $MYSQL_CFLAGS"

case "${host}" in
	*-*-mingw*)
		HAVE_MYSQL="yes"
		MYSQL_CFLAGS="$MYSQL_CFLAGS -Dmy_bool=bool"
		;;
	*)
		AC_CHECK_FUNC([mysql_init], [HAVE_MYSQL="yes"])
		AC_CHECK_HEADER([mysql.h], [], [HAVE_MYSQL=""])

		AC_MSG_CHECKING([whether my_bool is supported (MySQL)])
		AC_RUN_IFELSE(
			[AC_LANG_SOURCE([
				#ifdef WIN32
				#include "common/winapi.h" // Needed before mysql.h
				#endif
				#include <mysql.h>
				int main(int argc, char **argv){
					my_bool val = (my_bool)0;
					return (int)val;
				}
			])],
			[
				AC_MSG_RESULT([yes])
			],
			[
				AC_MSG_RESULT([no (converting my_bool to bool)])
				MYSQL_CFLAGS="$MYSQL_CFLAGS -Dmy_bool=bool"
			]
		)
		;;
esac

CPPFLAGS="$MYSQL_OLD_CPPFLAGS"
LIBS="$MYSQL_OLD_LIBS"

AC_MSG_CHECKING([MySQL library (required)])
if test "$HAVE_MYSQL" = "yes" ; then
	AC_MSG_RESULT([yes ($MYSQL_VERSION)])
else
	AC_MSG_RESULT([no])
	AC_MSG_ERROR([MySQL not found or incompatible])
fi

AC_SUBST([HAVE_MYSQL])
AC_SUBST([MYSQL_VERSION])
AC_SUBST([MYSQL_CFLAGS])
AC_SUBST([MYSQL_LIBS])


#
# PCRE library (optional)
#

AC_MSG_CHECKING([PCRE library])
AC_CHECK_HEADER([pcre.h], [], [AC_MSG_ERROR([PCRE header not found])])
AC_SEARCH_LIBS([pcre_study], [pcre], [], AC_MSG_ERROR([PCRE not found or incompatible]))

#
# static Support test
#
if test "$enable_static" != "no" ; then

	STATIC="-static"
	PLUGINSTATIC="-static-libgcc"

	OLD_CFLAGS="$CFLAGS"
	OLD_LDFLAGS="$LDFLAGS"
	OLD_LIBS="$LIBS"

	CFLAGS="$OLD_CFLAGS $MYSQL_CFLAGS -static"
	LDFLAGS="$OLD_LDFLAGS -static"
	LIBS="$OLD_LIBS $MYSQL_LIBS"

	AC_MSG_CHECKING([whether $CC supports -static])
	AC_RUN_IFELSE(
		[AC_LANG_SOURCE([
			int main(int argc, char **argv){
				return 0;
			}
		])],
		[
			AC_MSG_RESULT([yes])
		],
		[
			AC_MSG_RESULT([no])
			STATIC=""
		],
		[
			AC_MSG_RESULT([guessing no])
			STATIC=""
		]
	)
	CFLAGS="$OLD_CFLAGS -static-libgcc"
	LDFLAGS="$OLD_LDFLAGS -static-libgcc"

	AC_MSG_CHECKING([whether $CC supports -static-libgcc])
	AC_RUN_IFELSE(
		[AC_LANG_SOURCE([
			int main(int argc, char **argv){
				return 0;
			}
		])],
		[
			AC_MSG_RESULT([yes])
		],
		[
			AC_MSG_RESULT([no])
			PLUGINSTATIC=""
		],
		[
			AC_MSG_RESULT([guessing no])
			PLUGINSTATIC=""
		]
	)

	CFLAGS="$OLD_CFLAGS"
	LDFLAGS="$OLD_LDFLAGS"
	LIBS="$OLD_LIBS"
fi

#
# -Werror compiler flag
#
if test "$enable_Werror" != "no" ; then

	OLD_CFLAGS="$CFLAGS"
	CFLAGS="$CFLAGS -Werror"

	AC_MSG_CHECKING([whether $CC supports -Werror])
	AC_RUN_IFELSE(
		[AC_LANG_SOURCE([
			int main(int argc, char **argv){
				return 0;
			}
		])],
		[
			AC_MSG_RESULT([yes])
		],
		[
			AC_MSG_RESULT([no])
			CFLAGS="$OLD_CFLAGS"
			LDFLAGS="$OLD_LDFLAGS"
		],
		[
			AC_MSG_RESULT([guessing no])
		]
	)
fi

AC_SUBST(STATIC, ${STATIC})
AC_SUBST(PLUGINSTATIC, ${PLUGINSTATIC})



#
# Doxygen, perl (for HPMHookGen)
#

AC_CHECK_PROG([HAVE_DOXYGEN],[doxygen],[yes],[no])

AC_CHECK_PROG([HAVE_PERL],[perl],[yes],[no])

###############################################################################
AC_OUTPUT
