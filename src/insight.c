/* NetHack 5.0	insight.c	$NHDT-Date: 1777004419 2026/04/23 20:20:19 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.134 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * Enlightenment and Conduct+Achievements and Vanquished+Extinct+Geno'd
 * and stethoscope/probing feedback.
 *
 * Most code used to reside in cmd.c, presumably because ^X was originally
 * a wizard mode command and the majority of those are in that file.
 * Some came from end.c where it is used during end of game disclosure.
 * And some came from priest.c that had once been in pline.c.
 */

#include "hack.h"

staticfn void enlght_out(const char *);
staticfn boolean enlght_i18n(char *, size_t);
staticfn const char *skill_name_i18n(const char *);
staticfn const char *pious_i18n(const char *);
staticfn void enlght_line(const char *, const char *, const char *,
                          const char *);
staticfn char *enlght_combatinc(const char *, int, int, char *);
staticfn void enlght_halfdmg(int, int);
staticfn boolean walking_on_water(void);
staticfn boolean cause_known(int);
staticfn char *attrval(int, int, char *);
staticfn char *fmt_elapsed_time(char *, int);
staticfn char *N_times(long, char *) NONNULL NONNULLARG2;
staticfn void background_enlightenment(int, int);
staticfn void basics_enlightenment(int, int);
staticfn void characteristics_enlightenment(int, int);
staticfn void one_characteristic(int, int, int);
staticfn void status_enlightenment(int, int);
staticfn void weapon_insight(int);
staticfn void attributes_enlightenment(int, int);
staticfn void show_achievements(int);
staticfn int QSORTCALLBACK vanqsort_cmp(const genericptr, const genericptr);
staticfn int num_extinct(void);
staticfn int num_gone(int, int *);
staticfn char *size_str(int);
staticfn void item_resistance_message(int, const char *, int);

extern const char *const hu_stat[];  /* hunger status from eat.c */
extern const char *const enc_stat[]; /* encumbrance status from botl.c */

static const char You_[] = N_("You "), are[] = N_("are "),
                  were[] = N_("were "), have[] = N_("have "),
                  had[] = N_("had "), can[] = N_("can "),
                  could[] = N_("could ");
static const char have_been[] = N_("have been "),
                  have_never[] = N_("have never "), never[] = N_("never ");

/* for livelogging: */
struct ll_achieve_msg {
    long llflag;
    const char *msg;
};
/* ordered per 'enum achievements' in you.h */
/* take care to keep them in sync! */
static struct ll_achieve_msg achieve_msg [] = {
    { 0, "" }, /* actual achievements are numbered from 1 */
    { LL_ACHIEVE, "acquired the Bell of Opening" },
    { LL_ACHIEVE, "entered Gehennom" },
    { LL_ACHIEVE, "acquired the Candelabrum of Invocation" },
    { LL_ACHIEVE, "acquired the Book of the Dead" },
    { LL_ACHIEVE, "performed the invocation" },
    { LL_ACHIEVE, "acquired The Amulet of Yendor" },
    { LL_ACHIEVE, "entered the Elemental Planes" },
    { LL_ACHIEVE, "entered the Astral Plane" },
    { LL_ACHIEVE, "ascended" },
    /* if the type of item isn't discovered yet, disclosing the event
       via #chronicle would be a spoiler (particularly for gray stone);
       the ID'd name for the type of item will be appended to the next
       two messages, for display via livelog and/or dumplog */
    { LL_ACHIEVE | LL_SPOILER, "acquired the Mines' End" }, /* " luckstone" */
    { LL_ACHIEVE | LL_SPOILER, "acquired the Sokoban" }, /* " <item>" */
    { LL_ACHIEVE | LL_UMONST, "killed Medusa" },
     /* these two are not logged */
    { 0, "hero was always blond, no, blind" },
    { 0, "hero never wore armor" },
     /* */
    { LL_MINORAC | LL_DUMP, "entered the Gnomish Mines" },
    { LL_ACHIEVE, "reached Mine Town" }, /* probably minor, but dnh logs it */
    { LL_MINORAC, "entered a shop" },
    { LL_MINORAC, "entered a temple" },
    { LL_ACHIEVE, "consulted the Oracle" }, /* minor, but rare enough */
    { LL_MINORAC | LL_DUMP, "read a Discworld novel" }, /* even more so */
    { LL_ACHIEVE, "entered Sokoban" }, /* keep as major for turn comparison
                                        * with completed sokoban */
    { LL_ACHIEVE, "entered the Bigroom" },
    /* The following 8 are for advancing through the ranks
       and messages differ by role so are created on the fly;
       rank 0 (Xp 1 and 2) isn't an achievement */
    { LL_MINORAC | LL_DUMP, "" }, /* Xp 3 */
    { LL_MINORAC | LL_DUMP, "" }, /* Xp 6 */
    { LL_MINORAC | LL_DUMP, "" }, /* Xp 10 */
    { LL_ACHIEVE, "" }, /* Xp 14, so able to attempt the quest */
    { LL_ACHIEVE, "" }, /* Xp 18 */
    { LL_ACHIEVE, "" }, /* Xp 22 */
    { LL_ACHIEVE, "" }, /* Xp 26 */
    { LL_ACHIEVE, "" }, /* Xp 30 */
    { LL_MINORAC, "learned castle drawbridge's tune" }, /* achievement #31 */
    { 0, "" } /* keep this one at the end */
};

/* macros to simplify output of enlightenment messages; also used by
   conduct and achievements */
#define enl_msg(prefix, present, past, suffix, ps) \
    enlght_line((prefix), final ? (past) : (present), (suffix), (ps))
#define you_are(attr, ps) enl_msg(You_, are, were, (attr), (ps))
#define you_have(attr, ps) enl_msg(You_, have, had, (attr), (ps))
#define you_can(attr, ps) enl_msg(You_, can, could, (attr), (ps))
#define you_have_been(goodthing) \
    enl_msg(You_, have_been, were, (goodthing), "")
#define you_have_never(badthing) \
    enl_msg(You_, have_never, never, (badthing), "")
#define you_have_X(something) \
    enl_msg(You_, have, (const char *) "", (something), "")

staticfn void
enlght_out(const char *buf)
{
    if (ge.en_via_menu) {
        add_menu_str(ge.en_win, buf);
    } else
        putstr(ge.en_win, 0, buf);
}

staticfn void
enlght_line(
    const char *start,
    const char *middle,
    const char *end,
    const char *ps)
{
#ifndef NO_ENLGHT_CONTRACTIONS
    static const struct contrctn {
        const char *twowords, *contrctn;
    } contra[] = {
        { " are not ", " aren't " },
        { " were not ", " weren't " },
        { " have not ", " haven't " },
        { " had not ", " hadn't " },
        { " can not ", " can't " },
        { " could not ", " couldn't " },
    };
    int i;
#endif
    char buf[BUFSZ];

    Sprintf(buf, " %s%s%s%s.", start, middle, end, ps);
    if (i18n_active()) {
        /* a whole sentence in the catalog, else one made of the
           translated fragments ("Vous " "êtes " "rapide") */
        if (!enlght_i18n(buf, sizeof buf))
            Snprintf(buf, sizeof buf, " %s%s%s%s.", _(start), _(middle),
                     gendered_word(end, flags.female ? 1 : 0), _(ps));
        enlght_out(buf);
        return;
    }
#ifndef NO_ENLGHT_CONTRACTIONS
    if (strstri(buf, " not ")) { /* TODO: switch to libc strstr() */
        for (i = 0; i < SIZE(contra); ++i)
            (void) strsubst(buf, contra[i].twowords, contra[i].contrctn);
    }
#endif
    enlght_out(buf);
}

/* translate the composed enlightenment/conduct line buf (" You ...."),
   looked up as a whole sentence; a number in it is looked up as "%d" */
staticfn boolean
enlght_i18n(char *buf, size_t bufsz)
{
#ifdef NHI18N
    char key[BUFSZ], *p, *q;
    const char *tr = 0;
    long num = 0L;

    if (flags.female)
        tr = i18n_lookup("heroine", buf + 1);
    if (!tr && i18n_has(buf + 1))
        tr = _(buf + 1);
    if (tr) {
        char tmp[BUFSZ];

        Snprintf(tmp, sizeof tmp, " %s", tr);
        copynchars(buf, tmp, (int) bufsz - 1);
        return TRUE;
    }
    /* "You have genocided 3 types of monsters." */
    for (p = buf + 1; *p && !digit(*p); ++p)
        continue;
    if (!*p)
        return FALSE;
    for (q = p; digit(*q); ++q)
        num = num * 10L + (*q - '0');
    if (p > buf + 1 && p[-1] == '-') /* "<-860>" */
        --p, num = -num;
    copynchars(key, buf + 1, min((int) (p - (buf + 1)), BUFSZ - 1));
    Strcat(key, "%ld");
    if (strlen(key) + strlen(q) >= sizeof key)
        return FALSE;
    Strcat(key, q);
    if (flags.female)
        tr = i18n_lookup("heroine", key);
    if (!tr && i18n_has(key))
        tr = _(key);
    if (tr) {
        char tmp[BUFSZ];

        DISABLE_WARNING_FORMAT_NONLITERAL;
        Snprintf(tmp, sizeof tmp, tr, num);
        RESTORE_WARNING_FORMAT_NONLITERAL;
        Snprintf(buf, bufsz, " %s", tmp);
        return TRUE;
    }
#else
    nhUse(buf);
    nhUse(bufsz);
#endif
    return FALSE;
}

#if 0
/* fragments of enlightenment lines, translated by enlght_line() */
N_("take") N_("took") N_("haven't encountered") N_("didn't encounter")
N_(" any bones levels") N_("Total elapsed playing time ") N_("is") N_("was")
N_("just started your adventure") N_("entered ") N_("It ") N_("is ")
N_("was ") N_("the midnight hour") N_("nighttime") N_("There ")
N_("Your score ") N_("Your armor class ") N_("you ") N_("have ") N_("had ")
N_("Autopickup ") N_("levitating, at will") N_("levitating") N_("flying")
N_("underwater") N_("swimming") N_("in water") N_("turning to stone")
N_("turning into slime") N_("buried") N_("terminally sick from illness")
N_("terminally sick from food poisoning") N_("nauseated") N_("stunned")
N_("confused") N_("hallucinating") N_("deaf") N_("are ") N_("were ")
N_(" has ") N_(" had ") N_("fumble") N_("fumbled") N_("You ") N_("fall")
N_("fell") N_(" asleep uncontrollably") N_("hunger") N_("hungered")
N_(" rapidly") N_("do not wear") N_("did not wear") N_(" any armor")
N_("not wearing any armor") N_("wielding two weapons at once")
N_("can enhance") N_("could have enhanced") N_("Your items ")
N_("are somewhat") N_("are") N_("were somewhat") N_("were")
N_("Your alignment ") N_("invulnerable") N_("magic-protected")
N_("fire resistant") N_("cold resistant") N_("sleep resistant")
N_("disintegration resistant") N_("shock resistant") N_("poison resistant")
N_("level-drain resistant") N_("immune to sickness") N_("resist")
N_("resisted") N_(" hallucinations") N_("recognize detrimental food")
N_("see") N_("not subject to light-induced blindness") N_("saw")
N_(" invisible") N_("will see") N_("would have seen")
N_(" invisible when not blind") N_("would see") N_(" invisible if not blind")
N_("telepathic") N_("warned") N_("warned of undead") N_("automatic searching")
N_("clairvoyant") N_("could be") N_("could have been") N_(" clairvoyant")
N_("infravision") N_("will confuse") N_("would have confused") N_("invisible")
N_("invisible to others") N_("visible") N_("displaced") N_("stealthy")
N_("would be") N_("would have been") N_(" monsters")
N_("aggravate") N_("aggravated") N_("cause") N_("caused") N_(" conflict")
N_("regenerate") N_("regenerated") N_("jump") N_("teleport")
N_("teleport control") N_("would levitate") N_("would have levitated")
N_("would fly") N_("would have flown") N_("hadn't been") N_("weren't")
N_("had permitted") N_("permitted") N_(" if you weren't levitating")
N_(" if you weren't trapped") N_(" if circumstances permitted")
N_("cling to the ceiling") N_("could cling") N_("could have clung")
N_("walk on water") N_("swim") N_("survive without air") N_("breathe water")
N_("walk through walls") N_("slower digestion")
N_("damage") N_("defense") N_(" reduced poison gas damage")
N_("Your spell casting ") N_("protected from shape changers")
N_("not change from your current form")
N_(" if not locked into your current form") N_("polymorphing periodically")
N_("polymorph control") N_("lay eggs") N_("harmed by silver") N_("very fast")
N_("fast") N_("reflection") N_("free action") N_("fixed abilities")
N_("Your life ") N_("will be") N_(" saved") N_("Your luck ") N_(" zero")
N_("extra luck") N_("reduced luck") N_("Bad luck ") N_("does not time out")
N_("did not time out") N_(" for you") N_("Good luck ") N_(" is") N_(" was")
N_("The current fruit ") N_("The made fruit flag ") N_("have been killed ")
N_("blind from birth") N_("deaf from birth") N_("started") N_("started out")
N_(" without possessions") N_("faithfully nudist") N_("have gone") N_("went")
N_(" without food") N_("followed a strict vegan diet") N_("vegetarian")
N_("an atheist") N_("hit with a wielded weapon") N_("a pacifist")
N_("illiterate") N_("had a pet") N_("genocided any monsters")
N_("polymorphed an object") N_("changed form") N_("used no wishes")
N_("have not wished") N_("did not wish") N_(" for any artifacts")
N_("are exploring") N_("explored") N_(" without being able to see")
N_(" without any armor") N_("entered the Gnomish Mines")
N_("entered Minetown") N_("entered a shop") N_("entered a temple")
N_("consulted the Oracle of Delphi") N_("read from a Discworld novel")
N_("entered Sokoban") N_("completed Sokoban")
N_("completed the Gnomish Mines") N_("entered the Big Room")
N_("defeated Medusa")
N_("learned the tune to open and close the Castle's drawbridge") N_("have")
N_("have handled") N_("had") N_("handled") N_(" the Bell of Opening")
N_("entered Gehennom") N_(" the Candelabrum of Invocation")
N_(" the Book of the Dead") N_("gained access to Moloch's Sanctum")
N_("have obtained") N_("delivered") N_("had obtained")
N_(" the Amulet of Yendor") N_("reached the Elemental Planes")
N_("reached the Astral Plane")
#endif

#if 0
/* feminine forms given by gendered_word() in enlght_line() */
C_("feminine", "invulnerable")
C_("feminine", "magic-protected")
C_("feminine", "cold resistant")
C_("feminine", "fire resistant")
C_("feminine", "sleep resistant")
C_("feminine", "disintegration resistant")
C_("feminine", "poison resistant")
C_("feminine", "shock resistant")
C_("feminine", "immune to sickness")
C_("feminine", "level-drain resistant")
C_("feminine", "telepathic")
C_("feminine", "warned")
C_("feminine", "warned of undead")
C_("feminine", "clairvoyant")
C_("feminine", " clairvoyant")
C_("feminine", "displaced")
C_("feminine", "stealthy")
C_("feminine", "buried")
C_("feminine", "nauseated")
C_("feminine", "deaf")
C_("feminine", "very fast")
C_("feminine", "protected from shape changers")
C_("feminine", "terminally sick from illness")
C_("feminine", "terminally sick from food poisoning")
C_("feminine", "blind from birth")
C_("feminine", "deaf from birth")
C_("feminine", "vegetarian")
C_("feminine", "illiterate")
C_("feminine", "not subject to light-induced blindness")
C_("feminine", "harmed by silver")
C_("feminine", "invisible to others")
C_("feminine", "visible")
#endif

#if 0
/* whole status lines of enlightenment, looked up by enlght_i18n() */
N_("You are not hungry.")
N_("You are not hungry <%ld>.")
N_("You are hungry.")
N_("You are hungry <%ld>.")
N_("You are satiated.")
N_("You are satiated <%ld>.")
N_("You are weak from severe hunger.")
N_("You are weak from severe hunger <%ld>.")
N_("You are fainting due to starvation.")
N_("You are fainting due to starvation <%ld>.")
N_("You are fainted due to starvation.")
N_("You are fainted due to starvation <%ld>.")
N_("You are burdened; movement is slightly slowed.")
N_("You are burdened <%ld>; movement is slightly slowed.")
N_("You are stressed; movement is moderately slowed.")
N_("You are stressed <%ld>; movement is moderately slowed.")
N_("You are strained; movement is very slowed.")
N_("You are strained <%ld>; movement is very slowed.")
N_("You are overtaxed; movement is extremely slowed.")
N_("You are overtaxed <%ld>; movement is extremely slowed.")
N_("You are overloaded; movement is not possible.")
N_("You are unencumbered.")
N_("You are overloaded <%ld>; movement is not possible.")
N_("You are unencumbered <%ld>.")
N_("You were not hungry.")
N_("You were not hungry <%ld>.")
N_("You were hungry.")
N_("You were hungry <%ld>.")
N_("You were satiated.")
N_("You were satiated <%ld>.")
N_("You were weak from severe hunger.")
N_("You were weak from severe hunger <%ld>.")
N_("You were fainting due to starvation.")
N_("You were fainting due to starvation <%ld>.")
N_("You were fainted due to starvation.")
N_("You were fainted due to starvation <%ld>.")
N_("You were burdened; movement was slightly slowed.")
N_("You were burdened <%ld>; movement was slightly slowed.")
N_("You were stressed; movement was moderately slowed.")
N_("You were stressed <%ld>; movement was moderately slowed.")
N_("You were strained; movement was very slowed.")
N_("You were strained <%ld>; movement was very slowed.")
N_("You were overtaxed; movement was extremely slowed.")
N_("You were overtaxed <%ld>; movement was extremely slowed.")
N_("You were overloaded; movement was not possible.")
N_("You were unencumbered.")
N_("You were overloaded <%ld>; movement was not possible.")
N_("You were unencumbered <%ld>.")
NC_("heroine", "You are not hungry.")
NC_("heroine", "You are not hungry <%ld>.")
NC_("heroine", "You are hungry.")
NC_("heroine", "You are hungry <%ld>.")
NC_("heroine", "You are satiated.")
NC_("heroine", "You are satiated <%ld>.")
NC_("heroine", "You are fainted due to starvation.")
NC_("heroine", "You are fainted due to starvation <%ld>.")
NC_("heroine", "You are burdened; movement is slightly slowed.")
NC_("heroine", "You are burdened <%ld>; movement is slightly slowed.")
NC_("heroine", "You are stressed; movement is moderately slowed.")
NC_("heroine", "You are stressed <%ld>; movement is moderately slowed.")
NC_("heroine", "You are strained; movement is very slowed.")
NC_("heroine", "You are strained <%ld>; movement is very slowed.")
NC_("heroine", "You are overtaxed; movement is extremely slowed.")
NC_("heroine", "You are overtaxed <%ld>; movement is extremely slowed.")
NC_("heroine", "You are overloaded; movement is not possible.")
NC_("heroine", "You are overloaded <%ld>; movement is not possible.")
NC_("heroine", "You were not hungry.")
NC_("heroine", "You were not hungry <%ld>.")
NC_("heroine", "You were hungry.")
NC_("heroine", "You were hungry <%ld>.")
NC_("heroine", "You were satiated.")
NC_("heroine", "You were satiated <%ld>.")
NC_("heroine", "You were fainted due to starvation.")
NC_("heroine", "You were fainted due to starvation <%ld>.")
NC_("heroine", "You were burdened; movement was slightly slowed.")
NC_("heroine", "You were burdened <%ld>; movement was slightly slowed.")
NC_("heroine", "You were stressed; movement was moderately slowed.")
NC_("heroine", "You were stressed <%ld>; movement was moderately slowed.")
NC_("heroine", "You were strained; movement was very slowed.")
NC_("heroine", "You were strained <%ld>; movement was very slowed.")
NC_("heroine", "You were overtaxed; movement was extremely slowed.")
NC_("heroine", "You were overtaxed <%ld>; movement was extremely slowed.")
NC_("heroine", "You were overloaded; movement was not possible.")
NC_("heroine", "You were overloaded <%ld>; movement was not possible.")
#endif

#if 0
/* ^X lines given by gendered_word() */
N_(" You are normally right-handed.") N_(" You are normally left-handed.")
N_(" You are right-handed.") N_(" You are left-handed.")
N_(" You entered the dungeon %ld turns ago.")
C_("feminine", " You are normally right-handed.")
C_("feminine", " You are normally left-handed.")
C_("feminine", " You are right-handed.")
C_("feminine", " You are left-handed.")
C_("feminine", " You entered the dungeon %ld turns ago.")
#endif

#if 0
/* skill levels shown in enlightenment (lowercase skill_level_name()) */
NC_("skill level", "unskilled") NC_("skill level", "basic")
NC_("skill level", "skilled") NC_("skill level", "expert")
NC_("skill level", "master") NC_("skill level", "grand master")
N_(" You have strayed.") N_(" You had strayed.")
C_("feminine", " You have strayed.") C_("feminine", " You had strayed.")
N_("aligned") C_("feminine", "aligned")
N_("empty handed") N_("bare handed") N_("not wielding anything")
#endif

/* translated name of skill (weapon.c's P_NAME()) */
staticfn const char *
skill_name_i18n(const char *name)
{
#ifdef NHI18N
    const char *res = i18n_lookup("skill", name);

    if (res)
        return res;
#endif
    return nh_npgettext("object", name, FALSE);
}

