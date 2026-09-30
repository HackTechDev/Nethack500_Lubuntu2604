/* NetHack 5.0	iactions.c	$NHDT-Date: 1762680996 2025/11/09 01:36:36 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.543 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Pasi Kallinen, 2026. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

staticfn boolean item_naming_classification(struct obj *, char *, char *);
staticfn int item_reading_classification(struct obj *, char *);
staticfn void ia_addmenu(winid, int, char, const char *);
staticfn void itemactions_pushkeys(struct obj *, int);

enum item_action_actions {
    IA_NONE          = 0,
    IA_UNWIELD, /* hack for 'w-' */
    IA_APPLY_OBJ, /* 'a' */
    IA_DIP_OBJ, /* 'a' on a potion == dip */
    IA_NAME_OBJ, /* 'c' name individual item */
    IA_NAME_OTYP, /* 'C' name item's type */
    IA_DROP_OBJ, /* 'd' */
    IA_EAT_OBJ, /* 'e' */
    IA_ENGRAVE_OBJ, /* 'E' */
    IA_FIRE_OBJ, /* 'f' */
    IA_ADJUST_OBJ, /* 'i' #adjust inventory letter */
    IA_ADJUST_STACK, /* 'I' #adjust with count to split stack */
    IA_SACRIFICE, /* 'O' offer sacrifice */
    IA_BUY_OBJ, /* 'p' pay shopkeeper */
    IA_QUAFF_OBJ,
    IA_QUIVER_OBJ,
    IA_READ_OBJ,
    IA_RUB_OBJ,
    IA_THROW_OBJ,
    IA_TAKEOFF_OBJ,
    IA_TIP_CONTAINER,
    IA_INVOKE_OBJ,
    IA_WIELD_OBJ,
    IA_WEAR_OBJ,
    IA_SWAPWEAPON,
    IA_TWOWEAPON,
    IA_ZAP_OBJ,
    IA_WHATIS_OBJ, /* '/' specify inventory object */
};

DISABLE_WARNING_FORMAT_NONLITERAL

/* construct text for the menu entries for IA_NAME_OBJ and IA_NAME_OTYP */
staticfn boolean
item_naming_classification(
    struct obj *obj,
    char *onamebuf,
    char *ocallbuf)
{
    static const char
        Name[] = "Name",
        Rename[] = "Rename or un-name",
        Call[] = "Call",
        /* "re-call" seems a bit weird, but "recall" and
           "rename" don't fit for changing a type name */
        Recall[] = "Re-call or un-call";

    onamebuf[0] = ocallbuf[0] = '\0';
    if (i18n_active()) {
        /* translated menu entries don't distinguish "this specific" */
        if (name_ok(obj) == GETOBJ_SUGGEST)
            Sprintf(onamebuf, (!has_oname(obj) || !*ONAME(obj))
                              ? _("Name %s") : _("Rename or un-name %s"),
                    the(simpleonames(obj)));
        if (call_ok(obj) == GETOBJ_SUGGEST)
            Sprintf(ocallbuf, (!objects[obj->otyp].oc_uname
                               || !*objects[obj->otyp].oc_uname)
                              ? _("Call the object type: %s")
                              : _("Re-call or un-call the object type: %s"),
                    simpleonames(obj));
        return (*onamebuf || *ocallbuf) ? TRUE : FALSE;
    }
    if (name_ok(obj) == GETOBJ_SUGGEST) {
        Sprintf(onamebuf, "%s %s %s",
                (!has_oname(obj) || !*ONAME(obj)) ? Name : Rename,
                the_unique_obj(obj) ? "the"
                : !is_plural(obj) ? "this specific"
                  : "this stack of", /*"these",*/
                simpleonames(obj));
    }
    if (call_ok(obj) == GETOBJ_SUGGEST) {
        char *callname = simpleonames(obj);

        /* prefix known unique item with "the", make all other types plural */
        if (the_unique_obj(obj)) /* treats unID'd fake amulets as if real */
            callname = the(callname);
        else if (!is_plural(obj))
            callname = makeplural(callname);
        Sprintf(ocallbuf, "%s the type for %s",
                (!objects[obj->otyp].oc_uname
                 || !*objects[obj->otyp].oc_uname) ? Call : Recall,
                callname);
    }
    return (*onamebuf || *ocallbuf) ? TRUE : FALSE;
}

