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

################  TEST CONFIGURATION  ################################
#                                                                    #
# When you add a test, add its name here:                            #
# Example: if you have a test named test_foo.c and another           #
# one named test_bar.c, add them to the list like this:              #
#                                                                    #
#set(HERC_TESTS                                                      #
#  foo                                                               #
#  bar                                                               #
#)                                                                   #
#                                                                    #
# This is only needed if you want to build your plugin through       #
#   'make plugins' or 'make all'. If you don't add it to this list,  #
#   you will still be able to build your plugin through              #
#   'make plugin.my_plugin'                                          #
#                                                                    #
# Note: DO NOT include the .c extension!!!                           #

set(HERC_TESTS
  base62
  chunked
  libconfig
  spinlock
)

#                                                                    #
#########  DO NOT EDIT ANYTHING BELOW THIS LINE!!!  ##################

function(add_test_target _test_name)
  set(_target_name "test_${_test_name}")

  add_executable(${_target_name}
    "${_target_name}.c"
  )

  target_link_libraries(${_target_name} PUBLIC common)

  add_dependencies(tests ${_target_name})

  add_test(
    NAME "${_test_name}"
    COMMAND
      "${_target_name}"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  )

  message(STATUS "Added test ${_test_name} to build files")
endfunction()

add_custom_target(tests)

foreach(TEST_NAME IN LISTS MY_PLUGINS HERC_TESTS)
  add_test_target(${TEST_NAME})
endforeach()