/* format increased chance to hit or damage or defense (Protection) */
staticfn char *
enlght_combatinc(
    const char *inctyp, /* "to hit" or "damage" or "defense" */
    int incamt,         /* amount of increment (negative if decrement) */
    int final,          /* ENL_{GAMEINPROGRESS,GAMEOVERALIVE,GAMEOVERDEAD} */
    char *outbuf)
{
    const char *modif, *bonus;
    boolean invrt;
    int absamt;

    absamt = abs(incamt);
    /* Protection amount is typically larger than damage or to-hit;
       reduce magnitude by a third in order to stretch modifier ranges
       (small:1..5, moderate:6..10, large:11..19, huge:20+) */
    if (!strcmp(inctyp, "defense"))
        absamt = (absamt * 2) / 3;

    if (absamt <= 3)
        modif = "small";
    else if (absamt <= 6)
        modif = "moderate";
    else if (absamt <= 12)
        modif = "large";
    else
        modif = "huge";

    modif = !incamt ? "no" : an(modif); /* ("no" case shouldn't happen) */
    bonus = (incamt >= 0) ? "bonus" : "penalty";
    /* "bonus <foo>" (to hit) vs "<bar> bonus" (damage, defense) */
    invrt = strcmp(inctyp, "to hit") ? TRUE : FALSE;

    Sprintf(outbuf, "%s %s %s", modif, invrt ? inctyp : bonus,
            invrt ? bonus : inctyp);
    if (final || wizard)
        Sprintf(eos(outbuf), " (%s%d)", (incamt > 0) ? "+" : "", incamt);

    return outbuf;
}

/* report half physical or half spell damage */
staticfn void
enlght_halfdmg(int category, int final)
{
    const char *category_name;
    char buf[BUFSZ];

    switch (category) {
    case HALF_PHDAM:
        category_name = "physical";
        break;
    case HALF_SPDAM:
        category_name = "spell";
        break;
    default:
        category_name = "unknown";
        break;
    }
    Sprintf(buf, " %s %s damage", (final || wizard) ? "half" : "reduced",
            category_name);
    enl_msg(You_, "take", "took", buf, from_what(category));
}

/* is hero actively using water walking capability on water (or lava)? */
staticfn boolean
walking_on_water(void)
{
    if (u.uinwater || Levitation || Flying)
        return FALSE;
    return (boolean) (Wwalking && is_pool_or_lava(u.ux, u.uy));
}

/* describe u.utraptype; used by status_enlightenment() and self_lookat() */
char *
trap_predicament(char *outbuf, int final, boolean wizxtra)
{
    struct trap *t;

    /* caller has verified u.utrap */
    *outbuf = '\0';
    switch (u.utraptype) {
    case TT_BURIEDBALL:
        Strcpy(outbuf, "tethered to something buried");
        break;
    case TT_LAVA:
        Sprintf(outbuf, "sinking into %s", final ? "lava" : hliquid("lava"));
        break;
    case TT_INFLOOR:
        Sprintf(outbuf, "stuck in %s", the(surface(u.ux, u.uy)));
        break;
    default: /* TT_BEARTRAP, TT_PIT, or TT_WEB */
        Strcpy(outbuf, "trapped");
        if ((t = t_at(u.ux, u.uy)) != 0) /* should never be null */
            Sprintf(eos(outbuf), " in %s", an(trapname(t->ttyp, FALSE)));
        break;
    }
    if (wizxtra) { /* give extra information for wizard mode enlightenment */
        /* curly braces: u.utrap is an escape attempt counter rather than a
           turn timer so use different ornamentation than usual parentheses */
        Sprintf(eos(outbuf), " {%u}", u.utrap);
    }
    return outbuf;
}

/* check whether hero is wearing something that player definitely knows
   confers the target property; item must have been seen and its type
   discovered but it doesn't necessarily have to be fully identified */
staticfn boolean
cause_known(
    int propindx) /* index of a property which can be conveyed by worn item */
{
    struct obj *o;
    long mask = W_ARMOR | W_AMUL | W_RING | W_TOOL;

    /* simpler than from_what()/what_gives(); we don't attempt to
       handle artifacts and we deliberately ignore wielded items */
    for (o = gi.invent; o; o = o->nobj) {
        if (!(o->owornmask & mask))
            continue;
        if ((int) objects[o->otyp].oc_oprop == propindx
            && objects[o->otyp].oc_name_known && o->dknown)
            return TRUE;
    }
    return FALSE;
}

/* format a characteristic value, accommodating Strength's strangeness */
staticfn char *
attrval(
    int attrindx,
    int attrvalue,
    char resultbuf[]) /* should be at least [7] to hold "18/100\0" */
{
    if (attrindx != A_STR || attrvalue <= 18)
        Sprintf(resultbuf, "%d", attrvalue);
    else if (attrvalue > STR18(100)) /* 19 to 25 */
        Sprintf(resultbuf, "%d", attrvalue - 100);
    else /* simplify "18/\**" to be "18/100" */
        Sprintf(resultbuf, "18/%02d", attrvalue - 18);
    return resultbuf;
}

/* format urealtime.realtime as
      " D days, H hours, M minutes and S seconds"
   with any fields having a value of 0 omitted:
      0-00:00:20 => " 20 seconds"
      0-00:15:05 => " 15 minutes and 5 seconds"
      0-00:16:00 => " 16 minutes"
      0-01:15:10 => " 1 hour, 15 minutes and 10 seconds"
      0-02:00:01 => " 2 hours and 1 second"
      3-00:25:40 => " 3 days, 25 minutes and 40 seconds"
   (note: for a list of more than two entries, nethack usually includes the
   [style-wise] optional comma before "and" but in this instance it does not)
 */
staticfn char *
fmt_elapsed_time(char *outbuf, int final)
{
    int fieldcnt;
    long edays, ehours, eminutes, eseconds;
    /* for a game that's over, reallydone() has updated urealtime.realtime
       to its final value before calling us during end of game disclosure;
       for a game that's still in progress, it holds the amount of elapsed
       game time from previous sessions up through most recent save/restore
       (or up through latest level change when 'checkpoint' is On);
       '.start_timing' has a non-zero value even if '.realtime' is 0 */
    long etim = urealtime.realtime;

    if (!final)
        etim += timet_delta(getnow(), urealtime.start_timing);
    /* we could use localtime() to convert the value into a 'struct tm'
       to get date and time fields but this is simple and straightforward */
    eseconds = etim % 60L, etim /= 60L;
    eminutes = etim % 60L, etim /= 60L;
    ehours = etim % 24L;
    edays = etim / 24L;
    fieldcnt = !!edays + !!ehours + !!eminutes + !!eseconds;

    Strcpy(outbuf, fieldcnt ? "" : _(" none")); /* 'none' never happens */
    if (edays) {
        Sprintf(eos(outbuf), (edays == 1L) ? _(" %ld day") : _(" %ld days"),
                edays);
        if (fieldcnt > 1) /* hours and/or minutes and/or seconds to follow */
            Strcat(outbuf, (fieldcnt == 2) ? _(" and") : ",");
        --fieldcnt; /* edays has been processed */
    }
    if (ehours) {
        Sprintf(eos(outbuf), (ehours == 1L) ? _(" %ld hour")
                                            : _(" %ld hours"), ehours);
        if (fieldcnt > 1) /* minutes and/or seconds to follow */
            Strcat(outbuf, (fieldcnt == 2) ? _(" and") : ",");
        --fieldcnt; /* ehours has been processed */
    }
    if (eminutes) {
        Sprintf(eos(outbuf), (eminutes == 1L) ? _(" %ld minute")
                                              : _(" %ld minutes"), eminutes);
        if (fieldcnt > 1) /* seconds to follow */
            Strcat(outbuf, _(" and"));
        /* eminutes has been processed but no need to decrement fieldcnt */
    }
    if (eseconds)
        Sprintf(eos(outbuf), (eseconds == 1L) ? _(" %ld second")
                                              : _(" %ld seconds"), eseconds);
    return outbuf;
}

/* "once" vs "twice" vs "17 times", used in several places */
staticfn char *
N_times(long n, char *outbuf)
{
    switch (n) {
    case 0:
    default:
        Sprintf(outbuf, "%ld times", n);
        break;
    case 1:
        Strcpy(outbuf, "once");
        break;
    case 2:
        Strcpy(outbuf, "twice");
        break;
    case 3:
        Strcpy(outbuf, "thrice");
        break;
    }
    return outbuf;
}

void
enlightenment(
    int mode,  /* BASICENLIGHTENMENT | MAGICENLIGHTENMENT (| both) */
    int final) /* ENL_GAMEINPROGRESS:0, ENL_GAMEOVERALIVE, ENL_GAMEOVERDEAD */
{
    char buf[BUFSZ], tmpbuf[BUFSZ];

    ge.en_win = create_nhwindow(NHW_MENU);
    ge.en_via_menu = !final;
    if (ge.en_via_menu)
        start_menu(ge.en_win, MENU_BEHAVE_STANDARD);

    Strcpy(tmpbuf, svp.plname);
    *tmpbuf = highc(*tmpbuf); /* same adjustment as bottom line */
    /* as in background_enlightenment, when poly'd we need to use the saved
       gender in u.mfemale rather than the current you-as-monster gender */
    Snprintf(buf, sizeof(buf), _("%s the %s's attributes:"), tmpbuf,
             _(((Upolyd ? u.mfemale : flags.female) && gu.urole.name.f)
                ? gu.urole.name.f
                : gu.urole.name.m));

    /* title */
    enlght_out(buf); /* "Conan the Archeologist's attributes:" */
    /* background and characteristics; ^X or end-of-game disclosure */
    if (mode & BASICENLIGHTENMENT) {
        /* role, race, alignment, deities, dungeon level, time, experience */
        background_enlightenment(mode, final);
        /* hit points, energy points, armor class, gold */
        basics_enlightenment(mode, final);
        /* strength, dexterity, &c */
        characteristics_enlightenment(mode, final);
    }
    /* expanded status line information, including things which aren't
       included there due to space considerations;
       shown for both basic and magic enlightenment */
    status_enlightenment(mode, final);
    /* remaining attributes; shown for potion,&c or wizard mode and
       explore mode ^X or end of game disclosure */
    if (mode & MAGICENLIGHTENMENT) {
        /* intrinsics and other traditional enlightenment feedback */
        attributes_enlightenment(mode, final);
    }

    enlght_out(""); /* separator */
    enlght_out(_("Miscellaneous:"));
    /* reminder to player and/or information for dumplog */
    if ((mode & BASICENLIGHTENMENT) != 0 && (wizard || discover || final)) {
        if (wizard || discover) {
            if (i18n_active()) {
                enlght_out(wizard
                           ? (!final ? _(" You are running in debug mode.")
                                     : _(" You were running in debug mode."))
                           : (!final
                              ? _(" You are running in explore mode.")
                              : _(" You were running in explore mode.")));
            } else {
                Sprintf(buf, "running in %s mode",
                        wizard ? "debug" : "explore");
                you_are(buf, "");
            }
        }

        if (!flags.bones) {
            /* mention not saving bones iff hero just died */
            Sprintf(buf, "disabled loading%s of bones levels",
                    (final == ENL_GAMEOVERDEAD) ? " and storing" : "");
            you_have_X(buf);
        } else if (!u.uroleplay.numbones) {
            enl_msg(You_, "haven't encountered", "didn't encounter",
                    " any bones levels", "");
        } else {
            Sprintf(buf, "encountered %ld bones level%s",
                    u.uroleplay.numbones, plur(u.uroleplay.numbones));
            you_have_X(buf);
        }
    }
    (void) fmt_elapsed_time(buf, final);
    enl_msg("Total elapsed playing time ", "is", "was", buf, "");

    if (!ge.en_via_menu) {
        display_nhwindow(ge.en_win, TRUE);
    } else {
        menu_item *selected = 0;

        end_menu(ge.en_win, (char *) 0);
        if (select_menu(ge.en_win, PICK_NONE, &selected) > 0)
            free((genericptr_t) selected);
        ge.en_via_menu = FALSE;
    }
    destroy_nhwindow(ge.en_win);
    ge.en_win = WIN_ERR;
}

/*ARGSUSED*/
/* display role, race, alignment and such to en_win */
staticfn void
background_enlightenment(int unused_mode UNUSED, int final)
{
    const char *role_titl, *rank_titl;
    int innategend, difgend, difalgn;
    char buf[BUFSZ], tmpbuf[BUFSZ];

    /* note that if poly'd, we need to use u.mfemale instead of flags.female
       to access hero's saved gender-as-human/elf/&c rather than current */
    innategend = (Upolyd ? u.mfemale : flags.female) ? 1 : 0;
    role_titl = (innategend && gu.urole.name.f) ? gu.urole.name.f
                                                : gu.urole.name.m;
    rank_titl = rank_of(u.ulevel, Role_switch, innategend);

    enlght_out(""); /* separator after title */
    enlght_out(_("Background:"));

    /* if polymorphed, report current shape before underlying role;
       will be repeated as first status: "you are transformed" and also
       among various attributes: "you are in beast form" (after being
       told about lycanthropy) or "you are polymorphed into <a foo>"
       (with countdown timer appended for wizard mode); we really want
       the player to know he's not a samurai at the moment... */
    if (Upolyd) {
        char anbuf[20]; /* includes trailing space; [4] suffices */
        struct permonst *uasmon = gy.youmonst.data;
        boolean altphrasing = vampshifted(&gy.youmonst);

        tmpbuf[0] = '\0';
        /* here we always use current gender, not saved role gender */
        if (!is_male(uasmon) && !is_female(uasmon) && !is_neuter(uasmon))
            Sprintf(tmpbuf, "%s ", genders[flags.female ? 1 : 0].adj);
        if (altphrasing)
            Sprintf(eos(tmpbuf), "%s in ",
                    pmname(&mons[gy.youmonst.cham],
                           flags.female ? FEMALE : MALE));
        Snprintf(buf, sizeof(buf), "%s%s%s%s form",
                 !final ? "currently " : "",
                 altphrasing ? just_an(anbuf, tmpbuf) : "in ",
                 tmpbuf, pmname(uasmon, flags.female ? FEMALE : MALE));
        you_are(buf, "");
    }

    /* report role; omit gender if it's redundant (eg, "female priestess") */
    tmpbuf[0] = '\0';
    if (!gu.urole.name.f
        && ((gu.urole.allow & ROLE_GENDMASK) == (ROLE_MALE | ROLE_FEMALE)
            || innategend != flags.initgend))
        Sprintf(tmpbuf, "%s ", genders[innategend].adj);
    buf[0] = '\0';
    if (i18n_active()) {
        char rbuf[BUFSZ];

        Snprintf(rbuf, sizeof rbuf,
                 Upolyd ? _(" You are actually %s, %s %s of level %d.")
                        : _(" You are %s, %s %s of level %d."),
                 _(rank_titl), _(role_titl),
                 gendered_word(gu.urace.adj, innategend), u.ulevel);
        enlght_out(rbuf);
    } else {
    if (Upolyd)
        Strcpy(buf, "actually "); /* "You are actually a ..." */
    if (!strcmpi(rank_titl, role_titl)) {
        /* omit role when rank title matches it */
        Sprintf(eos(buf), "%s, level %d %s%s", an(rank_titl), u.ulevel,
                tmpbuf, gu.urace.noun);
    } else {
        Sprintf(eos(buf), "%s, a level %d %s%s %s", an(rank_titl), u.ulevel,
                tmpbuf, gu.urace.adj, role_titl);
    }
    you_are(buf, "");
    }

    /* report alignment (bypass you_are() in order to omit ending period);
       adverb is used to distinguish between temporary change (helm of opp.
       alignment), permanent change (one-time conversion), and original */
    if (i18n_active()) {
        const char *how = (u.ualign.type != u.ualignbase[A_CURRENT])
                          ? (!final ? _("currently ") : _("temporarily "))
                          : (u.ualign.type != u.ualignbase[A_ORIGINAL])
                            ? (!final ? _("now ") : _("belatedly "))
                            : (!u.uconduct.gnostic && svm.moves > 1000L)
                              ? _("nominally ") : "";

        Snprintf(buf, sizeof buf,
                 !final ? _(" You are %s%s, on a mission for %s")
                        : _(" You were %s%s, on a mission for %s"),
                 how, gendered_word(align_str(u.ualign.type),
                                    flags.female ? 1 : 0), u_gname());
        enlght_out(buf);
        Strcpy(buf, !final ? _(" who is opposed by")
                           : _(" who was opposed by"));
        if (u.ualign.type != A_LAWFUL)
            Sprintf(eos(buf), _(" %s (%s) and"), align_gname(A_LAWFUL),
                    _(align_str(A_LAWFUL)));
        if (u.ualign.type != A_NEUTRAL)
            Sprintf(eos(buf), " %s (%s)%s", align_gname(A_NEUTRAL),
                    _(align_str(A_NEUTRAL)),
                    (u.ualign.type != A_CHAOTIC) ? _(" and") : "");
        if (u.ualign.type != A_CHAOTIC)
            Sprintf(eos(buf), " %s (%s)", align_gname(A_CHAOTIC),
                    _(align_str(A_CHAOTIC)));
        Strcat(buf, ".");
        enlght_out(buf);
        goto align_done;
    }
    Sprintf(buf, " %s%s%s, %son a mission for %s",
            You_, !final ? are : were,
            align_str(u.ualign.type),
            /* helm of opposite alignment (might hide conversion) */
            (u.ualign.type != u.ualignbase[A_CURRENT])
               /* what's the past tense of "currently"? if we used "formerly"
                  it would sound like a reference to the original alignment */
               ? (!final ? "currently " : "temporarily ")
               /* permanent conversion */
               : (u.ualign.type != u.ualignbase[A_ORIGINAL])
                  /* and what's the past tense of "now"? certainly not "then"
                     in a context like this...; "belatedly" == weren't that
                     way sooner (in other words, didn't start that way) */
                  ? (!final ? "now " : "belatedly ")
                  /* atheist (ignored in very early game) */
                  : (!u.uconduct.gnostic && svm.moves > 1000L)
                     ? "nominally "
                     /* lastly, normal case */
                     : "",
            u_gname());
    enlght_out(buf);
    /* show the rest of this game's pantheon (finishes previous sentence)
       [appending "also Moloch" at the end would allow for straightforward
       trailing "and" on all three aligned entries but looks too verbose] */
    Sprintf(buf, " who %s opposed by", !final ? "is" : "was");
    if (u.ualign.type != A_LAWFUL)
        Sprintf(eos(buf), " %s (%s) and", align_gname(A_LAWFUL),
                align_str(A_LAWFUL));
    if (u.ualign.type != A_NEUTRAL)
        Sprintf(eos(buf), " %s (%s)%s", align_gname(A_NEUTRAL),
                align_str(A_NEUTRAL),
                (u.ualign.type != A_CHAOTIC) ? " and" : "");
    if (u.ualign.type != A_CHAOTIC)
        Sprintf(eos(buf), " %s (%s)", align_gname(A_CHAOTIC),
                align_str(A_CHAOTIC));
    Strcat(buf, "."); /* terminate sentence */
    enlght_out(buf);
 align_done:

    /* show original alignment,gender,race,role if any have been changed;
       giving separate message for temporary alignment change bypasses need
       for tricky phrasing otherwise necessitated by possibility of having
       helm of opposite alignment mask a permanent alignment conversion */
    difgend = (innategend != flags.initgend);
    difalgn = (((u.ualign.type != u.ualignbase[A_CURRENT]) ? 1 : 0)
               + ((u.ualignbase[A_CURRENT] != u.ualignbase[A_ORIGINAL])
                  ? 2 : 0));
    if (difalgn & 1) { /* have temporary alignment so report permanent one */
        Sprintf(buf, "actually %s", align_str(u.ualignbase[A_CURRENT]));
        you_are(buf, "");
        difalgn &= ~1; /* suppress helm from "started out <foo>" message */
    }
    if (difgend || difalgn) { /* sex change or perm align change or both */
        Sprintf(buf, " You started out %s%s%s.",
                difgend ? genders[flags.initgend].adj : "",
                (difgend && difalgn) ? " and " : "",
                difalgn ? align_str(u.ualignbase[A_ORIGINAL]) : "");
        enlght_out(buf);
    }

    /* "You are left-handed." won't work well if polymorphed into something
       without hands; use "You are normally left-handed." in that situation */
    {
        boolean normally = strcmp(mbodypart_english(&gy.youmonst, HANDED),
                                  "handed") != 0;

        if (i18n_active()) {
            Strcpy(buf, gendered_word(normally
                        ? (URIGHTY ? " You are normally right-handed."
                                   : " You are normally left-handed.")
                        : (URIGHTY ? " You are right-handed."
                                   : " You are left-handed."),
                        flags.female ? 1 : 0));
            enlght_out(buf);
        } else {
            Sprintf(buf, "%s%s-handed", normally ? "normally " : "",
                    URIGHTY ? "right" : "left");
            you_are(buf, "");
        }
    }

    /* As of 3.6.2: dungeon level, so that ^X really has all status info as
       claimed by the comment below; this reveals more information than
       the basic status display, but that's one of the purposes of ^X;
       similar information is revealed by #overview; the "You died in
       <location>" given by really_done() is more rudimentary than this */
    *buf = *tmpbuf = '\0';
    if (In_endgame(&u.uz)) {
        int egdepth = observable_depth(&u.uz);

        (void) endgamelevelname(tmpbuf, egdepth);
        Snprintf(buf, sizeof(buf), "in the endgame, on the %s%s",
                 !strncmp(tmpbuf, "Plane", 5) ? "Elemental " : "", tmpbuf);
    } else if (Is_knox(&u.uz)) {
        /* this gives away the fact that the knox branch is only 1 level */
        Sprintf(buf, "on the %s level", svd.dungeons[u.uz.dnum].dname);
        /* TODO? maybe phrase it differently when actually inside the fort,
           if we're able to determine that (not trivial) */
    } else {
        char dgnbuf[QBUFSZ];

        Strcpy(dgnbuf, svd.dungeons[u.uz.dnum].dname);
        if (!strncmpi(dgnbuf, "The ", 4))
            *dgnbuf = lowc(*dgnbuf);
        Sprintf(tmpbuf, "level %d",
                In_quest(&u.uz) ? dunlev(&u.uz) : depth(&u.uz));
        /* TODO? maybe extend this bit to include various other automatic
           annotations from the dungeon overview code */
        if (Is_rogue_level(&u.uz))
            Strcat(tmpbuf, ", a primitive area");
        else if (Is_bigroom(&u.uz) && !Blind)
            Strcat(tmpbuf, ", a very big room");
        Snprintf(buf, sizeof(buf), "in %s, on %s", dgnbuf, tmpbuf);
        if (i18n_active()) {
            const char *extra = Is_rogue_level(&u.uz)
                                ? _(", a primitive area")
                                : (Is_bigroom(&u.uz) && !Blind)
                                  ? _(", a very big room") : "";

            char dnam[BUFSZ];

            Strcpy(dnam, _(svd.dungeons[u.uz.dnum].dname));
            /* "dans les Donjons du Destin" */
            if (!strncmp(dnam, "Le ", 3) || !strncmp(dnam, "La ", 3)
                || !strncmp(dnam, "Les ", 4))
                *dnam = lowc(*dnam);
            Snprintf(buf, sizeof buf,
                     !final ? _(" You are in %s, on level %d%s.")
                            : _(" You were in %s, on level %d%s."),
                     dnam,
                     In_quest(&u.uz) ? dunlev(&u.uz) : depth(&u.uz), extra);
            enlght_out(buf);
            *buf = '\0';
        }
    }
    if (*buf)
        you_are(buf, "");

    /* this is shown even if the 'time' option is off */
    if (svm.moves == 1L && i18n_active()) {
        enlght_out(_(" You have just started your adventure."));
    } else if (i18n_active()) {
        DISABLE_WARNING_FORMAT_NONLITERAL;
        Snprintf(buf, sizeof buf,
                 gendered_word(" You entered the dungeon %ld turns ago.",
                               flags.female ? 1 : 0), svm.moves);
        RESTORE_WARNING_FORMAT_NONLITERAL;
        enlght_out(buf);
    } else if (svm.moves == 1L) {
        you_have("just started your adventure", "");
    } else {
        /* 'turns' grates on the nerves in this context... */
        Sprintf(buf, "the dungeon %ld turn%s ago",
                svm.moves, plur(svm.moves));
        /* same phrasing for current and final: "entered" is unconditional */
        enlght_line(You_, "entered ", buf, "");
    }

    /* for gameover, these have been obtained in really_done() so that they
       won't vary if user leaves a disclosure prompt or --More-- unanswered
       long enough for the dynamic value to change between then and now */
    if (final ? iflags.at_midnight : midnight()) {
        enl_msg("It ", "is ", "was ", "the midnight hour", "");
    } else if (final ? iflags.at_night : night()) {
        enl_msg("It ", "is ", "was ", "nighttime", "");
    }
    /* other environmental factors */
    if (flags.moonphase == FULL_MOON || flags.moonphase == NEW_MOON) {
        /* [This had "tonight" but has been changed to "in effect".
           There is a similar issue to Friday the 13th--it's the value
           at the start of the current session but that session might
           have dragged on for an arbitrary amount of time.  We want to
           report the values that currently affect play--or affected
           play when game ended--rather than actual outside situation.] */
        Sprintf(buf, "a %s moon in effect%s",
                (flags.moonphase == FULL_MOON) ? "full"
                : (flags.moonphase == NEW_MOON) ? "new"
                  /* showing these would probably just lead to confusion
                     since they have no effect on game play... */
                  : (flags.moonphase < FULL_MOON) ? "first quarter"
                    : "last quarter",
                /* we don't have access to 'how' here--aside from survived
                   vs died--so settle for general platitude */
                final ? " when your adventure ended" : "");
        enl_msg("There ", "is ", "was ", buf, "");
    }
    if (flags.friday13) {
        /* let player know that friday13 penalty is/was in effect;
           we don't say "it is/was Friday the 13th" because that was at
           the start of the session and it might be past midnight (or
           days later if the game has been paused without save/restore),
           so phrase this similar to the start up message */
        Sprintf(buf, " Bad things %s on Friday the 13th.",
                !final ? "can happen"
                : (final == ENL_GAMEOVERALIVE) ? "could have happened"
                  /* there's no may to tell whether -1 Luck made a
                     difference but hero has died... */
                  : "happened");
        enlght_out(buf);
    }

    if (!Upolyd) {
        int ulvl = (int) u.ulevel;
        /* [flags.showexp currently does not matter; should it?] */

        /* experience level is already shown above */
        Sprintf(buf, "%-1ld experience point%s", u.uexp, plur(u.uexp));
        /* TODO?
         *  Remove wizard-mode restriction since patient players can
         *  determine the numbers needed without resorting to spoilers
         *  (even before this started being disclosed for 'final';
         *  just enable 'showexp' and look at normal status lines
         *  after drinking gain level potions or eating wraith corpses
         *  or being level-drained by vampires).
         */
        if (ulvl < 30 && (final || wizard)) {
            long nxtlvl = newuexp(ulvl), delta = nxtlvl - u.uexp;

            Sprintf(eos(buf), ", %ld %s%sneeded %s level %d",
                    delta, (u.uexp > 0) ? "more " : "",
                    /* present tense=="needed", past tense=="were needed" */
                    !final ? "" : (delta == 1L) ? "was " : "were ",
                    /* "for": grammatically iffy but less likely to wrap */
                    (ulvl < 18) ? "to attain" : "for", (ulvl + 1));
        }
        if (i18n_active()) {
            if (ulvl < 30 && (final || wizard))
                Snprintf(buf, sizeof buf,
                         !final ? _(" You have %ld experience points, %ld "
                                    "needed for level %d.")
                                : _(" You had %ld experience points, %ld "
                                    "were needed for level %d."),
                         u.uexp, newuexp(ulvl) - u.uexp, ulvl + 1);
            else
                Snprintf(buf, sizeof buf,
                         !final ? _(" You have %ld experience points.")
                                : _(" You had %ld experience points."),
                         u.uexp);
            enlght_out(buf);
        } else
            you_have(buf, "");
    }
#ifdef SCORE_ON_BOTL
    if (flags.showscore) {
        /* describes what's shown on status line, which is an approximation;
           only show it here if player has the 'showscore' option enabled */
        Sprintf(buf, "%ld%s", botl_score(),
                !final ? "" : " before end-of-game adjustments");
        enl_msg("Your score ", "is ", "was ", buf, "");
    }
#endif
}

