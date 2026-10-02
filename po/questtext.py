#!/usr/bin/env python3
# NetHack 5.0  questtext.py
# NetHack may be freely redistributed.  See license for details.
#
# List the texts of dat/quest.lua (text, synopsis and the strings of the
# text arrays) as NC_("quest", "...") for xgettext: com_pager() (questpgr.c)
# translates them whole, before replacing their %-codes.
# Usage: python3 po/questtext.py dat/quest.lua > quest.c

import re
import sys


def lua_strings(src):
    """yield (key, string) for each Lua string constant of src; key is
    the name of the field it is assigned to (or None) and the table path
    is tracked to skip msg_fallbacks"""
    i, n = 0, len(src)
    key, stack = None, []
    while i < n:
        c = src[i]
        if src.startswith('--', i):
            m = re.compile(r'--\[(=*)\[').match(src, i)
            if m:
                close = ']' + m.group(1) + ']'
                i = src.index(close, m.end()) + len(close)
            else:
                j = src.find('\n', i)
                i = n if j < 0 else j
            continue
        m = re.compile(r'\[(=*)\[').match(src, i)
        if m:
            close = ']' + m.group(1) + ']'
            j = src.index(close, m.end())
            s = src[m.end():j]
            if s.startswith('\n'):
                s = s[1:]
            yield key, stack, s
            key = None
            i = j + len(close)
            continue
        if c in '"\'':
            j, out = i + 1, []
            while src[j] != c:
                if src[j] == '\\':
                    e = src[j + 1]
                    out.append({'n': '\n', 't': '\t', '\\': '\\', '"': '"',
                                "'": "'", '\n': '\n'}.get(e, e))
                    j += 2
                else:
                    out.append(src[j])
                    j += 1
            yield key, stack, ''.join(out)
            key = None
            i = j + 1
            continue
        m = re.compile(r'([A-Za-z_][A-Za-z_0-9]*)\s*=(?!=)').match(src, i)
        if m:
            key = m.group(1)
            i = m.end()
            continue
        if c == '{':
            stack.append(key)
            key = None
        elif c == '}':
            stack.pop()
        elif c == ',':
            key = None
        i += 1


def c_string(s):
    s = s.replace('\\', '\\\\').replace('"', '\\"').replace('\t', '\\t')
    return '"' + s.replace('\n', '\\n"\n    "') + '"'


def main():
    src = open(sys.argv[1], encoding='utf-8').read()
    seen = set()
    for key, stack, s in lua_strings(src):
        if (key == 'output' or 'msg_fallbacks' in stack
                or 'TEST_PATTERN' in stack or s in seen):
            continue
        seen.add(s)
        print('NC_("quest", %s);' % c_string(s))


main()
