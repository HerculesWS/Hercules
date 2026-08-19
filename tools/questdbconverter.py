#!/usr/bin/env python3
#
# This file is part of Hercules.
# http://herc.ws - http://github.com/HerculesWS/Hercules
#
# Copyright (C) 2015-2025 Hercules Dev Team
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

# Base Author: Dastgir @ http://herc.ws

# This Script converts quest_db.txt to quest_db.conf format.
# usage example: python tools/questdbconverter.py < db/quest_db.txt > db/quest_db.conf

import fileinput
import re

QUEST_RE = re.compile(r"""
	^
	(?P<prefix>(?://[^0-9]*)?)
	(?P<QuestID>[0-9]+)[^,]*,[ \t]*
	(?P<TimeLimit>[0-9]+)[^,]*,[ \t]*
	(?P<Target1>[0-9]+)[^,]*,[ \t]*
	(?P<Val1>[0-9]+)[^,]*,[ \t]*
	(?P<Target2>[0-9]+)[^,]*,[ \t]*
	(?P<Val2>[0-9]+)[^,]*,[ \t]*
	(?P<Target3>[0-9]+)[^,]*,[ \t]*
	(?P<Val3>[0-9]+)[^,]*,[ \t]*
	"(?P<QuestTitle>[^"]*)"
""", re.VERBOSE)


def perl_truthy(s):
	# Perl treats the exact string "0" as false, unlike Python where any
	# non-empty string is truthy. The old quest_db.txt format uses "0" as
	# the "no target"/"no time limit" sentinel, so this quirk matters here.
	return s is not None and s != '' and s != '0'


def parse_questdb(lines):
	for raw in lines:
		line = raw.rstrip('\n').rstrip('\r')
		m = QUEST_RE.match(line)
		if m:
			cols = m.groupdict()
			for key in cols:
				if cols[key] is None:
					cols[key] = ''

			if cols['prefix']:
				print("/*")
			if cols['prefix'] and not re.match(r'^//[ \t]*$', cols['prefix']):
				print(cols['prefix'])
			print("{")
			print("\tId: %s" % cols['QuestID'])
			print("\tName: \"%s\"" % cols['QuestTitle'])
			if perl_truthy(cols['TimeLimit']):
				print("\tTimeLimit: %s" % cols['TimeLimit'])
			if perl_truthy(cols['Target1']) or perl_truthy(cols['Target2']) or perl_truthy(cols['Target3']):
				print("\tTargets: (")
			if perl_truthy(cols['Target1']):
				print("\t{")
				print("\t\tMobId: %s" % cols['Target1'])
				print("\t\tCount: %s" % cols['Val1'])
				print("\t},")
			if perl_truthy(cols['Target2']):
				print("\t{")
				print("\t\tMobId: %s" % cols['Target2'])
				print("\t\tCount: %s" % cols['Val2'])
				print("\t},")
			if perl_truthy(cols['Target3']):
				print("\t{")
				print("\t\tMobId: %s" % cols['Target3'])
				print("\t\tCount: %s" % cols['Val3'])
				print("\t},")
			if perl_truthy(cols['Target1']) or perl_truthy(cols['Target2']) or perl_truthy(cols['Target3']):
				print("\t)")
			print("},")
			if cols['prefix']:
				print("*/")
		elif line.startswith('//'):
			s = line[2:]
			if not re.match(r'^[ \t]*$', s):
				print("// %s" % s)
		elif not re.match(r'^\s*$', line):
			print("// Error parsing: %s" % line)


def main():
	print("""quest_db: (
//  Quest Database
/******************************************************************************
 ************* Entry structure ************************************************
 ******************************************************************************
{
	Id: Quest ID                    [int]
	Name: Quest Name                [string]
	TimeLimit: Time Limit (seconds) [int, optional]
	Targets: (                      [array, optional]
	{
		MobId: Mob ID           [int]
		Count:                  [int]
	},
	... (can repeated up to MAX_QUEST_OBJECTIVES times)
	)
	Drops: (
	{
		ItemId: Item ID to drop [int]
		Rate: Drop rate         [int]
		MobId: Mob ID to match  [int, optional]
	},
	... (can be repeated)
	)
},
******************************************************************************/
""")

	parse_questdb(fileinput.input())

	print(")")


if __name__ == '__main__':
	main()
