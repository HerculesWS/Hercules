#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
# *        _   _                     _
# *       | | | |                   | |
# *       | |_| | ___ _ __ ___ _   _| | ___  ___
# *       |  _  |/ _ \ '__/ __| | | | |/ _ \/ __|
# *       | | | |  __/ | | (__| |_| | |  __/\__ \
# *       \_| |_/\___|_|  \___|\__,_|_|\___||___/
# *
# * * * * * * * * * * * * * License * * * * * * * * * * * * * * * * * * * * * *
# * This file is part of Hercules.
# * http://herc.ws - http://github.com/HerculesWS/Hercules
# *
# * Copyright (C) 2016-2025 Hercules Dev Team
# * Copyright (C) 2016 Smokexyz
# *
# * Hercules is free software: you can redistribute it and/or modify
# * it under the terms of the GNU General Public License as published by
# * the Free Software Foundation, either version 3 of the License, or
# * (at your option) any later version.
# *
# * This program is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# * along with this program.  If not, see <http://www.gnu.org/licenses/>.
# * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
# * Credits : Smokexyz

import sys
import os
import re
import time
import math
import tracemalloc


# ---------------------------------------------------------------------------
# PHP-semantics helper functions
#
# These exist to faithfully reproduce PHP's loose-typing / string-vs-number
# comparison rules, which the original script relies on (sometimes exactly,
# sometimes seemingly by accident) in many of its conditionals.
# ---------------------------------------------------------------------------

_NUM_RE = re.compile(r'^[+-]?(\d+(\.\d*)?|\.\d+)([eE][+-]?\d+)?$')
_INT_RE = re.compile(r'^\s*[+-]?\d+')
_HEX_RE = re.compile(r'^\s*[+-]?(0[xX])?[0-9a-fA-F]+')
_NUMPREFIX_RE = re.compile(r'^\s*[+-]?(\d+\.\d*|\.\d+|\d+)([eE][+-]?\d+)?')


def is_numeric_str(s):
    """Mirrors PHP's is_numeric() for strings (used to decide == comparison mode)."""
    if s is None or not isinstance(s, str):
        return False
    t = s.strip()
    if t == "":
        return False
    return bool(_NUM_RE.match(t))


def php_s(v):
    """Stringify a value the way PHP would when concatenating (NULL -> "")."""
    if v is None:
        return ""
    return str(v)


def php_truthy(v):
    """PHP loose truthiness: None, "", "0", 0, 0.0, False are falsy."""
    if v is None:
        return False
    if isinstance(v, bool):
        return v
    if isinstance(v, str):
        return v != "" and v != "0"
    if isinstance(v, (int, float)):
        return v != 0
    return bool(v)


def php_eq(a, b):
    """
    Mimics PHP8 loose == comparison for the str/int/None/bool values that
    appear in this script. If either side is None or bool, PHP compares both
    sides as booleans. Otherwise, if both sides look like numeric strings,
    compares numerically; else compares as strings.
    """
    if a is None or isinstance(a, bool) or b is None or isinstance(b, bool):
        return php_truthy(a) == php_truthy(b)
    sa = a if isinstance(a, str) else str(a)
    sb = b if isinstance(b, str) else str(b)
    if is_numeric_str(sa) and is_numeric_str(sb):
        return float(sa) == float(sb)
    return sa == sb


def php_neq(a, b):
    return not php_eq(a, b)


def php_gt_int(a, b):
    """Mimics PHP8 `$a > $b` where $b is an int literal and $a a raw field."""
    sa = "" if a is None else str(a)
    if is_numeric_str(sa):
        return float(sa) > b
    return sa > str(b)


def php_lt_int(a, b):
    """Mimics PHP8 `$a < $b` where $b is an int literal and $a a raw field."""
    sa = "" if a is None else str(a)
    if is_numeric_str(sa):
        return float(sa) < b
    return sa < str(b)


def php_intval(v, base=10):
    """Mimics PHP's intval()."""
    if v is None:
        return 0
    if isinstance(v, bool):
        return 1 if v else 0
    if isinstance(v, int):
        return v
    if isinstance(v, float):
        return int(v)
    s = str(v)
    if base == 16:
        m = _HEX_RE.match(s)
        if not m:
            return 0
        text = m.group().strip()
        try:
            return int(text, 16)
        except ValueError:
            return 0
    else:
        m = _INT_RE.match(s)
        if not m:
            return 0
        return int(m.group())


def php_num(v):
    """
    Mimics PHP's automatic string-to-number coercion used in arithmetic
    contexts (e.g. `$lvs[$i] - $lvs[$j]`). Leading numeric prefix is parsed,
    non-numeric leading content yields 0.
    """
    if v is None:
        return 0
    if isinstance(v, bool):
        return 1 if v else 0
    if isinstance(v, (int, float)):
        return v
    s = str(v)
    m = _NUMPREFIX_RE.match(s)
    if not m:
        return 0
    text = m.group()
    if '.' in text or 'e' in text or 'E' in text:
        try:
            return float(text)
        except ValueError:
            return 0
    return int(text)


def php_hexdec(v):
    """Mimics PHP's hexdec(): strips all non-hex characters, then parses."""
    if v is None:
        return 0
    s = re.sub(r'[^0-9a-fA-F]', '', str(v))
    return int(s, 16) if s else 0


def php_colon_truthy(s):
    """
    Mimics the (likely unintended) PHP idiom `strpos($x, ':') == true`.
    PHP's strpos() returns the int position or FALSE. `== true` loosely
    compares: position 0 is falsy (0 == true is FALSE in PHP), so a colon
    found at position 0 is treated as "not found". Only position >= 1 counts.
    """
    if s is None:
        return False
    return str(s).find(':') > 0


def ucfirst(s):
    """Mimics PHP's ucfirst(): uppercase only the first character."""
    s = php_s(s)
    return (s[:1].upper() + s[1:]) if s else s


def fget(arr, idx, default=None):
    """Safe positional access mimicking PHP's `isset($arr[$idx])?$arr[$idx]:default`
    for out-of-range access (PHP would instead emit a NULL + warning, but never
    crash) -- used generously here so malformed/short lines don't raise."""
    return arr[idx] if 0 <= idx < len(arr) else default


def array_search_id(target, id_map):
    """
    Mimics PHP's array_search($target, $id_map) with loose (==) comparison,
    scanning in the map's insertion order (matches PHP array iteration order
    for arrays built purely via sequential integer-key assignment).
    Returns the matching key, or None (PHP FALSE).
    """
    for k, v in id_map.items():
        if php_eq(v, target):
            return k
    return None


# ---------------------------------------------------------------------------
# Formatter / helper functions (ported 1:1 from the PHP source)
# ---------------------------------------------------------------------------

def microtime_float():
    return time.time()


def show_status(done, total):
    perc = math.floor((done / total) * 100)
    perc = int(math.floor(perc / 2))
    left = 50 - perc
    finalperc = perc * 2
    write = "[" + ("=" * perc) + ">" + (" " * left) + "] - " + str(finalperc) + "% - " + str(done) + "/" + str(total) + "\r"
    sys.stderr.write(write)
    sys.stderr.flush()


def get_element(ele, id_):
    cases = [
        (-1, "Ele_Weapon"), (-2, "Ele_Endowed"), (-3, "Ele_Random"),
        (0, "Ele_Neutral"), (1, "Ele_Water"), (2, "Ele_Earth"), (3, "Ele_Fire"),
        (4, "Ele_Wind"), (5, "Ele_Poison"), (6, "Ele_Holy"), (7, "Ele_Dark"),
        (8, "Ele_Ghost"), (9, "Ele_Undead"),
    ]
    for val, name in cases:
        if php_eq(ele, val):
            return name
    sys.stdout.write("\rWarningUnknown Element " + php_s(ele) + " provided for skill Id " + php_s(id_) + "\n")
    return None


