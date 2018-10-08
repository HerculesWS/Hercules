#! /usr/bin/env python
# -*- coding: utf8 -*-
#
# This file is part of Hercules.
# http://herc.ws - http://github.com/HerculesWS/Hercules
#
# Copyright (C) 2018  Hercules Dev Team
# Copyright (C) 2018  Asheraf
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

import os
import re
import sys


def is_valid_entry(line):
    if re.match('^[0-9]+,.*', line):
        return True
    return False


def curl_split(line):
    return re.split('[{}]|},', line)


def comma_split(line):
    return line.split(',')


def print_int_field(name, value):
    if int(value) != 0:
        print('\t{0}: {1}'.format(name, value))


def print_int_field2(name, value):
    if int(value) != 0:
        print('\t\t{0}: {1}'.format(name, value))


def print_str_field(name, value):
    if value != '':
        print('\t{0}: \"{1}\"'.format(name, value))


def print_bool(name, value):
    if int(value) != 0:
        print('\t{0}: true'.format(name))


def print_intimacy(arr):
    if int(arr[9]) == 0 or int(arr[10]) == 0 or int(arr[11]) == 0 or int(arr[12]) == 0:
        return
    print('\tIntimacy: {')
    print_int_field2('Initial', arr[11])
    print_int_field2('FeedIncrement', arr[9])
    print_int_field2('OverFeedDecrement', arr[10])
    print_int_field2('OwnerDeathDecrement', arr[12])
    print('\t}')


def print_script(name, value):
    if re.match('.*[a-zA-Z0-9,]+.*', value):
        print('\t{0}: <\"{1}\">'.format(name, value))


def print_item_name(fieldname, itemid, item_db):
    value = int(itemid)
    if value != 0:
        if value not in item_db:
            print("// Error: pet item with id {0} not found in item_db.conf".format(value))
        else:
            print_str_field(fieldname, item_db[value])


def print_header():
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


def print_footer():
    print(')\n')


def convert_file(in_file, item_db):
    if in_file != "" and not os.path.exists(in_file):
        return

    if in_file == "":
        r = sys.stdin
    else:
        r = open(in_file, "r")

    print_header()
    for line in r:
        if is_valid_entry(line):
            print('{')
            firstsplit = curl_split(line)
            secondsplit = comma_split(firstsplit[0])
            print_int_field('Id', secondsplit[0])
            print_str_field('SpriteName', secondsplit[1])
            print_str_field('Name', secondsplit[2])
            print_item_name('TamingItem', secondsplit[3], item_db)
            print_item_name('EggItem', secondsplit[4], item_db)
            print_item_name('AccessoryItem', secondsplit[5], item_db)
            print_item_name('FoodItem', secondsplit[6], item_db)
            print_int_field('FoodEffectiveness', secondsplit[7])
            print_int_field('HungerDelay', secondsplit[8])
            print_intimacy(secondsplit)
            print_int_field('CaptureRate', secondsplit[13])
            print_int_field('Speed', secondsplit[14])
            print_bool('SpecialPerformance', secondsplit[15])
            print_bool('TalkWithEmotes', secondsplit[16])
            print_int_field('AttackRate', secondsplit[17])
            print_int_field('DefendRate', secondsplit[18])
            print_int_field('ChangeTargetRate', secondsplit[19])
            print_script('PetScript', firstsplit[1])
            print_script('EquipScript', firstsplit[3])
            print('},')
    print_footer()


def print_help():
    print("PetDB converter from txt to conf format")
    print("Usage:")
    print("    petdbconverter.py re serverpath dbfilepath")
    print("    petdbconverter.py pre-re serverpath dbfilepath")
    print("Usage for read from stdin:")
    print("    petdbconverter.py re dbfilepath")


def read_item_db(in_file, item_db):
    item_id = 0
    item_name = ""
    started = False
    with open(in_file, "r") as r:
        for line in r:
            line = line.strip()
            if started:
                if line == "},":
                    started = False
                elif line[:10] == "AegisName:":
                    item_name = line[12:-1]
                elif line[:3] == "Id:":
                    try:
                        item_id = int(line[4:])
                    except ValueError:
                        started = False
                if item_id != 0 and item_name != "":
                    # was need for remove wrong characters
                    #                    item_name = item_name.replace(".", "")
                    #                    if item_name[0] >= "0" and item_name[0] <= "9":
                    #                        item_name = "Num" + item_name
                    item_db[item_id] = item_name
                    started = False
            else:
                if line == "{":
                    started = True
                    item_id = 0
                    item_name = ""
    return item_db


if len(sys.argv) != 4 and len(sys.argv) != 3:
    print_help()
    exit(1)
startPath = sys.argv[2]
if len(sys.argv) == 4:
    sourceFile = sys.argv[3]
else:
    sourceFile = ""

itemDb = dict()
if sys.argv[1] == "re":
    itemDb = read_item_db(startPath + "/db/re/item_db.conf", itemDb)
    itemDb = read_item_db(startPath + "/db/item_db2.conf", itemDb)
elif sys.argv[1] == "pre-re":
    itemDb = read_item_db(startPath + "/db/pre-re/item_db.conf", itemDb)
    itemDb = read_item_db(startPath + "/db/item_db2.conf", itemDb)
else:
    print_help()
    exit(1)

convert_file(sourceFile, itemDb)
