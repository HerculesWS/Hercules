#!/usr/bin/env python
# -*- coding: utf8 -*-
#
# This file is part of Hercules.
# http://herc.ws - http://github.com/HerculesWS/Hercules
#
# Copyright (C) 2019 Hercules Dev Team
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

import csv

with open('../db/sc_config.txt') as dbfile:
	sc_config = csv.reader(dbfile, delimiter=',')

	print(r'''//================= Hercules Database =====================================
//=       _   _                     _
//=      | | | |                   | |
//=      | |_| | ___ _ __ ___ _   _| | ___  ___
//=      |  _  |/ _ \ '__/ __| | | | |/ _ \/ __|
//=      | | | |  __/ | | (__| |_| | |  __/\__ \
//=      \_| |_/\___|_|  \___|\__,_|_|\___||___/
//================= License ===============================================
//= This file is part of Hercules.
//= http://herc.ws - http://github.com/HerculesWS/Hercules
//=
//= Copyright (C) 2019  Hercules Dev Team
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
//================= Description ===========================================
// Configurations file for status effects
//=========================================================================
''')
	print('''/**************************************************************************
 ************* Entry structure ********************************************
 **************************************************************************
SC_TYPE: {
	Visible:           (bool) SC can be visible for all players
	Flags: {
		NoDeathReset:      (bool) SC cannot be removed by death.
		NoSave:            (bool) SC cannot be saved.
		NoDispelReset:     (bool) SC cannot be reset by dispell.
		NoClearanceReset:  (bool) SC cannot be reset by clearance.
		Buff:              (bool) SC considered as buff and be removed by Hermode and etc.
		Debuff:            (bool) SC considered as debuff and be removed by Gospel and etc.
		NoMadoReset:       (bool) SC cannot be reset when MADO Gear is taken off.
		NoAllReset:        (bool) SC cannot be reset by 'sc_end SC_ALL' and status change clear.
	}
}
**************************************************************************/''')
	for sc in sc_config:
		if len(sc) != 2 or sc[0].startswith('//'):
			continue
		print('{}: {{'.format(sc[0]))
		if int(sc[1]) & 256:
			print('\tVisible: true')
		print('\tFlags: {')
		if int(sc[1]) & 1:
			print('\t\tNoDeathReset: true')
		if int(sc[1]) & 2:
			print('\t\tNoSave: true')
		if int(sc[1]) & 4:
			print('\t\tNoDispelReset: true')
		if int(sc[1]) & 8:
			print('\t\tNoClearanceReset: true')
		if int(sc[1]) & 16:
			print('\t\tBuff: true')
		if int(sc[1]) & 32:
			print('\t\tDebuff: true')
		if int(sc[1]) & 64:
			print('\t\tNoMadoReset: true')
		if int(sc[1]) & 128:
			print('\t\tNoAllReset: true')
		print('\t}')
		print('}')
