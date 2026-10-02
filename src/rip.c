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

/* translated death description for the tombstone; formatkiller() keeps
   the English one that is written to the record and log files */
staticfn void
killer_i18n(char *buf, unsigned siz, int how)
{
    char eng[BUFSZ], *kname;
    const char *pfx = "", *tr;
    static const char *const prefixes[] = {
        "killed by ", "choked on ", "poisoned by ", "died of ",
        "drowned in ", "burned by ", "dissolved in ", "crushed to death by ",
        "petrified by ", "turned to slime by ", 0
    };
    int i;

    formatkiller(eng, sizeof eng, how, FALSE);
    kname = eng;
    for (i = 0; prefixes[i]; ++i)
        if (!strncmp(eng, prefixes[i], strlen(prefixes[i]))) {
            pfx = flags.female ? C_("feminine killed by", prefixes[i])
                               : C_("killed by", prefixes[i]);
            if (pfx == prefixes[i]) /* no feminine form */
                pfx = C_("killed by", prefixes[i]);
            kname = eng + strlen(prefixes[i]);
            break;
        }
    /* "a jackal" -> "un chacal": monster names with their article */
    if (!strncmp(kname, "a ", 2) && strcmp((tr = C_("monster", kname + 2)),
                                           kname + 2))
        tr = i18n_an_ctx("monster", kname + 2);
    else if (!strncmp(kname, "an ", 3)
             && strcmp((tr = C_("monster", kname + 3)), kname + 3))
        tr = i18n_an_ctx("monster", kname + 3);
    else if (strcmp((tr = C_("monster", kname)), kname))
        ; /* unique monster or plain name */
    else
        tr = _(kname);
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
