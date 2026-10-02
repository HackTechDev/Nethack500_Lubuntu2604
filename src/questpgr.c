/* NetHack 5.0	questpgr.c	$NHDT-Date: 1704043695 2023/12/31 17:28:15 $  $NHDT-Branch: keni-luabits2 $:$NHDT-Revision: 1.87 $ */
/*      Copyright 1991, M. Stephenson                             */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "dlb.h"

/*  quest-specific pager routines. */

#define QTEXT_FILE "quest.lua"

#ifdef TTY_GRAPHICS
#include "wintty.h"
#endif

staticfn const char *intermed(void);
/* sometimes find_qarti(gi.invent), and gi.invent can be null */
staticfn struct obj *find_qarti(struct obj *) NO_NNARGS;
staticfn const char *neminame(void);
staticfn const char *guardname(void);
staticfn const char *homebase(void);
staticfn void qtext_pronoun(char, char);
staticfn void convert_arg(char);
staticfn void quest_upper(char *);
staticfn int quest_gend(char, const char *);
staticfn boolean convert_arg_i18n(char, char);
staticfn char *quest_i18n(char *);
staticfn void convert_line(char *,char *);
staticfn void deliver_by_pline(const char *);
staticfn void deliver_by_window(const char *, int);
staticfn boolean skip_pager(boolean);
staticfn boolean com_pager_core(const char *, const char *, boolean, char **);

short
quest_info(int typ)
{
    switch (typ) {
    case 0:
        return gu.urole.questarti;
    case MS_LEADER:
        return gu.urole.ldrnum;
    case MS_NEMESIS:
        return gu.urole.neminum;
    case MS_GUARDIAN:
        return gu.urole.guardnum;
    default:
        impossible("quest_info(%d)", typ);
    }
    return 0;
}

/* return your role leader's name */
const char *
ldrname(void)
{
    int i = gu.urole.ldrnum;

    Sprintf(gn.nambuf, "%s%s", type_is_pname(&mons[i]) ? "" : "the ",
            mons[i].pmnames[NEUTRAL]);
    return gn.nambuf;
}

/* return your intermediate target string */
staticfn const char *
intermed(void)
{
    return gu.urole.intermed;
}

boolean
is_quest_artifact(struct obj *otmp)
{
    return (boolean) (otmp->oartifact == gu.urole.questarti);
}

staticfn struct obj *
find_qarti(struct obj *ochain)
{
    struct obj *otmp, *qarti;

    for (otmp = ochain; otmp; otmp = otmp->nobj) {
        if (is_quest_artifact(otmp))
            return otmp;
        if (Has_contents(otmp) && (qarti = find_qarti(otmp->cobj)) != 0)
            return qarti;
    }
    return (struct obj *) 0;
}

/* check several object chains for the quest artifact to determine
   whether it is present on the current level */
struct obj *
find_quest_artifact(unsigned whichchains)
{
    struct monst *mtmp;
    struct obj *qarti = 0;

    if ((whichchains & (1 << OBJ_INVENT)) != 0)
        qarti = find_qarti(gi.invent);
    if (!qarti && (whichchains & (1 << OBJ_FLOOR)) != 0)
        qarti = find_qarti(fobj);
    if (!qarti && (whichchains & (1 << OBJ_MINVENT)) != 0)
        for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
            if (DEADMONSTER(mtmp))
                continue;
            if ((qarti = find_qarti(mtmp->minvent)) != 0)
                break;
        }
    if (!qarti && (whichchains & (1 << OBJ_MIGRATING)) != 0) {
        /* check migrating objects and minvent of migrating monsters */
        for (mtmp = gm.migrating_mons; mtmp; mtmp = mtmp->nmon) {
            if (DEADMONSTER(mtmp))
                continue;
            if ((qarti = find_qarti(mtmp->minvent)) != 0)
                break;
        }
        if (!qarti)
            qarti = find_qarti(gm.migrating_objs);
    }
    if (!qarti && (whichchains & (1 << OBJ_BURIED)) != 0)
        qarti = find_qarti(svl.level.buriedobjlist);

    return qarti;
}

