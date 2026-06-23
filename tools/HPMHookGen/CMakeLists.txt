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

set(API_SERVER_PATH "${CMAKE_SOURCE_DIR}/src/api")

if(DOXYGEN_FOUND AND PERL_FOUND AND PYTHON3_FOUND)
  add_custom_target(hooks
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND
      echo
      "Preprocessing files"
    COMMAND
      python3
      ./preprocess.py
      "${API_SERVER_PATH}/handlers.h"
      "api/urlhandlers.h"
      ">"
      "${API_SERVER_PATH}/handlers.h.dox"
    COMMAND
      echo
      "Extracting functions information..."
    COMMAND
      doxygen
      "doxygen.conf"
    COMMAND
      echo
      "Removing temp preprocessed files"
    COMMAND
      rm
      "${API_SERVER_PATH}/handlers.h.dox"
    COMMAND
      echo
      "Regenerating HPM Hook definitions..."
    COMMAND
      perl
      "HPMHookGen.pl"
    COMMAND
      echo
      "Regenerating HPM Data Check definitions..."
    COMMAND
      perl
      "HPMDataCheckGen.pl"
    BYPRODUCTS
      "${CMAKE_CURRENT_SOURCE_DIR}/doxyoutput"
      "${API_SERVER_PATH}/handlers.h.dox"
  )

  set_property(
    TARGET hooks
    APPEND
    PROPERTY
      ADDITIONAL_CLEAN_FILES
      "${CMAKE_CURRENT_SOURCE_DIR}/doxyoutput"
  )
else()
  add_custom_target(hooks
    COMMAND
      echo
      "doxygen, perl and python3 are not found or disabled"
  )
endif()
