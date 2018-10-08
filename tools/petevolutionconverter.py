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
import utils.common as tools


def print_header():
    print('''
//================= Hercules Database =====================================
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


def print_id(db, name, tab_size=1):
    if name not in db or int(db[name]) == 0:
        return
    print('{}{}: {}'.format('\t' * tab_size, name, db[name]))


def print_string(db, name, tab_size=1):
    if name not in db or db[name].strip() == "":
        return
    print('{}{}: "{}"'.format('\t' * tab_size, name, db[name]))


def print_bool(db, name):
    if name not in db or db[name] == '0':
        return
    print('\t{}: true'.format(name))


def print_script(db, name):
    if name not in db or db[name].strip() == "":
        return
    print('\t{}: <{}>'.format(name, db[name]))


# item_db, entry variables are unused.
# should we remove them?
def print_entry(item_db, evolve_db, auto_feed_db, entry, mode, serverpath):
    pet_db = tools.load_db('pet_db', mode, serverpath)

    for i, db in enumerate(pet_db):
        print('{')
        print_id(db, 'Id')
        print_string(db, 'SpriteName')
        print_string(db, 'Name')

        print_string(db, 'TamingItem')
        print_string(db, 'EggItem')
        print_string(db, 'AccessoryItem')
        print_string(db, 'FoodItem')
        print_id(db, 'FoodEffectiveness')
        print_id(db, 'HungerDelay')

        if ('Intimacy' in db and (db['Intimacy']['Initial'] != 0 or db['Intimacy']['FeedIncrement'] != 0 or
                                  db['Intimacy']['OverFeedDecrement'] != 0 or db['Intimacy'][
                                      'OwnerDeathDecrement'] != 0)):
            print('\tIntimacy: {')
            print_id(db['Intimacy'], 'Initial', 2)
            print_id(db['Intimacy'], 'FeedIncrement', 2)
            print_id(db['Intimacy'], 'OverFeedDecrement', 2)
            print_id(db['Intimacy'], 'OwnerDeathDecrement', 2)
            print('\t}')
        #
        print_id(db, 'CaptureRate')
        print_id(db, 'Speed')
        print_bool(db, 'SpecialPerformance')
        print_bool(db, 'TalkWithEmotes')
        print_id(db, 'AttackRate')
        print_id(db, 'DefendRate')
        print_id(db, 'ChangeTargetRate')
        if str(db['Id']) in auto_feed_db:
            print('\tAutoFeed: true')
        else:
            print('\tAutoFeed: false')
        print_script(db, 'PetScript')
        print_script(db, 'EquipScript')

        if db['EggItem'] in evolve_db:
            entry = evolve_db[db['EggItem']]
            print('\tEvolve: {')

            for evolve in entry:
                if 'comment' in evolve:
                    print('/*')
                print('\t\t{}: {'.format(evolve['Id']))

                for items in evolve['items']:
                    print('\t\t\t{}: {}'.format(items[0], items[1]))

                print('\t\t}')
                if 'comment' in evolve:
                    print('*/')

            print('\t}')
        print('},')


def save_entry(evolve_db, entry):
    if entry['from'] not in evolve_db:
        evolve_db[entry['from']] = list()
    evolve_db[entry['from']].append(entry)
    return evolve_db


def get_item_constant(entry, item_db, item_id):
    if item_id in item_db:
        return item_db[item_id]
    print(item_id, "not found", entry)
    entry['comment'] = 1
    return item_id


def convert_db(lua_name, mode, serverpath):
    item_db = tools.load_db_consts('item_db', mode, serverpath)
    f = open(lua_name)
    content = f.read()
    f.close()

    recipe_db = re.findall(r'InsertEvolutionRecipeLGU\((\d+),\s*(\d+),\s*(\d+),\s*(\d+)\)', content)
    auto_feed_db = re.findall(r'InsertPetAutoFeeding\((\d+)\)', content)

    current = 0

    entry = dict()
    evolve_db = dict()

    print_header()
    for recipe in recipe_db:
        from_egg = get_item_constant(entry, item_db, int(recipe[0]))
        pet_egg = get_item_constant(entry, item_db, int(recipe[1]))

        if current == 0:
            entry = {
                'Id': pet_egg,
                'from': from_egg,
                'items': list()
            }
            current = pet_egg

        if current != pet_egg:
            evolve_db = save_entry(evolve_db, entry)
            entry = {
                'Id': pet_egg,
                'from': from_egg,
                'items': list(),
                'id': pet_egg
            }
            current = pet_egg

        item_const = get_item_constant(entry, item_db, int(recipe[2]))
        quantity = int(recipe[3])

        entry['items'].append((item_const, quantity))
    save_entry(evolve_db, entry)

    print_entry(item_db, evolve_db, auto_feed_db, entry, mode, serverpath)
    print(')')


if len(sys.argv) != 4:
    print('Pet Evolution Lua to DB')
    print('Usage:')
    print('    petevolutionconverter.py lua mode serverpath')
    print("example:")
    print('    petevolutionconverter.py PetEvolutionCln.lua pre-re ../')
    exit(1)

convert_db(sys.argv[1], sys.argv[2], sys.argv[3])
