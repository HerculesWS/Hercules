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

# Tests for a sanitizer flag
# _name flag name
# REQUIRED flag to fail if check doesn't pass
function(testadd_sanitizer_flag _name)
  set(options REQUIRED)
  cmake_parse_arguments(PARSE_ARGV
    1
    arg
    "${options}"
    ""
    ""
  )

  string(REGEX REPLACE "[^A-Za-z0-9_]" "_" _TESTADD_SANITIZEFLAG_NAME _TESTADD_SANITIZEFLAG_${_name})
  set(CMAKE_REQUIRED_FLAGS "-Werror -Wfatal-errors")
  check_c_compiler_flag("-fsanitize=${_name}" ${_TESTADD_SANITIZEFLAG_NAME}_FOUND)
  if(NOT ${_TESTADD_SANITIZEFLAG_NAME}_FOUND)
    check_c_compiler_flag("-fsanitize=${_name} -fsanitize-undefined-trap-on-error" SANITIZE_UNDEFINED_TRAP_CHECK)
    if(${SANITIZE_UNDEFINED_TRAP_CHECK})
      add_compile_options("-fsanitize=${_name} -fsanitize-undefined-trap-on-error")
      add_link_options("-fsanitize=${_name} -fsanitize-undefined-trap-on-error")
    elseif(${arg_REQUIRED})
      message(FATAL_ERROR "Sanitizer flag -fsanitize=${_name} is required")
    endif()
  else()
    add_compile_options("-fsanitize=${_name}")
    add_link_options("-fsanitize=${_name}")
  endif()
endfunction()

# Checks if given compiler flag is used
# _name flag name
# _result variable used for boolean result
function(has_compiler_flag _name _result)
  get_directory_property(MYCOMPILEFLAGS COMPILE_OPTIONS)
  if("-${_name}" IN_LIST MYCOMPILEFLAGS)
    set(${_result} ON)
  else()
    set(${_result} OFF)
  endif()
  return(PROPAGATE ${_result})
endfunction()

# Check if compiler accepts flag and append it to a variable
# _result result variable
# _name flag name
# REQUIRED flag to fail if check doesn't pass
function(testwrite_compiler_flag _result _name)
  set(options REQUIRED)
  cmake_parse_arguments(PARSE_ARGV
    2
    arg
    "${options}"
    ""
    ""
  )

  string(REGEX REPLACE "[^A-Za-z0-9_]" "_" _TESTWRITE_CFLAG_NAME _TESTWRITE_CFLAG_${_name})
  set(CMAKE_REQUIRED_FLAGS "-Werror -Wfatal-errors")
  check_c_compiler_flag("-${_name}" ${_TESTWRITE_CFLAG_NAME}_FOUND)
  if(${_TESTWRITE_CFLAG_NAME}_FOUND)
    list(APPEND ${_result} "-${_name}")
    return(PROPAGATE ${_result})
  elseif(${arg_REQUIRED})
    message(FATAL_ERROR "Compiler flag -${_name} is required")
  endif()
endfunction()
