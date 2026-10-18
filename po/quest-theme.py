#!/usr/bin/env python3
# NetHack 5.0  quest-theme.py
# NetHack may be freely redistributed.  See license for details.
#
# Help rewriting the quest texts (dat/quest.lua, msgctxt "quest") for a
# theme (CYBERPUNK.md).
#   python3 po/quest-theme.py dump ROLE [po/fr.po] > texts.txt
# lists the texts of ROLE ("common", "Wiz"...) with their translation, as
# "@@n" lines followed by the text; rewrite the texts, keeping the "@@n"
# lines and the %-codes, then
#   python3 po/quest-theme.py gen ROLE texts.txt >> po/themes/fr@cyber.po
# writes the theme entries and reports the %-codes that differ from those
# of po/fr.po.  A text shared by several roles gets a single translation.
# Run from the top of the source tree.

import re
import sys

# lua_strings() of questtext.py, without running its main()
exec(open('po/questtext.py', encoding='utf-8').read().rsplit('\nmain()', 1)[0])

ESCAPES = {'n': '\n', 't': '\t', '"': '"', '\\': '\\'}


def po_unquote(s):
    return re.sub(r'\\(.)', lambda m: ESCAPES.get(m.group(1), m.group(0)), s)


def po_quote(s):
    s = s.replace('\\', '\\\\').replace('"', '\\"').replace('\t', '\\t')
    lines = s.split('\n')
    if len(lines) == 1:
        return '"%s"' % s
    return ('""\n' + ''.join('"%s\\n"\n' % ln for ln in lines[:-1])
            + '"%s"' % lines[-1])


def quest_entries(path):
    """msgid -> msgstr of the msgctxt "quest" entries of a .po file"""
    res = {}
    for blk in open(path, encoding='utf-8').read().split('\n\n'):
        if not re.search(r'^msgctxt "quest"$', blk, re.M):
            continue
        cur, parts = None, {'msgid': [], 'msgstr': []}
        for ln in blk.split('\n'):
            m = re.match(r'^(msgctxt|msgid|msgstr) "(.*)"$', ln)
            if m:
                cur = m.group(1)
                if cur in parts:
                    parts[cur].append(m.group(2))
                continue
            m = re.match(r'^"(.*)"$', ln)
            if m and cur in parts:
                parts[cur].append(m.group(1))
        res[po_unquote(''.join(parts['msgid']))] = \
            po_unquote(''.join(parts['msgstr']))
    return res


def role_texts(role):
    """yield (key, table, text) for the texts of role"""
    src = open('dat/quest.lua', encoding='utf-8').read()
    for key, stack, s in lua_strings(src):
        if (role in stack and 'msg_fallbacks' not in stack
                and key != 'output' and 'TEST_PATTERN' not in stack):
            yield key, (stack[-1] if stack else None), s


def codes(s):
    return set(re.findall(r'%[a-zA-Z][a-zA-Z]?', s))


def dump(role, path):
    po = quest_entries(path)
    for i, (key, table, s) in enumerate(role_texts(role), 1):
        print('@@%d %s %s' % (i, table, key))
        print(po.get(s, '?? ' + s))


def gen(role, path):
    fr = quest_entries('po/fr.po')
    text = open(path, encoding='utf-8').read()
    new = {int(m.group(1)): m.group(2).rstrip('\n')
           for m in re.finditer(r'^@@(\d+)[^\n]*\n(.*?)(?=^@@|\Z)', text,
                                re.S | re.M)}
    seen = set()
    for i, (key, table, s) in enumerate(role_texts(role), 1):
        if i not in new:
            print('missing text %d' % i, file=sys.stderr)
            continue
        if s in seen:
            continue
        seen.add(s)
        old = fr.get(s, s)
        if codes(new[i]) != codes(old):
            print('%%-codes differ in text %d: %s / %s'
                  % (i, sorted(codes(old)), sorted(codes(new[i]))),
                  file=sys.stderr)
        print('msgctxt "quest"\nmsgid %s\nmsgstr %s\n'
              % (po_quote(s), po_quote(new[i])))


if len(sys.argv) in (3, 4) and sys.argv[1] == 'dump':
    dump(sys.argv[2], sys.argv[3] if len(sys.argv) == 4 else 'po/fr.po')
elif len(sys.argv) == 4 and sys.argv[1] == 'gen':
    gen(sys.argv[2], sys.argv[3])
else:
    sys.exit('usage: quest-theme.py dump ROLE [PO] | gen ROLE TEXTS')
