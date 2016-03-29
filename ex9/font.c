#include "font.h"
#include <fontconfig/fontconfig.h>


int trace_font;
static int font_cache = 0;

struct fontinfo {
    char *name;
    int fnt_m;
    Display *dpy;
    int screen;
};

static void free_fontinfo(struct fontinfo*fi);
static struct fontinfo* add_fontinfo(char *name);
static XftFont *font_cache_find(struct fontinfo *fi, int unicode, int *glyphindex );
static XftFont *add_font(struct fontinfo *fi, int unicode, int *glyphindex );

XftFont *font_load(Display *dpy, int screen, char *name)
{
    TRACE(trace_font,"font:%s", name);
    if( !font_cache )
      font_cache = m_create( 10, sizeof( struct fontinfo ));

    struct fontinfo *fi = add_fontinfo(name);
    fi->dpy = dpy;
    fi->screen = screen;
    XftFont** fnt= m_add(fi->fnt_m);
    return (*fnt) = XftFontOpenName(dpy,screen,name);
}

XftFont *font_unicode_find(char *name, int unicode, int *glyphindex )
{
    TRACE(trace_font,"");
    XftFont *fnt;
    struct fontinfo *fi;
    int p;
    p=m_lookup_str(font_cache, name, 1);
    if( p < 0 ) return NULL;
    fi = mls(font_cache,p);

    fnt = font_cache_find( fi, unicode, glyphindex );
    if( fnt ) return fnt;

    return add_font( fi, unicode, glyphindex  );
}

/** close xftfont in font_cache, remove fontinfo structures
 */
void font_cache_flush(void)
{
    struct fontinfo *fi;
    int p;
    if( !font_cache ) return; /* there is nothing */
    m_foreach( font_cache, p, fi ) free_fontinfo(fi);
    m_free(font_cache);
    font_cache=0;
}

static void free_fontinfo(struct fontinfo*fi)
{
    TRACE(trace_font,"remove fontinfo %s", fi->name);
    XftFont **d;
    int p;
    if( fi->name ) { free(fi->name); fi->name=0; }
    if( fi->fnt_m ) {
        m_foreach( fi->fnt_m, p, d) XftFontClose(fi->dpy, *d);
        m_free(fi->fnt_m); fi->fnt_m = 0;
    }
}

static struct fontinfo* add_fontinfo(char *name)
{
    TRACE(trace_font,"fontname: %s", name);
    struct fontinfo *fi = m_add(font_cache);
    fi->fnt_m = m_create(2, sizeof(XftFont*));
    fi->name = strdup(name);
    return fi;
}

static XftFont *font_cache_find(struct fontinfo *fi, int unicode, int *glyphindex )
{
    TRACE(trace_font,"");
    XftFont **d;
    int p;
    m_foreach(fi->fnt_m, p, d) {
        *glyphindex = XftCharIndex(fi->dpy, *d, unicode);
        if( *glyphindex ) return *d;
    }
    return NULL;
}

static XftFont *add_font(struct fontinfo *fi, int unicode, int *glyphindex )
{
       TRACE(trace_font,"glyph was not found. adding new font to fontcache");
    XftFont *fnt;
    FcResult fcres;
    FcPattern *pattern, *fontpattern;
    // FcFontSet *fcsets[] = { NULL };
    // FcFontSet *font_set;
    FcCharSet *fccharset;

    pattern = FcNameParse((FcChar8 *)fi->name);
    fccharset = FcCharSetCreate(); /* create character set */
    FcCharSetAddChar(fccharset, unicode); /* add wanted character */
    FcPatternAddCharSet(pattern, FC_CHARSET, fccharset);
    /* we want scaleable fonts only */
    FcPatternAddBool(pattern, FC_SCALABLE, 1);
    fontpattern = XftFontMatch(fi->dpy,fi->screen, pattern, &fcres);
    if( trace_font >= trace_level ) FcPatternPrint(fontpattern);
    if(!fontpattern) ERR("");
    fnt=XftFontOpenPattern(fi->dpy,fontpattern);
    if(!fnt)ERR("");
    *glyphindex = XftCharIndex(fi->dpy, fnt, unicode);
    if( *glyphindex) m_put( fi->fnt_m, &fnt);
    else fnt = mls(fi->fnt_m,0);
    FcPatternDestroy(fontpattern);
    FcPatternDestroy(pattern);
    FcCharSetDestroy(fccharset);
    return fnt;
}
