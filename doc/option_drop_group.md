# Option Drop Group Database

## Description
Explanation of the `db/option_drop_groups.conf` file and structure.

This database file allows the creation of groups of random options
that will be added to certain equipments when dropped. After creating
a group in this database file, you may set up drops in `mob_db` to use
it in order to get items with these options. For more information on
adding option drop groups to `mob_db`, check `doc/mob_db.txt` documentation file.

Each item may have up to `MAX_ITEM_OPTION` options at the same time,
in this document, each of these independent options will be called
`option slot`. One drop group will define the possibilities of random
options for each of these slots.

## Entries Format

```
<Group Name Constant>: (
	{ // Option Slot 1
		Rate: (int) chance of filling option slot 1 (100 = 1%)

		// Possible options for slot 1
		// min/max value : int, defaults to 0
		// chance : int, 100 = 1% if not set, will be 100%/number of possibilities
		OptionName: value
		// or
		OptionName: [min value, max value]
		// or
		OptionName: [min value, max value, chance]
		// ...  (as many as you want)
	},
	// ... (up to MAX_ITEM_OPTION)
),
```

### `Group Name Constant`
This is the group name, it is how this group is referenced in other files
(e.g. mob_db). It must be globally unique, as it is a server constant, and
must contain only letters, numbers and " _ ".

### `Rate`
This is the chance of this option slot to drop. In other words, this is the
chance of getting this slot filled with something, where something is given
by the list of `OptionName` that follows.

Rate is an integer value where 100 means 1%.

### `OptionName`
Adds `OptionName` as one option that may fill this slot when it drops.

The details of this option may be specified in one of 3 ways:

#### `OptionName: value`
The chance of this option being picked is auto calculated (see below),
and if this option is chosen, its value will be `value`.

#### `OptionName: [min, max]`
The chance of this option being picked is auto calculated (see below),
and if this option is chosen, its value will be a random integer between
`min` and `max` (both included).

#### `OptionName: [min, max, chance]`
The chance of this option being picked is `chance`, and if this option is chosen,
its value will be a random integer between `min` and `max` (both included).

#### Auto calculated chances
When chance is not specified in an option, it will be auto calculated by
the server as being `100%/num`, when `num` is the number of possibilities
in this option slot.

For example, if you specify 3 possible options, all of them without
a `chance` defined, all of them will have 33.33% chance of being
picked (100%/3). If you set the chance of one of them to 50%, you
will have one option with 50% chance, and each of the others with
33.33% chance.

## Example
```
MYITEM: (
	{ // Option Slot 1
		Rate: 10000 // It has 100% of chance of being filled

		// This slot may have one of the following options:
		WEAPON_ATTR_WIND: 5,                // WEAPON_ATTR_WIND Lv5 (33.33%)
		WEAPON_ATTR_GROUND: [2, 4]          // WEAPON_ATTR_GROUND Lv 2~4 (33.33%)
		WEAPON_ATTR_POISON: [1, 4, 8000]    // WEAPON_ATTR_POISON Lv 1~4 (80%)
	},
	{ // Option Slot 2
		Rate: 5000 // It has 50% of chance of being filled

		// If filled, may have one of the following options:
		WEAPON_ATTR_WATER: 4     // WEAPON_ATTR_WATER Lv4 (100%)
	}
)
```
