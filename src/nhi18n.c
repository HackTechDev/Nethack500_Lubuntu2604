/* NetHack 5.0  nhi18n.c */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * Run-time translation of game messages (see nhi18n.h).
 *
 * The catalog for the language chosen with the 'language' option is a
 * GNU .mo file, <lang>.mo in HACKDIR, compiled by msgfmt from po/<lang>.po.
 * It is read into memory as a whole and checked once when loaded, so that
 * lookups can't read outside of it; msgfmt sorts the original strings,
 * so a lookup is a binary search.
 *
 * Most translated strings are printf formats.  A translation whose
 * conversions don't consume the same argument types in the same order
 * as the original text (a translation may reorder them with "%2$s") would
 * make the game crash, so it is ignored and the original text is used.
 * Leaving out trailing arguments is harmless (printf ignores extra
 * arguments), so the original can pass last an argument that some
 * languages don't need.
 */

#include "hack.h"

#ifdef NHI18N

#define MO_MAGIC 0x950412deUL
#define MO_HEADERSIZE 28L
#define FMT_MAXCONV 16 /* conversions handled in one translated format */
#define FMT_CODELEN 4  /* length modifier + conversion letter + NUL */

enum fmtcheck { fmt_unchecked = 0, fmt_ok, fmt_bad };

struct mo_catalog {
    unsigned char *data; /* the whole .mo file */
    long size;
    unsigned long count;    /* number of original/translation pairs */
    unsigned long origtab;  /* offset of the table of original strings */
    unsigned long transtab; /* offset of the table of translations */
    boolean bigendian;      /* byte order of the file */
    char *fmtcheck;         /* enum fmtcheck for each translation */
};

staticfn unsigned long mo_u32(struct mo_catalog *, unsigned long);
staticfn boolean mo_strings_ok(struct mo_catalog *, unsigned long);
staticfn boolean mo_load(struct mo_catalog *, const char *);
staticfn void mo_free(struct mo_catalog *);
staticfn const char *mo_string(struct mo_catalog *, unsigned long,
                               unsigned long);
staticfn int fmt_conversions(const char *, char[FMT_MAXCONV][FMT_CODELEN]);
staticfn boolean fmt_compatible(const char *, const char *);
staticfn const char *mo_lookup(const char *);

static struct mo_catalog catalog;
static char cur_language[8] = "en";

/* 32-bit number at offset off of the file, in the file's byte order */
staticfn unsigned long
mo_u32(struct mo_catalog *cat, unsigned long off)
{
    const unsigned char *p = cat->data + off;

    if (cat->bigendian)
        return ((unsigned long) p[0] << 24) | ((unsigned long) p[1] << 16)
               | ((unsigned long) p[2] << 8) | (unsigned long) p[3];
    return ((unsigned long) p[3] << 24) | ((unsigned long) p[2] << 16)
           | ((unsigned long) p[1] << 8) | (unsigned long) p[0];
}

/* check that the table of strings at offset tab describes count strings
   lying inside the file, each followed by its terminating NUL */
staticfn boolean
mo_strings_ok(struct mo_catalog *cat, unsigned long tab)
{
    unsigned long i, len, off, size = (unsigned long) cat->size;

    if (tab > size || cat->count > (size - tab) / 8)
        return FALSE;
    for (i = 0; i < cat->count; ++i) {
        len = mo_u32(cat, tab + i * 8);
        off = mo_u32(cat, tab + i * 8 + 4);
        if (off >= size || len >= size - off || cat->data[off + len])
            return FALSE;
    }
    return TRUE;
}

