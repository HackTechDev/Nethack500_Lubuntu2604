/* NetHack 5.0  nhi18n.h */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef NHI18N_H
#define NHI18N_H

/*
 * Translation of game messages.
 *
 *  _("text")   translate "text" at run time using the message catalog
 *              selected by the 'language' option; untranslated text and
 *              English play return "text" unchanged.
 *  C_("context", "text")
 *              like _("text"), but the translation may depend on context,
 *              such as "feminine" for the feminine form of an adjective;
 *              falls back to _("text") when there is none for context.
 *  N_("text")  only marks "text" for extraction by xgettext (for strings
 *              in static tables); translate it with _() where it is used.
 *  NC_("context", "text")
 *              same, for text translated with C_("context", ...).
 *  NCP_("context", "text", "plural text")
 *              same, for text with a plural form (see nh_npgettext()).
 *  i18n_active()
 *              TRUE when messages are being translated; for text that has
 *              to be composed differently then (English articles, etc.).
 *  I18N_FILE(fname)
 *              name of the data file to open for fname: its translation
 *              "fname.<lang>" if installed, else fname.
 *  i18n_has("text")
 *              TRUE when "text" has a translation in use.
 *  i18n_mon_fem(mon)
 *              TRUE when the translated name of monster mon is feminine,
 *              to pick C_("feminine", ...) forms of the rest of a message.
 *
 * Catalogs are GNU .mo files compiled from po/<lang>.po and installed in
 * HACKDIR as <lang>.mo.  A translation whose printf conversions do not
 * match those of the original text is ignored (see nhi18n.c); it may
 * leave out trailing arguments of the original, though.
 */

#if defined(NHI18N) && !defined(SFCTOOL)
#define _(msgid) nh_gettext(msgid)
#define C_(ctx, msgid) nh_pgettext(ctx, msgid)
#define i18n_active() i18n_translating()
#define i18n_mon_fem(mon) monnam_is_feminine(mon)
#define i18n_has(msgid) (nh_gettext(msgid) != (msgid))
#define I18N_FILE(fname) i18n_datafile(fname)
#else
#define _(msgid) (msgid)
#define C_(ctx, msgid) (msgid)
#define i18n_active() FALSE
#define i18n_mon_fem(mon) FALSE
#define i18n_has(msgid) FALSE
#define I18N_FILE(fname) (fname)
#endif
#define N_(msgid) msgid
#define NC_(ctx, msgid) msgid
#define NCP_(ctx, msgid, plural) msgid

#endif /* NHI18N_H */
