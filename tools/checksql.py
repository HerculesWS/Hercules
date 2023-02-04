#! /usr/bin/env python3
# -*- coding: utf8 -*-
#
# This file is part of Hercules.
# http://herc.ws - http://github.com/HerculesWS/Hercules
#
# Copyright (C) 2014-2023 Hercules Dev Team
# Copyright (C) 2023 Andrei Karas (4144)
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

import os

insert_str_start1 = "INSERT IGNORE INTO `sql_updates` (`timestamp`) VALUES ("
insert_str_middle1 = "); -- "
insert_str_start2 = "INSERT INTO `sql_updates` (`timestamp`) VALUES ("
insert_str_start3 = "INSERT INTO `sql_updates` (`timestamp`, `ignored`) VALUES ("
insert_str_middle2 = ")"
insert_str_middle3 = ", 'No')"


class UpdateParser:
    def __init__(self, mainName, upgradesDir, indexName):
        self.mainName = mainName
        self.upgradesDir = upgradesDir
        self.indexName = indexName
        self.mainTimeStamps = set()
        self.mainNames = set()
        self.indexNames = set()
        self.upgradesNames = set()
        self.timeStampToName = dict()
        self.upgradesTimeStamps2 = set()
        self.result = 0


    def matchInsertSqlUpdateMain(self, line):
        if line[-1] == "\n":
            line = line[:-1]
        line = line.strip()
        idx = line.find(insert_str_start1)
        if idx < 0:
            return False, False
        line = line[len(insert_str_start1):]
        idx = line.find(insert_str_middle1)
        if idx < 0:
            return False, False
        timeStamp = line[:idx]
        name = line[idx + len(insert_str_middle1):]
        return timeStamp, name


    def matchInsertSqlUpdateUpgrade(self, line):
        if line[-1] == "\n":
            line = line[:-1]
        line = line.strip()
        if line.find("--") == 0:
            return False
        line = line.strip()
        idx = line.find(insert_str_start2)
        sz = len(insert_str_start2)
        if idx < 0:
            idx = line.find(insert_str_start3)
            sz = len(insert_str_start3)
        if idx < 0:
            return False
        line = line[sz:].strip()
        idx = line.find(insert_str_middle3)
        sz = len(insert_str_middle3)
        if idx < 0:
            idx = line.find(insert_str_middle2)
            sz = len(insert_str_middle2)
        if idx < 0:
            return False
        line = line[:idx].strip()
        timeStamp = line[:idx]
        return timeStamp



    def parseMainSql(self):
        with open(self.mainName, "rt") as r:
            for line in r:
                timeStamp, name = self.matchInsertSqlUpdateMain(line)
                if timeStamp is False:
                    continue
                self.mainTimeStamps.add(timeStamp)
                self.mainNames.add(name)


    def parseIndex(self):
        with open(self.indexName) as r:
            for line in r:
                if line[-1] == "\n":
                    line = line[:-1]
                self.indexNames.add(line)


    def parseUpgrades(self):
        files = sorted(os.listdir(self.upgradesDir))
        for fileName in files:
            if fileName[0] != "2":
                continue
            # print("Parse {0}".format(fileName))
            with open(os.path.join(self.upgradesDir, fileName)) as r:
                line = r.readline()
                if line[0] != "#":
                    print("Error: wrong timestamp format in "
                          "update {0}".format(fileName))
                    exit(1)
                if line[-1] == "\n":
                    line = line[:-1]
                line = line[1:]
                if line in self.timeStampToName:
                    print("Error: found double timestamp "
                          "{0} in file {1}".format(timeStamp,
                                                   fileName))
                    self.result = 1
                self.timeStampToName[line] = fileName
                self.upgradesNames.add(fileName)
                for line in r:
                    timeStamp = self.matchInsertSqlUpdateUpgrade(line)
                    if timeStamp is not False:
                        if timeStamp in self.upgradesTimeStamps2:
                            print("Error: found double timestamp in insert "
                                  "{0} in file {1}".format(timeStamp,
                                                           fileName))
                            self.result = 1
                        self.upgradesTimeStamps2.add(timeStamp)



    def parseAll(self):
        self.parseMainSql()
        self.parseIndex()
        self.parseUpgrades()


    def compareSets(self, set1, set2, err1, err2):
        if set1 != set2:
            res1 = set1 - set2
            res2 = set2 - set1
            if len(res1) != 0:
                print(err1.format(res1))
                self.result = 1
            if len(res2) != 0:
                print(err2.format(res2))
                self.result = 1


    def compareMainVsUpgrades(self):
        print("Compare main.sql vs upgrades/")
        self.compareSets(
            set(self.timeStampToName.keys()),
            self.mainTimeStamps,
            "Error: found missing sql updates in main.sql with timestamps: {0}",
            "Error: found extra sql updates in main.sql with timestamps: {0}"
        )
        self.compareSets(
            self.upgradesTimeStamps2,
            self.mainTimeStamps,
            "Error: found missing sql updates in main.sql with timestamps: {0}",
            "Error: found extra sql updates in main.sql with timestamps: {0}"
        )
        self.compareSets(
            self.upgradesNames,
            self.mainNames,
            "Error: found missing sql updates in main.sql with name: {0}",
            "Error: found extra sql updates in main.sql with name: {0}"
        )


    def compareMainVsIndex(self):
        print("Compare main.sql vs index.txt")
        self.compareSets(
            self.indexNames,
            self.mainNames,
            "Error: found missing sql updates in main.sql with name: {0}",
            "Error: found extra sql updates in main.sql with name: {0}"
        )


    def validate(self):
        self.compareMainVsUpgrades()
        self.compareMainVsIndex()



parser = UpdateParser("sql-files/main.sql",
                      "sql-files/upgrades/",
                      "sql-files/upgrades/index.txt")
parser.parseAll()
parser.validate()

exit(parser.result)
