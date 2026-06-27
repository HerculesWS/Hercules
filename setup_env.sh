#!/usr/bin/printf This script should be sourced. Run the following:\n  . %q %q %q %q %q %q %q %q %q %q\n

# This file is part of Hercules.
# http://herc.ws - http://github.com/HerculesWS/Hercules
#
# Copyright (C) 2026 Hercules Dev Team
# Copyright (C) 2026 Haru
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

# This script should be sourced, not run directly through a new shell
sourced=0
if [ -n "$ZSH_VERSION" ]; then
  case $ZSH_EVAL_CONTEXT in
    *:file)
      sourced=1
      ;;
  esac
elif [ -n "$KSH_VERSION" ]; then
  [ "$(cd -- "$(dirname -- "$0")" && pwd -P)/$(basename -- "$0")" != "$(cd -- "$(dirname -- "${.sh.file}")" && pwd -P)/$(basename -- "${.sh.file}")" ] \
    && sourced=1
elif [ -n "$BASH_VERSION" ]; then
  (return 0 2>/dev/null) \
    && sourced=1
else
  case ${0##*/} in
    sh|-sh|dash|-dash)
      sourced=1
      ;;
  esac
fi
if [ $sourced -ne 1 ]; then
  echo "This script should be sourced, not executed in a new shell."
  echo " "
  echo "The purpose of this script is to set up the current shell with all the required variables to build Hercules."
  echo "As such it should be sourced into an existing shell. It's meaningless to execute it in a new shell or subshell."
  echo " "
  echo "Please run it through . (or source):"
  echo "  . $0 [optional arguments]"
  echo "or"
  echo "  source $0 [optional arguments]"
  echo " "
  echo "For usage instructions, use:"
  echo "  . $0 --help"
  exit 1
fi

usage() {
  cat << EOF
SYNOPSIS
  Set up build environment for Hercules

SYNTAX
  . ./setup_env.sh [--help] [--build-dir <dirname>] [--build-type <string>]
EOF

  [ "$1" = "full" ] && cat << EOF

DESCRIPTION
  This script sets up the build environment for Hercules:
    - If conan is not installed, it will be configured in a local venv and activated.
    - If a CMake configuration is not detected in the build directory, a new default configuration will be created.
      - If ninja is available, it will be configured as build tool. Otherwise makefiles will be used.
      - The RelWithDebInfo mode will be used unless otherwise specified.
      - The build output will be configured into the "build" directory unless otherwise specified.

  This script can be safely reused with the same arguments to re-enter a configured build by setting up the appropriate environment variables.

  Note: this script should be sourced (through the "." or "source" commands) rather than in a subshell.

PARAMETERS
  --help | -h
      Show this help message

  --build-dir <dirname>
      Specify a different build directory name (default: "build")

  --build-type <string>
      Specify a different build type (one of "Debug", "Release", "RelWithDebInfo", default: "RelWithDebInfo")
EOF
}

build_dir="build"
build_type="RelWithDebInfo"
# parse arguments
while [ -n "$1" ]; do
  case "$1" in
  --build-dir)
      if [ -z "$2" ]; then
        echo "Error: --build-dir requires an additional argument."
        echo " "
        usage short
        return 1
      fi
      build_dir="$2"
      shift 2
    ;;
  --build-type)
      if [ -z "$2" ]; then
        echo "Error: --build-type requires an additional argument."
        echo " "
        usage short
        return 1
      fi
      case "$2" in
        Debug|Release|RelWithDebInfo)
          build_type="$2"
          ;;
        *)
          echo "Error: Unknown build type specified ($2). It must be one of Debug, Release, RelWithDebInfo"
          return 1
      esac
      shift 2
    ;;
  --help|-h)
    usage full
    return 0
    ;;
  *)
    echo "Error: Unknown argument: $1"
    echo " "
    usage short
    return 1
    ;;
  esac
done

herc_path="$(dirname "$0")"
if [ ! -f "${herc_path}/CMakeLists.txt" ] || [ ! -f "${herc_path}/athena-start" ]; then
  echo "Hercules path not found. The script must be in the Hercules repository root"
  return 2
fi

# Conan detection and setup

echo "Detecting conan 2 installation..."

if type conan >/dev/null 2>&1 && [ "$(conan --version 2>/dev/null | sed 's/Conan version \([0-9]*\)\..*/\1/')" -ge 2 ]; then
  echo "Detected conan 2 in the current environment. Venv setup will not be necessary."
elif [ -x "${herc_path}/.venv/bin/conan" ] && [ "$("${herc_path}/.venv/bin/conan" --version 2>/dev/null | sed 's/Conan version \([0-9]*\)\..*/\1/')" -ge 2 ]; then
  echo "Detected conan 2 installed in venv. The venv will be activated."
  . "${herc_path}/.venv/bin/activate"
elif [ -f "${herc_path}/.venv/bin/activate" ] && [ -x "${herc_path}/.venv/bin/pip" ]; then
  echo "Detected local venv. Installing conan 2 in it."
  "${herc_path}/.venv/bin/pip" install conan
  . "${herc_path}/.venv/bin/activate"
elif [ -d "${herc_path}/.venv" ]; then
  echo "Detected invalid local venv in \"${herc_path}/.venv\". Setup cannot proceed."
  return 3
elif type python3 >/dev/null 2>&1 && python3 -m venv --help >/dev/null 2>&1; then
  echo "Setting up venv for conan 2"
  python3 -m venv "${herc_path}/.venv"
  "${herc_path}/.venv/bin/pip" install conan
  . "${herc_path}/.venv/bin/activate"
elif type python >/dev/null 2>&1 && python -m venv --help >/dev/null 2>&1; then
  echo "Setting up venv for conan 2"
  python -m venv "${herc_path}/.venv"
  "${herc_path}/.venv/bin/pip" install conan
  . "${herc_path}/.venv/bin/activate"
else
  echo "Python3 not found. Setup of conan cannot proceed."
  return 3
fi

if ! type conan >/dev/null 2>&1 || ! [ "$(conan --version 2>/dev/null | sed 's/Conan version \([0-9]*\)\..*/\1/')" -ge 2 ]; then
  echo "Conan 2 installation or venv setup failed."
  return 3
fi
echo "Using conan from $(command -v conan) (version: $(conan --version | sed 's/Conan version //'))"

# CMake detection and setup
if [ -f "${build_dir}/CMakeCache.txt" ]; then
  echo "Detected CMake environment already configured in ${build_dir}."
else
  if ! type cmake >/dev/null 2>&1; then
    echo "CMake installation not detected. Configuration cannot proceed."
    return 4
  fi
  generator_args=""
  if type ninja >/dev/null 2>&1; then
    echo "Setting up environment in ${build_dir} using CMake from $(command -v cmake) and ninja from $(command -v ninja)"
    generator_args="-G Ninja"
  else
    echo "Setting up environment in ${build_dir} using CMake from $(command -v cmake). Ninja not detected, the default CMake generator will be used instead. Hercules recommends the installation and use of ninja."
  fi
  if [ ! -d "${build_dir}" ]; then
    mkdir -p "${build_dir}"
  fi
  cmake -S "${herc_path}" -B "${build_dir}" $generator_args -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=cmake_modules/conan_provider.cmake -DCMAKE_BUILD_TYPE="${build_type}" -DENABLE_TESTING=ON
  if [ $? -gt 0 ] || [ ! -f "${build_dir}/CMakeCache.txt" ]; then
    echo "Failed to generate build environment in ${build_dir}"
    return 4
  fi
fi

echo " "
echo " "
echo "Build environment ready in ${build_dir}"
