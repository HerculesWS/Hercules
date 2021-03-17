#!/usr/bin/python
# -*- coding: utf8 -*-
#
# This file is part of Hercules.
# http://herc.ws - http://github.com/HerculesWS/Hercules
#
# Copyright (C) 2018-2021 Hercules Dev Team
# Copyright (C) 2018 Asheraf
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

SKILL_STATES = {
	"any":       "MSS_ANY",
	"idle":      "MSS_IDLE",
	"walk":      "MSS_WALK",
	"loot":      "MSS_LOOT",
	"dead":      "MSS_DEAD",
	"attack":    "MSS_BERSERK",
	"angry":     "MSS_ANGRY",
	"chase":     "MSS_RUSH",
	"follow":    "MSS_FOLLOW",
	"anytarget": "MSS_ANYTARGET"
}
SKILL_COND1 = {
	"always":            "MSC_ALWAYS",
	"myhpltmaxrate":     "MSC_MYHPLTMAXRATE",
	"myhpinrate":        "MSC_MYHPINRATE",
	"friendhpltmaxrate": "MSC_FRIENDHPLTMAXRATE",
	"friendhpinrate":    "MSC_FRIENDHPINRATE",
	"mystatuson":        "MSC_MYSTATUSON",
	"mystatusoff":       "MSC_MYSTATUSOFF",
	"friendstatuson":    "MSC_FRIENDSTATUSON",
	"friendstatusoff":   "MSC_FRIENDSTATUSOFF",
	"attackpcgt":        "MSC_ATTACKPCGT",
	"attackpcge":        "MSC_ATTACKPCGE",
	"slavelt":           "MSC_SLAVELT",
	"slavele":           "MSC_SLAVELE",
	"closedattacked":    "MSC_CLOSEDATTACKED",
	"longrangeattacked": "MSC_LONGRANGEATTACKED",
	"skillused":         "MSC_SKILLUSED",
	"afterskill":        "MSC_AFTERSKILL",
	"casttargeted":      "MSC_CASTTARGETED",
	"rudeattacked":      "MSC_RUDEATTACKED",
	"masterhpltmaxrate": "MSC_MASTERHPLTMAXRATE",
	"masterattacked":    "MSC_MASTERATTACKED",
	"alchemist":         "MSC_ALCHEMIST",
	"onspawn":           "MSC_SPAWN"
}
SKILL_COND2 = {
	"anybad":    "MSC_ANY",
	"stone":     "SC_STONE",
	"freeze":    "SC_FREEZE",
	"stun":      "SC_STUN",
	"sleep":     "SC_SLEEP",
	"poison":    "SC_POISON",
	"curse":     "SC_CURSE",
	"silence":   "SC_SILENCE",
	"confusion": "SC_CONFUSION",
	"blind":     "SC_BLIND",
	"hiding":    "SC_HIDING",
	"sight":     "SC_SIGHT"
}
SKILL_TARGET = {
	"target":       "MST_TARGET",
	"randomtarget": "MST_RANDOM",
	"self":         "MST_SELF",
	"friend":       "MST_FRIEND",
	"master":       "MST_MASTER",
	"around5":      "MST_AROUND5",
	"around6":      "MST_AROUND6",
	"around7":      "MST_AROUND7",
	"around8":      "MST_AROUND8",
	"around1":      "MST_AROUND1",
	"around2":      "MST_AROUND2",
	"around3":      "MST_AROUND3",
	"around4":      "MST_AROUND4",
	"around":       "MST_AROUND"
}

def printHeader():
    print("""
mob_skill_db:(
{
/**************************************************************************
 ************* Entry structure ********************************************
 **************************************************************************
	<Monster_Constant>: {
		<Skill_Constant>: {
			ClearSkills:   (boolean, defaults to false) allows cleaning all previous defined skills for the mob.
			SkillLevel:    (int, defaults to 1)
			SkillState:    (int, defaults to 0)
			SkillTarget:   (int, defaults to 0)
			Rate:          (int, defaults to 1)
			CastTime:      (int, defaults to 0)
			Delay:         (int, defaults to 0)
			Cancelable:    (boolean, defaults to false)
			CastCondition: (int, defaults to 0)
			ConditionData: (int, defaults to 0)
			val0:          (int, defaults to 0)
			val1:          (int, defaults to 0)
			val2:          (int, defaults to 0)
			val3:          (int, defaults to 0)
			val4:          (int, defaults to 0)
			Emotion:       (int, defaults to 0)
			ChatMsgID:     (int, defaults to 0)
		}
	}
**************************************************************************/""")

def printFooter():
    print('}\n)\n')

def isValidEntry(line):
    if re.match('^[0-9]+,.*', line):
        return True
    return False

def commaSplit(line):
    return line.split(',')

def stripLinebreak(line):
    return line.replace('\r', '').replace('\n', '')

def printInt(key, value):
	if key in value:
		if int(value[key]) is not 0:
			print('\t\t\t{}: {}'.format(key, value[key]))

def printStrToInt(key, value):
	if value[key] is not '':
		if int(value[key]) is not 0:
			print('\t\t\t{}: {}'.format(key, value[key]))

