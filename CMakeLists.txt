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
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(CMAKE_INSTALL_LIBDIR "lib")
else()
  set(CMAKE_INSTALL_LIBDIR ".")
endif()
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules")

# RPATH settings
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH OFF)

  if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(CMAKE_INSTALL_RPATH "@executable_path/${CMAKE_INSTALL_LIBDIR}/")
    set(PLUGIN_INSTALL_RPATH "@executable_path/../${CMAKE_INSTALL_LIBDIR}/") # One directory up for plugins
  else()
    set(CMAKE_INSTALL_RPATH "\$ORIGIN/${CMAKE_INSTALL_LIBDIR}/") # Linux and modern BSD systems supports $ORIGIN
    set(PLUGIN_INSTALL_RPATH "\$ORIGIN/../${CMAKE_INSTALL_LIBDIR}/") # One directory up for plugins
  endif()
endif()

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "In-source builds are not allowed. Please create a separate build directory.")
endif()

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set_property(CACHE CMAKE_INSTALL_PREFIX PROPERTY VALUE "${CMAKE_SOURCE_DIR}/bin")
endif()

if(NOT CMAKE_BUILD_TYPE)
  message(FATAL_ERROR "Build type not specified. Specify it with -DCMAKE_BUILD_TYPE=type (type can be one of Release, RelWithDebInfo, Debug)")
endif()

# Global variables
set(PLUGIN_COMPILE_OPTIONS "")
set_property(GLOBAL PROPERTY ALL_CONFIG_HDRS "") # used to store all config headers made as property to make it global

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(RUNNING_ON_LINUX ON)
else()
  set(RUNNING_ON_LINUX OFF)
endif()

# Auto generated files directory in build dir
set(AUTOGEN_DIR "${CMAKE_BINARY_DIR}/autogen")
file(MAKE_DIRECTORY "${AUTOGEN_DIR}")
include_directories("${AUTOGEN_DIR}")

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
set(ENABLE_PROFILER "none" CACHE STRING "Profilers: none, gprof (disabled by default)")
option(ENABLE_RENEWAL "Enable renewal" ON)
option(ENABLE_LTO "Enables or Disables Linktime Code Optimization (LTO is disabled by default)" OFF)
option(ENABLE_SANITIZE "Enables sanitizer. (disabled by default)" OFF)
option(ENABLE_WERROR "Enables -Werror in the compiler flags. (disabled by default)" OFF)
set(MAX_CONNECTIONS "" CACHE STRING "optionally set the maximum connections the core can handle (Without epoll enabled, default: 1024. With epol enabled: 3072)")
set(WITH_HTTP_PARSER "http-parser" CACHE STRING "select http parser. Allowed values: http-parser, llhttp")
option(ENABLE_CLASSIC_AUTOSPELL "Enables classic auto spell list" OFF)
option(ENABLE_TESTING "Enables compiling and running of tests" OFF)

if(CMAKE_C_BYTE_ORDER STREQUAL "BIG_ENDIAN")
  message(FATAL_ERROR "bigendian is not supported... stopping")
elseif(NOT CMAKE_C_BYTE_ORDER STREQUAL "LITTLE_ENDIAN")
  set(WORDS_BIGENDIAN FALSE)
  message(WARNING "unable to determine endianess, only little endian is supported")
endif()

if (ENABLE_TESTING)
  enable_testing()
endif()

# [START] conan settings
# MAKE SURE THIS BLOCK EXISTS BEFORE THE FIRST find_package() CALL OR ELSE IT WON'T WORK
set(HERCULES_CONAN_NAME "hercules")

list(APPEND CONAN_INSTALL_ARGS "--deployer=runtime_deploy" "--deployer-folder=runtime-lib")
install(
  DIRECTORY "${CMAKE_BINARY_DIR}/runtime-lib/"
  TYPE LIB
)