/* construct text for the menu entries for IA_READ_OBJ */
staticfn int
item_reading_classification(struct obj *obj, char *outbuf)
{
    int otyp = obj->otyp, res = IA_READ_OBJ;

    *outbuf = '\0';
    if (otyp == FORTUNE_COOKIE) {
        Strcpy(outbuf, N_("Read the message inside this cookie"));
    } else if (otyp == T_SHIRT) {
        Strcpy(outbuf, N_("Read the slogan on the shirt"));
    } else if (otyp == ALCHEMY_SMOCK) {
        Strcpy(outbuf, N_("Read the slogan on the apron"));
    } else if (otyp == HAWAIIAN_SHIRT) {
        Strcpy(outbuf, N_("Look at the pattern on the shirt"));
    } else if (obj->oclass == SCROLL_CLASS) {
        boolean magic = (obj->dknown
#ifdef MAIL_STRUCTURES
                         && otyp != SCR_MAIL
#endif
                         && (otyp != SCR_BLANK_PAPER
                             || !objects[otyp].oc_name_known));

        /* ia_addmenu() translates it */
        Strcpy(outbuf, magic ? N_("Read this scroll to activate its magic")
                             : N_("Read this scroll"));
    } else if (obj->oclass == SPBOOK_CLASS) {
        boolean novel = (otyp == SPE_NOVEL),
                blank = (otyp == SPE_BLANK_PAPER
                         && objects[otyp].oc_name_known),
                tome = (otyp == SPE_BOOK_OF_THE_DEAD
                        && objects[otyp].oc_name_known);

        if (novel) /* "novel" or "paperback book" */
            Sprintf(outbuf, _("Read this %s"), simpleonames(obj));
        else
            Strcpy(outbuf, blank ? N_("Read this spellbook")
                           : tome ? N_("Examine this tome")
                             : N_("Study this spellbook"));
    } else {
        res = IA_NONE;
    }
    return res;
}

RESTORE_WARNING_FORMAT_NONLITERAL

staticfn void
ia_addmenu(winid win, int act, char let, const char *txt)
{
    anything any;
    int clr = NO_COLOR;

    any = cg.zeroany;
    any.a_int = act;
    add_menu(win, &nul_glyphinfo, &any, let, 0,
             ATR_NONE, clr, _(txt), MENU_ITEMFLAGS_NONE);
}

