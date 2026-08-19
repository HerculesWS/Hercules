#!/usr/bin/env python3
#
# This file is part of Hercules.
# http://herc.ws - http://github.com/HerculesWS/Hercules
#
# Copyright (C) 2013-2026 Hercules Dev Team
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

# Base Author: Haru @ http://herc.ws

# This script converts an item_db(2).txt to the new item_db(2).conf format.
# usage example: python tools/itemdbconverter.py < db/item_db2.txt > db/item_db2.conf

import fileinput
import re
import sys

ITEM_RE = re.compile(r"""
	^
	(?P<prefix>(?://[^0-9]*)?)
	(?P<ID>[0-9]+)[^,]*,
	(?P<AegisName>[^,]+),
	(?P<Name>[^,]+),[ \t]*
	(?P<Type>[0-9]+)[^,]*,[ \t]*
	(?P<Buy>[0-9]*)[^,]*,[ \t]*
	(?P<Sell>[0-9]*)[^,]*,[ \t]*
	(?P<Weight>[0-9]*)[^,]*,[ \t]*
	(?P<ATK>[0-9-]*)[^,:]*(?P<hasmatk>:[ \t]*(?P<MATK>[0-9-]*))?[^,]*,[ \t]*
	(?P<DEF>[0-9-]*)[^,]*,[ \t]*
	(?P<Range>[0-9]*)[^,]*,[ \t]*
	(?P<Slots>[0-9]*)[^,]*,[ \t]*
	(?P<Job>[x0-9A-Fa-f]*)[^,]*,[ \t]*
	(?P<Upper>[0-9]*)[^,]*,[ \t]*
	(?P<Gender>[0-9]*)[^,]*,[ \t]*
	(?P<Loc>[0-9]*)[^,]*,[ \t]*
	(?P<wLV>[0-9]*)[^,]*,[ \t]*
	(?P<eLV>[0-9]*)[^,:]*(?P<hasmaxlv>:[ \t]*(?P<eLVmax>[0-9]*))?[^,]*,[ \t]*
	(?P<Refineable>[0-9]*)[^,]*,[ \t]*
	(?P<View>[0-9]*)[^,]*,[ \t]*
	\{(?P<Script>.*)\},
	\{(?P<OnEquip>.*)\},
	\{(?P<OnUnequip>.*)\}
""", re.VERBOSE)


def perl_truthy(s):
	# Perl (and PHP) treat the exact string "0" as false, unlike Python
	# where any non-empty string is truthy. Several of the original
	# `if ($cols{Field})` checks below rely on that quirk (e.g. "0" is
	# used as a sentinel meaning "unset" for several numeric fields).
	return s is not None and s != '' and s != '0'


def prettifyscript(orig):
	orig = re.sub(r'^[ \t]*', '', orig)
	orig = re.sub(r'[ \t]*$', '', orig)
	if not re.search(r'[^ \t]', orig):
		return ''

	p = orig
	script = ''
	curly, lines, comment = 2, 0, 0
	linebreak, needindent = 0, 0

	while re.search(r'[^ \t]', p):
		linebreak = 0
		matched = None

		if comment and re.match(r'^\s*\*/\s*', p):
			p = re.sub(r'^\s*\*/\s*', '', p, count=1)
			comment = 0
			continue

		m = re.match(r'^\s*({)\s*', p)
		if m:
			p = p[m.end():]
			if not comment:
				curly += 1
			else:
				comment += 1
			script += " "
			linebreak = 1
			lines += 1
			matched = m.group(1)
		else:
			m = re.match(r'^\s*(})\s*', p)
			if m:
				p = p[m.end():]
				if not comment:
					curly -= 1
				if comment - 1 > 0:
					comment -= 1
				linebreak = 1
				lines += 1
				matched = m.group(1)
			else:
				m = re.match(r'^\s*(;)\s*', p)
				if m:
					p = p[m.end():]
					if p and (not comment or not re.match(r'^[ \t]*(?:\*/)[ \t]*$', p)):
						linebreak = 1
						lines += 1
					matched = m.group(1)
				else:
					m = re.match(r'^("[^"]*")', p)
					if m:
						p = p[m.end():]
						matched = m.group(1)
					else:
						m = re.match(r'^\s*/\*\s*', p)
						if m:
							p = p[m.end():]
							comment = 1
							continue
						else:
							m = re.match(r'^(.)', p)
							if not m:
								break
							p = p[m.end():]
							matched = m.group(1)

		if needindent:
			script += "\t" * curly
		if comment and (needindent or script == ''):
			script += "//" + ("\t" * (comment - 1))
		script += matched
		if linebreak:
			script += "\n"
			needindent = 1
		else:
			needindent = 0

	if curly != 2:
		sys.stderr.write("Parse error, curly braces count %d. returning unmodified script:\n%s\n\n" % (curly - 2, orig))
		return orig

	if lines:
		script = "\n\t\t" + script + "\n\t"
	else:
		script = " " + script + " "

	return script