if(ENABLE_LIBBACKTRACE)
  list(APPEND CONAN_INSTALL_ARGS "-o" "${HERCULES_CONAN_NAME}/*:with_libbacktrace=True")
endif()

# [END] conan settings

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
  # Check for mandatory compile flags
  include(CompilerFlagHelpers)
  testadd_compiler_flag(pipe REQUIRED)
  testadd_compiler_flag(ffast-math REQUIRED)
  testadd_compiler_flag(fsigned-char)
  testadd_compiler_flag(fcf-protection=none)
  testadd_compiler_flag(fno-strict-aliasing REQUIRED)
  testadd_compiler_flag(fno-omit-frame-pointer REQUIRED)
  testadd_compiler_flag(fno-var-tracking)

  # Warnings we explictily require or disable
  testadd_warning_compiler_flag(all REQUIRED)
  testadd_warning_compiler_flag(extra REQUIRED)
  testadd_warning_compiler_flag(no-sign-compare REQUIRED)
  testadd_warning_compiler_flag(no-unused-parameter)
  testadd_warning_compiler_flag(no-clobbered)
  testadd_warning_compiler_flag(empty-body)
  testadd_warning_compiler_flag(newline-eof)
  testadd_warning_compiler_flag(int-conversion)
  testadd_warning_compiler_flag(init-self)
  testadd_warning_compiler_flag(pointer-arith)
  testadd_warning_compiler_flag(switch-bool)
  testadd_warning_compiler_flag(enum-conversion)
  testadd_warning_compiler_flag(shorten-64-to-32)
  testadd_warning_compiler_flag(constant-conversion)
  testadd_warning_compiler_flag(bool-conversion)
  testadd_warning_compiler_flag(format-security)
  testadd_warning_compiler_flag(format)
  testadd_warning_compiler_flag(format-signedness)
  testadd_warning_compiler_flag(format-y2k)
  testadd_warning_compiler_flag(format-overflow=2)
  testadd_warning_compiler_flag(format-diag)
  # testadd_warning_compiler_flag(format-truncation=2)
  testadd_warning_compiler_flag(missing-include-dirs)
  testadd_warning_compiler_flag(suggest-attribute=noreturn)
  testadd_warning_compiler_flag(suggest-attribute=format)
  # testadd_warning_compiler_flag(suggest-attribute=malloc)
  testadd_warning_compiler_flag(undef)
  #testadd_warning_compiler_flag(cast-align)
  #testadd_warning_compiler_flag(logical-op) # some useless warnings
  testadd_warning_compiler_flag(nested-externs)
  testadd_warning_compiler_flag(old-style-definition)
  testadd_warning_compiler_flag(overlength-strings)
  testadd_warning_compiler_flag(redundant-decls)
  # testadd_warning_compiler_flag(cast-qual)
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
  #testadd_warning_compiler_flag(unreachable-code)
  #testadd_warning_compiler_flag(unused-const-variable=2)
  #testadd_warning_compiler_flag(write-strings)
  #testadd_warning_compiler_flag(no-format-nonliteral)
  testadd_warning_compiler_flag(no-switch)
  testadd_warning_compiler_flag(no-missing-field-initializers)
  # testadd_warning_compiler_flag(no-suggest-attribute=format)
  testadd_warning_compiler_flag(no-incompatible-pointer-types-discards-qualifiers)
  testadd_warning_compiler_flag(no-discarded-qualifiers)
  testadd_warning_compiler_flag(no-deprecated-non-prototype)
  testadd_warning_compiler_flag(no-unused-but-set-variable)
  testadd_warning_compiler_flag(no-enum-enum-conversion)

  if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    testadd_warning_compiler_flag(no-deprecated-declarations)
  endif()

  # Certain versions of gcc make -Wshadow completely useless by making it flood
  # you with unnecessary warnings <https://lkml.org/lkml/2006/11/28/239>
  # Let's check if we can really use it
  set(OLD_CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS}")
  set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -Werror -Wshadow")
  check_c_source_compiles([[
    int foo(void) {
      return 0;
    }

    int main(int argc, int *argv[]) {
      int foo = 0;
      return foo + 1;
    }
  ]] IS_WSHADOW_GOOD)
  if(IS_WSHADOW_GOOD)
    testadd_warning_compiler_flag(shadow)
  endif()
  set(CMAKE_REQUIRED_FLAGS "${OLD_CMAKE_REQUIRED_FLAGS}")

  # Certain versions of gcc implement -Wformat-truncation in a naive way,
  # disregarding the actual values that an integer can take in every code path
  # and assuming a worst case of INT_MAX. That makes it not very useful as it
  # produces meaningless warnings.
  has_compiler_flag(Wno-format-truncation HAS_NO_FORMAT_TRANSACTION)
  if(NOT HAS_NO_FORMAT_TRANSACTION)
    set(OLD_CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS}")
    set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -Werror")
    check_c_source_compiles([[
      #include <stdio.h>
      int foo(int argc, char *argv[]) {
        char buf[3];
        for (int i = 0; i < 4; i++) {
          snprintf(buf, sizeof(buf), "a%d", i);
        }
        return buf[1];
      }
    ]] IS_FORMAT_TRANSACTION_GOOD)

    if(NOT IS_FORMAT_TRANSACTION_GOOD)
      testadd_warning_compiler_flag(no-format-truncation)
    endif()
    set(CMAKE_REQUIRED_FLAGS "${OLD_CMAKE_REQUIRED_FLAGS}")
  endif()

  # Plugin flags
  testwrite_compiler_flag(PLUGIN_COMPILE_OPTIONS fPIC)

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