/* hit points, energy points, armor class -- essential information which
   doesn't fit very well in other categories */
/*ARGSUSED*/
staticfn void
basics_enlightenment(int mode UNUSED, int final)
{
    static char Power[] = "energy points (spell power)";
    char buf[BUFSZ];
    int pw = u.uen, hp = (Upolyd ? u.mh : u.uhp),
        pwmax = u.uenmax, hpmax = (Upolyd ? u.mhmax : u.uhpmax);

    enlght_out(""); /* separator after background */
    enlght_out(_("Basics:"));

    if (hp < 0)
        hp = 0;
    /* "1 out of 1" rather than "all" if max is only 1; should never happen */
    if (i18n_active()) {
        if (hp == hpmax && hpmax > 1)
            Snprintf(buf, sizeof buf,
                     !final ? _(" You have all %d hit points.")
                            : _(" You had all %d hit points."), hpmax);
        else
            Snprintf(buf, sizeof buf,
                     !final ? _(" You have %d out of %d hit points.")
                            : _(" You had %d out of %d hit points."),
                     hp, hpmax);
        enlght_out(buf);
        if (!pwmax)
            Strcpy(buf, !final
                        ? _(" You have no energy points (spell power).")
                        : _(" You had no energy points (spell power)."));
        else if (pw == pwmax)
            Snprintf(buf, sizeof buf,
                     !final
                     ? _(" You have all %d energy points (spell power).")
                     : _(" You had all %d energy points (spell power)."),
                     pwmax);
        else
            Snprintf(buf, sizeof buf,
                     !final ? _(" You have %d out of %d energy points (spell "
                                "power).")
                            : _(" You had %d out of %d energy points (spell "
                                "power)."), pw, pwmax);
        enlght_out(buf);
        goto hp_pw_done;
    }
    if (hp == hpmax && hpmax > 1)
        Sprintf(buf, "all %d hit points", hpmax);
    else
        Sprintf(buf, "%d out of %d hit point%s", hp, hpmax, plur(hpmax));
    you_have(buf, "");

    /* low max energy is feasible, so handle couple of extra special cases */
    if (pwmax == 0 || (pw == pwmax && pwmax == 2)) /* both: not "all 2" */
        Sprintf(buf, "%s %s", !pwmax ? "no" : "both", Power);
    else if (pw == pwmax && pwmax > 2)
        Sprintf(buf, "all %d %s", pwmax, Power);
    else
        Sprintf(buf, "%d out of %d %s", pw, pwmax, Power);
    you_have(buf, "");
 hp_pw_done:

    if (Upolyd) {
        switch (mons[u.umonnum].mlevel) {
        case 0:
            /* status line currently being explained shows "HD:0" */
            Strcpy(buf, "0 hit dice (actually 1/2)");
            break;
        case 1:
            Strcpy(buf, "1 hit die");
            break;
        default:
            Sprintf(buf, "%d hit dice", mons[u.umonnum].mlevel);
            break;
        }
        you_have(buf, "");
    }

    find_ac(); /* enforces AC_MAX cap */
    if (i18n_active()) {
        Snprintf(buf, sizeof buf,
                 (abs(u.uac) != AC_MAX)
                 ? (!final ? _(" Your armor class is %d.")
                           : _(" Your armor class was %d."))
                 : (u.uac < 0)
                   ? (!final
                      ? _(" Your armor class is %d, the best possible.")
                      : _(" Your armor class was %d, the best possible."))
                   : (!final
                      ? _(" Your armor class is %d, the worst possible.")
                      : _(" Your armor class was %d, the worst possible.")),
                 u.uac);
        enlght_out(buf);
        goto ac_done;
    }
    Sprintf(buf, "%d", u.uac);
    if (abs(u.uac) == AC_MAX)
        Sprintf(eos(buf), ", the %s possible",
                (u.uac < 0) ? "best" : "worst");
    enl_msg("Your armor class ", "is ", "was ", buf, "");
 ac_done:

    /* gold; similar to doprgold (#showgold) but without shop billing info;
       includes container contents, unlike status line but like doprgold */
    {
        long umoney = money_cnt(gi.invent), hmoney = hidden_gold(final);

        if (i18n_active()) {
            /* whole sentences, so that each can be translated */
            if (!umoney && !hmoney)
                Strcpy(buf, !final ? _(" Your wallet is empty.")
                                   : _(" Your wallet was empty."));
            else if (!hmoney)
                Snprintf(buf, sizeof buf,
                         !final ? _(" Your wallet contains %ld %s.")
                                : _(" Your wallet contained %ld %s."),
                         umoney, currency(umoney));
            else
                Snprintf(buf, sizeof buf,
                         !final ? _(" Your wallet contains %ld %s, and you "
                                    "have %ld stashed away in your pack.")
                                : _(" Your wallet contained %ld %s, and you "
                                    "had %ld stashed away in your pack."),
                         umoney, currency(umoney), hmoney);
            enlght_out(buf);
            goto wallet_done;
        }
        if (!umoney) {
            Sprintf(buf, " Your wallet %s empty", !final ? "is" : "was");
        } else {
            Sprintf(buf, " Your wallet contain%s %ld %s", !final ? "s" : "ed",
                    umoney, currency(umoney));
        }
        /* terminate the wallet line if appropriate, otherwise add an
           introduction to subsequent continuation; output now either way */
        Strcat(buf, !hmoney ? "." : !umoney ? ", but" : ", and");
        enlght_out(buf);

        /* put contained gold on its own line to avoid excessive width; it's
           phrased as a continuation of the wallet line so not capitalized */
        if (hmoney) {
            Sprintf(buf, "%ld %s stashed away in your pack",
                    hmoney, umoney ? "more" : currency(hmoney));
            enl_msg("you ", "have ", "had ", buf, "");
        }
    }
 wallet_done:

    if (flags.pickup) {
        char ocl[MAXOCLASSES + 1];

        Strcpy(buf, "on");
        if (costly_spot(u.ux, u.uy)) {
            /* being in a shop inhibits autopickup, even 'pickup_thrown' */
            Strcat(buf, ", but temporarily disabled while inside the shop");
        } else {
            oc_to_str(flags.pickup_types, ocl);
            Sprintf(eos(buf), " for %s%s%s", *ocl ? "'" : "",
                    *ocl ? ocl : "all types", *ocl ? "'" : "");
            if (flags.pickup_thrown && *ocl)
                Strcat(buf, " plus thrown"); /* show when not 'all types' */
            if (ga.apelist)
                Strcat(buf, ", with exceptions");
        }
    } else
        Strcpy(buf, "off");
    if (i18n_active() && !flags.pickup)
        enlght_out(!final ? _(" Autopickup is off.")
                          : _(" Autopickup was off."));
    else if (i18n_active() && costly_spot(u.ux, u.uy))
        enlght_out(!final ? _(" Autopickup is on, but temporarily disabled "
                              "while inside the shop.")
                          : _(" Autopickup was on, but temporarily disabled "
                              "while inside the shop."));
    else if (i18n_active() && !strcmp(buf, "on for all types"))
        enlght_out(!final ? _(" Autopickup is on for all types.")
                          : _(" Autopickup was on for all types."));
    else
        enl_msg("Autopickup ", "is ", "was ", buf, "");
}

/* characteristics: expanded version of bottom line strength, dexterity, &c */
staticfn void
characteristics_enlightenment(int mode, int final)
{
    char buf[BUFSZ];

    enlght_out("");
    Strcpy(buf, !final ? _("Characteristics:") : _("Final Characteristics:"));
    enlght_out(buf);

    /* bottom line order */
    one_characteristic(mode, final, A_STR); /* strength */
    one_characteristic(mode, final, A_DEX); /* dexterity */
    one_characteristic(mode, final, A_CON); /* constitution */
    one_characteristic(mode, final, A_INT); /* intelligence */
    one_characteristic(mode, final, A_WIS); /* wisdom */
    one_characteristic(mode, final, A_CHA); /* charisma */
}

/* display one attribute value for characteristics_enlightenment() */
staticfn void
one_characteristic(int mode, int final, int attrindx)
{
    extern const char *const attrname[]; /* attrib.c */
    boolean hide_innate_value = FALSE, interesting_alimit;
    int acurrent, abase, apeak, alimit;
    const char *paren_pfx;
    char subjbuf[BUFSZ], valubuf[BUFSZ], valstring[32];

    /* being polymorphed or wearing certain cursed items prevents
       hero from reliably tracking changes to characteristics so
       we don't show base & peak values then; when the items aren't
       cursed, hero could take them off to check underlying values
       and we show those in such case so that player doesn't need
       to actually resort to doing that */
    if (Upolyd) {
        hide_innate_value = TRUE;
    } else if (Fixed_abil) {
        if (stuck_ring(uleft, RIN_SUSTAIN_ABILITY)
            || stuck_ring(uright, RIN_SUSTAIN_ABILITY))
            hide_innate_value = TRUE;
    }
    switch (attrindx) {
    case A_STR:
        if (uarmg && uarmg->otyp == GAUNTLETS_OF_POWER && uarmg->cursed)
            hide_innate_value = TRUE;
        break;
    case A_DEX:
        break;
    case A_CON:
        if (u_wield_art(ART_OGRESMASHER) && uwep->cursed)
            hide_innate_value = TRUE;
        break;
    case A_INT:
        if (uarmh && uarmh->otyp == DUNCE_CAP && uarmh->cursed)
            hide_innate_value = TRUE;
        break;
    case A_WIS:
        if (uarmh && uarmh->otyp == DUNCE_CAP && uarmh->cursed)
            hide_innate_value = TRUE;
        break;
    case A_CHA:
        break;
    default:
        return; /* impossible */
    };
    /* note: final disclosure includes MAGICENLIGHTENTMENT */
    if ((mode & MAGICENLIGHTENMENT) && !Upolyd)
        hide_innate_value = FALSE;

    acurrent = ACURR(attrindx);
    (void) attrval(attrindx, acurrent, valubuf); /* Sprintf(valubuf,"%d",) */
    if (i18n_active())
        Snprintf(subjbuf, sizeof subjbuf, _("Your %s "),
                 _(attrname[attrindx]));
    else
        Sprintf(subjbuf, "Your %s ", attrname[attrindx]);

    if (!hide_innate_value) {
        /* show abase, amax, and/or attrmax if acurr doesn't match abase
           (a magic bonus or penalty is in effect) or abase doesn't match
           amax (some points have been lost to poison or exercise abuse
           and are restorable) or attrmax is different from normal human
           (while game is in progress; trying to reduce dependency on
           spoilers to keep track of such stuff) or attrmax was different
           from abase (at end of game; this attribute wasn't maxed out) */
        abase = ABASE(attrindx);
        apeak = AMAX(attrindx);
        alimit = ATTRMAX(attrindx);
        /* criterium for whether the limit is interesting varies */
        interesting_alimit =
            final ? TRUE /* was originally `(abase != alimit)' */
                  : (alimit != (attrindx != A_STR ? 18 : STR18(100)));
        paren_pfx = final ? " (" : " (current; ";
        if (acurrent != abase) {
            Sprintf(eos(valubuf), "%sbase:%s", paren_pfx,
                    attrval(attrindx, abase, valstring));
            paren_pfx = ", ";
        }
        if (abase != apeak) {
            Sprintf(eos(valubuf), "%speak:%s", paren_pfx,
                    attrval(attrindx, apeak, valstring));
            paren_pfx = ", ";
        }
        if (interesting_alimit) {
            Sprintf(eos(valubuf), "%s%slimit:%s", paren_pfx,
                    /* more verbose if exceeding 'limit' due to magic bonus */
                    (acurrent > alimit) ? "innate " : "",
                    attrval(attrindx, alimit, valstring));
            /* paren_pfx = ", "; */
        }
        if (acurrent != abase || abase != apeak || interesting_alimit)
            Strcat(valubuf, ")");
    }
    enl_msg(subjbuf, "is ", "was ", valubuf, "");
}

