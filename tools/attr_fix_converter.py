#!/usr/bin/env python3
#
# This file is part of Hercules.
# http://herc.ws - http://github.com/HerculesWS/Hercules
#
# Copyright (C) 2012-2026 Hercules Dev Team
# Copyright (C) 2021 KirieZ
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
# Element constants, in their numerical order.
#
import os
import sys

ELEMENTS = [
	"Ele_Neutral",  # 0
	"Ele_Water",
	"Ele_Earth",
	"Ele_Fire",
	"Ele_Wind",
	"Ele_Poison",
	"Ele_Holy",
	"Ele_Dark",
	"Ele_Ghost",
	"Ele_Undead",
]


def show_help():
	print("Usage: python attr_fix_converter.py [option]")
	print("Options:")
	print("\t-re       [--renewal]       for renewal elemental damage database conversion.")
	print("\t-pre-re   [--pre-renewal]   for pre-renewal elemental damage database conversion.")
	print("\t-i <path> [--input] <path>  provides a custom file name/path as input.")
	print("\t-h        [--help]          to display this help text.")
	print("\t                            (it stops afterwards)\n")
	print("----------------------- Additional Notes ----------------------")
	print("Important!")
	print("* Please be advised that either and only one of the arguments -re/-pre-re")
	print("  must be specified on execution.")
	print("* When using the -i option, -re/-pre-re is ignored. ")
	print("* When -i option is not used, this tool assumes it is at the \"tools/\" directory")
	print("  of your Hercules folder.\n")
	print("----------------------- Usage Example -------------------------")
	print("- Renewal Conversion: python attr_fix_converter.py --renewal")
	print("- Pre-renewal Conversion: python attr_fix_converter.py --pre-renewal")
	print("- Custom File Conversion: python attr_fix_converter.py --input my_db/attr_fix.txt")
	print("----------------------------------------------------------------")
	sys.exit()


def parse_args(argv):
	input_path = None
	show_help_flag = False
	re_flag = False
	pre_flag = False

	i = 1
	while i < len(argv):
		arg = argv[i]
		if arg in ("-re", "--renewal"):
			re_flag = True
		elif arg in ("-pre-re", "--pre-renewal"):
			pre_flag = True
		elif arg in ("-i", "--input"):
			if i + 1 >= len(argv):
				print("Error: \"%s\" option must be followed by a file path.\n" % arg)
				show_help()
			input_path = argv[i + 1]
			i += 1
		elif arg in ("-h", "--help"):
			show_help_flag = True
		else:
			print("Error: Invalid option \"%s\".\n" % arg)
			show_help()
		i += 1

	return {
		"re": re_flag,
		"pre": pre_flag,
		"input": input_path,
		"show_help": show_help_flag,
	}


def get_base_table():
	table = {}
	damage = {e: 100 for e in ELEMENTS}
	for e in ELEMENTS:
		table[e] = {1: dict(damage), 2: dict(damage), 3: dict(damage), 4: dict(damage)}
	return table


def is_comment_or_empty(line):
	l = line.strip()
	if l == "":
		return True
	if len(line) >= 2 and line[0] == '/' and line[1] == '/':
		return True
	return False


def parse_attr(path):
	if not os.path.exists(path):
		print("Error: File \"%s\" does not exists." % path)
		sys.exit()

	try:
		f = open(path, "r")
	except OSError:
		print("Error: Failed to open attribute table file")
		sys.exit()

	ele_table = get_base_table()

	ln = 0
	with f:
		while True:
			line = f.readline()
			if line == "":
				break
			ln += 1

			if is_comment_or_empty(line):
				continue

			level_size = line.split(",")
			if len(level_size) < 2:
				print("Error: Unexpected line %s. Expected level and array size, found less than 2 fields." % line)
				sys.exit()

			lv = int(level_size[0].strip() or 0)
			size = int(level_size[1].strip() or 0)
			i = 0
			max_loop = min(size, len(ELEMENTS))
			while i < max_loop:
				line = f.readline()
				while line != "" and is_comment_or_empty(line):
					ln += 1
					line = f.readline()

				ln += 1
				if line == "":
					print("Error: End of file reached before loading all data.")
					sys.exit()

				atk_ele = ELEMENTS[i]

				# Neut Watr Erth Fire Wind Pois Holy Shdw Gho  Und
				ele_row = line.split(',')
				# NOTE: the original PHP compared the array itself against $max_loop
				# (`$ele_row < $max_loop`), which in PHP is always false (arrays
				# compare greater than scalars), so that "incomplete line" check
				# never actually fired. Preserved here as a no-op for fidelity.

				for j in range(max_loop):
					def_ele = ELEMENTS[j]
					dmg_adjust = int(ele_row[j].strip() or 0) if j < len(ele_row) else 0
					ele_table[def_ele].setdefault(lv, {})[atk_ele] = dmg_adjust
				i += 1

	return ele_table


