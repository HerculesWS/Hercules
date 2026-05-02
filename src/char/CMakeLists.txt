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

set(CHAR_SRCS
  char.c
  HPMchar.c
  loginif.c
  mapif.c
  geoip.c
  inter.c
  int_achievement.c
  int_adventurer_agency.c
  int_auction.c
  int_clan.c
  int_elemental.c
  int_guild.c
  int_homun.c
  int_mail.c
  int_mercenary.c
  int_party.c
  int_pet.c
  int_quest.c
  int_rodex.c
  int_storage.c
  int_userconfig.c
  pincode.c
  capiif.c
)

set(CHAR_HDRS
  char.h
  HPMchar.h
  apipackets.h
  loginif.h
  mapif.h
  geoip.h
  inter.h
  int_achievement.h
  int_adventurer_agency.h
  int_auction.h
  int_clan.h
  int_elemental.h
  int_guild.h
  int_homun.h
  int_mail.h
  int_mercenary.h
  int_party.h
  int_pet.h
  int_quest.h
  int_rodex.h
  int_storage.h
  int_userconfig.h
  pincode.h
  packets_hc_struct.h
  capiif.h
)

list(APPEND CHAR_HDRS ${CONFIG_HDRS})

set(CHAR_PRVT
)

source_group("Char source files"
  FILES
  ${CHAR_SRCS}
)

source_group("Char header files"
  FILES
  ${CHAR_HDRS}
)

source_group("Char private header files"
  FILES
  ${CHAR_PRVT}
)

add_executable(char-server
  ${CHAR_SRCS}
  ${CHAR_HDRS}
  ${CHAR_PRVT}
)

target_link_libraries(char-server PUBLIC common)

install(
  TARGETS char-server
  RUNTIME
    DESTINATION
    .
)
