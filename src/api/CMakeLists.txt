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

set(API_SRCS
  api.c
  aclif.c
  aloginif.c
  HPMapi.c
  httpparser.c
  httpsender.c
  handlers.c
  imageparser.c
  jsonparser.c
  jsonwriter.c
  postheader.c
)

set(API_HDRS
  api.h
  apipackets.h
  aclif.h
  aloginif.h
  HPMapi.h
  apisessiondata.h
  httpparser.h
  httpparsehandler.h
  handlerfunc.h
  handlers.h
  mimepart.h
  urlhandlers.h
  imageparser.h
  jsonparser.h
  jsonwriter.h
  http_include.h
)

set(API_PRVT
)

source_group("API source files"
  FILES
  ${API_SRCS}
)

source_group("API header files"
  FILES
  ${API_HDRS}
)

source_group("API private header files"
  FILES
  ${API_PRVT}
)

add_executable(api-server
  ${API_SRCS}
  ${API_HDRS}
  ${API_PRVT}
)

target_link_libraries(api-server PUBLIC common)
if(WITH_HTTP_PARSER STREQUAL "http-parser")
  target_link_libraries(api-server PRIVATE hercules-http_parser)
else()
  target_link_libraries(api-server PRIVATE hercules-llhttp)
endif()
target_link_libraries(api-server PRIVATE hercules-multipart-parser)
target_link_libraries(api-server PRIVATE cjson::cjson)
target_link_libraries(api-server PRIVATE GIF::GIF)

add_dependencies(sql api-server)

install(
  TARGETS api-server
  RUNTIME
    DESTINATION
    .
)
