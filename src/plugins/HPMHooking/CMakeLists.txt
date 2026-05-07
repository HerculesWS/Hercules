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

# Plugin name
set(HERC_PLUGIN_NAME HPMHooking)

# List of source files of the plugin
set(HERC_PLUGIN_C
  HPMHooking.c
)

# List of header files of the plugin
set(HERC_PLUGIN_H
  HPMHooking.h
)

# List of include files
set(HPMHOOKING_INCLUDES
  HPMHooking_api.HPMHooksCore.inc
  HPMHooking_api.Hooks.inc
  HPMHooking_api.HookingPoints.inc
  HPMHooking_api.sources.inc
  HPMHooking_char.HPMHooksCore.inc
  HPMHooking_char.Hooks.inc
  HPMHooking_char.HookingPoints.inc
  HPMHooking_char.sources.inc
  HPMHooking_login.HPMHooksCore.inc
  HPMHooking_login.Hooks.inc
  HPMHooking_login.HookingPoints.inc
  HPMHooking_login.sources.inc
  HPMHooking_map.HPMHooksCore.inc
  HPMHooking_map.Hooks.inc
  HPMHooking_map.HookingPoints.inc
  HPMHooking_map.sources.inc
  HPMHooking.Defs.inc
)

# List of servers
set(HPMHOOKING_SERVERS
  api
  char
  login
  map
)

add_custom_target(plugin.HPMHooking)

foreach(server IN LISTS HPMHOOKING_SERVERS)
  string(TOUPPER ${server} server_upper)

  add_plugin_target("${HERC_PLUGIN_NAME}_${server}"
    CFILES
      ${HERC_PLUGIN_C}
      ${HERC_PLUGIN_H}
      ${HPMHOOKING_INCLUDES}
    CDEFS
      "HPMHOOKING_${server_upper}"
    VIRTUAL_TARGET
      "plugin.HPMHooking"
  )
endforeach()
