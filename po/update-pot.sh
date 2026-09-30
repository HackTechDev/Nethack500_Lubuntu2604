#!/bin/sh
# NetHack 5.0  update-pot.sh
# NetHack may be freely redistributed.  See license for details.
#
# Extract the translatable strings of the sources into po/nethack.pot
# (run by 'make update-po').
#
# Besides the strings marked with _() &c, the messages given to pline()
# and the other message functions are extracted, since vpline() looks
# them up.  You(), pline_The() and similar functions add a prefix ("You ",
# "The "...) to their message before the lookup, so that prefix is added
# to the strings extracted from their calls.

set -e
cd "$(dirname "$0")/.."

XGETTEXT=${XGETTEXT:-xgettext}
MSGCAT=${MSGCAT:-msgcat}
SRC="src/*.c win/tty/*.c win/curses/*.c"
COMMON="--language=C --from-code=UTF-8 --no-wrap --add-comments=TRANSLATORS:"

tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' 0

# shellcheck disable=SC2086
$XGETTEXT $COMMON --package-name=NetHack --package-version=5.0.0 \
    --keyword=_ --keyword=N_ \
    --keyword=C_:1c,2 --flag=C_:2:c-format --keyword=NC_:1c,2 \
    --keyword=NCP_:1c,2,3 \
    --keyword=pline --keyword=pline_dir:2 --keyword=pline_xy:3 \
    --keyword=pline_mon:2 --keyword=custompline:2 \
    --keyword=urgent_pline --keyword=Norep --keyword=verbalize \
    --keyword=yn_function --keyword=getlin --keyword=yn --keyword=y_n \
    --keyword=ynq --keyword=paranoid_query:2 --keyword=paranoid_ynq:2 \
    --keyword=selftouch --keyword=mselftouch:2 --keyword=objnam_fmt \
    --keyword=strange_feeling:2 --keyword=prinv:1 --keyword=query_category:1 \
    --keyword=query_objlist:1 --keyword=ia_addmenu:4 \
    --keyword=hold_another_object:2 --keyword=hold_another_object:4 \
    -o "$tmp/0base.pot" $SRC

# function:prefix
for spec in "You:You " "Your:Your " "You_feel:You feel " \
            "You_cant:You can't " "pline_The:The " "There:There " \
            "You_hear:You hear " "You_see:You see "; do
    fn=${spec%%:*}
    prefix=${spec#*:}
    # shellcheck disable=SC2086
    $XGETTEXT $COMMON --force-po -k --keyword="$fn" \
        -o "$tmp/$fn.pot" $SRC
    # prefix every msgid except the empty one of the header
    sed "s/^msgid \"\\([^\"]\\)/msgid \"$prefix\\1/" "$tmp/$fn.pot" \
        > "$tmp/$fn.pot.new"
    mv "$tmp/$fn.pot.new" "$tmp/$fn.pot"
done

# monster names (include/monsters.h) are translated with the context
# "monster"; the "gender" entry of a name gives the grammatical gender of
# its translation ("f" for feminine, empty for masculine; see do_name.c)
grep -o 'NAMS\{0,1\}([^)]*)' include/monsters.h | grep -o '"[^"]*"' \
    | sort -u | sed 's/.*/NC_("monster", &); NC_("gender", &);/' \
    > "$tmp/monsters.c"
# hallucinatory monsters (dat/bogusmon.txt, without their prefix code)
# are translated the same way
grep -v '^#' dat/bogusmon.txt | sed -n 's/^[-_+|=]\{0,1\}\(..*\)$/"\1"/p' \
    | sort -u | sed 's/.*/NC_("monster", &); NC_("gender", &);/' \
    >> "$tmp/monsters.c"
# shellcheck disable=SC2086
$XGETTEXT $COMMON --no-location -k --keyword=NC_:1c,2 \
    -o "$tmp/1monsters.pot" "$tmp/monsters.c"

# object names and descriptions (include/objects.h) are translated with
# the context "object" (with a plural form) and their grammatical gender
# with the context "objgender"; descriptions of potions, rings, wands,
# amulets, spellbooks, gems and scrolls are adjectives ("bubbly potion"),
# translated without context (masculine) or with "feminine" (see objnam.c)
awk '
match($0, /^[ \t]*[A-Z_]+\((OBJ\()?"[^"]*"/) {
    s = substr($0, RSTART, RLENGTH); rest = substr($0, RSTART + RLENGTH)
    macro = s; sub(/^[ \t]*/, "", macro); sub(/\(.*/, "", macro)
    name = s; sub(/^[^"]*/, "", name)
    if (macro == "GENERIC" || macro == "XTRA_SCROLL_LABEL")
        next
    plural = substr(name, 1, length(name) - 1) "s\""
    printf "NCP_(\"object\", %s, %s); NC_(\"objgender\", %s);\n", \
           name, plural, name
    if (match(rest, /^[ \t]*,[ \t]*"[^"]*"/)) {
        desc = substr(rest, RSTART, RLENGTH); sub(/^[^"]*/, "", desc)
        if (macro ~ /^(POTION|RING|WAND|AMULET|SPELL|GEM|ROCK|SCROLL)$/) {
            printf "N_(%s); C_(\"feminine\", %s);\n", desc, desc
        } else {
            plural = substr(desc, 1, length(desc) - 1) "s\""
            printf "NCP_(\"object\", %s, %s); NC_(\"objgender\", %s);\n", \
                   desc, plural, desc
        }
    }
}' include/objects.h > "$tmp/objects.c"
# shellcheck disable=SC2086
$XGETTEXT $COMMON --no-location -k --keyword=NCP_:1c,2,3 \
    --keyword=NC_:1c,2 --keyword=N_ --keyword=C_:1c,2 \
    -o "$tmp/2objects.pot" "$tmp/objects.c"

# verbs given to otense(), vtense(), aobjnam(), yobjnam(), Yobjnam2() and
# Tobjnam() are conjugated for the object with the context "objverb":
# third person singular and plural
# shellcheck disable=SC2086
grep -ohE '(Yobjnam2|yobjnam|Tobjnam|aobjnam|otense|vtense)\([^;()]*, *"[^"]+"\)' \
    $SRC | grep -oE '"[^"]+"\)$' | sed 's/)$//' | sort -u \
    | sed 's/.*/NCP_("objverb", &, &);/' > "$tmp/verbs.c"
# shellcheck disable=SC2086
$XGETTEXT $COMMON --no-location -k --keyword=NCP_:1c,2,3 \
    -o "$tmp/3verbs.pot" "$tmp/verbs.c"

# xgettext only takes the first string of "cond ? "a" : "b"" given to the
# message functions; po/msgargs.py lists all of them
# shellcheck disable=SC2086
python3 po/msgargs.py $SRC > "$tmp/msgargs.c"
# shellcheck disable=SC2086
$XGETTEXT $COMMON --no-location -k --keyword=N_ \
    -o "$tmp/zz_msgargs.pot" "$tmp/msgargs.c"

$MSGCAT --no-wrap --sort-by-file --use-first -o po/nethack.pot "$tmp"/*.pot
