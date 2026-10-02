/* NetHack 5.0	rip.c	$NHDT-Date: 1597967808 2020/08/20 23:56:48 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.33 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2017. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

/* Defining TEXT_TOMBSTONE causes genl_outrip() to exist, but it doesn't
   necessarily have to be used by a binary with multiple window-ports */

#if defined(TTY_GRAPHICS) || defined(X11_GRAPHICS) || defined(GEM_GRAPHICS) \
    || defined(DUMPLOG) || defined(CURSES_GRAPHICS) || defined(SHIM_GRAPHICS) \
    || defined(AMII_GRAPHICS)
#define TEXT_TOMBSTONE
#endif
#if defined(mac) || defined(__BEOS__)
#ifndef TEXT_TOMBSTONE
#define TEXT_TOMBSTONE
#endif
#endif

#ifdef TEXT_TOMBSTONE
staticfn void center(int, char *);
staticfn void killer_i18n(char *, unsigned, int);
staticfn void rip_word_i18n(char *, const char *);

#ifndef NH320_DEDICATION
/* A normal tombstone for end of game display. */
static const char *const rip_txt[] = {
    "                       ----------",
    "                      /          \\",
    "                     /    REST    \\",
    "                    /      IN      \\",
    "                   /     PEACE      \\",
    "                  /                  \\",
    "                  |                  |", /* Name of player */
    "                  |                  |", /* Amount of $ */
    "                  |                  |", /* Type of death */
    "                  |                  |", /* . */
    "                  |                  |", /* . */
    "                  |                  |", /* . */
    "                  |       1001       |", /* Real year of death */
    "                 *|     *  *  *      | *",
    "        _________)/\\\\_//(\\/(/\\)/\\//\\/|_)_______", 0
};
#define STONE_LINE_CENT 28 /* char[] element of center of stone face */
#else                      /* NH320_DEDICATION */
/* NetHack 3.2.x displayed a dual tombstone as a tribute to Izchak. */
static const char *const rip_txt[] = {
    "              ----------                      ----------",
    "             /          \\                    /          \\",
    "            /    REST    \\                  /    This    \\",
    "           /      IN      \\                /  release of  \\",
    "          /     PEACE      \\              /   NetHack is   \\",
    "         /                  \\            /   dedicated to   \\",
    "         |                  |            |  the memory of   |",
    "         |                  |            |                  |",
    "         |                  |            |  Izchak Miller   |",
    "         |                  |            |   1935 - 1994    |",
    "         |                  |            |                  |",
    "         |                  |            |     Ascended     |",
    "         |       1001       |            |                  |",
    "      *  |     *  *  *      | *        * |      *  *  *     | *",
    (" _____)/\\|\\__//(\\/(/\\)/\\//\\/|_)___"
     "_____)/|\\\\_/_/(\\/(/\\)/\\/\\/|_)____"),
    0
};
#define STONE_LINE_CENT 19 /* char[] element of center of stone face */
#endif                     /* NH320_DEDICATION */
#define STONE_LINE_LEN  16 /* # chars that fit on one line
                            * (note 1 ' ' border)           */
#define NAME_LINE  6 /* *char[] line # for player name */
#define GOLD_LINE  7 /* *char[] line # for amount of gold */
#define DEATH_LINE 8 /* *char[] line # for death description */
#define YEAR_LINE 12 /* *char[] line # for year */

staticfn void
center(int line, char *text)
{
    char *ip, *op;
    int width = 0, start;

    /* width in characters, not bytes (UTF-8 translations) */
    for (ip = text; *ip; ++ip)
        if ((*ip & 0xc0) != 0x80)
            ++width;
    start = STONE_LINE_CENT - ((width + 1) >> 1);
    if (width != (int) strlen(text)) {
        /* rebuild the line around the multibyte text */
        char *old = gr.rip[line], *nl;
        size_t oldlen = strlen(old);

        if ((size_t) (start + width) > oldlen)
            return;
        nl = (char *) alloc((unsigned) (oldlen + strlen(text) + 1));
        Sprintf(nl, "%.*s%s%s", start, old, text, old + start + width);
        free((genericptr_t) old);
        gr.rip[line] = nl;
        return;
    }
    ip = text;
    op = &gr.rip[line][start];
    while (*ip)
        *op++ = *ip++;
}

#if 0
/* killed_by_prefix[] of formatkiller(), translated for the tombstone */
NC_("killed by", "killed by ") NC_("killed by", "choked on ")
NC_("killed by", "poisoned by ") NC_("killed by", "died of ")
NC_("killed by", "drowned in ") NC_("killed by", "burned by ")
NC_("killed by", "dissolved in ") NC_("killed by", "crushed to death by ")
NC_("killed by", "petrified by ") NC_("killed by", "turned to slime by ")
NC_("feminine killed by", "killed by ")
NC_("feminine killed by", "choked on ")
NC_("feminine killed by", "poisoned by ")
NC_("feminine killed by", "died of ")
NC_("feminine killed by", "drowned in ")
NC_("feminine killed by", "burned by ")
NC_("feminine killed by", "dissolved in ")
NC_("feminine killed by", "crushed to death by ")
NC_("feminine killed by", "petrified by ")
NC_("feminine killed by", "turned to slime by ")
NC_("tombstone", "    REST    ") NC_("tombstone", "      IN      ")
NC_("tombstone", "     PEACE      ")
#endif

