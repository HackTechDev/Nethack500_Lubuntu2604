#!/usr/bin/env python3
"""Check a theme catalog (po/themes/<lang>@<theme>.po, see CYBERPUNK.md):
entries that are not in po/nethack.pot (typo or text gone from the game),
and renamed objects or monsters without an explicit gender entry (they
would keep the gender of their name in the language's catalog).
usage: python3 po/check-theme.py po/themes/fr@cyber.po"""
import re
import sys


def entries(path):
    """(msgctxt, msgid) -> msgstr (first form) of a .po/.pot file"""
    res = {}
    for blk in open(path, encoding='utf-8').read().split('\n\n'):
        ctx = re.search(r'^msgctxt "(.*)"$', blk, re.M)
        mid = re.search(r'^msgid "(.*)"$', blk, re.M)
        if not mid or not mid.group(1):
            continue
        st = re.search(r'^msgstr(?:\[0\])? "(.*)"$', blk, re.M)
        res[(ctx.group(1) if ctx else None, mid.group(1))] = \
            st.group(1) if st else ''
    return res


theme = entries(sys.argv[1])
pot = entries('po/nethack.pot')
bad = 0
for key in theme:
    if key not in pot:
        print('not in nethack.pot: %s %r' % key)
        bad += 1
for (ctx, mid), val in theme.items():
    want = {'object': 'objgender', 'monster': 'gender'}.get(ctx)
    if want and (want, mid) not in theme:
        print('no "%s" entry for renamed %s %r' % (want, ctx, mid))
        bad += 1
    if ctx in ('objgender', 'gender') and val == '':
        print('empty gender for %r (write "m" for masculine)' % mid)
        bad += 1
print('%d entries, %d problems' % (len(theme), bad))
sys.exit(1 if bad else 0)
