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
//= Pets Database
//=========================================================================

pet_db:(
/**************************************************************************
 ************* Entry structure ********************************************
 **************************************************************************
{
	// ================ Mandatory fields ==============================
	Id: ID                               (int)
	SpriteName: "Sprite_Name"            (string)
	Name: "Pet Name"                     (string)
	// ================ Optional fields ===============================
	TamingItem: Taming Item              (string, defaults to 0)
	EggItem: Egg Id                      (string, defaults to 0)
	AccessoryItem: Equipment Id          (string, defaults to 0)
	FoodItem: Food Id                    (string, defaults to 0)
	FoodEffectiveness: hunger points     (int, defaults to 0)
	HungerDelay: hunger time             (int, defaults to 0)
	Intimacy: {
		Initial: start intimacy                   (int, defaults to 0)
		FeedIncrement: feeding intimacy           (int, defaults to 0)
		OverFeedDecrement: overfeeding intimacy   (int, defaults to 0)
		OwnerDeathDecrement: owner die intimacy   (int, defaults to 0)
	}
	CaptureRate: capture rate            (int, defaults to 0)
	Speed: speed                         (int, defaults to 0)
	SpecialPerformance: true/false       (boolean, defaults to false)
	TalkWithEmotes: convert talk         (boolean, defaults to false)
	AttackRate: attack rate              (int, defaults to 0)
	DefendRate: Defence attack           (int, defaults to 0)
	ChangeTargetRate: change target      (int, defaults to 0)
	Evolve: {
		EggID: {						 (string, Evolved Pet EggID)
			Name: Amount                 (items required to perform evolution)
			...
		}
	}
	AutoFeed: true/false                 (boolean, defaults to false)
	PetScript: <" Pet Script (can also be multi-line) ">
	EquipScript: <" Equip Script (can also be multi-line) ">
},
**************************************************************************/''')

def printID(db, name, tabSize = 1):
	if (name not in db or int(db[name]) == 0):
		return
	print('{}{}: {}'.format('\t'*tabSize, name, db[name]))

def printString(db, name, tabSize = 1):
	if (name not in db or db[name].strip() == ""):
		return
	print('{}{}: "{}"'.format('\t'*tabSize, name, db[name]))

def printBool(db, name):
	if (name not in db or db[name] == '0'):
		return
	print('\t{}: true'.format(name))

def printScript(db, name):
	if (name not in db or db[name].strip() == ""):
		return
	print('\t{}: <{}>'.format(name, db[name]))

def printEntry(ItemDB, EvolveDB, autoFeedDB, entry, mode, serverpath):
	PetDB = Tools.LoadDB('pet_db', mode, serverpath)

	for i, db in enumerate(PetDB):
		print('{')
		printID(db, 'Id')
		printString(db, 'SpriteName')
		printString(db, 'Name')

		printString(db, 'TamingItem')
		printString(db, 'EggItem')
		printString(db, 'AccessoryItem')
		printString(db, 'FoodItem')
		printID(db, 'FoodEffectiveness')
		printID(db, 'HungerDelay')

		if ('Intimacy' in db and (db['Intimacy']['Initial'] != 0 or db['Intimacy']['FeedIncrement'] != 0 or
			db['Intimacy']['OverFeedDecrement'] != 0 or db['Intimacy']['OwnerDeathDecrement'] != 0)):
			print('\tIntimacy: {')
			printID(db['Intimacy'], 'Initial', 2)
			printID(db['Intimacy'], 'FeedIncrement', 2)
			printID(db['Intimacy'], 'OverFeedDecrement', 2)
			printID(db['Intimacy'], 'OwnerDeathDecrement', 2)
			print('\t}')
		#
		printID(db, 'CaptureRate')
		printID(db, 'Speed')
		printBool(db, 'SpecialPerformance')
		printBool(db, 'TalkWithEmotes')
		printID(db, 'AttackRate')
		printID(db, 'DefendRate')
		printID(db, 'ChangeTargetRate')
		if (str(db['Id']) in autoFeedDB):
			print('\tAutoFeed: true')
		else:
			print('\tAutoFeed: false')
		printScript(db, 'PetScript')
		printScript(db, 'EquipScript')

		if (db['EggItem'] in EvolveDB):
			entry = EvolveDB[db['EggItem']]
			print('\tEvolve: {')

			for evolve in entry:
				if ('comment' in evolve):
					print('/*')
				print('\t\t{}: {'.format(evolve['Id']))

				for items in evolve['items']:
					print('\t\t\t{}: {}'.format(items[0], items[1]))

				print('\t\t}')
				if ('comment' in evolve):
					print('*/')

			print('\t}')
		print('},')

def saveEntry(EvolveDB, entry):
	if (entry['from'] not in EvolveDB):
		EvolveDB[entry['from']] = list()
	EvolveDB[entry['from']].append(entry)
	return EvolveDB

def getItemConstant(entry, ItemDB, itemID):
	if (itemID in ItemDB):
		return ItemDB[itemID]
	print(itemID, "not found", entry)
	entry['comment'] = 1
	return itemID

def ConvertDB(luaName, mode, serverpath):
	ItemDB = Tools.LoadDBConsts('item_db', mode, serverpath)
	f = open(luaName)
	content = f.read()
	f.close()

	recipeDB = re.findall(r'InsertEvolutionRecipeLGU\((\d+),\s*(\d+),\s*(\d+),\s*(\d+)\)', content)
	autoFeedDB = re.findall(r'InsertPetAutoFeeding\((\d+)\)', content)

	current = 0

	entry = dict()
	EvolveDB = dict()

	printHeader()
	for recipe in recipeDB:
		fromEgg = getItemConstant(entry, ItemDB, int(recipe[0]))
		petEgg = getItemConstant(entry, ItemDB, int(recipe[1]))

		if (current == 0):
			entry = {
				'Id': petEgg,
				'from': fromEgg,
				'items': list()
			}
			current = petEgg

		if (current != petEgg):
			EvolveDB = saveEntry(EvolveDB, entry)
			entry = {
				'Id': petEgg,
				'from': fromEgg,
				'items': list()
			}
			entry['id'] = petEgg
			entry['items'] = list()
			current = petEgg

		itemConst = getItemConstant(entry, ItemDB, int(recipe[2]))
		quantity = int(recipe[3])

		entry['items'].append((itemConst, quantity))
	saveEntry(EvolveDB, entry)

	printEntry(ItemDB, EvolveDB, autoFeedDB, entry, mode, serverpath)
	print(')')


if len(sys.argv) != 4:
	print('Pet Evolution Lua to DB')
	print('Usage:')
	print('    petevolutionconverter.py lua mode serverpath')
	print("example:")
	print('    petevolutionconverter.py PetEvolutionCln.lua pre-re ../')
	exit(1)

ConvertDB(sys.argv[1], sys.argv[2], sys.argv[3])