/* replace English word 'en' on tombstone line by its translation of the
   same width (lines are allocated with their exact length) */
staticfn void
rip_word_i18n(char *line, const char *en)
{
    const char *tr = C_("tombstone", en);

    if (strlen(tr) == strlen(en))
        (void) strsubst(line, en, tr);
}

#if 0
/* death reasons (svk.killer.name) translated for the tombstone:
   msgctxt "killer a" with the indefinite article, else "killer" */
NC_("killer", "a grappling hook")
NC_("killer a", "acidic chair")
NC_("killer a", "acidic corpse")
NC_("killer a", "acidic glob")
NC_("killer a", "alchemic blast")
NC_("killer a", "anti-magic implosion")
NC_("killer", "axing a hard object")
NC_("killer a", "bear trap")
NC_("killer", "boiling water")
NC_("killer a", "cadaver")
NC_("killer a", "carnivorous bag")
NC_("killer a", "closing drawbridge")
NC_("killer a", "collapsing drawbridge")
NC_("killer", "colliding with the ceiling")
NC_("killer", "committed suicide")
NC_("killer", "contusion from a small passage")
NC_("killer", "crunched in the head by an iron ball")
NC_("killer", "crushed to death underneath a drawbridge")
NC_("killer a", "cursed throne")
NC_("killer", "dangerous winds")
NC_("killer", "deliberately meeting Medusa's gaze")
NC_("killer", "deliberately plunged into a pit")
NC_("killer", "dragged downstairs by an iron ball")
NC_("killer a", "electric chair")
NC_("killer a", "electric shock")
NC_("killer", "elementary physics")
NC_("killer", "exhaustion")
NC_("killer a", "exploding crystal ball")
NC_("killer a", "exploding drawbridge")
NC_("killer a", "exploding ring")
NC_("killer a", "exploding rune")
NC_("killer a", "exploding wand")
NC_("killer a", "explosion")
NC_("killer", "falling down a mine shaft")
NC_("killer a", "falling drawbridge")
NC_("killer a", "falling object")
NC_("killer", "falling off a ladder")
NC_("killer a", "falling rock")
NC_("killer", "fell from a drawbridge")
NC_("killer", "fell into a chasm")
NC_("killer", "fell into a pit")
NC_("killer a", "gas cloud")
NC_("killer", "genocidal confusion")
NC_("killer", "hurt in a chasm")
NC_("killer a", "imperious order")
NC_("killer a", "iron ball collision")
NC_("killer", "jumping out of a bear trap")
NC_("killer", "killed by petrification")
NC_("killer", "killed while stuck in creature form")
NC_("killer a", "land mine")
NC_("killer", "leg damage from being pulled out of a bear trap")
NC_("killer a", "magical explosion")
NC_("killer a", "mildly contaminated potion")
NC_("killer", "molten lava")
NC_("killer a", "potion of acid")
NC_("killer a", "potion of holy water")
NC_("killer a", "potion of unholy water")
NC_("killer a", "propelled potion")
NC_("killer a", "psychic blast")
NC_("killer", "quaffing a burning potion of oil")
NC_("killer a", "quit while already on Charon's boat")
NC_("killer a", "residual undead turning effect")
NC_("killer a", "riding accident")
NC_("killer a", "rotted glob")
NC_("killer a", "rotten lump of royal jelly")
NC_("killer", "rusting away")
NC_("killer a", "scroll of earth")
NC_("killer a", "scroll of fire")
NC_("killer a", "scroll of genocide")
NC_("killer", "self-genocide")
NC_("killer", "sipping boiling water")
NC_("killer", "sitting in lava")
NC_("killer", "sitting on an iron spike")
NC_("killer", "sitting on lava")
NC_("killer", "slimicide")
NC_("killer", "squished under a boulder")
NC_("killer", "starvation")
NC_("killer a", "system shock")
NC_("killer a", "thrown potion")
NC_("killer", "tumbling down a flight of stairs")
NC_("killer", "turned into green slime")
NC_("killer a", "unrefrigerated sip of juice")
NC_("killer a", "unsuccessful polymorph")
NC_("killer a", "wand")
NC_("killer", "went to heaven prematurely")
NC_("killer", "tasting cockatrice meat")
NC_("killer", "touching a cockatrice corpse")
#endif

/* translated death description for the tombstone; formatkiller() keeps
   the English one that is written to the record and log files */
