#!/usr/bin/env python3
# -*- coding: utf8 -*-
#
# This file is part of Hercules.
# http://herc.ws - http://github.com/HerculesWS/Hercules
#
# Copyright (C) 2019-2020 Hercules Dev Team
# Copyright (C) 2020 Andrei Karas (4144)
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

import sys


if len(sys.argv) != 3:
    print("Simple preprocessing. Expand given include file")
    print("Usage: ./preprocess.py SOURCE_FILE INCLUDE_STRING")
    exit(1)

def printLine(line):
    if line[-1] == "\r":
        line = line[:-1]
    if line[-1] == "\n":
        line = line[:-1]
    print(line)


def matchInclude(line, include):
    include_str = "#include \"{0}\"".format(include)
    idx = line.find(include_str)
    if idx >= 0:
        return include
    return None


def printFile(file_name):
    with open("../../src/{0}".format(file_name), "rt") as r:
        for line in r:
            printLine(line)


source_file = sys.argv[1]
include_string = sys.argv[2]
with open(source_file, "rt") as r:
    for line in r:
        if line.find("#include ") >= 0:
            pattern = matchInclude(line, include_string)
            if pattern is None:
                printLine(line)
                continue
            printFile(pattern)
            continue
        printLine(line)
