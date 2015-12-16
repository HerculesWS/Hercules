#! /usr/bin/env python
# -*- coding: utf8 -*-
#
# This file is part of Hercules.
# http://herc.ws - http://github.com/HerculesWS/Hercules
#
# Copyright (C) 2014-2015  Hercules Dev Team
# Copyright (C) 2014  Andrei Karas (4144)
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
import re
import sys
from sets import Set

interfaceRe = re.compile("struct (?P<name1>[a-z_]+)_interface (?P<name2>[a-z_]+)_s;")

class Tracker:
    pass

def searchDefault(r, ifname):
    defaultStr = "void {0}_defaults(void)".format(ifname);
    for line in r:
        if line.find(defaultStr) == 0:
            return True
    return False

def searchStructStart(r, ifname):
    for line in r:
        if line.find("struct {0}_interface".format(ifname)) == 0:
            return True
    return False

def readCFile(tracker, cFile):
    methods = Set()
    shortIfName = ""
    with open(cFile, "r") as r:
        for line in r:
#            print "cline1: " + line
            m = interfaceRe.search(line)
            if m != None and m.group("name1") == m.group("name2"):
                # found C file with interface
                ifname = m.group("name1")
                if searchDefault(r, ifname) == False:
                    return (None, shortIfName, methods)
                lineRe = re.compile("(?P<ifname>[a-z_]+)->(?P<method>[\w_]+)[ ][=][ ](?P<fullmethod>[^;]+);")
                for line in r:
#                    print "cline2: " + line
                    test = line.strip()
                    if len(test) > 2 and test[0:2] == "//":
                        continue
                    if len(line) > 0 and line[0] == "}":
                        break
                    m = lineRe.search(line)
                    if m != None:
                        tmp = m.group("ifname")
                        if len(tmp) < 2 or tmp[0] != ifname[0]:
                            continue
                        if shortIfName == "":
                            shortIfName = m.group("ifname")
#                        print "{2}: add {0}, from: {1}".format(m.group("method"), line, ifname)
                        methods.add(m.group("method"))
                        tracker.interfaces.add(ifname);
                        tracker.fullmethods.add(m.group("fullmethod"));
                return (ifname, shortIfName, methods)
    return (None, shortIfName, methods)

def readHFile(tracker, hFile, ifname):
    methods = Set()
    with open(hFile, "r") as r:
        if searchStructStart(r, ifname) == False:
            return methods
        lineRe = re.compile("[(][*](?P<method>[^)]+)[)]".format(ifname))
        for line in r:
#            print "hline: " + line
            test = line.strip()
            if len(test) > 2 and test[0:2] == "//":
                continue
            if len(line) > 0 and line[0] == "}":
                break
            m = lineRe.search(line)
            if m != None:
#                print "{2}: add {0}, from: {1}".format(m.group("method"), line, ifname)
                methods.add(m.group("method"))
                tracker.fullmethods.add(ifname + "_" + m.group("method"))
    return methods

def checkIfFile(tracker, cFile, hFile):
    data = readCFile(tracker, cFile)
    cMethods = data[2]
    ifname = data[0]
    shortIfName = data[1]
    if len(cMethods) > 0:
        hMethods = readHFile(tracker, hFile, ifname)
        for method in hMethods:
            tracker.arr[ifname + "_" + method] = list()
            tracker.methods.add(ifname + "_" + method)
            if method not in cMethods:
                print "Missing initialisation in file {0}: {1}".format(cFile, method)
                tracker.retCode = 1
#        for method in cMethods:
#            if method not in hMethods:
#                print "Extra method in file {0}: {1}".format(cFile, method)

def processIfDir(tracker, srcDir):
    files = os.listdir(srcDir)
    for file1 in files:
        if file1[0] == '.' or file1 == "..":
            continue
        cPath = os.path.abspath(srcDir + os.path.sep + file1)
        if not os.path.isfile(cPath):
            processIfDir(tracker, cPath)
        else:
            if file1[-2:] == ".c":
                file2 = file1[:-2] + ".h"
                hPath = srcDir + os.path.sep + file2;
                if os.path.exists(hPath) and os.path.isfile(hPath):
                    checkIfFile(tracker, cPath, hPath)


