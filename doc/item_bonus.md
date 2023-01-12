# Hercules Item Bonuses List

<!--
# Copyright
> This file is part of Hercules.
> http://herc.ws - http://github.com/HerculesWS/Hercules
> 
> Copyright (C) 2012-2023 Hercules Dev Team
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
List of script instructions used in item bonuses, mainly `bonus`/`bonus2`/`bonus3`/`bonus4`/`bonus5` arguments and etc.


## Constants
This table contains all available constants referenced in the `bonus` commands.

Race (`r`)         | Status effect (`eff`) | Element (`e`)  | Monster Race (`mr`) | Size (`s`) 
:----------------- | :-------------------- | :------------- | :------------------ | :-----------
RC_Formless        | Eff_Stone             | Ele_Neutral    | RC2_Goblin          | Size_Small
RC_Undead          | Eff_Freeze            | Ele_Water      | RC2_Kobold          | Size_Medium
RC_Brute           | Eff_Stun              | Ele_Earth      | RC2_Orc             | Size_Large
RC_Plant           | Eff_Sleep             | Ele_Fire       | RC2_Golem           |
RC_Insect          | Eff_Poison            | Ele_Wind       | RC2_Guardian        |
RC_Fish            | Eff_Curse             | Ele_Poison     | RC2_Ninja           |
RC_Demon           | Eff_Silence           | Ele_Holy       | RC2_Scaraba         |
RC_DemiHuman       | Eff_Confusion         | Ele_Dark       | RC2_Turtle          |
RC_Angel           | Eff_Blind             | Ele_Ghost      |                     |
RC_Dragon          | Eff_Bleeding          | Ele_Undead     |                     |
RC_Player          | Eff_DPoison           | Ele_All        |                     |
RC_Boss            | Eff_Fear              |                |                     |
RC_NonBoss         | Eff_Cold              |                |                     |
RC_NonDemiHuman    | Eff_Burning           |                |                     |
RC_NonPlayer       | Eff_Deepsleep         |                |                     |
RC_DemiPlayer      |                       |                |                     |
RC_NonDemiPlayer   |                       |                |                     |
RC_All             |                       |                |                     |

      
### Trigger criteria (`bf`)
  Type 1           | Description
:----------------- | :------------------------------
`BF_WEAPON`        | Trigger on weapon skills 
`BF_MAGIC`         | Trigger on magic skills 
`BF_MISC`          | Trigger on misc skills

(Default: `BF_WEAPON`)

  Type 2           | Description
:----------------- | :------------------------------
`BF_SHORT`         | Trigger on melee attacks
`BF_LONG`          | Trigger on ranged attacks

(Default: `BF_SHORT`+`BF_LONG`)

  Type 3           | Description
:----------------- | :------------------------------
`BF_NORMAL`        | Trigger on normal attacks
`BF_SKILL`         | Trigger on skills

(Default: `BF_SKILL` if type is `BF_MISC` or `BF_MAGIC`, `BF_NORMAL` if type is `BF_WEAPON`)
 
 
### Attack Trigger Criteria (`abf`)
  Type 1           | Description
:----------------- | :------------------------------
`ATF_SELF`         | Trigger on self
`ATF_TARGET`       | Trigger on target

(Default: `ATF_TARGET`)

  Type 2           | Description
:----------------- | :------------------------------
`ATF_SHORT`        | Trigger on melee attack
`ATF_LONG`         | Trigger on ranged attack

(Default: `ATF_SHORT`+`ATF_LONG`)
 
  Type 3           | Description
:----------------- | :------------------------------
`ATF_WEAPON`       | Trigger on Weapon Skills
`ATF_MAGIC`        | Trigger on magic attacks
`ATF_MISC`         | Trigger on misc skills
`ATF_SKILL`        | Trigger on skill attack

(Default: `ATF_WEAPON`)
 
  Type 4           | Description
:----------------- | :------------------------------
`ATF_SELF`         | Trigger effect on self.
`ATF_TARGET`       | Trigger effect on target (default)
`ATF_SHORT`        | Trigger on melee attacks
`ATF_LONG`         | Trigger in ranged attacks (default: trigger on all attacks)

(Default: `ATF_TARGET`)
 
  Other values     | Remarks
