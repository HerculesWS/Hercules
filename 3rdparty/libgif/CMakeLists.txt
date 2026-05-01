# This file is part of Hercules.
# http://herc.ws - http://github.com/HerculesWS/Hercules
#
# Copyright (C) 2026 Hercules Dev Team
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

set(HERC_LIBGIF_SRCS
  dgif_lib.c
  egif_lib.c
  gifalloc.c
  gif_err.c
  gif_font.c
  gif_hash.c
  openbsd-reallocarray.c
)

set(HERC_LIBGIF_HDRS
  gif_hash.h
  gif_lib.h
  gif_lib_private.h
)

source_group("libgif source files"
  FILES
  ${HERC_LIBGIF_SRCS}
)

source_group("libgif header files"
  FILES
  ${HERC_LIBGIF_HDRS}
)

add_library(hercules-libgif STATIC
  ${HERC_LIBGIF_SRCS}
  ${HERC_LIBGIF_HDRS}
)

if(NOT MSVC)
  target_compile_options(hercules-libgif PRIVATE -Wno-switch-enum -Wno-undef)

  get_target_property(CURRENT_OPTIONS hercules-libgif COMPILE_OPTIONS)
  if("-Wshorten-64-to-32" IN_LIST CURRENT_OPTIONS)
    target_compile_options(hercules-libgif PRIVATE -Wno-shorten-64-to-32)
  endif()
endif()