def get_file_header():
	return """//================= Hercules Database =====================================
//=       _   _                     _
//=      | | | |                   | |
//=      | |_| | ___ _ __ ___ _   _| | ___  ___
//=      |  _  |/ _ \\ '__/ __| | | | |/ _ \\/ __|
//=      | | | |  __/ | | (__| |_| | |  __/\\__ \\
//=      \\_| |_/\\___|_|  \\___|\\__,_|_|\\___||___/
//================= License ===============================================
//= This file is part of Hercules.
//= http://herc.ws - http://github.com/HerculesWS/Hercules
//=
//= Copyright (C) 2015-2026 Hercules Dev Team
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
//= Elemental attribute damage adjustment tables
//=
//= This file controls the increase/reduction of the attacker's damage element
//= against a defending enemy element.
//=
//= For example:
//= A Fire Lv1 monster will take 150% damage (+50%) from a Water-element attack.
//= - Fire Lv1 is the defending element
//= - Water is the attacking element
//= - 150% is the damage modifier, an increase of 50% over the base damage (100%)
//=
//= Notes:
//= - By default, all defending elements/levels has the adjustment at 100% (base damage)
//= - If the same Defending Element/Level is declared more than one time,
//=   their definitions are merged.
//=========================================================================

/**************************************************************************
 ************* Entry structure ********************************************
 **************************************************************************
<Defending Element>: { // Ele_* constant of the Defending element
	// Level of the defending element (by default, may be Lv1 up to Lv4)
	Lv1: {
		// Attacking Element: the attacking element Ele_* constant
		// adjustment: damage rate, where 100 means 100% (base damage, no additions/reductions)
		<Attacking Element>: <adjustment>
	}
	Lv2: {
		<Attacking Element>: <adjustment>
	}
	Lv3: {
		<Attacking Element>: <adjustment>
	}
	Lv4: {
		<Attacking Element>: <adjustment>
	}
}
**************************************************************************/
"""


def write_libconfig(out_path, table):
	if os.path.exists(out_path):
		print("Error: File \"%s\" already exists. Please remove it first." % out_path)
		sys.exit()

	output = get_file_header()
	for def_ele, levels in table.items():
		output += "\n%s: {\n" % def_ele

		for level, modifiers in levels.items():
			output += "\tLv%s: {\n" % level

			for atk_ele, rate in modifiers.items():
				output += "\t\t%s: %s\n" % (atk_ele, rate)

			output += "\t}\n"

		output += "}"

	output += "\n"

	with open(out_path, "w", newline='\n') as f:
		f.write(output)


def main():
	argv = sys.argv
	if len(argv) < 2:
		show_help()

	args = parse_args(argv)
	if args["show_help"]:
		show_help()

	if not args["re"] and not args["pre"] and not args["input"]:
		print("Error: You must inform a server mode or an input file.\n")
		show_help()

	if (args["re"] or args["pre"]) and args["input"]:
		print("Error: You must inform a server mode or an input file, not both.\n")
		show_help()

	if args["re"] and args["pre"]:
		print("Error: You can not use \"-re\" and \"-pre-re\" at the same time.\n")
		show_help()

	script_dir = os.path.dirname(os.path.abspath(__file__))

	if args["input"]:
		base_path = script_dir + os.sep + ".." + os.sep
		input_path = args["input"]
		if not (input_path.startswith("/") or (len(input_path) > 1 and input_path[1] == ":")):
			input_path = base_path + input_path

		path = input_path
		out_path = input_path + ".conf"
	else:
		base_path = script_dir + os.sep + ".." + os.sep + "db" + os.sep
		path = base_path + ("re" if args["re"] else "pre-re") + os.sep + "attr_fix.txt"
		out_path = path[:-4] + ".conf"

	attr_table = parse_attr(path)
	write_libconfig(out_path, attr_table)

	print("Conversion finished.")


if __name__ == '__main__':
	main()