/* status: selected obvious capabilities, assorted troubles */
staticfn void
status_enlightenment(int mode, int final)
{
    boolean magic = (mode & MAGICENLIGHTENMENT) ? TRUE : FALSE;
    int cap;
    char buf[BUFSZ], youtoo[BUFSZ], heldmon[BUFSZ];
    boolean Riding = (u.usteed
                      /* if hero dies while dismounting, u.usteed will still
                         be set; we want to ignore steed in that situation */
                      && !(final == ENL_GAMEOVERDEAD
                           && !strcmp(svk.killer.name, "riding accident")));
    const char *steedname = (!Riding ? (char *) 0
                      : x_monnam(u.usteed,
                                 u.usteed->mtame ? ARTICLE_YOUR : ARTICLE_THE,
                                 (char *) 0,
                                 (SUPPRESS_SADDLE | SUPPRESS_HALLUCINATION),
                                 FALSE));

    /*\
     * Status (many are abbreviated on bottom line; others are or
     *     should be discernible to the hero hence to the player)
    \*/
    enlght_out(""); /* separator after title or characteristics */
    enlght_out(final ? _("Final Status:") : _("Status:"));

    Strcpy(youtoo, You_);
    /* not a traditional status but inherently obvious to player; more
       detail given below (attributes section) for magic enlightenment */
    if (Upolyd) {
        Strcpy(buf, "transformed");
        if (ugenocided())
            Sprintf(eos(buf), " and %s %s inside",
                    final ? "felt" : "feel", udeadinside());
        you_are(buf, "");
    }
    /* not a trouble, but we want to display riding status before maybe
       reporting steed as trapped or hero stuck to cursed saddle */
    if (Riding) {
        Sprintf(buf, "riding %s", steedname);
        you_are(buf, "");
        Sprintf(eos(youtoo), "and %s ", steedname);
    }
    /* other movement situations that hero should always know */
    if (Levitation) {
        if (Lev_at_will && magic)
            you_are("levitating, at will", "");
        else
            enl_msg(youtoo, are, were, "levitating", from_what(LEVITATION));
    } else if (Flying) { /* can only fly when not levitating */
        enl_msg(youtoo, are, were, "flying", from_what(FLYING));
    }
    if (Underwater) {
        you_are("underwater", "");
    } else if (u.uinwater) {
        you_are(Swimming ? "swimming" : "in water", from_what(SWIMMING));
    } else if (walking_on_water()) {
        /* show active Wwalking here, potential Wwalking elsewhere */
        Sprintf(buf, "walking on %s",
                is_pool(u.ux, u.uy) ? "water"
                : is_lava(u.ux, u.uy) ? "lava"
                  : surface(u.ux, u.uy)); /* catchall; shouldn't happen */
        you_are(buf, from_what(WWALKING));
    }
    if (Upolyd && (u.uundetected || U_AP_TYPE != M_AP_NOTHING))
        youhiding(TRUE, final);

    /* internal troubles, mostly in the order that prayer ranks them */
    if (Stoned) {
        if (final && (Stoned & I_SPECIAL))
            enlght_out(" You turned into stone.");
        else
            you_are("turning to stone", "");
    }
    if (Slimed) {
        if (final && (Slimed & I_SPECIAL))
            enlght_out(" You turned into slime.");
        else
            you_are("turning into slime", "");
    }
    if (Strangled) {
        if (u.uburied) {
            you_are("buried", "");
        } else {
            if (final && (Strangled & I_SPECIAL)) {
                enlght_out(" You died from strangulation.");
            } else {
                Strcpy(buf, "being strangled");
                if (wizard)
                    Sprintf(eos(buf), " (%ld)", (Strangled & TIMEOUT));
                you_are(buf, from_what(STRANGLED));
            }
        }
    }
    if (Sick) {
        /* the two types of sickness are lumped together; hero can be
           afflicted by both but there is only one timeout; botl status
           puts TermIll before FoodPois and death due to timeout reports
           terminal illness if both are in effect, so do the same here */
        if (final && (Sick & I_SPECIAL)) {
            Sprintf(buf, " %sdied from %s.", You_, /* has trailing space */
                    (u.usick_type & SICK_NONVOMITABLE)
                    ? "terminal illness" : "food poisoning");
            enlght_out(buf);
        } else {
            /* unlike death due to sickness, report the two cases separately
               because it is possible to cure one without curing the other */
            if (u.usick_type & SICK_NONVOMITABLE)
                you_are("terminally sick from illness", "");
            if (u.usick_type & SICK_VOMITABLE)
                you_are("terminally sick from food poisoning", "");
        }
    }
    if (Vomiting)
        you_are("nauseated", "");
    if (Stunned)
        you_are("stunned", "");
    if (Confusion)
        you_are("confused", "");
    if (Hallucination)
        you_are("hallucinating", "");
    if (Blind) {
        /* check the reasons in same order as from_what() */
        Sprintf(buf, "%s blind",
                (HBlinded & FROMOUTSIDE) != 0L ? "permanently"
                : (HBlinded & FROMFORM) ? "innately"
                  /* better phrasing desperately wanted... */
                  : Blindfolded_only ? "deliberately"
                    /* timed, possibly combined with blindfold */
                    : "temporarily");
        if (wizard && (HBlinded == BlindedTimeout && !Blindfolded))
            Sprintf(eos(buf), " (%ld)", BlindedTimeout);
        /* !haseyes: avoid "you are innately blind innately" */
        you_are(buf, !haseyes(gy.youmonst.data) ? "" : from_what(BLINDED));
    }
    if (Deaf)
        you_are("deaf", from_what(DEAF));

    /* external troubles, more or less */
    if (Punished) {
        if (uball) {
            Sprintf(buf, "chained to %s", ansimpleoname(uball));
        } else {
            impossible("Punished without uball?");
            Strcpy(buf, "punished");
        }
        you_are(buf, "");
    }
    if (u.utrap) {
        char predicament[BUFSZ];
        boolean anchored = (u.utraptype == TT_BURIEDBALL);

        (void) trap_predicament(predicament, final, wizard);
        if (u.usteed) { /* not `Riding' here */
            Sprintf(buf, "%s%s ", anchored ? "you and " : "", steedname);
            *buf = highc(*buf);
            enl_msg(buf, (anchored ? "are " : "is "),
                    (anchored ? "were " : "was "), predicament, "");
        } else
            you_are(predicament, "");
    } /* (u.utrap) */
    heldmon[0] = '\0'; /* lint suppression */
    if (u.ustuck) { /* includes u.uswallow */
        Strcpy(heldmon, a_monnam(u.ustuck));
        if (!strcmp(heldmon, "it")
            && (!has_mgivenname(u.ustuck)
                || strcmp(MGIVENNAME(u.ustuck), "it") != 0))
            Strcpy(heldmon, "an unseen creature");
    }
    if (u.uswallow) {
        assert(u.ustuck != NULL); /* implied by u.uswallow */
        Snprintf(buf, sizeof buf, "%s by %s",
                digests(u.ustuck->data) ? "swallowed" : "engulfed",
                heldmon);
        if (dmgtype(u.ustuck->data, AD_DGST)) {
            /* if final, death via digestion can be deduced by u.uswallow
               still being True and u.uswldtim having been decremented to 0 */
            if (final && !u.uswldtim)
                Strcat(buf, " and got totally digested");
            else
                Sprintf(eos(buf), " and %s being digested",
                        final ? "were" : "are");
        }
        if (wizard)
            Sprintf(eos(buf), " (%u)", u.uswldtim);
        you_are(buf, "");
    } else if (u.ustuck) {
        boolean ustick = (Upolyd && sticks(gy.youmonst.data));
        int dx = u.ustuck->mx - u.ux, dy = u.ustuck->my - u.uy;

        Snprintf(buf, sizeof buf, "%s %s (%s)",
                 ustick ? "holding" : "held by",
                 heldmon, dxdy_to_dist_descr(dx, dy, TRUE));
        you_are(buf, "");
    }
    if (Riding) {
        struct obj *saddle = which_armor(u.usteed, W_SADDLE);

        if (saddle && saddle->cursed) {
            Sprintf(buf, "stuck to %s %s", s_suffix(steedname),
                    simpleonames(saddle));
            you_are(buf, "");
        }
    }
    if (Wounded_legs) {
        /* EWounded_legs is used to track left/right/both rather than some
           form of extrinsic impairment; HWounded_legs is used for timeout;
           both apply to steed instead of hero when mounted */
        long whichleg = (EWounded_legs & BOTH_SIDES);
        const char *bp = u.usteed ? mbodypart(u.usteed, LEG) : body_part(LEG),
            *article = "a ", /* precedes "wounded", so never "an " */
            *leftright = "";

        if (whichleg == BOTH_SIDES)
            bp = makeplural(bp), article = "";
        else
            leftright = (whichleg == LEFT_SIDE) ? "left " : "right ";
        Sprintf(buf, "%swounded %s%s", article, leftright, bp);

        /* when mounted, Wounded_legs applies to steed rather than to
           hero; we only report steed's wounded legs in wizard mode */
        if (u.usteed) { /* not `Riding' here */
            if (wizard && steedname) {
                char steednambuf[BUFSZ];

                Strcpy(steednambuf, steedname);
                *steednambuf = highc(*steednambuf);
                enl_msg(steednambuf, " has ", " had ", buf, "");
            }
        } else {
            you_have(buf, "");
        }
    }
    if (Glib) {
        Sprintf(buf, "slippery %s", fingers_or_gloves(TRUE));
        if (wizard)
            Sprintf(eos(buf), " (%ld)", (Glib & TIMEOUT));
        you_have(buf, "");
    }
    if (Fumbling) {
        if (magic || cause_known(FUMBLING))
            enl_msg(You_, "fumble", "fumbled", "", from_what(FUMBLING));
    }
    if (Sleepy) {
        if (magic || cause_known(SLEEPY)) {
            Strcpy(buf, from_what(SLEEPY));
            if (wizard)
                Sprintf(eos(buf), " (%ld)", (HSleepy & TIMEOUT));
            enl_msg("You ", "fall", "fell", " asleep uncontrollably", buf);
        }
    }
    /* hunger/nutrition */
    if (Hunger) {
        if (magic || cause_known(HUNGER))
            enl_msg(You_, "hunger", "hungered", " rapidly",
                    from_what(HUNGER));
    }
    Strcpy(buf, hu_stat[u.uhs]); /* hunger status; omitted if "normal" */
    mungspaces(buf);             /* strip trailing spaces */
    /* status line doesn't show hunger when state is "not hungry", we do;
       needed for wizard mode's reveal of u.uhunger but add it for everyone */
    if (!*buf)
        Strcpy(buf, "not hungry");
    if (*buf) { /* (since "not hungry" was added, this will always be True) */
        *buf = lowc(*buf); /* override capitalization */
        if (!strcmp(buf, "weak"))
            Strcat(buf, " from severe hunger");
        else if (!strncmp(buf, "faint", 5)) /* fainting, fainted */
            Strcat(buf, " due to starvation");
        if (wizard)
            Sprintf(eos(buf), " <%d>", u.uhunger);
        you_are(buf, "");
    }
    /* encumbrance */
    if ((cap = near_capacity()) > UNENCUMBERED) {
        const char *adj = "?_?"; /* (should always get overridden) */

        Strcpy(buf, enc_stat[cap]);
        *buf = lowc(*buf);
        switch (cap) {
        case SLT_ENCUMBER:
            adj = "slightly";
            break; /* burdened */
        case MOD_ENCUMBER:
            adj = "moderately";
            break; /* stressed */
        case HVY_ENCUMBER:
            adj = "very";
            break; /* strained */
        case EXT_ENCUMBER:
            adj = "extremely";
            break; /* overtaxed */
        case OVERLOADED:
            adj = "not possible";
            break;
        }
        if (wizard)
            Sprintf(eos(buf), " <%d>", inv_weight());
        Sprintf(eos(buf), "; movement %s %s%s", !final ? "is" : "was", adj,
                (cap < OVERLOADED) ? " slowed" : "");
        you_are(buf, "");
    } else {
        /* last resort entry, guarantees Status section is non-empty
           (no longer needed for that purpose since weapon status added;
           still useful though) */
        Strcpy(buf, "unencumbered");
        if (wizard)
            Sprintf(eos(buf), " <%d>", inv_weight());
        you_are(buf, "");
    }
    /* current weapon(s) and corresponding skill level(s) */
    weapon_insight(final);
    /* unlike ring of increase accuracy's effect, the monk's suit penalty
       is too blatant to be restricted to magical enlightenment */
    if (iflags.tux_penalty && !Upolyd) {
        (void) enlght_combatinc("to hit", -gu.urole.spelarmr, final, buf);
        /* if from_what() ever gets extended from wizard mode to normal
           play, it could be adapted to handle this */
        Sprintf(eos(buf), " due to your %s", suit_simple_name(uarm));
        you_have(buf, "");
    }
    /* report 'nudity' */
    if (!uarm && !uarmu && !uarmc && !uarms && !uarmg && !uarmf && !uarmh) {
        if (u.uroleplay.nudist)
            enl_msg(You_, "do not wear", "did not wear", " any armor", "");
        else
            you_are("not wearing any armor", "");
    }
}

/* extracted from status_enlightenment() to reduce clutter there */
staticfn void
weapon_insight(int final)
{
    char buf[BUFSZ];
    int wtype;

    /* report being weaponless; distinguish whether gloves are worn
       [perhaps mention silver ring(s) when not wearing gloves?] */
    if (!uwep) {
        you_are(empty_handed(), "");

    /* two-weaponing implies hands and
       a weapon or wep-tool (not other odd stuff) in each hand */
    } else if (u.twoweap) {
        you_are("wielding two weapons at once", "");

    /* report most weapons by their skill class (so a katana will be
       described as a long sword, for instance; mattock, hook, and aklys
       are exceptions), or wielded non-weapon item by its object class */
    } else {
        const char *what = weapon_descr(uwep);

        /* [what about other silver items?] */
        if (uwep->otyp == SHIELD_OF_REFLECTION)
            what = shield_simple_name(uwep); /* silver|smooth shield */
        else if (is_wet_towel(uwep))
            what = /* (uwep->spe < 3) ? "moist towel" : */ "wet towel";

        if (i18n_active()) {
            Snprintf(buf, sizeof buf,
                     !final ? _(" You are wielding %s.")
                            : _(" You were wielding %s."),
                     (uwep->quan == 1L) ? an(xname(uwep)) : xname(uwep));
            enlght_out(buf);
            *buf = '\0';
        } else if (!strcmpi(what, "armor") || !strcmpi(what, "food")
            || !strcmpi(what, "venom"))
            Sprintf(buf, "wielding some %s", what);
        else
            /* [maybe include known blessed?] */
            Sprintf(buf, "wielding %s",
                    (uwep->quan == 1L) ? an(what) : makeplural(what));
        if (*buf)
            you_are(buf, "");
    }

    /*
     * Skill with current weapon.  Might help players who've never
     * noticed #enhance or decided that it was pointless.
     */
    if ((wtype = weapon_type(uwep)) != P_NONE && (!uwep || !is_ammo(uwep))) {
        char sklvlbuf[20];
        int sklvl = P_SKILL(wtype);
        boolean hav = (sklvl != P_UNSKILLED && sklvl != P_SKILLED);

        if (sklvl == P_ISRESTRICTED)
            Strcpy(sklvlbuf, "no");
        else
            (void) lcase(skill_level_name(wtype, sklvlbuf));
        /* "you have no/basic/expert/master/grand-master skill with <skill>"
           or "you are unskilled/skilled in <skill>" */
        Sprintf(buf, "%s %s %s", sklvlbuf,
                hav ? "skill with" : "in", skill_name(wtype));

        if (!u.twoweap && i18n_active()) {
            Snprintf(buf, sizeof buf,
                     !final ? _(" Your skill in %s is: %s%s.")
                            : _(" Your skill in %s was: %s%s."),
                     skill_name_i18n(skill_name(wtype)),
                     (sklvl == P_ISRESTRICTED) ? _("restricted")
                                               : C_("skill level", sklvlbuf),
                     !can_advance(wtype, FALSE) ? ""
                     : !final ? _(" (you can enhance it)")
                              : _(" (you could have enhanced it)"));
            enlght_out(buf);
        } else if (!u.twoweap) {
            if (can_advance(wtype, FALSE))
                Sprintf(eos(buf), " and %s that",
                        !final ? "can enhance" : "could have enhanced");
            if (hav)
                you_have(buf, "");
            else
                you_are(buf, "");

        } else if (i18n_active()) { /* two-weapon, whole sentences */
            int wtype2 = weapon_type(uswapwep), sklvl2 = P_SKILL(wtype2),
                twoskl = P_SKILL(P_TWO_WEAPON_COMBAT), w, i;
            char sklvlbuf2[20], twobuf[20];
            const char *enh[3];
            int nenh = 0;

            if (twoskl == P_ISRESTRICTED) {
                twoskl = P_UNSKILLED;
                Strcpy(twobuf, _("restricted"));
            } else {
                Strcpy(twobuf, C_("skill level",
                                  lcase(skill_level_name(P_TWO_WEAPON_COMBAT,
                                                         twobuf))));
            }
            for (w = 0; w < 2; ++w) {
                int wt = w ? wtype2 : wtype, sk = w ? sklvl2 : sklvl;

                if (w && wtype2 == wtype)
                    break;
                if (w)
                    (void) lcase(skill_level_name(wt, sklvlbuf2));
                Snprintf(buf, sizeof buf,
                         !final ? _(" Your skill in %s is: %s%s.")
                                : _(" Your skill in %s was: %s%s."),
                         skill_name_i18n(skill_name(wt)),
                         (sk == P_ISRESTRICTED) ? _("restricted")
                         : C_("skill level", w ? sklvlbuf2 : sklvlbuf), "");
                enlght_out(buf);
                if (twoskl < sk)
                    Snprintf(buf, sizeof buf,
                             !final ? _(" With two weapons, your skill in %s"
                                        " is limited to %s.")
                                    : _(" With two weapons, your skill in %s"
                                        " was limited to %s."),
                             skill_name_i18n(skill_name(wt)), twobuf);
                else if (twoskl > sk)
                    Snprintf(buf, sizeof buf,
                             !final
                               ? _(" Your two weapon skill is limited by"
                                   " your skill in %s.")
                               : _(" Your two weapon skill was limited by"
                                   " your skill in %s."),
                             skill_name_i18n(skill_name(wt)));
                else
                    *buf = '\0';
                if (*buf)
                    enlght_out(buf);
            }
            Snprintf(buf, sizeof buf,
                     !final ? _(" Your two weapon skill is: %s.")
                            : _(" Your two weapon skill was: %s."), twobuf);
            enlght_out(buf);
            if (can_advance(wtype, FALSE))
                enh[nenh++] = skill_name_i18n(skill_name(wtype));
            if (wtype2 != wtype && can_advance(wtype2, FALSE))
                enh[nenh++] = skill_name_i18n(skill_name(wtype2));
            if (can_advance(P_TWO_WEAPON_COMBAT, FALSE))
                enh[nenh++] = _("two weapons");
            if (nenh) {
                char lst[BUFSZ];

                Strcpy(lst, enh[0]);
                for (i = 1; i < nenh; ++i)
                    Snprintf(eos(lst), sizeof lst - strlen(lst),
                             (i == nenh - 1) ? _(" and %s") : ", %s", enh[i]);
                Snprintf(buf, sizeof buf,
                         !final ? _(" You can enhance your skill with %s.")
                                : _(" You could have enhanced your skill with"
                                    " %s."), lst);
                enlght_out(buf);
            }
            *buf = '\0';
        } else { /* two-weapon */
            static const char also_[] = "also ";
            char pfx[QBUFSZ], sfx[QBUFSZ],
                sknambuf2[20], sklvlbuf2[20], twobuf[20];
            const char *also = "", *also2 = "", *also3 = (char *) 0,
                       *verb_present, *verb_past;
            int wtype2 = weapon_type(uswapwep),
                sklvl2 = P_SKILL(wtype2),
                twoskl = P_SKILL(P_TWO_WEAPON_COMBAT);
            boolean a1, a2, ab,
                    hav2 = (sklvl2 != P_UNSKILLED && sklvl2 != P_SKILLED);

            /* normally hero must have access to two-weapon skill in
               order to initiate u.twoweap, but not if polymorphed into
               a form which has multiple weapon attacks, so we need to
               avoid getting bitten by unexpected skill value */
            if (twoskl == P_ISRESTRICTED) {
                twoskl = P_UNSKILLED;
                /* restricted is the same as unskilled as far as bonus
                   or penalty goes, and it isn't ordinarily seen so
                   skill_level_name() returns "Unknown" for it */
                Strcpy(twobuf, "restricted");
            } else {
                (void) lcase(skill_level_name(P_TWO_WEAPON_COMBAT, twobuf));
            }

            /* keep buf[] from above in case skill levels match */
            pfx[0] = sfx[0] = '\0';
            if (twoskl < sklvl) {
                /* twoskil won't be restricted so sklvl is at least basic */
                Sprintf(pfx, "Your skill in %s ", skill_name(wtype));
                Sprintf(sfx, " limited by being %s with two weapons", twobuf);
                also = also_;
            } else if (twoskl > sklvl) {
                /* sklvl might be restricted */
                Strcpy(pfx, "Your two weapon skill ");
                Strcpy(sfx, " limited by ");
                if (sklvl > P_ISRESTRICTED)
                    Sprintf(eos(sfx), "being %s", sklvlbuf);
                else
                    Sprintf(eos(sfx), "having no skill");
                Sprintf(eos(sfx), " with %s", skill_name(wtype));
                also2 = also_;
            } else {
                Strcat(buf, " and two weapons");
                also3 = also_;
            }
            if (*pfx)
                enl_msg(pfx, "is", "was", sfx, "");
            else if (hav)
                you_have(buf, "");
            else
                you_are(buf, "");

            /* skip comparison between secondary and two-weapons if it is
               identical to the comparison between primary and twoweap */
            if (wtype2 != wtype) {
                Strcpy(sknambuf2, skill_name(wtype2));
                (void) lcase(skill_level_name(wtype2, sklvlbuf2));
                verb_present = "is", verb_past = "was";
                pfx[0] = sfx[0] = buf[0] = '\0';
                if (twoskl < sklvl2) {
                    /* twoskil is at least unskilled, sklvl2 at least basic */
                    Sprintf(pfx, "Your skill in %s ", sknambuf2);
                    Sprintf(sfx, " %slimited by being %s with two weapons",
                            also, twobuf);
                } else if (twoskl > sklvl2) {
                    /* sklvl2 might be restricted */
                    Strcpy(pfx, "Your two weapon skill ");
                    Sprintf(sfx, " %slimited by ", also2);
                    if (sklvl2 > P_ISRESTRICTED)
                        Sprintf(eos(sfx), "being %s", sklvlbuf2);
                    else
                        Strcat(eos(sfx), "having no skill");
                    Sprintf(eos(sfx), " with %s", sknambuf2);
                } else {
                    /* equal; two-weapon is at least unskilled, so sklvl2 is
                       too; "you [also] have basic/expert/master/grand-master
                       skill with <skill>" or "you [also] are unskilled/
                       skilled in <skill> */
                    Sprintf(buf, "%s %s %s", sklvlbuf2,
                            hav2 ? "skill with" : "in", sknambuf2);
                    Strcat(buf, " and two weapons");
                    if (also3) {
                        Strcpy(pfx, "You also ");
                        Snprintf(sfx, sizeof(sfx), " %s", buf), buf[0] = '\0';
                        verb_present = hav2 ? "have" : "are";
                        verb_past = hav2 ? "had" : "were";
                    }
                }
                if (*pfx)
                    enl_msg(pfx, verb_present, verb_past, sfx, "");
                else if (hav2)
                    you_have(buf, "");
                else
                    you_are(buf, "");
            } /* wtype2 != wtype */

            /* if training and available skill credits already allow
               #enhance for any of primary, secondary, or two-weapon,
               tell the player; avoid attempting figure out whether
               spending skill credits enhancing one might make either
               or both of the others become ineligible for enhancement */
            a1 = can_advance(wtype, FALSE);
            a2 = (wtype2 != wtype) ? can_advance(wtype2, FALSE) : FALSE;
            ab = can_advance(P_TWO_WEAPON_COMBAT, FALSE);
            if (a1 || a2 || ab) {
                static const char also_wik_[] = " and also with ";

                /* for just one, the conditionals yield
                   1) "skill with <that one>"; for more than one:
                   2) "skills with <primary> and also with <secondary>" or
                   3) "skills with <primary> and also with two-weapons" or
                   4) "skills with <secondary> and also with two-weapons" or
                   5) "skills with <primary>, <secondary>, and two-weapons"
                   (no 'also's or extra 'with's for case 5); when primary
                   and secondary use the same skill, only cases 1 and 3 are
                   possible because 'a2' gets forced to False above */
                Sprintf(sfx, " skill%s with %s%s%s%s%s",
                        ((int) a1 + (int) a2 + (int) ab > 1) ? "s" : "",
                        a1 ? skill_name(wtype) : "",
                        ((a1 && a2 && ab) ? ", "
                         : (a1 && (a2 || ab)) ? also_wik_ : ""),
                        a2 ? skill_name(wtype2) : "",
                        ((a1 && a2 && ab) ? ", and "
                         : (a2 && ab) ? also_wik_ : ""),
                        ab ? "two weapons" : "");
                enl_msg(You_, "can enhance", "could have enhanced", sfx, "");
            }
        } /* two-weapon */
    } /* skill applies */
}

