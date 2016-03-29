#include "xprint2.h"
#include "xlayout.h"
#define XP2LABEL(x,n) ((struct xp2_label *)mls(x->label,n)) 

xp2_t *xp2_init(Display *dpy)
{
    xp2_t *xp = malloc( sizeof(xp2_t));
    xp->dpy = dpy;
    xp->attr  = m_create(5,sizeof( struct xp2_attr  ) );
    xp->label = m_create(5,sizeof( struct xp2_label ) );    
    return xp;
}
void xp2_set_window( xp2_t *xp, int x,int y,int w, int h )
{
    rect_t r = { x: x, y: y, w: w, h: h };
    xp->win = r;
}
void xp2_set_drawable( xp2_t *xp, XftDraw *draw)
{
    xp->draw = draw;
}

void xp2_free( xp2_t *xp )
{
    struct xp2_label *l;
    int i;
    m_foreach( xp->label, i, l )
        {
            m_free( l->glyph );
        }

    m_free( xp->label );
    m_free( xp->attr  );
    free( xp );
}

void xp2_attr(xp2_t *x, int num, XftColor *fg,  XftColor *bg, XftFont *font )
{
    if( num >= m_len(x->attr) ) m_setlen( x->attr, num+1 );
    else if(num<0 ) num = m_new(x->attr,1);
    struct xp2_attr *a = mls( x->attr, num );
    a->fg = fg; a->bg = bg; a->font = font;
}

int xp2_new_attr( xp2_t *x, XftColor *fg,  XftColor *bg, XftFont *font )
{
  int num = m_len(x->attr);
  xp2_attr(x,num,fg,bg,font);
  return num;
}

int xp2_label( xp2_t *x, int num, int attr )
{
    if( num >= m_len(x->label) ) m_setlen( x->label, num+1 );
    else if(num<0 ) num = m_new(x->label,1);
    struct xp2_label *l = mls( x->label, num );
    l->attr = attr;
    return num;
}


int xp2_new_label( xp2_t *x, int attr )
{
    int label_num = m_new( x->label, 1 );
    struct xp2_label *l = mls( x->label, label_num );
    l->attr = attr;
    return label_num;
}


void xp2_update_label_str(xp2_t *x, int num, char *s, int measure )
{
  int tmp = s_printf( 0,0,s );
  xp2_update_label(x,num,tmp,measure);
  m_free(tmp);
}

int xp2_setup_label( xp2_t *x, char *str, int attr, int other_label, int place )
{
    int l = xp2_new_label( x, attr );
    xp2_update_label_str( x, l, str, 1 );
    /* zusätzliche initialisierung */
    struct xp2_label *pl = XP2LABEL(x,l);
    pl->other_label = other_label;
    pl->place = place;
    return l;
}



void xp2_layout_label( xp2_t *x, int label )
{
    struct xp2_label *pl = XP2LABEL(x,label);
    xp2_place( x, label, pl->other_label, pl->place );
}


void xp2_measure_label( xp2_t *x, int num )
{
    XGlyphInfo extents;
    struct xp2_label *l = mls( x->label, num );
    struct xp2_attr *a = mls( x->attr, l->attr );
    
    XftGlyphExtents( x->dpy, a->font, 
                     m_buf(l->glyph), m_len(l->glyph), & extents );
    l->text_width = extents.width - extents.x +2;
    l->text_height = extents.height +2;
}

void xp2_update_label(xp2_t *x, int num, int str, int measure )
{

    struct xp2_label *l = mls( x->label, num );
    struct xp2_attr *a = mls( x->attr, l->attr );
    if( l->glyph > 0 ) m_clear( l->glyph );
    else l->glyph = m_create( 10, sizeof(int));
    get_glyphs( x->dpy, a->font, l->glyph, str );

    if( measure ) xp2_measure_label(x,num);
}

int xp2_place(xp2_t*x, int l0, int l1, int where )
{
    rect_t *r0,*r1;
    
    r0 =& XP2LABEL(x,l0)->r;
    if( l1 < 0 ) r1 = & x->win; else r1 =& XP2LABEL(x,l1)->r;
    
    placer( r0, r1, where );
    return l0;
}

void placer( rect_t *r0, rect_t *r1, int where )
{
    if( where & LabelGravity )
        {
            switch( where & LabelGravity ) {
                
            case  LabelCenter:
                r0->x = (r1->w - r0->w) / 2;
                r0->y = (r1->h - r0->h) / 2;
                break;
                
            case  LabelWest:
                r0->x = r1->x;
                r0->y = (r1->h - r0->h)/2;
                break;
            case  LabelEast:
                r0->x = r1->x + (r1->w - r0->w);
                r0->y = (r1->h - r0->h)/2;
                break;
            case  LabelNorthWest:
                r0->x = r1->x;
                r0->y = r1->y;
                break;
            case  LabelNorth:
                r0->x = r1->x + r1->w/2 - r0->w/2;
                r0->y = r1->y;
                break;
                
            }
            return;
        }
    
    if( where & LabelRightOf ) /* r0 rightof r1 */
        r0->x = r1->x + r1->w;
    if( where & LabelLeftOf ) /* r0 leftof r1 */
        r0->x = r1->x - r0->w;
    if( where & LabelBelow ) /* r0 below r1 */
        r0->y = r1->y + r1->h;
    if( where & LabelAbove ) /* r0 above r1 */
        r0->y = r1->y - r0->h;
    
}


void xp2_draw_label(xp2_t*x, int label_num, int clear_background )
{
  struct xp2_label *l = mls( x->label, label_num );
  struct xp2_attr  *a  =mls( x->attr, l->attr );  
  if( clear_background ) 
      XftDrawRect( x->draw, a->bg, l->r.x, l->r.y, l->r.w, l->r.h );

  XftDrawGlyphs(
                x->draw, a->fg, a->font, l->r.x, l->r.y + a->font->ascent, 
                m_buf(l->glyph), m_len(l->glyph));
}

rect_t *xp2_get_rect(xp2_t *x, int num)
{  
  struct xp2_label *l = mls( x->label, num );
  return & l->r;
}

void xp2_get_bounds( xp2_t *x, rect_t *b )
{
  struct xp2_label *label; 
  rect_t *l;
  int i;
  b->x = INT_MAX; b->w = 0;
  b->y = INT_MAX; b->h = 0;
  m_foreach( x->label, i, label ) {
    l = &label->r;
    b->x = Min( b->x, l->x );
    b->y = Min( b->y, l->y );
    b->w = Max( b->w, l->w );
    b->h = Max( b->h, l->h );
  }
}
