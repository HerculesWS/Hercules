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
  achievement.c
  atcommand.c
  battle.c
  battleground.c
  buyingstore.c
  channel.c
  chat.c
  chrif.c
  clan.c
  clif.c
  date.c
  duel.c
  elemental.c
  enchantui.c
  goldpc.c
  grader.c
  guild.c
  homunculus.c
  HPMmap.c
  instance.c
  intif.c
  irc-bot.c
  itemdb.c
  log.c
  macro.c
  mail.c
  map.c
  mapiif.c
  mapreg_sql.c
  mercenary.c
  mob.c
  npc.c
  npc_chat.c
  party.c
  path.c
  pc.c
  pc_groups.c
  pet.c
  quest.c
  refine.c
  rodex.c
  script.c
  searchstore.c
  skill.c
  status.c
  storage.c
  stylist.c
  trade.c
  unit.c
  vending.c
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

list(APPEND MAP_HDRS ${CONFIG_HDRS})

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