/* return your role nemesis' name */
staticfn const char *
neminame(void)
{
    int i = gu.urole.neminum;

    Sprintf(gn.nambuf, "%s%s", type_is_pname(&mons[i]) ? "" : "the ",
            mons[i].pmnames[NEUTRAL]);
    return gn.nambuf;
}

staticfn const char *
guardname(void) /* return your role leader's guard monster name */
{
    int i = gu.urole.guardnum;

    return mons[i].pmnames[NEUTRAL];
}

staticfn const char *
homebase(void) /* return your role leader's location */
{
    return gu.urole.homebase;
}

/* returns 1 if nemesis death message mentions noxious fumes, otherwise 0;
   does not display the message */
int
stinky_nemesis(struct monst *mon)
{
    char *mesg = 0;
    int res = 0;

#if 0
    /* get the quest text for dying nemesis; don't assume that mon is
       hero's own role's nemesis (overkill since m_detach() and nemdead()
       both make that assumption--valid for normal play but not necessarily
       valid for wizard mode) */
    int r, mndx = monsndx(mon->data);
    for (r = 0; roles[r].name.m || roles[r].name.f; ++r)
        if (roles[r].neminum == mndx) {
            (void) com_pager_core(roles[r].filecode, "killed_nemesis",
                                  FALSE, &mesg);
            break;
        }
#else
    nhUse(mon);
    /* since nemdead() just gave the message for hero's nemesis even if 'mon'
       is some other role's nemesis (feasible in wizard mode), base any gas
       cloud on the text that was shown even if not appropriate for 'mon' */
    (void) com_pager_core(gu.urole.filecode, "killed_nemesis", FALSE, &mesg);
#endif

    /* this is somewhat fragile; it assumes that when both {noxious or
       poisonous or toxic} and {gas or fumes} are present, the latter
       refers to the former rather than to something unrelated; it does
       make sure that fumes occurs after noxious rather than before */
    if (mesg) {
        char *p;

        /* change newlines into spaces to cope with "...noxious\nfumes..." */
        (void) strNsubst(mesg, "\n", " ", 0);

        if (((p = strstri(mesg, "noxious")) != 0
             || (p = strstri(mesg, "poisonous")) != 0
             || (p = strstri(mesg, "toxic")) != 0)
            && (strstri(p, " gas") || strstri(p, " fumes")))
            res = 1;

        free((genericptr_t) mesg);
    }
    return res;
}

/* replace deity, leader, nemesis, or artifact name with pronoun;
   overwrites cvt_buf[] */
staticfn void
qtext_pronoun(
    char who,   /* 'd' => deity, 'l' => leader, 'n' => nemesis, 'o' => arti */
    char which) /* 'h'|'H'|'i'|'I'|'j'|'J' */
{
    const char *pnoun;
    int godgend;
    char lwhich = lowc(which); /* H,I,J -> h,i,j */

    /*
     * Invalid subject (not d,l,n,o) yields neuter, singular result.
     *
     * For %o, treat all artifacts as neuter; some have plural names,
     * which genders[] doesn't handle; cvt_buf[] already contains name.
     */
    if (who == 'o'
        && (strstri(gc.cvt_buf, "Eyes ")
            || strcmpi(gc.cvt_buf, makesingular(gc.cvt_buf)))) {
        pnoun = (lwhich == 'h') ? "they"
                : (lwhich == 'i') ? "them"
                : (lwhich == 'j') ? "their" : "?";
    } else {
        godgend = (who == 'd') ? svq.quest_status.godgend
            : (who == 'l') ? svq.quest_status.ldrgend
            : (who == 'n') ? svq.quest_status.nemgend
            : 2; /* default to neuter */
        pnoun = (lwhich == 'h') ? genders[godgend].he
                : (lwhich == 'i') ? genders[godgend].him
                : (lwhich == 'j') ? genders[godgend].his : "?";
    }
    Strcpy(gc.cvt_buf, pnoun);
    /* capitalize for H,I,J */
    if (lwhich != which)
        gc.cvt_buf[0] = highc(gc.cvt_buf[0]);
    return;
}

