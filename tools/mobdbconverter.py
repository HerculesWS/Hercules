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


def print_header():
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


def print_footer():
    print(")")


def print_field(name, value):
    print("\t{0}: {1}".format(name, value))


def print_field2(name, value):
    print("\t\t{0}: {1}".format(name, value))


def print_field_cond2(cond, name):
    if cond != 0:
        print("\t\t{0}: true".format(name))


def print_field_arr(name, value, value2):
    print("\t{0}: [{1}, {2}]".format(name, value, value2))


def print_field_str(name, value):
    print("\t{0}: \"{1}\"".format(name, value))


def start_group(name):
    print("\t{0}: {{".format(name))


def end_group():
    print("\t}")


def print_help():
    print("MobDB converter from txt to conf format")
    print("Usage:")
    print("    mobdbconverter.py re serverpath dbfilepath")
    print("    mobdbconverter.py pre-re serverpath dbfilepath")
    print("Usage for read from stdin:")
    print("    mobdbconverter.py re dbfilepath")


def is_have_data(fields, start, cnt):
    for f in range(0, cnt):
        value = fields[start + f * 2]
        chance = fields[start + f * 2]
        if value == "" or value == "0" or chance == "" or chance == "0":
            continue
        return True
    return False


def convert_file(in_file, item_db):
    if in_file != "" and not os.path.exists(in_file):
        return

    if in_file == "":
        r = sys.stdin
    else:
        r = open(in_file, "r")

    print_header()
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
        print_field("Id", fields[0])
        print_field_str("SpriteName", fields[1])
        print_field_str("Name", fields[2])
        print_field("Lv", fields[4])
        print_field("Hp", fields[5])
        print_field("Sp", fields[6])
        print_field("Exp", fields[7])
        print_field("JExp", fields[8])
        print_field("AttackRange", fields[9])
        print_field_arr("Attack", fields[10], fields[11])
        print_field("Def", fields[12])
        print_field("Mdef", fields[13])
        start_group("Stats")
        print_field2("Str", fields[14])
        print_field2("Agi", fields[15])
        print_field2("Vit", fields[16])
        print_field2("Int", fields[17])
        print_field2("Dex", fields[18])
        print_field2("Luk", fields[19])
        end_group()
        print_field("ViewRange", fields[20])
        print_field("ChaseRange", fields[21])
        print_field("Size", fields[22])
        print_field("Race", fields[23])
        print("\tElement: ({0}, {1})".format(int(fields[24]) % 10, int(fields[24]) / 20))
        mode = int(fields[25], 0)
        if mode != 0:
            start_group("Mode")
            print_field_cond2(mode & 0x0001, "CanMove")
            print_field_cond2(mode & 0x0002, "Looter")
            print_field_cond2(mode & 0x0004, "Aggressive")
            print_field_cond2(mode & 0x0008, "Assist")
            print_field_cond2(mode & 0x0010, "CastSensorIdle")
            print_field_cond2(mode & 0x0020, "Boss")
            print_field_cond2(mode & 0x0040, "Plant")
            print_field_cond2(mode & 0x0080, "CanAttack")
            print_field_cond2(mode & 0x0100, "Detector")
            print_field_cond2(mode & 0x0200, "CastSensorChase")
            print_field_cond2(mode & 0x0400, "ChangeChase")
            print_field_cond2(mode & 0x0800, "Angry")
            print_field_cond2(mode & 0x1000, "ChangeTargetMelee")
            print_field_cond2(mode & 0x2000, "ChangeTargetChase")
            print_field_cond2(mode & 0x4000, "TargetWeak")
            print_field_cond2(mode & 0x8000, "LiveWithoutMaster")
            end_group()
        print_field("MoveSpeed", fields[26])
        print_field("AttackDelay", fields[27])
        print_field("AttackMotion", fields[28])
        print_field("DamageMotion", fields[29])
        print_field("MvpExp", fields[30])
        if is_have_data(fields, 31, 3):
            start_group("MvpDrops")
            for f in range(0, 3):
                value = fields[31 + f * 2]
                chance = fields[32 + f * 2]
                if value == "" or value == "0" or chance == "" or chance == "0":
                    continue
                value = int(value)
                if value not in item_db:
                    print("// Error: mvp drop with id {0} not found in item_db.conf".format(value))
                else:
                    print_field2(item_db[value], chance)
            end_group()
        if is_have_data(fields, 37, 10):
            start_group("Drops")
            for f in range(0, 10):
                value = fields[37 + f * 2]
                chance = fields[38 + f * 2]
                if value == "" or value == "0" or chance == "" or chance == "0":
                    continue
                value = int(value)
                if value not in item_db:
                    print("// Error: drop with id {0} not found in item_db.conf".format(value))
                else:
                    print_field2(item_db[value], chance)
            end_group()
        print("},")
    print_footer()
    if in_file != "":
        r.close()


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
