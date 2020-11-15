# Hercules Monster Modes Reference

<!--
## Copyright
> This file is part of Hercules.
> http://herc.ws - http://github.com/HerculesWS/Hercules
> 
> Copyright (C) 2012-2020 Hercules Dev Team
> Copyright (C) Athena Dev Teams
> 
> Hercules is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
> 
> This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
> See the GNU General Public License for more details.
>
> You should have received a copy of the GNU General Public License along with this program.  
> If not, see <http://www.gnu.org/licenses/>.
-->

## Description
A reference description of Hercules's mob_db.conf `mode` field.

## Monster Mode Legend:
Constant Name         | Bits    | Value    | Description
:-------------------- | :-----: | :------: | :----------------
MD_CANMOVE            | 0x00001 |        1 | Enables the mob to move/chase characters.
MD_LOOTER             | 0x00002 |        2 | The mob will loot up nearby items on the ground when it's on idle state.
MD_AGGRESSIVE         | 0x00004 |        4 | Normal aggressive mob, will look for a close-by player to attack.
MD_ASSIST             | 0x00008 |        8 | When a nearby mob of the same class attacks, assist types will join them.
MD_CASTSENSOR_IDLE    | 0x00010 |       16 | Will go after characters who start casting on them if idle or walking (without a target).
MD_BOSS               | 0x00020 |       32 | Special flag which makes mobs immune to certain status changes and skills.
MD_PLANT              | 0x00040 |       64 | Always receives 1 damage from attacks.
MD_CANATTACK          | 0x00080 |      128 | Enables the mob to attack/retaliate when you are within attack range. <br/>Note that this only enables them to use normal attacks, skills are always allowed.
MD_DETECTOR           | 0x00100 |      256 | Enables mob to detect and attack characters who are in hiding/cloak.
MD_CASTSENSOR_CHASE   | 0x00200 |      512 | Will go after characters who start casting on them if idle or chasing other players (they switch chase targets)
MD_CHANGECHASE        | 0x00400 |     1024 | Allows chasing mobs to switch targets if another player happens to be within attack range (handy on ranged attackers, for example)
MD_ANGRY              | 0x00800 |     2048 | These mobs are "hyper-active". Apart from "chase"/"attack", they have the states "follow"/"angry". </br>Once hit, they stop using these states and use the normal ones. The new states are used to determine a different skill-set for their "before attacked" and "after attacked" states. </br>Also, when "following", they automatically switch to whoever character is closest.
MD_CHANGETARGET_MELEE | 0x01000 |     4096 | Enables a mob to switch targets when attacked while attacking someone else.
MD_CHANGETARGET_CHASE | 0x02000 |     8192 | Enables a mob to switch targets when attacked while chasing another character.
MD_TARGETWEAK         | 0x04000 |    16384 | Allows aggressive monsters to only be aggressive against characters that are five levels below it's own level. </br>For example, a monster of level 104 will not pick fights with a level 99.
MD_NOKNOCKBACK        | 0x08000 |    32768 | Monsters will be immune to knockback's effect.
MD_RANDOMTARGET       | 0x10000 |    65536 | Picks a new random target in range on each attack/skill. (not implemented)

## Aegis Mob Types:
What Aegis has are mob-types, where each type represents an AI behavior that is mimicked by a group of eA mode bits. 
This is the table to convert from one to another:

No. | Bits   | Mob Type | Aegis/eA Description
--: | :----: | :------: | :----------------
 01 | 0x0081 | Any      | passive
 02 | 0x0083 | Any      | passive, looter
 03 | 0x1089 | Any      | passive, assist and change-target melee
 04 | 0x3885 | Any      | angry, change-target melee/chase
 05 | 0x2085 | Any      | aggressive, change-target chase
 06 | 0x0000 | Plants   | passive, immobile, can't attack
 07 | 0x108B | Any      | passive, looter, assist, change-target melee
 08 | 0x6085 | Any      | aggressive, change-target chase, target weak enemies
 09 | 0x3095 | Guardian | aggressive, change-target melee/chase, cast sensor idle
 10 | 0x0084 | Any      | aggressive, immobile
 11 | 0x0084 | Guardian | aggressive, immobile
 12 | 0x2085 | Guardian | aggressive, change-target chase
 13 | 0x308D | Any      | aggressive, change-target melee/chase, assist
 17 | 0x0091 | Any      | passive, cast sensor idle
 19 | 0x3095 | Any      | aggressive, change-target melee/chase, cast sensor idle
 20 | 0x3295 | Any      | aggressive, change-target melee/chase, cast sensor idle/chase
 21 | 0x3695 | Any      | aggressive, change-target melee/chase, cast sensor idle/chase, chase-change target
 25 | 0x0001 | Pet      | passive, can't attack
 26 | 0xB695 | Any      | aggressive, change-target melee/chase, cast sensor idle/chase, chase-change target, random target
 27 | 0x8084 | Any      | aggressive, immobile, random target

- Note that the detector bit due to being Insect/Demon, Plant and Boss mode bits need to be added independently of this list.

