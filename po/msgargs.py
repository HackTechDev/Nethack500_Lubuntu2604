#!/usr/bin/env python3
# NetHack 5.0  msgargs.py
# NetHack may be freely redistributed.  See license for details.
#
# Complement xgettext for po/update-pot.sh: xgettext only extracts the
# first string of an argument like  cond ? "a" : (c2 ? "b" : "c"),
# so every string literal of the message argument of pline(), You() &c
# is written out here as N_("...") (with the prefix that You() and
# similar functions add), ignoring the strings given to nested function
# calls (_(), C_() and the like are extracted by xgettext itself).
#
# usage: msgargs.py file.c... > out.c

import re
import sys

# function: (argument number, prefix)
FUNCS = {
    'pline': (1, ''), 'pline_dir': (2, ''), 'pline_xy': (3, ''),
    'pline_mon': (2, ''), 'custompline': (2, ''), 'urgent_pline': (1, ''),
    'Norep': (1, ''), 'verbalize': (1, ''), 'yn_function': (1, ''),
    'getlin': (1, ''), 'yn': (1, ''), 'y_n': (1, ''), 'ynq': (1, ''),
    'paranoid_query': (2, ''), 'paranoid_ynq': (2, ''),
    'selftouch': (1, ''), 'mselftouch': (2, ''),
    'You': (1, 'You '), 'Your': (1, 'Your '), 'You_feel': (1, 'You feel '),
    'You_cant': (1, "You can't "), 'pline_The': (1, 'The '),
    'There': (1, 'There '), 'You_hear': (1, 'You hear '),
    'You_see': (1, 'You see '),
}
CALL = re.compile(r'(?<![\w.>])(' + '|'.join(FUNCS) + r')\s*\(')


def skip_string(src, i):
    """index after the string or char literal starting at src[i]"""
    q = src[i]
    i += 1
    while i < len(src) and src[i] != q:
        i += 2 if src[i] == '\\' else 1
    return i + 1


def skip_comment(src, i):
    if src.startswith('/*', i):
        j = src.find('*/', i + 2)
        return len(src) if j < 0 else j + 2
    j = src.find('\n', i)
    return len(src) if j < 0 else j


def arguments(src, i):
    """split the call arguments starting after '(' at src[i]; each is a
    list of (kind, text) tokens: 'str' for literals outside nested
    function calls, 'other' for the rest"""
    args, cur, stack = [], [], []
    while i < len(src):
        c = src[i]
        if c == '"':
            j = skip_string(src, i)
            cur.append(('str', src[i + 1:j - 1]) if 'call' not in stack
                       else ('other', ''))
            i = j
            continue
        if c == "'":
            i = skip_string(src, i)
            cur.append(('other', ''))
            continue
        if src.startswith('/*', i) or src.startswith('//', i):
            i = skip_comment(src, i)
            continue
        if c == '(':
            iscall = re.search(r'\w\s*$', src[max(0, i - 40):i]) is not None
            stack.append('call' if iscall else 'group')
            cur.append(('other', c))
        elif c == ')':
            if not stack:
                if cur:
                    args.append(cur)
                return args
            stack.pop()
            cur.append(('other', c))
        elif c == ',' and not stack:
            args.append(cur)
            cur = []
        elif not c.isspace():
            cur.append(('other', c))
        i += 1
    return args


def literals(tokens):
    """the strings of an argument, adjacent literals concatenated"""
    out, prev_str = [], False
    for kind, text in tokens:
        if kind == 'str':
            if prev_str:
                out[-1] += text
            else:
                out.append(text)
            prev_str = True
        else:
            prev_str = False
    return out


def main():
    seen = set()
    for fname in sys.argv[1:]:
        src = open(fname, encoding='utf-8', errors='replace').read()
        for m in CALL.finditer(src):
            argno, prefix = FUNCS[m.group(1)]
            args = arguments(src, m.end())
            if len(args) < argno:
                continue
            for s in literals(args[argno - 1]):
                if not s:
                    continue
                s = prefix + s
                if s not in seen:
                    seen.add(s)
                    print('N_("%s");' % s)


main()
