#! /usr/bin/env python
# -*- coding: utf8 -*-
#
# This file is part of Hercules.
# http://herc.ws - http://github.com/HerculesWS/Hercules
#
# Copyright (C) 2019  Hercules Dev Team
# Copyright (C) 2019  Asheraf
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

import re
import sys
import utils.common as Tools

Races = {
	0: "RC_Formless",
	1: "RC_Undead",
	2: "RC_Brute",
	3: "RC_Plant",
	4: "RC_Insect",
	5: "RC_Fish",
	6: "RC_Demon",
	7: "RC_DemiHuman",
	8: "RC_Angel",
	9: "RC_Dragon",
	10: "RC_Player",
	11: "RC_Boss",
	12: "RC_NonBoss",
	14: "RC_NonDemiHuman",
	15: "RC_NonPlayer",
	16: "RC_DemiPlayer",
	17: "RC_NonDemiPlayer",
	255: "RC_All"
}

Elements = {
	0: "Ele_Neutral",
	1: "Ele_Water",
	2: "Ele_Earth",
	3: "Ele_Fire",
	4: "Ele_Wind",
	5: "Ele_Poison",
	6: "Ele_Holy",
	7: "Ele_Dark",
	8: "Ele_Ghost",
	9: "Ele_Undead"
}

Sizes = {
	0: "Size_Small",
	1: "Size_Medium",
	2: "Size_Large"
}

def isValidEntry(line):
    if re.match('^[0-9]+,.*', line):
        return True
    return False

def commaSplit(line):
    return line.split(',')

def printIntField(name, value, indentation = 1):
    if int(value) != 0:
        print('{0}{1}: {2}'.format('\t' * indentation, name, value))

def printStrField(name, value, indentation = 1):
    if value != '':
        print('{0}{1}: \"{2}\"'.format('\t' * indentation, name, value))

def printBool(name, value, indentation = 1):
    if int(value) != 0:
        print('{0}{1}: true'.format('\t' * indentation, name))

def printRaceField(name, value, indentation = 1):
    if int(value) not in Races:
	    print("Error: homunculus invalid race id {0}.".format(value))
	    exit()
    else:
        print('{0}{1}: "{2}"'.format('\t' * indentation, name, Races[int(value)]))

def printElementField(name, value, indentation = 1):
    if int(value) not in Elements:
	    print("Error: homunculus invalid element id {0}.".format(value))
	    exit()
    else:
        print('{0}{1}: "{2}"'.format('\t' * indentation, name, Elements[int(value)]))

def printSizeField(name, value, indentation = 1):
    if int(value) not in Sizes:
	    print("Error: homunculus invalid size id {0}.".format(value))
	    exit()
    else:
        print('{0}{1}: "{2}"'.format('\t' * indentation, name, Sizes[int(value)]))

def printIntMinMaxField(name, minval, maxval, indentation = 1):
	print('{0}{1}: ({2}, {3})'.format('\t' * indentation, name, minval, maxval))

def printItemName(name, itemid, itemdb, indentation = 1):
	if itemid not in itemdb:
	    print("Error: homunculus food item with id {0} not found in item_db.conf".format(value))
	    exit()
	else:
	   printStrField(name, itemdb[itemid], indentation)