staticfn void
convert_arg(char c)
{
    const char *str;

    switch (c) {
    case 'p':
        str = svp.plname;
        break;
    case 'c':
        str = (flags.female && gu.urole.name.f) ? gu.urole.name.f
                                               : gu.urole.name.m;
        break;
    case 'r':
        str = rank_of(u.ulevel, Role_switch, flags.female);
        break;
    case 'R':
        str = rank_of(MIN_QUEST_LEVEL, Role_switch, flags.female);
        break;
    case 's':
        str = (flags.female) ? "sister" : "brother";
        break;
    case 'S':
        str = (flags.female) ? "daughter" : "son";
        break;
    case 'l':
        str = ldrname();
        break;
    case 'i':
        str = intermed();
        break;
    case 'O':
    case 'o':
        str = the(artiname(gu.urole.questarti));
        if (c == 'O') {
            /* shorten "the Foo of Bar" to "the Foo"
               (buffer returned by the() is modifiable) */
            char *p = strstri(str, " of ");

            if (p)
                *p = '\0';
        }
        break;
    case 'n':
        str = neminame();
        break;
    case 'g':
        str = guardname();
        break;
    case 'G':
        str = align_gtitle(u.ualignbase[A_ORIGINAL]);
        break;
    case 'H':
        str = homebase();
        break;
    case 'a':
        str = align_str(u.ualignbase[A_ORIGINAL]);
        break;
    case 'A':
        str = align_str(u.ualign.type);
        break;
    case 'd':
        str = align_gname(u.ualignbase[A_ORIGINAL]);
        break;
    case 'D':
        str = align_gname(A_LAWFUL);
        break;
    case 'C':
        str = "chaotic";
        break;
    case 'N':
        str = "neutral";
        break;
    case 'L':
        str = "lawful";
        break;
    case 'x':
        str = Blind ? "sense" : "see";
        break;
    case 'Z':
        str = svd.dungeons[0].dname;
        break;
    case '%':
        str = "%";
        break;
    default:
        str = "";
        break;
    }
    Strcpy(gc.cvt_buf, str);
}

#if 0
/* for xgettext: values of the %-codes of the translated quest texts */
NC_("quest", "sister"), NC_("quest", "brother"),
NC_("quest", "daughter"), NC_("quest", "son"),
NC_("quest", "god"), NC_("quest", "goddess"),
NC_("quest", "see"), NC_("quest", "sense"),
NC_("quest", "the College of Archeology"),
NC_("quest", "the Tomb of the Toltec Kings"),
NC_("quest", "the Camp of the Duali Tribe"), NC_("quest", "the Duali Oasis"),
NC_("quest", "the Caves of the Ancestors"), NC_("quest", "the Dragon's Lair"),
NC_("quest", "the Temple of Epidaurus"), NC_("quest", "the Temple of Coeus"),
NC_("quest", "Camelot Castle"), NC_("quest", "the Isle of Glass"),
NC_("quest", "the Monastery of Chan-Sune"),
NC_("quest", "the Monastery of the Earth-Lord"),
NC_("quest", "the Great Temple"), NC_("quest", "the Temple of Nalzok"),
NC_("quest", "the Thieves' Guild Hall"),
NC_("quest", "the Assassins' Guild Hall"),
NC_("quest", "Orion's camp"), NC_("quest", "the cave of the wumpus"),
NC_("quest", "the Castle of the Taro Clan"),
NC_("quest", "the Shogun's Castle"), NC_("quest", "Ankh-Morpork"),
NC_("quest", "the Shrine of Destiny"), NC_("quest", "the cave of Surtur"),
NC_("quest", "the Lonely Tower"), NC_("quest", "the Tower of Darkness"),
NC_("quest", "the Orb of Detection"), NC_("quest", "the Orb"),
NC_("quest", "the Heart of Ahriman"), NC_("quest", "the Heart"),
NC_("quest", "the Sceptre of Might"), NC_("quest", "the Sceptre"),
NC_("quest", "the Staff of Aesculapius"), NC_("quest", "the Staff"),
NC_("quest", "the Magic Mirror of Merlin"), NC_("quest", "the Magic Mirror"),
NC_("quest", "the Eyes of the Overworld"), NC_("quest", "the Eyes"),
NC_("quest", "the Mitre of Holiness"), NC_("quest", "the Mitre"),
NC_("quest", "the Longbow of Diana"), NC_("quest", "the Longbow"),
NC_("quest", "the Master Key of Thievery"), NC_("quest", "the Master Key"),
NC_("quest", "the Tsurugi of Muramasa"), NC_("quest", "the Tsurugi"),
NC_("quest", "the Platinum Yendorian Express Card"),
NC_("quest", "the Platinum Yendorian Express Card"),
NC_("quest", "the Orb of Fate"),
NC_("quest", "the Eye of the Aethiopica"), NC_("quest", "the Eye"),
NC_("quest", "the Palantir of Westernesse"), NC_("quest", "the Palantir"),
#endif