# Check if epoll is available and we're on linux
if(ENABLE_EPOLL)
  check_source_runs(C [[
    #ifndef __linux__
    #error This is not Linux
    #endif
    #include <sys/epoll.h>
    int main(int argc, char** argv)
    {
      return 0;
    }
  ]] HAVE_LINUX_EPOLL)

  if(NOT HAVE_LINUX_EPOLL)
    message(FATAL_ERROR "epoll support explicitly enabled but not available")
  else()
    add_compile_definitions(SOCKET_EPOLL)
  endif()
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
find_package(GIF REQUIRED)

# Check for compile settings

# Memory manager setting
if(MEMORY_MANAGER STREQUAL "none")
  add_compile_definitions(NO_MEMMGR)
elseif(MEMORY_MANAGER STREQUAL "memwatch")
  test_header_def(memwatch REQUIRED)
  add_compile_definitions(MEMWATCH)
elseif(MEMORY_MANAGER STREQUAL "dmalloc")
  test_header_def(dmalloc REQUIRED)
  add_compile_definitions(DMALLOC DMALLOC_FUNC_CHECK)
  set(COMMON_MEMORY_MANAGER_LINK "dmalloc")
elseif(MEMORY_MANAGER STREQUAL "gcollect")
  test_header_def(gc REQUIRED)
  add_compile_definitions(GCOLLECT)
  set(COMMON_MEMORY_MANAGER_LINK "gc")
elseif(MEMORY_MANAGER STREQUAL "bcheck")
  add_compile_definitions(BCHECK)
elseif(NOT MEMORY_MANAGER STREQUAL "builtin")
  message(FATAL_ERROR "Unknown memory manager chosen ${MEMORY_MANAGER}")
endif()

