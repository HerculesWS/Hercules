# Unit parameters database

<!--
## Copyright
> This file is part of Hercules.
> http://herc.ws - http://github.com/HerculesWS/Hercules
>
> Copyright (C) 2023-2023 Hercules Dev Team
> Copyright (C) KirieZ
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
This file is a documentation for the unit parameters database, which defines groups
of general aspects, such as stats and maximum HP, of game "live" units (currently, players only).
The groups defined in this file should later be linked in other dbs.

It explains the following database files:
- [db/pre-re/unit_parameters_db.conf](../db/pre-re/unit_parameters_db.conf)
- [db/re/unit_parameters_db.conf](../db/re/unit_parameters_db.conf)


> **Note:** Currently, this database is only used by Job Database in order to define
> some differences between jobs stats.
>

--------------------------------------------------------------

## Entry structure
```
<Group Name>: {
	Inherit: "<Another Group Name>"    (string, optional, default: no inheritance)
	NaturalHealWeightRate: <value>     (int)
	MaxASPD: <value>                   (int)
	MaxHP: <value>                     (int) (can be groupped by level ranges)
	MaxStats: <value>                  (int)
}
```

--------------------------------------------------------------

## Field explanation

### Group Name
The key that uniquely identify this group.

It is used whenever we have to reference it, for example:
- For `Inherit` field when inheriting it in another group
- When linking it to a job in JobDB (`ParametersGroup` field)

The value of this field should be unique in the entire db.


### Inherit
When defined, it copies all the values of the other group.

This is the first field checked when creating a new group, thus any other definitions
for this group will override the previously inherited value.

This field is optional, and when not informed will simply be ignored (no inheritance).


### NaturalHealWeightRate
For players, the percent of weight at which natural healing is blocked.

A value of `50` would mean that once this unit reaches 50% of weight, it will no longer
heal automatically until it reduces some weight.

The value must be between `1` and `101`, where `101` will disable this restriction.


### MaxASPD
The maximum ASPD (Attack speed) this unit may reach. After this value is reached,
any further increments in ASPD is ignored.

The value must be between `100` and `199`.


### MaxHP
The maximum HP this unit may reach. After this value is reached, any further increases in MaxHP
is ignored.

> **Note:** This is **NOT** the default class MaxHP, but how much the class' MaxHP may be increased
> with items/stats/etc.
>

This configuration may be provided in 2 different ways:

As a single, flat value:
```
	MaxHP: 1_000_000
```

Would mean that the unit using this group may go up to 1,000,000 HP, regardless of its level.

Another option is to group it by level ranges:
```
	MaxHP: {
		Lv99: 330_000     // From Level 1 to Level 99
		Lv150: 660_000    // From Level 100 to Level 150
		Lv175: 1_100_000  // From Level 151 to Level 175 to "Max Level"
	}
```

In this case, each level specified in the `Lv<number>` format will be used as the upper
limit level for this MaxHP value.

> **NOTE:** The highest level defined in the group (in the example above, Lv175) will be expanded
> to `MAX_LEVEL` during startup in order to cover missing levels.
>
> In other words, if your server's `MAX_LEVEL` was set to 200, using the above configuration would
> make units between level 176 and 200 to have the same limit as units between level 151 and 175
> (`Lv175` config "becomes" `Lv200`).
>


### MaxStats
The maximum value a single stat (STR, AGI, DEX, VIT, INT, LUK) may have.

The value must be between `10` and `10_000`.