:----------------- | :------------------------------
Skill (`sk`)       | see `db/(pre-)re/skill_db.txt` (NOTE: Both skill IDs and names, with and without quotes, are supported.)
Monster id (`mid`) | see `db/(pre-)re/mob_db.txt`
Item id (`id`)     | see `db/(pre-)re/item_db.conf`
Item chain (`ic`)  | see `db/(pre-)re/item_chain.conf` (Only Constants)
Item group (`ig`)  | see `db/(pre-)re/item_group.conf` (ItemID)
Weapon type (`w`)  | see `doc/item_db.txt` -> View -> Weapons
Class (`c`)        | see `db/(pre-re)/mob_db.txt` -> For Players, `c` = JobID

---------------


## Bonuses
---------------
The format of bonuses listed in this file is as follows:
 * 1. Basic Bonuses
 * 2. Extended Bonuses
 * 3. Group-specific Bonuses
 * 4. Status-related Bonuses
 * 5. AutoSpell Bonuses
 * 6. Misc Bonuses


---------------

### 1. Basic Bonuses

Base Stats                        | Description
:-------------------------------- | :-------------------------
bonus bStr,`n`;                   | STR + `n`
bonus bAgi,`n`;                   | AGI + `n`
bonus bVit,`n`;                   | VIT + `n`
bonus bInt,`n`;                   | INT + `n`
bonus bDex,`n`;                   | DEX + `n`
bonus bLuk,`n`;                   | LUK + `n`
bonus bAgiVit,`n`;                | AGI + `n`, VIT + `n`
bonus bAgiDexStr,`n`;             | STR + `n`, AGI + `n`, DEX + `n`
bonus bAllStats,`n`;              | STR + `n`, AGI + `n`, VIT + `n`, INT + `n`, DEX + `n`, LUK + `n`

HP/SP                             | Description
:-------------------------------- | :-------------------------
bonus bMaxHP,`n`;                 | MaxHP + `n`
bonus bMaxHPrate,`n`;             | MaxHP + `n`%
bonus bMaxSP,`n`;                 | MaxSP + `n`
bonus bMaxSPrate,`n`;             | MaxSP + `n`%

Attack/Def                        | Description
:-------------------------------- | :-------------------------
bonus bAtk,`n`;                   | ATK + `n`
bonus bAtk2,`n`;                  | ATK2 + `n`
bonus bAtkRate,`n`;               | Attack Power + `n`%
bonus bBaseAtk,`n`;               | Basic Attack Power + `n`
bonus bDef,`n`;                   | Equipment DEF + `n`
bonus bDef2,`n`;                  | VIT based DEF + `n`
bonus bDefRate,`n`;               | Equipment DEF + `n`%
bonus bDef2Rate,`n`;              | VIT based DEF + `n`%

Magic Attack/Def                  | Description
:-------------------------------- | :-------------------------
bonus bMatk,`n`;                  | Magical attack power + `n`
bonus bMatkRate,`n`;              | Magical attack power + `n`%
bonus bMdef,`n`;                  | Equipment MDEF + `n`
bonus bMdef2,`n`;                 | INT based MDEF + `n`
bonus bMdefRate,`n`;              | Equipment MDEF + `n`%
bonus bMdef2Rate,`n`;             | INT based MDEF + `n`%

Other Stats                       | Description
:-------------------------------- | :-------------------------
bonus bHit,`n`;                   | Hit + `n`
bonus bHitRate,`n`;               | Hit + `n`%
bonus bCritical,`n`;              | Critical + `n`
bonus bCriticalRate,`n`;          | Critical + `n`%
bonus bFlee,`n`;                  | Flee + `n`
bonus bFleeRate,`n`;              | Flee + `n`%
bonus bFlee2,`n`;                 | Perfect Dodge + `n`
bonus bFlee2Rate,`n`;             | Perfect Dodge + `n`%
bonus bPerfectHitRate,`n`;        | On-target impact attack probability `n`% (only the highest among all is applied)
bonus bPerfectHitAddRate,`n`;     | On-target impact attack probability + `n`%
bonus bSpeedRate,`n`;             | Moving speed + `n`% (only the highest among all is applied)
bonus bSpeedAddRate,`n`;          | Moving speed + `n`%
bonus bAspd,`n`;                  | Attack speed + `n`
bonus bAspdRate,`n`;              | Attack speed + `n`%
bonus bAtkRange,`n`;              | Attack range + `n`
bonus bAddMaxWeight,`n`;          | MaxWeight + `n` (in units of 0.1)


---------------

### 2. Extended Bonuses

