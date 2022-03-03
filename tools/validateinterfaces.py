#! /usr/bin/env python2
# -*- coding: utf8 -*-
#
# This file is part of Hercules.
# http://herc.ws - http://github.com/HerculesWS/Hercules
#
# Copyright (C) 2014-2022 Hercules Dev Team
# Copyright (C) 2014 Andrei Karas (4144)
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

debug = False

interfaceRe = re.compile("struct (?P<name1>[a-z_]+)_interface (?P<name2>[a-z_]+)_s;")

# known methods what must be not added to interfaces
ignoreMethods = (
    "cmdline_args_init_local",
    "map_bl_list_expand",
    "map_block_free_expand",
    "map_zone_mf_cache_add",
    "mob_parse_dbrow_cap_value",
    "clif_calc_delay",
)

# need fix all this names, or add them to ignoreMethods
skipBeforeFix = (
    "atcommand_command_type2idx",
    "atcommand_help_string",
    "chrif_reset",
    "console_handler",
    "console_parse",
    "console_parse_create",
    "console_parse_final",
    "console_parse_init",
    "console_parse_key_pressed",
    "console_parse_list_subs",
    "console_parse_sub",
    "console_parse_timer",
    "console_setSQL",
    "des_E",
    "des_FP",
    "des_IP",
    "des_RoundFunction",
    "des_SBOX",
    "des_TP",
    "duel_leave_sub",
    "duel_savetime",
    "duel_showinfo_sub",
    "grfio_add",
    "grfio_entryread",
    "grfio_filelist_add",
    "grfio_filelist_compact",
    "grfio_filelist_find",
    "grfio_filelist_modify",
    "grfio_is_full_encrypt",
    "grfio_localpath_create",
    "grfio_parse_restable_row",
    "grfio_resourcecheck",
    "guild_storage_additem",
    "guild_storage_delete",
    "guild_storage_delitem",
    "instance_cleanup_sub",
    "instance_del_load",
    "lclif_parse_CA_ACK_MOBILE_OTP",
    "lclif_parse_CA_CHARSERVERCONNECT",
    "lclif_parse_CA_CONNECT_INFO_CHANGED",
    "lclif_parse_CA_EXE_HASHCHECK",
    "lclif_parse_CA_LOGIN",
    "lclif_parse_CA_LOGIN2",
    "lclif_parse_CA_LOGIN3",
    "lclif_parse_CA_LOGIN4",
    "lclif_parse_CA_LOGIN_HAN",
    "lclif_parse_CA_LOGIN_OTP",
    "lclif_parse_CA_LOGIN_PCBANG",
    "lclif_parse_CA_OTP_CODE",
    "lclif_parse_CA_REQ_HASH",
    "lclif_parse_CA_SSO_LOGIN_REQ",
    "lclif_parse_sub",
    "log_atcommand_sub_sql",
    "log_branch_sub_sql",
    "log_chat_sub_sql",
    "log_config_read_database",
    "log_config_read_filter",
    "log_config_read_filter_chat",
    "log_config_read_filter_item",
    "log_mvpdrop_sub_sql",
    "log_npc_sub_sql",
    "log_pick",
    "log_pick_sub_sql",
    "log_zeny_sub_sql",
    "npc_chat_def_pattern",
    "npc_chat_finalize",
    "npc_chat_sub",
    "nullpo_backtrace_get_executable_path",
    "nullpo_backtrace_print",
    "nullpo_error_callback",
    "nullpo_print_callback",
    "pc_bonus_addele",
    "pc_bonus_subele",
    "pc_group_exists",
    "pc_group_get_dummy_group",
    "pc_group_get_idx",
    "pc_group_get_level",
    "pc_group_get_name",
    "pc_group_has_permission",
    "pc_group_id2group",
    "pc_groups_add_permission",
    "pc_group_should_log_commands",
    "pc_groups_reload",
    "pc_setequipindex",
    "quest_reload_check_sub",
    "refine_announce_behavior_string2enum",
    "refine_failure_behavior_string2enum",
    "refine_is_refinable",
    "refine_readdb_refine_libconfig",
    "refine_readdb_refine_libconfig_sub",
    "refine_readdb_refinery_ui_settings",
    "refine_readdb_refinery_ui_settings_items",
    "refine_readdb_refinery_ui_settings_sub",
    "rodex_refresh_stamps",
    "script_array_index_cmp",
    "script_casecheck_add_str_sub",
    "script_casecheck_clear_sub",
    "script_dump_stack",
    "script_global_casecheck_add_str",
    "script_global_casecheck_clear",
    "script_load_translation_sub",
    "script_local_casecheck_add_str",
    "script_local_casecheck_clear",
    "searchstore_getsearchallfunc",
    "searchstore_getsearchfunc",
    "searchstore_getstoreid",
    "searchstore_hasstore",
    "skill_reveal_trap",
    "skill_validate_unit_id_array",
    "skill_validate_unit_id_group",
    "skill_validate_unit_id_value",
    "status_get_matk_sub",
    "status_get_rand_matk",
    "storage_guild_storageadd",
    "storage_guild_storageaddfromcart",
    "storage_guild_storageclose",
    "storage_guild_storageget",
    "storage_guild_storagegettocart",
    "storage_guild_storageopen",
    "storage_guild_storage_quit",
    "storage_guild_storagesave",
    "storage_guild_storagesaved",
    "sysinfo_arch_retrieve",
    "sysinfo_cpu_retrieve",
    "sysinfo_git_get_revision",
    "sysinfo_osversion_retrieve",
    "sysinfo_svn_get_revision",
    "sysinfo_systeminfo_retrieve",
    "sysinfo_vcsrevision_src_retrieve",
    "sysinfo_vcstype_name_retrieve",
    "thread_main_redirector",
    "thread_terminated",
)

