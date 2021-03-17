# Quest Variables

<!--
## Copyright
> This file is part of Hercules.
> http://herc.ws - http://github.com/HerculesWS/Hercules
> 
> Copyright (C) 2012-2021 Hercules Dev Team
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
This file should help to understand and manage bit-wise quest variables. 
You can store up to 31 boolean value into a single variable.

## Sample Variable: `MISC_QUEST`

Quest # No    | Sample Quest
:-------------| :------------
Description   | How to assign a bit-wise value and check it.
Assign Value  | ```MISC_QUEST = MISC_QUEST \| X;```
Check Value   | ```if (MISC_QUEST & X) { ...  }```

- Where `X` refer to the bit-wise value that could be used to represent the state/progress of a quest.

--------------------------------------------------------------

## Example

Quest # 1     | Juice Maker Quest
:-------------| :------------
Description   | How to make juices. This bit keeps final state of the quest.
Assign Value  | ```MISC_QUEST = MISC_QUEST \| 1;```
Check Value   | ```if (MISC_QUEST & 1) { ...  }```

Quest # 2     | Tempestra Quest
:-------------| :------------
Description   | Determines if player has given a potion to Tempestra.
Assign Value  | ```MISC_QUEST = MISC_QUEST \| 2;```
Check Value   | ```if (MISC_QUEST & 2) { ...  }```

Quest # 3     | Morgenstein Quest
:-------------| :------------
Description   | How to make Mixture & Counteragent. This bit keeps final state of the quest.
Assign Value  | ```MISC_QUEST = MISC_QUEST \| 4;```
Check Value   | ```if (MISC_QUEST & 4) { ...  }```

Quest # 4     | Prontera Culvert Quest
:-------------| :------------
Description   | Determines if player can enter Prontera Culverts.
Assign Value  | ```MISC_QUEST = MISC_QUEST \| 8;```
Check Value   | ```if (MISC_QUEST & 8) { ...  }```

Quest # 5     | Edgar's Offer
:-------------| :------------
Description   | Cheap ticket from Izlude to Alberta. This bit keeps final state of the quest.
Assign Value  | ```MISC_QUEST = MISC_QUEST \| 16;```
Check Value   | ```if (MISC_QUEST & 16) { ...  }```

Quest # 6     | Piano Quest
:-------------| :------------
Description   | The only way from Niflheim to Umbala.
Assign Value  | ```MISC_QUEST = MISC_QUEST \| 32;```
Check Value   | ```if (MISC_QUEST & 32) { ...  }```

Quest # 7     | Bio Ethics Quest
:-------------| :------------
Description   | Quest for homunculus skill for alchemists. This bit keeps final state of the quest.
Assign Value  | ```MISC_QUEST = MISC_QUEST \| 64;```
Check Value   | ```if (MISC_QUEST & 64) { ...  }```

Quest # 8     | DTS Warper
:-------------| :------------
Description   | Determines if player has already voted.
Assign Value  | ```MISC_QUEST = MISC_QUEST \| 128;```
Check Value   | ```if (MISC_QUEST & 128) { ...  }```

Quest # 9     | -
:-------------| :------------
Description   | -
Assign Value  | ```MISC_QUEST = MISC_QUEST \| 256;```
Check Value   | ```if (MISC_QUEST & 256) { ...  }```

Quest # 10    | Cube Room
:-------------| :------------
Description   | Lighthalzen Cube Room quest (to enter Bio-Lab)
Assign Value  | ```MISC_QUEST = MISC_QUEST \| 512;```
Check Value   | ```if (MISC_QUEST & 512) { ...  }```

Quest # 11    | Reset Skills Event
:-------------| :------------
Description   | Yuno, Hypnotist Teacher
Assign Value  | ```MISC_QUEST = MISC_QUEST \| 1024;```
Check Value   | ```if (MISC_QUEST & 1024) { ...  }```

Quest # 12    | Slotted Arm Guard Quest
:-------------| :------------
Description   | Ninja Job Room, Boshuu
Assign Value  | ```MISC_QUEST = MISC_QUEST \| 2048;```
Check Value   | ```if (MISC_QUEST & 2048) { ...  }```

Quest # 13    | Improved Arm Guard Quest
:-------------| :------------
Description   | Ninja Job Room, Basshu
Assign Value  | ```MISC_QUEST = MISC_QUEST \| 4096;```
Check Value   | ```if (MISC_QUEST & 4096) { ...  }```

Quest # 14    | Rachel Sanctuary Quest
:-------------| :------------
Description   | Determines if player can access Rachel Santuary.
Assign Value  | ```MISC_QUEST = MISC_QUEST \| 8192;```
Check Value   | ```if (MISC_QUEST & 8192) { ...  }```

Quest # 15    | Message Delivery Quest
:-------------| :------------
Description   | Send a message to Elly, in Niflheim from Erious.
Assign Value  | ```MISC_QUEST = MISC_QUEST \| 16384;```
Check Value   | ```if (MISC_QUEST & 16384) { ...  }```

Quest # 16    | Umbala Domestic Dispute?
:-------------| :------------
Description   | Reward: 1 Yggdrasil Leaf.
Assign Value  | ```MISC_QUEST = MISC_QUEST \| 32768;```
Check Value   | ```if (MISC_QUEST & 32768) { ...  }```

Quest # 17    | Access to the Turtle Island
:-------------| :------------
Description   | Reward: ~1 Old Card Album , Old Violet Box, GB.
Assign Value  | ```MISC_QUEST = MISC_QUEST \| 65536;```
Check Value   | ```if (MISC_QUEST & 65536) { ...  }```

