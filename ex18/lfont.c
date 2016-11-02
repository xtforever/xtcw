#include "lfont.h"

static int lfont_list = 0;

static XftFont *load_font(Display *dpy,
                          char *family, char *style, int sl )
{
    //    Display *dpy = XtDisplay(w);
    int screen = DefaultScreen(dpy);
    XftPattern *fnt_pat;
    XftFont *fnt;
    XftPattern *match_pat;  // the best available match on the system
    XftResult match_result; // the result of our matching attempt

    fnt_pat = XftNameParse (family);
    if( ! is_empty(style ) )
            XftPatternAddString( fnt_pat, XFT_STYLE, style );

    XftPatternAddInteger( fnt_pat, XFT_SLANT, sl );

    match_pat = XftFontMatch(dpy,screen, fnt_pat, &match_result);
    XftPatternDestroy(fnt_pat);
    if( !match_pat ) ERR("no fonts");
    fnt = XftFontOpenPattern(dpy, match_pat);
    XftPatternDestroy(match_pat);
    return fnt;
}

static void load_default_fonts(Display *dpy, char *name, XftFont** fn)
{
    fn[bold] = load_font(dpy, name, "bold",0  );
    fn[italic] = load_font(dpy, name, "", 1 );
    fn[regular] = load_font(dpy, name, "",0 );
}

static void xft_draw_text( XftDraw *draw,
                           XftColor *col,
                           XftFont *fn,
                           int x, int y,
                           char *string, int len,
                           int *ret_x, int *ret_y)
{
    XGlyphInfo extents;
    Display *dpy = XftDrawDisplay(draw);
    XftTextExtentsUtf8(dpy, fn, (FcChar8*)string, len,
		       &extents);
    XftDrawStringUtf8( draw, col, fn,
                       x, y + fn->ascent,
                       (FcChar8 *)string, len );
    *ret_x = x + extents.xOff;
    *ret_y = y + extents.yOff;
}


int lfont_init(Display *dpy, char *family )
{
    if( !lfont_list ) {
        m_init();
        lfont_list = m_create(3,sizeof(struct lfont_st));
    }

    int key = 0;
    int lf = m_lookup_obj(lfont_list,&key,sizeof(int));
    struct lfont_st *fnt = mls(lfont_list,lf);
    memset(fnt,0, sizeof(*fnt) );
    fnt->dpy = dpy;
    fnt->def_font_name = family;
    load_default_fonts( dpy, family, fnt->xft_font_def );
    fnt->init = 1;
    return lf;
}

void lfont_free(int lf)
{
    struct lfont_st *fnt = mls(lfont_list, lf);
    if( fnt->init == 0 ) return;
    int i;
    for(i=0;i<LFONT_MAX; i++) XftFontClose(fnt->dpy, fnt->xft_font_def[i] );
    memset(fnt,0, sizeof(*fnt) );
}

void font_destroy(void)
{
    int i;
    for(i=0;i<m_len(lfont_list);i++)
        lfont_free(lfont_list);
    m_free(lfont_list);
    lfont_list=0;
}

void lfont_set_drawable( int lf, Drawable d )
{
    struct lfont_st *fnt = mls(lfont_list, lf);
    if( !fnt->init ) ERR("lfont %d not init", lf );
    Display *dpy = fnt->dpy;
    if( fnt->draw )
        XftDrawChange( fnt->draw, d );
    else
        fnt->draw = XftDrawCreate(dpy, d,
                                  DefaultVisual(dpy, DefaultScreen(dpy)), None);
    fnt->init = 2;
}

void lfont_draw_text( int lf,
                      enum lfont_names font,
                      int x, int y,
                      char *string, int len,
                      int *ret_x, int *ret_y)
{
    struct lfont_st *fnt = mls(lfont_list, lf);
    if( fnt->init != 2 ) ERR("lfont %d not init or no drawable", lf );
    xft_draw_text( fnt->draw,
                   &fnt->xft_color_def,
                   fnt->xft_font_def[font],
                   x,y,string,len,ret_x, ret_y);
}

struct lfont_st *lfont_get(int lf)
{
    struct lfont_st *fnt = mls(lfont_list, lf);
    if( !fnt->init ) ERR("lfont %d not init", lf );
    return fnt;
}

void lfont_set_color( int lf, XftColor *color )
{
    struct lfont_st *fnt = mls(lfont_list, lf);
    if( !fnt->init ) ERR("lfont %d not init", lf );
    memcpy(& (fnt->xft_color_def), color, sizeof(XftColor));
}

int lfont_height(int lf, enum lfont_names font )
{
    struct lfont_st *fnt = mls(lfont_list, lf);
    if( !fnt->init ) ERR("lfont %d not init", lf );
    XftFont *f = fnt->xft_font_def[font];
    return f->ascent + f->descent;
}
