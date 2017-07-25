#!/usr/bin/env bash
#
# This file is part of Hercules.
# http://herc.ws - http://github.com/HerculesWS/Hercules
#
# Copyright (C) 2015-2015  Hercules Dev Team
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

./mobdbconverter.py re .. ../db/re/mob_db.txt > ../db/re/mob_db.conf
./mobdbconverter.py re .. ../db/mob_db2.txt > ../db/mob_db2.conf
./mobdbconverter.py pre-re .. ../db/pre-re/mob_db.txt > ../db/pre-re/mob_db.conf
#./mobdbconverter.py pre-re .. ../db/mob_db2.txt > ../db/mob_db2.conf
