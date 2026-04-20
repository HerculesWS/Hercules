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
# This script merges mob_race2_db.txt data into mob_db.conf by adding
# RaceGroup fields. Run from the tools/ directory.
#
# Usage:
#   python3 mob_race2_db_converter.py [pre-re|re]
#
# Output is written to stdout; redirect to replace the target file:
#   python3 mob_race2_db_converter.py re > ../db/re/mob_db.conf

import re
import sys

RC2_NAMES = {
    0: 'RC2_None',
    1: 'RC2_Goblin',
    2: 'RC2_Kobold',
    3: 'RC2_Orc',
    4: 'RC2_Golem',
    5: 'RC2_Guardian',
    6: 'RC2_Ninja',
    7: 'RC2_Scaraba',
    8: 'RC2_Turtle',
}

def log(message):
    print(message, file=sys.stderr)

def parse_race2_db(path):
    """Returns dict of mob_id -> RC2 constant name."""
    mapping = {}
    with open(path, encoding='utf-8') as f:
        for line in f:
            line = re.sub(r'//.*', '', line).strip()
            if not line:
                continue
            parts = [p.strip() for p in line.split(',')]
            if not parts or not parts[0]:
                continue
            race_id = int(parts[0])
            if race_id not in RC2_NAMES:
                log(f'Warning: unknown race2 id {race_id}, skipping')
                continue
            rc2_name = RC2_NAMES[race_id]
            for mob_id_str in parts[1:]:
                if mob_id_str:
                    mapping[int(mob_id_str)] = rc2_name
    return mapping

def inject_racegroup(mob_db, race2_map):
    """Inserts RaceGroup after the Race: line for each mob that has a mapping."""
    def replace_entry(m):
        mob_id = int(m.group(1))
        body = m.group(0)
        if mob_id not in race2_map:
            return body
        rc2 = race2_map[mob_id]
        if 'RaceGroup:' in body:
            log(f'Mob {mob_id}: already has RaceGroup, skipping')
            return body
        log(f'Mob {mob_id}: adding RaceGroup: "{rc2}"')
        return re.sub(
            r'(\tRace:[ \t]+[^\n]+\n)',
            r'\1\tRaceGroup: "' + rc2 + r'"\n',
            body,
            count=1,
        )

    # Match each mob entry block (from Id: N to the closing },)
    return re.sub(
        r'\{[^{]*?\tId:[ \t]+(\d+)\b[\s\S]*?\n\},',
        replace_entry,
        mob_db,
    )

def main():
    mode = 're'
    if len(sys.argv) > 1:
        mode = sys.argv[1].lower()
    if mode not in ('pre-re', 're'):
        log(f'Usage: {sys.argv[0]} [pre-re|re]')
        sys.exit(1)

    race2_path = f'../db/{mode}/mob_race2_db.txt'
    mob_db_path = f'../db/{mode}/mob_db.conf'

    race2_map = parse_race2_db(race2_path)
    log(f'Loaded {len(race2_map)} mob->race2 mappings from {race2_path}')

    with open(mob_db_path, encoding='utf-8') as f:
        mob_db = f.read()

    mob_db = inject_racegroup(mob_db, race2_map)
    print(mob_db, end='')

if __name__ == '__main__':
    main()