def _tab_presets(tab):
    if tab == 1:
        return "\t\t\t", "\t\t"
    elif tab == 2:
        return "\t\t\t\t", "\t\t\t"
    else:
        return "\t\t", "\t"


def leveled_ele(s, max_, skill_id):
    if php_colon_truthy(s):
        lvs = php_s(s).split(":")
        retval = "{\n"
        i = 0
        while i < max_ and i < len(lvs):
            retval += "\t\tLv" + str(i + 1) + ": \"" + php_s(get_element(lvs[i], skill_id)) + "\"\n"
            i += 1
        retval += "\t}"
    else:
        retval = "\"" + php_s(get_element(s, skill_id)) + "\""
    return retval


def leveled(s, max_, id_, tab=0):
    ittab, endtab = _tab_presets(tab)
    if php_colon_truthy(s):
        lvs = php_s(s).strip().split(":")
        retval = "{\n"
        i = 0
        while i < max_ and i < len(lvs):
            retval += ittab + "Lv" + str(i + 1) + ": " + php_s(lvs[i]) + "\n"
            i += 1
        retval += endtab + "}"
        return retval
    else:
        return php_intval(s)


def leveled_guessfill(s, max_level, id_, tab=0):
    ittab, endtab = _tab_presets(tab)
    if not php_colon_truthy(s):
        return php_intval(s)

    lvs = php_s(s).strip().split(":")
    retval = "{\n"
    i = 0
    while i < max_level and i < len(lvs):
        retval += ittab + "Lv" + str(i + 1) + ": " + php_s(lvs[i]) + "\n"
        i += 1

    if i < max_level:
        # Algorithm borrowed from skill_split_atoi(), as used by the old parser.
        step = 1
        while step <= i / 2:
            diff = php_num(lvs[i - 1]) - php_num(lvs[i - step - 1])
            j = i - 1
            while j >= step:
                if php_num(lvs[j]) - php_num(lvs[j - step]) != diff:
                    break
                j -= 1

            if j >= step:  # No match, try next step.
                step += 1
                continue

            # Match found: apply linear increase.
            while i < max_level:
                newval = php_num(lvs[i - step]) + diff
                if newval < 1 and php_num(lvs[i - 1]) >= 0:
                    # Check if we have switched from + to -, cap the decrease to 0 in said cases.
                    newval = 1
                    diff = 0
                    step = 1
                if i < len(lvs):
                    lvs[i] = newval
                else:
                    lvs.append(newval)
                retval += ittab + "Lv" + str(i + 1) + ": " + str(newval) + "\n"
                i += 1
            retval += endtab + "}"
            return retval

        # Okay.. we can't figure this one out, just fill out the stuff with the previous value.
        while i < max_level:
            newval = php_num(lvs[i - 1])
            if i < len(lvs):
                lvs[i] = newval
            else:
                lvs.append(newval)
            retval += ittab + "Lv" + str(i + 1) + ": " + str(newval) + "\n"
            i += 1

    retval += endtab + "}"
    return retval


def getstate(state, id_):
    mapping = {
        "hiding": "Hiding",
        "cloaking": "Cloaking",
        "hidden": "Hidden",
        "riding": "Riding",
        "falcon": "Falcon",
        "cart": "Cart",
        "shield": "Shield",
        "sight": "Sight",
        "explosionspirits": "ExplosionSpirits",
        "cartboost": "CartBoost",
        "recover_weight_rate": "NotOverWeight",
        "move_enable": "Moveable",
        "water": "InWater",
        "dragon": "Dragon",
        "warg": "Warg",
        "ridingwarg": "RidingWarg",
        "mado": "MadoGear",
        "elementalspirit": "ElementalSpirit",
        "poisonweapon": "PoisonWeapon",
        "rollingcutter": "RollingCutter",
        "mh_fighting": "MH_Fighting",
        "mh_grappling": "MH_Grappling",
        "peco": "Peco",
    }
    key = php_s(state)
    if key in mapping:
        return mapping[key]
    sys.stdout.write("\rWarning - Invalid State " + php_s(state) + " provided for Skill ID " + php_s(id_) + ", please correct this manually.\n")
    return None


def getinf(inf):
    inf_i = php_intval(inf)
    bitmask = [("Passive", 0), ("Enemy", 1), ("Place", 2), ("Self", 4), ("Friend", 16), ("Trap", 32)]
    retval = "{\n"
    for key, val in bitmask:
        if inf_i & val:
            retval += "\t\t" + key + ": true\n"
    retval += "\t}"
    return retval


def getinf2(inf2=0x0000):
    bitmask = [
        ("Quest", 0x0001), ("NPC", 0x0002), ("Wedding", 0x0004), ("Spirit", 0x0008),
        ("Guild", 0x0010), ("Song", 0x0020), ("Ensemble", 0x0040), ("Trap", 0x0080),
        ("TargetSelf", 0x0100), ("NoCastSelf", 0x0200), ("PartyOnly", 0x0400),
        ("GuildOnly", 0x0800), ("NoEnemy", 0x1000), ("IgnoreLandProtector", 0x2000),
        ("Chorus", 0x4000),
    ]
    inf2_i = php_intval(php_s(inf2)[2:], 16)
    retval = "{\n"
    for key, val in bitmask:
        if inf2_i & val:
            retval += "\t\t" + key + ": true\n"
    retval += "\t}"
    return retval


def getnk(nk):
    bitmask = [
        ("NoDamage", 0x01), ("SplashArea", 0x02), ("SplitDamage", 0x04),
        ("IgnoreCards", 0x08), ("IgnoreElement", 0x10), ("IgnoreDefense", 0x20),
        ("IgnoreFlee", 0x40), ("IgnoreDefCards", 0x80),
    ]
    nk_i = php_intval(nk, 16)
    retval = "{\n"
    for key, val in bitmask:
        if nk_i & val:
            retval += "\t\t" + key + ": true\n"
    retval += "\t}"
    return retval


def getnocast(opt, id_):
    bitmask = [("Default", 0), ("IgnoreDex", 1), ("IgnoreStatusEffect", 2), ("IgnoreItemBonus", 4)]
    opt_i = php_intval(opt)
    bitsum = sum(v for _, v in bitmask)
    if opt_i > bitsum or opt_i < 0:
        sys.stdout.write("\rWarning - a bitmask for CastNoDex entry for skill ID " + php_s(id_) + " is higher than total of masks or lower than 0.")
    retval = "{\n"
    for key, val in bitmask:
        if opt_i & val:
            retval += "\t\t" + key + ": true\n"
    retval += "\t}"
    return retval


def getweapontypes(list_, id_):
    bitmask = {
        0: "NoWeapon", 1: "Daggers", 2: "1HSwords", 3: "2HSwords", 4: "1HSpears",
        5: "2HSpears", 6: "1HAxes", 7: "2HAxes", 8: "Maces", 9: "2HMaces",
        10: "Staves", 11: "Bows", 12: "Knuckles", 13: "Instruments", 14: "Whips",
        15: "Books", 16: "Katars", 17: "Revolvers", 18: "Rifles", 19: "GatlingGuns",
        20: "Shotguns", 21: "GrenadeLaunchers", 22: "FuumaShurikens", 23: "2HStaves",
        24: "MaxSingleWeaponType", 25: "DWDaggers", 26: "DWSwords", 27: "DWAxes",
        28: "DWDaggerSword", 29: "DWDaggerAxe", 30: "DWSwordAxe",
    }
    if php_colon_truthy(list_):
        type_list = php_s(list_).split(":")
        type_vals = [php_intval(t) for t in type_list]
        wmask = 0
        for i, tv in enumerate(type_vals):
            wmask |= 1 << tv
            if tv > 30 or tv < 0:
                sys.stdout.write("\rWarning - Invalid weapon type " + str(i) + " for skill ID " + php_s(id_) + "\n")
        retval = "{\n"
        for j, tv in enumerate(type_vals):
            if wmask & (1 << tv):
                retval += "\t\t\t" + php_s(bitmask.get(tv)) + ": true\n"
        retval += "\t\t}"
    else:
        retval = "{\n"
        retval += "\t\t\t" + php_s(bitmask.get(php_intval(list_))) + ": true\n"
        retval += "\t\t}"
    return retval


