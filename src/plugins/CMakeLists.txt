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

################  PLUGIN CONFIGURATION  ##############################
#                                                                    #
# When you add a plugin, add its name here:                          #
# Example: if you have a plugin named my_cool_plugin.c and another   #
# one named my_second_plugin.c, add them to the list like this:      #
#                                                                    #
# set(MY_PLUGINS                                                     #
#   my_cool_plugin                                                   #
#   my_second_plugin                                                 #
# )                                                                  #
#                                                                    #
# This is only needed if you want to build your plugin through       #
#   'make plugins' or 'make all'. If you don't add it to this list,  #
#   you will still be able to build your plugin through              #
#   'make plugin.my_plugin'                                          #
#                                                                    #
# Note: DO NOT include the .c extension!!!                           #

set(MY_PLUGINS
)

#                                                                    #
#########  DO NOT EDIT ANYTHING BELOW THIS LINE!!!  ##################

set(HERC_PLUGINS
  sample
  httpsample
  db2sql
  constdb2doc
  generate-translations
  mapcache
  script_mapquit
  HPMHooking_api
  HPMHooking_char
  HPMHooking_login
  HPMHooking_map
)

function(add_plugin_target _plugin_name)
  if(${_plugin_name} MATCHES "HPMHooking_.*")
    set(_plugin_c_file "HPMHooking.c")
  else()
    set(_plugin_c_file "${_plugin_name}.c")
  endif()

  source_group("Login header files"
    FILES
    ${_plugin_c_file}
  )

  add_library(${_plugin_name} SHARED ${_plugin_c_file})
  add_custom_target("plugin.${_plugin_name}")
  add_dependencies("plugin.${_plugin_name}" ${_plugin_name})

  target_compile_options(${_plugin_name} PRIVATE ${PLUGIN_COMPILE_OPTIONS})

  target_link_libraries(${_plugin_name} PUBLIC common)
  target_link_libraries(${_plugin_name} PRIVATE cjson::cjson)
  if(ENABLE_LIBBACKTRACE)
    target_link_libraries(${_plugin_name} PRIVATE libbacktrace::libbacktrace)
  endif()
  target_link_libraries(${_plugin_name} PRIVATE mariadb-connector-c::mariadb-connector-c)
  target_link_libraries(${_plugin_name} PRIVATE ZLIB::ZLIB)
  target_link_libraries(${_plugin_name} PRIVATE pcre::pcre)

  # Shouldn't we just turn this into some regex?
  if(${_plugin_name} STREQUAL "HPMHooking_api")
    target_compile_definitions(${_plugin_name} PRIVATE HPMHOOKING_API)
  elseif(${_plugin_name} STREQUAL "HPMHooking_char")
    target_compile_definitions(${_plugin_name} PRIVATE HPMHOOKING_CHAR)
  elseif(${_plugin_name} STREQUAL "HPMHooking_login")
    target_compile_definitions(${_plugin_name} PRIVATE HPMHOOKING_LOGIN)
  elseif(${_plugin_name} STREQUAL "HPMHooking_map")
    target_compile_definitions(${_plugin_name} PRIVATE HPMHOOKING_MAP)
  endif()

  if(${_plugin_name} MATCHES "HPMHooking_.*")
    add_dependencies(plugin.HPMHooking ${_plugin_name})
  else()
    add_dependencies(plugins ${_plugin_name})
  endif()

  install(
    TARGETS ${_plugin_name}
    OPTIONAL
    RUNTIME
      DESTINATION
      plugins
    LIBRARY
      DESTINATION
      plugins
  )

  message(STATUS "Added plugin ${_plugin_name} to build files")
endfunction()

add_custom_target(plugins)
add_custom_target(plugin.HPMHooking)

foreach(PLUGIN_NAME IN LISTS MY_PLUGINS HERC_PLUGINS)
  add_plugin_target(${PLUGIN_NAME})
endforeach()
