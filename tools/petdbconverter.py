#! /usr/bin/env python
# -*- coding: utf8 -*-
#
# This file is part of Hercules.
# http://herc.ws - http://github.com/HerculesWS/Hercules
#
# Copyright (C) 2018-2022 Hercules Dev Team
# Copyright (C) 2018 Asheraf
# Copyright (C) 2015 Andrei Karas (4144)
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

import os
import re
import sys

def isValidEntry(line):
    if re.match('^[0-9]+,.*', line):
        return True
    return False

def curlSplit(line):
    return re.split('[{}]|},', line)

def commaSplit(line):
    return line.split(',')

def printIntField(name, value):
    if int(value) != 0:
        print('\t{0}: {1}'.format(name, value))

def printIntField2(name, value):
    if int(value) != 0:
        print('\t\t{0}: {1}'.format(name, value))

def printStrField(name, value):
    if value != '':
        print('\t{0}: \"{1}\"'.format(name, value))

def printBool(name, value):
    if int(value) != 0:
        print('\t{0}: true'.format(name))

def printIntimacy(arr):
    if int(arr[9]) == 0 or int(arr[10]) == 0 or int(arr[11]) == 0 or int(arr[12]) == 0:
        return
    print('\tIntimacy: {')
    printIntField2('Initial', arr[11])
    printIntField2('FeedIncrement', arr[9])
    printIntField2('OverFeedDecrement', arr[10])
    printIntField2('OwnerDeathDecrement', arr[12])
    print('\t}')

def printScript(name, value):
    if re.match('.*[a-zA-Z0-9,]+.*', value):
        print('\t{0}: <\"{1}\">'.format(name, value))

def printItemName(fieldname, itemid, itemDb):
    value = int(itemid)
    if value != 0:
        if value not in itemDb:
            print("// Error: pet item with id {0} not found in item_db.conf".format(value))
        else:
           printStrField(fieldname, itemDb[value])


def printHeader():
    print("""
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
    PetScript: <" Pet Script (can also be multi-line) ">
    EquipScript: <" Equip Script (can also be multi-line) ">
},
**************************************************************************/
    """)

def printFooter():
    print(')\n')

def convertFile(inFile, itemDb):
    if inFile != "" and not os.path.exists(inFile):
        return

    if inFile == "":
        r = sys.stdin
    else:
        r = open(inFile, "r")

    printHeader()
    for line in r:
        if isValidEntry(line) == True:
            print('{')
            firstsplit = curlSplit(line)
            secondsplit = commaSplit(firstsplit[0])
            printIntField('Id', secondsplit[0])
            printStrField('SpriteName', secondsplit[1])
            printStrField('Name', secondsplit[2])
            printItemName('TamingItem', secondsplit[3], itemDb)
            printItemName('EggItem', secondsplit[4], itemDb)
            printItemName('AccessoryItem', secondsplit[5], itemDb)
            printItemName('FoodItem', secondsplit[6], itemDb)
            printIntField('FoodEffectiveness', secondsplit[7])
            printIntField('HungerDelay', secondsplit[8])
            printIntimacy(secondsplit)
            printIntField('CaptureRate', secondsplit[13])
            printIntField('Speed', secondsplit[14])
            printBool('SpecialPerformance', secondsplit[15])
            printBool('TalkWithEmotes', secondsplit[16])
            printIntField('AttackRate', secondsplit[17])
            printIntField('DefendRate', secondsplit[18])
            printIntField('ChangeTargetRate', secondsplit[19])
            printScript('PetScript', firstsplit[1])
            printScript('EquipScript', firstsplit[3])
            print('},')
    printFooter()

def printHelp():
    print("PetDB converter from txt to conf format")
    print("Usage:")
    print("    petdbconverter.py re serverpath dbfilepath")
    print("    petdbconverter.py pre-re serverpath dbfilepath")
    print("Usage for read from stdin:")
    print("    petdbconverter.py re dbfilepath")

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
                    except ValueError:
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