HP                                | Description
:-------------------------------- | :-------------------------
bonus bHPrecovRate,`n`;           | Natural HP recovery ratio + `n`%
bonus2 bHPRegenRate,`n`,`t`;      | Gain `n` HP every `t` milliseconds
bonus2 bHPLossRate,`n`,`t`;       | Lose `n` HP every `t` millisecond

SP                                | Description
:-------------------------------- | :-------------------------
bonus bSPrecovRate,`n`;           | Natural SP recovery ratio + `n`%
bonus2 bSPRegenRate,`n`,`t`;      | Gain `n` SP every `t` milliseconds
bonus2 bSPLossRate,`n`,`t`;       | Lose `n` SP every `t` milliseconds
bonus bUseSPrate,`n`;             | SP consumption + `n`%
bonus2 bSkillUseSP,`sk`,`n`;      | Reduces SP consumption of skill `sk` by n.
bonus2 bSkillUseSPrate,`sk`,`n`;  | Reduces SP consumption of skill `sk` by `n`%
bonus bNoRegen,`x`;               | Stops regeneration for `x` (`x`: 1=HP, 2=SP)

Attack/Def                        | Description
:-------------------------------- | :-------------------------
bonus bNearAtkDef,`n`;            | Adds `n`% damage reduction against melee physical attacks
bonus bLongAtkDef,`n`;            | Adds `n`% damage reduction against ranged physical attacks
bonus bMagicAtkDef,`n`;           | Adds `n`% damage reduction against magical attacks
bonus bMiscAtkDef,`n`;            | Adds `n`% damage reduction against MISC attacks (traps, falcon, ...)
bonus bCriticalDef,`n`;           | Decreases Chance of being hit by critical by `n`%
bonus2 bSkillAtk,`sk`,`n`;        | Increase damage of skill `sk` by `n`%
bonus2 bWeaponAtk,`w`,`n`;        | Adds `n` ATK when weapon of type `w` is equipped
bonus2 bWeaponAtkRate,`w`,`n`;    | Adds `n`% damage to weapon attacks when weapon of type `w` is equipped
bonus bLongAtkRate,`n`;           | Increases damage of ranged attacks by `n`%
bonus bCritAtkRate,`n`;           | Increase critical damage by +`n`%
bonus bNoWeaponDamage,`n`;        | Prevents from receiving `n`% physical damage
bonus bNoMagicDamage,`n`;         | Prevents from receiving `n`% magical effect (Attack, Healing, Support spells are all blocked)
bonus bNoMiscDamage,`n`;          | Adds `n`% reduction to received misc damage
bonus2 bSubSkill,`sk`,`n`;        | Reduce damage received from `sk` skill by `n`%

Heal                              | Description
:-------------------------------- | :-------------------------
bonus bHealPower,`n`;             | Increase heal amount of all heal skills used by player on self by `n`%
bonus bHealPower2,`n`;            | Increase heal amount if you are healed by any skills of others by `n`%
bonus2 bSkillHeal,`sk`,`n`;       | Increase heal amount of skill `sk` by `n`%
bonus2 bSkillHeal2,`sk`,`n`;      | Increase heal amount if you are healed by skill `sk` by `n`%
bonus bAddItemHealRate,`n`;       | Increases HP recovered by `n`% for healing items.
bonus2 bAddItemHealRate,`id`,`n`; | Increases HP recovered by `n`% for item `id`/`ig`

Skill Cast                          | Description
:---------------------------------- | :-------------------------
bonus bCastrate,`n`;                | Skill casting time rate + `n`%
bonus2 bCastrate,`sk`,`n`;          | Adjust casting time of skill `sk` by `n`%
bonus bFixedCastrate,`n`;           | Increases fixed cast time of all skills by `n`%
bonus2 bFixedCastrate,`s`,`n`;      | Increases fixed cast time of skill `sk` by `n`%
bonus bFixedCast,`t`;               | Increases fixed cast time of all skills by `t` milliseconds
bonus2 bSkillFixedCast,`sk`,`t`;    | Increases fixed cast time of skill `sk` by `t` milliseconds
bonus bVariableCastrate,`n`;        | Increases variable cast time of all skills by `n`%
bonus2 bVariableCastrate,`sk`,`n`;  | Increases variable cast time of skill `sk` by `n`%
bonus bVariableCast,`t`;            | Increases variable cast time of all skills by `t` milliseconds
bonus2 bSkillVariableCast,`sk`,`t`; | Increases variable cast time of skill `sk` by `t` milliseconds
bonus bNoCastCancel,`n`;            | Prevents casting from being interrupted when hit (does not work in GvG | `n` is meaningless)
bonus bNoCastCancel2,`n`;           | Prevents casting from being interrupted when hit (works even in GvG | `n` is meaningless)
bonus bDelayrate,`n`;               | Increases skill delay by `n`%
bonus2 bSkillCooldown,`sk`,`t`;     | Increases cooldown of skill `sk` by `t` milliseconds


