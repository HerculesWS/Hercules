#!/usr/bin/env python3

# This file is part of Hercules.
# http://herc.ws - http://github.com/HerculesWS/Hercules
#
# Copyright (C) 2013-2025 Hercules Dev Team
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

import datetime
import glob
import os
import re
import xml.etree.ElementTree as ET


# ---------------------------------------------------------------------------
# Small helpers replicating Perl semantics used throughout the original
# script (falsy strings, trimming, etc).
# ---------------------------------------------------------------------------

def perl_str_truthy(s):
    """Replicates Perl's boolean-string truthiness: undef, '' and '0' are false."""
    return s is not None and s != '' and s != '0'


def perl_or(a, b):
    """Replicates Perl's `a || b` for possibly-undef/empty/zero string values."""
    return a if perl_str_truthy(a) else b


def trim(s):
    """sub trim($) { $s =~ s/^\\s+//; $s =~ s/\\s+$//; return $s; }"""
    if s is None:
        s = ''
    s = re.sub(r'^\s+', '', s)
    s = re.sub(r'\s+$', '', s)
    return s


def flat_text(elem):
    """Flatten an element's text content (including any nested child tags,
    e.g. doxygen <ref> links) into a single string, mimicking what a human
    reading the raw declaration text would see."""
    if elem is None:
        return ''
    return ''.join(elem.itertext())


def plain_text_or_none(elem):
    """Returns the element's text if it is a simple text-only element (no
    child elements). Returns None if the element is missing, has child
    elements, or has no text - this mirrors the Perl code's `ref $t eq
    'HASH'` check, which is true for XML::Simple when an element is empty
    or contains nested markup instead of a plain scalar string."""
    if elem is None:
        return None
    if len(elem) > 0:
        return None
    return elem.text


# ---------------------------------------------------------------------------
# parse($$) -- the C-declaration argument-list parser.
# ---------------------------------------------------------------------------

_FUNCPTR_RE = re.compile(r"""
    ^
    ([\w\s\[\]*]+\()          # Capture words, spaces, array subscripts, up to the first '(' (return type)
    \s*                       # Skip spaces
    (\*)                      # Capture the '*' from the function name
    \s*                       # Skip spaces
    ([\w]*)                   # Capture words (function name)
    \s*                       # Skip spaces
    (\)\s*\([\w\s\[\]*,]*\))  # Capture first ')' followed by a '( )' block containing arguments
    \s*                       # Skip spaces
    $
""", re.VERBOSE)

_VARTYPE_RE = re.compile(r'^\s*(\w+)((?:const|[*\s])*)(\w*)\s*((?:\[\])?)$')


