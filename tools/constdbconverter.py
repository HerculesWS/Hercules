#!/usr/bin/env python3
#
# This file is part of Hercules.
# http://herc.ws - http://github.com/HerculesWS/Hercules
#
# Copyright (C) 2016-2026 Hercules Dev Team
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

# This Script converts const.txt to constants.conf format.
# usage example: python tools/constdbconverter.py < db/const.txt > db/constants.conf

import datetime
import fileinput
import re
import sys

CONST_RE = re.compile(r"""
	^
	(?P<prefix>(?://[^A-Za-z0-9'_]*)?)
	(?P<ConstantName>[A-Za-z0-9'_]+)
	(?:,|[ \t]+)(?P<Value>(?:0x[a-fA-F0-9]+|-?[0-9]+))
	(?:(?:,|[ \t]+)(?P<IsParameter>[01]))?
""", re.VERBOSE)


def parse_constdb(lines):
	for raw in lines:
		line = raw.rstrip('\n').rstrip('\r')
		m = CONST_RE.match(line)
		if m:
			cols = m.groupdict()
			for key in cols:
				if cols[key] is None:
					cols[key] = ''
			if not cols['prefix'] and re.match(r'^\s*(true|false)\s*$', cols['ConstantName']):
				cols['prefix'] = '// '
			if cols['IsParameter'] and cols['IsParameter'] == '1':
				if cols['prefix']:
					print("/*")
				if cols['prefix'] and not re.match(r'^//[ \t]*$', cols['prefix']):
					print("\t%s" % cols['prefix'])
				print("\t%s: {" % cols['ConstantName'])
				print("\t\tValue: %s" % cols['Value'])
				print("\t\tParameter: true")
				print("\t}")
				if cols['prefix']:
					print("*/")
				continue
			if cols['prefix']:
				sys.stdout.write("\t%s" % cols['prefix'])
			print("\t%s: %s" % (cols['ConstantName'], cols['Value']))
		elif re.match(r'^//(.*)$', line):
			s = re.match(r'^//(.*)$', line).group(1)
			if not re.match(r'^[ \t]*$', s):
				print("\t// %s" % s)
		elif not re.match(r'^\s*$', line):
			print("// Error parsing: %s" % line)


def main():
	year = datetime.date.today().year
	print(r"""//================= Hercules Database =====================================
//=       _   _                     _
//=      | | | |                   | |
//=      | |_| | ___ _ __ ___ _   _| | ___  ___
//=      |  _  |/ _ \ '__/ __| | | | |/ _ \/ __|
//=      | | | |  __/ | | (__| |_| | |  __/\__ \
//=      \_| |_/\___|_|  \___|\__,_|_|\___||___/
//================= License ===============================================""")
	print("""//= This file is part of Hercules.
//= http://herc.ws - http://github.com/HerculesWS/Hercules
//=
//= Copyright (C) 2016-%d Hercules Dev Team
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
//= Script Constants Database
//=========================================================================

constants_db: {
/************* Entry structure (short) ************************************
	Identifier: value            // (int)
 ************* Entry structure (full) *************************************
	Identifier: {
		Value: value         // (int)
		Parameter: true      // (boolean)      Defaults to false.
		Deprecated: true     // (boolean)      Defaults to false.
	}
**************************************************************************/
// NOTE:
//   Parameters are special in that they retrieve certain runtime values
//   depending on the specified ID in field Value. Depending on the
//   implementation values assigned by scripts to parameters will affect
//   runtime values, such as Zeny, as well (see pc_readparam/pc_setparam).
""" % year)

	parse_constdb(fileinput.input())

	print("}")


if __name__ == '__main__':
	main()
