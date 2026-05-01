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

include(CheckIncludeFile)

# Helpers to test existance of header and add definition for its existance
# _header header name without .h
# CDEF name of the def to be defined if the header is found ommit to use default scheme
# REQUIRED flag to fail if check doesn't pass
function(test_header_def _header)
  set(options REQUIRED)
  set(oneValueArgs CDEF)
  cmake_parse_arguments(PARSE_ARGV
    1
    arg
    "${options}"
    "${oneValueArgs}"
    ""
  )

  string(TOUPPER ${_header} _HEADER_UPPER)
  string(REGEX REPLACE "[^A-Za-z0-9_]" "_" _HEADER_UPPER ${_HEADER_UPPER})
  check_include_file(${_header}.h HAVE_${_HEADER_UPPER}_H)
  if(HAVE_${_HEADER_UPPER}_H)
    if(NOT arg_CDEF)
      add_compile_definitions(HAVE_${_HEADER_UPPER}_H=1)
    else()
      add_compile_definitions(${arg_CDEF})
    endif()
  elseif(${arg_REQUIRED})
    message(FATAL_ERROR "Header ${_header}.h is required")
  endif()
endfunction()

# Helpers to test existance of a function and add definition for its existance
# _function function name
# CDEF name of the def to be defined if the function is found ommit to use default scheme
# REQUIRED flag to fail if check doesn't pass
include(CheckFunctionExists)
function(test_function_def _function)
  set(options REQUIRED)
  set(oneValueArgs CDEF)
  cmake_parse_arguments(PARSE_ARGV
    1
    arg
    "${options}"
    "${oneValueArgs}"
    ""
  )

  string(TOUPPER ${_function} _FUNCTION_UPPER)
  check_function_exists(${_function} HAVE_${_FUNCTION_UPPER})
  if(HAVE_${_FUNCTION_UPPER})
    if(NOT arg_CDEF)
      add_compile_definitions(HAVE_${_FUNCTION_UPPER}=1)
    else()
      add_compile_definitions(${arg_CDEF})
    endif()
  elseif(${arg_REQUIRED})
    message(FATAL_ERROR "Function ${_function} is required")
  endif()
endfunction()
