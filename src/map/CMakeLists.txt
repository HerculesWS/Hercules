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

set(MAP_SRCS
  achievement.cpp
  atcommand.cpp
  battle.cpp
  battleground.cpp
  buyingstore.cpp
  channel.cpp
  chat.cpp
  chrif.cpp
  clan.cpp
  clif.cpp
  date.cpp
  duel.cpp
  elemental.cpp
  enchantui.cpp
  goldpc.cpp
  grader.cpp
  guild.cpp
  homunculus.cpp
  HPMmap.cpp
  instance.cpp
  intif.cpp
  irc-bot.cpp
  itemdb.cpp
  log.cpp
  macro.cpp
  mail.cpp
  map.cpp
  mapiif.cpp
  mapreg_sql.cpp
  mercenary.cpp
  mob.cpp
  npc.cpp
  npc_chat.cpp
  party.cpp
  path.cpp
  pc.cpp
  pc_groups.cpp
  pet.cpp
  quest.cpp
  refine.cpp
  rodex.cpp
  script.cpp
  searchstore.cpp
  skill.cpp
  status.cpp
  storage.cpp
  stylist.cpp
  trade.cpp
  unit.cpp
  vending.cpp
)

set(MAP_HDRS
  achievement.h
  apipackets.h
  atcommand.h
  battle.h
  battleground.h
  buyingstore.h
  channel.h
  chat.h
  chrif.h
  clan.h
  clif.h
  date.h
  duel.h
  elemental.h
  enchantui.h
  goldpc.h
  grader.h
  guild.h
  homunculus.h
  HPMmap.h
  instance.h
  intif.h
  irc-bot.h
  itemdb.h
  log.h
  macro.h
  mail.h
  map.h
  mapiif.h
  mapreg.h
  mercenary.h
  messages.h
  messages_ad.h
  messages_main.h
  messages_re.h
  messages_sak.h
  messages_zero.h
  mob.h
  npc.h
  packets.h
  packets_keys_main.h
  packets_keys_zero.h
  packets_shuffle_main.h
  packets_shuffle_re.h
  packets_shuffle_zero.h
  packets_struct.h
  party.h
  path.h
  pc.h
  pc_groups.h
  pet.h
  quest.h
  refine.h
  rodex.h
  script.h
  searchstore.h
  skill.h
  status.h
  storage.h
  stylist.h
  trade.h
  unit.h
  vending.h
)

set(MAP_PRVT
  refine.p.h
)

source_group("Map source files"
  FILES
  ${MAP_SRCS}
)

source_group("Map header files"
  FILES
  ${MAP_HDRS}
)

source_group("Map private header files"
  FILES
  ${MAP_PRVT}
)

add_executable(map-server
  ${MAP_SRCS}
  ${MAP_HDRS}
  ${MAP_PRVT}
)

target_link_libraries(map-server PUBLIC common)
target_link_libraries(map-server PRIVATE pcre::pcre)

install(
  TARGETS map-server
  RUNTIME
    DESTINATION
    .
)

add_dependencies(sql map-server)
