#! /usr/bin/env python
# -*- coding: utf8 -*-
#
# Copyright (C) 2014  Andrei Karas (4144)

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

def readCFile(cFile):
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
                lineRe = re.compile("(?P<ifname>[a-z_]+)->(?P<method>[\w_]+) ")
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
                return (ifname, shortIfName, methods)
    return (None, shortIfName, methods)

def readHFile(hFile, ifname):
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
    return methods

def checkIfFile(tracker, cFile, hFile):
    data = readCFile(cFile)
    cMethods = data[2]
    ifname = data[0]
    shortIfName = data[1]
    if len(cMethods) > 0:
        hMethods = readHFile(hFile, ifname)
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
    print "Checking: " + cFile
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
                        if line[-1] == "\n":
                            line = line[:-1]
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


tracker = Tracker()
tracker.arr = dict()
tracker.methods = Set()
tracker.retCode = 0
if len(sys.argv) > 1 and sys.argv[1] == "silent":
    processIfDir(tracker, "../src/char");
    processIfDir(tracker, "../src/map");
    processIfDir(tracker, "../src/login");
    processIfDir(tracker, "../src/common");
else:
    print "Checking initerfaces initialisation"
    processIfDir(tracker, "../src/char");
    processIfDir(tracker, "../src/map");
    processIfDir(tracker, "../src/login");
    processIfDir(tracker, "../src/common");
    print "Checking interfaces usage"
    processDir(tracker, "../src/char");
    processDir(tracker, "../src/map");
    processDir(tracker, "../src/login");
    processDir(tracker, "../src/common");
    reportMethods(tracker)
exit(tracker.retCode)
