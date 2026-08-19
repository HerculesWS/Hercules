#!/usr/bin/env python3
#
# This file is part of Hercules.
# http://herc.ws - http://github.com/HerculesWS/Hercules
#
# Copyright (C) 2016-2025 Hercules Dev Team
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

# This script converts item_db.conf Jobmask field into groups format
# usage example: python tools/itemdb_jobmask_converter.py < db/item_db2.conf > db/item_db_out.conf

import fileinput
import re

JOB_NAMES = [
	"Novice",
	"Swordsman",
	"Magician",
	"Archer",
	"Acolyte",
	"Merchant",
	"Thief",
	"Knight",
	"Priest",
	"Wizard",
	"Blacksmith",
	"Hunter",
	"Assassin",
	"Unused",
	"Crusader",
	"Monk",
	"Sage",
	"Rogue",
	"Alchemist",
	"Bard",
	"Unused",
	"Taekwon",
	"Star_Gladiator",
	"Soul_Linker",
	"Gunslinger",
	"Ninja",
	"Gangsi",
	"Death_Knight",
	"Dark_Collector",
	"Kagerou",
	"Rebellion",
]

JOB_RE = re.compile(r'^\s*Job\s*:\s*(?P<Job>(?:0x)?[0-9A-Fa-f]+)')


def parsedb(lines):
	job_size = len(JOB_NAMES)

	for raw in lines:
		line = raw.rstrip('\n').rstrip('\r')
		m = JOB_RE.match(line)
		if m:
			job_str = m.group('Job')
			job_mask = int(job_str, 16)
			all_jobs = 0xFFFFFFFF
			all_jobs_except_novice = 0xFFFFFFFE
			if job_mask < 0:
				print(line)
				continue
			print("\tJob: {")
			if (job_mask & all_jobs) == all_jobs:
				print("\t\tAll: true")
			elif (job_mask & all_jobs_except_novice) == all_jobs_except_novice:
				print("\t\tAll: true")
				print("\t\tNovice: false")
			elif job_mask == 0:
				print("\t\tAll: false")
			else:
				for i in range(job_size):
					curr_bit = 1 << i
					if (job_mask & curr_bit) == curr_bit:
						if JOB_NAMES[i] != "Unused":
							print("\t\t%s: true" % JOB_NAMES[i])
			print("\t}")
		else:
			print(line)


def main():
	parsedb(fileinput.input())


if __name__ == '__main__':
	main()