def parse(p, d):
    # Clean up extra parentheses )(around the arglist)
    p = re.sub(r'^.*?\)\((.*)\).*$', r'\1', p, count=1)

    # Retrieve return type
    m = re.search(r'^(.+)\(\*\s*[a-zA-Z0-9_]+_interface(?:_private)?::([^)]+)\s*\)\s*\(.*\)$', d)
    if not m:
        print(f"Error: unable to parse '{d}'")
        return {}
    rt = trim(m.group(1))    # return type
    name = trim(m.group(2))  # function name

    args = []
    anonvars = 0
    variadic = 0
    lastvar = ''
    notes = ''

    if not p:
        p = ' '  # ensure there's at least one character (we don't want a do{} block)

    while perl_str_truthy(p):  # Scan the string for variables
        current = ''
        paren = 0
        needspace = 0

        while perl_str_truthy(p):  # Parse tokens
            p = re.sub(r'^(?:\s*(?:/\*.*?\*/)?)*', '', p, count=1)  # strip heading whitespace and c-style comments
            if not perl_str_truthy(p):
                break

            m = re.match(r'^([a-zA-Z0-9_]+)', p)  # Word (variable, type)
            if m:
                if needspace:
                    current += ' '
                current += m.group(1)
                p = p[m.end():]
                needspace = 1
                continue

            m = re.match(r'^(,)', p)  # Comma
            if m:
                p = p[m.end():]
                if not paren:  # Argument terminator unless inside parentheses
                    break
                current += m.group(1)
                needspace = 1
                continue

            m = re.match(r'^(\*)', p)  # Pointer
            if m:
                if needspace:
                    current += ' '
                current += m.group(1)
                p = p[m.end():]
                needspace = 0
                continue

            m = re.match(r'^([\[\].])', p)  # Array subscript
            if m:
                current += m.group(1)
                p = p[m.end():]
                needspace = 0
                continue

            m = re.match(r'^(\()', p)  # Open parenthesis
            if m:
                if needspace:
                    current += ' '
                current += m.group(1)
                p = p[m.end():]
                needspace = 0
                paren += 1
                continue

            m = re.match(r'^(\))', p)  # Closed parenthesis
            if m:
                current += m.group(1)
                p = p[m.end():]
                needspace = 1
                if not paren:
                    notes += "\n/* Error: unexpected ')' */"
                    print(f"Error: unexpected ')' at '{p}'")
                else:
                    paren -= 1
                continue

            # Any other symbol
            ch = p[0]
            p = p[1:]
            notes += f"\n/* Error: Unexpected character '{ch}' */"
            print(f"Error: Unexpected character '{ch}' at '{p}'")
            current += ch
            needspace = 0

        current = re.sub(r'^\s+', '', current)
        current = re.sub(r'\s+$', '', current)  # trim

        if not current or re.search(r'^void$', current):  # Skip if empty
            continue

        array = ''
        type1 = ''
        var = ''
        type2 = ''
        indir = ''

        fm = _FUNCPTR_RE.match(current)  # Match a function pointer
        if fm:
            type1 = trim(fm.group(1))
            indir = fm.group(2)
            var = trim(fm.group(3) if fm.group(3) is not None else '')
            type2 = trim(fm.group(4))
        elif current == '...':  # Match a '...' variadic argument
            type1 = '...'
            indir = ''
            var = ''
            type2 = ''
        else:  # Match a "regular" variable
            type1 = ''
            while True:
                cm = re.search(r'^(const)\s+(.*)$', current)  # const modifier
                if cm:
                    type1 += f"{cm.group(1)} "
                    current = cm.group(2) if cm.group(2) is not None else ''
                    continue
                cm = re.search(r'^((?:un)?signed)\s+((?:char|int|long|short)[*\s]+.*)$', current)
                if not cm:
                    cm = re.search(r'^(long|short)\s+((?:int|long)[*\s]+.*)$', current)
                if cm:  # signed/unsigned/long/short modifiers
                    current = cm.group(2)
                    type1 += f"{cm.group(1)} "
                    continue
                cm = re.search(r'^(struct|enum|union)\s+(.*)$', current)  # union, enum and struct names
                if cm:
                    current = cm.group(2) if cm.group(2) is not None else ''
                    type1 += f"{cm.group(1)} "
                break  # No other modifiers

            vm = _VARTYPE_RE.match(current)  # Variable type and name
            if vm:
                type1 += trim(vm.group(1))
                indir = trim(vm.group(2) if vm.group(2) is not None else '')
                var = trim(vm.group(3) if vm.group(3) is not None else '')
                array = trim(vm.group(4) if vm.group(4) is not None else '')
                type2 = ''
            else:  # Unsupported
                notes += f"\n/* Error: Unhandled var type '{current}' */"
                print(f"Error: Unhandled var type '{current}'")
                args.append({
                    'var': current, 'callvar': '', 'type': '', 'orig': '',
                    'indir': 0, 'hookpref': '', 'hookpostf': '', 'hookprec': '',
                    'hookpostc': '', 'origc': '', 'pre': '', 'post': '',
                })
                continue

        if not var:
            anonvars += 1
            var = f"p{anonvars}"

        callvar = var
        pre_code = ''
        post_code = ''
        dereference = ''
        addressof = ''
        indirectionlvl = 1 if re.search(r'\*', indir) else 0

        if type1 == 'va_list':  # Special handling for argument-list variables
            callvar = f"{var}___copy"
            pre_code = f"va_list {callvar}; va_copy({callvar}, {var});"
            post_code = f"va_end({callvar});"
        elif type1 == '...':  # Special handling for variadic arguments
            if not lastvar:
                notes += "\n/* Error: Variadic function with no fixed arguments */"
                print("Error: Variadic function with no fixed arguments")
                continue
            pre_code = f"va_list {callvar}; va_start({callvar}, {lastvar});"
            post_code = f"va_end({callvar});"
            var = ''
            variadic = 1
        else:  # Increase indirection level when necessary
            dereference = '*'
            addressof = '&'

        if array:
            indirectionlvl += 1  # Arrays are pointer, no matter how cute you write them

        args.append({
            'var': var,
            'callvar': callvar,
            'type': type1 + array + type2,
            'orig': '...' if type1 == '...' else trim(f"{type1} {indir}{var}{array} {type2}"),
            'indir': indirectionlvl,
            'hookpref': f"va_list {var}" if type1 == '...' else trim(f"{type1} {dereference}{indir}{var}{array} {type2}"),
            'hookpostf': f"va_list {var}" if type1 == '...' else trim(f"{type1} {indir}{var}{array} {type2}"),
            'hookprec': trim(f"{addressof}{callvar}"),
            'hookpostc': trim(f"{callvar}"),
            'origc': trim(callvar),
            'pre': pre_code,
            'post': post_code,
        })
        lastvar = var

    rtinit = ''
    x = rt
    cm = re.search(r'^const\s+(.+)$', x)  # Strip const modifier
    if cm:
        x = cm.group(1)

    if re.search(r'\*', x):  # Pointer
        rtinit = ' = NULL'
    elif x == 'void':  # void
        rtinit = ''
    elif x == 'bool':  # bool
        rtinit = ' = false'
    elif re.search(r'^(?:enum\s+)?damage_lv$', x):  # Known enum damage_lv
        rtinit = ' = ATK_NONE'
    elif re.search(r'^(?:enum\s+)?sc_type$', x):  # Known enum sc_type
        rtinit = ' = SC_NONE'
    elif re.search(r'^(?:enum\s+)?c_op$', x):  # Known enum c_op
        rtinit = ' = C_NOP'
    elif re.search(r'^enum\s+BATTLEGROUNDS_QUEUE_ACK$', x):  # Known enum BATTLEGROUNDS_QUEUE_ACK
        rtinit = ' = BGQA_SUCCESS'
    elif re.search(r'^enum\s+bl_type$', x):  # Known enum bl_type
        rtinit = ' = BL_NUL'
    elif re.search(r'^enum\s+homun_type$', x):  # Known enum homun_type
        rtinit = ' = HT_INVALID'
    elif re.search(r'^enum\s+channel_operation_status$', x):  # Known enum channel_operation_status
        rtinit = ' = HCS_STATUS_FAIL'
    elif re.search(r'^enum\s+bg_queue_types$', x):  # Known enum bg_queue_types
        rtinit = ' = BGQT_INVALID'
    elif re.search(r'^enum\s+parsefunc_rcode$', x):  # Known enum parsefunc_rcode
        rtinit = ' = PACKET_UNKNOWN'
    elif re.search(r'^enum\s+DBOptions$', x):  # Known enum DBOptions
        rtinit = ' = DB_OPT_BASE'
    elif re.search(r'^enum\s+thread_priority$', x):  # Known enum thread_priority
        rtinit = ' = THREADPRIO_NORMAL'
    elif re.search(r'^enum\s+market_buy_result$', x):  # Known enum market_buy_result
        rtinit = ' = MARKET_BUY_RESULT_ERROR'
    elif re.search(r'^enum\s+unit_dir$', x):  # Known enum unit_dir
        rtinit = ' = UNIT_DIR_UNDEFINED'
    elif re.search(r'^enum\s+quest_mobtype$', x):  # Known enum quest_mobtype
        rtinit = ' = QMT_RC_DEMIHUMAN'
    elif re.search(r'^e_scb_flag$', x):  # Known typedef e_scb_flag
        rtinit = ' = SCB_NONE'
    elif x in ('DBComparator', 'DBHasher', 'DBReleaser'):  # DB function pointers
        rtinit = ' = NULL'
    elif re.search(r'^(?:struct|union)\s+.*$', x):  # Structs and unions
        rtinit = ' = { 0 }'
    elif re.search(r'^float|double$', x):  # Floating point variables
        rtinit = ' = 0.'
    elif (re.search(r'^(?:(?:un)?signed\s+)?(?:char|int|long|short)$', x)
            or re.search(r'^(?:long|short)\s+(?:int|long)$', x)
            or re.search(r'^u?int(?:8|16|32|64)$', x)
            or x == 'defType'
            or x == 'size_t'
            or x == 'time_t'):  # Numeric variables
        rtinit = ' = 0'
    elif x == 'JsonWBool':  # bool
        rtinit = ' = cJSON_False'
    elif x == 'http_method':
        rtinit = ' = HTTP_GET'
    else:  # Anything else
        notes += f"\n/* Unknown return type '{rt}'. Initializing to '0'. */"
        print(f"Unknown return type '{rt}'. Initializing to '0'.")
        rtinit = ' = 0'

    return {
        'name': name,
        'vname': f"v{name}" if variadic else name,
        'type': rt,
        'typeinit': rtinit,
        'variadic': variadic,
        'args': args,
        'notes': notes,
    }


