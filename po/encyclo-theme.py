# NetHack 5.0  encyclo-theme.py
# NetHack may be freely redistributed.  See license for details.
#
# Write the encyclopedia of a theme (CYBERPUNK.md):
#   python3 po/encyclo-theme.py SOURCE.base TEXTS...
# copies SOURCE.base (dat/data-fr.base, or dat/data-fr@cyber.base to
# complete it) into dat/data-fr@cyber.base, replacing the text of the
# entries named in the TEXTS files: "@@ key" lines (the first key of an
# entry, as in SOURCE) followed by the new text, plain lines for the
# description and "[ source ]" for the attribution.  The keys stay those
# of data.base: the game looks entries up by the English name.
# Run from the top of the source tree.
import sys
src = open(sys.argv[1], encoding='utf-8').read().split('\n')
new = {}
for path in sys.argv[2:]:
    cur = None
    for ln in open(path, encoding='utf-8').read().split('\n'):
        if ln.startswith('@@ '):
            cur = ln[3:].strip(); new[cur] = []
        elif cur is not None and ln.strip():
            new[cur].append(ln.rstrip())
out = []; i = 0; done = set(); n = len(src)
hdr_done = False
while i < n:
    ln = src[i]
    if not hdr_done and ln.startswith('#'):
        out.append(ln); i += 1; continue
    if not hdr_done and not any('thème cyberpunk' in x for x in out):
        out[1:1] = ['# Version du thème cyberpunk (CYBERPUNK.md) : compilée en',
                    '# « data.fr@cyber » par « MAKEDEFS_LANG=fr@cyber makedefs -d ».',
                    '# Les entrées des choses renommées par le thème ont un texte',
                    '# écrit pour Night City ; les autres reprennent data-fr.base.']
    hdr_done = True
    if ln and ln[0] not in '\t #':
        # the keys, possibly with comment lines between them
        keys = []
        while i < n and src[i] and src[i][0] not in '\t ':
            keys.append(src[i]); i += 1
        while keys and keys[-1].startswith('#'):
            keys.pop(); i -= 1
        out.extend(keys)
        k0 = keys[0].strip()  # the first key names the entry
        if k0 in new:
            done.add(k0)
            # the old text, paragraphs separated by blank lines included
            while i < n and (src[i].startswith('\t') or src[i].startswith(' ')
                             or not src[i].strip()):
                i += 1
            for t in new[k0]:
                t = t.strip()
                out.append(('\t\t' + t) if t.startswith('[') else ('\t' + t))
            out.append('')
        continue
    out.append(ln); i += 1
open('dat/data-fr@cyber.base', 'w', encoding='utf-8').write('\n'.join(out))
miss = set(new) - done
print('%d entrées réécrites' % len(done), ('; clés absentes : %s' % sorted(miss)) if miss else '')