def checkChr(ch):
    if (ch >= "a" and ch <= "z") or ch == "_" or (ch >= "0" and ch <= "9" or ch == "\"" or ch == ">"):
        return True
    return False

def checkFile(tracker, cFile):
#    print "Checking: " + cFile
    with open(cFile, "r") as r:
        for line in r:
            parts = re.findall(r'[\w_]+', line)
            for part in parts:
                if part in tracker.methods:
                    idx = line.find(part)
                    if idx > 0:
                        if idx + len(part) >= len(line):
                            continue
                        if checkChr(line[idx + len(part)]):
                            continue
                        if checkChr(line[idx - 1]):
                            continue
                        if line[0:3] == " * ":
                            continue;
                        if line[-1] == "\n":
                            line = line[:-1]
                        text = line.strip()
                        if text[0:2] == "/*" or text[0:2] == "//":
                            continue
                        idx2 = line.find("//")
                        if idx2 > 0 and idx2 < idx:
                            continue
                        tracker.arr[part].append(line)

def processDir(tracker, srcDir):
    files = os.listdir(srcDir)
    for file1 in files:
        if file1[0] == '.' or file1 == "..":
            continue
        cPath = os.path.abspath(srcDir + os.path.sep + file1)
        if not os.path.isfile(cPath):
            processDir(tracker, cPath)
        elif file1[-2:] == ".c" or file1[-2:] == ".h":
#        elif file1[-2:] == ".c":
            checkFile(tracker, cPath)

def reportMethods(tracker):
    print "\n"
    for method in tracker.methods:
        if len(tracker.arr[method]) > 2:
            print method
            for t in tracker.arr[method]:
                print t
            print "\n"


def checkLostFile(tracker, cFile):
#    print "Checking: " + cFile
    methodRe = re.compile("^([\w0-9* _]*)([ ]|[*])(?P<ifname>[a-z_]+)_(?P<method>[\w_]+)(|[ ])[(]")
    with open(cFile, "r") as r:
        for line in r:
            if line.find("(") < 1 or len(line) < 3 or line[0] == "\t" or line[0] == " " or line.find("_defaults") > 0:
                continue
            m = methodRe.search(line)
            if m != None:
                name = "{0}_{1}".format(m.group("ifname"), m.group("method"))
                if name[:name.find("_")] not in tracker.interfaces and m.group("ifname") not in tracker.interfaces:
                    continue
                if name not in tracker.fullmethods:
#                    print "src  : " + line
                    print name

def processLostDir(tracker, srcDir):
    files = os.listdir(srcDir)
    for file1 in files:
        if file1[0] == '.' or file1 == "..":
            continue
        cPath = os.path.abspath(srcDir + os.path.sep + file1)
        if not os.path.isfile(cPath):
            processLostDir(tracker, cPath)
        elif file1[-2:] == ".c":
            checkLostFile(tracker, cPath)

def runIf():
    processIfDir(tracker, "../src/char");
    processIfDir(tracker, "../src/map");
    processIfDir(tracker, "../src/login");
    processIfDir(tracker, "../src/common");

def runLost():
    processLostDir(tracker, "../src/char");
    processLostDir(tracker, "../src/map");
    processLostDir(tracker, "../src/login");
    processLostDir(tracker, "../src/common");

def runLong():
    processDir(tracker, "../src/char");
    processDir(tracker, "../src/map");
    processDir(tracker, "../src/login");
    processDir(tracker, "../src/common");
    reportMethods(tracker)

tracker = Tracker()
tracker.arr = dict()
tracker.methods = Set()
tracker.fullmethods = Set()
tracker.interfaces = Set()
tracker.retCode = 0

if len(sys.argv) > 1:
    cmd = sys.argv[1]
else:
    cmd = "default"

if cmd == "silent":
    runIf()
elif cmd == "init":
    print "Checking interfaces initialisation"
    runIf()
elif cmd == "lost":
    print "Checking not added functions to interfaces"
    runLost();
elif cmd == "long":
    print "Checking interfaces usage"
    runLong();
else:
    print "Checking interfaces initialisation"
    runIf()
    print "Checking not added functions to interfaces"
    runLost();
    print "Checking interfaces usage"
    runLong();
exit(tracker.retCode)