/* set up a command to execute on a specific item next */
staticfn void
itemactions_pushkeys(struct obj *otmp, int act)
{
    switch (act) {
    default:
        impossible("Unknown item action %d", act);
        break;
    case IA_NONE:
        break;
    case IA_UNWIELD:
        cmdq_add_ec(CQ_CANNED, (otmp == uwep) ? dowield
                    : (otmp == uswapwep) ? remarm_swapwep
                      : (otmp == uquiver) ? dowieldquiver
                        : donull); /* can't happen */
        cmdq_add_key(CQ_CANNED, HANDS_SYM);
        break;
    case IA_APPLY_OBJ:
        cmdq_add_ec(CQ_CANNED, doapply);
        cmdq_add_key(CQ_CANNED, otmp->invlet);
        break;
    case IA_DIP_OBJ:
        /* #altdip instead of normal #dip - takes potion to dip into
           first (the inventory item instigating this) and item to
           be dipped second, also ignores floor features such as
           fountain/sink so we don't need to force m-prefix here */
        cmdq_add_ec(CQ_CANNED, dip_into);
        cmdq_add_key(CQ_CANNED, otmp->invlet);
        break;
    case IA_NAME_OBJ:
    case IA_NAME_OTYP:
        cmdq_add_ec(CQ_CANNED, docallcmd);
        cmdq_add_key(CQ_CANNED, (act == IA_NAME_OBJ) ? 'i' : 'o');
        cmdq_add_key(CQ_CANNED, otmp->invlet);
        break;
    case IA_DROP_OBJ:
        cmdq_add_ec(CQ_CANNED, dodrop);
        cmdq_add_key(CQ_CANNED, otmp->invlet);
        break;
    case IA_EAT_OBJ:
        /* start with m-prefix; for #eat, it means ignore floor food
           if present and eat food from invent */
        cmdq_add_ec(CQ_CANNED, do_reqmenu);
        cmdq_add_ec(CQ_CANNED, doeat);
        cmdq_add_key(CQ_CANNED, otmp->invlet);
        break;
    case IA_ENGRAVE_OBJ:
        cmdq_add_ec(CQ_CANNED, doengrave);
        cmdq_add_key(CQ_CANNED, otmp->invlet);
        break;
    case IA_FIRE_OBJ:
        cmdq_add_ec(CQ_CANNED, dofire);
        break;
    case IA_ADJUST_OBJ:
        cmdq_add_ec(CQ_CANNED, doorganize); /* #adjust */
        cmdq_add_key(CQ_CANNED, otmp->invlet);
        break;
    case IA_ADJUST_STACK:
        cmdq_add_ec(CQ_CANNED, adjust_split); /* #altadjust */
        cmdq_add_key(CQ_CANNED, otmp->invlet);
        break;
    case IA_SACRIFICE:
        cmdq_add_ec(CQ_CANNED, dosacrifice);
        cmdq_add_key(CQ_CANNED, otmp->invlet);
        break;
    case IA_BUY_OBJ:
        cmdq_add_ec(CQ_CANNED, dopay);
        cmdq_add_key(CQ_CANNED, otmp->invlet);
        break;
    case IA_QUAFF_OBJ:
        /* start with m-prefix; for #quaff, it means ignore fountain
           or sink if present and drink a potion from invent */
        cmdq_add_ec(CQ_CANNED, do_reqmenu);
        cmdq_add_ec(CQ_CANNED, dodrink);
        cmdq_add_key(CQ_CANNED, otmp->invlet);
        break;
    case IA_QUIVER_OBJ:
        cmdq_add_ec(CQ_CANNED, dowieldquiver);
        cmdq_add_key(CQ_CANNED, otmp->invlet);
        break;
    case IA_READ_OBJ:
        cmdq_add_ec(CQ_CANNED, doread);
        cmdq_add_key(CQ_CANNED, otmp->invlet);
        break;
    case IA_RUB_OBJ:
        cmdq_add_ec(CQ_CANNED, dorub);
        cmdq_add_key(CQ_CANNED, otmp->invlet);
        break;
    case IA_THROW_OBJ:
        cmdq_add_ec(CQ_CANNED, dothrow);
        cmdq_add_key(CQ_CANNED, otmp->invlet);
        break;
    case IA_TAKEOFF_OBJ:
        cmdq_add_ec(CQ_CANNED, ia_dotakeoff); /* #altdotakeoff */
        cmdq_add_key(CQ_CANNED, otmp->invlet);
        break;
    case IA_TIP_CONTAINER:
        /* start with m-prefix to skip floor containers;
           for menustyle:Traditional when more than one floor container
           is present, player will get a #tip menu and have to pick
           the "tip something being carried" choice, then this item
           will be already chosen from inventory; suboptimal but
           possibly an acceptable tradeoff since combining item actions
           with use of traditional ggetobj() is an unlikely scenario */
        cmdq_add_ec(CQ_CANNED, do_reqmenu);
        cmdq_add_ec(CQ_CANNED, dotip);
        cmdq_add_key(CQ_CANNED, otmp->invlet);
        break;
    case IA_INVOKE_OBJ:
        cmdq_add_ec(CQ_CANNED, doinvoke);
        cmdq_add_key(CQ_CANNED, otmp->invlet);
        break;
    case IA_WIELD_OBJ:
        cmdq_add_ec(CQ_CANNED, dowield);
        cmdq_add_key(CQ_CANNED, otmp->invlet);
        break;
    case IA_WEAR_OBJ:
        cmdq_add_ec(CQ_CANNED, dowear);
        cmdq_add_key(CQ_CANNED, otmp->invlet);
        break;
    case IA_SWAPWEAPON:
        cmdq_add_ec(CQ_CANNED, doswapweapon);
        break;
    case IA_TWOWEAPON:
        cmdq_add_ec(CQ_CANNED, dotwoweapon);
        break;
    case IA_ZAP_OBJ:
        cmdq_add_ec(CQ_CANNED, dozap);
        cmdq_add_key(CQ_CANNED, otmp->invlet);
        break;
    case IA_WHATIS_OBJ:
        cmdq_add_ec(CQ_CANNED, dowhatis); /* "/" command */
        cmdq_add_key(CQ_CANNED, 'i');     /* "i" == item from inventory */
        cmdq_add_key(CQ_CANNED, otmp->invlet);
        break;
    }
}

#if 0
/* entries composed with objnam_fmt() and their forms */
N_("Light this %s") N_("Extinguish this %s")
N_("Light these %s") N_("Extinguish these %s") N_("Rub this %s")
C_("feminine", "Light this %s") C_("feminine", "Extinguish this %s")
C_("feminine", "Rub this %s")
#endif

DISABLE_WARNING_FORMAT_NONLITERAL

