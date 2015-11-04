#! /usr/bin/env python
# -*- coding: utf8 -*-
#
# This file is part of Hercules.
# http://herc.ws - http://github.com/HerculesWS/Hercules
#
# Copyright (C) 2015  Hercules Dev Team
# Copyright (C) 2015  Andrei Karas (4144)
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

# This Script converts mob_db.txt to mob_db.conf format

import os
import re
import sys

comaSplit = re.compile(",")

def printHeader():
    print("""mob_db: (
//  Mobs Database
//
/******************************************************************************
 ************* Entry structure ************************************************
 ******************************************************************************
{
	// =================== Mandatory fields ===============================
	Id: ID                                (int)
	SpriteName: "SPRITE_NAME"             (string)
	Name: "Mob name"                      (string)
	// =================== Optional fields ================================
	Lv: level                             (int, defaults to 1)
	Hp: health                            (int, defaults to 1)
	Sp: mana                              (int, defaults to 0)
	Exp: basic experience                 (int, defaults to 0)
	JExp: job experience                  (int, defaults to 0)
	AttackRange: attack range             (int, defaults to 1)
	Attack: [attack1, attack2]            (int, defaults to 0)
	Def: defence                          (int, defaults to 0)
	Mdef: magic defence                   (int, defaults to 0)
	Stats: {
		Str: strength                 (int, defaults to 0)
		Agi: agility                  (int, defaults to 0)
		Vit: vitality                 (int, defaults to 0)
		Int: intelligence             (int, defaults to 0)
		Dex: dexterity                (int, defaults to 0)
		Luk: luck                     (int, defaults to 0)
	}
	ViewRange: view range                 (int, defaults to 1)
	ChaseRange: chase range               (int, defaults to 1)
	Size: size                            (int, defaults to 1)
	Race: race                            (int, defaults to 0)
	Element: (type, level)
	Mode: {
		CanMove: true/false           (bool)
		Looter: true/false            (bool)
		Aggressive: true/false        (bool)
		Assist: true/false            (bool)
		CastSensorIdle:true/false     (bool)
		Boss: true/false              (bool)
		Plant: true/false             (bool)
		CanAttack: true/false         (bool)
		Detector: true/false          (bool)
		CastSensorChase: true/false   (bool)
		ChangeChase: true/false       (bool)
		Angry: true/false             (bool)
		ChangeTargetMelee: true/false (bool)
		ChangeTargetChase: true/false (bool)
		TargetWeak: true/false        (bool)
	}
	MoveSpeed: move speed                 (int, defaults to 0)
	AttackDelay: attack delay             (int, defaults to 4000)
	AttackMotion: attack motion           (int, defaults to 2000)
	DamageMotion: damage motion           (int, defaults to 0)
	MvpExp: mvp experience                (int, defaults to 0)
	MvpDrops: {
		AegisName: chance             (string: int)
		...
	}
	Drops: {
		AegisName: chance         (string: int)
		...
	}

},
******************************************************************************/

""")

def printFooter():
    print(")")

def printField(name, value):
    print("\t{0}: {1}".format(name, value))

def printField2(name, value):
    print("\t\t{0}: {1}".format(name, value))

def printFieldCond2(cond, name):
    if cond != 0:
        print("\t\t{0}: true".format(name))

def printFieldArr(name, value, value2):
    print("\t{0}: [{1}, {2}]".format(name, value, value2))

def printFieldStr(name, value):
    print("\t{0}: \"{1}\"".format(name, value))

def startGroup(name):
    print("\t{0}: {{".format(name))

def endGroup():
    print("\t}")

def printHelp():
    print("MobDB converter from txt to conf format")
    print("Usage:")
    print("    mobdbconverter.py re serverpath dbfilepath")
    print("    mobdbconverter.py pre-re serverpath dbfilepath")
    print("Usage for read from stdin:")
    print("    mobdbconverter.py re dbfilepath")

def isHaveData(fields, start, cnt):
    for f in range(0, cnt):
        value = fields[start + f * 2]
        chance = fields[start + f * 2]
        if value == "" or value == "0" or chance == "" or chance == "0":
            continue
        return True
    return False