staticfn void
item_resistance_message(
    int adtyp,
    const char *prot_message,
    int final)
{
    int protection = u_adtyp_resistance_obj(adtyp);

    if (protection) {
        boolean somewhat = protection < 99;

        enl_msg("Your items ",
                somewhat ? "are somewhat" : "are",
                somewhat ? "were somewhat" : "were",
                prot_message, item_what(adtyp));
    }
}

/* attributes: intrinsics and the like, other non-obvious capabilities */
staticfn void
attributes_enlightenment(
    int unused_mode UNUSED,
    int final)
{
    static NEARDATA const char
        if_surroundings_permitted[] = " if surroundings permitted";
    int ltmp, armpro, warnspecies;
    char buf[BUFSZ];

    /*\
     *  Attributes
    \*/
    enlght_out("");
    enlght_out(final ? _("Final Attributes:") : _("Attributes:"));

    if (u.uevent.uhand_of_elbereth) {
        static const char *const hofe_titles[3] = { "the Hand of Elbereth",
                                                    "the Envoy of Balance",
                                                    "the Glory of Arioch" };
        you_are(hofe_titles[u.uevent.uhand_of_elbereth - 1], "");
    }

    Sprintf(buf, "%s", piousness(TRUE, "aligned"));
    if (i18n_active() && u.ualign.record < 0) {
        enlght_out(!strcmp(buf, "strayed")
                   ? gendered_word(!final ? " You have strayed."
                                          : " You had strayed.",
                                   flags.female ? 1 : 0)
                   : !strcmp(buf, "sinned")
                     ? (!final ? _(" You have sinned.")
                               : _(" You had sinned."))
                     : (!final ? _(" You have transgressed.")
                               : _(" You had transgressed.")));
    } else if (i18n_active()) {
        Snprintf(buf, sizeof buf, "%s",
                 pious_i18n(piousness(TRUE, "aligned")));
        you_are(buf, "");
    } else if (u.ualign.record >= 0)
        you_are(buf, "");
    else
        you_have(buf, "");

    if (wizard) {
        Sprintf(buf, " %d", u.ualign.record);
        enl_msg("Your alignment ", "is", "was", buf, "");
    }

    /*** Resistances to troubles ***/
    if (Invulnerable)
        you_are("invulnerable", from_what(INVULNERABLE));
    if (Antimagic)
        you_are("magic-protected", from_what(ANTIMAGIC));
    if (Fire_resistance)
        you_are("fire resistant", from_what(FIRE_RES));
    item_resistance_message(AD_FIRE, " protected from fire", final);
    if (Cold_resistance)
        you_are("cold resistant", from_what(COLD_RES));
    item_resistance_message(AD_COLD, " protected from cold", final);
    if (Sleep_resistance)
        you_are("sleep resistant", from_what(SLEEP_RES));
    if (Disint_resistance)
        you_are("disintegration resistant", from_what(DISINT_RES));
    item_resistance_message(AD_DISN, " protected from disintegration", final);
    if (Shock_resistance)
        you_are("shock resistant", from_what(SHOCK_RES));
    item_resistance_message(AD_ELEC, " protected from electric shocks",
                            final);
    if (Poison_resistance)
        you_are("poison resistant", from_what(POISON_RES));
    if (Acid_resistance) {
        Sprintf(buf, "%.20s%.30s",
                temp_resist(ACID_RES) ? "temporarily " : "",
                "acid resistant");
        you_are(buf, from_what(ACID_RES));
    }
    item_resistance_message(AD_ACID, " protected from acid", final);
    if (Drain_resistance)
        you_are("level-drain resistant", from_what(DRAIN_RES));
    if (Sick_resistance)
        you_are("immune to sickness", from_what(SICK_RES));
    if (Stone_resistance) {
        Sprintf(buf, "%.20s%.30s",
                temp_resist(STONE_RES) ? "temporarily " : "",
                "petrification resistant");
        you_are(buf, from_what(STONE_RES));
    }
    if (Halluc_resistance)
        enl_msg(You_, "resist", "resisted", " hallucinations",
                from_what(HALLUC_RES));
    if (u.uedibility)
        you_can("recognize detrimental food", "");

    /*** Vision and senses ***/
    if ((HBlinded || EBlinded) && BBlinded) /* blind w/ blindness blocked */
        you_can("see", from_what(-BLINDED)); /* Eyes of the Overworld */
    if (Blnd_resist && !Blind) /* skip if no eyes or blindfolded */
        you_are("not subject to light-induced blindness",
                from_what(BLND_RES));
    if (See_invisible) {
        if (!Blind)
            enl_msg(You_, "see", "saw", " invisible", from_what(SEE_INVIS));
        else if (!PermaBlind)
            enl_msg(You_, "will see", "would have seen",
                    " invisible when not blind", "");
        else
            enl_msg(You_, "would see", "would have seen",
                    " invisible if not blind", "");
    }
    if (Blind_telepat)
        you_are("telepathic", from_what(TELEPAT));
    if (Warning)
        you_are("warned", from_what(WARNING));
    if (Warn_of_mon && svc.context.warntype.obj) {
        Sprintf(buf, "aware of the presence of %s",
                (svc.context.warntype.obj & M2_ORC) ? "orcs"
                : (svc.context.warntype.obj & M2_ELF) ? "elves"
                  : (svc.context.warntype.obj & M2_DEMON) ? "demons"
                    : something);
        you_are(buf, from_what(WARN_OF_MON));
    }
    if (Warn_of_mon && svc.context.warntype.polyd) {
        Sprintf(buf, "aware of the presence of %s",
                ((svc.context.warntype.polyd & (M2_HUMAN | M2_ELF))
                 == (M2_HUMAN | M2_ELF)) ? "humans and elves"
                    : (svc.context.warntype.polyd & M2_HUMAN) ? "humans"
                      : (svc.context.warntype.polyd & M2_ELF) ? "elves"
                        : (svc.context.warntype.polyd & M2_ORC) ? "orcs"
                          : (svc.context.warntype.polyd & M2_DEMON) ? "demons"
                            : "certain monsters");
        you_are(buf, "");
    }
    warnspecies =  svc.context.warntype.speciesidx;
    if (Warn_of_mon && ismnum(warnspecies)) {
        Sprintf(buf, "aware of the presence of %s",
                makeplural(mons[warnspecies].pmnames[NEUTRAL]));
        you_are(buf, from_what(WARN_OF_MON));
    }
    if (Undead_warning)
        you_are("warned of undead", from_what(WARN_UNDEAD));
    if (Searching)
        you_have("automatic searching", from_what(SEARCHING));
    if (Clairvoyant) {
        you_are("clairvoyant", from_what(CLAIRVOYANT));
    } else if ((HClairvoyant || EClairvoyant) && BClairvoyant) {
        Strcpy(buf, from_what(-CLAIRVOYANT));
        (void) strsubst(buf, " because of ", " if not for ");
        enl_msg(You_, "could be", "could have been", " clairvoyant", buf);
    }
    if (Infravision)
        you_have("infravision", from_what(INFRAVISION));
    if (Detect_monsters) {
        Strcpy(buf, "sensing the presence of monsters");
        if (wizard) {
            long detectmon_timeout = (HDetect_monsters & TIMEOUT);

            if (detectmon_timeout)
                Sprintf(eos(buf), " (%ld)", detectmon_timeout);
        }
        you_are(buf, "");
    }
    if (u.umconf) { /* 'u.umconf' is a counter rather than a timeout */
        Strcpy(buf, " monsters when hitting them");
        if (wizard && !final) {
            if (u.umconf == 1)
                Strcat(buf, " (next hit only)");
            else /* u.umconf > 1 */
                Sprintf(eos(buf), " (next %u hits)", u.umconf);
        }
        enl_msg(You_, "will confuse", "would have confused", buf, "");
    }

    /*** Appearance and behavior ***/
    if (Adornment) {
        int adorn = 0;

        if (uleft && uleft->otyp == RIN_ADORNMENT)
            adorn += uleft->spe;
        if (uright && uright->otyp == RIN_ADORNMENT)
            adorn += uright->spe;
        /* the sum might be 0 (+0 ring or two which negate each other);
           that yields "you are charismatic" (which isn't pointless
           because it potentially impacts seduction attacks) */
        Sprintf(buf, "%scharismatic",
                (adorn > 0) ? "more " : (adorn < 0) ? "less " : "");
        you_are(buf, from_what(ADORNED));
    }
    if (Invisible)
        you_are("invisible", from_what(INVIS));
    else if (Invis)
        you_are("invisible to others", from_what(INVIS));
    /* ordinarily "visible" is redundant; this is a special case for
       the situation when invisibility would be an expected attribute */
    else if ((HInvis || EInvis) && BInvis)
        you_are("visible", from_what(-INVIS));
    if (Displaced)
        you_are("displaced", from_what(DISPLACED));
    if (Stealth) {
        you_are("stealthy", from_what(STEALTH));
    } else if (BStealth && (HStealth || EStealth)) {
        Sprintf(buf, " stealthy%s",
                (BStealth == FROMOUTSIDE) ? " if not mounted" : "");
        enl_msg(You_, "would be", "would have been", buf, "");
    }
    if (Aggravate_monster)
        enl_msg(You_, "aggravate", "aggravated", " monsters",
                from_what(AGGRAVATE_MONSTER));
    if (Conflict)
        enl_msg(You_, "cause", "caused", " conflict", from_what(CONFLICT));

    /*** Transportation ***/
    if (Jumping)
        you_can("jump", from_what(JUMPING));
    if (Teleportation)
        you_can("teleport", from_what(TELEPORT));
    if (Teleport_control)
        you_have("teleport control", from_what(TELEPORT_CONTROL));
    /* actively levitating handled earlier as a status condition */
    if (BLevitation) { /* levitation is blocked */
        long save_BLev = BLevitation;

        BLevitation = 0L;
        if (Levitation) {
            /* either trapped in the floor or inside solid rock
               (or both if chained to buried iron ball and have
               moved one step into solid rock somehow) */
            boolean trapped = (save_BLev & I_SPECIAL) != 0L,
                    terrain = (save_BLev & FROMOUTSIDE) != 0L;

            Sprintf(buf, "%s%s%s",
                    trapped ? " if not trapped" : "",
                    (trapped && terrain) ? " and" : "",
                    terrain ? if_surroundings_permitted : "");
            enl_msg(You_, "would levitate", "would have levitated", buf, "");
        }
        BLevitation = save_BLev;
    }
    /* actively flying handled earlier as a status condition */
    if (BFlying) { /* flight is blocked */
        long save_BFly = BFlying;

        BFlying = 0L;
        if (Flying) {
            enl_msg(You_, "would fly", "would have flown",
                    /* wording quibble: for past tense, "hadn't been"
                       would sound better than "weren't" (and
                       "had permitted" better than "permitted"), but
                       "weren't" and "permitted" are adequate so the
                       extra complexity to handle that isn't worth it */
                    Levitation
                       ? " if you weren't levitating"
                       : (save_BFly == I_SPECIAL)
                          /* this is an oversimplification; being trapped
                             might also be blocking levitation so flight
                             would still be blocked after escaping trap */
                          ? " if you weren't trapped"
                          : (save_BFly == FROMOUTSIDE)
                             ? if_surroundings_permitted
                             /* two or more of levitation, surroundings,
                                and being trapped in the floor */
                             : " if circumstances permitted",
                    "");
        }
        BFlying = save_BFly;
    }
    /* including this might bring attention to the fact that ceiling
       clinging has inconsistencies... */
    if (is_clinger(gy.youmonst.data)) {
        boolean has_lid = has_ceiling(&u.uz);

        if (has_lid && !u.uinwater) {
            you_can("cling to the ceiling", "");
        } else {
            Sprintf(buf, " to the ceiling if %s%s%s",
                    !has_lid ? "there was one" : "",
                    (!has_lid && u.uinwater) ? " and " : "",
                    u.uinwater ? (Underwater ? "you weren't underwater"
                                  : "you weren't in the water") : "");
            /* past tense is applicable for death while Unchanging */
            enl_msg(You_, "could cling", "could have clung", buf, "");
        }
    }
    /* actively walking on water handled earlier as a status condition */
    if (Wwalking && !walking_on_water())
        you_can("walk on water", from_what(WWALKING));
    /* actively swimming (in water but not under it) handled earlier */
    if (Swimming && (Underwater || !u.uinwater))
        you_can("swim", from_what(SWIMMING));
    if (Breathless)
        you_can("survive without air", from_what(MAGICAL_BREATHING));
    else if (Amphibious)
        you_can("breathe water", from_what(MAGICAL_BREATHING));
    if (Passes_walls)
        you_can("walk through walls", from_what(PASSES_WALLS));

    /*** Physical attributes ***/
    if (Regeneration)
        enl_msg(You_, "regenerate", "regenerated", "",
                from_what(REGENERATION));
    if (Slow_digestion)
        you_have("slower digestion", from_what(SLOW_DIGESTION));
    if (u.uhitinc) {
        (void) enlght_combatinc("to hit", u.uhitinc, final, buf);
        if (iflags.tux_penalty && !Upolyd)
            Sprintf(eos(buf), " %s your suit's penalty",
                    (u.uhitinc < 0) ? "increasing"
                    : (u.uhitinc < 4 * gu.urole.spelarmr / 5)
                      ? "partly offsetting"
                      : (u.uhitinc < gu.urole.spelarmr) ? "nearly offsetting"
                        : "overcoming");
        you_have(buf, "");
    }
    if (u.udaminc)
        you_have(enlght_combatinc("damage", u.udaminc, final, buf), "");
    if (u.uspellprot || Protection) {
        int prot = 0;

        if (uleft && uleft->otyp == RIN_PROTECTION)
            prot += uleft->spe;
        if (uright && uright->otyp == RIN_PROTECTION)
            prot += uright->spe;
        if (uamul && uamul->otyp == AMULET_OF_GUARDING)
            prot += 2;
        if (HProtection & INTRINSIC)
            prot += u.ublessed;
        prot += u.uspellprot;
        if (prot)
            you_have(enlght_combatinc("defense", prot, final, buf), "");
    }
    if ((armpro = magic_negation(&gy.youmonst)) > 0) {
        /* magic cancellation factor, conferred by worn armor */
        static const char *const mc_types[] = {
            "" /*ordinary*/, "warded", "guarded", "protected",
        };
        /* sanity check */
        if (armpro >= SIZE(mc_types))
            armpro = SIZE(mc_types) - 1;
        you_are(mc_types[armpro], "");
    }
    if (Half_physical_damage)
        enlght_halfdmg(HALF_PHDAM, final);
    if (Half_spell_damage)
        enlght_halfdmg(HALF_SPDAM, final);
    if (Half_gas_damage)
        enl_msg(You_, "take", "took", " reduced poison gas damage", "");
    if (spellid(0) > NO_SPELL) { /* skip if no spells are known yet */
        /* greatly simplified edition of percent_success(spell.c)--may need
           to be suppressed if oversimplification leads to player confusion */
        char cast_adj[QBUFSZ];
        boolean suit = uarm && is_metallic(uarm),
                robe = uarmc && uarmc->otyp == ROBE;

        *cast_adj = '\0';
        if (suit) /* omit "wearing" to shorten the text */
            Sprintf(cast_adj, " impaired by metallic armor%s",
                    robe ? ", mitigated by your robe" : "");
        else if (robe)
            Strcpy(cast_adj, " enhanced by wearing a robe");

        if (*cast_adj)
            enl_msg("Your spell casting ", "is", "was", cast_adj, "");
    }
    /* polymorph and other shape change */
    if (Protection_from_shape_changers)
        you_are("protected from shape changers",
                from_what(PROT_FROM_SHAPE_CHANGERS));
    if (Unchanging) {
        const char *what = 0;

        if (!Upolyd) /* Upolyd handled below after current form */
            you_can("not change from your current form",
                    from_what(UNCHANGING));
        /* blocked shape changes */
        if (Polymorph)
            what = !final ? "polymorph" : "have polymorphed";
        else if (ismnum(u.ulycn))
            what = !final ? "change shape" : "have changed shape";
        if (what) {
            Sprintf(buf, "would %s periodically", what);
            /* omit from_what(UNCHANGING); too verbose */
            enl_msg(You_, buf, buf, " if not locked into your current form",
                    "");
        }
    } else if (Polymorph) {
        you_are("polymorphing periodically", from_what(POLYMORPH));
    }
    if (Polymorph_control)
        you_have("polymorph control", from_what(POLYMORPH_CONTROL));
    if (Upolyd && u.umonnum != u.ulycn
        /* if we've died from turning into slime, we're polymorphed
           right now but don't want to list it as a temporary attribute
           [we need a more reliable way to detect this situation] */
        && !(final == ENL_GAMEOVERDEAD
             && u.umonnum == PM_GREEN_SLIME && !Unchanging)) {
        /* foreign shape (except were-form which is handled below) */
        if (!vampshifted(&gy.youmonst))
            Sprintf(buf, "polymorphed into %s",
                    an(pmname(gy.youmonst.data,
                              flags.female ? FEMALE : MALE)));
        else
            Sprintf(buf, "polymorphed into %s in %s form",
                    an(pmname(&mons[gy.youmonst.cham],
                              flags.female ? FEMALE : MALE)),
                    pmname(gy.youmonst.data, flags.female ? FEMALE : MALE));
        if (wizard)
            Sprintf(eos(buf), " (%d)", u.mtimedone);
        you_are(buf, "");
    }
    if (lays_eggs(gy.youmonst.data) && flags.female) /* Upolyd */
        you_can("lay eggs", "");
    if (ismnum(u.ulycn)) {
        /* "you are a werecreature [in beast form]" */
        Strcpy(buf, an(pmname(&mons[u.ulycn],
               flags.female ? FEMALE : MALE)));
        if (u.umonnum == u.ulycn) {
            Strcat(buf, " in beast form");
            if (wizard)
                Sprintf(eos(buf), " (%d)", u.mtimedone);
        }
        you_are(buf, "");
    }
    if (Unchanging && Upolyd) /* !Upolyd handled above */
        you_can("not change from your current form", from_what(UNCHANGING));
    if (Hate_silver)
        you_are("harmed by silver", "");
    /* movement and non-armor-based protection */
    if (Fast)
        you_are(Very_fast ? "very fast" : "fast", from_what(FAST));
    if (Reflecting)
        you_have("reflection", from_what(REFLECTING));
    if (Free_action)
        you_have("free action", from_what(FREE_ACTION));
    if (Fixed_abil)
        you_have("fixed abilities", from_what(FIXED_ABIL));
    if (Lifesaved)
        enl_msg("Your life ", "will be", "would have been", " saved", "");

    /*** Miscellany ***/
    if (Luck) {
        ltmp = abs((int) Luck);
        Sprintf(buf, "%s%slucky",
                ltmp >= 10 ? "extremely " : ltmp >= 5 ? "very " : "",
                Luck < 0 ? "un" : "");
        if (wizard)
            Sprintf(eos(buf), " (%d)", Luck);
        you_are(buf, "");
    } else if (wizard)
        enl_msg("Your luck ", "is", "was", " zero", "");
    if (u.moreluck > 0)
        you_have("extra luck", "");
    else if (u.moreluck < 0)
        you_have("reduced luck", "");
    if (carrying(LUCKSTONE) || stone_luck(TRUE)) {
        ltmp = stone_luck(FALSE);
        if (ltmp <= 0)
            enl_msg("Bad luck ", "does not time out", "did not time out",
                    " for you", "");
        if (ltmp >= 0)
            enl_msg("Good luck ", "does not time out", "did not time out",
                    " for you", "");
    }

    if (u.ugangr) {
        Sprintf(buf, " %sangry with you",
                u.ugangr > 6 ? "extremely " : u.ugangr > 3 ? "very " : "");
        if (wizard)
            Sprintf(eos(buf), " (%d)", u.ugangr);
        enl_msg(u_gname(), " is", " was", buf, "");
    } else {
        /*
         * We need to suppress this when the game is over, because death
         * can change the value calculated by can_pray(), potentially
         * resulting in a false claim that you could have prayed safely.
         */
        if (!final) {
#if 0
            /* "can [not] safely pray" vs "could [not] have safely prayed" */
            Sprintf(buf, "%s%ssafely pray%s", can_pray(FALSE) ? "" : "not ",
                    final ? "have " : "", final ? "ed" : "");
#else
            Sprintf(buf, "%ssafely pray", can_pray(FALSE) ? "" : "not ");
#endif
            if (wizard)
                Sprintf(eos(buf), " (%d)", u.ublesscnt);
            you_can(buf, "");
        }
    }

#ifdef DEBUG
    /* named fruit debugging (doesn't really belong here...); to enable,
       include 'fruit' in DEBUGFILES list (even though it isn't a file...) */
    if (wizard && explicitdebug("fruit")) {
        struct fruit *f;

        reorder_fruit(TRUE); /* sort by fruit index, from low to high;
                              * this modifies the gf.ffruit chain, so could
                              * possibly mask or even introduce a problem,
                              * but it does useful sanity checking */
        for (f = gf.ffruit; f; f = f->nextf) {
            Sprintf(buf, "Fruit #%d ", f->fid);
            enl_msg(buf, "is ", "was ", f->fname, "");
        }
        enl_msg("The current fruit ", "is ", "was ", svp.pl_fruit, "");
        Sprintf(buf, "%d", flags.made_fruit);
        enl_msg("The made fruit flag ", "is ", "was ", buf, "");
    }
#endif

    {
        const char *p;

        buf[0] = '\0';
        if (final < 2) { /* still in progress, or quit/escaped/ascended */
            p = "survived after being killed ";
            if (!u.umortality)
                p = !final ? (char *) 0 : "survived";
            else
                (void) N_times((long) u.umortality, buf);
        } else { /* game ended in character's death */
            p = "are dead";
            switch (u.umortality) {
            case 0:
                impossible("dead without dying?");
                FALLTHROUGH;
                /* FALLTHRU */
            case 1:
                break; /* just "are dead" */
            default:
                Sprintf(buf, " (%d%s time!)", u.umortality,
                        ordin(u.umortality));
                break;
            }
        }
        if (p)
            enl_msg(You_, "have been killed ", p, buf, "");
    }
}

