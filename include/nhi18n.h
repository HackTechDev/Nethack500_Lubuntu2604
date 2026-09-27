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
 *  N_("text")  only marks "text" for extraction by xgettext (for strings
 *              in static tables); translate it with _() where it is used.
 *
 * Catalogs are GNU .mo files compiled from po/<lang>.po and installed in
 * HACKDIR as <lang>.mo.  A translation whose printf conversions do not
 * match those of the original text is ignored (see nhi18n.c).
 */

#ifdef NHI18N
#define _(msgid) nh_gettext(msgid)
#else
#define _(msgid) (msgid)
#endif
#define N_(msgid) msgid

#endif /* NHI18N_H */
