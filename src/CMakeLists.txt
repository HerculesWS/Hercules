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

add_custom_target(sql) # used to emulate the behavior of old makefiles to compile all servers

add_subdirectory(api)
add_subdirectory(common)
add_subdirectory(char)
add_subdirectory(login)
add_subdirectory(map)
if(BUILD_PLUGINS)
  add_subdirectory(plugins)
endif()
if(ENABLE_TESTING)
  add_subdirectory(test)
endif()