/* ^X command */
int
doattributes(void)
{
    int mode = BASICENLIGHTENMENT;

    /* show more--as if final disclosure--for wizard and explore modes */
    if (wizard || discover)
        mode |= MAGICENLIGHTENMENT;

    enlightenment(mode, ENL_GAMEINPROGRESS);
    return ECMD_OK;
}

void
youhiding(boolean via_enlghtmt, /* enlightenment line vs topl message */
          int msgflag)          /* for variant message phrasing */
{
    char *bp, buf[BUFSZ];

    Strcpy(buf, "hiding");
    if (U_AP_TYPE != M_AP_NOTHING) {
        /* mimic; hero is only able to mimic a strange object or gold
           or hallucinatory alternative to gold, so we skip the details
           for the hypothetical furniture and monster cases */
        bp = eos(strcpy(buf, "mimicking"));
        if (U_AP_TYPE == M_AP_OBJECT) {
            Sprintf(bp, " %s", an(simple_typename(gy.youmonst.mappearance)));
        } else if (U_AP_TYPE == M_AP_FURNITURE) {
            Strcpy(bp, " something");
        } else if (U_AP_TYPE == M_AP_MONSTER) {
            Strcpy(bp, " someone");
        } else {
            ; /* something unexpected; leave 'buf' as-is */
        }
    } else if (u.uundetected) {
        bp = eos(buf); /* points past "hiding" */
        if (gy.youmonst.data->mlet == S_EEL) {
            if (is_pool(u.ux, u.uy))
                Sprintf(bp, " in the %s", waterbody_name(u.ux, u.uy));
        } else if (hides_under(gy.youmonst.data)) {
            struct obj *o = svl.level.objects[u.ux][u.uy];

            if (o)
                Sprintf(bp, " underneath %s", ansimpleoname(o));
        } else if (is_clinger(gy.youmonst.data) || Flying) {
            /* Flying: 'lurker above' hides on ceiling but doesn't cling */
            Sprintf(bp, " on the %s", ceiling(u.ux, u.uy));
        } else {
            /* on floor; is_hider() but otherwise not special: 'trapper' */
            if (u.utrap && u.utraptype == TT_PIT) {
                struct trap *t = t_at(u.ux, u.uy);

                Sprintf(bp, " in a %spit",
                        (t && t->ttyp == SPIKED_PIT) ? "spiked " : "");
            } else
                Sprintf(bp, " on the %s", surface(u.ux, u.uy));
        }
    } else {
        ; /* shouldn't happen; will result in generic "you are hiding" */
    }

    if (via_enlghtmt) {
        int final = msgflag; /* 'final' is used by you_are() macro */

        you_are(buf, "");
    } else {
        /* for dohide(), when player uses '#monster' command */
        You("are %s %s.", msgflag ? "already" : "now", buf);
    }
}

/* #conduct command [KMH]; shares enlightenment's tense handling */
int
doconduct(void)
{
    show_conduct(ENL_GAMEINPROGRESS);
    return ECMD_OK;
}

/* display conducts; for doconduct(), also disclose() and dump_everything() */
#if 0
/* whole conduct sentences, translated by enlght_i18n() */
NC_("heroine", "You have been deaf from birth.")
NC_("heroine", "You were deaf from birth.")
NC_("heroine", "You have been vegetarian.")
NC_("heroine", "You were vegetarian.")
NC_("heroine", "You have been illiterate.")
NC_("heroine", "You were illiterate.")
NC_("heroine", "You have gone without food.")
NC_("heroine", "You have gone without any armor.")
N_("You have been blind from birth.")
N_("You were blind from birth.")
N_("You have been deaf from birth.")
N_("You were deaf from birth.")
N_("You have been faithfully nudist.")
N_("You were faithfully nudist.")
N_("You have been vegetarian.")
N_("You were vegetarian.")
N_("You have been an atheist.")
N_("You were an atheist.")
N_("You have been a pacifist.")
N_("You were a pacifist.")
N_("You have been illiterate.")
N_("You were illiterate.")
N_("You have never hit with a wielded weapon.")
N_("You never hit with a wielded weapon.")
N_("You have never had a pet.")
N_("You never had a pet.")
N_("You have never genocided any monsters.")
N_("You never genocided any monsters.")
N_("You have never polymorphed an object.")
N_("You never polymorphed an object.")
N_("You have never changed form.")
N_("You never changed form.")
N_("You have followed a strict vegan diet.")
N_("You followed a strict vegan diet.")
N_("You have used no wishes.")
N_("You used no wishes.")
N_("You started without possessions.")
N_("You are without possessions.")
N_("You started out without possessions.")
N_("You have gone without food.")
N_("You went without food.")
N_("You haven't wished for any artifacts.")
N_("You did not wish for any artifacts.")
N_("You haven't violated any of the special Sokoban rules.")
N_("You did not violate any of the special Sokoban rules.")
N_("You have genocided %ld type of monster.")
N_("You genocided %ld type of monster.")
N_("You have genocided %ld types of monsters.")
N_("You genocided %ld types of monsters.")
N_("You have used %ld wish.")
N_("You used %ld wish.")
N_("You have used %ld wishes.")
N_("You used %ld wishes.")
N_("You have used %ld wish (for an artifact).")
N_("You used %ld wish (for an artifact).")
N_("You have used %ld wishes (both for artifacts).")
N_("You used %ld wishes (both for artifacts).")
N_("You have used %ld wishes (all for artifacts).")
N_("You used %ld wishes (all for artifacts).")
#endif

void
show_conduct(int final)
{
    char buf[BUFSZ], bufN[40];
    int ngenocided;

    /* Create the conduct window */
    ge.en_win = create_nhwindow(NHW_MENU);
    putstr(ge.en_win, 0, _("Voluntary challenges:"));

    /* rerolling; "You <this or that>" is about the character, rerolling
       is about the player so phrase it differently;
       also, always use past tense since the chance to do something with it
       is gone by time player can issue #conduct command or see disclosure */
    if (!u.uroleplay.reroll)
        Strcpy(buf, _(" Character rerolling was not enabled."));
    else if (!u.uroleplay.numrerolls)
        Strcpy(buf, _(" Your character was not rerolled."));
    else if (i18n_active())
        Snprintf(buf, sizeof buf,
                 _(" Your character was rerolled %ld time(s)."),
                 (long) u.uroleplay.numrerolls);
    else
        Sprintf(buf, " Your character was rerolled %s.",
                N_times(u.uroleplay.numrerolls, bufN));
    enlght_out(buf);

    if (u.uroleplay.blind)
        you_have_been("blind from birth");
    if (u.uroleplay.deaf)
        you_have_been("deaf from birth");
    /* note: we don't report "you are without possessions" unless the
       game started with the pauper option set */
    if (u.uroleplay.pauper)
        enl_msg(You_, gi.invent ? "started" : "are", "started out",
                " without possessions", "");
    /* nudist is far more than a subset of possessionless, and a much
       more impressive accomplishment, but showing "started out without
       possessions" before "faithfully nudist" looks more logical */
    if (u.uroleplay.nudist)
        you_have_been("faithfully nudist");

    if (!u.uconduct.food)
        enl_msg(You_, "have gone", "went", " without food", "");
        /* but beverages are okay */
    else if (!u.uconduct.unvegan)
        you_have_X("followed a strict vegan diet");
    else if (!u.uconduct.unvegetarian)
        you_have_been("vegetarian");

    if (!u.uconduct.gnostic)
        you_have_been("an atheist");

    if (!u.uconduct.weaphit) {
        you_have_never("hit with a wielded weapon");
    } else if (wizard) {
        Sprintf(buf, "hit with a wielded weapon %ld time%s",
                u.uconduct.weaphit, plur(u.uconduct.weaphit));
        you_have_X(buf);
    }
    if (!u.uconduct.killer)
        you_have_been("a pacifist");

    if (!u.uconduct.literate) {
        you_have_been("illiterate");
    } else if (wizard) {
        Sprintf(buf, "read items or engraved %ld time%s", u.uconduct.literate,
                plur(u.uconduct.literate));
        you_have_X(buf);
    }

    if (!u.uconduct.pets)
        you_have_never("had a pet");

    ngenocided = num_genocides();
    if (ngenocided == 0) {
        you_have_never("genocided any monsters");
    } else {
        Sprintf(buf, "genocided %d type%s of monster%s", ngenocided,
                plur(ngenocided), plur(ngenocided));
        you_have_X(buf);
    }

    if (!u.uconduct.polypiles) {
        you_have_never("polymorphed an object");
    } else if (wizard) {
        Sprintf(buf, "polymorphed %ld item%s", u.uconduct.polypiles,
                plur(u.uconduct.polypiles));
        you_have_X(buf);
    }

    if (!u.uconduct.polyselfs) {
        you_have_never("changed form");
    } else if (wizard) {
        Sprintf(buf, "changed form %ld time%s", u.uconduct.polyselfs,
                plur(u.uconduct.polyselfs));
        you_have_X(buf);
    }

    if (!u.uconduct.wishes) {
        you_have_X("used no wishes");
    } else {
        Sprintf(buf, "used %ld wish%s", u.uconduct.wishes,
                (u.uconduct.wishes > 1L) ? "es" : "");
        if (u.uconduct.wisharti) {
            /* if wisharti == wishes
             *  1 wish (for an artifact)
             *  2 wishes (both for artifacts)
             *  N wishes (all for artifacts)
             * else (N is at least 2 in order to get here; M < N)
             *  N wishes (1 for an artifact)
             *  N wishes (M for artifacts)
             */
            if (u.uconduct.wisharti == u.uconduct.wishes)
                Sprintf(eos(buf), " (%s",
                        (u.uconduct.wisharti > 2L) ? "all "
                          : (u.uconduct.wisharti == 2L) ? "both " : "");
            else
                Sprintf(eos(buf), " (%ld ", u.uconduct.wisharti);

            Sprintf(eos(buf), "for %s)",
                    (u.uconduct.wisharti == 1L) ? "an artifact"
                                                : "artifacts");
        }
        you_have_X(buf);

        if (!u.uconduct.wisharti)
            enl_msg(You_, "have not wished", "did not wish",
                    " for any artifacts", "");
    }

    /* only report Sokoban conduct if the Sokoban branch has been entered */
    if (sokoban_in_play()) {
        const char *presentverb = "have violated", *pastverb = "violated";

        if (!u.uconduct.sokocheat) {
            presentverb = "have not violated";
            pastverb = "did not violate";
            Strcpy(buf, " any of the special Sokoban rules");
        } else {
            Strcpy(buf, " the special Sokoban rules ");
            Strcat(buf, N_times(u.uconduct.sokocheat, bufN));
        }
        enl_msg(You_, presentverb, pastverb, buf, "");
    }

    show_achievements(final);

    /* Pop up the window and wait for a key */
    display_nhwindow(ge.en_win, TRUE);
    destroy_nhwindow(ge.en_win);
    ge.en_win = WIN_ERR;
}

/*
 *      Achievements (see 'enum achievements' in you.h).
 */

#if 0
/* whole achievement sentences, translated by enlght_i18n() */
N_("You have entered the Gnomish Mines.")
N_("You entered the Gnomish Mines.")
N_("You have entered Minetown.")
N_("You entered Minetown.")
N_("You have entered a shop.")
N_("You entered a shop.")
N_("You have entered a temple.")
N_("You entered a temple.")
N_("You have consulted the Oracle of Delphi.")
N_("You consulted the Oracle of Delphi.")
N_("You have read from a Discworld novel.")
N_("You read from a Discworld novel.")
N_("You have entered Sokoban.")
N_("You entered Sokoban.")
N_("You have completed Sokoban.")
N_("You completed Sokoban.")
N_("You have completed the Gnomish Mines.")
N_("You completed the Gnomish Mines.")
N_("You have entered the Big Room.")
N_("You entered the Big Room.")
N_("You have defeated Medusa.")
N_("You defeated Medusa.")
N_("You have learned the tune to open and close the Castle's drawbridge.")
N_("You learned the tune to open and close the Castle's drawbridge.")
N_("You have gained access to Moloch's Sanctum.")
N_("You gained access to Moloch's Sanctum.")
N_("You have reached the Elemental Planes.")
N_("You reached the Elemental Planes.")
N_("You have reached the Astral Plane.")
N_("You reached the Astral Plane.")
N_("You have entered Gehennom.")
N_("You entered Gehennom.")
N_("You are exploring without being able to see.")
N_("You explored without being able to see.")
N_("You have gone without any armor.")
N_("You went without any armor.")
N_("You have the Bell of Opening.")
N_("You had the Bell of Opening.")
N_("You have handled the Bell of Opening.")
N_("You handled the Bell of Opening.")
N_("You have the Candelabrum of Invocation.")
N_("You had the Candelabrum of Invocation.")
N_("You have handled the Candelabrum of Invocation.")
N_("You handled the Candelabrum of Invocation.")
N_("You have the Book of the Dead.")
N_("You had the Book of the Dead.")
N_("You have handled the Book of the Dead.")
N_("You handled the Book of the Dead.")
N_("You have the Amulet of Yendor.")
N_("You have obtained the Amulet of Yendor.")
N_("You delivered the Amulet of Yendor.")
N_("You had the Amulet of Yendor.")
N_("You had obtained the Amulet of Yendor.")
N_("You ascended!")
#endif

staticfn void
show_achievements(
    int final) /* 'final' is used "behind the curtain" by enl_foo() macros */
{
    int i, achidx, absidx, acnt;
    char title[QBUFSZ], buf[QBUFSZ];
    winid awin = WIN_ERR;

    /* unfortunately we can't show the achievements (at least not all of
       them) while the game is in progress because it would give away the
       ID of luckstone (at Mine's End) and of real Amulet of Yendor */
    if (!final && !wizard)
        return;

    /* first, figure whether any achievements have been accomplished
       so that we don't show the header for them if the resulting list
       below it would be empty */
    if ((acnt = count_achievements()) == 0)
        return;

    if (ge.en_win != WIN_ERR) {
        awin = ge.en_win; /* end of game disclosure window */
        putstr(awin, 0, "");
    } else {
        awin = create_nhwindow(NHW_MENU);
    }
    Strcpy(title, (acnt == 1) ? _("Achievement:") : _("Achievements:"));
    putstr(awin, 0, title);

    /* display achievements in the order in which they were recorded;
       lone exception is to defer the Amulet if we just ascended;
       it warrants alternate wording when given away during ascension,
       but the Amulet achievement is always attained before entering
       endgame and the alternate wording looks strange if shown before
       "reached endgame" and "reached Astral" */
    if (remove_achievement(ACH_UWIN)) { /* UWIN == Ascended! */
        /* for ascension, force it to be last and Amulet next to last
           by taking them out and then adding them back */
        if (remove_achievement(ACH_AMUL)) /* should always be True here */
            record_achievement(ACH_AMUL);
        record_achievement(ACH_UWIN);
    }
    for (i = 0; i < acnt; ++i) {
        achidx = u.uachieved[i];
        absidx = abs(achidx);

        switch (absidx) {
        case ACH_BLND:
            enl_msg(You_, "are exploring", "explored",
                    " without being able to see", "");
            break;
        case ACH_NUDE:
            enl_msg(You_, "have gone", "went", " without any armor", "");
            break;
        case ACH_MINE:
            you_have_X("entered the Gnomish Mines");
            break;
        case ACH_TOWN:
            you_have_X("entered Minetown");
            break;
        case ACH_SHOP:
            you_have_X("entered a shop");
            break;
        case ACH_TMPL:
            you_have_X("entered a temple");
            break;
        case ACH_ORCL:
            you_have_X("consulted the Oracle of Delphi");
            break;
        case ACH_NOVL:
            you_have_X("read from a Discworld novel");
            break;
        case ACH_SOKO:
            you_have_X("entered Sokoban");
            break;
        case ACH_SOKO_PRIZE: /* hard to reach guaranteed bag or amulet */
            you_have_X("completed Sokoban");
            break;
        case ACH_MINE_PRIZE: /* hidden guaranteed luckstone */
            you_have_X("completed the Gnomish Mines");
            break;
        case ACH_BGRM:
            you_have_X("entered the Big Room");
            break;
        case ACH_MEDU:
            you_have_X("defeated Medusa");
            break;
        case ACH_TUNE:
            you_have_X(
                "learned the tune to open and close the Castle's drawbridge");
            break;
        case ACH_BELL:
            /* alternate phrasing for present vs past and also for
               possessing the item vs once held it */
            enl_msg(You_,
                    u.uhave.bell ? "have" : "have handled",
                    u.uhave.bell ? "had" : "handled",
                    " the Bell of Opening", "");
            break;
        case ACH_HELL:
            enl_msg(You_, "have ", "", "entered Gehennom", "");
            break;
        case ACH_CNDL:
            enl_msg(You_,
                    u.uhave.menorah ? "have" : "have handled",
                    u.uhave.menorah ? "had" : "handled",
                    " the Candelabrum of Invocation", "");
            break;
        case ACH_BOOK:
            enl_msg(You_,
                    u.uhave.book ? "have" : "have handled",
                    u.uhave.book ? "had" : "handled",
                    " the Book of the Dead", "");
            break;
        case ACH_INVK:
            you_have_X("gained access to Moloch's Sanctum");
            break;
        case ACH_AMUL:
            /* alternate wording for ascended (always past tense) since
               hero had it until #offer forced it to be relinquished */
            enl_msg(You_,
                    u.uhave.amulet ? "have" : "have obtained",
                    u.uevent.ascended ? "delivered"
                     : u.uhave.amulet ? "had" : "had obtained",
                    " the Amulet of Yendor", "");
            break;

        /* reaching Astral makes feedback about reaching the Planes
           be redundant and ascending makes both be redundant, but
           we display all that apply */
        case ACH_ENDG:
            you_have_X("reached the Elemental Planes");
            break;
        case ACH_ASTR:
            you_have_X("reached the Astral Plane");
            break;
        case ACH_UWIN:
            /* the ultimate achievement... */
            enlght_out(_(" You ascended!"));
            break;

        /* rank 0 is the starting condition, not an achievement; 8 is Xp 30 */
        case ACH_RNK1: case ACH_RNK2: case ACH_RNK3: case ACH_RNK4:
        case ACH_RNK5: case ACH_RNK6: case ACH_RNK7: case ACH_RNK8:
            if (i18n_active()) {
                Snprintf(buf, sizeof buf,
                         final ? _(" You attained the rank of %s.")
                               : _(" You have attained the rank of %s."),
                         _(rank_of(rank_to_xlev(absidx - (ACH_RNK1 - 1)),
                                   Role_switch,
                                   (achidx < 0) ? TRUE : FALSE)));
                enlght_out(buf);
                break;
            }
            Sprintf(buf, "attained the rank of %s",
                    rank_of(rank_to_xlev(absidx - (ACH_RNK1 - 1)),
                            Role_switch, (achidx < 0) ? TRUE : FALSE));
            you_have_X(buf);
            break;

        default:
            Sprintf(buf, " [Unexpected achievement #%d.]", achidx);
            enlght_out(buf);
            break;
        } /* switch */
    } /* for */

    if (awin != ge.en_win) {
        display_nhwindow(awin, TRUE);
        destroy_nhwindow(awin);
    }
}