/* read and check the catalog file fname; FALSE if it can't be used */
staticfn boolean
mo_load(struct mo_catalog *cat, const char *fname)
{
    FILE *fp;
    unsigned long magic;

    (void) memset((genericptr_t) cat, 0, sizeof *cat);
    if (!(fp = fopen_datafile(fname, "rb", HACKPREFIX)))
        return FALSE;
    if (fseek(fp, 0L, SEEK_END) == 0 && (cat->size = ftell(fp)) > 0
        && cat->size >= MO_HEADERSIZE && fseek(fp, 0L, SEEK_SET) == 0) {
        cat->data = (unsigned char *) alloc((unsigned) cat->size);
        if (fread(cat->data, 1, (size_t) cat->size, fp)
            != (size_t) cat->size) {
            mo_free(cat);
        }
    }
    (void) fclose(fp);
    if (!cat->data)
        return FALSE;

    cat->bigendian = FALSE;
    magic = mo_u32(cat, 0UL);
    if (magic != MO_MAGIC) {
        cat->bigendian = TRUE;
        magic = mo_u32(cat, 0UL);
    }
    /* only major revision 0 of the format is supported */
    if (magic != MO_MAGIC || (mo_u32(cat, 4UL) >> 16) != 0) {
        mo_free(cat);
        return FALSE;
    }
    cat->count = mo_u32(cat, 8UL);
    cat->origtab = mo_u32(cat, 12UL);
    cat->transtab = mo_u32(cat, 16UL);
    if (!mo_strings_ok(cat, cat->origtab)
        || !mo_strings_ok(cat, cat->transtab)) {
        mo_free(cat);
        return FALSE;
    }
    if (cat->count) {
        cat->fmtcheck = (char *) alloc((unsigned) cat->count);
        (void) memset((genericptr_t) cat->fmtcheck, fmt_unchecked,
                      (size_t) cat->count);
    }
    return TRUE;
}

staticfn void
mo_free(struct mo_catalog *cat)
{
    if (cat->data)
        free((genericptr_t) cat->data);
    if (cat->fmtcheck)
        free((genericptr_t) cat->fmtcheck);
    (void) memset((genericptr_t) cat, 0, sizeof *cat);
}

/* string i of the table at offset tab (already checked by mo_load) */
staticfn const char *
mo_string(struct mo_catalog *cat, unsigned long tab, unsigned long i)
{
    return (const char *) cat->data + mo_u32(cat, tab + i * 8 + 4);
}

/* collect in conv[] the length modifier and conversion letter of each
   printf conversion of fmt, indexed by the argument it consumes;
   returns the number of arguments, or -1 for a format we won't accept
   (%n, '*' along with "%N$", a mix of positional and plain conversions,
   too many conversions, unknown conversions) */
staticfn int
fmt_conversions(const char *fmt, char conv[FMT_MAXCONV][FMT_CODELEN])
{
    const char *p, *q;
    char code[FMT_CODELEN];
    int i, n = 0, pos, positional = -1;

    for (i = 0; i < FMT_MAXCONV; ++i)
        conv[i][0] = '\0';
    for (p = fmt; *p; ++p) {
        if (*p != '%')
            continue;
        if (*++p == '%')
            continue;
        /* "%N$" selects argument N */
        pos = 0;
        for (q = p; digit(*q); ++q)
            pos = pos * 10 + (*q - '0');
        if (q > p && *q == '$')
            p = q + 1;
        else
            pos = 0;
        if (positional == -1)
            positional = (pos > 0);
        else if (positional != (pos > 0))
            return -1;
        while (*p && strchr("-+ #0'", *p))
            ++p;
        for (i = 0; i < 2; ++i) { /* width, then precision */
            if (*p == '*') {
                /* '*' consumes an int argument of its own */
                if (positional || n >= FMT_MAXCONV)
                    return -1;
                Strcpy(conv[n++], "*");
                ++p;
            }
            while (digit(*p))
                ++p;
            if (i == 0 && *p == '.')
                ++p;
            else
                break;
        }
        i = 0;
        while (*p && strchr("hlLqjzt", *p) && i < FMT_CODELEN - 2)
            code[i++] = *p++;
        if (!*p || !strchr("diouxXeEfFgGaAcsp", *p))
            return -1;
        code[i++] = *p;
        code[i] = '\0';
        if (positional) {
            if (pos > FMT_MAXCONV
                || (conv[pos - 1][0] && strcmp(conv[pos - 1], code)))
                return -1;
            Strcpy(conv[pos - 1], code);
            n = max(n, pos);
        } else {
            if (n >= FMT_MAXCONV)
                return -1;
            Strcpy(conv[n++], code);
        }
    }
    for (i = 0; i < n; ++i)
        if (!conv[i][0]) /* an argument skipped by "%N$" conversions */
            return -1;
    return n;
}