/* Show menu of possible actions hero could do with item otmp */
int
itemactions(struct obj *otmp)
{
    int n, act = IA_NONE;
    winid win;
    char buf[BUFSZ], buf2[BUFSZ];
    menu_item *selected;
    struct monst *mtmp;
    boolean lit = otmp->lamplit ? TRUE : FALSE;
    boolean already_worn = (otmp->owornmask & (W_ARMOR | W_ACCESSORY)) != 0;

    win = create_nhwindow(NHW_MENU);
    start_menu(win, MENU_BEHAVE_STANDARD);

    /* -: unwield; picking current weapon offers an opportunity for 'w-'
       to wield bare/gloved hands; likewise for 'Q-' with quivered item(s) */
    if (otmp == uwep || otmp == uswapwep || otmp == uquiver) {
        /* whole entries, so that each can be translated;
           [quivered][plural][weapon] */
        static const char *const unwield_fmt[2][2][2] = {
            { { N_("Wield '%c' to un-wield this item"),
                N_("Wield '%c' to un-wield this weapon") },
              { N_("Wield '%c' to un-wield these items"),
                N_("Wield '%c' to un-wield these weapons") } },
            { { N_("Quiver '%c' to un-ready this item"),
                N_("Quiver '%c' to un-ready this weapon") },
              { N_("Quiver '%c' to un-ready these items"),
                N_("Quiver '%c' to un-ready these weapons") } },
        };
        boolean weapon = (otmp->oclass == WEAPON_CLASS || is_weptool(otmp));

        /*
         * TODO: if uwep is ammo, tell player that to shoot instead of toss,
         *       the corresponding launcher must be wielded;
         */
        Sprintf(buf, _(unwield_fmt[otmp == uquiver][is_plural(otmp) ? 1 : 0]
                                  [weapon ? 1 : 0]), HANDS_SYM);
        ia_addmenu(win, IA_UNWIELD, '-', buf);
    }

    /* a: apply */
    if (otmp->oclass == COIN_CLASS)
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Flip a coin");
    else if (otmp->otyp == CREAM_PIE)
        ia_addmenu(win, IA_APPLY_OBJ, 'a',
                   "Hit yourself with this cream pie");
    else if (otmp->otyp == BULLWHIP)
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Lash out with this whip");
    else if (otmp->otyp == GRAPPLING_HOOK)
        ia_addmenu(win, IA_APPLY_OBJ, 'a',
                   "Grapple something with this hook");
    else if (otmp->otyp == BAG_OF_TRICKS && objects[otmp->otyp].oc_name_known)
        /* bag of tricks skips this unless discovered */
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Reach into this bag");
    else if (Is_container(otmp))
        /* bag of tricks gets here only if not yet discovered */
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Open this container");
    else if (otmp->otyp == CAN_OF_GREASE)
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Use the can to grease an item");
    else if (otmp->otyp == LOCK_PICK
             || otmp->otyp == CREDIT_CARD
             || otmp->otyp == SKELETON_KEY)
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Use this tool to pick a lock");
    else if (otmp->otyp == TINNING_KIT)
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Use this kit to tin a corpse");
    else if (otmp->otyp == LEASH)
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Tie a pet to this leash");
    else if (otmp->otyp == SADDLE)
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Place this saddle on a pet");
    else if (otmp->otyp == MAGIC_WHISTLE
             || otmp->otyp == TIN_WHISTLE)
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Blow this whistle");
    else if (otmp->otyp == EUCALYPTUS_LEAF)
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Use this leaf as a whistle");
    else if (otmp->otyp == STETHOSCOPE)
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Listen through the stethoscope");
    else if (otmp->otyp == MIRROR)
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Show something its reflection");
    else if (otmp->otyp == BELL || otmp->otyp == BELL_OF_OPENING)
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Ring the bell");
    else if (otmp->otyp == CANDELABRUM_OF_INVOCATION) {
        Strcpy(buf, lit ? N_("Extinguish the candelabrum")
                        : N_("Light the candelabrum"));
        ia_addmenu(win, IA_APPLY_OBJ, 'a', buf);
    } else if (otmp->otyp == WAX_CANDLE || otmp->otyp == TALLOW_CANDLE) {
        boolean multiple = (otmp->quan == 1L) ? FALSE : TRUE;
        struct obj *o = carrying(CANDELABRUM_OF_INVOCATION);

        if (o && o->spe < 7) {
            /* whole entries, so that each can be translated */
            if (multiple)
                Strcpy(buf, !lit ? N_("Attach these to your candelabrum, "
                                      "or light them")
                                 : N_("Attach these to your candelabrum, "
                                      "or extinguish them"));
            else
                Strcpy(buf, !lit ? N_("Attach this to your candelabrum, "
                                      "or light it")
                                 : N_("Attach this to your candelabrum, "
                                      "or extinguish it"));
        } else {
            const char *nm = simpleonames(otmp);

            Sprintf(buf, objnam_fmt(multiple
                                    ? (lit ? "Extinguish these %s"
                                           : "Light these %s")
                                    : (lit ? "Extinguish this %s"
                                           : "Light this %s"), nm, otmp),
                    nm);
        }
        ia_addmenu(win, IA_APPLY_OBJ, 'a', buf);
    } else if (otmp->otyp == OIL_LAMP || otmp->otyp == MAGIC_LAMP
               || otmp->otyp == BRASS_LANTERN) {
        Strcpy(buf, lit ? N_("Extinguish this light source")
                        : N_("Light this light source"));
        ia_addmenu(win, IA_APPLY_OBJ, 'a', buf);
    } else if (otmp->otyp == POT_OIL && objects[otmp->otyp].oc_name_known) {
        Strcpy(buf, lit ? N_("Extinguish this oil") : N_("Light this oil"));
        ia_addmenu(win, IA_APPLY_OBJ, 'a', buf);
    } else if (otmp->oclass == POTION_CLASS) {
        /* FIXME? this should probably be moved to 'D' rather than be 'a' */
        Strcpy(buf, (otmp->quan != 1L)
                    ? N_("Dip something into one of these potions")
                    : N_("Dip something into this potion"));
        ia_addmenu(win, IA_DIP_OBJ, 'a', buf);
    } else if (otmp->otyp == EXPENSIVE_CAMERA)
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Take a photograph");
    else if (otmp->otyp == TOWEL)
        ia_addmenu(win, IA_APPLY_OBJ, 'a',
                   "Clean yourself off with this towel");
    else if (otmp->otyp == CRYSTAL_BALL)
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Peer into this crystal ball");
    else if (otmp->otyp == MAGIC_MARKER)
        ia_addmenu(win, IA_APPLY_OBJ, 'a',
                   "Write on something with this marker");
    else if (otmp->otyp == FIGURINE)
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Make this figurine transform");
    else if (otmp->otyp == UNICORN_HORN)
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Use this unicorn horn");
    else if (otmp->otyp == HORN_OF_PLENTY
             && objects[otmp->otyp].oc_name_known)
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Blow into the horn of plenty");
    else if (otmp->otyp >= WOODEN_FLUTE && otmp->otyp <= DRUM_OF_EARTHQUAKE)
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Play this musical instrument");
    else if (otmp->otyp == LAND_MINE || otmp->otyp == BEARTRAP)
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Arm this trap");
    else if (otmp->otyp == PICK_AXE || otmp->otyp == DWARVISH_MATTOCK)
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Dig with this digging tool");
    else if (otmp->oclass == WAND_CLASS)
        ia_addmenu(win, IA_APPLY_OBJ, 'a', "Break this wand");

    /* 'c', 'C' - call an item or its type something */
    if (item_naming_classification(otmp, buf, buf2)) {
        if (*buf)
            ia_addmenu(win, IA_NAME_OBJ, 'c', buf);
        if (*buf2)
            ia_addmenu(win, IA_NAME_OTYP, 'C', buf2);
    }

    /* d: drop item, works on everything except worn items; those will
       always have a takeoff/remove choice so we don't have to worry
       about the menu maybe being empty when 'd' is suppressed */
    if (!already_worn) {
        Strcpy(buf, (otmp->quan > 1L) ? N_("Drop this stack")
                                      : N_("Drop this item"));
        ia_addmenu(win, IA_DROP_OBJ, 'd', buf);
    }

    /* e: eat item */
    if (otmp->otyp == TIN) {
        boolean opener = (uwep && uwep->otyp == TIN_OPENER);

        if (otmp->quan > 1L)
            Strcpy(buf, opener ? N_("Open one of these tins with your "
                                    "tin opener and eat the contents")
                               : N_("Open one of these tins and eat the "
                                    "contents"));
        else
            Strcpy(buf, opener ? N_("Open this tin with your tin opener "
                                    "and eat the contents")
                               : N_("Open this tin and eat the contents"));
        ia_addmenu(win, IA_EAT_OBJ, 'e', buf);
    } else if (is_edible(otmp)) {
        Strcpy(buf, (otmp->quan > 1L) ? N_("Eat one of these")
                                      : N_("Eat this"));
        ia_addmenu(win, IA_EAT_OBJ, 'e', buf);
    }

    /* E: engrave with item */
    if (otmp->otyp == TOWEL) {
        ia_addmenu(win, IA_ENGRAVE_OBJ, 'E',
                   "Wipe the floor with this towel");
    } else if (otmp->otyp == MAGIC_MARKER) {
        ia_addmenu(win, IA_ENGRAVE_OBJ, 'E',
                   "Scribble graffiti on the floor");
    } else if (otmp->oclass == WEAPON_CLASS || otmp->oclass == WAND_CLASS
             || otmp->oclass == GEM_CLASS || otmp->oclass == RING_CLASS) {
        boolean engr = (is_blade(otmp) || otmp->oclass == WAND_CLASS
                        || ((otmp->oclass == GEM_CLASS
                             || otmp->oclass == RING_CLASS)
                            && objects[otmp->otyp].oc_tough));

        if (i18n_active())
            Sprintf(buf, engr ? ((otmp->quan > 1L)
                                 ? _("Engrave on %s with one of these items")
                                 : _("Engrave on %s with this item"))
                              : ((otmp->quan > 1L)
                                 ? _("Write on %s with one of these items")
                                 : _("Write on %s with this item")),
                    i18n_the(surface(u.ux, u.uy)));
        else
            Sprintf(buf, "%s on the %s with %s", engr ? "Engrave" : "Write",
                    surface(u.ux, u.uy),
                    (otmp->quan > 1L) ? "one of these items" : "this item");
        ia_addmenu(win, IA_ENGRAVE_OBJ, 'E', buf);
    }

    /* f: fire quivered ammo */
    if (otmp == uquiver) {
        boolean shoot = ammo_and_launcher(otmp, uwep);

        /* FIXME: see the multi-shot FIXME about "one of" for 't: throw' */
        if (shoot) {
            assert(uwep != NULL);
            Sprintf(buf, (otmp->quan > 1L)
                         ? _("Shoot one of these with your wielded %s")
                         : _("Shoot this with your wielded %s"),
                    simpleonames(uwep));
        } else {
            Strcpy(buf, (otmp->quan > 1L) ? N_("Throw one of these")
                                          : N_("Throw this"));
        }
        ia_addmenu(win, IA_FIRE_OBJ, 'f', buf);
    }

    /* i: #adjust inventory letter; gold can't be adjusted unless there
       is some in a slot other than '$' (which shouldn't be possible) */
    if (otmp->oclass != COIN_CLASS || check_invent_gold("item-action"))
        ia_addmenu(win, IA_ADJUST_OBJ, 'i',
                   "Adjust inventory by assigning new letter");
    /* I: #adjust inventory item by splitting its stack  */
    if (otmp->quan > 1L && otmp->oclass != COIN_CLASS)
        ia_addmenu(win, IA_ADJUST_STACK, 'I',
                   "Adjust inventory by splitting this stack");

    /* O: offer sacrifice */
    if (IS_ALTAR(levl[u.ux][u.uy].typ) && !u.uswallow) {
        /* FIXME: this doesn't match #offer's likely candidates, which don't
           include corpses on Astral and don't include amulets off Astral */
        if (otmp->otyp == CORPSE)
            ia_addmenu(win, IA_SACRIFICE, 'O',
                       "Offer this corpse as a sacrifice at this altar");
        else if (otmp->otyp == AMULET_OF_YENDOR
                 || otmp->otyp == FAKE_AMULET_OF_YENDOR)
            ia_addmenu(win, IA_SACRIFICE, 'O',
                       "Offer this amulet as a sacrifice at this altar");
    }

    /* p: pay for unpaid utems */
    if (otmp->unpaid
        /* FIXME: should also handle player owned container (so not
           flagged 'unpaid') holding shop owned items */
        && (mtmp = shop_keeper(*in_rooms(u.ux, u.uy, SHOPBASE))) != 0
        && inhishop(mtmp)) {
        Strcpy(buf, (otmp->quan > 1L) ? N_("Buy this unpaid stack")
                                      : N_("Buy this unpaid item"));
        ia_addmenu(win, IA_BUY_OBJ, 'p', buf);
    }

    /* P: put on accessory */
    if (!already_worn) {
        /* if 'otmp' is worn, we'll skip 'P' and show 'R' below;
           if not worn, we show 'P - Put on this <simple-item>' if
           the slot is available, or 'P - <unavailable>'; for the latter,
           'P' will fail but we don't want to omit the choice because
           item actions can be used to learn commands */
        *buf = '\0';
        if (otmp->oclass == AMULET_CLASS) {
            Strcpy(buf, !uamul ? N_("Put this amulet on")
                               : N_("[already wearing an amulet]"));
        } else if (otmp->oclass == RING_CLASS || otmp->otyp == MEAT_RING) {
            if (!uleft || !uright)
                Strcpy(buf, N_("Put this ring on"));
            else
                Sprintf(buf, _("[both ring %s in use]"),
                        makeplural(body_part(FINGER)));
        } else if (otmp->otyp == BLINDFOLD || otmp->otyp == TOWEL
                   || otmp->otyp == LENSES) {
            if (ublindf)
                Strcpy(buf, N_("[already wearing eyewear]"));
            else if (otmp->otyp == LENSES)
                Strcpy(buf, N_("Put these lenses on"));
            else
                Strcpy(buf, (otmp->otyp == TOWEL)
                            ? N_("Put this on to blindfold yourself")
                            : N_("Put this on"));
        }
        if (*buf)
            ia_addmenu(win, IA_WEAR_OBJ, 'P', buf);
    }

    /* q: drink item */
    if (otmp->oclass == POTION_CLASS) {
        Strcpy(buf, (otmp->quan > 1L)
                    ? N_("Quaff (drink) one of these potions")
                    : N_("Quaff (drink) this potion"));
        ia_addmenu(win, IA_QUAFF_OBJ, 'q', buf);
    }

    /* Q: quiver throwable item */
    if ((otmp->oclass == GEM_CLASS || otmp->oclass == WEAPON_CLASS)
        && otmp != uquiver) {
        boolean shoot = ammo_and_launcher(otmp, uwep);

        if (otmp->quan > 1L)
            Strcpy(buf, shoot
                   ? N_("Quiver this stack for easy shooting with 'f'ire")
                   : N_("Quiver this stack for easy throwing with 'f'ire"));
        else
            Strcpy(buf, shoot
                   ? N_("Quiver this item for easy shooting with 'f'ire")
                   : N_("Quiver this item for easy throwing with 'f'ire"));
        ia_addmenu(win, IA_QUIVER_OBJ, 'Q', buf);
    }

    /* r: read item */
    if (item_reading_classification(otmp, buf) == IA_READ_OBJ)
        ia_addmenu(win, IA_READ_OBJ, 'r', buf);

    /* R: remove accessory or rub item */
    if (otmp->owornmask & W_ACCESSORY) {
        Strcpy(buf, (otmp->owornmask & W_AMUL) ? N_("Remove this amulet")
                    : (otmp->owornmask & W_RING) ? N_("Remove this ring")
                      : (otmp->owornmask & W_TOOL) ? N_("Remove this eyewear")
                        /* catchall -- can't happen */
                        : N_("Remove this accessory"));
        ia_addmenu(win, IA_TAKEOFF_OBJ, 'R', buf);
    }
    if (otmp->otyp == OIL_LAMP || otmp->otyp == MAGIC_LAMP
        || otmp->otyp == BRASS_LANTERN) {
        const char *nm = simpleonames(otmp);

        Sprintf(buf, objnam_fmt("Rub this %s", nm, otmp), nm);
        ia_addmenu(win, IA_RUB_OBJ, 'R', buf);
    } else if (otmp->oclass == GEM_CLASS && is_graystone(otmp))
        ia_addmenu(win, IA_RUB_OBJ, 'R', "Rub something on this stone");

    /* t: throw item */
    if (!already_worn) {
        boolean shoot = ammo_and_launcher(otmp, uwep);

        /*
         * FIXME:
         *  'one of these' should be changed to 'some of these' when there
         *  is the possibility of a multi-shot volley but we don't have
         *  any way to determine that except by actually calculating the
         *  volley count and that could randomly yield 1 here and 2..N
         *  while throwing or vice versa.
         */
        /* whole entries, so that each can be translated;
           [shoot][this item, them, one of these][same as 'f'] */
        static const char *const throw_fmt[2][3][2] = {
            { { N_("Throw this item"), N_("Throw this item (same as 'f')") },
              { N_("Throw them"), N_("Throw them (same as 'f')") },
              { N_("Throw one of these"),
                N_("Throw one of these (same as 'f')") } },
            { { N_("Shoot this item"), N_("Shoot this item (same as 'f')") },
              { N_("Shoot them"), N_("Shoot them (same as 'f')") },
              { N_("Shoot one of these"),
                N_("Shoot one of these (same as 'f')") } },
        };
        int which = (otmp->quan == 1L) ? 0
                    : (otmp->otyp == GOLD_PIECE) ? 1 : 2;
        /* if otmp is quivered, we've already listed
           'f - shoot|throw this item' as a choice;
           if 't' is duplicating that, say so ('t' and 'f'
           behavior differs for throwing a stack of gold) */
        boolean same_f = (otmp == uquiver
                          && (otmp->otyp != GOLD_PIECE || otmp->quan == 1L));

        Strcpy(buf, throw_fmt[shoot ? 1 : 0][which][same_f ? 1 : 0]);
        ia_addmenu(win, IA_THROW_OBJ, 't', buf);
    }

    /* T: take off armor, tip carried container */
    if (otmp->owornmask & W_ARMOR)
        ia_addmenu(win, IA_TAKEOFF_OBJ, 'T', "Take off this armor");
    if ((Is_container(otmp) && (Has_contents(otmp) || !otmp->cknown))
        || (otmp->otyp == HORN_OF_PLENTY && (otmp->spe > 0 || !otmp->known)))
        ia_addmenu(win, IA_TIP_CONTAINER, 'T',
                   "Tip all the contents out of this container");

    /* V: invoke */
    if ((otmp->otyp == FAKE_AMULET_OF_YENDOR && !otmp->known)
        || otmp->oartifact || objects[otmp->otyp].oc_unique
        /* non-artifact crystal balls don't have any unique power but
           the #invoke command lists them as likely candidates */
        || otmp->otyp == CRYSTAL_BALL)
        ia_addmenu(win, IA_INVOKE_OBJ, 'V',
                   "Try to invoke a unique power of this object");

    /* w: wield, hold in hands, works on everything but with different
       advice text; not mentioned for things that are already wielded */
    if (otmp == uwep || cantwield(gy.youmonst.data)) {
        ; /* either already wielded or can't wield anything; skip 'w' */
    } else if (otmp->oclass == WEAPON_CLASS || is_weptool(otmp)
               || is_wet_towel(otmp) || otmp->otyp == HEAVY_IRON_BALL) {
        Strcpy(buf, (otmp->quan > 1L) ? N_("Wield this stack as your weapon")
                                      : N_("Wield this item as your weapon"));
        ia_addmenu(win, IA_WIELD_OBJ, 'w', buf);
    } else if (otmp->otyp == TIN_OPENER) {
        ia_addmenu(win, IA_WIELD_OBJ, 'w',
                   "Wield the tin opener to easily open tins");
    } else if (!already_worn) {
        /* originally this was using "hold this item in your hands" but
           there's no concept of "holding an item", plus it unwields
           whatever item you already have wielded so use "wield this item" */
        Sprintf(buf, (otmp->quan > 1L) ? _("Wield this stack in your %s")
                                       : _("Wield this item in your %s"),
                /* only two-handed weapons and unicorn horns care about
                   pluralizing "hand" and they won't reach here, but plural
                   sounds better when poly'd into something with "claw" */
                makeplural(body_part(HAND)));
        ia_addmenu(win, IA_WIELD_OBJ, 'w', buf);
    }

    /* W: wear armor */
    if (!already_worn) {
        if (otmp->oclass == ARMOR_CLASS) {
            /* if 'otmp' is worn we skip 'W' (and show 'T' above instead);
               if it isn't, we either show "W - wear this" if otmp's slot
               isn't populated, or "W - [already wearing <simple-armor>]";
               for the latter, picking 'W' will fail but we don't want to
               omit 'W' in this situation */
            long Wmask = armcat_to_wornmask(objects[otmp->otyp].oc_armcat);
            struct obj *o = wearmask_to_obj(Wmask);

            if (!o)
                Strcpy(buf, N_("Wear this armor"));
            else if (i18n_active())
                Sprintf(buf, _("[already wearing %s]"),
                        i18n_an_ctx("noun", armor_simple_name(o)));
            else
                Sprintf(buf, "[already wearing %s]", an(armor_simple_name(o)));

            ia_addmenu(win, IA_WEAR_OBJ, 'W', buf);
        }
    }

    /* x: Swap main and readied weapon */
    if (otmp == uwep && uswapwep)
        ia_addmenu(win, IA_SWAPWEAPON, 'x',
                   "Swap this with your alternate weapon");
    else if (otmp == uwep)
        ia_addmenu(win, IA_SWAPWEAPON, 'x',
                   "Ready this as an alternate weapon");
    else if (otmp == uswapwep)
        ia_addmenu(win, IA_SWAPWEAPON, 'x',
                   "Swap this with your main weapon");

    /* this is based on TWOWEAPOK() in wield.c; we don't call can_two_weapon()
       because it is very verbose; attempting to two-weapon might be rejected
       but we screen out most reasons for rejection before offering it as a
       choice */
#define MAYBETWOWEAPON(obj) \
    ((((obj)->oclass == WEAPON_CLASS)                           \
      ? !(is_launcher(obj) || is_ammo(obj) || is_missile(obj))  \
      : is_weptool(obj))                                        \
     && !bimanual(obj))

    /* X: Toggle two-weapon mode on or off */
    if ((otmp == uwep || otmp == uswapwep)
        /* if already two-weaponing, no special checks needed to toggle off */
        && (u.twoweap
        /* but if not, try to filter most "you can't do that" here */
            || (could_twoweap(gy.youmonst.data) && !uarms
                && uwep && MAYBETWOWEAPON(uwep)
                && uswapwep && MAYBETWOWEAPON(uswapwep)))) {
        Strcpy(buf, u.twoweap ? N_("Toggle two-weapon combat off")
                              : N_("Toggle two-weapon combat on"));
        ia_addmenu(win, IA_TWOWEAPON, 'X', buf);
    }

#undef MAYBETWOWEAPON

    /* z: Zap wand */
    if (otmp->oclass == WAND_CLASS)
        ia_addmenu(win, IA_ZAP_OBJ, 'z',
                   "Zap this wand to release its magic");

    /* ?: Look up an item in the game's database */
    if (ia_checkfile(otmp)) {
        Strcpy(buf, (otmp->quan > 1L) ? N_("Look up information about these")
                                      : N_("Look up information about this"));
        ia_addmenu(win, IA_WHATIS_OBJ, '/', buf);
    }

    Sprintf(buf, _("Do what with %s?"), the(cxname(otmp)));
#ifdef NHI18N
    i18n_contract(buf);
#endif
    end_menu(win, buf);

    n = select_menu(win, PICK_ONE, &selected);

    if (n > 0) {
        act = selected[0].item.a_int;
        free((genericptr_t) selected);

        itemactions_pushkeys(otmp, act);
    }
    destroy_nhwindow(win);

    /* finish the 'i' command:  no time elapses and cancelling without
       selecting an action doesn't matter */
    return ECMD_OK;
}

RESTORE_WARNING_FORMAT_NONLITERAL

/*iactions.c*/