/* record an achievement (add at end of list unless already present) */
void
record_achievement(schar achidx)
{
    int i, absidx;
    int repeat_achievement = 0;

    absidx = abs(achidx);
    /* valid achievements range from 1 to N_ACH-1; however, ranks can be
       stored as the complement (ie, negative) to track gender */
    if ((achidx < 1 && (absidx < ACH_RNK1 || absidx > ACH_RNK8))
        || achidx >= N_ACH) {
        impossible("Achievement #%d is out of range.", achidx);
        return;
    }

    /* the list has an extra slot so there is always at least one 0 at
       its end (more than one unless all N_ACH-1 possible achievements
       have been recorded); find first empty slot or achievement #achidx;
       an attempt to duplicate an achievement can happen if any of Bell,
       Candelabrum, Book, or Amulet is dropped then picked up again */
    for (i = 0; u.uachieved[i]; ++i)
        if (abs(u.uachieved[i]) == absidx) {
            repeat_achievement = 1;
            break;
        }

    /*
     * We do the sound for an achievement, even if it has already been
     * achieved before. Some players might have set up level-based
     * theme music or something. We do let the sound interface know
     * that it's not the original achievement though.
     */
    SoundAchievement(achidx, 0, repeat_achievement);

    if (repeat_achievement)
        return; /* already recorded, don't duplicate it */
    u.uachieved[i] = achidx;

    /* avoid livelog for achievements recorded during final disclosure:
       nudist and blind-from-birth; also ascension which is suppressed
       by this gets logged separately in really_done() */
    if (program_state.gameover)
        return;

    if (absidx >= ACH_RNK1 && absidx <= ACH_RNK8) {
        livelog_printf(achieve_msg[absidx].llflag,
                       "attained the rank of %s (level %d)",
                       rank_of(rank_to_xlev(absidx - (ACH_RNK1 - 1)),
                               Role_switch, (achidx < 0) ? TRUE : FALSE),
                       u.ulevel);
    } else if (achidx == ACH_SOKO_PRIZE
               || achidx == ACH_MINE_PRIZE) {
        /* need to supply extra information for these two */
        short otyp = ((achidx == ACH_SOKO_PRIZE)
                      ? svc.context.achieveo.soko_prize_otyp
                      : svc.context.achieveo.mines_prize_otyp);

        /* note: OBJ_NAME() works here because both "bag of holding" and
           "amulet of reflection" are fully named in their objects[] entry
           but that's not true in the general case */
        livelog_printf(achieve_msg[achidx].llflag, "%s %s",
                       achieve_msg[achidx].msg, OBJ_NAME(objects[otyp]));
    } else {
        livelog_printf(achieve_msg[absidx].llflag, "%s",
                       achieve_msg[absidx].msg);
    }
}

/* discard a recorded achievement; return True if removed, False otherwise */
boolean
remove_achievement(schar achidx)
{
    int i;

    for (i = 0; u.uachieved[i]; ++i)
        if (abs(u.uachieved[i]) == abs(achidx))
            break; /* stop when found */
    if (!u.uachieved[i]) /* not found */
        return FALSE;
    /* list is 0 terminated so any beyond the removed one move up a slot */
    do {
        u.uachieved[i] = u.uachieved[i + 1];
    } while (u.uachieved[++i]);
    return TRUE;
}

/* used to decide whether there are any achievements to display */
int
count_achievements(void)
{
    int i, acnt = 0;

    for (i = 0; u.uachieved[i]; ++i)
        ++acnt;
    return acnt;
}

/* convert a rank index to an achievement number; encode it when female
   in order to subsequently report gender-specific ranks accurately */
schar
achieve_rank(int rank) /* 1..8 */
{
    schar achidx = (schar) ((rank - 1) + ACH_RNK1);

    if (flags.female)
        achidx = -achidx;
    return achidx;
}

/* return True if sokoban branch has been entered, False otherwise */
boolean
sokoban_in_play(void)
{
    int achidx;

    /* TODO? move this to dungeon.c and test furthest level reached of the
       sokoban branch instead of relying on the entered-sokoban achievement */

    for (achidx = 0; u.uachieved[achidx]; ++achidx)
        if (u.uachieved[achidx] == ACH_SOKO)
            return TRUE;
    return FALSE;
}

/* #chronicle command */
int
do_gamelog(void)
{
#ifdef CHRONICLE
    if (gg.gamelog) {
        show_gamelog(ENL_GAMEINPROGRESS);
    } else {
        pline("No chronicled events.");
    }
#else
    pline("Chronicle was turned off during compile-time.");
#endif /* !CHRONICLE */
    return ECMD_OK;
}

/* 'major' events for dumplog; inclusion or exclusion here may need tuning */
#define LL_majors (0L \
                   | LL_WISH            \
                   | LL_ACHIEVE         \
                   | LL_UMONST          \
                   | LL_DIVINEGIFT      \
                   | LL_LIFESAVE        \
                   | LL_ARTIFACT        \
                   | LL_GENOCIDE        \
                   | LL_DUMP) /* explicitly for dumplog */
#define majorevent(llmsg) (((llmsg)->flags & LL_majors) != 0)
#define spoilerevent(llmsg) (((llmsg)->flags & LL_SPOILER) != 0)

/* #chronicle details */
void
show_gamelog(int final)
{
#ifdef CHRONICLE
    struct gamelog_line *llmsg;
    winid win;
    char buf[BUFSZ];
    int eventcnt = 0;

    win = create_nhwindow(NHW_TEXT);
    Sprintf(buf, "%s events:", final ? "Major" : "Logged");
    putstr(win, 0, buf);
    for (llmsg = gg.gamelog; llmsg; llmsg = llmsg->next) {
        if (final && !majorevent(llmsg))
            continue;
        if (!final && !wizard && spoilerevent(llmsg))
            continue;
        if (!eventcnt++)
            putstr(win, 0, " Turn");
        Snprintf(buf, sizeof buf, "%5ld: %s", llmsg->turn, llmsg->text);
        putstr(win, 0, buf);
    }
    /* since start of game is logged as a major event, 'eventcnt' should
       never end up as 0; for 'final', end of game is a major event too */
    if (!eventcnt)
        putstr(win, 0, " none");

    display_nhwindow(win, TRUE);
    destroy_nhwindow(win);
#else
    nhUse(final);
#endif /* !CHRONICLE */
    return;
}

/*
 *      Vanquished monsters.
 */

/* the two uppercase choices are implemented but suppressed from menu.
   also used in options.c */
const char *const vanqorders[NUM_VANQ_ORDER_MODES][3] = {
    { "t", N_("traditional: by monster level"),
           N_("traditional: by monster level, by internal monster index") },
    { "d", N_("by monster difficulty rating"),
           N_("by monster difficulty rating, by internal monster index") },
    { "a", N_("alphabetically, unique monsters separate"),
           N_("alphabetically, first unique monsters, then others") },
    { "A", N_("alphabetically, unique monsters intermixed"),
           N_("alphabetically, unique monsters and others intermixed") },
    { "C", N_("by monster class, high to low level in class"),
           N_("by monster class, high to low level within class") },
    { "c", N_("by monster class, low to high level in class"),
           N_("by monster class, low to high level within class") },
    { "n", N_("by count, high to low"),
           N_("by count, high to low, by internal index within tied count") },
    { "z", N_("by count, low to high"),
           N_("by count, low to high, by internal index within tied count") },
};

staticfn int QSORTCALLBACK
vanqsort_cmp(
    const genericptr vptr1,
    const genericptr vptr2)
{
    int indx1 = *(short *) vptr1, indx2 = *(short *) vptr2,
        mlev1, mlev2, mstr1, mstr2, uniq1, uniq2, died1, died2, res;
    const char *name1, *name2, *punct;
    schar mcls1, mcls2;

    switch (flags.vanq_sortmode) {
    default:
    case VANQ_MLVL_MNDX:
        /* sort by monster level */
        mlev1 = mons[indx1].mlevel;
        mlev2 = mons[indx2].mlevel;
        res = mlev2 - mlev1; /* mlevel high to low */
        break;
    case VANQ_MSTR_MNDX:
        /* sort by monster toughness */
        mstr1 = mons[indx1].difficulty;
        mstr2 = mons[indx2].difficulty;
        res = mstr2 - mstr1; /* monstr high to low */
        break;
    case VANQ_ALPHA_SEP:
        uniq1 = ((mons[indx1].geno & G_UNIQ) && indx1 != PM_HIGH_CLERIC);
        uniq2 = ((mons[indx2].geno & G_UNIQ) && indx2 != PM_HIGH_CLERIC);
        if (uniq1 ^ uniq2) { /* one or other uniq, but not both */
            res = uniq2 - uniq1;
            break;
        } /* else both unique or neither unique */
        FALLTHROUGH;
        /*FALLTHRU*/
    case VANQ_ALPHA_MIX:
        name1 = mons[indx1].pmnames[NEUTRAL];
        name2 = mons[indx2].pmnames[NEUTRAL];
        res = strcmpi(name1, name2); /* caseblind alpha, low to high */
        break;
    case VANQ_MCLS_HTOL:
    case VANQ_MCLS_LTOH:
        /* mons[].mlet is a small integer, 1..N, of type plain char;
           if 'char' happens to be unsigned, (mlet1 - mlet2) would yield
           an inappropriate result when mlet2 is greater than mlet1,
           so force our copies (mcls1, mcls2) to be signed */
        mcls1 = (schar) mons[indx1].mlet;
        mcls2 = (schar) mons[indx2].mlet;
        /* S_ANT through S_ZRUTY correspond to lowercase monster classes,
           S_ANGEL through S_ZOMBIE correspond to uppercase, and various
           punctuation characters are used for classes beyond those */
        if (mcls1 > S_ZOMBIE && mcls2 > S_ZOMBIE) {
            /* force a specific order to the punctuation classes that's
               different from the internal order;
               internal order is ok if neither or just one is punctuation
               since letters have lower values so come out before punct */
            static const char punctclasses[] = {
                S_LIZARD, S_EEL, S_GOLEM, S_GHOST, S_DEMON, S_HUMAN, '\0'
            };

            if ((punct = strchr(punctclasses, mcls1)) != 0)
                mcls1 = (schar) (S_ZOMBIE + 1 + (int) (punct - punctclasses));
            if ((punct = strchr(punctclasses, mcls2)) != 0)
                mcls2 = (schar) (S_ZOMBIE + 1 + (int) (punct - punctclasses));
        }
        res = mcls1 - mcls2; /* class */
        if (res == 0) {
            /* Riders are in the same class as major demons, yielding res==0
               above when both mcls1 and mcls2 are either Riders or demons or
               one of each; force Riders to be sorted before demons */
            res = is_rider(&mons[indx2]) - is_rider(&mons[indx1]);
            /* res -1 => #1 is a Rider, #2 isn't;
                    0 => both Riders or neither;
                   +1 => #2 is a Rider, #1 isn't */
            if (res)
                break;
            mlev1 = mons[indx1].mlevel;
            mlev2 = mons[indx2].mlevel;
            res = mlev1 - mlev2; /* mlevel low to high */
            if (flags.vanq_sortmode == VANQ_MCLS_HTOL)
                res = -res; /* mlevel high to low */
        }
        break;
    case VANQ_COUNT_H_L:
    case VANQ_COUNT_L_H:
        died1 = svm.mvitals[indx1].died;
        died2 = svm.mvitals[indx2].died;
        res = died2 - died1; /* dead count high to low */
        if (flags.vanq_sortmode == VANQ_COUNT_L_H)
            res = -res; /* dead count low to high */
        break;
    }
    /* tiebreaker: internal mons[] index */
    if (res == 0)
        res = indx1 - indx2; /* mndx low to high */
    return res;
}

/* returns -1 if cancelled via ESC */
int
set_vanq_order(boolean for_vanq)
{
    winid tmpwin;
    menu_item *selected;
    anything any;
    char buf[BUFSZ];
    const char *desc;
    int i, n, choice,
        clr = NO_COLOR;

    tmpwin = create_nhwindow(NHW_MENU);
    start_menu(tmpwin, MENU_BEHAVE_STANDARD);
    any = cg.zeroany; /* zero out all bits */
    for (i = 0; i < SIZE(vanqorders); i++) {
        if (i == VANQ_ALPHA_MIX || i == VANQ_MCLS_HTOL) /* skip these */
            continue;
        /* suppress some orderings if this menu if for 'm #genocided' */
        if (!for_vanq && (i == VANQ_COUNT_H_L || i == VANQ_COUNT_L_H))
            continue;
        desc = vanqorders[i][2];
        /* unique monsters can't be genocided so "alpha, unique separate"
           and "alpha, unique intermixed" are confusing descriptions when
           this menu is for #genocided rather than for #vanquished */
        if (!for_vanq && i == VANQ_ALPHA_SEP)
            desc = N_("alphabetically");
        any.a_int = i + 1;
        add_menu(tmpwin, &nul_glyphinfo, &any, *vanqorders[i][0], 0,
                 ATR_NONE, clr, _(desc),
                 (i == flags.vanq_sortmode) ? MENU_ITEMFLAGS_SELECTED
                                            : MENU_ITEMFLAGS_NONE);
    }
    Strcpy(buf, for_vanq
                ? _("Sort order for vanquished monster counts (also "
                    "genocided types)")
                : _("Sort order for genocided monster types (also "
                    "vanquished counts)"));
    end_menu(tmpwin, buf);

    n = select_menu(tmpwin, PICK_ONE, &selected);
    destroy_nhwindow(tmpwin);
    if (n > 0) {
        choice = selected[0].item.a_int - 1;
        /* skip preselected entry if we have more than one item chosen */
        if (n > 1 && choice == flags.vanq_sortmode)
            choice = selected[1].item.a_int - 1;
        free((genericptr_t) selected);
        flags.vanq_sortmode = choice;
    }
    return (n < 0) ? -1 : flags.vanq_sortmode;
}

/* #vanquished command */
int
dovanquished(void)
{
    list_vanquished(iflags.menu_requested ? 'A' : 'y', FALSE);
    iflags.menu_requested = FALSE;
    return ECMD_OK;
}

/* high priests aren't unique but are flagged as such to simplify something */
#define UniqCritterIndx(mndx) \
    ((mons[mndx].geno & G_UNIQ) != 0 && mndx != PM_HIGH_CLERIC)

#define done_stopprint program_state.stopprint

/* used for #vanquished and end of game disclosure and end of game dumplog */
void
list_vanquished(char defquery, boolean ask)
{
    int i;
    int pfx, nkilled;
    unsigned ntypes, ni;
    long total_killed = 0L;
    winid klwin;
    short mindx[NUMMONS];
    char c, buf[BUFSZ], buftoo[BUFSZ];
    /* 'A' is only supplied by 'm #vanquished'; 'd' is only supplied by
       dump_everything() when writing dumplog, so won't happen if built
       without '#define DUMPLOG' but there's no need for conditionals here */
    boolean force_sort = (defquery == 'A'),
            dumping = (defquery == 'd');

    /* normally we don't ask about sort order for the vanquished list unless
       it contains at least two entries; however, if player has used explicit
       'm #vanquished', choose order no matter what it contains so far */
    if (force_sort) { /* iflags.menu_requested via dovanquished() */
        /* choose value for vanq_sortmode via menu; ESC cancels choosing
           sort order but continues with vanquishd monsters display */
        (void) set_vanq_order(TRUE);
    }
    if (dumping || force_sort) {
        /* switch from 'A' or 'd' to 'y'; 'ask' is already False for the
           cases that might supply 'A' or 'd' */
        defquery = 'y';
        ask = FALSE; /* redundant */
    }

    /* get totals first */
    ntypes = 0;
    for (i = LOW_PM; i < NUMMONS; i++) {
        if ((nkilled = (int) svm.mvitals[i].died) == 0)
            continue;
        mindx[ntypes++] = i;
        total_killed += (long) nkilled;
    }

    /* vanquished creatures list;
     * includes all dead monsters, not just those killed by the player
     */
    if (ntypes != 0) {
        char mlet, prev_mlet = 0; /* used as small integer, not character */
        boolean class_header, uniq_header, Rider,
                was_uniq = FALSE, special_hdr = FALSE;

        if (ask) {
            char allow_yn[10];

            if (ntypes > 1) {
                Strcpy(allow_yn, ynaqchars);
            } else {
                Strcpy(allow_yn, ynqchars); /* don't include 'a', but */
                Strcat(allow_yn, "\033a");  /* allow user to answer 'a' */
                if (defquery == 'a') /* potential default from 'disclose' */
                    defquery = 'y';
            }
            c = yn_function("Do you want an account of creatures vanquished?",
                            allow_yn, defquery, TRUE);
        } else {
            c = defquery;
        }

        if (c == 'q')
            done_stopprint++;
        if (c == 'y' || c == 'a') {
            if (c == 'a' && ntypes > 1) { /* ask user to choose sort order */
                /* choose value for vanq_sortmode via menu; ESC cancels list
                   of vanquished monsters but does not set 'done_stopprint' */
                if (set_vanq_order(TRUE) < 0)
                    return;
            }
            uniq_header = (flags.vanq_sortmode == VANQ_ALPHA_SEP);
            class_header = ((flags.vanq_sortmode == VANQ_MCLS_LTOH
                             || flags.vanq_sortmode == VANQ_MCLS_HTOL)
                            && ntypes > 1);

            klwin = create_nhwindow(NHW_MENU);
            putstr(klwin, 0, "Vanquished creatures:");
            if (!dumping)
                putstr(klwin, 0, "");

            qsort((genericptr_t) mindx, ntypes, sizeof *mindx, vanqsort_cmp);
            for (ni = 0; ni < ntypes; ni++) {
                i = mindx[ni];
                nkilled = svm.mvitals[i].died;
                Rider = is_rider(&mons[i]);
                mlet = mons[i].mlet;
                if (class_header
                    && (mlet != prev_mlet || (special_hdr && !Rider))) {
                    if (!Rider) {
                        Strcpy(buf, def_monsyms[(int) mlet].explain);
                        special_hdr = FALSE;
                    } else {
                        Strcpy(buf, "Rider");
                        special_hdr = TRUE;
                    }
                    /* 'ask' implies final disclosure, where highlighting
                       of various header lines is suppressed */
                    putstr(klwin, ask ? ATR_NONE : iflags.menu_headings.attr,
                           upstart(buf));
                    prev_mlet = mlet;
                }
                if (UniqCritterIndx(i)) {
                    Sprintf(buf, "%s%s",
                            !type_is_pname(&mons[i]) ? "the " : "",
                            mons[i].pmnames[NEUTRAL]);
                    if (nkilled > 1)
                        Sprintf(eos(buf), " (%s)",
                                N_times((long) nkilled, buftoo));
                    was_uniq = TRUE;
                } else {
                    if (uniq_header && was_uniq) {
                        putstr(klwin, 0, "");
                        was_uniq = FALSE;
                    }
                    /* trolls or undead might have come back,
                       but we don't keep track of that */
                    if (nkilled == 1)
                        Strcpy(buf, an(mons[i].pmnames[NEUTRAL]));
                    else
                        Sprintf(buf, "%3d %s", nkilled,
                                makeplural(mons[i].pmnames[NEUTRAL]));
                }
                /* number of leading spaces to match 3 digit prefix */
                pfx = !strncmpi(buf, "the ", 4) ? 0
                      : !strncmpi(buf, "an ", 3) ? 1
                        : !strncmpi(buf, "a ", 2) ? 2
                          : !digit(buf[2]) ? 4 : 0;
                if (class_header)
                    ++pfx;
                Snprintf(buftoo, sizeof buftoo, "%*s%s", pfx, "", buf);
                putstr(klwin, 0, buftoo);
            }
            /*
             * if (Hallucination)
             *     putstr(klwin, 0, "and a partridge in a pear tree");
             */
            if (ntypes > 1) {
                if (!dumping)
                    putstr(klwin, 0, "");
                Sprintf(buf, "%ld creatures vanquished.", total_killed);
                putstr(klwin, 0, buf);
            }
            display_nhwindow(klwin, TRUE);
            destroy_nhwindow(klwin);
        }

    /*
     * For end-of-game disclosure, we're only called when some monsters
     * were vanquished and won't reach these 'else-if's.
     *
     * If no monsters have been vanquished, we're either called for game
     * still in progress, so use present tense via pline(), or for dumplog
     * which needs putstr() and past tense.
     */
    } else if (!program_state.gameover) {
        /* #vanquished rather than final disclosure, so pline() is ok */
        pline("No creatures have been vanquished.");
#ifdef DUMPLOG
    } else if (dumping) {
        putstr(0, 0, "No creatures were vanquished."); /* not pline() */
#endif
    }
}