/* capitalize the first letter of s, also an accented UTF-8 one */
staticfn void
quest_upper(char *s)
{
    unsigned char *us = (unsigned char *) s;

    if (us[0] == 0xC3 && us[1] >= 0xA0 && us[1] <= 0xBE && us[1] != 0xB7)
        us[1] -= 0x20;
    else
        s[0] = highc(s[0]);
}

/* the gender (as quest_status.ldrgend: 0 male, 1 female) of who, whose
   translated value is name; an artifact is feminine if its translation
   starts with "la " */
staticfn int
quest_gend(char who, const char *name)
{
    return (who == 'd') ? svq.quest_status.godgend
           : (who == 'l') ? svq.quest_status.ldrgend
           : (who == 'n') ? svq.quest_status.nemgend
           : (who == 'o' || who == 'O') ? (!strncmpi(name, "la ", 3) ? 1 : 0)
           : flags.female ? 1 : 0;
}

/* translation of the value of %-code c with modifier mod into
   gc.cvt_buf; returns TRUE if mod was used.  The values are translated,
   'a' gives the indefinite article, 'f' the feminine of an adjective,
   pronouns are il/elle (h), lui/elle (i, after a preposition) and son (j:
   the possessive agrees with the noun, write it), and 's' and 't' give the
   name without possessive or article (write "@de %l" for "of the
   leader": i18n_contract() turns "de le" into "du") */