def getammotypes(list_, id_):
    bitmask = {
        1: "A_ARROW", 2: "A_DAGGER", 3: "A_BULLET", 4: "A_SHELL", 5: "A_GRENADE",
        6: "A_SHURIKEN", 7: "A_KUNAI", 8: "A_CANNONBALL", 9: "A_THROWWEAPON",
    }
    if php_colon_truthy(list_):
        type_list = php_s(list_).split(":")
        type_vals = [php_intval(t) for t in type_list]
        wmask = 0
        for i, tv in enumerate(type_vals):
            wmask |= 1 << tv
            if tv > 9 or tv < 1:
                sys.stdout.write("\rWarning - Invalid weapon type " + str(i) + " for skill ID " + php_s(id_) + "\n")
        retval = "{\n"
        for j, tv in enumerate(type_vals):
            if wmask & (1 << tv):
                retval += "\t\t\t" + php_s(bitmask.get(tv)) + ": true\n"
        retval += "\t\t}"
    else:
        retval = "{\n"
        retval += "\t\t\t" + php_s(bitmask.get(php_intval(list_))) + ": true\n"
        retval += "\t\t}"
    return retval


def getunitflag(flag, id_):
    bitmask = [
        ("UF_DEFNOTENEMY", 0x001), ("UF_NOREITERATION", 0x002), ("UF_NOFOOTSET", 0x004),
        ("UF_NOOVERLAP", 0x008), ("UF_PATHCHECK", 0x010), ("UF_NOPC", 0x020),
        ("UF_NOMOB", 0x040), ("UF_SKILL", 0x080), ("UF_DANCE", 0x100),
        ("UF_ENSEMBLE", 0x200), ("UF_SONG", 0x400), ("UF_DUALMODE", 0x800),
        ("UF_RANGEDSINGLEUNIT", 0x2000),
    ]
    flag_i = php_intval(flag)
    if flag_i <= 0:
        return 0

    ret = "{\n"
    for key, val in bitmask:
        if flag_i & val:
            ret += "\t\t\t" + key + ": true\n"

    bitsum = sum(v for _, v in bitmask)
    if flag_i > bitsum:
        sys.stdout.write("\rWarning - Invalid Unit Flag " + php_s(flag) + " provided for skill Id " + php_s(id_) + "\n")

    ret += "\t\t}"
    return ret


def print_mem():
    if tracemalloc.is_tracing():
        current, _peak = tracemalloc.get_traced_memory()
    else:
        current = 0
    return convert(current)


def convert(size):
    units = ['b', 'kb', 'mb', 'gb', 'tb', 'pb']
    if size <= 0:
        return "0 " + units[0]
    i = int(math.floor(math.log(size, 1024)))
    i = max(0, min(i, len(units) - 1))
    val = round(size / (1024 ** i), 2)
    return str(val) + " " + units[i]


def gethelp():
    p("Usage: php skilldbconverter.php [option]\n")
    p("Options:\n")
    p("\t-re     [--renewal]          for renewal skill database conversion.\n")
    p("\t-pre-re [--pre-renewal]      for pre-renewal skill database conversion.\n")
    p("\t-itid   [--use-itemid]       to use item IDs instead of constants.\n")
    p("\t-dir    [--directory]        provide a custom directory.\n")
    p("\t                             (Must include the correct -pre-re/-re option)\n")
    p("\t-dbg    [--with-debug]       print debug information.\n")
    p("\t-h      [--help]             to display this help text.\n\n")
    p("----------------------- Additional Notes ----------------------\n")
    p("Important!\n")
    p("* Please be advised that either and only one of the arguments -re/-pre-re\n")
    p("  must be specified on execution.\n")
    p("* When using the -dir option, -re/-pre-re options must be specified. \n")
    p("* This tool isn't designed to convert renewal data to pre-renewal.\n")
    p("* This tool should ideally be used from the 'tools/' folder, which can be found\n")
    p("  in the root of your Hercules installation. This tool will not delete any files\n")
    p("  from any of the directories that it reads from or prints to.\n\n")
    p("* Prior to using this tool, please ensure at least 30MB of free RAM.\n")
    p("----------------------- Usage Example -------------------------\n")
    p("- Renewal Conversion: php skilldbconverter.php --renewal\n")
    p("- Pre-renewal Conversion: php skilldbconverter.php --pre-renewal\n")
    p("----------------------------------------------------------------\n")
    sys.exit(0)


def printcredits():
    p(
        "      _   _                     _           \n"
        "     | | | |                   | |          \n"
        "     | |_| | ___ _ __ ___ _   _| | ___  ___ \n"
        "     |  _  |/ _ \\ '__/ __| | | | |/ _ \\/ __|\n"
        "     | | | |  __/ | | (__| |_| | |  __/\\__ \\ \n"
        "     \\_| |_/\\___|_|  \\___|\\__,_|_|\\___||___/\n"
        "Hercules Skill Database TXT to Libconfig Converter by Smokexyz\n"
        "Copyright (C) 2016-2025 Hercules Dev Team\n"
        "-----------------------------------------------\n\n"
    )


# The four pieces below are the exact text of getcomments()'s PHP return
# expression (extracted programmatically from the PHP source and verified via
# repr() round-tripping to avoid any manual-transcription escaping mistakes).
# The PHP source builds this as:
#   PART1 . ($re?"Renewal":"Pre-Renewal") . PART2 . ($re?PART3A:"") . PART4
_GC_PART1 = "//================= Hercules Database ==========================================\n//=       _   _                     _\n//=      | | | |                   | |\n//=      | |_| | ___ _ __ ___ _   _| | ___  ___\n//=      |  _  |/ _ \\ '__/ __| | | | |/ _ \\/ __|\n//=      | | | |  __/ | | (__| |_| | |  __/\\__ \\\n//=      \\_| |_/\\___|_|  \\___|\\__,_|_|\\___||___/\n//================= License ====================================================\n//= This file is part of Hercules.\n//= http://herc.ws - http://github.com/HerculesWS/Hercules\n//=\n//= Copyright (C) 2014-2025 Hercules Dev Team\n//=\n//= Hercules is free software: you can redistribute it and/or modify\n//= it under the terms of the GNU General Public License as published by\n//= the Free Software Foundation, either version 3 of the License, or\n//= (at your option) any later version.\n//=\n//= This program is distributed in the hope that it will be useful,\n//= but WITHOUT ANY WARRANTY; without even the implied warranty of\n//= MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n//= GNU General Public License for more details.\n//=\n//= You should have received a copy of the GNU General Public License\n//= along with this program.  If not, see <http://www.gnu.org/licenses/>.\n//==============================================================================\n//= "

