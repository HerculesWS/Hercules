#!/usr/bin/env python
# -*- coding: utf8 -*-
#
# This file is part of Hercules.
# http://herc.ws - http://github.com/HerculesWS/Hercules
#
# Copyright (C) 2019-2022 Hercules Dev Team
# Copyright (C) 2019 Asheraf
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

import re
import csv

f = open('../db/re/mob_db.conf')
mob_db = f.read()

with open('../db/mob_avail.txt') as dbfile:
	mob_avail = csv.reader(dbfile, delimiter=',')

	for mob in mob_avail:
		if len(mob) == 0 or mob[0].startswith('//'):
			continue

		mob = [re.sub(r'//.*', '', i).strip() for i in mob]

		mob_id = int(mob[0])
		sprite_id = int(mob[1])
		weapon = 0
		shield = 0
		head_top = 0
		head_mid = 0
		head_bottom = 0
		hair_style = 0
		hair_color = 0
		cloth_color = 0
		gender = 0
		option = 0
		if len(mob) == 3:
			head_bottom = int(mob[2])
		elif len(mob) == 12:
			gender = int(mob[2])
			hair_style = int(mob[3])
			hair_color = int(mob[4])
			weapon = int(mob[5])
			shield = int(mob[6])
			head_top = int(mob[7])
			head_mid = int(mob[8])
			head_bottom = int(mob[9])
			option = int(mob[10])
			cloth_color = int(mob[11])

		s = ''
		s += '\tViewData: {\n'
		s += '\t\tSpriteId: {}\n'.format(sprite_id)
		if weapon != 0:
			s += '\t\tWeaponId: {}\n'.format(weapon)
		if shield != 0:
			s += '\t\tShieldId: {}\n'.format(shield)
		if head_top != 0:
			s += '\t\tHeadTopId: {}\n'.format(head_top)
		if head_mid != 0:
			s += '\t\tHeadMidId: {}\n'.format(head_mid)
		if head_bottom != 0:
			s += '\t\tHeadLowId: {}\n'.format(head_bottom)
		if hair_style != 0:
			s += '\t\tHairStyleId: {}\n'.format(hair_style)
		if hair_color != 0:
			s += '\t\tHairColorId: {}\n'.format(hair_color)
		if cloth_color != 0:
			s += '\t\tBodyColorId: {}\n'.format(cloth_color)
		if gender != 0:
			s += '\t\tGender: SEX_MALE\n'
		if option != 0:
			s += '\t\tOptions: {}\n'.format(option)
		s += '\t}'

		mob_db = re.sub(
			r'(\tId: ' + str(mob_id) + r'\n([\S\s]*?)(?=},))},',
			r'\1' + str(s) + r'\n},',
			mob_db)
	print(mob_db)