staticfn boolean
convert_arg_i18n(char c, char mod)
{
    const char *en = 0, *str;
    char lmod = lowc(mod);
    int mndx = NON_PM, gend;
    boolean fem, used = TRUE;

    switch (c) {
    case 'c':
        fem = flags.female;
        str = (fem && gu.urole.name.f) ? _(gu.urole.name.f)
                                       : gendered_word(gu.urole.name.m, fem);
        break;
    case 'r':
    case 'R':
        str = gendered_word(rank_of((c == 'r') ? u.ulevel : MIN_QUEST_LEVEL,
                                    Role_switch, flags.female),
                            flags.female);
        break;
    case 's':
        str = C_("quest", flags.female ? "sister" : "brother");
        break;
    case 'S':
        str = C_("quest", flags.female ? "daughter" : "son");
        break;
    case 'l':
    case 'n':
    case 'g':
        mndx = (c == 'l') ? gu.urole.ldrnum
               : (c == 'n') ? gu.urole.neminum : gu.urole.guardnum;
        en = mons[mndx].pmnames[NEUTRAL];
        str = (c == 'g' || type_is_pname(&mons[mndx]))
                  ? C_("monster", en) : i18n_the_ctx("monster", en);
        break;
    case 'i':
        str = C_("quest", intermed());
        break;
    case 'H':
        str = C_("quest", homebase());
        break;
    case 'o':
    case 'O':
        Strcpy(gc.cvt_buf, the(artiname(gu.urole.questarti)));
        if (c == 'O') {
            char *p = strstri(gc.cvt_buf, " of ");

            if (p)
                *p = '\0';
        }
        str = C_("quest", gc.cvt_buf);
        break;
    case 'G':
        str = C_("quest", align_gtitle(u.ualignbase[A_ORIGINAL]));
        break;
    case 'a':
    case 'A':
    case 'C':
    case 'N':
    case 'L':
        en = (c == 'C') ? "chaotic" : (c == 'N') ? "neutral"
             : (c == 'L') ? "lawful"
             : align_str((c == 'a') ? u.ualignbase[A_ORIGINAL]
                                    : u.ualign.type);
        str = (mod == 'f') ? C_("feminine", en) : _(en);
        break;
    case 'x':
        str = C_("quest", Blind ? "sense" : "see");
        break;
    case 'Z':
        str = _(svd.dungeons[0].dname);
        break;
    default:
        convert_arg(c);
        str = 0;
        break;
    }
    if (str)
        Strcpy(gc.cvt_buf, str);

    switch (mod) {
    case 'a':
    case 'A':
        if (mndx != NON_PM && c != 'l' && c != 'n') {
            Strcpy(gc.cvt_buf, i18n_an_ctx("monster", en));
        } else {
            gend = quest_gend(c, gc.cvt_buf);
            Strcpy(gc.cvt_buf, gend == 1 ? C_("feminine", "a %s")
                                         : _("a %s"));
            (void) strNsubst(gc.cvt_buf, "%s", str ? str : "", 1);
        }
        if (mod == 'A')
            quest_upper(gc.cvt_buf);
        break;
    case 'C':
        quest_upper(gc.cvt_buf);
        break;
    case 'f':
        used = (strchr("aACNL", c) != 0);
        break;
    case 'h':
    case 'H':
    case 'i':
    case 'I':
    case 'j':
    case 'J':
        if (!strchr("dlno", c)) {
            used = FALSE;
            break;
        }
        gend = quest_gend(c, gc.cvt_buf);
        Strcpy(gc.cvt_buf, (lmod == 'h') ? (gend == 1 ? "elle" : "il")
                           : (lmod == 'i') ? (gend == 1 ? "elle" : "lui")
                           : "son");
        if (lmod != mod)
            quest_upper(gc.cvt_buf);
        break;
    case 'p':
    case 'P':
        if (mndx != NON_PM)
            Strcpy(gc.cvt_buf, i18n_mon_plural(en));
        else
            Strcpy(gc.cvt_buf, makeplural(gc.cvt_buf));
        if (mod == 'P')
            quest_upper(gc.cvt_buf);
        break;
    case 's':
    case 'S':
        if (mod == 'S')
            quest_upper(gc.cvt_buf);
        break;
    case 't':
        if (!strncmp(gc.cvt_buf, "le ", 3) || !strncmp(gc.cvt_buf, "la ", 3))
            (void) memmove(gc.cvt_buf, gc.cvt_buf + 3,
                           strlen(gc.cvt_buf + 3) + 1);
        else if (!strncmp(gc.cvt_buf, "les ", 4))
            (void) memmove(gc.cvt_buf, gc.cvt_buf + 4,
                           strlen(gc.cvt_buf + 4) + 1);
        else if (!strncmp(gc.cvt_buf, "l'", 2))
            (void) memmove(gc.cvt_buf, gc.cvt_buf + 2,
                           strlen(gc.cvt_buf + 2) + 1);
        break;
    default:
        used = FALSE;
        break;
    }
    return used;
}

/* the translation of quest text 'text' (msgctxt "quest"), its
   "{masculine|feminine}" alternatives chosen for the hero; frees 'text'
   if it returns a new string */
