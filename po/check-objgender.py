#!/usr/bin/env python3
"""List the empty msgctxt "objgender" entries of po/fr.po whose gender
matters (whole object names and descriptions, not the "of" part of
"potion of %s"), with their French translation.  Run from the repo root.
usage: python3 po/check-objgender.py [--all]
Output: English name, translation, gender (- = masculine), roles."""
import re
import sys

OFNOUN = ('POTION', 'SCROLL', 'WAND', 'RING', 'SPELL')  # "potion of %s"
ADJDESC = ('POTION', 'RING', 'WAND', 'AMULET', 'SPELL', 'GEM', 'ROCK',
           'SCROLL')  # descriptions translated as adjectives

role = {}
for line in open('include/objects.h', encoding='utf-8'):
    m = re.match(r'^[ \t]*([A-Z_]+)\((?:OBJ\()?"([^"]*)"(.*)$', line)
    if not m:
        continue
    macro, name, rest = m.groups()
    if macro in ('GENERIC', 'XTRA_SCROLL_LABEL'):
        continue
    role.setdefault(name, set()).add(
        'of' if macro in OFNOUN else 'noun:' + macro)
    d = re.match(r'^[ \t]*,[ \t]*"([^"]*)"', rest)
    if d and macro not in ADJDESC:
        role.setdefault(d.group(1), set()).add('desc:' + macro)

po = open('po/fr.po', encoding='utf-8').read().split('\n\n')
trans = {}
for e in po:
    if 'msgctxt "object"\n' in e:
        mid = re.search(r'^msgid "(.*)"', e, re.M).group(1)
        m = re.search(r'msgstr\[0\] "(.*)"', e)
        trans[mid] = m.group(1) if m else ''
for e in po:
    if 'msgctxt "objgender"' not in e:
        continue
    mid = re.search(r'^msgid "(.*)"', e, re.M).group(1)
    val = re.search(r'msgstr "(.*)"', e).group(1)
    if val and '--all' not in sys.argv:
        continue
    r = role.get(mid, {'(code)'})
    if r == {'of'}:
        continue  # gender of "potion" &c applies
    print('%s\t%s\t%s\t%s' % (mid, trans.get(mid, '?'), val or '-',
                              ','.join(sorted(r))))