_GC_PART2 = ' Skill Database [Hercules]\n//==============================================================================\n//= @Format Notes:\n//= - All string entries are case-sensitive and must be quoted.\n//= - All setting names are case-sensitive and must be keyed accurately.\n\n\n/******************************************************************************\n********************************* Entry structure *****************************\n*******************************************************************************\n{\n\t// ------------------------------ Mandatory Fields ----------------------------\n\tId: ID                                      (int)     (Required)\n\tName: "Skill Name"                          (string)  (Required)\n\tMaxLevel: Skill Level                       (int)     (Required)\n\t// ------------------------------ Optional Fields -----------------------------\n\tDescription: "Skill Description"            (string)  (optional but recommended)\n\tRange: Skill Range                          (int) (optional, defaults to 0) (can be grouped by Levels)\n\t                                            Note: Range < 5 is considered Melee range.\n\tHit: Hit Type                               (int) (optional, default "BDT_NORMAL")\n\t                                            Types - "BDT_SKILL", "BDT_MULTIHIT" or "BDT_NORMAL"\n\tSkillType: {                                (bool, defaults to "Passive")\n\t\tPassive: true/false                     (boolean, defaults to false)\n\t\tEnemy: true/false                       (boolean, defaults to false)\n\t\tPlace: true/false                       (boolean, defaults to false)\n\t\tSelf: true/false                        (boolean, defaults to false)\n\t\tFriend: true/false                      (boolean, defaults to false)\n\t\tTrap: true/false                        (boolean, defaults to false)\n\t}\n\tSkillInfo: {                                (bool, defaults to "None")\n\t\tQuest: true/false                       (boolean, defaults to false)\n\t\tNPC: true/false                         (boolean, defaults to false)\n\t\tWedding: true/false                     (boolean, defaults to false)\n\t\tSpirit: true/false                      (boolean, defaults to false)\n\t\tGuild: true/false                       (boolean, defaults to false)\n\t\tSong: true/false                        (boolean, defaults to false)\n\t\tEnsemble: true/false                    (boolean, defaults to false)\n\t\tTrap: true/false                        (boolean, defaults to false)\n\t\tTargetSelf: true/false                  (boolean, defaults to false)\n\t\tNoCastSelf: true/false                  (boolean, defaults to false)\n\t\tPartyOnly: true/false                   (boolean, defaults to false)\n\t\tGuildOnly: true/false                   (boolean, defaults to false)\n\t\tNoEnemy: true/false                     (boolean, defaults to false)\n\t\tIgnoreLandProtector: true/false         (boolean, defaults to false)\n\t\tChorus: true/false                      (boolean, defaults to false)\n\t\tFreeCastReduced: true/false             (boolean, defaults to false)\n\t\t\t\t\t\t\tWorks like skill SA_FREECAST, allow move and attack with reduced speed.\n\t\tFreeCastNormal: true/false              (boolean, defaults to false)\n\t\t\t\t\t\t\tWorks like FreeCastReduced, but not reduce speed.\n\t}\n\tAttackType: "Attack Type"                   (string, defaults to "None")\n\t                                            Types: "None", "Weapon", "Magic" or "Misc"\n\tElement: "Element Type"                     (string) (Optional field - Default "Ele_Neutral")\n\t                                            (can be grouped by Levels)\n\t                                            Types: "Ele_Neutral", "Ele_Water", "Ele_Earth", "Ele_Fire", "Ele_Wind"\n\t                                            "Ele_Poison", "Ele_Holy", "Ele_Dark", "Ele_Ghost", "Ele_Undead"\n\t                                            "Ele_Weapon" - Uses weapon\'s element.\n\t                                            "Ele_Endowed" - Uses Endowed element.\n\t                                            "Ele_Random" - Uses random element.\n\tDamageType: {                               (bool, default to "NoDamage")\n\t\tNoDamage: true/false                     No damage skill\n\t\tSplashArea: true/false                   Has splash area (requires source modification)\n\t\tSplitDamage: true/false                  Damage should be split among targets (requires \'SplashArea\' in order to work)\n\t\tIgnoreCards: true/false                  Skill ignores caster\'s % damage cards (misc type always ignores)\n\t\tIgnoreElement: true/false                Skill ignores elemental adjustments\n\t\tIgnoreDefense: true/false                Skill ignores target\'s defense (misc type always ignores)\n\t\tIgnoreFlee: true/false                   Skill ignores target\'s flee (magic type always ignores)\n\t\tIgnoreDefCards: true/false               Skill ignores target\'s def cards\n\t}\n\tSplashRange: Damage Splash Area             (int, defaults to 0) (can be grouped by Levels)\n\t                                            Note: -1 for screen-wide.\n\tNumberOfHits: Number of Hits                (int, defaults to 1) (can be grouped by Levels)\n\t                                            Note: when positive, damage is increased by hits,\n\t                                            negative values just show number of hits without\n\t                                            increasing total damage.\n\tInterruptCast: Cast Interruption            (bool, defaults to false)\n\tCastDefRate: Cast Defense Reduction         (int, defaults to 0)\n\tSkillInstances: Skill instances             (int, defaults to 0) (can be grouped by Levels)\n\t                                            Notes: max amount of skill instances to place on the ground when\n\t                                            player_land_skill_limit/monster_land_skill_limit is enabled. For skills\n\t                                            that attack using a path, this is the path length to be used.\n\tKnockBackTiles: Knock-back by \'n\' Tiles     (int, defaults to 0) (can be grouped by Levels)\n\tCastTime: Skill cast Time (in ms)           (int, defaults to 0) (can be grouped by Levels)\n\tAfterCastActDelay: Skill Delay (in ms)      (int, defaults to 0) (can be grouped by Levels)\n\tAfterCastWalkDelay: Walk Delay (in ms)      (int, defaults to 0) (can be grouped by Levels)\n\tSkillData1: Skill Data/Duration (in ms)     (int, defaults to 0) (can be grouped by Levels)\n\tSkillData2: Skill Data/Duration (in ms)     (int, defaults to 0) (can be grouped by Levels)\n\tCoolDown: Skill Cooldown (in ms)            (int, defaults to 0) (can be grouped by Levels)\n\t'

_GC_PART3A = 'FixedCastTime: Fixed Cast Time (in ms)      (int, defaults to 0) (can be grouped by Levels)\n\t                                            Note: when 0, uses 20% of cast time and less than\n\t                                            0 means no fixed cast time.'

