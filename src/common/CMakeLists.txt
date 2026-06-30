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

set(COMMON_SRCS
  base62.c
  conf.c
  db.c
  des.c
  ers.c
  extraconf.c
  grfio.c
  HPM.c
  mapindex.c
  md5calc.c
  mutex.c
  nullpo.c
  packets.c
  random.c
  showmsg.c
  strlib.c
  sysinfo.c
  thread.c
  timer.c
  utils.c
  console.c
  core.c
  memmgr.c
  socket.c
  sql.c
)

set(COMMON_HDRS
  atomic.h
  cbasetypes.h
  base62.h
  conf.h
  console.h
  core.h
  db.h
  des.h
  ers.h
  extraconf.h
  grfio.h
  hercules.h
  HPM.h
  HPMi.h
  memmgr.h
  memmgr_inc.h
  mapindex.h
  md5calc.h
  mmo.h
  mutex.h
  nullpo.h
  packets.h
  packets_len.h
  packets_struct.h
  random.h
  showmsg.h
  socket.h
  spinlock.h
  sql.h
  strlib.h
  sysinfo.h
  thread.h
  timer.h
  utils.h
  winapi.h
  api.h
  charloginpackets.h
  charmappackets.h
  mapcharpackets.h
  chunked/rfifo.h
  chunked/wfifo.h
  config/defc.h
  config/emblems.h
  config/undefc.h
  ../plugins/HPMHooking/HPMHooking.h
)

set(CONFIG_HDRS
  ../config/const.h
  ../config/core.h
  ../config/renewal.h
  ../config/secure.h
  ../config/classes/general.h
)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
  list(APPEND COMMON_HDRS "${AUTOGEN_DIR}/sysinfo.inc")
endif()

set(COMMON_PRVT
)

source_group("Common source files"
  FILES
  ${COMMON_SRCS}
)

source_group("Common header files"
  FILES
  ${COMMON_HDRS}
  ${CONFIG_HDRS}
)

source_group("Common private header files"
  FILES
  ${COMMON_PRVT}
)

add_library(common STATIC
  ${COMMON_SRCS}
  ${COMMON_HDRS}
  ${COMMON_PRVT}
  ${CONFIG_HDRS}
)

target_link_libraries(common PRIVATE hercules-mt19937ar)
target_link_libraries(common PRIVATE hercules-libconfig)
if(ENABLE_LIBBACKTRACE)
  target_link_libraries(common PRIVATE libbacktrace::libbacktrace)
endif()
target_link_libraries(common PRIVATE mariadb-connector-c::mariadb-connector-c)
target_link_libraries(common PRIVATE ZLIB::ZLIB)

if(COMMON_MEMORY_MANAGER_LINK)
  target_link_libraries(common PRIVATE ${COMMON_MEMORY_MANAGER_LINK})
endif()

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
  # Run sysinfogen.sh whenever common is required in a build
  get_directory_property(SYSGEN_CFLAGS COMPILE_OPTIONS)
  get_directory_property(SYSGEN_DEFS COMPILE_DEFINITIONS)
  get_directory_property(SYSGEN_LDFLAGS LINK_OPTIONS)

  add_custom_target(sysinfo
    COMMAND
      "${CMAKE_SOURCE_DIR}/sysinfogen.sh"
      "${AUTOGEN_DIR}/sysinfo.inc.new"
      "${AUTOGEN_DIR}/sysinfo.inc"
      ${SYSGEN_CFLAGS}
      ${SYSGEN_LDFLAGS}
      ${SYSGEN_DEFS}
    COMMENT "Generate synsinfo.inc"
    DEPENDS "${CMAKE_SOURCE_DIR}/sysinfogen.sh"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    BYPRODUCTS "${AUTOGEN_DIR}/sysinfo.inc"
  )

  add_dependencies(common sysinfo)
endif()

set_source_files_properties(${COMMON_SRCS} PROPERTIES LANGUAGE CXX)