/* number of monster species which have been genocided */
int
num_genocides(void)
{
    int i, n = 0;

    for (i = LOW_PM; i < NUMMONS; ++i) {
        if (svm.mvitals[i].mvflags & G_GENOD) {
            ++n;
            if (UniqCritterIndx(i))
                impossible("unique creature '%d: %s' genocided?",
                           i, mons[i].pmnames[NEUTRAL]);
        }
    }
    return n;
}

/* return a count of the number of extinct species */
staticfn int
num_extinct(void)
{
    int i, n = 0;

    for (i = LOW_PM; i < NUMMONS; ++i) {
        if (UniqCritterIndx(i))
            continue;
        if ((svm.mvitals[i].mvflags & G_GONE) == G_EXTINCT)
            ++n;
    }
    return n;
}

/* collect both genocides and extinctions, skipping uniques */
staticfn int
num_gone(int mvflags, int *mindx)
{
    uchar mflg = (uchar) mvflags;
    int i, n = 0;

    (void) memset((genericptr_t) mindx, 0, NUMMONS * sizeof *mindx);

    for (i = LOW_PM; i < NUMMONS; ++i) {
        /* uniques can't be genocided but can become extinct;
           however, they're never reported as extinct, so skip them */
        if (UniqCritterIndx(i))
            continue;

        if ((svm.mvitals[i].mvflags & mflg) != 0)
            mindx[n++] = i;
    }
    return n;
}

/* show genocided and extinct monster types for final disclosure/dumplog
   or for the #genocided command */
void
list_genocided(char defquery, boolean ask)
{
    int i, mndx;
    int ngenocided, nextinct, ngone, mvflags, mindx[NUMMONS];
    char c;
    winid klwin;
    char buf[BUFSZ];
    boolean genoing, /* prompting for genocide or class genocide */
            dumping; /* for DUMPLOG; doesn't need to be conditional */
    boolean both = (program_state.gameover || wizard || discover);

    dumping = (defquery == 'd');
    genoing = (defquery == 'g');
    if (dumping || genoing)
        defquery = 'y';
    if (genoing)
        both = FALSE; /* genocides only, not extinctions */

    /* this goes through the whole monster list up to three times but will
       happen rarely and is simpler than a more general single pass check;
       extinctions are only revealed during end of game disclosure or when
       running in wizard or explore mode */
    ngenocided = num_genocides();
    nextinct = both ? num_extinct() : 0;
    mvflags = G_GENOD | (both ? G_EXTINCT : 0);
    ngone = num_gone(mvflags, mindx);

    /* genocided or extinct species list */
    if (ngone > 0) {
        Sprintf(buf, "Do you want a list of %sspecies%s%s?",
                (nextinct && !ngenocided) ? "extinct " : "",
                (ngenocided) ? " genocided" : "",
                (nextinct && ngenocided) ? " and extinct" : "");
        c = ask ? yn_function(buf, (ngone > 1) ? "ynaq" : "ynq\033a",
                              defquery, TRUE)
                : defquery;
        if (c == 'q')
            done_stopprint++;
        if (c == 'y' || c == 'a') {
            int save_sortmode;
            char mlet, prev_mlet = 0;
            boolean class_header = FALSE;

            if (ngone > 1) {
                if (c == 'a') { /* ask player to choose sort order */
                    /* #genocided shares #vanquished's sort order */
                    if (set_vanq_order(FALSE) < 0)
                        return;
                }
                /* sort orderings count-high-to-low or count-low-to-high
                   don't make sense for genocides; if the preferred order
                   to set to either of those, use alphabetical instead;
                   note: the tie breaker for by-class is level-high-to-low
                   or level-low-to-high rather than count so is ok as-is */
                save_sortmode = flags.vanq_sortmode;
                if (flags.vanq_sortmode == VANQ_COUNT_H_L
                    || flags.vanq_sortmode == VANQ_COUNT_L_H)
                    flags.vanq_sortmode = VANQ_ALPHA_MIX;
                qsort((genericptr_t) mindx, ngone,
                      sizeof *mindx, vanqsort_cmp);
                class_header = (flags.vanq_sortmode == VANQ_MCLS_LTOH
                                || flags.vanq_sortmode == VANQ_MCLS_HTOL);
                flags.vanq_sortmode = save_sortmode;
            }

            klwin = create_nhwindow(NHW_MENU);
            Sprintf(buf, "%s%s species:",
                    (ngenocided) ? "Genocided" : "Extinct",
                    (nextinct && ngenocided) ? " or extinct" : "");
            putstr(klwin, 0, buf);
            if (!dumping)
                putstr(klwin, 0, "");

            for (i = 0; i < ngone; ++i) {
                mndx = mindx[i];
                mlet = mons[mndx].mlet;
                if (class_header && mlet != prev_mlet) {
                    Strcpy(buf, def_monsyms[(int) mlet].explain);
                    /* 'ask' implies final disclosure, where highlighting
                       of various header lines is suppressed */
                    putstr(klwin, ask ? ATR_NONE : iflags.menu_headings.attr,
                           upstart(buf));
                    prev_mlet = mlet;
                }
                Sprintf(buf, " %s", makeplural(mons[mndx].pmnames[NEUTRAL]));
                /*
                 * "Extinct" is unfortunate terminology.  A species
                 * is marked extinct when its birth limit is reached,
                 * but there might be members of the species still
                 * alive, contradicting the meaning of the word.
                 *
                 * We only append "(extinct)" if the G_GENOD bit is
                 * clear.  During normal play, 'mndx' won't be in the
                 * collected list unless that bit is set.
                 */
                if ((svm.mvitals[mndx].mvflags & G_GONE) == G_EXTINCT)
                    Strcat(buf, " (extinct)");
                putstr(klwin, 0, buf);
            }
            if (!dumping)
                putstr(klwin, 0, "");
            if (ngenocided > 0) {
                Sprintf(buf, "%d species genocided.", ngenocided);
                putstr(klwin, 0, buf);
            }
            if (nextinct > 0) {
                Sprintf(buf, "%d species extinct.", nextinct);
                putstr(klwin, 0, buf);
            }

            display_nhwindow(klwin, TRUE);
            destroy_nhwindow(klwin);
        }

    /* See the comment for similar code near the end of list_vanquished(). */
    } else if (!program_state.gameover) {
        /* #genocided rather than final disclosure, so pline() is ok and
           extinction has been ignored */
        pline(genoing ? "No creatures have been genocided yet."
                      : "No creatures have been genocided.");
#ifdef DUMPLOG
    } else if (dumping) { /* 'gameover' is True if we make it here */
        putstr(0, 0, "No species were genocided or became extinct.");
#endif
    }
}

/* M-g - #genocided command */
int
dogenocided(void)
{
    list_genocided(iflags.menu_requested ? 'a' : 'y', FALSE);
    return ECMD_OK;
}

DISABLE_WARNING_FORMAT_NONLITERAL

/* #wizborn extended command */
int
doborn(void)
{
    static const char fmt[] = "%4i %4i %c %-30s";
    int i;
    winid datawin = create_nhwindow(NHW_TEXT);
    char buf[BUFSZ];
    int nborn = 0, ndied = 0;

    putstr(datawin, 0, "died born");
    for (i = LOW_PM; i < NUMMONS; i++)
        if (svm.mvitals[i].born || svm.mvitals[i].died
            || (svm.mvitals[i].mvflags & G_GONE) != 0) {
            Sprintf(buf, fmt,
                    svm.mvitals[i].died, svm.mvitals[i].born,
                    ((svm.mvitals[i].mvflags & G_GONE) == G_EXTINCT) ? 'E'
                    : ((svm.mvitals[i].mvflags & G_GONE) == G_GENOD) ? 'G'
                      : ((svm.mvitals[i].mvflags & G_GONE) != 0) ? 'X'
                        : ' ',
                    mons[i].pmnames[NEUTRAL]);
            putstr(datawin, 0, buf);
            nborn += svm.mvitals[i].born;
            ndied += svm.mvitals[i].died;
        }

    putstr(datawin, 0, "");
    Sprintf(buf, fmt, ndied, nborn, ' ', "");

    display_nhwindow(datawin, FALSE);
    destroy_nhwindow(datawin);

    return ECMD_OK;
}

RESTORE_WARNING_FORMAT_NONLITERAL

/*
 * align_str(), piousness(), mstatusline() and ustatusline() once resided
 * in pline.c, then got moved to priest.c just to be out of there.  They
 * fit better here.
 */

const char *
align_str(aligntyp alignment)
{
    switch ((int) alignment) {
    case A_CHAOTIC:
        return "chaotic";
    case A_NEUTRAL:
        return "neutral";
    case A_LAWFUL:
        return "lawful";
    case A_NONE:
        return "unaligned";
    }
    return "unknown";
}

staticfn char *
size_str(int msize)
{
    static char outbuf[40];

    switch (msize) {
    case MZ_TINY:
        Strcpy(outbuf, "tiny");
        break;
    case MZ_SMALL:
        Strcpy(outbuf, "small");
        break;
    case MZ_MEDIUM:
        Strcpy(outbuf, "medium");
        break;
    case MZ_LARGE:
        Strcpy(outbuf, "large");
        break;
    case MZ_HUGE:
        Strcpy(outbuf, "huge");
        break;
    case MZ_GIGANTIC:
        Strcpy(outbuf, "gigantic");
        break;
    default:
        Sprintf(outbuf, "unknown size (%d)", msize);
        break;
    }
    return outbuf;
}

/* used for self-probing */
char *
piousness(boolean showneg, const char *suffix)
{
    static char buf[32]; /* bigger than "insufficiently neutral" */
    const char *pio;

    /* note: piousness 20 matches MIN_QUEST_ALIGN (quest.h) */
    if (u.ualign.record >= 20)
        pio = "piously";
    else if (u.ualign.record > 13)
        pio = "devoutly";
    else if (u.ualign.record > 8)
        pio = "fervently";
    else if (u.ualign.record > 3)
        pio = "stridently";
    else if (u.ualign.record == 3)
        pio = "";
    else if (u.ualign.record > 0)
        pio = "haltingly";
    else if (u.ualign.record == 0)
        pio = "nominally";
    else if (!showneg)
        pio = "insufficiently";
    else if (u.ualign.record >= -3)
        pio = "strayed";
    else if (u.ualign.record >= -8)
        pio = "sinned";
    else
        pio = "transgressed";

    Sprintf(buf, "%s", pio);
    if (suffix && (!showneg || u.ualign.record >= 0)) {
        if (u.ualign.record != 3)
            Strcat(buf, " ");
        Strcat(buf, suffix);
    }
    return buf;
}

#if 0
/* fragments of piousness() and size_str(), translated where displayed */
N_("piously") N_("devoutly") N_("fervently") N_("stridently")
N_("haltingly") N_("nominally") N_("insufficiently") N_("strayed")
N_("sinned") N_("transgressed") N_("chaotic") N_("neutral") N_("lawful")
N_("unaligned") N_("unknown") N_("tiny") N_("small") N_("medium")
N_("large") N_("huge") N_("gigantic")
#endif

/* translation of piousness() "<adverb> <alignment>" about the hero */
staticfn const char *
pious_i18n(const char *str)
{
    static char buf[BUFSZ];
    char adv[BUFSZ];
    const char *sp = strchr(str, ' ');

    if (!sp)
        return gendered_word(str, flags.female ? 1 : 0);
    copynchars(adv, str, min((int) (sp - str), BUFSZ - 1));
    Snprintf(buf, sizeof buf, "%s %s", _(adv),
             gendered_word(sp + 1, flags.female ? 1 : 0));
    return buf;
}

/* stethoscope or probing applied to monster -- one-line feedback */
void
mstatusline(struct monst *mtmp)
{
    aligntyp alignment = mon_aligntyp(mtmp);
    char info[BUFSZ], monnambuf[BUFSZ];

    info[0] = 0;
    if (mtmp->mtame) {
        Strcat(info, _(", tame"));
        if (wizard) {
            Sprintf(eos(info), " (%d", mtmp->mtame);
            if (!mtmp->isminion)
                Sprintf(eos(info), "; hungry %ld; apport %d",
                        EDOG(mtmp)->hungrytime, EDOG(mtmp)->apport);
            Strcat(info, ")");
        }
    } else if (mtmp->mpeaceful)
        Strcat(info, _(", peaceful"));

    if (mtmp->data == &mons[PM_LONG_WORM]) {
        int segndx, nsegs = count_wsegs(mtmp);

        /* the worm code internals don't consider the head to be one of
           the worm's segments, but we count it as such when presenting
           worm feedback to the player */
        if (!nsegs) {
            Strcat(info, _(", single segment"));
        } else {
            ++nsegs; /* include head in the segment count */
            segndx = wseg_at(mtmp, gb.bhitpos.x, gb.bhitpos.y);
            if (i18n_active())
                Sprintf(eos(info), _(", segment %d of %d"), segndx, nsegs);
            else
                Sprintf(eos(info), ", %d%s of %d segments",
                        segndx, ordin(segndx), nsegs);
        }
    }
    if (ismnum(mtmp->cham) && mtmp->data != &mons[mtmp->cham])
        /* don't reveal the innate form (chameleon, vampire, &c),
           just expose the fact that this current form isn't it */
        Strcat(info, _(", shapechanger"));
    /* pets eating mimic corpses mimic while eating, so this comes first */
    if (mtmp->meating)
        Strcat(info, _(", eating"));
    /* a stethoscope exposes mimic before getting here so this
       won't be relevant for it, but wand of probing doesn't */
    if (mtmp->mundetected || mtmp->m_ap_type
        || visible_region_at(gb.bhitpos.x, gb.bhitpos.y))
        mhidden_description(mtmp,
                       MHID_PREFIX | MHID_ARTICLE | MHID_ALTMON | MHID_REGION,
                            eos(info));
    if (mtmp->mcan)
        Strcat(info, _(", cancelled"));
    if (mtmp->mconf)
        Strcat(info, _(", confused"));
    if (mtmp->mblinded || !mtmp->mcansee)
        Strcat(info, _(", blind"));
    if (mtmp->mstun)
        Strcat(info, _(", stunned"));
    if (mtmp->msleeping)
        Strcat(info, _(", asleep"));
#if 0 /* unfortunately mfrozen covers temporary sleep and being busy
       * (donning armor, for instance) as well as paralysis */
    else if (mtmp->mfrozen)
        Strcat(info, _(", paralyzed"));
#else
    else if (mtmp->mfrozen || !mtmp->mcanmove)
        Strcat(info, _(", can't move"));
#endif
    /* [arbitrary reason why it isn't moving] */
    else if ((mtmp->mstrategy & STRAT_WAITMASK) != 0)
        Strcat(info, _(", meditating"));
    if (mtmp->mflee)
        Strcat(info, _(", scared"));
    if (mtmp->mtrapped)
        Strcat(info, _(", trapped"));
    if (mtmp->mspeed)
        Strcat(info, (mtmp->mspeed == MFAST) ? _(", fast")
                      : (mtmp->mspeed == MSLOW) ? _(", slow")
                         : ", [? speed]");
    if (mtmp->minvis)
        Strcat(info, _(", invisible"));
    if (mtmp == u.ustuck) {
        struct permonst *pm = u.ustuck->data;

        /* being swallowed/engulfed takes priority over sticks(youmonst);
           this used to have that backwards and checked sticks() first */
        Strcat(info, u.uswallow ? (digests(pm)
                                   ? _(", digesting you")
                                   /* note: the "swallowing you" case won't
                                      happen because all animal engulfers
                                      either digest their victims (purple
                                      worm) or enfold them (trappers and
                                      lurkers above) */
                                   : (is_animal(pm) && !enfolds(pm))
                                     ? _(", swallowing you")
                                     : _(", engulfing you"))
                     /* !u.uswallow; if both youmonst and ustuck are holders,
                        youmonst wins */
                     : (!sticks(gy.youmonst.data) ? _(", holding you")
                                                 : _(", held by you")));
    }
    if (mtmp == u.usteed) {
        Strcat(info, _(", carrying you"));
        if (Wounded_legs) {
            /* EWounded_legs is used to track left/right/both rather than
               some form of extrinsic impairment; HWounded_legs is used for
               timeout; both apply to steed instead of hero when mounted */
            long legs = (EWounded_legs & BOTH_SIDES);
            const char *what = mbodypart(mtmp, LEG);

            if (legs == BOTH_SIDES)
                what = makeplural(what);
            Sprintf(eos(info), _(", injured %s"), what);
        }
    }
    if (mtmp->mleashed)
        Strcat(info, _(", leashed"));

    /* avoid "Status of the invisible newt ..., invisible" */
    /* and unlike a normal mon_nam, use "saddled" even if it has a name */
    Strcpy(monnambuf, x_monnam(mtmp, ARTICLE_YOUR, (char *) 0,
                               (SUPPRESS_IT | SUPPRESS_INVISIBLE), FALSE));

    pline("Status of %s (%s, %s):  Level %d  HP %d(%d)  AC %d%s.",
          monnambuf, _(align_str(alignment)),
          _(size_str(mtmp->data->msize)),
          mtmp->m_lev, mtmp->mhp, mtmp->mhpmax, find_mac(mtmp), info);
}

/* stethoscope or probing applied to hero -- one-line feedback */
void
ustatusline(void)
{
    NhRegion *reg;
    char info[BUFSZ];
    size_t ln;

    info[0] = '\0';
    if (Sick) {
        Strcat(info, _(", dying from"));
        if (u.usick_type & SICK_VOMITABLE)
            Strcat(info, _(" food poisoning"));
        if (u.usick_type & SICK_NONVOMITABLE) {
            if (u.usick_type & SICK_VOMITABLE)
                Strcat(info, _(" and"));
            Strcat(info, _(" illness"));
        }
    }
    if (Stoned)
        Strcat(info, _(", solidifying"));
    if (Slimed)
        Strcat(info, _(", becoming slimy"));
    if (Strangled)
        Strcat(info, _(", being strangled"));
    if (Vomiting)
        Strcat(info, _(", nauseated")); /* !"nauseous" */
    if (Confusion)
        Strcat(info, _(", confused"));
    if (Blind) {
        Strcat(info, _(", blind"));
        if (u.ucreamed) {
            boolean cover = ((long) u.ucreamed < BlindedTimeout
                             || Blindfolded || !haseyes(gy.youmonst.data));

            if (i18n_active()) {
                Strcat(info, cover ? _(", covered by sticky goop")
                                   : _(" by sticky goop"));
            } else {
                if (cover)
                    Strcat(info, ", cover");
                Strcat(info, "ed by sticky goop");
            }
        } /* note: "goop" == "glop"; variation is intentional */
    }
    if (Stunned)
        Strcat(info, _(", stunned"));
    if (Wounded_legs && !u.usteed) {
        /* EWounded_legs is used to track left/right/both rather than some
           form of extrinsic impairment; HWounded_legs is used for timeout;
           both apply to steed instead of hero when mounted */
        long legs = (EWounded_legs & BOTH_SIDES);
        const char *what = body_part(LEG);

        if (legs == BOTH_SIDES)
            what = makeplural(what);
        /* when it's just one leg, ^X reports which, left or right;
           ustatusline() doesn't, in order to keep the output a bit shorter */
        Sprintf(eos(info), ", injured %s", what);
    }
    if (Glib)
        Sprintf(eos(info), _(", slippery %s"), fingers_or_gloves(TRUE));
    if (u.utrap)
        Strcat(info, _(", trapped"));
    if (Fast)
        Strcat(info, Very_fast ? _(", very fast") : _(", fast"));
    if (u.uundetected)
        Strcat(info, _(", concealed"));
    else if (U_AP_TYPE != M_AP_NOTHING)
        Strcat(info, _(", disguised"));
    if (Invis)
        Strcat(info, _(", invisible"));
    if (u.ustuck) {
        if (u.uswallow)
            Strcat(info, digests(u.ustuck->data) ? _(", being digested by ")
                                                 : _(", engulfed by "));
        else if (!sticks(gy.youmonst.data))
            Strcat(info, _(", held by "));
        else
            Strcat(info, _(", holding "));
        /* FIXME? a_monnam() uses x_monnam() which has a special case that
           forces "the" instead of "a" when formatting u.ustuck while hero
           is swallowed; we don't really want that here but it isn't worth
           fiddling with just for self-probing while engulfed */
        Strcat(info, a_monnam(u.ustuck));
    }
    if (!u.uswallow
        && (reg = visible_region_at(u.ux, u.uy)) != 0
        && (ln = strlen(info)) < sizeof info)
        Snprintf(eos(info), sizeof info - ln, "%s",
                 reg_damg(reg) ? _(", in a cloud of poison gas")
                               : _(", in a cloud of vapor"));

    pline("Status of %s (%s):  Level %d  HP %d(%d)  AC %d%s.", svp.plname,
          i18n_active() ? pious_i18n(piousness(FALSE,
                                               align_str(u.ualign.type)))
                        : piousness(FALSE, align_str(u.ualign.type)),
          Upolyd ? mons[u.umonnum].mlevel : u.ulevel, Upolyd ? u.mh : u.uhp,
          Upolyd ? u.mhmax : u.uhpmax, u.uac, info);
}

/* for 'onefile' processing where end of this file isn't necessarily the
   end of the source code seen by the compiler */
#undef enl_msg
#undef you_are
#undef you_have
#undef you_can
#undef you_have_been
#undef you_have_never
#undef you_have_X
#undef LL_majors
#undef majorevent
#undef spoilerevent
#undef UniqCritterIndx
#undef done_stopprint

/*insight.c*/
