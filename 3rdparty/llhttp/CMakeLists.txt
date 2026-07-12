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

set(HERC_LLHTTP_SRCS
  api.c
  http.c
  llhttp.c
)

set(HERC_LLHTTP_HDRS
  llhttp.h
)

source_group("llhttp source files"
  FILES
  ${HERC_LLHTTP_SRCS}
)

source_group("llhttp header files"
  FILES
  ${HERC_LLHTTP_HDRS}
)

add_library(hercules-llhttp STATIC
  ${HERC_LLHTTP_SRCS}
  ${HERC_LLHTTP_HDRS}
)

if(NOT MSVC)
  target_compile_options(hercules-llhttp PRIVATE -Wno-error -Wno-format -Wno-format-y2k -Wno-format-security -Wno-redundant-decls -Wno-undef -Wno-switch-enum)
else()
  target_compile_options(hercules-llhttp PRIVATE /wd4702)
endif()