def printHeader():
	print('''homunculus_db: (
/**************************************************************************
 ************* Entry structure ********************************************
 **************************************************************************
{
	// ================ Mandatory fields ==============================
	Id: ID                                (int)
	EvoId: ID of the evolve target        (int)
	Name: "Homunculus name"               (string)
	FoodItem: Food Id                     (string)
	// ================ Optional fields ===============================
	HungryDelay: delay                    (int, defaults to 0)
	Size: size                            (string, defaults to "Size_Small")
	EvoSize: size                         (string, defaults to "Size_Small")
	Race: race                            (string, defaults to "RC_Formless")
	Element: type                         (string, defaults to "Ele_Neutral" and level is always 1)
	Aspd: base aspd                       (int, defaults to 0)
	BaseStats: {
		Hp:   val                         (int, defaults to 0)
		Sp:   val                         (int, defaults to 0)
		Str:  val                         (int, defaults to 0)
		Agi:  val                         (int, defaults to 0)
		Vit:  val                         (int, defaults to 0)
		Int:  val                         (int, defaults to 0)
		Dex:  val                         (int, defaults to 0)
		Luk:  val                         (int, defaults to 0)
	}
	GrowthStats: {
		Hp:  (min, max)                   (int, defaults to 0)
		Sp:  (min, max)                   (int, defaults to 0)
		Str: (min, max)                   (int, defaults to 0)
		Agi: (min, max)                   (int, defaults to 0)
		Vit: (min, max)                   (int, defaults to 0)
		Int: (min, max)                   (int, defaults to 0)
		Dex: (min, max)                   (int, defaults to 0)
		Luk: (min, max)                   (int, defaults to 0)
	}
	EvolutionStats: {
		Hp:  (min, max)                   (int, defaults to 0)
		Sp:  (min, max)                   (int, defaults to 0)
		Str: (min, max)                   (int, defaults to 0)
		Agi: (min, max)                   (int, defaults to 0)
		Vit: (min, max)                   (int, defaults to 0)
		Int: (min, max)                   (int, defaults to 0)
		Dex: (min, max)                   (int, defaults to 0)
		Luk: (min, max)                   (int, defaults to 0)
	}
},
**************************************************************************/''')

def printFooter():
    print(')\n')


def ConvertDB(mode, serverpath):

	r = open('{}db/{}/homunculus_db.txt'.format(serverpath, mode), "r")

	ItemDB = Tools.LoadDBConsts('item_db', mode, serverpath)
	printHeader()
	for line in r:
		if isValidEntry(line) == True:
			print('{')
			split = commaSplit(line)
			printIntField('Id', split[0])
			printIntField('EvoId', split[1])
			printStrField('Name', split[2])
			printItemName('FoodItem', int(split[3]), ItemDB)
			printIntField('HungryDelay', split[4])
			printSizeField('Size', split[5])
			printSizeField('EvoSize', split[6])
			printRaceField('Race', split[7])
			printElementField('Element', split[8])
			printIntField('Aspd', int(split[9]))
			print('\tBaseStats: {')
			printIntField('Hp',  int(split[10]), 2)
			printIntField('Sp',  int(split[11]), 2)
			printIntField('Str', int(split[12]), 2)
			printIntField('Agi', int(split[13]), 2)
			printIntField('Vit', int(split[14]), 2)
			printIntField('Int', int(split[15]), 2)
			printIntField('Dex', int(split[16]), 2)
			printIntField('Luk', int(split[17]), 2)
			print('\t}')
			print('\tGrowthStats: {')
			printIntMinMaxField('Hp',  int(split[18]), int(split[19]), 2)
			printIntMinMaxField('Sp',  int(split[20]), int(split[21]), 2)
			printIntMinMaxField('Str', int(split[22]), int(split[23]), 2)
			printIntMinMaxField('Agi', int(split[24]), int(split[25]), 2)
			printIntMinMaxField('Vit', int(split[26]), int(split[27]), 2)
			printIntMinMaxField('Int', int(split[28]), int(split[29]), 2)
			printIntMinMaxField('Dex', int(split[30]), int(split[31]), 2)
			printIntMinMaxField('Luk', int(split[32]), int(split[33]), 2)
			print('\t}')
			print('\tEvolutionStats: {')
			printIntMinMaxField('Hp',  int(split[34]), int(split[35]), 2)
			printIntMinMaxField('Sp',  int(split[36]), int(split[37]), 2)
			printIntMinMaxField('Str', int(split[38]), int(split[39]), 2)
			printIntMinMaxField('Agi', int(split[40]), int(split[41]), 2)
			printIntMinMaxField('Vit', int(split[42]), int(split[43]), 2)
			printIntMinMaxField('Int', int(split[44]), int(split[45]), 2)
			printIntMinMaxField('Dex', int(split[46]), int(split[47]), 2)
			printIntMinMaxField('Luk', int(split[48]), int(split[49]), 2)
			print('\t}')
			print('},')
	printFooter()

if len(sys.argv) != 3:
	print('Homunculus db converter from txt to conf format')
	print('Usage:')
	print('    homundbconverter.py mode serverpath')
	print("example:")
	print('    homundbconverter.py pre-re ../')
	exit(1)

if sys.argv[1] != 're' and sys.argv[1] != 'pre-re':
	print('you have entred an invalid server mode')
	exit(1)

ConvertDB(sys.argv[1], sys.argv[2])