def printBool(key, value):
	if value[key] == 'yes':
		print('\t\t\t{}: true'.format(key))

def printClearSkills(key, value):
	if value[key] == 'clear':
		print('\t\t\t{}: true'.format(key))

def printSkillState(key, value):
	if value[key]:
		print('\t\t\t{}: "{}"'.format(key, SKILL_STATES[value[key]]))

def printSkillTarget(key, value):
	if value[key]:
		print('\t\t\t{}: "{}"'.format(key, SKILL_TARGET[value[key]]))

def printCastCondition(key, value):
	if value[key]:
		print('\t\t\t{}: "{}"'.format(key, SKILL_COND1[value[key]]))

def printConditionData(key, value):
	if value[key] in SKILL_COND2:
		print('\t\t\t{}: "{}"'.format(key, SKILL_COND2[value[key]]))
	elif value[key] is not '':
		if int(value[key]) is not 0:
			print('\t\t\t{}: {}'.format(key, value[key]))

def printEmotion(key, value):
	if value[key] is not '':
		print('\t\t\t{}: {}'.format(key, value[key]))

def LoadOldDB(mode, serverpath):

	r = open('{}db/{}/mob_skill_db.txt'.format(serverpath, mode), "r")

	Db = dict()
	for line in r:
		if isValidEntry(line) == True:
			entry = commaSplit(stripLinebreak(line))
			MonsterId = entry[0]
			if MonsterId not in Db:
				Db[MonsterId] = dict()
			skillidx = len(Db[MonsterId])
			Db[MonsterId][skillidx] = dict()
			Db[MonsterId][skillidx]['ClearSkills'] = entry[1]
			Db[MonsterId][skillidx]['SkillState'] = entry[2]
			Db[MonsterId][skillidx]['SkillId'] = entry[3]
			Db[MonsterId][skillidx]['SkillLevel'] = entry[4]
			Db[MonsterId][skillidx]['Rate'] = entry[5]
			Db[MonsterId][skillidx]['CastTime'] = entry[6]
			Db[MonsterId][skillidx]['Delay'] = entry[7]
			Db[MonsterId][skillidx]['Cancelable'] = entry[8]
			Db[MonsterId][skillidx]['SkillTarget'] = entry[9]
			Db[MonsterId][skillidx]['CastCondition'] = entry[10]
			Db[MonsterId][skillidx]['ConditionData'] = entry[11]
			for i in range(5):
				if entry[12 + i] is '':
					continue
				try:
					Db[MonsterId][skillidx]['val{}'.format(i)] = int(entry[12 + i])
				except ValueError:
					Db[MonsterId][skillidx]['val{}'.format(i)] = int(entry[12 + i], 16)
			Db[MonsterId][skillidx]['Emotion'] = entry[17]
			Db[MonsterId][skillidx]['ChatMsgID'] = entry[18]
	return Db

def ConvertDB(mode, serverpath):
	db = LoadOldDB(mode, serverpath)
	MobDB = Tools.LoadDBConsts('mob_db', mode, serverpath)
	SkillDB = Tools.LoadDBConsts('skill_db', mode, serverpath)

	printHeader()
	for mobid in sorted(db.iterkeys()):
		print('\t{}: {{'.format(MobDB[int(mobid)]))
		for skillidx in sorted(db[mobid].iterkeys()):
			valid = True
			if int(db[mobid][skillidx]['SkillId']) not in SkillDB:
				valid = False
				print('/*')
				print('// Can\'t find skill with id {} in skill_db'.format(db[mobid][skillidx]['SkillId']))
				print('\t\t{}: {{'.format(db[mobid][skillidx]['SkillId']))
			else:
				print('\t\t{}: {{'.format(SkillDB[int(db[mobid][skillidx]['SkillId'])]))
			printClearSkills('ClearSkills', db[mobid][skillidx])
			printSkillState('SkillState', db[mobid][skillidx])
			printStrToInt('SkillLevel', db[mobid][skillidx])
			printStrToInt('Rate', db[mobid][skillidx])
			printStrToInt('CastTime', db[mobid][skillidx])
			printStrToInt('Delay', db[mobid][skillidx])
			printBool('Cancelable', db[mobid][skillidx])
			printSkillTarget('SkillTarget', db[mobid][skillidx])
			printCastCondition('CastCondition', db[mobid][skillidx])
			printConditionData('ConditionData', db[mobid][skillidx])
			for i in range(5):
				printInt('val{}'.format(i), db[mobid][skillidx])
			printEmotion('Emotion', db[mobid][skillidx])
			printStrToInt('ChatMsgID', db[mobid][skillidx])
			print('\t\t}')
			if valid is False:
				print('*/')
		print('\t}')
	printFooter()

if len(sys.argv) != 3:
	print('Monster Skill db converter from txt to conf format')
	print('Usage:')
	print('    mobskilldbconverter.py mode serverpath')
	print("example:")
	print('    mobskilldbconverter.py pre-re ../')
	exit(1)

if sys.argv[1] != 're' and sys.argv[1] != 'pre-re':
	print('you have entred an invalid server mode')
	exit(1)

ConvertDB(sys.argv[1], sys.argv[2])
