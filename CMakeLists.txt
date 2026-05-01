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

cmake_minimum_required(VERSION 3.25)

project(hercules LANGUAGES C CXX)

# CMake settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Enable compile commands" FORCE)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules")

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "In-source builds are not allowed. Please create a separate build directory.")
endif()

if(NOT CMAKE_BUILD_TYPE)
  message(FATAL_ERROR "Build type not specified. Specify it with -DCMAKE_BUILD_TYPE=type (type can be one of Release, RelWithDebInfo, Debug)")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(RUNNING_ON_LINUX ON)
else()
  set(RUNNING_ON_LINUX OFF)
endif()

include(CMakeDependentOption)
# Configuration options
set(MEMORY_MANAGER "builtin" CACHE STRING "memory managers: none, builtin, memwatch, dmalloc, gcollect, bcheck (defaults to builtin)")
set(PACKETVER "" CACHE STRING "Sets the PACKETVER define. (see src/common/mmo.h)")
option(CASHSHOP_PREVIEW_PATCH "Enable Nemo patch ExtendCashShopPreview." OFF)
option(OLD_CASHSHOP_PREVIEW_PATCH "Enable Nemo patch ExtendOldCashShopPreview." OFF)
option(ENABLE_PACKETVER_RE "Sets or unsets the PACKETVER_RE define - see src/common/mmo.h (currently disabled by default)" OFF)
option(ENABLE_PACKETVER_ZERO "Sets or unsets the PACKETVER_ZERO define - see src/common/mmo.h (currently disabled by default)" OFF)
option(ENABLE_PACKETVER_AD "Sets or unsets the PACKETVER_AD define - see src/common/mmo.h (currently disabled by default)" OFF)
option(ENABLE_PACKETVER_SAK "Sets or unsets the PACKETVER_SAK define - see src/common/mmo.h (currently disabled by default)" OFF)
option(ENABLE_EPOLL "use epoll(4) on Linux" OFF)
set(OBFUSCATIONKEY1 "" CACHE STRING "Sets the first obfuscation key (ignored unless the other two are also specified)")
set(OBFUSCATIONKEY2 "" CACHE STRING "Sets the second obfuscation key (ignored unless the other two are also specified)")
set(OBFUSCATIONKEY3 "" CACHE STRING "Sets the third obfuscation key (ignored unless the other two are also specified)")
option(ENABLE_GDB "Compiles with gdb specific optimzations for better debugging. (off by default)" OFF)
cmake_dependent_option(ENABLE_LIBBACKTRACE "Compiles with libbacktrace. (off by default - experimental)" ON RUNNING_ON_LINUX OFF)
option(ENABLE_BUILDBOT "(available options: ON, OFF)" OFF)
option(ENABLE_RDTSC "Uses rdtsc as timing source (disabled by default)
			Enable it when you've timing issues.

			(For example:  in conjunction with XEN or Other Virtualization mechanisms)

			Note:
				Please ensure that you've disabled dynamic CPU-Frequencys, such as power saving options.
				(On most modern Dedicated Servers cpufreq is preconfigured, see your distribution's
				 manual how to disable it).
				Furthermore, If your CPU has built-in CPU-Frequency scaling features (such as Intel's
				 SpeedStep(R)), do not enable this option. Recent CPUs (Intel Core or newer) guarantee
				 a fixed increment rate for their TSC, so it should be safe to use, but please doublecheck
				 the documentation of both your CPU and OS before enabling this option." OFF)
set(PROFILER "none" CACHE STRING "Profilers: none, gprof (disabled by default)")
option(ENABLE_RENEWAL "Enable renewal" ON)
option(ENABLE_LTO "Enables or Disables Linktime Code Optimization (LTO is disabled by default)" OFF)
option(ENABLE_SANITIZE "Enables sanitizer. (disabled by default)" OFF)
option(ENABLE_WERROR "Enables -Werror in the compiler flags. (disabled by default)" OFF)
set(MAX_CONNECTIONS "" CACHE STRING "optionally set the maximum connections the core can handle (Without epoll enabled, default: 1024. With epol enabled: 3072)")
set(WITH_HTTP_PARSER "http-parser" CACHE STRING "select http parser. Allowed values: http-parser, llhttp")

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
  if(ENABLE_GDB)
    message(FATAL_ERROR "ENABLE_GDB can be only used in debug builds")
  endif()
endif()

include(LanguageFeatureHelper)
# Check for headers
test_header_def(sys/stat)
test_header_def(sys/types)
test_header_def(xlocale) # Libconfig

# Check for functions
test_function_def(uselocale) # Libconfig
test_function_def(newlocale) # Libconfig
test_function_def(freelocale) # Libconfig
test_function_def(setrlimit)

if(MSVC)
  # Warnings
  add_compile_options(/W3 /wd4100 /wd4800)
  add_compile_options(/wd4090) # Disable const qualifier mismatch because msvs doesn't like const passed to our aFree
  add_compile_options(/wd4244 /wd4267) # Disable int conversion possible lose of data
  add_compile_options(/wd4018) # Disable signed/unsigned arthmatic mismatch
  add_compile_options(/wd5287) # Disable enum conversion warning
  add_compile_options(/wd4312) # Disable conversion to greater size warning (see 0xdeadbeef pointers)
  # Definations
  add_compile_definitions(WIN32 _WIN32 __WIN32 _CRT_SECURE_NO_DEPRECATE _CRT_NONSTDC_NO_DEPRECATE FD_SETSIZE=4096 WIN32_LEAN_AND_MEAN)
else()
  # Check for mandatory compile flags (currently it's just a list of all default flag using just ./configure)
  include(CompilerFlagHelpers)
  testadd_compiler_flag(pipe REQUIRED)
  testadd_compiler_flag(ffast-math REQUIRED)
  testadd_compiler_flag(fno-finite-math-only REQUIRED)
  testadd_compiler_flag(fsigned-char REQUIRED)
  testadd_compiler_flag(fcf-protection=none REQUIRED)
  testadd_compiler_flag(fno-strict-aliasing REQUIRED)
  testadd_compiler_flag(fno-omit-frame-pointer REQUIRED)
  testadd_compiler_flag(fno-var-tracking REQUIRED)

  # Warnings we explictily require or disable
  testadd_warning_compiler_flag(all)
  testadd_warning_compiler_flag(extra)
  testadd_warning_compiler_flag(no-sign-compare)
  testadd_warning_compiler_flag(no-unused-parameter)
  testadd_warning_compiler_flag(no-clobbered)
  testadd_warning_compiler_flag(empty-body)
  testadd_warning_compiler_flag(int-conversion)
  testadd_warning_compiler_flag(init-self)
  testadd_warning_compiler_flag(pointer-arith)
  testadd_warning_compiler_flag(switch-bool)
  testadd_warning_compiler_flag(enum-conversion)
  testadd_warning_compiler_flag(format-security)
  testadd_warning_compiler_flag(format)
  testadd_warning_compiler_flag(format-signedness)
  testadd_warning_compiler_flag(format-y2k)
  testadd_warning_compiler_flag(format-overflow=2)
  testadd_warning_compiler_flag(format-diag)
  testadd_warning_compiler_flag(suggest-attribute=noreturn)
  testadd_warning_compiler_flag(suggest-attribute=format)
  testadd_warning_compiler_flag(undef)
  testadd_warning_compiler_flag(nested-externs)
  testadd_warning_compiler_flag(old-style-definition)
  testadd_warning_compiler_flag(overlength-strings)
  testadd_warning_compiler_flag(redundant-decls)
  testadd_warning_compiler_flag(misleading-indentation)
  testadd_warning_compiler_flag(null-dereference)
  testadd_warning_compiler_flag(vla)
  testadd_warning_compiler_flag(vla-parameter)
  testadd_warning_compiler_flag(switch-enum)
  testadd_warning_compiler_flag(mismatched-dealloc)
  testadd_warning_compiler_flag(sizeof-array-div)
  testadd_warning_compiler_flag(stringop-overflow=4)
  testadd_warning_compiler_flag(stringop-overread)
  testadd_warning_compiler_flag(string-compare)
  testadd_warning_compiler_flag(zero-length-bounds)
  testadd_warning_compiler_flag(absolute-value)
  testadd_warning_compiler_flag(shift-overflow=2)
  testadd_warning_compiler_flag(enum-int-mismatch)
  testadd_warning_compiler_flag(no-switch)
  testadd_warning_compiler_flag(no-missing-field-initializers)
  testadd_warning_compiler_flag(no-discarded-qualifiers)
  testadd_warning_compiler_flag(no-unused-but-set-variable)
  testadd_warning_compiler_flag(shadow)
  testadd_warning_compiler_flag(no-deprecated-non-prototype)

  # Linker flags

  # Check if the linker supports/accepts -rdynamic
  # Generally only needed by the ELF linker, in order to produce backtraces.
  # On non-ELF platforms, some compilers (i.e. gcc < 5 and clang on OSX) are able to ignore it, others will error out.
  testadd_linker_flag(rdynamic)
endif()

# Checks for language features and behavior
include(CheckCSourceCompiles)
include(CheckSourceRuns)

# Check if thread local storage is supported
# TODO: remove the check in source as we mandate C11 now
add_compile_definitions(HAS_TLS)

# CLOCK_MONOTONIC clock for clock_gettime
# Normally defines _POSIX_TIMERS > 0 and _POSIX_MONOTONIC_CLOCK (for posix
# compliant systems) and __FreeBSD_cc_version >= 500005 (for FreeBSD
# >= 5.1.0, which does not have the posix defines (ref. r11983)) would be
# checked but some systems define them even when they do not support it
# (ref. bugreport:1003).
check_source_runs(C [[
  #include <sys/time.h>
  #include <time.h>
  #include <unistd.h>

  int main(int argc, char** argv)
  {
    struct timespec tval;
    return clock_gettime(CLOCK_MONOTONIC, &tval);
  }
]] HAVE_MONOTONIC_CLOCK)
if(HAVE_MONOTONIC_CLOCK)
  message(STATUS "CLOCK_MONOTONIC is supported and working")
  add_compile_definitions(HAVE_MONOTONIC_CLOCK)
else()
  message(STATUS "CLOCK_MONOTONIC is either unsupported or not working as expected")
endif()

# Check for libraries
if(ENABLE_LIBBACKTRACE)
  find_package(libbacktrace REQUIRED)
  test_header_def(execinfo CDEF HAVE_EXECINFO)
  testadd_compiler_flag(funwind-tables REQUIRED)
  add_compile_definitions(HAVE_LIBBACKTRACE)
endif()

if(WITH_HTTP_PARSER STREQUAL "http-parser")
  add_compile_definitions(USE_HTTP_PARSER)
  message(STATUS "Use http-parser as http library")
elseif(WITH_HTTP_PARSER STREQUAL "llhttp")
  message(STATUS "Use llhttp as http library")
else()
  message(FATAL_ERROR "unknown http_parser selected '${WITH_HTTP_PARSER}'. Allowed values: http-parser, llhttp. ... stopping")
endif()

find_package(cJSON REQUIRED)
find_package(mariadb-connector-c REQUIRED)
find_package(pcre REQUIRED)
find_package(ZLIB REQUIRED)

# Check for compile settings

# Debug setting
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_definitions(DEBUG)
  if(ENABLE_GDB)
    testadd_compiler_flag(ggdb REQUIRED)
  endif()
endif()

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
  # Run sysinfogen.sh whenever src/common/sysinfo.inc is required in a build
  add_custom_command(
    OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/src/common/sysinfo.inc"
    COMMAND
      ./sysinfogen.sh
      ${CMAKE_CURRENT_SOURCE_DIR}/src/common/sysinfo.inc
      ${CMAKE_C_FLAGS}
      ${CMAKE_CPP_FLAGS}
    DEPENDS sysinfogen.sh
  )
endif()

get_directory_property(MYDEFS LINK_OPTIONS)
message("LDFLAGS=${MYDEFS}")
get_directory_property(MYDEFS COMPILE_OPTIONS)
message("CFLAGS=${MYDEFS}")
get_directory_property(MYDEFS COMPILE_DEFINITIONS)
message("DEFS=${MYDEFS}")

include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/3rdparty)
add_subdirectory(3rdparty)
add_subdirectory(src)