/* can translation be used as a format in place of msgid? */
staticfn boolean
fmt_compatible(const char *msgid, const char *translation)
{
    char convid[FMT_MAXCONV][FMT_CODELEN], convtr[FMT_MAXCONV][FMT_CODELEN];
    int i, nid, ntr;

    nid = fmt_conversions(msgid, convid);
    if (nid < 0) /* not a format we understand: plain text only */
        return !strchr(translation, '%');
    ntr = fmt_conversions(translation, convtr);
    if (ntr < 0 || ntr > nid)
        return FALSE;
    for (i = 0; i < ntr; ++i)
        if (strcmp(convid[i], convtr[i]))
            return FALSE;
    return TRUE;
}

/* translation of key (a msgid, or "context\004msgid") that can be used
   in place of msgid; Null if there is none */
staticfn const char *
mo_lookup(const char *key)
{
    unsigned long lo, hi, mid;
    const char *translation, *msgid;
    int cmp;

    lo = 0, hi = catalog.count;
    while (lo < hi) {
        mid = lo + (hi - lo) / 2;
        cmp = strcmp(key, mo_string(&catalog, catalog.origtab, mid));
        if (cmp < 0) {
            hi = mid;
        } else if (cmp > 0) {
            lo = mid + 1;
        } else {
            translation = mo_string(&catalog, catalog.transtab, mid);
            if (catalog.fmtcheck[mid] == fmt_unchecked) {
                boolean ok;

                if ((msgid = strchr(key, '\004')) != 0)
                    ++msgid;
                else
                    msgid = key;
                ok = (*translation && fmt_compatible(msgid, translation));
                catalog.fmtcheck[mid] = ok ? fmt_ok : fmt_bad;
            }
            return (catalog.fmtcheck[mid] == fmt_ok) ? translation : 0;
        }
    }
    return (const char *) 0;
}

/* translation of msgid in the current language, or msgid itself */
const char *
nh_gettext(const char *msgid)
{
    const char *translation;

    if (!catalog.data || !msgid || !*msgid)
        return msgid;
    translation = mo_lookup(msgid);
    return translation ? translation : msgid;
}

/* translation of msgid for context ctx; without one, that of msgid */
const char *
nh_pgettext(const char *ctx, const char *msgid)
{
    char key[BUFSZ];
    const char *translation;

    if (!catalog.data || !msgid || !*msgid)
        return msgid;
    if (strlen(ctx) + strlen(msgid) + 2 <= sizeof key) {
        Sprintf(key, "%s\004%s", ctx, msgid);
        if ((translation = mo_lookup(key)) != 0)
            return translation;
    }
    return nh_gettext(msgid);
}

/* translation of msgid for context ctx, or Null if the catalog has none;
   for data rather than text, such as the grammatical gender of a noun */
const char *
i18n_lookup(const char *ctx, const char *msgid)
{
    char key[BUFSZ];

    if (!catalog.data || !*msgid
        || strlen(ctx) + strlen(msgid) + 2 > sizeof key)
        return (const char *) 0;
    Sprintf(key, "%s\004%s", ctx, msgid);
    return mo_lookup(key);
}

/* are messages being translated? */
boolean
i18n_translating(void)
{
    return catalog.data ? TRUE : FALSE;
}

/* switch to language lang ("en" or a code like "fr" or "pt_BR" whose
   catalog is <lang>.mo); FALSE, keeping the current language, if lang
   isn't a valid code or its catalog can't be loaded */
boolean
i18n_set_language(const char *lang)
{
    struct mo_catalog newcat;
    char fname[sizeof cur_language + sizeof ".mo"];
    size_t len = strlen(lang);

    if (!(len == 2 || (len == 5 && lang[2] == '_'
                       && isupper((uchar) lang[3])
                       && isupper((uchar) lang[4])))
        || !islower((uchar) lang[0]) || !islower((uchar) lang[1]))
        return FALSE;
    if (!strcmp(lang, "en")) {
        mo_free(&catalog);
    } else {
        Sprintf(fname, "%s.mo", lang);
        if (!mo_load(&newcat, fname))
            return FALSE;
        mo_free(&catalog);
        catalog = newcat;
    }
    Strcpy(cur_language, lang);
    return TRUE;
}

/* code of the current language */
const char *
i18n_language(void)
{
    return cur_language;
}

#endif /* NHI18N */

/*nhi18n.c*/
