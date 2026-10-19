#!/usr/bin/env python3
"""Convert doc/Guidebook-fr.mn (troff, tmac.n/tmac.nh) to LaTeX in the style
of doc/Guidebook.tex, giving doc/Guidebook-fr.tex.

usage (in doc/): python3 mn2tex-fr.py Guidebook-fr.mn > Guidebook-fr.tex

Only the macros and escapes used by Guidebook-fr.mn are handled; check the
result (balanced braces and environments) after changing the .mn file."""
import re
import sys

ESC = [  # roff escape -> LaTeX (applied after LaTeX special chars escaping)
    (r'\(Fo', '«'), (r'\(Fc', '»'), (r'\(oq', '`'), (r'\(cq', "'"),
    (r'\(lq', '``'), (r'\(rq', "''"), (r'\(dq', '\\textquotedbl{}'),
    (r'\(ha', '\\textasciicircum{}'), (r'\(ti', '\\textasciitilde{}'),
    (r'\(em', '---'), (r'\(rs', '\\textbackslash{}'),
    (r'\(aq', '\\textquotesingle{}'), (r'\(bu', '\\textbullet{}'),
    (r'\(rg', '\\textregistered{}'), (r'\~', '~'), (r'\-', '-'),
    (r'\&', ''), (r'\e', '\\textbackslash{}'), (r'\ ', '~'), (r'\.', '.'),
    (r'\`', '`'), (r'\0', '~'), (r'\|', ''), (r'\^', ''), (r'\%', ''),
    (r'\\', '\\textbackslash{}'),
]
SPECIAL = {'#': '\\#', '$': '\\$', '%': '\\%', '&': '\\&', '_': '\\_',
           '{': '\\{', '}': '\\}', '~': '\\textasciitilde{}',
           '^': '\\textasciicircum{}', '<': '\\textless{}',
           '>': '\\textgreater{}', '|': '\\textbar{}', '"': '\\textquotedbl{}'}

font_open = []  # stack of open LaTeX groups for \f fonts


PREAMBLE = r"""% Traduction française de Guidebook.tex (même révision que l'original).
% Produit à partir de Guidebook-fr.mn ; garder les deux traductions
% d'accord quand Guidebook.mn ou Guidebook.tex changent.
\documentclass[titlepage]{article}
\usepackage[utf8]{inputenc}
\usepackage[T1]{fontenc}
\usepackage{lmodern}
\usepackage[french]{babel}
\usepackage[hidelinks]{hyperref}
\usepackage{longtable}
\usepackage{enumitem}
\usepackage[a4paper, text={160mm, 220mm}, centering]{geometry}

% le texte contient déjà les espaces insécables avant ; : ! ?
\frenchsetup{AutoSpacePunctuation=false}

\setlist[description]{leftmargin=30mm, topsep=2mm, partopsep=0mm, parsep=0mm, itemsep=1mm, labelwidth=28mm, labelsep=2mm}

\hyphenation{CRASHREPORTURL}

\begin{document}

"""


def inline(s):
    """convert one line of roff text to LaTeX"""
    out = []
    i = 0
    while i < len(s):
        c = s[i]
        if c == '\\':
            if s.startswith('\\"', i):  # trailing comment
                break
            if s.startswith('\\f(CR', i):
                out.append('\\texttt{'); font_open.append('}'); i += 5
                continue
            if s.startswith('\\f', i) and i + 2 < len(s):
                f = s[i + 2]
                if f in 'IB':
                    out.append('\\textit{' if f == 'I' else '\\textbf{')
                    font_open.append('}')
                elif f in 'PR':
                    if font_open:
                        out.append(font_open.pop())
                i += 3
                continue
            if s.startswith('\\*(', i):  # string register
                name = s[i + 3:i + 5]
                out.append({'vr': 'NetHack 5.0.0', 'f2': '2 mai 2026',
                            'rg': '\\textregistered{}'}.get(name, ''))
                i += 5
                continue
            for k, v in ESC:
                if s.startswith(k, i):
                    out.append(v); i += len(k)
                    break
            else:
                out.append('\\textbackslash{}'); i += 1
            continue
        out.append(SPECIAL.get(c, c))
        i += 1
    return ''.join(out)


def close_fonts():
    r = ''.join(reversed(font_open))
    font_open.clear()
    return r


def args(line):
    # roff arguments: separated by spaces, "..." groups, backslashes kept;
    # a trailing \" comment ends the line
    line = re.sub(r'\s*\\".*$', '', line)
    res, cur, q, had = [], '', False, False
    for ch in line:
        if ch == '"':
            q = not q
            had = True
        elif ch == ' ' and not q:
            if cur or had:
                res.append(cur)
            cur, had = '', False
        else:
            cur += ch
    if cur or had:
        res.append(cur)
    return [x.replace('\\\\', '\\') for x in res]


