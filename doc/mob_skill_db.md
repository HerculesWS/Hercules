# Monster skill database

<!--
## Copyright
> This file is part of Hercules.
> http://herc.ws - http://github.com/HerculesWS/Hercules
> 
> Copyright (C) 2020-2021 Hercules Dev Team
> Copyright (C) Zarbony
> Copyright (C) Kenpachi
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
This file is a documentation for the monster skill database files.
 * [db/mob_skill_db2.conf](../db/mob_skill_db2.conf)
 * [db/pre-re/mob_skill_db.conf](../db/pre-re/mob_skill_db.conf)
 * [db/re/mob_skill_db.conf](../db/re/mob_skill_db.conf)

--------------------------------------------------------------

## Entry structure
```
	<Monster_Constant>: {
		<Skill_Constant>: {
			ClearSkills:
			SkillLevel:
			SkillState:
			SkillTarget:
			Rate:
			CastTime:
			Delay:
			Cancelable:
			CastCondition:
			ConditionData:
			val0:
			val1:
			val2:
			val3:
			val4:
			Emotion:
			ChatMsgID:
		}
	}
```

--------------------------------------------------------------

## Field list

Name             | Data type | Default value
:--------------- | :-------- | :------------
Monster_Constant | constant  | No default value
Skill_Constant   | constant  | No default value
ClearSkills      | boolean   | false
SkillLevel       | int       | 1
SkillState       | string    | "MSS_ANY"
SkillTarget      | string    | "MST_TARGET"
Rate             | int       | 1
CastTime         | int       | 0
Delay            | int       | 0
Cancelable       | boolean   | false
CastCondition    | string    | "MSC_ALWAYS"
ConditionData    | int       | 0
val0             | int       | 0
val1             | int       | 0
val2             | int       | 0
val3             | int       | 0
val4             | int       | 0
Emotion          | int       | -1
ChatMsgID        | int       | 0

--------------------------------------------------------------

## Field explanation

### Monster_Constant
The monster's name constant, found in [mob_db.conf](../db/re/mob_db.conf) as *SpriteName*.  
There are 3 special constants for global skill assignment:  
* `ALL_MOBS`: Add skills to all monsters.
* `ALL_MOBS_BOSS`: Add skills to all boss monsters.
* `ALL_MOBS_NONBOSS`: Add skills to all non-boss monsters.

### Skill_Constant
The skill's name constant, found in [skill_db.conf](../db/re/skill_db.conf) as *Name*.  
Note: You can add multiple Skill_Constant blocks.

### ClearSkills
If set to `true`, all previously defined skills for this monster will be removed.

