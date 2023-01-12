#!/usr/bin/env python3
# -*- coding: utf8 -*-
#
# This file is part of Hercules.
# http://herc.ws - http://github.com/HerculesWS/Hercules
#
# Copyright (C) 2021-2023 Hercules Dev Team
# Copyright (C) 2021 Asheraf
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

import utils.common as Tools
import sys

old_databases = {}
MOBG_NAME = {
    0: 'MOBG_DEAD_BRANCH',
    1: 'MOBG_PORING',
    2: 'MOBG_BLOODY_BRANCH',
    3: 'MOBG_POUCH',
    4: 'MOBG_CLASS_CHANGE',
}

def LoadOldDatabase(filepath, gid):
    db = {}
    with open(filepath) as f:
        for l in f:
            l = l.strip()
            if not l:
                continue
            if l.startswith('//'):
                continue
            class_, name, rate = l.split(',')
            if int(class_) > 0:
                db[int(class_)] = int(rate)
    old_databases[gid] = db

def WriteDatabase(mode, serverpath):
    MobDB = Tools.LoadDBConsts('mob_db', mode, serverpath)

    for k, v in old_databases.items():
        print(f'{MOBG_NAME[k]}: (')
        for i in v:
            print(f'\t("{MobDB[i]}", {v[i]}),')
        print(')')

if len(sys.argv) != 3:
	print('Monster Skill db converter from txt to conf format')
	print('Usage:')
	print('    mobgroupconverter.py mode serverpath')
	print("example:")
	print('    mobgroupconverter.py pre-re ../')
	exit(1)

if sys.argv[1] != 're' and sys.argv[1] != 'pre-re':
	print('you have entred an invalid server mode')
	exit(1)

LoadOldDatabase(f'{sys.argv[2]}/db/{sys.argv[1]}/mob_branch.txt', 0)
LoadOldDatabase(f'{sys.argv[2]}/db/{sys.argv[1]}/mob_poring.txt', 1)
LoadOldDatabase(f'{sys.argv[2]}/db/{sys.argv[1]}/mob_boss.txt', 2)
LoadOldDatabase(f'{sys.argv[2]}/db/mob_pouch.txt', 3)
LoadOldDatabase(f'{sys.argv[2]}/db/mob_classchange.txt', 4)
WriteDatabase(sys.argv[1], sys.argv[2])