_GC_PART4 = '\n\tCastTimeOptions: {\n\t\tIgnoreDex: true/false                   (boolean, defaults to false)\n\t\tIgnoreStatusEffect: true/false          (boolean, defaults to false)\n\t\tIgnoreItemBonus: true/false             (boolean, defaults to false)\n\t\t                                        Note: Delay setting \'IgnoreDex\' only makes sense when\n\t\t                                        delay_dependon_dex is enabled.\n\t}\n\tSkillDelayOptions: {\n\t\tIgnoreDex: true/false                   (boolean, defaults to false)\n\t\tIgnoreStatusEffect: true/false          (boolean, defaults to false)\n\t\tIgnoreItemBonus: true/false             (boolean, defaults to false)\n\t\t                                        Note: Delay setting \'IgnoreDex\' only makes sense when\n\t\t                                        delay_dependon_dex is enabled.\n\t}\n\tRequirements: {\n\t\tHPCost: HP Cost                         (int, defaults to 0) (can be grouped by Levels)\n\t\tSPCost: SP Cost                         (int, defaults to 0) (can be grouped by Levels)\n\t\tHPRateCost: HP % Cost                   (int, defaults to 0) (can be grouped by Levels)\n\t\t                                        Note: If positive, it is a percent of your current hp,\n\t\t                                        otherwise it is a percent of your max hp.\n\t\tSPRateCost: SP % Cost                   (int, defaults to 0) (can be grouped by Levels)\n\t\t                                        Note: If positive, it is a percent of your current sp,\n\t\t                                        otherwise it is a percent of your max sp.\n\t\tZenyCost: Zeny Cost                     (int, defaults to 0) (can be grouped by Levels)\n\t\tWeaponTypes: {                          (bool or string, defaults to "All")\n\t\t\tNoWeapon: true/false                (boolean, defaults to false)\n\t\t\tDaggers: true/false                 (boolean, defaults to false)\n\t\t\t1HSwords: true/false                (boolean, defaults to false)\n\t\t\t2HSwords: true/false                (boolean, defaults to false)\n\t\t\t1HSpears: true/false                (boolean, defaults to false)\n\t\t\t2HSpears: true/false                (boolean, defaults to false)\n\t\t\t1HAxes: true/false                  (boolean, defaults to false)\n\t\t\t2HAxes: true/false                  (boolean, defaults to false)\n\t\t\tMaces: true/false                   (boolean, defaults to false)\n\t\t\t2HMaces: true/false                 (boolean, defaults to false)\n\t\t\tStaves: true/false                  (boolean, defaults to false)\n\t\t\tBows: true/false                    (boolean, defaults to false)\n\t\t\tKnuckles: true/false                (boolean, defaults to false)\n\t\t\tInstruments: true/false             (boolean, defaults to false)\n\t\t\tWhips: true/false                   (boolean, defaults to false)\n\t\t\tBooks: true/false                   (boolean, defaults to false)\n\t\t\tKatars: true/false                  (boolean, defaults to false)\n\t\t\tRevolvers: true/false               (boolean, defaults to false)\n\t\t\tRifles: true/false                  (boolean, defaults to false)\n\t\t\tGatlingGuns: true/false             (boolean, defaults to false)\n\t\t\tShotguns: true/false                (boolean, defaults to false)\n\t\t\tGrenadeLaunchers: true/false        (boolean, defaults to false)\n\t\t\tFuumaShurikens: true/false          (boolean, defaults to false)\n\t\t\t2HStaves: true/false                (boolean, defaults to false)\n\t\t\tMaxSingleWeaponType: true/false     (boolean, defaults to false)\n\t\t\tDWDaggers: true/false               (boolean, defaults to false)\n\t\t\tDWSwords: true/false                (boolean, defaults to false)\n\t\t\tDWAxes: true/false                  (boolean, defaults to false)\n\t\t\tDWDaggerSword: true/false           (boolean, defaults to false)\n\t\t\tDWDaggerAxe: true/false             (boolean, defaults to false)\n\t\t\tDWSwordAxe: true/false              (boolean, defaults to false)\n\t\t}\n\t\tAmmoTypes: {                            (for all types use string "All")\n\t\t\tA_ARROW: true/false                 (boolean, defaults to false)\n\t\t\tA_DAGGER: true/false                (boolean, defaults to false)\n\t\t\tA_BULLET: true/false                (boolean, defaults to false)\n\t\t\tA_SHELL: true/false                 (boolean, defaults to false)\n\t\t\tA_GRENADE: true/false               (boolean, defaults to false)\n\t\t\tA_SHURIKEN: true/false              (boolean, defaults to false)\n\t\t\tA_KUNAI: true/false                 (boolean, defaults to false)\n\t\t\tA_CANNONBALL: true/false            (boolean, defaults to false)\n\t\t\tA_THROWWEAPON: true/false           (boolean, defaults to false)\n\t\t}\n\t\tAmmoAmount: Ammunition Amount           (int, defaults to 0) (can be grouped by Levels)\n\t\tState: "Required State"                 (string, defaults to "None") (can be grouped by Levels)\n\t\t                                        Types : \'None\' = Nothing special\n\t\t                                        \'Moveable\' = Requires to be able to move\n\t\t                                        \'NotOverWeight\' = Requires to be less than 50% weight\n\t\t                                        \'InWater\' = Requires to be standing on a water cell\n\t\t                                        \'Cart\' = Requires a Pushcart\n\t\t                                        \'Riding\' = Requires to ride either a peco or a dragon\n\t\t                                        \'Falcon\' = Requires a Falcon\n\t\t                                        \'Sight\' = Requires Sight skill activated\n\t\t                                        \'Hiding\' = Requires Hiding skill activated\n\t\t                                        \'Cloaking\' = Requires Cloaking skill activated\n\t\t                                        \'ExplosionSpirits\' = Requires Fury skill activated\n\t\t                                        \'CartBoost\' = Requires a Pushcart and Cart Boost skill activated\n\t\t                                        \'Shield\' = Requires a 0,shield equipped\n\t\t                                        \'Warg\' = Requires a Warg\n\t\t                                        \'Dragon\' = Requires to ride a Dragon\n\t\t                                        \'RidingWarg\' = Requires to ride a Warg\n\t\t                                        \'Mado\' = Requires to have an active mado\n\t\t                                        \'PoisonWeapon\' = Requires to be under Poisoning Weapon.\n\t\t                                        \'RollingCutter\' = Requires at least one Rotation Counter from Rolling Cutter.\n\t\t                                        \'ElementalSpirit\' = Requires to have an Elemental Spirit summoned.\n\t\t                                        \'MH_Fighting\' = Requires Eleanor fighthing mode\n\t\t                                        \'MH_Grappling\' = Requires Eleanor grappling mode\n\t\t                                        \'Peco\' = Requires riding a peco\n\t\tSpiritSphereCost: Spirit Sphere Cost    (int, defaults to 0) (can be grouped by Levels)\n\t\tItems: {\n\t\t\tItemID or Aegis_Name : Amount       (int, defaults to 0) (can be grouped by Levels)\n\t\t\t                                    Item example: "ID717" or "Blue_Gemstone".\n\t\t\t                                    Notes: Items with amount 0 will not be consumed.\n\t\t\t                                    Amount can also be grouped by levels.\n\t\t}\n\t}\n\tUnit: {\n\t\tId: [ UnitID, UnitID2 ]                 (int, defaults to 0) (can be grouped by Levels)\n\t\tLayout: Unit Layout                     (int, defaults to 0) (can be grouped by Levels)\n\t\tRange: Unit Range                       (int, defaults to 0) (can be grouped by Levels)\n\t\tInterval: Unit Interval                 (int, defaults to 0) (can be grouped by Levels)\n\t\tTarget: "Unit Target"                   (string, defaults to "None")\n\t\t                                        Types:\n\t\t                                        All             - affects everyone\n\t\t                                        NotEnemy        - affects anyone who isn\'t an enemy\n\t\t                                        Friend          - affects party, guildmates and neutral players\n\t\t                                        Party           - affects party only\n\t\t                                        Guild           - affects guild only\n\t\t                                        Ally            - affects party and guildmates only\n\t\t                                        Sameguild       - affects guild but not allies\n\t\t                                        Enemy           - affects enemies only\n\t\t                                        None            - affects nobody\n\t\tFlag: {\n\t\t\tUF_DEFNOTENEMY: true/false          (boolean, defaults to false)\n\t\t\tUF_NOREITERATION: true/false        (boolean, defaults to false)\n\t\t\tUF_NOFOOTSET: true/false            (boolean, defaults to false)\n\t\t\tUF_NOOVERLAP: true/false            (boolean, defaults to false)\n\t\t\tUF_PATHCHECK: true/false            (boolean, defaults to false)\n\t\t\tUF_NOPC: true/false                 (boolean, defaults to false)\n\t\t\tUF_NOMOB: true/false                (boolean, defaults to false)\n\t\t\tUF_SKILL: true/false                (boolean, defaults to false)\n\t\t\tUF_DANCE: true/false                (boolean, defaults to false)\n\t\t\tUF_ENSEMBLE: true/false             (boolean, defaults to false)\n\t\t\tUF_SONG: true/false                 (boolean, defaults to false)\n\t\t\tUF_DUALMODE: true/false             (boolean, defaults to false)\n\t\t\tUF_RANGEDSINGLEUNI: true/false      (boolean, defaults to false)\n\t\t}\n\t}\n}\n* This file has been generated by Smokexyz\'s skilldbconverter.php tool.\n* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */\n\n'


def getcomments(re_):
    return (
        _GC_PART1
        + ("Renewal" if re_ else "Pre-Renewal")
        + _GC_PART2
        + (_GC_PART3A if re_ else "")
        + _GC_PART4
    )