add_compile_definitions(
  $<$<BOOL:${PACKETVER}>:PACKETVER=${PACKETVER}>
  $<$<BOOL:${ENABLE_PACKETVER_RE}>:ENABLE_PACKETVER_RE>
  $<$<BOOL:${ENABLE_PACKETVER_ZERO}>:ENABLE_PACKETVER_ZERO>
  $<$<BOOL:${ENABLE_PACKETVER_SAK}>:ENABLE_PACKETVER_SAK>
  $<$<BOOL:${ENABLE_PACKETVER_AD}>:ENABLE_PACKETVER_AD>
  $<$<BOOL:${CASHSHOP_PREVIEW_PATCH}>:ENABLE_CASHSHOP_PREVIEW_PATCH>
  $<$<BOOL:${OLD_CASHSHOP_PREVIEW_PATCH}>:ENABLE_OLD_CASHSHOP_PREVIEW_PATCH>
  $<$<BOOL:${ENABLE_EPOLL}>:SOCKET_EPOLL>
  $<$<BOOL:${ENABLE_BUILDBOT}>:BUILDBOT>
  $<$<BOOL:${ENABLE_RDTSC}>:ENABLE_RDTSC>
  $<$<BOOL:${ENABLE_CLASSIC_AUTOSPELL}>:CLASSIC_AUTOSPELL_LIST>
  $<$<NOT:$<BOOL:${ENABLE_RENEWAL}>>:DISABLE_RENEWAL>
)

if(NOT "${OBFUSCATIONKEY1}" STREQUAL "" OR NOT "${OBFUSCATIONKEY2}" STREQUAL "" OR NOT "${OBFUSCATIONKEY3}" STREQUAL "")
  if("${OBFUSCATIONKEY1}" STREQUAL "" OR "${OBFUSCATIONKEY2}" STREQUAL "" OR "${OBFUSCATIONKEY3}" STREQUAL "")
    message(FATAL_ERROR "Less than three obfuscation keys have been specified. They will all be ignored.")
  elseif(NOT OBFUSCATIONKEY1 MATCHES "^0x[A-Fa-f0-9][A-Fa-f0-9][A-Fa-f0-9][A-Fa-f0-9][A-Fa-f0-9][A-Fa-f0-9][A-Fa-f0-9][A-Fa-f0-9]$")
    message(FATAL_ERROR "Invalid OBFUSCATIONKEY1 format specified. It should be in the 0xNNNNNNNN format (with 8 hex digits).")
  elseif(NOT OBFUSCATIONKEY2 MATCHES "^0x[A-Fa-f0-9][A-Fa-f0-9][A-Fa-f0-9][A-Fa-f0-9][A-Fa-f0-9][A-Fa-f0-9][A-Fa-f0-9][A-Fa-f0-9]$")
    message(FATAL_ERROR "Invalid OBFUSCATIONKEY2 format specified. It should be in the 0xNNNNNNNN format (with 8 hex digits).")
  elseif(NOT OBFUSCATIONKEY3 MATCHES "^0x[A-Fa-f0-9][A-Fa-f0-9][A-Fa-f0-9][A-Fa-f0-9][A-Fa-f0-9][A-Fa-f0-9][A-Fa-f0-9][A-Fa-f0-9]$")
    message(FATAL_ERROR "Invalid OBFUSCATIONKEY3 format specified. It should be in the 0xNNNNNNNN format (with 8 hex digits).")
  else()
    add_compile_definitions("OBFUSCATIONKEY1=${OBFUSCATIONKEY1}" "OBFUSCATIONKEY2=${OBFUSCATIONKEY2}" "OBFUSCATIONKEY3=${OBFUSCATIONKEY3}")
  endif()
endif()