class Tracker:
    pass


def searchDefault(r, ifname):
    defaultStr = "void {0}_defaults(void)".format(ifname);
    defaultStr2 = "static void {0}_defaults(void)".format(ifname);
    for line in r:
        if line.find(defaultStr) == 0 or line.find(defaultStr2) == 0:
            return True
    return False


def searchStructStart(r, ifname):
    for line in r:
        if line.find("struct {0}_interface".format(ifname)) == 0:
            return True
    return False


def searchPossibleInterfacesInCFile(tracker, cFile):
    possibleInterfaces = []
    with open(cFile, "r") as r:
        for line in r:
            m = interfaceRe.search(line)
            if m != None:
                name1 = m.group("name1")
                if name1 == m.group("name2"):
                    if debug:
                        line = line.replace("\n", "")
                        print("{0}: Found interface var {1}_interface at line:\n {2}".format(cFile, name1, line))
                    possibleInterfaces.append(name1)
    return possibleInterfaces


def addCInterfaceMethods(tracker, r, ifname):
    methods = set()
    shortIfName = ""
    lineRe = re.compile("(?P<ifname>[a-z_]+)->(?P<method>[\w_]+)[ ][=][ ](?P<fullmethod>[^;]+);")
    for line in r:
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
            # print "{2}: add {0}, from: {1}".format(m.group("method"), line, ifname)
            methods.add(m.group("method"))
            tracker.interfaces.add(ifname);
            tracker.fullmethods.add(m.group("fullmethod"));
    return (ifname, shortIfName, methods)


def readCFile(tracker, cFile):
    possibleInterfaces = searchPossibleInterfacesInCFile(tracker, cFile)
    interfaces = []
    for ifname in possibleInterfaces:
        with open(cFile, "r") as r:
            if searchDefault(r, ifname) == False:
                if debug:
                    print("{0}: Error: no defaults function found: {1}_defaults".format(cFile, ifname))
                    tracker.retCode = 1
                continue
            info = addCInterfaceMethods(tracker, r, ifname)
            interfaces.append(info)
    return interfaces


def readHFile(tracker, hFile, ifname):
    methods = set()
    with open(hFile, "r") as r:
        if searchStructStart(r, ifname) == False:
            print("{0}: Error: No struct found: {1}".format(cFile, ifname))
            tracker.retCode = 1
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
    interfaces = readCFile(tracker, cFile)
    for data in interfaces:
        cMethods = data[2]
        ifname = data[0]
        shortIfName = data[1]
        if len(cMethods) > 0:
            hMethods = readHFile(tracker, hFile, ifname)
            for method in hMethods:
                if method in ignoreMethods:
                    continue
                tracker.arr[ifname + "_" + method] = list()
                tracker.methods.add(ifname + "_" + method)
                if method not in cMethods:
                    print("{0} Error: missing initialisation {1}".format(cFile, method))
                    tracker.retCode = 1


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
    print("\n")
    for method in tracker.methods:
        if method in ignoreMethods:
            continue
        if len(tracker.arr[method]) > 2:
            print(method)
            for t in tracker.arr[method]:
                print(t)
            print("\n")


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
                if name in ignoreMethods:
                    continue
                if name not in tracker.fullmethods and name not in tracker.skipMethods:
#                    print "src  : " + line
                    print(name)
                    tracker.retCode = 1


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
tracker.methods = set()
tracker.fullmethods = set()
tracker.interfaces = set()
tracker.retCode = 0

if len(sys.argv) > 1:
    cmd = sys.argv[1]
else:
    cmd = "default"

tracker.skipMethods = []
if cmd == "silent":
    runIf()
    tracker.skipMethods = skipBeforeFix
    runLost();
elif cmd == "init":
    print("Checking interfaces initialisation")
    runIf()
elif cmd == "lost":
    print("Checking interfaces initialisation")
    runIf()
    print("Checking not added functions to interfaces")
    runLost();
elif cmd == "long":
    print("Checking interfaces usage")
    runLong();
else:
    print("Checking interfaces initialisation")
    runIf()
    print("Checking not added functions to interfaces")
    runLost();
    print("Checking interfaces usage")
    runLong();
exit(tracker.retCode)
