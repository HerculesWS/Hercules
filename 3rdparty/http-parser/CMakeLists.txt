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

set(HERC_HTTP_PARSER_SRCS
  http_parser.c
)

set(HERC_HTTP_PARSER_HDRS
  http_parser.h
)

source_group("http_parser source files"
  FILES
  ${HERC_HTTP_PARSER_SRCS}
)

source_group("http_parser header files"
  FILES
  ${HERC_HTTP_PARSER_HDRS}
)

add_library(hercules-http_parser STATIC
  ${HERC_HTTP_PARSER_SRCS}
  ${HERC_HTTP_PARSER_HDRS}
)

if(NOT MSVC)
  target_compile_options(hercules-http_parser PRIVATE -Wno-shadow -Wno-switch-enum)
endif()
