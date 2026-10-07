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

set(LOGIN_SRCS
  account.cpp
  HPMlogin.cpp
  ipban.cpp
  lclif.cpp
  login.cpp
  loginlog.cpp
  lapiif.cpp
)

set(LOGIN_HDRS
  login.h
  account.h
  apipackets.h
  HPMlogin.h
  ipban.h
  lclif.h
  loginlog.h
  lapiif.h
)

set(LOGIN_PRVT
  lclif.p.h
)

source_group("Login source files"
  FILES
  ${LOGIN_SRCS}
)

source_group("Login header files"
  FILES
  ${LOGIN_HDRS}
)

source_group("Login private header files"
  FILES
  ${LOGIN_PRVT}
)

add_library(herc-login SHARED
  ${LOGIN_SRCS}
  ${LOGIN_HDRS}
  ${LOGIN_PRVT}
)

target_compile_definitions(herc-login
  PRIVATE
    HERCULES_CORE
    HERCULES_CORE_LOGIN
)

target_link_libraries(herc-login PUBLIC herc-common)
if(WIN32)
  target_link_libraries(herc-login PRIVATE wsock32 ws2_32)
endif()

add_executable(login-server
  main.cpp
)

target_compile_definitions(login-server
  PRIVATE
    HERCULES_CORE
)

target_link_libraries(login-server PUBLIC herc-login)

add_dependencies(sql login-server)

install(
  TARGETS
    login-server
    herc-login
  RUNTIME
    DESTINATION
    .
)
