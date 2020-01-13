#!/usr/bin/python
# -*- coding: utf8 -*-
#
# This file is part of Hercules.
# http://herc.ws - http://github.com/HerculesWS/Hercules
#
# Copyright (C) 2018-2020 Hercules Dev Team
# Copyright (C) 2018 Asheraf
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

import io
import sys
if sys.version_info >= (3, 0):
	from utils import libconf as libconf
else:
	import libconf as libconf
import os.path

def LoadDBConsts(DBname, mode, serverpath):
	filenames = [serverpath + 'db/{}/{}.conf'.format(mode, DBname)]

	if os.path.isfile(serverpath + 'db/{}2.conf'.format(DBname)):
		filenames.append(serverpath + 'db/{}2.conf'.format(DBname))

	consts = dict()
	for filename in filenames:
		with io.open(filename) as f:
			config = libconf.load(f)
			db = config[DBname]
			if DBname == 'item_db':
				for i, v in enumerate(db):
					consts[db[i].Id] = db[i].AegisName
			elif DBname == 'mob_db':
				for i, v in enumerate(db):
					consts[db[i].Id] = db[i].SpriteName
			elif DBname == 'skill_db':
				for i, v in enumerate(db):
					consts[db[i].Id] = db[i].Name
			else:
				print('LoadDBConsts: invalid database name {}'.format(DBname))
				exit(1)
	return consts

def LoadDB(DBname, mode, serverpath):
	filenames = [serverpath + 'db/{}/{}.conf'.format(mode, DBname)]

	if os.path.isfile(serverpath + 'db/{}2.conf'.format(DBname)):
		filenames.append(serverpath + 'db/{}2.conf'.format(DBname))

	for filename in filenames:
		with io.open(filename) as f:
			config = libconf.load(f)
			db = config[DBname]
			return db
	print('LoadDB: invalid database name {}'.format(DBname))
	exit(1)
