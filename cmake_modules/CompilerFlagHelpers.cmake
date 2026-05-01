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

# Helpers to help test and set flags and warnings
include(CheckCCompilerFlag)

# Tests for warning compiler flag
# _name warning flag name without the leading W
# REQUIRED flag to fail if check doesn't pass
function(testadd_warning_compiler_flag _name)
  set(options REQUIRED)
  cmake_parse_arguments(PARSE_ARGV
    1
    arg
    "${options}"
    ""
    ""
  )

  string(REGEX REPLACE "[^A-Za-z0-9_]" "_" _TESTADD_WARNING_FLAG_NAME _TESTADD_WARNING_FLAG_${_name})
  set(CMAKE_REQUIRED_FLAGS "-Werror -Wfatal-errors")
  check_c_compiler_flag("-W${_name}" ${_TESTADD_WARNING_FLAG_NAME}_FOUND)
  if(${_TESTADD_WARNING_FLAG_NAME}_FOUND)
    add_compile_options("-W${_name}")
  elseif(${arg_REQUIRED})
    message(FATAL_ERROR "Compiler warning flag -W${_name} is required")
  endif()
endfunction()

# Tests for compiler flag
# _name flag name
# REQUIRED flag to fail if check doesn't pass
function(testadd_compiler_flag _name)
  set(options REQUIRED)
  cmake_parse_arguments(PARSE_ARGV
    1
    arg
    "${options}"
    ""
    ""
  )

  string(REGEX REPLACE "[^A-Za-z0-9_]" "_" _TESTADD_FLAG_NAME _TESTADD_FLAG_${_name})
  set(CMAKE_REQUIRED_FLAGS "-Werror -Wfatal-errors")
  check_c_compiler_flag("-${_name}" ${_TESTADD_FLAG_NAME}_FOUND)
  if(${_TESTADD_FLAG_NAME}_FOUND)
    add_compile_options("-${_name}")
  elseif(${arg_REQUIRED})
    message(FATAL_ERROR "Compiler flag -${_name} is required")
  endif()
endfunction()

include(CheckLinkerFlag)

# Tests for linker flag
# _name flag name
# REQUIRED flag to fail if check doesn't pass
function(testadd_linker_flag _name)
  set(options REQUIRED)
  cmake_parse_arguments(PARSE_ARGV
    1
    arg
    "${options}"
    ""
    ""
  )

  string(REGEX REPLACE "[^A-Za-z0-9_]" "_" _TESTADD_LFLAG_NAME _TESTADD_LFLAG_${_name})
  set(CMAKE_REQUIRED_FLAGS "-Werror -Wfatal-errors")
  check_linker_flag(C "-${_name}" ${_TESTADD_LFLAG_NAME}_FOUND)
  if(${_TESTADD_LFLAG_NAME}_FOUND)
    add_link_options("-${_name}")
  elseif(${arg_REQUIRED})
    message(FATAL_ERROR "Linker flag -${_name} is required")
  endif()
endfunction()
