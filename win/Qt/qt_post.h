/* NetHack 5.0	qt_post.h	$NHDT-Date: 1597276832 2020/08/13 00:00:32 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.0 $ */

/*
 * qt_post.h -- reverse part of qt_pre.h.
 *
 * #include after <Qt.../Qt...>.
 */

/* restore the translation macro of nhi18n.h, undefined by qt_pre.h */
#ifndef _
#if defined(NHI18N) && !defined(SFCTOOL)
#define _(msgid) nh_gettext(msgid)
#else
#define _(msgid) (msgid)
#endif
#endif

#if defined(__cplusplus)
#ifdef __clang__
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif /* compiler-specific bits */
#endif /* __cplusplus */

/*qt_post.h*/