# ---------------------------------------------------------------------------
# Main program
# ---------------------------------------------------------------------------

def main():
    key2original = {}
    key2pointer = {}
    ifs = {}
    keys = {
        'login': [],
        'char': [],
        'map': [],
        'api': [],
        'all': [],
    }
    fileguards = {}

    files = sorted(f for f in glob.glob('doxyoutput/xml/*interface*.xml') if os.path.isfile(f))

    def member_sort_key(memberdef):
        loc = memberdef.find('location')
        bodystart = loc.get('bodystart') if loc is not None else None
        line = loc.get('line') if loc is not None else None
        v = perl_or(bodystart, line)
        try:
            return int(v)
        except (TypeError, ValueError):
            return 0

    for file in files:  # Loop through the xml files
        tree = ET.parse(file)
        root = tree.getroot()
        compounddef = root.find('compounddef')
        if compounddef is None:
            continue

        loc = compounddef.find('location')
        loc_file = loc.get('file') if loc is not None else ''

        fm = re.search(r'src/(api|map|char|login|common)/', loc_file)
        if not fm:
            continue
        if re.search(r'/HPM.*\.h', loc_file):  # Don't allow hooking into the HPM itself
            continue
        if re.search(r'/memmgr\.h', loc_file):  # Don't allow hooking into the memory manager
            continue
        servertype = fm.group(1)

        compoundname_el = compounddef.find('compoundname')
        key = compoundname_el.text if compoundname_el is not None and compoundname_el.text is not None else ''
        original = key
        servertypes = []
        servermask = 'SERVER_TYPE_NONE'
        if servertype != 'common':
            servertypes.append(servertype)
            servermask = 'SERVER_TYPE_' + servertype.upper()
        elif key == 'mapindex_interface':
            servertypes.extend(['map', 'char'])  # Currently not used by the login server
            servermask = 'SERVER_TYPE_MAP|SERVER_TYPE_CHAR'
        elif key == 'grfio_interface':
            servertypes.append('map')  # Currently not used by the login and char servers
            servermask = 'SERVER_TYPE_MAP'
        else:
            servertypes.extend(['api', 'map', 'char', 'login'])
            servermask = 'SERVER_TYPE_ALL'

        filepath = re.split(r'[/\\]', loc_file)
        foldername = filepath[-2].upper()
        filename = filepath[-1].upper()
        filename = re.sub(r'[.-]', '_', filename)
        filename = re.sub(r'\.[^.]*$', '', filename)
        filename = re.sub(r'_H_DOX$', '_H', filename)

        guardname = f"{foldername}_{filename}"
        private = 1 if re.search(r'_interface_private$', key) else 0

        # Some known interfaces with different names
        if re.search(r'battleground', key):
            key = 'bg'
        elif re.search(r'guild_storage', key):
            key = 'gstorage'
        elif key == 'homunculus_interface':
            key = 'homun'
        elif key == 'irc_bot_interface':
            key = 'ircbot'
        elif key == 'log_interface':
            key = 'logs'
        elif key == 'pc_groups_interface':
            key = 'pcg'
        elif key == 'pcre_interface':
            key = 'libpcre'
        elif key == 'char_interface':
            key = 'chr'
        elif key == 'db_interface':
            key = 'DB'
        elif key == 'socket_interface':
            key = 'sockt'
        elif key == 'sql_interface':
            key = 'SQL'
        elif key == 'stringbuf_interface':
            key = 'StrBuf'
        elif key == 'console_input_interface':
            # TODO
            continue
        else:
            key = re.sub(r'_interface', '', key)

        if private:
            key = re.sub(r'^(.*)_private$', r'PRIV__\1', key)
        pointername = key
        if private:
            pointername = re.sub(r'^PRIV__(.*)$', r'\1->p', pointername)

        for sectiondef in compounddef.findall('sectiondef'):  # Loop through the sections
            memberdefs = sectiondef.findall('memberdef')
            for f in sorted(memberdefs, key=member_sort_key):  # Loop through the members
                if f.get('kind') != 'variable':  # Skip macros
                    continue

                argsstring_el = f.find('argsstring')
                t = plain_text_or_none(argsstring_el)
                definition_el = f.find('definition')
                d = flat_text(definition_el)

                type_el = f.find('type')
                type_text = flat_text(type_el)
                if re.search(r'^\s*LoginParseFunc\s*\*\s*$', type_text):
                    t = ')(int fd, struct login_session_data *sd)'  # typedef LoginParseFunc
                    d = re.sub(
                        r'^LoginParseFunc\s*\*\s*(.*)$',
                        r'enum parsefunc_rcode(* \1) (int fd, struct login_session_data *sd)',
                        d,
                    )

                if t is None:  # Skip if it's not a string
                    continue
                if re.search(r'^\)?\[.*\]$', t):  # Skip arrays or pointers to array
                    continue

                if_ = parse(t, d)
                if not if_:  # If it returns an empty dict, an error must've occurred
                    continue

                # Skip variadic functions, we only allow hooks on their arglist equivalents.
                # i.e. you can't hook on map->foreachinmap, but you hook on map->vforeachinmap
                # (foreachinmap is guaranteed to do nothing other than call vforeachinmap)
                if if_['variadic']:
                    continue

                # Some preprocessing
                if_['hname'] = f"HP_{key}_{if_['name']}"
                if_['hvname'] = f"HP_{key}_{if_['vname']}"

                if_['handlerdef'] = f"{if_['type']} {if_['hname']}("
                if_['predef'] = f"{if_['type']} (*preHookFunc) ("
                if_['postdef'] = f"{if_['type']} (*postHookFunc) ("
                if if_['type'] == 'void':
                    if_['precall'] = ''
                    if_['postcall'] = ''
                    if_['origcall'] = ''
                else:
                    if_['precall'] = 'retVal___ = '
                    if_['postcall'] = 'retVal___ = '
                    if_['origcall'] = 'retVal___ = '
                if_['precall'] += 'preHookFunc('
                if_['postcall'] += 'postHookFunc('
                if_['origcall'] += f"HPMHooks.source.{key}.{if_['vname']}("
                if_['before'] = []
                if_['after'] = []

                i = 0
                j = 0

                if if_['type'] != 'void':
                    j += 1
                    if_['postdef'] += f"{if_['type']} retVal___"
                    if_['postcall'] += 'retVal___'

                for arg in if_['args']:
                    if arg.get('pre'):
                        if_['before'].append(arg['pre'])
                    if arg.get('post'):
                        if_['after'].append(arg['post'])
                    if i:
                        if_['handlerdef'] += ', '
                        if_['predef'] += ', '
                        if_['precall'] += ', '
                        if_['origcall'] += ', '
                    if j:
                        if_['postdef'] += ', '
                        if_['postcall'] += ', '
                    if_['handlerdef'] += arg.get('orig', '')
                    if_['predef'] += arg.get('hookpref', '')
                    if_['precall'] += arg.get('hookprec', '')
                    if_['postdef'] += arg.get('hookpostf', '')
                    if_['postcall'] += arg.get('hookpostc', '')
                    if_['origcall'] += arg.get('origc', '')
                    i += 1
                    j += 1

                if not i:
                    if_['predef'] += 'void'
                    if_['handlerdef'] += 'void'
                if not j:
                    if_['postdef'] += 'void'

                if_['handlerdef'] += ')'
                if_['predef'] += ');'
                if_['precall'] += ');'
                if_['postdef'] += ');'
                if_['postcall'] += ');'
                if_['origcall'] += ');'

                key2original[key] = original
                key2pointer[key] = pointername
                ifs.setdefault(key, []).append(if_)

        for st in servertypes:
            if key2original.get(key):
                keys[st].append(key)
        if key2original.get(key):
            keys['all'].append(key)
        fileguards[key] = {
            'guard': guardname,
            'type': servermask,
            'private': private,
        }

    year = datetime.date.today().year

    fileheader = ("/**\n"
                  " * This file is part of Hercules.\n"
                  " * http://herc.ws - http://github.com/HerculesWS/Hercules\n"
                  " *\n"
                  f" * Copyright (C) 2013-{year} Hercules Dev Team\n"
                  " *\n"
                  " * Hercules is free software: you can redistribute it and/or modify\n"
                  " * it under the terms of the GNU General Public License as published by\n"
                  " * the Free Software Foundation, either version 3 of the License, or\n"
                  " * (at your option) any later version.\n"
                  " *\n"
                  " * This program is distributed in the hope that it will be useful,\n"
                  " * but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
                  " * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
                  " * GNU General Public License for more details.\n"
                  " *\n"
                  " * You should have received a copy of the GNU General Public License\n"
                  " * along with this program.  If not, see <http://www.gnu.org/licenses/>.\n"
                  " */\n"
                  "\n"
                  "/*\n"
                  " * NOTE: This file was auto-generated and should never be manually edited,\n"
                  " *       as it will get overwritten.\n"
                  " */\n"
                  "\n"
                  "/* GENERATED FILE DO NOT EDIT */\n")

    def open_out(fname):
        return open(fname, 'w', encoding='utf-8', newline='\n')

    for servertype, keysref in keys.items():
        # Some interfaces use different names
        def export_symbol(k):
            if re.search(r'^bg$', k):
                return 'battlegrounds'
            if re.search(r'^pcg$', k):
                return 'pc_groups'
            return k

        exportsymbols = {k: export_symbol(k) for k in keysref}

        maxlen = 0

        if servertype == 'all':
            fname = "../../src/common/HPMSymbols.inc.h"
            fh = open_out(fname)

            fh.write(fileheader + "\n#if !defined(HERCULES_CORE)\n")

            for key in keysref:
                if fileguards[key]['private']:
                    continue
                guard = fileguards[key]['guard']
                fh.write(f"#ifdef {guard} /* {key} */\n"
                         f"struct {key2original[key]} *{key};\n"
                         f"#endif // {guard}\n")

            fh.write("#endif // ! HERCULES_CORE\n"
                      "\n"
                      "HPExport const char *HPM_shared_symbols(int server_type)\n"
                      "{\n")

            for key in keysref:
                if fileguards[key]['private']:
                    continue
                guard = fileguards[key]['guard']
                typ = fileguards[key]['type']
                exp = exportsymbols[key]
                fh.write(f"#ifdef {guard} /* {key} */\n"
                         f"\tif ((server_type&({typ})) != 0 && !HPM_SYMBOL(\"{exp}\", {key}))\n"
                         f"\t\treturn \"{exp}\";\n"
                         f"#endif // {guard}\n")

            fh.write("\treturn NULL;\n"
                      "}\n")
            fh.close()

            fname = "../../src/plugins/HPMHooking/HPMHooking.Defs.inc"
            fh = open_out(fname)

            fh.write(fileheader + "\n")

            for key in keysref:
                guard = fileguards[key]['guard']
                fh.write(f"#ifdef {guard} /* {key} */\n")

                for if_ in ifs.get(key, []):
                    predef = if_['predef'].replace('preHookFunc', f"HPMHOOK_pre_{key}_{if_['name']}", 1)
                    postdef = if_['postdef'].replace('postHookFunc', f"HPMHOOK_post_{key}_{if_['name']}", 1)

                    fh.write(f"typedef {predef}\ntypedef {postdef}\n")

                fh.write(f"#endif // {guard}\n")
            fh.close()

            continue

        fname = f"../../src/plugins/HPMHooking/HPMHooking_{servertype}.HookingPoints.inc"
        fh = open_out(fname)

        fh.write(fileheader + "\nstruct HookingPointData HookingPoints[] = {\n")

        for key in keysref:
            fh.write(f"/* {key2original[key]} */\n")
            pointername = key2pointer[key]
            for if_ in ifs.get(key, []):
                fh.write(f"\t{{ HP_POP({pointername}->{if_['name']}, {if_['hname']}) }},\n")

                length = len(key + "->" + if_['name'])
                if length > maxlen:
                    maxlen = length

        fh.write("};\n\nint HookingPointsLenMax = %d;\n" % maxlen)
        fh.close()

        fname = f"../../src/plugins/HPMHooking/HPMHooking_{servertype}.sources.inc"
        fh = open_out(fname)

        fh.write(fileheader + "\n")
        for key in keysref:
            fh.write(f"HPMHooks.source.{key} = *{key2pointer[key]};\n")
        fh.close()

        fname = f"../../src/plugins/HPMHooking/HPMHooking_{servertype}.HPMHooksCore.inc"
        fh = open_out(fname)

        fh.write(fileheader + "\nstruct {\n")

        for key in keysref:
            for if_ in ifs.get(key, []):
                fh.write(f"\tstruct HPMHookPoint *{if_['hname']}_pre;\n"
                         f"\tstruct HPMHookPoint *{if_['hname']}_post;\n")
        fh.write("} list;\n\nstruct {\n")

        for key in keysref:
            for if_ in ifs.get(key, []):
                fh.write(f"\tint {if_['hname']}_pre;\n"
                         f"\tint {if_['hname']}_post;\n")
        fh.write("} count;\n\nstruct {\n")

        for key in keysref:
            fh.write(f"\tstruct {key2original[key]} {key};\n")

        fh.write("} source;\n")
        fh.close()

        fname = f"../../src/plugins/HPMHooking/HPMHooking_{servertype}.Hooks.inc"
        fh = open_out(fname)

        fh.write(fileheader + "\n")
        for key in keysref:
            fh.write(f"/* {key2original[key]} */\n")

            for if_ in ifs.get(key, []):
                initialization = ''
                beforeblock3 = ''
                beforeblock2 = ''
                afterblock3 = ''
                afterblock2 = ''
                retval = ''

                if if_['type'] != 'void':
                    initialization = f"\n\t{if_['type']} retVal___{if_['typeinit']};"

                for bit in if_['before']:
                    beforeblock3 += f"\n\t\t\t{bit}"
                for bit in if_['after']:
                    afterblock3 += f"\n\t\t\t{bit}"
                for bit in if_['before']:
                    beforeblock2 += f"\n\t\t{bit}"
                for bit in if_['after']:
                    afterblock2 += f"\n\t\t{bit}"
                if if_['type'] != 'void':
                    retval = ' retVal___'

                hname = if_['hname']
                block = (
                    if_['handlerdef'] + " {" + if_['notes'] + "\n"
                    + "\tint hIndex = 0;" + initialization + "\n"
                    + "\tif (HPMHooks.count." + hname + "_pre > 0) {\n"
                    + "\t\t" + if_['predef'] + "\n"
                    + "\t\t*HPMforce_return = false;\n"
                    + "\t\tfor (hIndex = 0; hIndex < HPMHooks.count." + hname + "_pre; hIndex++) {" + beforeblock3 + "\n"
                    + "\t\t\tpreHookFunc = HPMHooks.list." + hname + "_pre[hIndex].func;\n"
                    + "\t\t\t" + if_['precall'] + afterblock3 + "\n"
                    + "\t\t}\n"
                    + "\t\tif (*HPMforce_return) {\n"
                    + "\t\t\t*HPMforce_return = false;\n"
                    + "\t\t\treturn" + retval + ";\n"
                    + "\t\t}\n"
                    + "\t}\n"
                    + "\t{" + beforeblock2 + "\n"
                    + "\t\t" + if_['origcall'] + afterblock2 + "\n"
                    + "\t}\n"
                    + "\tif (HPMHooks.count." + hname + "_post > 0) {\n"
                    + "\t\t" + if_['postdef'] + "\n"
                    + "\t\tfor (hIndex = 0; hIndex < HPMHooks.count." + hname + "_post; hIndex++) {" + beforeblock3 + "\n"
                    + "\t\t\tpostHookFunc = HPMHooks.list." + hname + "_post[hIndex].func;\n"
                    + "\t\t\t" + if_['postcall'] + afterblock3 + "\n"
                    + "\t\t}\n"
                    + "\t}\n"
                    + "\treturn" + retval + ";\n"
                    + "}\n"
                )
                fh.write(block)

        fh.close()


if __name__ == '__main__':
    main()