def parsedb(lines):
	for raw in lines:
		line = raw.rstrip('\n').rstrip('\r')
		m = ITEM_RE.match(line)
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
			print("\tId: %s" % cols['ID'])
			print("\tAegisName: \"%s\"" % cols['AegisName'])
			print("\tName: \"%s\"" % cols['Name'])
			print("\tType: %s" % cols['Type'])
			if cols['Buy'] or cols['Buy'] == '0':
				print("\tBuy: %s" % cols['Buy'])
			if cols['Sell'] or cols['Sell'] == '0':
				print("\tSell: %s" % cols['Sell'])
			if perl_truthy(cols['Weight']):
				print("\tWeight: %s" % cols['Weight'])
			if perl_truthy(cols['ATK']):
				print("\tAtk: %s" % cols['ATK'])
			if perl_truthy(cols['MATK']):
				print("\tMatk: %s" % cols['MATK'])
			if perl_truthy(cols['DEF']):
				print("\tDef: %s" % cols['DEF'])
			if perl_truthy(cols['Range']):
				print("\tRange: %s" % cols['Range'])
			if perl_truthy(cols['Slots']):
				print("\tSlots: %s" % cols['Slots'])
			if not perl_truthy(cols['Job']):
				cols['Job'] = '0xFFFFFFFF'
			if not re.search(r'0xFFFFFFFF', cols['Job'], re.IGNORECASE):
				print("\tJob: %s" % cols['Job'])
			if perl_truthy(cols['Upper']) and (
				(perl_truthy(cols['hasmatk']) and int(cols['Upper']) != 0x3f)
				or (not perl_truthy(cols['hasmatk']) and int(cols['Upper']) != 7)
			):
				print("\tUpper: %s" % cols['Upper'])
			if not perl_truthy(cols['Gender']):
				cols['Gender'] = '2'
			if cols['Gender'] != '2':
				print("\tGender: %s" % cols['Gender'])
			if perl_truthy(cols['Loc']):
				print("\tLoc: %s" % cols['Loc'])
			if perl_truthy(cols['wLV']):
				print("\tWeaponLv: %s" % cols['wLV'])
			if perl_truthy(cols['hasmaxlv']) and perl_truthy(cols['eLVmax']):
				if not perl_truthy(cols['eLV']):
					cols['eLV'] = '0'
				print("\tEquipLv: [%s, %s]" % (cols['eLV'], cols['eLVmax']))
			else:
				if perl_truthy(cols['eLV']):
					print("\tEquipLv: %s" % cols['eLV'])
			if not perl_truthy(cols['Refineable']) and int(cols['Type']) in (4, 5):
				print("\tRefine: false")
			if perl_truthy(cols['View']):
				print("\tView: %s" % cols['View'])
			cols['Script'] = prettifyscript(cols['Script'])
			if perl_truthy(cols['Script']):
				print("\tScript: <\"%s\">" % cols['Script'])
			cols['OnEquip'] = prettifyscript(cols['OnEquip'])
			if perl_truthy(cols['OnEquip']):
				print("\tOnEquipScript: <\"%s\">" % cols['OnEquip'])
			cols['OnUnequip'] = prettifyscript(cols['OnUnequip'])
			if perl_truthy(cols['OnUnequip']):
				print("\tOnUnequipScript: <\"%s\">" % cols['OnUnequip'])
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
	print("""item_db: (
/******************************************************************************
 ************* Entry structure ************************************************
 ******************************************************************************
{
	// =================== Mandatory fields ===============================
	Id: ID                        (int)
	AegisName: "Aegis_Name"       (string, optional if Inherit: true)
	Name: "Item Name"             (string, optional if Inherit: true)
	// =================== Optional fields ================================
	Type: Item Type               (int, defaults to 3 = etc item)
	Buy: Buy Price                (int, defaults to Sell * 2)
	Sell: Sell Price              (int, defaults to Buy / 2)
	Weight: Item Weight           (int, defaults to 0)
	Atk: Attack                   (int, defaults to 0)
	Matk: Magical Attack          (int, defaults to 0, ignored in pre-re)
	Def: Defense                  (int, defaults to 0)
	Range: Attack Range           (int, defaults to 0)
	Slots: Slots                  (int, defaults to 0)
	Job: Job mask                 (int, defaults to all jobs = 0xFFFFFFFF)
	Upper: Upper mask             (int, defaults to any = 0x3f)
	Gender: Gender                (int, defaults to both = 2)
	Loc: Equip location           (int, required value for equipment)
	WeaponLv: Weapon Level        (int, defaults to 0)
	EquipLv: Equip required level (int, defaults to 0)
	EquipLv: [min, max]           (alternative syntax with min / max level)
	Refine: Refineable            (boolean, defaults to true)
	View: View ID                 (int, defaults to 0)
	BindOnEquip: true/false       (boolean, defaults to false)
	Script: <"
		Script
		(it can be multi-line)
	">
	OnEquipScript: <" OnEquip Script (can also be multi-line) ">
	OnUnequipScript: <" OnUnequip Script (can also be multi-line) ">
	// =================== Optional fields (item_db2 only) ================
	Inherit: true/false           (boolean, if true, inherit the values
	                              that weren't specified, from item_db.conf,
	                              else override it and use default values)
},
******************************************************************************/
""")

	parsedb(fileinput.input())

	print(")")


if __name__ == '__main__':
	main()
