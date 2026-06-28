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
set(HERC_PLUGIN_NAME test_equippos)

# List of source files of the plugin
set(HERC_PLUGIN_C
  test_equippos.c
)

if(NOT MSVC) # Uses GCC extensions
  # List of compiler flags for the plugin
  set(HERC_PLUGIN_CFLAGS
    -Wno-enum-conversion
  )

  add_plugin_target(${HERC_PLUGIN_NAME}
    CFILES ${HERC_PLUGIN_C}
    CFLAGS ${HERC_PLUGIN_CFLAGS}
  )
endif()
