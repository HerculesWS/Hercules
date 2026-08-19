#!/usr/bin/env python3
#
# This file is part of Hercules.
# http://herc.ws - http://github.com/HerculesWS/Hercules
#
# Copyright (C) 2014-2026 Hercules Dev Team
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

# Run from the tools/HPMHookGen/ directory, after `doxygen` has produced
# doxyoutput/xml/*.xml, to (re)generate ../../src/common/HPMDataCheck.h

import datetime
import glob
import os
import re
import xml.etree.ElementTree as ET

HPM_DATA_CHECK_API_VER = 1


def main():
	# Perl's glob() sorts its results alphabetically by default; Python's
	# glob.glob() does not, so the sort must be done explicitly to keep
	# entries within each #ifdef guard group in a stable, matching order.
	files = sorted(
		f for f in glob.glob('doxyoutput/xml/struct*.xml')
		if os.path.isfile(f) and re.search(r'[^h]\.xml$', f)
	)

	out = {}

	for file in files:
		try:
			tree = ET.parse(file)
		except ET.ParseError:
			continue
		root = tree.getroot()
		compounddef = root.find('compounddef')
		if compounddef is None:
			continue

		# means its a struct from a .c file, plugins cant access those so we don't care.
		if compounddef.find('includes') is None:
			continue

		compoundname_el = compounddef.find('compoundname')
		compoundname = compoundname_el.text if compoundname_el is not None and compoundname_el.text else ''
		# its a duplicate with a :: name e.g. struct script_state {<...>} ay;
		if '::' in compoundname:
			continue

		location = compounddef.find('location')
		file_attr = location.get('file', '') if location is not None else ''
		filepath = re.split(r'[/\\]', file_attr)
		foldername = filepath[-2].upper() if len(filepath) >= 2 else ''

		# Skip the HPM core, plugins don't need it
		if filepath[-1] == "HPM.h":
			continue

		filename = filepath[-1].upper()
		filename = re.sub(r'[.-]', '_', filename)
		filename = re.sub(r'\.[^.]*$', '', filename)

		plugintypes = 'SERVER_TYPE_UNKNOWN'
		if foldername == 'COMMON':
			if filename == 'MAPINDEX_H':
				plugintypes = 'SERVER_TYPE_CHAR|SERVER_TYPE_MAP'
			elif filename == 'GRFIO_H':
				plugintypes = 'SERVER_TYPE_MAP'
			else:
				plugintypes = 'SERVER_TYPE_ALL'
		elif re.match(r'^(LOGIN|CHAR|MAP)', foldername):
			plugintypes = "SERVER_TYPE_%s" % foldername

		symboldata = {
			'name': compoundname,
			'type': plugintypes,
		}
		name = "%s_%s" % (foldername, filename)
		out.setdefault(name, []).append(symboldata)

	fname = '../../src/common/HPMDataCheck.h'
	year = datetime.date.today().year

	with open(fname, 'w', newline='\n') as fh:
		fh.write("""/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2014-%d Hercules Dev Team
 *
 * Hercules is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * NOTE: This file was auto-generated and should never be manually edited,
 *       as it will get overwritten.
 */

/* GENERATED FILE DO NOT EDIT */

#ifndef HPM_DATA_CHECK_H
#define HPM_DATA_CHECK_H

#if !defined(HPMHOOKGEN)
#include "common/HPMSymbols.inc.h"
#endif // ! HPMHOOKGEN
#ifdef HPM_SYMBOL
#undef HPM_SYMBOL
#endif // HPM_SYMBOL

HPExport const struct s_HPMDataCheck HPMDataCheck[] = {
""" % year)

		for key in sorted(out.keys()):
			fh.write("\t#ifdef %s\n" % key)
			for entry in out[key]:
				fh.write("\t\t{ \"%s\", sizeof(struct %s), %s },\n" % (entry['name'], entry['name'], entry['type']))
			fh.write("\t#else\n")
			fh.write("\t\t#define %s\n" % key)
			fh.write("\t#endif // %s\n" % key)

		fh.write("""};
HPExport unsigned int HPMDataCheckLen = ARRAYLENGTH(HPMDataCheck);
HPExport int HPMDataCheckVer = %d;

#endif /* HPM_DATA_CHECK_H */
""" % HPM_DATA_CHECK_API_VER)


if __name__ == '__main__':
	main()
