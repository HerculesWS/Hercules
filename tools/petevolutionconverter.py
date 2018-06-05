#!/usr/bin/python
# -*- coding: utf8 -*-
#
# This file is part of Hercules.
# http://herc.ws - http://github.com/HerculesWS/Hercules
#
# Copyright (C) 2018  Hercules Dev Team
# Copyright (C) 2018 Dastgir
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
# Usage:
#  python petevolutionconverter.py PetEvolutionCln.lub re ../ > pet_evolve_db.conf

import re
import sys
import utils.common as Tools

def printHeader():
	print('''//================= Hercules Database =====================================
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
//= Copyright (C) 2018  Hercules Dev Team
//= Copyright (C) 2018 Dastgir
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
//= Pet Evolution Database
//=========================================================================

pet_evolve: (
/**************************************************************************
 ************* Entry structure ********************************************
 **************************************************************************
{
	// ================ Mandatory fields ==============================
	Id: ID                               (int, Evolved Pet EggID)
	From: ID                             (int, Evolving Pet EggID)
	Items: {
		Name: Amount                     (items required to perform evolution)
		...
	}
},
**************************************************************************/''')

def printEntry(entry):
	print('{')
	print('\tId: {}'.format(entry['id']))
	print('\tFrom: {}'.format(entry['from']))
	print('\tItems: {')
	for items in entry['items']:
		if (items[2] == 1):
			print('\t\t{}: {}'.format(items[0], items[1]))
		else:
			print('\t\t//ID{}: {}'.format(items[0], items[1]))
	print('\t}')
	print('},')

def ConvertDB(luaName, mode, serverpath):
	ItemDB = Tools.LoadDBConsts('item_db', mode, serverpath)
	f = open(luaName)
	content = f.read()
	f.close()

	recipeDB = re.findall(r'InsertEvolutionRecipeLGU\((\d+),\s*(\d+),\s*(\d+),\s*(\d+)\)', content)

	current = 0

	entry = dict()
	printHeader()
	for recipe in recipeDB:
		fromEgg = int(recipe[0])
		petEgg = int(recipe[1])
		itemID = int(recipe[2])
		quantity = int(recipe[3])

		if (current == 0):
			entry = {
				'id': petEgg,
				'from': fromEgg,
				'items': list()
			}
			current = petEgg

		if (current != petEgg):
			printEntry(entry)
			entry = {
				'id': petEgg,
				'from': fromEgg,
				'items': list()
			}
			entry['id'] = petEgg
			entry['items'] = list()
			current = petEgg

		if (itemID in ItemDB):
			entry['items'].append((ItemDB[itemID], quantity, 1))
		else:
			entry['items'].append((itemID, quantity, 0))
	printEntry(entry)
	print(')')


if len(sys.argv) != 4:
	print('Pet Evolution Lua to DB')
	print('Usage:')
	print('    petevolutionconverter.py lua mode serverpath')
	print("example:")
	print('    petevolutionconverter.py PetEvolutionCln.lua pre-re ../')
	exit(1)

ConvertDB(sys.argv[1], sys.argv[2], sys.argv[3])