def table(tbl):
    """convert a tbl table (lines between .TS and .TE)"""
    # options and format lines end with ';' and '.'
    k = 0
    opts = ''
    if tbl and tbl[0].rstrip().endswith(';'):
        opts = tbl[0]
        k = 1
    fmts = []
    while k < len(tbl):
        fmts.append(tbl[k])
        k += 1
        if fmts[-1].rstrip().endswith('.'):
            break
    rows = []
    cur = None
    for l in tbl[k:]:
        if cur is not None:
            if l.startswith('T}'):
                rows[-1][-1] = cur.strip()
                rest = l[2:].lstrip('\t')
                if rest:
                    rows[-1].extend(rest.split('\t'))
                cur = None
            else:
                cur += ' ' + l
            continue
        if l.startswith('.'):
            continue
        cells = l.split('\t')
        if cells and cells[-1] == 'T{':
            cells[-1] = ''
            cur = ''
        rows.append(cells)
    if 'box' in opts and len(fmts) == 1 and fmts[0].strip() == 'L.':
        res = ['', '\\vbox{', '\\begin{verbatim}']  # end the paragraph
        for r in rows:
            t = r[0]
            for a, b in [('\\&', ''), ('\\-', '-'), ('\\(ha', '^')]:
                t = t.replace(a, b)
            res.append('        ' + t)
        res += ['\\end{verbatim}', '}']
        return res
    if 'box' in opts:  # Figure 3, the movement keys
        return ['\\begin{center}', '\\begin{tabular}{cc}',
                '\\verb+   y  k  u   + & \\verb+   7  8  9   +\\\\',
                '\\verb+    \\ | /    + & \\verb+    \\ | /    +\\\\',
                '\\verb+   h- . -l   + & \\verb+   4- . -6   +\\\\',
                '\\verb+    / | \\    + & \\verb+    / | \\    +\\\\',
                '\\verb+   b  j  n   + & \\verb+   1  2  3   +\\\\',
                '(\\textit{number\\_pad} désactivé) & '
                '(\\textit{number\\_pad} activé)',
                '\\end{tabular}', '\\end{center}']
    ncol = max(len(re.findall(r'[lrcLRCs]', fmts[-1].split('.')[0]
                              .replace('fCR', ''))), 1)
    spans = [f for f in fmts[:-1] if ' s' in f]
    env = 'longtable' if len(rows) > 30 else 'tabular'
    res = ['\\begin{center}'] if env == 'tabular' else []
    res.append('\\begin{%s}{%s}' % (env, 'l' * ncol))
    first = True
    for r in rows:
        if all(c.strip() in ('\\_', '_', '=') for c in r):
            res.append('\\hline')
            continue
        cells = [inline(c) + close_fonts() for c in r]
        if first and spans and len(cells) == 1:
            res.append('\\multicolumn{%d}{l}{%s}\\\\' % (ncol, cells[0]))
        else:
            cells += [''] * (ncol - len(cells))
            res.append(' & '.join(cells[:ncol]) + '\\\\')
        first = False
    res.append('\\end{%s}' % env)
    if env == 'tabular':
        res.append('\\end{center}')
    return res


