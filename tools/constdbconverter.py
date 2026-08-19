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
			# NOTE: the old const.txt "IsParameter" flag has no effect here.
			# Hercules' constants_db parser (script.c read_constdb) dropped
			# support for the "Parameter" key entirely; only "Value" and
			# "Deprecated" are read now, so parameter constants are emitted
			# the same as any other constant.
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
	Identifier: value            // (integer literal)
 ************* Entry structure (full) *************************************
	Identifier: {
		Value: value         // (integer literal)
		Deprecated: true     // (boolean)      Defaults to false.
	}
 ************* Supported integer literals *********************************
 decimal:      1337        // no prefix
 hexadecimal:  0x1337      // prefix: 0x
 octal:        0o1337      // prefix: 0o
 binary:       0b101101    // prefix: 0b

 Underscores can also be used as visual separators for digit grouping purposes:
 	2_147_483_647
 	0x7FFF_FFFF

 Keep in mind that number literals cannot start or end with a separator and no
 more than one separator can be used in a row (so 12_3___456 is illegal).
**************************************************************************/
""" % year)

	parse_constdb(fileinput.input())

	print("}")


if __name__ == '__main__':
	main()