def convertFile(inFile, itemDb):
    if inFile != "" and not os.path.exists(inFile):
        return

    if inFile == "":
        r = sys.stdin
    else:
        r = open(inFile, "r")

    printHeader()
    for line in r:
        if line.strip() == "":
            continue
        if len(line) < 5 or line[:2] == "//":
            print(line)
            continue
        fields = comaSplit.split(line)
        if len(fields) != 57:
            print(line)
            continue
        for f in range(0, len(fields)):
            fields[f] = fields[f].strip()
        print("{")
        printField("Id", fields[0])
        printFieldStr("SpriteName", fields[1])
        printFieldStr("Name", fields[2])
        printField("Lv", fields[4])
        printField("Hp", fields[5])
        printField("Sp", fields[6])
        printField("Exp", fields[7])
        printField("JExp", fields[8])
        printField("AttackRange", fields[9])
        printFieldArr("Attack", fields[10], fields[11])
        printField("Def", fields[12])
        printField("Mdef", fields[13])
        startGroup("Stats")
        printField2("Str", fields[14])
        printField2("Agi", fields[15])
        printField2("Vit", fields[16])
        printField2("Int", fields[17])
        printField2("Dex", fields[18])
        printField2("Luk", fields[19])
        endGroup()
        printField("ViewRange", fields[20])
        printField("ChaseRange", fields[21])
        printField("Size", fields[22])
        printField("Race", fields[23])
        print("\tElement: ({0}, {1})".format(int(fields[24]) % 10, int(fields[24]) / 20));
        mode = int(fields[25], 0)
        if mode != 0:
            startGroup("Mode")
            printFieldCond2(mode & 0x0001, "CanMove")
            printFieldCond2(mode & 0x0002, "Looter")
            printFieldCond2(mode & 0x0004, "Aggressive")
            printFieldCond2(mode & 0x0008, "Assist")
            printFieldCond2(mode & 0x0010, "CastSensorIdle")
            printFieldCond2(mode & 0x0020, "Boss")
            printFieldCond2(mode & 0x0040, "Plant")
            printFieldCond2(mode & 0x0080, "CanAttack")
            printFieldCond2(mode & 0x0100, "Detector")
            printFieldCond2(mode & 0x0200, "CastSensorChase")
            printFieldCond2(mode & 0x0400, "ChangeChase")
            printFieldCond2(mode & 0x0800, "Angry")
            printFieldCond2(mode & 0x1000, "ChangeTargetMelee")
            printFieldCond2(mode & 0x2000, "ChangeTargetChase")
            printFieldCond2(mode & 0x4000, "TargetWeak")
            printFieldCond2(mode & 0x8000, "LiveWithoutMaster")
            endGroup()
        printField("MoveSpeed", fields[26])
        printField("AttackDelay", fields[27])
        printField("AttackMotion", fields[28])
        printField("DamageMotion", fields[29])
        printField("MvpExp", fields[30])
        if isHaveData(fields, 31, 3):
            startGroup("MvpDrops")
            for f in range(0, 3):
                value = fields[31 + f * 2]
                chance = fields[32 + f * 2]
                if value == "" or value == "0" or chance == "" or chance == "0":
                    continue
                value = int(value)
                if value not in itemDb:
                    print("// Error: mvp drop with id {0} not found in item_db.conf".format(value))
                else:
                    printField2(itemDb[value], chance)
            endGroup()
        if isHaveData(fields, 37, 10):
            startGroup("Drops")
            for f in range(0, 10):
                value = fields[37 + f * 2]
                chance = fields[38 + f * 2]
                if value == "" or value == "0" or chance == "" or chance == "0":
                    continue
                value = int(value)
                if value not in itemDb:
                    print("// Error: drop with id {0} not found in item_db.conf".format(value))
                else:
                    printField2(itemDb[value], chance)
            endGroup()
        print("},")
    printFooter()
    if inFile != "":
        r.close()

def readItemDB(inFile, itemDb):
    itemId = 0
    itemName = ""
    started = False
    with open(inFile, "r") as r:
        for line in r:
            line = line.strip()
            if started == True:
                if line == "},":
                    started = False
                elif line[:10] == "AegisName:":
                    itemName = line[12:-1]
                elif line[:3] == "Id:":
                    try:
                        itemId = int(line[4:])
                    except:
                        started = False
                if itemId != 0 and itemName != "":
# was need for remove wrong characters
#                    itemName = itemName.replace(".", "")
#                    if itemName[0] >= "0" and itemName[0] <= "9":
#                        itemName = "Num" + itemName
                    itemDb[itemId] = itemName
                    started = False
            else:
                if line == "{":
                    started = True
                    itemId = 0
                    itemName = ""
    return itemDb

if len(sys.argv) != 4 and len(sys.argv) != 3:
    printHelp();
    exit(1)
startPath = sys.argv[2]
if len(sys.argv) == 4:
    sourceFile = sys.argv[3]
else:
    sourceFile = "";

itemDb = dict()
if sys.argv[1] == "re":
    itemDb = readItemDB(startPath + "/db/re/item_db.conf", itemDb)
    itemDb = readItemDB(startPath + "/db/item_db2.conf", itemDb)
elif sys.argv[1] == "pre-re":
    itemDb = readItemDB(startPath + "/db/pre-re/item_db.conf", itemDb)
    itemDb = readItemDB(startPath + "/db/item_db2.conf", itemDb)
else:
    printHelp();
    exit(1)

convertFile(sourceFile, itemDb)