def main():
    lines = open(sys.argv[1], encoding='utf-8').read().split('\n')
    # skip the macro header: start at .mt
    start = next(i for i, l in enumerate(lines) if l.startswith('.mt'))
    out = []
    lists = []  # stack of 'desc' (from .lp) / 'pl' (.PS) / 'cc'
    verb = False  # inside verbatim
    disp = False  # inside .sd/.SD display
    i = start
    pending_title = None

    def end_lp():
        while lists and lists[-1] == 'desc':
            out.append('\\end{description}')
            lists.pop()

    while i < len(lines):
        l = lines[i]
        i += 1
        if pending_title is not None:
            lvl = pending_title
            pending_title = None
            cmd = {1: 'section', 2: 'subsection', 3: 'subsubsection'}.get(
                lvl, 'section*')
            out.append('\\%s{%s}' % (cmd, inline(l) + close_fonts()))
            out.append('')
            continue
        if verb:
            if l.startswith('.ft') or l.startswith('.ed') or \
               l.startswith('.ED'):
                out.append('\\end{verbatim}')
                verb = False
                if l.startswith('.ed') or l.startswith('.ED'):
                    disp = False
                continue
            if l.startswith('.\\"') or l.startswith('.'):
                continue
            # roff escapes still need decoding, but no LaTeX escaping
            t = l
            for k, v in [('\\(dq', '"'), ('\\(ti', '~'), ('\\(ha', '^'),
                         ('\\e', '\\'), ('\\-', '-'), ('\\&', ''),
                         ('\\\\', '\\'), ('\\f(CR', ''), ('\\fP', ''),
                         ('\\fR', ''), ('\\fI', ''), ('\\fB', '')]:
                t = t.replace(k, v)
            t = re.sub(r'\s*\\".*$', '', t)
            out.append('    ' + t)
            continue
        if l.startswith('.TS'):
            tbl = []
            while i < len(lines) and not lines[i].startswith('.TE'):
                tbl.append(lines[i])
                i += 1
            i += 1
            out.extend(table(tbl))
            continue
        if l.startswith('.\\"') or l.startswith('."') or l == '.' \
           or l.startswith('. '):
            if l.startswith('.\\"'):
                out.append('%' + l[3:])
            elif l.startswith('."'):
                out.append('%' + l[2:])
            continue
        if l.startswith('.'):
            a = args(l[1:])
            if not a:
                continue
            m = a[0]
            if m == 'mt':
                t1, t2 = lines[i], lines[i + 1]
                out.append('\\title{\\LARGE %s~:\\\\\n'
                           '\\Large Guide de \\textit{NetHack}}' % inline(t1))
                i += 2
            elif m == 'au':
                a1, a2 = lines[i], lines[i + 1]
                out.append('\\author{%s\\\\\n%s}' % (inline(a1), inline(a2)))
                out.append('\\date{2 mai 2026}\n\n\\maketitle\n')
                i += 2
                # skip .sp and the date line
                while i < len(lines) and not lines[i].startswith('.hn'):
                    i += 1
            elif m == 'hn':
                out.append(close_fonts())
                while lists:
                    out.append('\\end{description}')
                    lists.pop()
                lvl = int(a[1]) if len(a) > 1 and a[1].isdigit() else 0
                pending_title = lvl
            elif m == 'pg':
                out.append(close_fonts())
                end_lp()
                out.append('')
            elif m == 'lp':
                out.append(close_fonts())
                lab = a[1] if len(a) > 1 else ''
                if lists and lists[-1] in ('pl', 'cc'):
                    pass
                if lab.strip() == '' and (not lists or lists[-1] != 'desc'):
                    out.append('')  # continuation outside a list
                    continue
                if not lists or lists[-1] != 'desc':
                    out.append('\\begin{description}')
                    lists.append('desc')
                if lab.strip() == '':
                    out.append('')  # continuation paragraph
                else:
                    out.append('%%.lp\n\\item[{%s}]' %
                               (inline(lab.strip()) + close_fonts()))
            elif m == 'op':
                name = a[1] if len(a) > 1 else ''
                suf = a[2] if len(a) > 2 else ''
                pre = a[3] if len(a) > 3 else ''
                out.append(inline(pre) + '\\textit{' + inline(name) +
                           close_fonts() + '}' + inline(suf) + close_fonts())
            elif m in ('PS',):
                out.append('\\begin{description}')
                lists.append('pl')
            elif m == 'PL':
                out.append('\\item[{%s}]' % (inline(a[1] if len(a) > 1
                                                    else '')
                                             + close_fonts()))
            elif m == 'PE':
                if lists and lists[-1] == 'pl':
                    out.append('\\end{description}')
                    lists.pop()
            elif m == 'CC':
                out.append('\\texttt{%s} --- %s\\\\' %
                           (inline(a[1]), inline(a[2] if len(a) > 2 else '')
                            + close_fonts()))
            elif m in ('sd', 'SD'):
                disp = True
                out.append('%.sd')
            elif m in ('ed', 'ED'):
                disp = False
                out.append('%.ed')
            elif m == 'ft':
                if len(a) > 1 and a[1] == 'CR' and disp:
                    out.append('\\begin{verbatim}')
                    verb = True
            elif m == 'UX':
                suf = a[1] if len(a) > 1 else ''
                pre = a[2] if len(a) > 2 else ''
                out.append(inline(pre) + 'UNIX' + inline(suf))
            elif m in ('BR', 'sp'):
                out.append('\\medskip')
            elif m == 'ce':
                while i < len(lines) and lines[i].startswith('.'):
                    i += 1
                out.append('\\begin{center}\n%s\n\\end{center}' %
                           (inline(lines[i]) + close_fonts()))
                i += 1
            elif m in ('si', 'ei', 'in', 'hw', 'if', 'ie', 'el', 'ds', 'nr',
                       'tr', 'de', 'rm', 'So', 'tm', 'ab', 'po', 'll', 'lt',
                       'sm', 'UR', 'TE'):
                out.append('%' + l)
            else:
                out.append('%UNKNOWN ' + l)
            continue
        # plain text
        if disp and l.strip() and not verb:
            out.append(inline(l) + '\\\\')
        else:
            out.append(inline(l))
    out.append(close_fonts())
    while lists:
        out.append('\\end{description}')
        lists.pop()
    text = '\n'.join(out).split('\n')
    # a line starting with '[' after one ending with \\ would be read as
    # the optional argument of \\
    for n in range(1, len(text)):
        if text[n].lstrip().startswith('[') and text[n - 1].endswith('\\\\'):
            text[n] = '{}' + text[n]
    sys.stdout.write(PREAMBLE + '\n'.join(text) + '\n\n\\end{document}\n')


main()
