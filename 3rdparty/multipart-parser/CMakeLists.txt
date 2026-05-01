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

set(HERC_MULTIPART_PARSER_SRCS
  multipartparser.c
)

set(HERC_MULTIPART_PARSER_HDRS
  multipartparser.h
)

source_group("multipart-parser source files"
  FILES
  ${HERC_MULTIPART_PARSER_SRCS}
)

source_group("multipart-parser header files"
  FILES
  ${HERC_MULTIPART_PARSER_HDRS}
)

add_library(hercules-multipart-parser STATIC
  ${HERC_MULTIPART_PARSER_SRCS}
  ${HERC_MULTIPART_PARSER_HDRS}
)

if(NOT MSVC)
  get_target_property(CURRENT_OPTIONS hercules-multipart-parser COMPILE_OPTIONS)
  if("-Wshorten-64-to-32" IN_LIST CURRENT_OPTIONS)
    target_compile_options(hercules-multipart-parser PRIVATE -Wno-shorten-64-to-32)
  endif()
endif()
