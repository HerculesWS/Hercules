# This file is part of Hercules.
# http://herc.ws - http://github.com/HerculesWS/Hercules
#
# Copyright (C) 2012-2020  Hercules Dev Team
# Copyright (C)  Athena Dev Teams
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

# 3rdparty/libgif/Makefile.  Generated from Makefile.in by configure.

LIBGIF_OBJ = dgif_lib.o egif_lib.o gifalloc.o gif_err.o gif_font.o gif_hash.o openbsd-reallocarray.o
LIBGIF_H = gif_hash.h gif_lib.h gif_lib_private.h



CC = ccache gcc
export CC

#####################################################################
.PHONY: all clean buildclean help

all: $(LIBGIF_OBJ)

buildclean:
	@echo "	CLEAN	libgif (build temp files)"
	@rm -rf *.o

clean: buildclean
	@echo "	CLEAN	libgif"

help:
	@echo "possible targets are 'all' 'clean' 'buildclean' 'help'"
	@echo "'all'                 - builds $(LIBGIF_OBJ)"
	@echo "'clean', 'buildclean' - deletes $(LIBGIF_OBJ)"
	@echo "'help'                - outputs this message"

#####################################################################

Makefile: Makefile.in
	@$(MAKE) -C ../.. 3rdparty/libgif/Makefile

%.o: %.c $(LIBGIF_H) Makefile
	@echo "	CC	$<"
	@$(CC) -g -O2 -std=c11 -pipe -ffast-math -Wall -Wextra -Wno-sign-compare -fsanitize=address -fsanitize=shift -fsanitize=integer-divide-by-zero -fsanitize=unreachable -fsanitize=vla-bound -fsanitize=null -fsanitize=return -fsanitize=signed-integer-overflow -fsanitize=bounds -fsanitize=bounds-strict -fsanitize=object-size -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fsanitize=nonnull-attribute -fsanitize=returns-nonnull-attribute -fsanitize=bool -fsanitize=enum -fsanitize=vptr -fsanitize=pointer-overflow -fsanitize=builtin -fsanitize=pointer-compare -fsanitize=pointer-subtract -fsanitize=shift-exponent -fsanitize=shift-base -fsigned-char -fcf-protection=none -Wno-unused-parameter -Wno-clobbered -Wempty-body -Wint-conversion -Winit-self -Wpointer-arith -Wswitch-bool -Wenum-conversion -Wformat-security -Wformat -Wformat-signedness -Wformat-y2k -Wformat-overflow=2 -Wformat-diag -Wsuggest-attribute=noreturn -Wsuggest-attribute=format -Wundef -Wnested-externs -Wold-style-definition -Woverlength-strings -Wredundant-decls -Wmisleading-indentation -Wnull-dereference -Wvla -Wvla-parameter -Wswitch-enum -Wmismatched-dealloc -Wsizeof-array-div -Wstringop-overflow=4 -Wstringop-overread -Wstring-compare -Wzero-length-bounds -Wabsolute-value -Wshift-overflow=2 -Wno-switch -Wno-missing-field-initializers -Wno-discarded-qualifiers -Wshadow -fno-strict-aliasing -ggdb -fno-omit-frame-pointer -funwind-tables -fno-var-tracking -DHAVE_EXECINFO -Werror -DPACKAGE_NAME=\"\" -DPACKAGE_TARNAME=\"\" -DPACKAGE_VERSION=\"\" -DPACKAGE_STRING=\"\" -DPACKAGE_BUGREPORT=\"\" -DPACKAGE_URL=\"\" -DHAVE_STDIO_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_STRINGS_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_UNISTD_H=1 -DHAVE_WCHAR_H=1 -DSTDC_HEADERS=1 -D_ALL_SOURCE=1 -D_DARWIN_C_SOURCE=1 -D_GNU_SOURCE=1 -D_HPUX_ALT_XOPEN_SOCKET_API=1 -D_NETBSD_SOURCE=1 -D_OPENBSD_SOURCE=1 -D_POSIX_PTHREAD_SEMANTICS=1 -D__STDC_WANT_IEC_60559_ATTRIBS_EXT__=1 -D__STDC_WANT_IEC_60559_BFP_EXT__=1 -D__STDC_WANT_IEC_60559_DFP_EXT__=1 -D__STDC_WANT_IEC_60559_FUNCS_EXT__=1 -D__STDC_WANT_IEC_60559_TYPES_EXT__=1 -D__STDC_WANT_LIB_EXT2__=1 -D__STDC_WANT_MATH_SPEC_FUNCS__=1 -D_TANDEM_SOURCE=1 -D__EXTENSIONS__=1 -DHAVE_USELOCALE=1 -DHAVE_NEWLOCALE=1 -DHAVE_FREELOCALE=1 -DHAVE_GETIPINFO=1 -DHAVE_SYNC_FUNCTIONS=1 -DHAVE_ATOMIC_FUNCTIONS=1 -DBACKTRACE_ELF_SIZE=64 -DBACKTRACE_XCOFF_SIZE=unused -DHAVE_DL_ITERATE_PHDR=1 -DHAVE_FCNTL=1 -DHAVE_DECL_STRNLEN=1 -DHAVE_LSTAT=1 -DHAVE_READLINK=1 -DHAVE_EXECINFO_H=1  -DSANITIZE -DHTTP_PARSER_INCLUDE= -I../common -DHAS_TLS -DHAVE_SETRLIMIT -DHAVE_STRNLEN -DNO_MEMMGR -DPACKETVER=20220504 -DENABLE_CASHSHOP_PREVIEW_PATCH -DSOCKET_EPOLL -DDEBUG -DHAVE_LIBBACKTRACE -DHAVE_MONOTONIC_CLOCK -c $(OUTPUT_OPTION) $<