staticfn void
killer_i18n(char *buf, unsigned siz, int how)
{
    static const char *const prefixes[] = {
        /* DIED, CHOKING, POISONING, STARVING, */
        "killed by ", "choked on ", "poisoned by ", "died of ",
        /* DROWNING, BURNING, DISSOLVED, CRUSHING, */
        "drowned in ", "burned by ", "dissolved in ", "crushed to death by ",
        /* STONING, TURNED_SLIME, GENOCIDED, */
        "petrified by ", "turned to slime by ", "killed by ",
        /* PANICKED, TRICKED, QUIT, ESCAPED, ASCENDED */
        "", "", "", "", ""
    };
    const char *pfx = "", *tr = 0;
#ifdef NHI18N
    const char *kname = svk.killer.name;
#endif

    if (svk.killer.format == KILLED_BY || svk.killer.format == KILLED_BY_AN) {
        const char *en = (how >= 0 && how < SIZE(prefixes)) ? prefixes[how]
                                                            : "";

        if (*en) {
            pfx = flags.female ? C_("feminine killed by", en)
                               : C_("killed by", en);
            if (pfx == en) /* no feminine form */
                pfx = C_("killed by", en);
        }
    }
#ifdef NHI18N
    if (svk.killer.format == KILLED_BY_AN) {
        if (strcmp(C_("monster", kname), kname))
            tr = i18n_an_ctx("monster", kname);
        else
            tr = i18n_lookup("killer a", kname);
    } else {
        if (strcmp(C_("monster", kname), kname))
            tr = C_("monster", kname);
        else
            tr = i18n_lookup("killer", kname);
    }
#endif
    if (!tr) { /* no translation: the English description */
        char eng[BUFSZ];

        formatkiller(eng, sizeof eng, how, FALSE);
        Snprintf(buf, siz, "%s", eng);
        return;
    }
    Snprintf(buf, siz, "%s%s", pfx, tr);
}

void
genl_outrip(winid tmpwin, int how, time_t when)
{
    char **dp;
    char *dpx;
    char buf[BUFSZ];
    int x;
    int line, year;
    long cash;

    gr.rip = dp = (char **) alloc(sizeof(rip_txt));
    for (x = 0; rip_txt[x]; ++x)
        dp[x] = dupstr(rip_txt[x]);
    dp[x] = (char *) 0;
    if (i18n_active() && tmpwin != 0) {
        /* same widths: "REST" "IN" "PEACE" */
        for (x = 0; dp[x]; ++x) {
            rip_word_i18n(dp[x], "    REST    ");
            rip_word_i18n(dp[x], "      IN      ");
            rip_word_i18n(dp[x], "     PEACE      ");
        }
    }

    /* Put name on stone */
    Sprintf(buf, "%.*s", (int) STONE_LINE_LEN, svp.plname);
    center(NAME_LINE, buf);

    /* Put $ on stone */
    cash = max(gd.done_money, 0L);
    /* arbitrary upper limit; practical upper limit is quite a bit less */
    if (cash > 999999999L)
        cash = 999999999L;
    Sprintf(buf, "%ld Au", cash);
    center(GOLD_LINE, buf);

    /* Put together death description */
    if (i18n_active() && tmpwin != 0)
        killer_i18n(buf, sizeof buf, how);
    else
        formatkiller(buf, sizeof buf, how, FALSE);

    /* Put death type on stone */
    for (line = DEATH_LINE, dpx = buf; line < YEAR_LINE; line++) {
        char tmpchar;
        int i, i0 = (int) strlen(dpx);

        if (i0 > STONE_LINE_LEN) {
            for (i = STONE_LINE_LEN; (i > 0) && (i0 > STONE_LINE_LEN); --i)
                if (dpx[i] == ' ')
                    i0 = i;
            if (!i)
                i0 = STONE_LINE_LEN;
        }
        tmpchar = dpx[i0];
        dpx[i0] = 0;
        center(line, dpx);
        if (tmpchar != ' ') {
            dpx[i0] = tmpchar;
            dpx = &dpx[i0];
        } else
            dpx = &dpx[i0 + 1];
    }

    /* Put year on stone */
    year = (int) ((yyyymmdd(when) / 10000L) % 10000L);
    Sprintf(buf, "%4d", year);
    center(YEAR_LINE, buf);

#ifdef DUMPLOG
    if (tmpwin == 0)
        dump_forward_putstr(0, 0, "Game over:", TRUE);
    else
#endif
        putstr(tmpwin, 0, "");

    for (; *dp; dp++)
        putstr(tmpwin, 0, *dp);

    putstr(tmpwin, 0, "");
#ifdef DUMPLOG
    if (tmpwin != 0)
#endif
        putstr(tmpwin, 0, "");

    for (x = 0; rip_txt[x]; x++) {
        free((genericptr_t) gr.rip[x]);
    }
    free((genericptr_t) gr.rip);
    gr.rip = 0;
}

#endif /* TEXT_TOMBSTONE */

/*rip.c*/