### SkillLevel
The skill level which should be used.  
Minimum value is `1`. Maximum value is `mob_max_skilllvl` from [conf/map/battle/skill.conf](../conf/map/battle/skill.conf#L318).

### SkillState
Defines in which state the monster is able to cast the skill.
State         | Description
:------------ | :----------
MSS_ANY       | Monster is in any state except `MSS_DEAD`.
MSS_IDLE      | Monster has no target and isn't walking.
MSS_WALK      | Monster is walking.
MSS_LOOT      | Monster is looting or walking to loot.
MSS_DEAD      | Monster is dying.
MSS_BERSERK   | Monster is attacking after starting the battle.
MSS_ANGRY     | Monster is attacking after being attacked.
MSS_RUSH      | Monster is following an enemy after being attacked.
MSS_FOLLOW    | Monster is following an enemy without being attacked.
MSS_ANYTARGET | Same as `MSS_ANY` but monster must have a target.

### SkillTarget
Defines the skill's target.
Target      | Description
:---------- | :----------
MST_TARGET  | The monster's current target.
MST_RANDOM  | A random enemy within skill range.
MST_SELF    | The monster itself.
MST_FRIEND  | A random friend within skill range. If no friend was found, `MST_SELF` is used.
MST_MASTER  | The monster's master. If no master was found, `MST_FRIEND` is used.
MST_AROUND1 | Random cell within a range of `1`. (Affects ground skills only.)
MST_AROUND2 | Random cell within a range of `2`. (Affects ground skills only.)
MST_AROUND3 | Random cell within a range of `3`. (Affects ground skills only.)
MST_AROUND4 | Random cell within a range of `4`. (Affects ground skills only.)
MST_AROUND5 | Same as `MST_AROUND1`, but the monster's current target must be in skill range.
MST_AROUND6 | Same as `MST_AROUND2`, but the monster's current target must be in skill range.
MST_AROUND7 | Same as `MST_AROUND3`, but the monster's current target must be in skill range.
MST_AROUND8 | Same as `MST_AROUND4`, but the monster's current target must be in skill range.
MST_AROUND  | Same as `MST_AROUND4`.

### Rate
The chance of successfully casting the skill if the condition is fulfilled. (10000 = 100%)  
Minimum value is `1`. Maximum value is `10000`.

### CastTime
The skill's cast time in milliseconds.  
Minimum value is `0`. Maximum value is `MOB_MAX_CASTTIME` from [src/map/mob.c](../src/map/mob.c#L81).

### Delay
The time in milliseconds before attempting to cast the same skill again.  
Minimum value is `0`. Maximum value is `MOB_MAX_DELAY` from [src/map/mob.c](../src/map/mob.c#L82).

### Cancelable
Defines whether the skill is cancelable or not.

### CastCondition
Defines the condition to successfully cast the skill.
Condition             | Description
:-------------------- | :----------
MSC_ALWAYS            | No condition.
MSC_MYHPLTMAXRATE     | Monster's HP in percent is less than or equal to `ConditionData`.
MSC_MYHPINRATE        | Monster's HP in percent is greater than or equal to `ConditionData` and less than or equal to `val0`.
MSC_FRIENDHPLTMAXRATE | Friend's HP in percent is less than or equal to `ConditionData`.
MSC_FRIENDHPINRATE    | Friend's HP in percent is greater than or equal to `ConditionData` and less than or equal to `val0`.
MSC_MYSTATUSON        | Monster has status change `ConditionData` enabled. (See [doc/constants.md](./constants.md#Status-Changes) for a list of available status changes.)
MSC_MYSTATUSOFF       | Monster has status change `ConditionData` disabled. (See [doc/constants.md](./constants.md#Status-Changes) for a list of available status changes.)
MSC_FRIENDSTATUSON    | Friend has status change `ConditionData` enabled. (See [doc/constants.md](./constants.md#Status-Changes) for a list of available status changes.)
MSC_FRIENDSTATUSOFF   | Friend has status change `ConditionData` disabled. (See [doc/constants.md](./constants.md#Status-Changes) for a list of available status changes.)
MSC_ATTACKPCGT        | Monster is attacked by more than `ConditionData` units.
MSC_ATTACKPCGE        | Monster is attacked by `ConditionData` or more units.
MSC_SLAVELT           | Monster has less than `ConditionData` slaves.
MSC_SLAVELE           | Monster has `ConditionData` or less active slaves.
MSC_CLOSEDATTACKED    | Monster is melee attacked.
MSC_LONGRANGEATTACKED | Monster is range attacked.
MSC_AFTERSKILL        | Monster has used skill `ConditionData`. (If `ConditionData` is `0`, all skills are triggered.)
MSC_SKILLUSED         | Skill `ConditionData` was used on the monster. (If `ConditionData` is `0`, all skills are triggered.)
MSC_CASTTARGETED      | A skill is being cast on the monster.
MSC_RUDEATTACKED      | Monster was rude attacked `RUDE_ATTACKED_COUNT` times. ([src/map/mob.c#L84](../src/map/mob.c))
MSC_MASTERHPLTMAXRATE | The monster master's HP in percent is less than `ConditionData`.
MSC_MASTERATTACKED    | The monster's master is attacked.
MSC_ALCHEMIST         | The monster was summoned by an Alchemist class character.
MSC_SPAWN             | The monster spawns.
MSC_MAGICATTACKED     | The monster has received magic damage.

### ConditionData
Additional cast condition data. Meaning depends on the situation. See `CastCondition` table.

### val0
Additional data. Meaning depends on the situation.
 * `MSC_MYHPINRATE`/`MSC_FRIENDHPINRATE`: See `CastCondition` table.
 * `NPC_SUMMONMONSTER`: Slave monster ID.
 * `NPC_SUMMONSLAVE`: Slave monster ID.
 * `NPC_METAMORPHOSIS`: Transform monster ID.
 * `NPC_EMOTION`: Emotion ID. (See [doc/constants.md](./constants.md#emotes) for a list of available emotions.)
 * `NPC_EMOTION_ON`: Emotion ID. (See [doc/constants.md](./constants.md#emotes) for a list of available emotions.)

### val1
Additional data. Meaning depends on the situation.
 * `NPC_SUMMONMONSTER`: Slave monster ID.
 * `NPC_SUMMONSLAVE`: Slave monster ID.
 * `NPC_METAMORPHOSIS`: Transform monster ID.
 * `NPC_EMOTION`: Monster's mode is changed to specified value.
 * `NPC_EMOTION_ON`: Monster's mode is changed to specified value.

### val2
Additional data. Meaning depends on the situation.
 * `NPC_SUMMONMONSTER`: Slave monster ID.
 * `NPC_SUMMONSLAVE`: Slave monster ID.
 * `NPC_METAMORPHOSIS`: Transform monster ID.

### val3
Additional data. Meaning depends on the situation.
 * `NPC_SUMMONMONSTER`: Slave monster ID.
 * `NPC_SUMMONSLAVE`: Slave monster ID.
 * `NPC_METAMORPHOSIS`: Transform monster ID.

### val4
Additional data. Meaning depends on the situation.
 * `NPC_SUMMONMONSTER`: Slave monster ID.
 * `NPC_SUMMONSLAVE`: Slave monster ID.
 * `NPC_METAMORPHOSIS`: Transform monster ID.

### Emotion
The ID of the emotion the monster will use when casting the skill.  
(See [doc/constants.md](./constants.md#emotes) for a list of available emotions.)

### ChatMsgID
The ID of the message the monster will say when casting the skill.
(See [db/mob_chat_db.txt](../db/mob_chat_db.txt) for a list of available messages.)
