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

if(DOXYGEN_FOUND)
  add_custom_target(docs
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND
      doxygen
      "doxygen.conf"
    BYPRODUCTS "${CMAKE_CURRENT_SOURCE_DIR}/docs"
  )

  set_property(
    TARGET docs
    APPEND
    PROPERTY
      ADDITIONAL_CLEAN_FILES
      "${CMAKE_CURRENT_SOURCE_DIR}/docs"
  )
else()
  add_custom_target(docs
    COMMAND
      echo
      "doxygen not found or disabled"
  )
endif()
