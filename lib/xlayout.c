#include "xlayout.h"



void get_glyphs(Display*dpy, XftFont *pub, int m_glyphs, int m );
static void va_render( struct xlprint_int *p, va_list ap );

void xlprint_free( struct xlprint_int *p )
{
    if( p->buf ) m_free( p->buf );
    if( p->glyphs ) m_free(p->glyphs);
    p->buf = 0; p->glyphs = 0;
}

void xlprint_init( struct xlprint_int *p, ... )
{
    p->buf = m_create( 10, 1 );
    p->glyphs = m_create( 10, sizeof( FT_UInt ));
    va_list argptr;
    va_start(argptr, p );
    va_render( p,argptr );
    va_end(argptr);
    p->r.width = p->extents.width;
    p->r.height = p->dev->fnt->height;
}

static void va_render( struct xlprint_int *p, va_list ap )
{
    int len = vsnprintf( m_buf(p->buf), m_bufsize(p->buf), p->format, ap );
    if( len >= m_bufsize(p->buf) ) {
	m_setlen(p->buf, len+1 );
	len = vsnprintf( m_buf(p->buf), m_bufsize(p->buf), p->format, ap );
    }
    m_setlen(p->buf, len );
    get_glyphs( p->dev->dpy, p->dev->fnt, p->glyphs, p->buf );
    XftGlyphExtents( p->dev->dpy, p->dev->fnt, m_buf(p->glyphs), m_len(p->glyphs), & p->extents );
}


void xlprint_render( struct xlprint_int *p, ... )
{
    va_list argptr;
    va_start(argptr, p);
    va_render( p,argptr );
    va_end(argptr);
}

void xlprint_draw2( XRectangle *backgr, struct xlprint_int *p )
{
    draw_rect( p->dev->draw, p->dev->bg, backgr );
    int x = p->r.x;
    int w = p->extents.width;
    if( w < p->r.width ) x += p->r.width - w; /* right adjusted */

    XftDrawSetClipRectangles( p->dev->draw,0,0,& p->r, 1);
    draw_glyphs(p->dev->draw, p->dev->fg, p->dev->fnt,
		x + p->extents.x, p->r.y + p->dev->fnt->ascent, p->glyphs );
    XftDrawSetClip( p->dev->draw, 0 );
}

void xlprint_draw3( XRectangle *backgr, struct xlprint_int *p )
{
  


    draw_rect( p->dev->draw, p->dev->bg, backgr );
    int x = p->r.x;

    XftDrawSetClipRectangles( p->dev->draw,0,0,& p->r, 1);
    draw_glyphs(p->dev->draw, p->dev->fg, p->dev->fnt,
		x+ p->extents.x, p->r.y + p->dev->fnt->ascent, p->glyphs );
    XftDrawSetClip( p->dev->draw, 0 );
}


void xlprint_draw( struct xlprint_int *p )
{
    draw_rect( p->dev->draw, p->dev->bg, & p->r );
    int x = p->r.x;
    int w = p->extents.width;
    if( w < p->r.width ) x += p->r.width - w; /* right adjusted */

    XftDrawSetClipRectangles( p->dev->draw,0,0,& p->r, 1);
    draw_glyphs(p->dev->draw, p->dev->fg, p->dev->fnt,
		x + p->extents.x, p->r.y + p->dev->fnt->ascent, p->glyphs );
    XftDrawSetClip( p->dev->draw, 0 );
}

void draw_glyphs( XftDraw *draw, XftColor *col, XftFont *fnt, int x, int y,  int glyphs )
{
    if( m_len(glyphs) )
	XftDrawGlyphs(draw, col, fnt, x, y, m_buf(glyphs), m_len(glyphs));
}

void draw_rect( XftDraw *draw,  XftColor *col, XRectangle *r )
{
    XftDrawRect( draw, col, r->x, r->y, r->width, r->height );
}

void get_glyphs(Display	    *dpy,
		       XftFont	    *pub,
		       int m_glyphs,
		       int m )
{
    FT_UInt	    glyph;
    FcChar32	    ucs4;
    int		    i, l, len;

    len = m_len(m);
    m_clear( m_glyphs );
    i = 0;
    while(( len > 0 ) && CHAR(m,i) && (l = FcUtf8ToUcs4 ( mls(m,i), &ucs4, len)) > 0)
    {
	glyph = XftCharIndex (dpy, pub, ucs4);
	m_put( m_glyphs, &glyph );
	i += l;
	len -= l;
    }
}