staticfn char *
quest_i18n(char *text)
{
    const char *tr, *p, *bar, *end;
    char *res, *q;

    if (!text || !i18n_active() || !(tr = i18n_text("quest", text)))
        return text;
    res = q = (char *) alloc(strlen(tr) + 1);
    for (p = tr; *p; ) {
        if (*p == '{' && (bar = strchr(p, '|')) != 0
            && (end = strchr(bar, '}')) != 0) {
            const char *from = flags.female ? bar + 1 : p + 1,
                       *to = flags.female ? end : bar;

            while (from < to)
                *q++ = *from++;
            p = end + 1;
        } else {
            *q++ = *p++;
        }
    }
    *q = '\0';
    free((genericptr_t) text);
    return res;
}

staticfn void
convert_line(char *in_line, char *out_line)
{
    char *c, *cc;

    cc = out_line;
    for (c = in_line; *c; c++) {
        *cc = 0;
        switch (*c) {
        case '\r':
        case '\n':
            *(++cc) = 0;
            goto convert_done;

        case '%':
            if (*(c + 1) && i18n_active()) {
                /* translated values for a translated text */
                ++c;
                if (convert_arg_i18n(*c, *(c + 1)))
                    ++c;
                Strcat(cc, gc.cvt_buf);
                cc += strlen(gc.cvt_buf);
                break;
            }
            if (*(c + 1)) {
                convert_arg(*(++c));
                switch (*(++c)) {
                /* insert "a"/"an" prefix */
                case 'A':
                    Strcat(cc, An(gc.cvt_buf));
                    cc += strlen(cc);
                    continue; /* for */
                case 'a':
                    Strcat(cc, an(gc.cvt_buf));
                    cc += strlen(cc);
                    continue; /* for */

                /* capitalize */
                case 'C':
                    gc.cvt_buf[0] = highc(gc.cvt_buf[0]);
                    break;

                /* replace name with pronoun;
                   valid for %d, %l, %n, and %o */
                case 'h': /* he/she */
                case 'H': /* He/She */
                case 'i': /* him/her */
                case 'I':
                case 'j': /* his/her */
                case 'J':
                    if (strchr("dlno", lowc(*(c - 1))))
                        qtext_pronoun(*(c - 1), *c);
                    else
                        --c; /* default action */
                    break;

                /* pluralize */
                case 'P':
                    gc.cvt_buf[0] = highc(gc.cvt_buf[0]);
                    FALLTHROUGH;
                    /*FALLTHRU*/
                case 'p':
                    Strcpy(gc.cvt_buf, makeplural(gc.cvt_buf));
                    break;

                /* append possessive suffix */
                case 'S':
                    gc.cvt_buf[0] = highc(gc.cvt_buf[0]);
                    FALLTHROUGH;
                    /*FALLTHRU*/
                case 's':
                    Strcpy(gc.cvt_buf, s_suffix(gc.cvt_buf));
                    break;

                /* strip any "the" prefix */
                case 't':
                    if (!strncmpi(gc.cvt_buf, "the ", 4)) {
                        Strcat(cc, &gc.cvt_buf[4]);
                        cc += strlen(cc);
                        continue; /* for */
                    }
                    break;

                default:
                    --c; /* undo switch increment */
                    break;
                }
                Strcat(cc, gc.cvt_buf);
                cc += strlen(gc.cvt_buf);
                break;
            }
            FALLTHROUGH;
            /* FALLTHRU */
        default:
            *cc++ = *c;
            break;
        }
        if (cc > &out_line[BUFSZ - 1])
            panic("convert_line: overflow");
    }
    *cc = 0;
 convert_done:
    if (i18n_active())
        i18n_contract(out_line);
    return;
}

staticfn void
deliver_by_pline(const char *str)
{
    char in_line[BUFSZ], out_line[BUFSZ];
    const char *msgp = str, *msgend = eos((char *) str);

    while (msgp < msgend) {
        /* copynchars() will stop at newline if it finds one */
        copynchars(in_line, msgp, (int) sizeof in_line - 1);
        msgp += strlen(in_line) + 1;

        /* translated before its %-codes are replaced */
        if (i18n_active()) {
            const char *tr = flags.female ? C_("heroine", in_line) : in_line;

            if (tr == in_line)
                tr = _(in_line);
            copynchars(in_line, tr, (int) sizeof in_line - 1);
        }
        convert_line(in_line, out_line);
        pline("%s", out_line);
    }
}

