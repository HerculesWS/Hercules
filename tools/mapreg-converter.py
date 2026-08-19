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

	for line in data:
		m = pat_with_index.match(line)
		if m:
			varname, index, value = m.group(1), m.group(2), m.group(3)
			sys.stdout.write(
				"INSERT INTO `mapreg` (`varname`,`index`,`value`) VALUES ('%s',%s,'%s');\n" % (
					mysql_escape_string(varname),
					mysql_escape_string(index),
					mysql_escape_string(value.rstrip()),
				)
			)
			continue

		m = pat_no_index.match(line)
		if m:
			varname, value = m.group(1), m.group(2)
			sys.stdout.write(
				"INSERT INTO `mapreg` (`varname`,`index`,`value`) VALUES ('%s',0,'%s');\n" % (
					mysql_escape_string(varname),
					mysql_escape_string(value.rstrip()),
				)
			)
			continue

		sys.stderr.write("Invalid data: %s\n" % line)

	sys.stderr.write("done.\n")


if __name__ == '__main__':
	main()