---------------

### 3. Group-specific Bonuses

Damage Modifiers                     | Description
:----------------------------------- | :-------------------------
bonus2 bAddSize,`s`,`n`;             | +n% Physical damage against size `s`
bonus2 bMagicAddSize,`s`,`n`;        | +n% Magical damage against size `s`
bonus2 bSubSize,`s`,`n`;             | +n% Damage reduction against size `s`
bonus2 bAddRaceTolerance,`r`,`n`;    | +n% tolerance against race `r` (Renewal Only)
bonus2 bAddRace,`r`,`n`;             | +n% Physical damage against race `r`
bonus2 bMagicAddRace,`n`,`x`;        | +n% Magical damage against race `r`
bonus2 bSubRace,`r`,`n`;             | +n% Damage reduction against race `r`
bonus2 bAddRace2,`mr`,`n`;           | +n% Damage Against monster race `mr`
bonus2 bSubRace2,`mr`,`n`;           | +n% Damage reduction against monster race `mr`
bonus2 bAddEle,`e`,`n`;              | +n% Physical damage against element `e`
bonus2 bMagicAddEle,`e`,`n`;         | +n% Magical damage against element `e`
bonus2 bMagicAtkEle,`e`,`n`;         | Increases damage of element `e` magic by `n`%
bonus3 bAddEle,`e`,`n`,`bf`;         | +n% physical damage against element `e`
bonus2 bSubEle,`e`,`n`;              | +n% Damage reduction against element `e`
bonus3 bSubEle,`e`,`n`,`bf`;         | +n% Damage reduction against element `e`.
bonus3 bSubDefEle,`e`,`n`,`i`;       | +n% Physical damage reduction against defense element `e`.<br/> i: <br/> Flags (bitfield)<br/> &1: Reduce damage from monsters.<br/> &2: Reduce damage from players.
bonus3 bMagicSubDefEle,`e`,`n`,`i`;  | +n% Magical damage reduction against defense element `e`.<br/> i: <br/> Flags (bitfield)<br/> &1: Reduce damage from monsters.<br/> &2: Reduce damage from players.
bonus2 bAddDamageClass,`c`,`x`;      | +n% extra physical damage against monsters of class `c`
bonus2 bAddMagicDamageClass,`c`,`x`; | +n% extra magical damage against monsters of class `c`
bonus2 bAddDefClass,`c`,`x`;         | +n% physical damage reduction against monsters of class `c`
bonus2 bAddMdefClass,`c`,`x`;        | +n% magical damage reduction against monsters of class `c`
bonus2 bCriticalAddRace,`r`,`n`;     | +`n` Critical Against race `r`

Attack/Def                             | Description
:------------------------------------- | :-------------------------
bonus bAtkEle,`e`;                     | Gives the player's attacks element `e`
bonus bDefEle,`e`;                     | Gives the player's defense element `e`
bonus bDefRatioAtkEle,`e`;             | Deals more damage to enemies of element `e` with higher defense
bonus bDefRatioAtkRace,`r`;            | Deals more damage to enemies of race `r` with higher defense
bonus4 bSetDefRace,`r`,`n`,`t`,`y`;    | Set DEF to `y` of an enemy of race `r` at `n`/100% for `t` milliseconds with normal attack
bonus4 bSetMDefRace,`r`,`n`,`t`,`y`;   | Set MDEF to `y` of an enemy of race `r` at `n`/100% for `t` milliseconds with normal attack

Ignore Def                        | Description
:-------------------------------- | :-------------------------
bonus bIgnoreDefRace,`r`;         | Disregard DEF against enemies of race `r`
bonus bIgnoreMdefRace,`r`;        | Disregard MDEF against enemies of race `r`
bonus bIgnoreDefEle,`e`;          | Disregard DEF against enemies of element `e`
bonus bIgnoreMdefEle,`e`;         | Disregard MDEF against enemies of element `e`
bonus2 bIgnoreDefRate,`r`,`n`;    | Disregard `n`% of the target's DEF if the target belongs to race `r`
bonus2 bIgnoreMdefRate,`r`,`n`;   | Disregard `n`% of the target's MDEF if the target belongs to race `r`
bonus bIgnoreMdefRate,`n`;        | Disregard `n`% of the target's MDEF