#if 0
/* for xgettext: des.message() texts of the special levels (dat/ Lua files) */
N_("What a strange feeling!")
N_("You notice that there is no gravity here.")
N_("You arrive on the Astral Plane!")
N_("Here the High Temple of %d is located.")
N_("You sense alarm, hostility, and excitement in the air!")
N_("Well done, mortal!")
N_("But now thou must face the final Test...")
N_("Prove thyself worthy or perish!")
N_("You find yourself suspended in an air bubble surrounded by water.")
NC_("heroine",
    "You find yourself suspended in an air bubble surrounded by water.")
#endif

staticfn void
deliver_by_window(const char *msg, int how)
{
    char in_line[BUFSZ], out_line[BUFSZ];
    const char *msgp = msg, *msgend = eos((char *) msg);
    winid datawin = create_nhwindow(how);

    while (msgp < msgend) {
        /* copynchars() will stop at newline if it finds one */
        copynchars(in_line, msgp, (int) sizeof in_line - 1);
        msgp += strlen(in_line) + 1;

        convert_line(in_line, out_line);
        putstr(datawin, 0, out_line);
    }

    display_nhwindow(datawin, TRUE);
    destroy_nhwindow(datawin);
}

staticfn boolean
skip_pager(boolean common UNUSED)
{
    /* WIZKIT: suppress plot feedback if starting with quest artifact */
    if (program_state.wizkit_wishing)
        return TRUE;
    return FALSE;
}

