#!/usr/bin/python
# -*- coding: utf8 -*-
#
# This file is part of Hercules.
# http://herc.ws - http://github.com/HerculesWS/Hercules
#
# Copyright (C) 2018  Hercules Dev Team
# Copyright (C) 2018  Asheraf
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
import os
import utils.libconf as libconf


def load_db_consts(db_name, mode, serverpath):
    filenames = [serverpath + 'db/{}/{}.conf'.format(mode, db_name)]

    if os.path.isfile(serverpath + 'db/{}2.conf'.format(db_name)):
        filenames.append(serverpath + 'db/{}2.conf'.format(db_name))

    consts = dict()
    for filename in filenames:
        with io.open(filename) as f:
            config = libconf.load(f)
            db = config[db_name]
            if db_name == 'item_db':
                for i, v in enumerate(db):
                    consts[db[i].Id] = db[i].AegisName
            elif db_name == 'mob_db':
                for i, v in enumerate(db):
                    consts[db[i].Id] = db[i].SpriteName
            elif db_name == 'skill_db':
                for i, v in enumerate(db):
                    consts[db[i].Id] = db[i].Name
            else:
                print('load_db_consts: invalid database name {}'.format(db_name))
                exit(1)
    return consts


def load_db(db_name, mode, serverpath):
    filenames = [serverpath + 'db/{}/{}.conf'.format(mode, db_name)]

    if os.path.isfile(serverpath + 'db/{}2.conf'.format(db_name)):
        filenames.append(serverpath + 'db/{}2.conf'.format(db_name))

    for filename in filenames:
        with io.open(filename) as f:
            config = libconf.load(f)
            db = config[db_name]
            return db
    print('load_db: invalid database name {}'.format(db_name))
    exit(1)
