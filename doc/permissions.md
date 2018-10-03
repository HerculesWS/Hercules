# Group Permission List
A list of player group permission, configured in `conf/groups.conf`.

## Copyright
> This file is part of Hercules.
> http://herc.ws - http://github.com/HerculesWS/Hercules
> 
> Copyright (C) 2012-2018 Hercules Dev Team
> Copyright (C) Athena Dev Teams
> 
> Hercules is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
> 
> This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
> See the GNU General Public License for more details.
>
> You should have received a copy of the GNU General Public License along with this program.  
> If not, see <http://www.gnu.org/licenses/>.

## Description
The Hercules emulator has a permission system that enables certain groups of players to perform certain actions, or have access to certain visual enhancements or in-game activity.

Permission                 | Description
:------------------------  | :---------------------------------------------
can_trade                  | Ability to trade or otherwise distribute items (drop, storage, vending etc).
can_party                  | Ability to join parties.
all_skill                  | Ability to use all skills.
all_equipment              | Ability to equip anything (can cause client errors).
skill_unconditional        | Ability to use skills without meeting the required conditions (SP, items, etc).
join_chat                  | Ability to join a password protected chatrooms.
kick_chat                  | Protection from being kicked from a chat.
hide_session               | Hides player session from being displayed by `@commands`.
who_display_aid            | Ability to see GMs and Account/Char IDs in the `@who` command.
hack_info                  | Ability to receive all informations about any player that try to hack, spoof a name, etc.
any_warp                   | Ability to bypass nowarp, nowarpto, noteleport and nomemo mapflags. This option is mainly used in commands which modify a character's map/coordinates (like `@memo`, `@mapmove`, `@go`, `@jump`, etc).
view_hpmeter               | Ability to see HP bar of every player.
view_equipment             | Ability to view players equipment regardless of their setting.
use_check                  | Ability to use client command `/check` (display character status).
use_changemaptype          | Ability to use client command `/changemaptype`.
all_commands               | Ability to use all atcommands and charcommands.
receive_requests           | Ability to receive `@requests`.
show_bossmobs              | Ability to see boss mobs with `@showmobs`.
disable_pvm                | Ability to disable Player vs. Monster (PvM).
disable_pvp                | Ability to disable Player vs. Player (PvP).
disable_commands_when_dead | Ability to disable atcommands usage when dead.
can_trade_bound            | Ability to trade or otherwise distribute bound items (drop, storage, vending etc).
hchsys_admin               | Hercules Chat System Admin (Ability to modify channel settings regardless of ownership and join password-protected channels without requiring a password.)
disable_pickup             | Ability to disable the player from picking up any item from ground, they can still receive items picked up by others means like party share píck.
disable_exp                | Ability to disable the player from gaining any experience point.
disable_store              | Ability to disable the player from using/openning npc and player stores.
disable_skill_usage        | Ability to disable the player from using any skill.

