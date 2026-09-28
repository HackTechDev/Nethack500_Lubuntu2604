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
    --keyword=pline --keyword=pline_dir:2 --keyword=pline_xy:3 \
    --keyword=pline_mon:2 --keyword=custompline:2 \
    --keyword=urgent_pline --keyword=Norep --keyword=verbalize \
    --keyword=yn_function --keyword=getlin --keyword=yn --keyword=y_n \
    --keyword=ynq --keyword=paranoid_query:2 --keyword=paranoid_ynq:2 \
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
# shellcheck disable=SC2086
$XGETTEXT $COMMON --no-location -k --keyword=NC_:1c,2 \
    -o "$tmp/1monsters.pot" "$tmp/monsters.c"

$MSGCAT --no-wrap --sort-by-file --use-first -o po/nethack.pot "$tmp"/*.pot