Experience                        | Description
:-------------------------------- | :-------------------------
bonus2 bExpAddRace,`r`,`n`;       | +n% Experience from enemies of race `r`


---------------

### 4. Status-related Bonuses

Status-related Bonuses                       | Description
:------------------------------------------- | :-------------------------
bonus2 bResEff,`e`,`n`;                      | Adds a `n`/100% tolerance to effect `e`
bonus2 bAddEff,`eff`,`n`;                    | Adds a `n`/100% chance to cause effect `eff` to the target when attacking
bonus2 bAddEff2,`eff`,`n`;                   | Adds a `n`/100% chance to cause effect `eff` on self when attacking.
bonus3 bAddEff,`eff`,`n`,`abf`;              | Adds a `n`/100% chance to cause effect `eff` to the target when attacking for target abf
bonus4 bAddEff,`eff`,`n`,`abf`,`t`;          | Adds a `n`/100% chance to cause effect `eff` to the target when attacking for target `abf` for `t` milliseconds <br/> (Note:The effect can't be avoided nor its duration reduced. Duration: 0-65535)
bonus3 bAddEffOnSkill,`sk`,`eff`,`n`;        | Adds a `n`/100% chance to cause effect `eff` on enemy when using skill `sk`
bonus4 bAddEffOnSkill,`sk`,`eff`,`n`,`abf`;  | Adds a `n`/100% chance to cause effect `eff` when using skill `sk`
bonus2 bAddEffWhenHit,`eff`,`n`;             | `n`/100% chance to cause effect `eff` to the enemy when being hit by physical damage
bonus3 bAddEffWhenHit,`eff`,`n`,`abf`;       | Adds a `n`/100% chance to cause effect `eff` to the enemy when being hit by physical damage
bonus2 bWeaponComaRace,`r`,`n`;              | Adds a `n`/100% chance to cause Coma when attacking a monster of race `r` with a weapon attack
bonus2 bWeaponComaEle,`e`,`n`;               | Adds a `n`/100% chance to cause Coma when attacking a monster of element `e` with weapon attack


---------------

### 5. AutoSpell Bonuses

NOTES:
 - For all AutoSpell bonuses, target must be within the spell's range to go off.
 - By default, AutoSpell skills are casted on target unless it is a self or support skill (inf = 4/16).

AutoSpell Bonuses                                  | Description
:------------------------------------------------- | :-------------------------
bonus4 bAutoSpellOnSkill,`sk`,`x`,`y`,`n`;         | Adds a `n`/10% chance to autospell skill `x` at level `y` when using skill `sk`
bonus5 bAutoSpellOnSkill,`sk`,`x`,`y`,`n`,`i`;     | Adds a `n`/10% chance to autospell skill `x` at level `y` when using skill `sk` <br/> i: <br/> Flags (bitfield)<br/> &1: Forces the skill to be casted on self, rather than on the target of skill `sk`<br/> &2: Random skill level between 1 and l is chosen.
bonus4 bAutoSpell,`sk`,`y`,`n`,`i`;                | `n`/10% chance to cast skill `sk` of level `y` when attacking
bonus5 bAutoSpell,`sk`,`y`,`n`,`bf`,`i`;           | `n`/10% chance to cast skill `sk` of level `y` when attacking
bonus4 bAutoSpellWhenHit,`sk`,`y`,`n`,`i`;         | `n`/10% chance to cast skill `sk` of level `y` when being hit by a direct attack
bonus5 bAutoSpellWhenHit,`sk`,`y`,`n`,`bf`,`i`;    | `n`/10% chance to cast skill `sk` of level `y` when being hit by a direct attack <br/>i: <br/>0 = cast on self <br/>1 = cast on enemy, not on self <br/>2 = use random skill lv in [1..y] <br/>3 = 1+2 (random lv on enemy)
bonus3 bAutoSpellWhenHit,`sk`,`x`,`n`;             | `n`/10% chance to cast skill `sk` of level `x` on attacker when being hit by a direct attack
bonus3 bAutoSpell,`sk`,`x`,`n`;                    | Auto Spell casting on attack of spell `sk` at level `x` with `n`/10% chance


---------------

### 6. Misc Bonuses

HP/SP Drain                            | Description
:------------------------------------- | :-------------------------
bonus bHPDrainValue,`n`;               | Heals +`n` HP with weapon attack.
bonus2 bHPDrainValue,`n`,`x`;          | Heals +`n` HP with weapon attack. When `x` is non-zero, the HP is drained instead.
bonus2 bHPDrainRate,`n`,`x`;           | `n`/10% probability to drain `x`% HP when attacking
bonus bSPDrainValue,`n`;               | When hitting a monster by physical attack, you gain `n` SP
bonus2 bSPDrainRate,`n`,`x`;           | `n`/10% probability to drain `x`% SP when attacking
bonus2 bSPDrainValue,`n`,`x`;          | When hitting a monster by physical attack <br/> x: <br/> 0: Gain `n` SP <br/> 1: drain `n` SP from target
bonus3 bSPDrainRate,`n`,`x`,`y`;       | When attacking there is a `n`/10% chance to either gain SP equivalent to `x`% of damage dealt, OR drain the amount of sp from the enemy. <br/> y: <br/> 0: Gain SP <br/> 1: Drain SP from target
bonus2 bHPDrainValueRace,`r`,`n`;      | Heals +`n` HP when attacking a monster of race `r` with weapon attack.
bonus2 bSPDrainValueRace,`r`,`n`;      | Heals +`n` SP when attacking a monster of race `r` with weapon attack.
bonus3 bHPDrainRateRace,`r`,`n`,`x`;   | Adds a `n`/10% chance to receive `x`% of damage dealt as HP from a monster of race `r` with weapon attack.
bonus3 bSPDrainRateRace,`r`,`n`,`x`;   | Adds a `n`/10% chance to receive `x`% of damage dealt as SP from a monster of race `r` with weapon attack.

HP/SP Vanish                           | Description
:------------------------------------- | :-------------------------
bonus2 bHPVanishRate,`n`,`x`;          | Add the (`n`/10)% chance of decreasing enemy HP amount by `x`% when attacking
bonus2 bSPVanishRate,`n`,`x`;          | Add the (`n`/10)% chance of decreasing enemy SP amount by `x`% when attacking
bonus3 bHPVanishRate,`n`,`x`,`bf`;     | Add the (`n`/10)% chance of decreasing enemy HP amount by `x`% when attacking for criteria `bf`
bonus3 bSPVanishRate,`n`,`x`,`bf`;     | Add the (`n`/10)% chance of decreasing enemy SP amount by `x`% when attacking for criteria `bf`

HP/SP Gain                             | Description
:------------------------------------- | :-------------------------
bonus bHPGainValue,`n`;                | When killing a monster by physical attack, you gain `n` HP
bonus bSPGainValue,`n`;                | When killing a monster by physical attack, you gain `n` SP
bonus bMagicHPGainValue,`n`;           | Gains +`n` HP when killing an enemy with magic attack
bonus bMagicSPGainValue,`n`;           | Gains +`n` SP when killing an enemy with magic attack
bonus2 bHPGainRaceAttack,`r`,`n`;      | Heals `n` HP when attacking race `r` on every hit
bonus2 bSPGainRaceAttack,`r`,`n`;      | Heals `n` SP when attacking race `r` on every hit
bonus2 bSPGainRace,`r`,`n`;            | When killing a monster of race `r` by physical attack gain `n` SP
bonus3 bStateNoRecoverRace,`r`,`x`,`t`;| Set target to a no recovery state based on race `r` at `x`% for `t` milliseconds with normal attack.

Damage return                          | Description
:------------------------------------- | :-------------------------
bonus bMagicDamageReturn,`n`;          | Adds a `n`% chance to reflect targetted magic spells back to the enemy that caused it
bonus bShortWeaponDamageReturn,`n`;    | Reflects `n`% of received melee damage back to the enemy that caused it
bonus bLongWeaponDamageReturn,`n`;     | Reflects `n`% of received ranged damage back to the enemy that caused it

NOTE:
 - `n` is meaningless if not mentioned.
 
Strip/Break equipment                    | Description
:--------------------------------------- | :-------------------------
bonus bUnstripable,`n`;                  | Equipment cannot be taken off via strip skills
bonus bUnstripableWeapon,`n`;            | Weapon cannot be taken off via Strip skills
bonus bUnstripableArmor,`n`;             | Armor cannot be taken off via Strip skills 
bonus bUnstripableHelm,`n`;              | Helm cannot be taken off via Strip skills
bonus bUnstripableShield,`n`;            | Shield cannot be taken off via Strip skills
bonus bUnbreakable,`n`;                  | Reduces the break chance of all equipped equipment by `n`%.
bonus bUnbreakableGarment,`n`;           | Garment cannot be damaged/broken by any means
bonus bUnbreakableWeapon,`n`;            | Weapon cannot be damaged/broken by any means
bonus bUnbreakableArmor,`n`;             | Armor cannot be damaged/broken by any means
bonus bUnbreakableHelm,`n`;              | Helm cannot be damaged/broken by any means 
bonus bUnbreakableShield,`n`;            | Shield cannot be damaged/broken by any means
bonus bUnbreakableShoes,`n`;             | Shoes cannot be damaged/broken by any means
bonus bBreakWeaponRate,`n`;              | Adds a `n`/100% chance to break enemy's weapon while attacking (Stackable)
bonus bBreakArmorRate,`n`;               | Adds a `n`/100% chance to break enemy's armor while attacking (Stackable)

NOTE:
 - `n` is meaningless if not mentioned.
 
Monster Related                            | Description
:----------------------------------------- | :-------------------------
bonus3 bAddClassDropItem,`id`,`c`,`n`;     | Adds a `n`/100% chance of dropping item id when killing monster mid
bonus2 bAddMonsterDropItem,`id`,`n`;       | Adds a `n`/100% chance for item id to be dropped, when killing any monster.
bonus3 bAddMonsterDropItem,`id`,`r`,`n`;   | Adds a `n`/100% chance for item id to be dropped, when killing any monster of race `r`. <br/> If `n` is negative value, then it's a part of formula <br/> `chance = -y*(killed_mob_level/10)+1`
bonus bAddMonsterDropChainItem,`ic`;       | Able to get Item of chain `ic` when you kill a monster
bonus2 bAddMonsterDropChainItem,`ic`,`r`;  | Able to get item of chain `ic` when you kill a monster of race `r`
bonus2 bGetZenyNum,`x`,`n`;                | When killing a monster, there is a `n`% chance of gaining 1~x zeny (only the highest among all is applied).
bonus2 bAddGetZenyNum,`x`,`n`;             | When killing a monster, there is a `n`% chance of gaining 1~x zeny (Stackable) <br/> x: <br/> < 0: Max Zeny gain is `(-x*monster_level)`
bonus2 bDropAddRace,`r`,`n`;               | Increase item drop rate by `n`% when you kill a monster of race `r`

Misc effects                           | Description
:------------------------------------- | :-------------------------
skill i,`n`;                           | Gives skill #i at level n
bonus bDoubleRate,`n`;                 | Double Attack probability +n% (works with all weapons | only the highest among all is applied)
bonus bDoubleAddRate,`n`;              | Double Attack probability +n% (works with all weapons)
bonus bSplashRange,`n`;                | Splash attack radius +`n` (highest is applied)
bonus bSplashAddRange,`n`;             | Splash attack radius + `n` (e.g. `n`=1 makes a `3*3` cells area, `n`=2 a `5*5` area, etc) <br/> `n`: <br/> 1: 3*3 Area <br/> 2: 5*5 Area <br/> ...
bonus bClassChange,`n`;                | Gives a `n`/100% chance to change the attacked monster's class with normal attack.
bonus bAddStealRate,`n`;               | `n`/100% increase to Steal skill success chance
bonus bRestartFullRecover,`n`;         | When reviving, HP and SP are fully healed
bonus bNoSizeFix,`n`;                  | The attack revision with the size of the monster is not received
bonus bNoGemStone,`n`;                 | Skills requiring Gemstones do no require them (Hocus Pocus will still require 1 Yellow Gemstone)
bonus bIntravision,`n`;                | Always see Hiding and Cloaking players/mobs <br/> `n`: is meaningless
bonus2 bAddSkillBlow,`sk`,`n`;         | Knockbacks the target by `n` cells when using skill `sk`
bonus bNoKnockback,`n`;                | Character is no longer knocked back by enemy skills with such effect (`n` is meaningless)
bonus bPerfectHide,`n`;                | Hidden/cloaked character is no longer detected by monsters with 'detector' mode (`n` is meaningless).

