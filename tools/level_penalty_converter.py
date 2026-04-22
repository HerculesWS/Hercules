#!/usr/bin/env python3
# -*- coding: utf8 -*-
#
# This file is part of Hercules.
# http://herc.ws - http://github.com/HerculesWS/Hercules
#
# Copyright (C) 2015-2026 Hercules Dev Team
# Copyright (C) 2026 KeiKun
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
#
# Converts db/re/level_penalty.txt to db/re/level_penalty.conf format.
# Run from the tools/ directory.
#
# Usage:
#   python3 level_penalty_converter.py > ../db/re/level_penalty.conf

import re
import sys

RACE_NAMES = {
    0:  'RC_Formless',
    1:  'RC_Undead',
    2:  'RC_Brute',
    3:  'RC_Plant',
    4:  'RC_Insect',
    5:  'RC_Fish',
    6:  'RC_Demon',
    7:  'RC_DemiHuman',
    8:  'RC_Angel',
    9:  'RC_Dragon',
    10: 'RC_Player',
    11: 'RC_Boss',
    12: 'RC_NonBoss',
}

TYPE_NAMES = {
    1: 'EXP_PENALTY_RATE',
    2: 'ITEM_DROP_PENALTY_RATE',
}

HEADER = """\
//================= Hercules Database =====================================
//=       _   _                     _
//=      | | | |                   | |
//=      | |_| | ___ _ __ ___ _   _| | ___  ___
//=      |  _  |/ _ \\ '__/ __| | | | |/ _ \\/ __|
//=      | | | |  __/ | | (__| |_| | |  __/\\__ \\
//=      \\_| |_/\\___|_|  \\___|\\__,_|_|\\___||___/
//================= License ===============================================
//= This file is part of Hercules.
//= http://herc.ws - http://github.com/HerculesWS/Hercules
//=
//= Copyright (C) 2015-2026 Hercules Dev Team
//=
//= Hercules is free software: you can redistribute it and/or modify
//= it under the terms of the GNU General Public License as published by
//= the Free Software Foundation, either version 3 of the License, or
//= (at your option) any later version.
//=
//= This program is distributed in the hope that it will be useful,
//= but WITHOUT ANY WARRANTY; without even the implied warranty of
//= MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//= GNU General Public License for more details.
//=
//= You should have received a copy of the GNU General Public License
//= along with this program.  If not, see <http://www.gnu.org/licenses/>.
//=========================================================================
//= Level penalty configuration file
//= Used in RENEWAL ONLY to determine experience and item drop penalties
//= based on the level difference between the player and the monster.
//=========================================================================

level_penalty_db: (
/**************************************************************************
 ************* Entry structure ********************************************
 **************************************************************************
{
\t// Level of the defending element (by default, may be Lv1 up to Lv4)
\ttype: <"EXP_PENALTY_RATE" or "ITEM_DROP_PENALTY_RATE">,
\trace: <RC_* constant (e.g. "RC_Boss", "RC_NonBoss")>,
\tdiff: <level difference>,
\trate: <penalty rate, where 100 means 100% (base rate, no additions/reductions)>
},
**************************************************************************/\
"""

def log(message):
    print(message, file=sys.stderr)

def parse_level_penalty_txt(path):
    entries = []
    with open(path, encoding='utf-8') as f:
        for line in f:
            line = re.sub(r'//.*', '', line).strip()
            if not line:
                continue
            parts = [p.strip() for p in line.split(',')]
            if len(parts) != 4:
                log(f'Warning: skipping malformed line: {line!r}')
                continue
            type_id, race_id, diff, rate = int(parts[0]), int(parts[1]), int(parts[2]), int(parts[3])
            if type_id not in TYPE_NAMES:
                log(f'Warning: unknown type {type_id}, skipping')
                continue
            if race_id not in RACE_NAMES:
                log(f'Warning: unknown race {race_id}, skipping')
                continue
            entries.append((type_id, race_id, diff, rate))
    return entries

def main():
    txt_path = '../db/re/level_penalty.txt'
    entries = parse_level_penalty_txt(txt_path)
    log(f'Read {len(entries)} entries from {txt_path}')

    print(HEADER)
    for type_id, race_id, diff, rate in entries:
        print('{')
        print(f'\ttype: "{TYPE_NAMES[type_id]}",')
        print(f'\trace: "{RACE_NAMES[race_id]}",')
        print(f'\tdiff: {diff},')
        print(f'\trate: {rate}')
        print('},')
    print(')')

if __name__ == '__main__':
    main()