Quest # 18    | -
:-------------| :------------
Description   | -
Assign Value  | ```MISC_QUEST = MISC_QUEST \| 131072;```
Check Value   | ```if (MISC_QUEST & 131072) { ...  }```

Quest # 19    | -
:-------------| :------------
Description   | -
Assign Value  | ```MISC_QUEST = MISC_QUEST \| 262144;```
Check Value   | ```if (MISC_QUEST & 262144) { ...  }```

Quest # 20    | -
:-------------| :------------
Description   | -
Assign Value  | ```MISC_QUEST = MISC_QUEST \| 524288;```
Check Value   | ```if (MISC_QUEST & 524288) { ...  }```

Quest # 21    | -
:-------------| :------------
Description   | -
Assign Value  | ```MISC_QUEST = MISC_QUEST \| 1048576;```
Check Value   | ```if (MISC_QUEST & 1048576) { ...  }```

Quest # 22    | -
:-------------| :------------
Description   | -
Assign Value  | ```MISC_QUEST = MISC_QUEST \| 2097152;```
Check Value   | ```if (MISC_QUEST & 2097152) { ...  }```

Quest # 23    | -
:-------------| :------------
Description   | -
Assign Value  | ```MISC_QUEST = MISC_QUEST \| 4194304;```
Check Value   | ```if (MISC_QUEST & 4194304) { ...  }```

Quest # 24    | -
:-------------| :------------
Description   | -
Assign Value  | ```MISC_QUEST = MISC_QUEST \| 8388608;```
Check Value   | ```if (MISC_QUEST & 8388608) { ...  }```

Quest # 25    | -
:-------------| :------------
Description   | -
Assign Value  | ```MISC_QUEST = MISC_QUEST \| 16777216;```
Check Value   | ```if (MISC_QUEST & 16777216) { ...  }```

Quest # 26    | -
:-------------| :------------
Description   | -
Assign Value  | ```MISC_QUEST = MISC_QUEST \| 33554432;```
Check Value   | ```if (MISC_QUEST & 33554432) { ...  }```

Quest # 27    | -
:-------------| :------------
Description   | -
Assign Value  | ```MISC_QUEST = MISC_QUEST \| 67108864;```
Check Value   | ```if (MISC_QUEST & 67108864) { ...  }```

Quest # 28    | -
:-------------| :------------
Description   | -
Assign Value  | ```MISC_QUEST = MISC_QUEST \| 134217728;```
Check Value   | ```if (MISC_QUEST & 134217728) { ...  }```

Quest # 29    | -
:-------------| :------------
Description   | -
Assign Value  | ```MISC_QUEST = MISC_QUEST \| 268435456;```
Check Value   | ```if (MISC_QUEST & 268435456) { ...  }```

Quest # 30    | -
:-------------| :------------
Description   | -
Assign Value  | ```MISC_QUEST = MISC_QUEST \| 536870912;```
Check Value   | ```if (MISC_QUEST & 536870912) { ...  }```

Quest # 31    | -
:-------------| :------------
Description   | -
Assign Value  | ```MISC_QUEST = MISC_QUEST \| 1073741824;```
Check Value   | ```if (MISC_QUEST & 1073741824) { ...  }```

### Quest#32 and onwards
You had to use a new variable to store it.
The existing variable `MISC_QUEST`'s value may overflow as it already reaching the max value.
Basically the cycle repeat every 32th quests, unless the limit has been lifted in the future.

 No | Formulae    | Bits Value | Accumulate
:--:|:-----------:|-----------:|-------------: 
  1 |   2 ^  0    |          1 |           1
  2 |   2 ^  1    |          2 |           3
  3 |   2 ^  2    |          4 |           7
  4 |   2 ^  3    |          8 |          15
  5 |   2 ^  4    |         16 |          31
  6 |   2 ^  5    |         32 |          63
  7 |   2 ^  6    |         64 |         127
  8 |   2 ^  7    |        128 |         255
  9 |   2 ^  8    |        256 |         511
 10 |   2 ^  9    |        512 |        1023
 11 |   2 ^ 10    |       1024 |        2047
 12 |   2 ^ 11    |       2048 |        4095
 13 |   2 ^ 12    |       4096 |        8191
 14 |   2 ^ 13    |       8192 |       16383
 15 |   2 ^ 14    |      16384 |       32767
 16 |   2 ^ 15    |      32768 |       65535
 17 |   2 ^ 16    |      65536 |      131071
 18 |   2 ^ 17    |     131072 |      262143
 19 |   2 ^ 18    |     262144 |      524287
 20 |   2 ^ 19    |     524288 |     1048575
 21 |   2 ^ 20    |    1048576 |     2097151
 22 |   2 ^ 21    |    2097152 |     4194303
 23 |   2 ^ 22    |    4194304 |     8388607
 24 |   2 ^ 23    |    8388608 |    16777215
 25 |   2 ^ 24    |   16777216 |    33554431
 26 |   2 ^ 25    |   33554432 |    67108863
 27 |   2 ^ 26    |   67108864 |   134217727
 28 |   2 ^ 27    |  134217728 |   268435455
 29 |   2 ^ 28    |  268435456 |   536870911
 30 |   2 ^ 29    |  536870912 |  1073741823
 31 |   2 ^ 30    | 1073741824 |  2147483647 