staticfn boolean
com_pager_core(
    const char *section,
    const char *msgid,
    boolean showerror,
    char **rawtext)
{
    static const char *const howtoput[] = {
        "pline", "window", "text", "menu", "default", NULL
    };
    static const int howtoput2i[] = { 1, 2, 2, 3, 0, 0 };
    int output;
    lua_State *L;
    char *text = NULL, *synopsis = NULL, *fallback_msgid = NULL;
    boolean res = FALSE;
    nhl_sandbox_info sbi = {NHL_SB_SAFE, 1*1024*1024, 0, 1*1024*1024};

    if (skip_pager(TRUE))
        return FALSE;

    L = nhl_init(&sbi);
    if (!L) {
        if (showerror)
            impossible("com_pager: nhl_init() failed");
        goto compagerdone;
    }

    if (!nhl_loadlua(L, QTEXT_FILE)) {
        if (showerror)
            impossible("com_pager: %s not found.", QTEXT_FILE);
        goto compagerdone;
    }

    lua_settop(L, 0);
    lua_getglobal(L, "questtext");
    if (!lua_istable(L, -1)) {
        if (showerror)
            impossible("com_pager: questtext in %s is not a lua table",
                       QTEXT_FILE);
        goto compagerdone;
    }

    lua_getfield(L, -1, section);
    if (!lua_istable(L, -1)) {
        if (showerror)
            impossible("com_pager: questtext[%s] in %s is not a lua table",
                       section, QTEXT_FILE);
        goto compagerdone;
    }

 tryagain:
    lua_getfield(L, -1, fallback_msgid ? fallback_msgid : msgid);
    if (!lua_istable(L, -1)) {
        if (!fallback_msgid) {
            /* Do we have questtxt[msg_fallbacks][<msgid>]? */
            lua_getfield(L, -3, "msg_fallbacks");
            if (lua_istable(L, -1)) {
                fallback_msgid = get_table_str_opt(L, msgid, NULL);
                lua_pop(L, 2);
                if (fallback_msgid)
                    goto tryagain;
            }
        }
        if (showerror) {
            if (!fallback_msgid)
                impossible(
                      "com_pager: questtext[%s][%s] in %s is not a lua table",
                           section, msgid, QTEXT_FILE);
            else
                impossible(
           "com_pager: questtext[%s][%s] and [][%s] in %s are not lua tables",
                           section, msgid, fallback_msgid, QTEXT_FILE);
        }
        goto compagerdone;
    }

    text = get_table_str_opt(L, "text", NULL);
    if (rawtext) {
        *rawtext = dupstr(text);
        res = TRUE;
        goto compagerdone;
    }
    synopsis = get_table_str_opt(L, "synopsis", NULL);
    output = howtoput2i[get_table_option(L, "output", "default", howtoput)];

    if (!text) {
        int nelems;

        lua_len(L, -1);
        nelems = (int) lua_tointeger(L, -1);
        lua_pop(L, 1);
        if (nelems < 2) {
            if (showerror)
                impossible(
              "com_pager: questtext[%s][%s] in %s is not an array of strings",
                           section, fallback_msgid ? fallback_msgid : msgid,
                           QTEXT_FILE);
            goto compagerdone;
        }
        nelems = rn2(nelems) + 1;
        lua_pushinteger(L, nelems);
        lua_gettable(L, -2);
        text = dupstr(luaL_checkstring(L, -1));
    }
    text = quest_i18n(text);
    synopsis = quest_i18n(synopsis);

    /* switch from by_pline to by_window if line has multiple segments or
       is unreasonably long (the latter ought to checked after formatting
       conversions rather than before...) */
    if (output == 0 && (strchr(text, '\n') || strlen(text) >= BUFSZ - 1)) {
        output = 2;

        /*
         * FIXME:  should update quest.lua to include proper synopsis line
         * for any item subject to having its delivery converted to by_window.
         */
        if (!synopsis) {
            char tmpbuf[BUFSZ];

            Sprintf(tmpbuf, "[%.*s]", BUFSZ - 1 - 2, text);
            /* change every newline character to a space */
            (void) strNsubst(tmpbuf, "\n", " ", 0);
            synopsis = dupstr(tmpbuf);
        }
    }

    if (output == 0 || output == 1)
        deliver_by_pline(text);
    else
        deliver_by_window(text, (output == 3) ? NHW_MENU : NHW_TEXT);

    if (synopsis) {
        char in_line[BUFSZ], out_line[BUFSZ];

#if 0   /* not yet -- brackets need to be removed from quest.lua */
        Sprintf(in_line, "[%.*s]",
                (int) (sizeof in_line - sizeof "[]"), synopsis);
#else
        Strcpy(in_line, synopsis);
#endif
        convert_line(in_line, out_line);
        /* bypass message delivery but be available for ^P recall */
        putmsghistory(out_line, FALSE);
    }
    res = TRUE;

 compagerdone:
    if (text)
        free((genericptr_t) text);
    if (synopsis)
        free((genericptr_t) synopsis);
    if (fallback_msgid)
        free((genericptr_t) fallback_msgid);
    nhl_done(L);
    return res;
}

void
com_pager(const char *msgid)
{
    (void) com_pager_core("common", msgid, TRUE, (char **) 0);
}

void
qt_pager(const char *msgid)
{
    if (!com_pager_core(gu.urole.filecode, msgid, FALSE, (char **) 0))
        (void) com_pager_core("common", msgid, TRUE, (char **) 0);
}

struct permonst *
qt_montype(void)
{
    int qpm;

    if (rn2(5)) {
        qpm = gu.urole.enemy1num;
        if (qpm != NON_PM && rn2(5) && !(svm.mvitals[qpm].mvflags & G_GENOD))
            return &mons[qpm];
        return mkclass(gu.urole.enemy1sym, 0);
    }
    qpm = gu.urole.enemy2num;
    if (qpm != NON_PM && rn2(5) && !(svm.mvitals[qpm].mvflags & G_GENOD))
        return &mons[qpm];
    return mkclass(gu.urole.enemy2sym, 0);
}

/* special levels can include a custom arrival message; display it */
void
deliver_splev_message(void)
{
    /* there's no provision for delivering via window instead of pline */
    if (gl.lev_message) {
        deliver_by_pline(gl.lev_message);

        free((genericptr_t) gl.lev_message);
        gl.lev_message = NULL;
    }
}

#undef QTEXT_FILE

/*questpgr.c*/
