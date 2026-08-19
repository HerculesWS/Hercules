#!/usr/bin/env python3
# mapreg.txt -> sql import file converter
# author : theultramage / Yommy
# version: 16. august 2008

import re
import sys


def mysql_escape_string(s):
	out = []
	for ch in s:
		if ch == '\x00':
			out.append('\\0')
		elif ch == '\n':
			out.append('\\n')
		elif ch == '\r':
			out.append('\\r')
		elif ch == '\\':
			out.append('\\\\')
		elif ch == "'":
			out.append("\\'")
		elif ch == '"':
			out.append('\\"')
		elif ch == '\x1a':
			out.append('\\Z')
		else:
			out.append(ch)
	return ''.join(out)


def main():
	sys.stderr.write("mapreg txt->sql converter\n")
	sys.stderr.write("-------------------------\n")

	if len(sys.argv) < 2:
		sys.stderr.write("Usage: %s [file]\n" % sys.argv[0])
		sys.exit()

	input_path = sys.argv[1]
	try:
		with open(input_path, 'r', encoding='utf-8', errors='surrogateescape') as f:
			data = f.readlines()
	except OSError:
		sys.exit("Invalid input file '%s'!" % input_path)

	sys.stderr.write("Converting %s...\n" % input_path)

	pat_with_index = re.compile(r'(.*),(\d+)\t(.*)')
	pat_no_index = re.compile(r'(.*)\t(.*)')

	def emit(varname, index, value):
		# NOTE: the `mapreg` table was dropped from Hercules in 2020
		# (sql-files/upgrades/2020-05-10--23-11.sql), split into
		# `map_reg_num_db` / `map_reg_str_db` (column `varname` renamed to
		# `key`), following the scripting-engine convention that a variable
		# name ending in "$" is a string variable, everything else numeric.
		if varname.endswith('$'):
			sys.stdout.write(
				"INSERT INTO `map_reg_str_db` (`key`,`index`,`value`) VALUES ('%s',%s,'%s');\n" % (
					mysql_escape_string(varname),
					mysql_escape_string(index),
					mysql_escape_string(value.rstrip()),
				)
			)
		else:
			stripped = value.strip()
			try:
				num_value = str(int(stripped or '0'))
			except ValueError:
				sys.stderr.write(
					"Warning: non-numeric value for numeric variable '%s': %r - inserting quoted, review manually.\n"
					% (varname, value)
				)
				# Quoted so the generated statement is at least valid SQL;
				# MySQL will raise a clear error/warning on insert for the
				# genuinely bad data instead of a confusing syntax error here.
				num_value = "'%s'" % mysql_escape_string(stripped)
			sys.stdout.write(
				"INSERT INTO `map_reg_num_db` (`key`,`index`,`value`) VALUES ('%s',%s,%s);\n" % (
					mysql_escape_string(varname),
					mysql_escape_string(index),
					num_value,
				)
			)

	for line in data:
		m = pat_with_index.match(line)
		if m:
			emit(m.group(1), m.group(2), m.group(3))
			continue

		m = pat_no_index.match(line)
		if m:
			emit(m.group(1), '0', m.group(2))
			continue

		sys.stderr.write("Invalid data: %s\n" % line)

	sys.stderr.write("done.\n")


if __name__ == '__main__':
	main()