def extract_description(field):
    """Ports the `strpos($arr[16], "//")` truncation-with-fallback quirk."""
    field = php_s(field)
    idx = field.find("//")
    candidate = field[:idx] if idx != -1 else ""
    return candidate if len(candidate) > 0 else field


# ---------------------------------------------------------------------------
# I/O helpers
# ---------------------------------------------------------------------------

def p(s):
    """Equivalent of PHP's print(): writes exactly the given text, no extras."""
    sys.stdout.write(s)


def php_die(msg):
    sys.stdout.write(msg)
    sys.exit(0)


def open_or_die(path, mode="r"):
    try:
        return open(path, mode, encoding="utf-8", errors="replace", newline="" if "w" in mode else None)
    except OSError:
        php_die("Unable to open '" + path + "'.\n")


def issetarg(argv, arg):
    """
    Mimics PHP's issetarg(): a *prefix* match (strncmp) against each argv
    entry (argv[1:]), returning the (1-based) matching index or 0.
    """
    for i in range(1, len(argv)):
        if argv[i][:len(arg)] == arg:
            return i
    return 0


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    argv = sys.argv

    # Credits before anything else.
    printcredits()

    if len(argv) < 2:
        gethelp()

    renewal = bool(issetarg(argv, "-re") or issetarg(argv, "--renewal"))
    prere = bool(issetarg(argv, "-pre-re") or issetarg(argv, "--pre-renewal"))
    constants = not (issetarg(argv, "-itid") or issetarg(argv, "--use-itemid"))
    help_ = bool(issetarg(argv, "-h") or issetarg(argv, "--help"))

    def get_directory():
        arg = issetarg(argv, "--directory") or issetarg(argv, "-dir") or 0
        if arg:
            part = argv[arg].split("=")
            if len(part) < 2:
                php_die("A directory path was not provided!\n")
            elif not os.path.isdir(part[1]):
                php_die("The given directory " + part[1] + " doesn't exist.\n")
            else:
                return part[1]
        return None

    dirpath = get_directory()

    if dirpath:
        p("Read/Write Directory has been set to '" + dirpath + "'\n")
        p("Please ensure all skill_db TXT files are placed in this path.\n")
        p("Please also provide the correct version of the database (re/pre-re).\n")

    debug = bool(issetarg(argv, "-dbg") or issetarg(argv, "--with-debug"))

    t_init = None
    if debug:
        p("\033[0mDebug Mode Enabled.\n")
        tracemalloc.start()
        t_init = microtime_float()

    if help_ or (not renewal and not prere) or (renewal and prere):
        gethelp()

    renewal_defined = False
    if renewal:
        p("Renewal enabled.\n")
        p("skill_db.txt and associated files (cast, nocastdex, require & unit) will be converted.\n")
        if dirpath is None:
            dirpath = '../db/re/'
        renewal_defined = True
    elif prere:
        p("Pre-Renewal enabled.\n")
        p("skill_db.txt and associated files (cast, nocastdex, require & unit) will be converted.\n")
        if dirpath is None:
            dirpath = '../db/pre-re/'

    DIRPATH = dirpath
    RENEWAL = renewal_defined

    # check for existence of files.
    file_check = [
        DIRPATH + "skill_require_db.txt",
        DIRPATH + "skill_cast_db.txt",
        DIRPATH + "skill_castnodex_db.txt",
        DIRPATH + "skill_unit_db.txt",
        DIRPATH + "skill_db.txt",
    ]
    if constants:
        file_check.append(DIRPATH + "item_db.conf")

    for file_ in file_check:
        if os.path.exists(file_):
            p(file_ + " - Found\n")
        else:
            php_die(file_ + " - Not Found!\n")

    if constants:
        p("Using of Item Constants : enabled\n")
    else:
        p("Using of Item Constants : disabled.\n")

    # ----------------------------------------------------------------- #
    # Begin the Loading of Files
    # ----------------------------------------------------------------- #

    # skill_require_db.txt
    skreq = {k: {} for k in ['ID', 'HPCost', 'MaxHPTrigger', 'SPCost', 'HPRateCost',
                              'SPRateCost', 'ZenyCost', 'Weapons', 'AmmoTypes',
                              'AmmoAmount', 'State', 'SpiritSphere']}
    skreqit_itemid = {}
    skreqit_amount = {}

    i = 0
    file_ = "skill_require_db.txt"
    f = open_or_die(DIRPATH + file_, "r")
    p("Reading '" + DIRPATH + file_ + "' ...\n")
    for raw_line in f:
        if raw_line[:2] == "//" or len(raw_line) < 10:
            continue
        stripped = re.sub(r'\s+', '', raw_line)
        idx = stripped.find("//")
        line = stripped[:idx] if idx != -1 else ""  # strstr(..., "//", true) == FALSE -> "" via explode() coercion
        arr = line.split(",")
        if len(arr) == 0:
            continue
        skreq['ID'][i] = fget(arr, 0)
        skreq['HPCost'][i] = fget(arr, 1)
        skreq['MaxHPTrigger'][i] = fget(arr, 2)
        skreq['SPCost'][i] = fget(arr, 3)
        skreq['HPRateCost'][i] = fget(arr, 4)
        skreq['SPRateCost'][i] = fget(arr, 5)
        skreq['ZenyCost'][i] = fget(arr, 6)
        skreq['Weapons'][i] = fget(arr, 7)
        skreq['AmmoTypes'][i] = fget(arr, 8)
        skreq['AmmoAmount'][i] = fget(arr, 9)
        skreq['State'][i] = fget(arr, 10)
        skreq['SpiritSphere'][i] = fget(arr, 11)
        itids = []
        amts = []
        j = 12
        while j <= 31:
            itids.append(fget(arr, j, 0))
            amts.append(fget(arr, j + 1, 0))
            j += 2
        skreqit_itemid[i] = itids
        skreqit_amount[i] = amts
        i += 1
    if debug:
        p("[Debug] Read require_db Memory: " + print_mem() + "\n")
    f.close()

    # skill_cast_db.txt (NOTE: unlike the others, this file is NOT whitespace-stripped)
    skcast = {k: {} for k in ['ID', 'casttime', 'actdelay', 'walkdelay', 'data1', 'data2', 'cooldown', 'fixedcast']}
    file_ = "skill_cast_db.txt"
    f = open_or_die(DIRPATH + file_, "r")
    p("Reading '" + DIRPATH + file_ + "' ...\n")
    i = 0
    for raw_line in f:
        if raw_line[:2] == "//" or len(raw_line) < 10:
            continue
        arr = raw_line.split(",")
        if len(arr) == 0:
            continue
        skcast['ID'][i] = fget(arr, 0)
        skcast['casttime'][i] = fget(arr, 1)
        skcast['actdelay'][i] = fget(arr, 2)
        skcast['walkdelay'][i] = fget(arr, 3)
        skcast['data1'][i] = fget(arr, 4)
        skcast['data2'][i] = fget(arr, 5)
        skcast['cooldown'][i] = fget(arr, 6)
        if RENEWAL:
            skcast['fixedcast'][i] = fget(arr, 7)
        i += 1
    if debug:
        p("[Debug] Read cast_db Memory: " + print_mem() + "\n")
    f.close()

    # skill_castnodex_db.txt
    sknodex_id = {}
    sknodex_cast = {}
    sknodex_delay = {}
    file_ = "skill_castnodex_db.txt"
    f = open_or_die(DIRPATH + file_, "r")
    p("Reading '" + DIRPATH + file_ + "' ...\n")
    i = 0
    for raw_line in f:
        if raw_line[:2] == "//" or len(raw_line) <= 2:
            continue
        stripped = re.sub(r'\s+', '', raw_line)
        idx = stripped.find("//")
        line = stripped[:idx] if idx != -1 else ""
        arr = line.split(",")
        sknodex_id[i] = fget(arr, 0)
        sknodex_cast[i] = fget(arr, 1, 0)
        sknodex_delay[i] = fget(arr, 2, 0)
        i += 1
    if debug:
        p("[Debug] Read cast_nodex Memory: " + print_mem() + "\n")
    f.close()

    # item_db.conf (only if constants requested)
    itemdb_id = {}
    itemdb_name = {}
    if constants:
        file_ = "item_db.conf"
        path = DIRPATH + file_
        if os.path.exists(path):
            f = open_or_die(path, "r")
            p("Reading '" + path + "' ...\n")
            i = 0
            # NOTE ON A PHP QUIRK ("$started" state machine):
            # The original PHP trims each line *before* comparing it to the
            # literal strings "{\n" / "},\n". Since trim() always strips a
            # trailing "\n", the trimmed line can never equal either literal,
            # so `strcmp($line,"{\n")` is always non-zero (truthy) and
            # `$started` is set to true on essentially every line starting
            # with the very first one -- the `else if` branch that would set
            # it back to false is therefore unreachable (dead code). The net
            # effect is that `$started` is always true and the gating has no
            # real effect: every line of the file is processed. We reproduce
            # that *actual* behavior here (not the apparently-intended one)
            # by simply processing every line unconditionally.
            for raw_line in f:
                line = raw_line.strip()
                parts = line.split(":")
                if len(parts) >= 1:
                    if parts[0] == "Id":
                        itemdb_id[i] = php_intval(fget(parts, 1))
                    if parts[0] == "AegisName":
                        itemdb_name[i] = fget(parts, 1, "").replace('"', '')
                        i += 1
            if debug:
                p("[Debug] Read item_db Memory: " + print_mem() + "\n")
            f.close()
        else:
            p("Unable to open '" + path + "'... defaulting to using Item ID's instead of Constants.\n")
            constants = False

    # skill_unit_db.txt
    skunit_id = {}
    skunit_unitid = {}
    skunit_unitid2 = {}
    skunit_layout = {}
    skunit_range = {}
    skunit_interval = {}
    skunit_target = {}
    skunit_flag = {}
    i = 0
    file_ = "skill_unit_db.txt"
    f = open_or_die(DIRPATH + file_, "r")
    p("Reading '" + DIRPATH + file_ + "' ...\n")
    for raw_line in f:
        if raw_line[:2] == "//" or len(raw_line) < 10:
            continue
        stripped = re.sub(r'\s+', '', raw_line)
        idx = stripped.find("//")
        line = stripped[:idx] if idx != -1 else ""
        arr = line.split(",")
        skunit_id[i] = fget(arr, 0)
        skunit_unitid[i] = fget(arr, 1)
        skunit_unitid2[i] = fget(arr, 2)
        skunit_layout[i] = fget(arr, 3)
        skunit_range[i] = fget(arr, 4)
        skunit_interval[i] = fget(arr, 5)
        skunit_target[i] = fget(arr, 6)
        skunit_flag[i] = php_hexdec(fget(arr, 7))
        i += 1
    if debug:
        p("[Debug] Read unit_db Memory: " + print_mem() + "\n")
    f.close()

    out = []  # accumulates the output ($putsk)
    out.append(getcomments(RENEWAL))
    out.append("skill_db: (\n")

    # Main skill_db.txt
    file_ = "skill_db.txt"
    f = open_or_die(DIRPATH + file_, "r")
    p("Reading '" + DIRPATH + file_ + "' ...\n")
    linecount = 0
    for raw_line in f:
        if raw_line[:2] == "//" or len(raw_line) < 10:
            continue
        linecount += 1
    if debug:
        p("[Debug] Read skill_db Memory: " + print_mem() + "\n")
    f.close()
    p(str(linecount) + " entries found in skill_db.txt.\n")

    i = 0
    f = open_or_die(DIRPATH + file_, "r")
    max_level = 10
    max_items = 10
    for raw_line in f:
        if raw_line[:2] == "//" or len(raw_line) < 10:
            continue
        arr = raw_line.split(",")
        # id,range,hit,inf,element,nk,splash,max,list_num,castcancel,cast_defence_rate,inf2,maxcount,skill_type,blow_count,name,description
        id_ = fget(arr, 0)
        range_ = fget(arr, 1)
        hit = fget(arr, 2)
        inf = fget(arr, 3)
        element = fget(arr, 4)
        nk = fget(arr, 5)
        splash = fget(arr, 6)
        max_field = fget(arr, 7)
        max_out = 1 if php_lt_int(max_field, 1) else max_field
        list_num = fget(arr, 8)
        castcancel = fget(arr, 9)
        cast_defence_rate = fget(arr, 10)
        inf2 = fget(arr, 11)
        maxcount = fget(arr, 12)
        skill_type = fget(arr, 13)
        blow_count = fget(arr, 14)
        name = fget(arr, 15)
        desc_field = fget(arr, 16, "")
        description = extract_description(desc_field)

        out.append("{\n")
        out.append("\tId: " + php_s(id_) + "\n")
        out.append("\tName: \"" + php_s(name).strip() + "\"\n")
        out.append("\tDescription: \"" + php_s(description).strip() + "\"\n")
        out.append("\tMaxLevel: " + php_s(max_out) + "\n")
        if php_truthy(range_):
            out.append("\tRange: " + str(leveled_guessfill(range_, max_level, id_)) + "\n")
        if php_eq(hit, 8):
            out.append("\tHit: \"BDT_MULTIHIT\"\n")
        elif php_eq(hit, 6):
            out.append("\tHit: \"BDT_SKILL\"\n")
        if php_truthy(inf):
            out.append("\tSkillType: " + getinf(inf) + "\n")
        if php_truthy(inf2):
            out.append("\tSkillInfo: " + getinf2(inf2) + "\n")
        if php_neq(skill_type, "none"):
            out.append("\tAttackType: \"" + ucfirst(skill_type) + "\"\n")
        if php_truthy(element):
            out.append("\tElement: " + leveled_ele(element, max_level, id_) + "\n")
        if php_truthy(nk) and php_neq(nk, "0x0"):
            out.append("\tDamageType: " + getnk(nk) + "\n")
        if php_truthy(splash):
            out.append("\tSplashRange: " + str(leveled_guessfill(splash, max_level, id_)) + "\n")
        if php_neq(list_num, "1"):
            out.append("\tNumberOfHits: " + str(leveled_guessfill(list_num, max_level, id_)) + "\n")
        if php_eq(castcancel, "yes"):
            out.append("\tInterruptCast: true\n")
        if php_truthy(cast_defence_rate):
            out.append("\tCastDefRate: " + php_s(cast_defence_rate) + "\n")
        if php_truthy(maxcount):
            out.append("\tSkillInstances: " + str(leveled_guessfill(maxcount, max_level, id_)) + "\n")
        if php_truthy(blow_count):
            out.append("\tKnockBackTiles: " + str(leveled_guessfill(blow_count, max_level, id_)) + "\n")

        # Cast Db
        key = array_search_id(id_, skcast['ID'])
        if key is not None:
            if php_truthy(skcast['casttime'].get(key)):
                out.append("\tCastTime: " + str(leveled_guessfill(skcast['casttime'][key], max_level, id_)) + "\n")
            if php_truthy(skcast['actdelay'].get(key)):
                out.append("\tAfterCastActDelay: " + str(leveled_guessfill(skcast['actdelay'][key], max_level, id_)) + "\n")
            if php_s(skcast['walkdelay'].get(key)) != '0':
                out.append("\tAfterCastWalkDelay: " + str(leveled_guessfill(skcast['walkdelay'][key], max_level, id_)) + "\n")
            if php_s(skcast['data1'].get(key)) != '0':
                out.append("\tSkillData1: " + str(leveled_guessfill(skcast['data1'][key], max_level, id_)) + "\n")
            if php_s(skcast['data2'].get(key)) != '0':
                out.append("\tSkillData2: " + str(leveled_guessfill(skcast['data2'][key], max_level, id_)) + "\n")
            if php_s(skcast['cooldown'].get(key)) != '0':
                out.append("\tCoolDown: " + str(leveled_guessfill(skcast['cooldown'][key], max_level, id_)) + "\n")
            if RENEWAL:
                fc = skcast['fixedcast'].get(key)
                if fc is not None and len(php_s(fc)) > 1 and php_s(fc) != '0':
                    out.append("\tFixedCastTime: " + str(leveled_guessfill(fc, max_level, id_)) + "\n")

        # Cast NoDex
        key = array_search_id(id_, sknodex_id)
        if key is not None:
            cast_val = sknodex_cast.get(key)
            if cast_val is not None and php_neq(cast_val, 0):
                out.append("\tCastTimeOptions: " + getnocast(cast_val, id_) + "\n")
            delay_val = sknodex_delay.get(key)
            if delay_val is not None and php_neq(delay_val, 0):
                out.append("\tSkillDelayOptions: " + getnocast(delay_val, id_) + "\n")
            sknodex_id.pop(key, None)
            sknodex_cast.pop(key, None)
            sknodex_delay.pop(key, None)

        # require DB
        key = array_search_id(id_, skreq['ID'])
        if key is not None:
            out.append("\tRequirements: {\n")
            if php_truthy(skreq['HPCost'][key]):
                out.append("\t\tHPCost: " + str(leveled_guessfill(skreq['HPCost'][key], max_level, id_, 1)) + "\n")
            if php_truthy(skreq['SPCost'][key]):
                out.append("\t\tSPCost: " + str(leveled_guessfill(skreq['SPCost'][key], max_level, id_, 1)) + "\n")
            if php_truthy(skreq['HPRateCost'][key]):
                out.append("\t\tHPRateCost: " + str(leveled_guessfill(skreq['HPRateCost'][key], max_level, id_, 1)) + "\n")
            if php_truthy(skreq['SPRateCost'][key]):
                out.append("\t\tSPRateCost: " + str(leveled_guessfill(skreq['SPRateCost'][key], max_level, id_, 1)) + "\n")
            if php_truthy(skreq['ZenyCost'][key]):
                out.append("\t\tZenyCost: " + str(leveled_guessfill(skreq['ZenyCost'][key], max_level, id_, 1)) + "\n")
            if php_neq(skreq['Weapons'][key], 99):
                out.append("\t\tWeaponTypes: " + getweapontypes(skreq['Weapons'][key], id_) + "\n")
            if php_eq(skreq['AmmoTypes'][key], 99):
                out.append("\t\tAmmoTypes: \"All\"\n")
            elif php_truthy(skreq['AmmoTypes'][key]):
                out.append("\t\tAmmoTypes: " + getammotypes(skreq['AmmoTypes'][key], id_) + "\n")
            if php_truthy(skreq['AmmoAmount'][key]):
                out.append("\t\tAmmoAmount: " + str(leveled_guessfill(skreq['AmmoAmount'][key], max_level, id_, 1)) + "\n")
            if php_neq(skreq['State'][key], "none") and php_truthy(skreq['State'][key]):
                out.append("\t\tState: \"" + php_s(getstate(skreq['State'][key], id_)) + "\"\n")
            if php_truthy(skreq['SpiritSphere'][key]):
                out.append("\t\tSpiritSphereCost: " + str(leveled_guessfill(skreq['SpiritSphere'][key], max_level, id_, 1)) + "\n")

            itids = skreqit_itemid[key]
            amts = skreqit_amount[key]
            if php_gt_int(itids[0], 0):
                out.append("\t\tItems: {\n")
                for index in range(len(itids)):
                    itemID = itids[index]
                    itemamt = amts[index]

                    if php_colon_truthy(itemID):
                        items = php_s(itemID).split(":")
                        it = 0
                        while it < len(items):
                            if constants and php_truthy(itemID):
                                itkey = array_search_id(items[it], itemdb_id)
                                if itkey is None:
                                    itemname = "ID" + items[it]
                                else:
                                    itemname = itemdb_name.get(itkey, "")
                                out.append("\t\t\t" + php_s(itemname).strip() + ": " + str(leveled(itemamt, max_level, id_, 2)) + "\n")
                            elif php_intval(itemID):
                                out.append("\t\t\tID" + items[it] + ": " + str(leveled(itemamt, max_level, id_, 2)) + "\n")
                            it += 1
                    else:
                        if constants and php_truthy(itemID):
                            itkey = array_search_id(itemID, itemdb_id)
                            if itkey is None:
                                itemname = "ID" + php_s(itemID)
                            else:
                                itemname = itemdb_name.get(itkey, "")
                            out.append("\t\t\t" + php_s(itemname).strip() + ": " + str(leveled(itemamt, max_items, id_, 2)) + "\n")
                        elif php_truthy(itemID):
                            out.append("\t\t\tID" + php_s(itemID) + ": " + str(leveled(itemamt, max_items, id_, 2)) + "\n")
                out.append("\t\t}\n")
            out.append("\t}\n")

        # Unit
        key = array_search_id(id_, skunit_id)
        if key is not None:
            out.append("\tUnit: {\n")
            if key in skunit_unitid:
                uid2 = skunit_unitid2.get(key)
                if uid2 is not None and len(php_s(uid2)) > 0:
                    out.append("\t\tId: [ " + php_s(skunit_unitid[key]) + ", " + php_s(uid2) + " ]\n")
                else:
                    out.append("\t\tId: " + php_s(skunit_unitid[key]) + "\n")
            if key in skunit_layout and php_neq(skunit_layout[key], 0):
                out.append("\t\tLayout: " + str(leveled_guessfill(skunit_layout[key], max_level, id_, 1)) + "\n")
            if key in skunit_range and php_neq(skunit_range[key], 0):
                out.append("\t\tRange: " + str(leveled_guessfill(skunit_range[key], max_level, id_, 1)) + "\n")
            if key in skunit_interval:
                out.append("\t\tInterval: " + str(php_intval(skunit_interval[key])) + "\n")
            if key in skunit_target and php_neq(skunit_target[key], "noone"):
                out.append("\t\tTarget: \"" + ucfirst(skunit_target[key]).strip() + "\"\n")
            if key in skunit_flag and php_neq(skunit_flag[key], ""):
                out.append("\t\tFlag: " + php_s(getunitflag(skunit_flag[key], id_)) + "\n")
            out.append("\t}\n")

        # close skill
        out.append("},\n")
        # Display progress bar
        show_status(i, linecount)
        i += 1
    show_status(linecount, linecount)
    f.close()

    p("\n")
    p("The skill database has been successfully converted to Hercules' libconfig\n")
    p("format and has been saved as '" + DIRPATH + "skill_db.conf'.\n")
    p("The following files are now deprecated and can be deleted -\n")
    p(DIRPATH + "skill_db.txt\n")
    p(DIRPATH + "skill_cast_db.txt\n")
    p(DIRPATH + "skill_castnodex_db.txt\n")
    p(DIRPATH + "skill_require_db.txt\n")
    p(DIRPATH + "skill_unit_db.txt\n")

    out.append(")")
    putsk = "".join(out)
    skconf = "skill_db.conf"
    with open(DIRPATH + skconf, "w", encoding="utf-8", newline="") as outf:
        outf.write(putsk)

    if debug:
        p("[Debug] Memory after converting: " + print_mem() + "\n")
        p("[Debug] Execution Time : " + str(microtime_float() - t_init) + "s\n")


if __name__ == "__main__":
    main()
