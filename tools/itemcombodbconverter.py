#!/usr/bin/env python3
# -*- coding: utf8 -*-
#
# This file is part of Hercules.
# http://herc.ws - http://github.com/HerculesWS/Hercules
#
# Copyright (C) 2019-2022 Hercules Dev Team
# Copyright (C) 2019 Asheraf
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

import argparse
import json
import re
from utils import libconf
import utils.common as Tools


def ConvertFile(args):
	print(r'''//================= Hercules Database =====================================
//=       _   _                     _
//=      | | | |                   | |
//=      | |_| | ___ _ __ ___ _   _| | ___  ___
//=      |  _  |/ _ \ '__/ __| | | | |/ _ \/ __|
//=      | | | |  __/ | | (__| |_| | |  __/\__ \
//=      \_| |_/\___|_|  \___|\__,_|_|\___||___/
//================= License ===============================================
//= This file is part of Hercules.
//= http://herc.ws - http://github.com/HerculesWS/Hercules
//=
//= Copyright (C) 2019-2022 Hercules Dev Team
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
//================= Description ===========================================
// Configurations file for status effects
//=========================================================================

combo_db: (
/**************************************************************************
 ************* Entry structure ********************************************
 **************************************************************************
{
	// ================ Mandatory fields ===============================
	Items: ["item_list"]   (string, array) list of items
	Script: <"
		Script
		(it can be multi-line)
	">
}
**************************************************************************/''')
	ItemDB = Tools.LoadDBConsts('item_db', f'{args.mode}', '../')
	with open (f'../db/{args.mode}/item_combo_db.txt') as dbfile:
		line = 0
		for entry in dbfile:
			line = line + 1
			if not entry.strip() or entry.startswith('//'):
				continue
			m = re.search(r'(^[0-9:]+),\{(.*)\}$', entry)
			if not m:
				print(f'Error: Invalid pattern in entry {entry}, line {line}, aborting..')
				exit()

			items_list = m.group(1).split(':')
			script = m.group(2)
			for item in range(len(items_list)):
				if int(items_list[item]) not in ItemDB:
					print(f'Error: invalid item {item} found in line {line}, aborting..')
					exit()
				items_list[item] = ItemDB[int(items_list[item])]

			if args.enable_jsbeautifier:
				import jsbeautifier
				opts = jsbeautifier.default_options()
				opts.indent_with_tabs = True
				opts.indent_level = 2
				script = jsbeautifier.beautify(script, opts)
			print(
f'''{{
	Items: {json.dumps(items_list)}
	Script: <"\n{script}\n\t">
}},''')
	print(')')

if __name__ == '__main__':
	parser = argparse.ArgumentParser(description='Convert item combo db to new format')
	parser.add_argument('--mode', type=str, dest='mode', help='Define usage mode re/pre-re.')
	parser.add_argument('--enable-jsbeautifier', type=bool, dest='enable_jsbeautifier', help='Use jsbeautifier to auto format script fields.')
	parsed_args = parser.parse_args()
	ConvertFile(parsed_args)