# Debug setting
if(NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug" AND NOT "${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
  if(${ENABLE_GDB})
    message(FATAL_ERROR "ENABLE_GDB can be only used in debug builds")
  endif()
else()
  if(${ENABLE_GDB})
    testadd_compiler_flag(ggdb REQUIRED)
  endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_definitions(DEBUG)
endif()

if(ENABLE_PROFILER STREQUAL "none")
elseif(ENABLE_PROFILER STREQUAL "gprof")
  add_compile_options(pg)
  add_link_options(pg)
else()
  message(FATAL_ERROR "Unknown profiler option ${ENABLE_PROFILER}")
endif()

if(ENABLE_WERROR)
  if (MSVC)
    add_compile_options(/WX)
  else()
    testadd_warning_compiler_flag(error REQUIRED)
  endif()
endif()

if(ENABLE_LTO)
  if (MSVC)
    message(FATAL_ERROR "LTO Cannot be enabled on MSVC builds")
  endif()
  testadd_compiler_flag(flto REQUIRED)
  testadd_compiler_flag(ffat-lto-objects REQUIRED)
  testadd_linker_flag(flto REQUIRED)
  testadd_linker_flag(ffat-lto-objects REQUIRED)
endif()

if(ENABLE_SANITIZE)
  if (MSVC)
    message(FATAL_ERROR "Sanitizer Cannot be enabled on MSVC builds")
  endif()
  add_compile_definitions(SANITIZE)
  # skipped because server have multiple issues: -fsanitize=alignment
  testadd_sanitizer_flag(address)
  testadd_sanitizer_flag(shift)
  testadd_sanitizer_flag(integer-divide-by-zero)
  testadd_sanitizer_flag(unreachable)
  testadd_sanitizer_flag(vla-bound)
  testadd_sanitizer_flag(null)
  testadd_sanitizer_flag(return)
  testadd_sanitizer_flag(signed-integer-overflow)
  testadd_sanitizer_flag(bounds)
  testadd_sanitizer_flag(bounds-strict)
  testadd_sanitizer_flag(object-size)
  testadd_sanitizer_flag(float-divide-by-zero)
  testadd_sanitizer_flag(float-cast-overflow)
  testadd_sanitizer_flag(nonnull-attribute)
  testadd_sanitizer_flag(returns-nonnull-attribute)
  testadd_sanitizer_flag(bool)
  testadd_sanitizer_flag(enum)
  testadd_sanitizer_flag(vptr)
  testadd_sanitizer_flag(address-use-after-scope)
  testadd_sanitizer_flag(pointer-overflow)
  testadd_sanitizer_flag(builtin)
  testadd_sanitizer_flag(pointer-compare)
  testadd_sanitizer_flag(pointer-subtract)
  testadd_sanitizer_flag(shift-exponent)
  testadd_sanitizer_flag(shift-base)
  testadd_sanitizer_flag(sanitize-address-use-after-scope)
endif()

if(MAX_CONNECTIONS)
  if(MAX_CONNECTIONS GREATER 0)
    add_compile_definitions("MAXCONN=${MAX_CONNECTIONS}")
  endif()
endif()

# Checks for external programs that may be needed in some targets/commands
find_program(DOXYGEN_FOUND doxygen)
find_program(PERL_FOUND perl)
find_program(PYTHON3_FOUND python3)

# Checks and creates the import conf directory
if(EXISTS "${CMAKE_SOURCE_DIR}/conf/import")
  message(STATUS "config import directory already exists.")
else()
  message(STATUS "creating config import directory ${CMAKE_SOURCE_DIR}/conf/import")
  file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/conf/import")
endif()

# Check import templates and copy them if not already copied
file(GLOB_RECURSE import_tmpl_items RELATIVE "${CMAKE_SOURCE_DIR}/conf/import-tmpl/" CONFIGURE_DEPENDS "conf/import-tmpl/*")
foreach(tmpl_name ${import_tmpl_items})
  if(NOT EXISTS "${CMAKE_SOURCE_DIR}/conf/import/${tmpl_name}")
    message(STATUS "Creating ${CMAKE_SOURCE_DIR}/conf/import/${tmpl_name}")
    file(COPY_FILE "${CMAKE_SOURCE_DIR}/conf/import-tmpl/${tmpl_name}" "${CMAKE_SOURCE_DIR}/conf/import/${tmpl_name}")
  endif()
endforeach()

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
add_subdirectory(tools/doxygen)
add_subdirectory(tools/HPMHookGen)
