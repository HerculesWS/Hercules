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

set(HERC_LIBCONF_SRCS
  grammar.c
  libconfig.c
  scanctx.c
  scanner.c
  strbuf.c
)

set(HERC_LIBCONF_HDRS
  grammar.h
  libconfig.h
  parsectx.h
  scanctx.h
  scanner.h
  strbuf.h
  wincompat.h
)

source_group("libconfig source files"
  FILES
  ${HERC_LIBCONF_SRCS}
)

source_group("libconfig header files"
  FILES
  ${HERC_LIBCONF_HDRS}
)

add_library(hercules-libconfig STATIC
  ${HERC_LIBCONF_SRCS}
  ${HERC_LIBCONF_HDRS}
)
