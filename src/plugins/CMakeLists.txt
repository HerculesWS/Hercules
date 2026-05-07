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

# Creates a hercules plugin target
# _plugin_name the plugin name which will be used for the output file and target name, keep it file system friendly
# CFILES list or lists of plugin files to compile
# CDEFS list or lists of definations to set
# CFLAGS list or lists of compiler options to be set
# VIRTUAL_TARGET a name of a target that this plugin will be added as a dependency to
function(add_plugin_target _plugin_name)
  set(oneValueArgs VIRTUAL_TARGET)
  set(multiValueArgs CFILES CDEFS CFLAGS)
  cmake_parse_arguments(PARSE_ARGV
    1
    arg
    ""
    "${oneValueArgs}"
    "${multiValueArgs}"
  )

  if(NOT arg_CFILES)
    message(FATAL_ERROR "Plugins must define CFILES")
  endif()

  set(_cfiles "")
  set(_cdefs "")
  set(_cflags "")

  foreach(value IN LISTS arg_CFILES)
    list(APPEND _cfiles "${value}")
  endforeach()

  if(arg_CDEFS)
    foreach(value IN LISTS arg_CDEFS)
      list(APPEND _cdefs "${value}")
    endforeach()
  endif()

  if (arg_CFLAGS)
    foreach(value IN LISTS arg_CFLAGS)
      list(APPEND _cflags "${value}")
    endforeach()
  endif()

  add_library(${_plugin_name} SHARED ${_cfiles})
  add_custom_target("plugin.${_plugin_name}")
  add_dependencies("plugin.${_plugin_name}" ${_plugin_name})
  add_dependencies(plugins ${_plugin_name})

  target_compile_options(${_plugin_name} PRIVATE ${PLUGIN_COMPILE_OPTIONS})
  if(arg_CDEFS)
    target_compile_definitions(${_plugin_name} PRIVATE ${_cdefs})
  endif()
  if (arg_CFLAGS)
    target_compile_options(${_plugin_name} PRIVATE ${_cflags})
  endif()
  set_target_properties(${_plugin_name} PROPERTIES PREFIX "")

  target_link_libraries(${_plugin_name} PUBLIC common)
  target_link_libraries(${_plugin_name} PRIVATE cjson::cjson)
  if(ENABLE_LIBBACKTRACE)
    target_link_libraries(${_plugin_name} PRIVATE libbacktrace::libbacktrace)
  endif()
  target_link_libraries(${_plugin_name} PRIVATE mariadb-connector-c::mariadb-connector-c)
  target_link_libraries(${_plugin_name} PRIVATE ZLIB::ZLIB)
  target_link_libraries(${_plugin_name} PRIVATE pcre::pcre)

  if (arg_VIRTUAL_TARGET)
    add_dependencies(${arg_VIRTUAL_TARGET} ${_plugin_name})
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

# Get all items in the current directory
file(GLOB dir_items RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *)

foreach(item ${dir_items})
  if(IS_DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/${item}
    AND
    EXISTS
    ${CMAKE_CURRENT_SOURCE_DIR}/${item}/CMakeLists.txt
  )
    add_subdirectory(${item})
  endif()
endforeach()
