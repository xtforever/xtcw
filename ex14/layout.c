@proc layout($, int *max_w, int *max_h, int do_draw )
{
    TRACE(2,"layout" );
    XftGlyphFontSpec *specs;
    XGlyphInfo *info;

    lay.line_info_m = m_create(20,sizeof(*line_info_ent));
    lay.glyph_spec_m = m_create(20, sizeof( XftGlyphFontSpec) );
    lay.text_code_m = m_create(20, sizeof( *text_code_ent ));

    int old_w = *max_w;
    int old_h = *max_h;

    int p,i;
    int xp=0,yp=0;
    int line_height = 0;
    int max_ascent = 0;
    int line_start = -1;

    int wrap_x = 10000;
    /* den string in unicode 32 bit umwandeln */
    /* und ende der zeilen finden
       umbruch bei wrap_x
    */
    *max_w = 0; *max_h = 0;
    for(p=0; (unicode = m_utf8char($str_m, &p)) != -1;)
    {
            if( unicode == '\n' ) {
                if( line_start == -1 ) continue;
                line_info_ent = m_add( line_info_m );
                line_info_ent->endp = m_len(glyph_spec_m) -1;
                line_info_ent->height = line_height;
                for(i=line_start;i<=m_len(glyph_spec_m) -1;i++)
                    {
                        specs = mls( glyph_spec_m, i );
                        specs->y += max_ascent;
                    }

                yp += line_height;
                if( xp >*max_w ) *max_w = xp;
                if( yp >*max_h ) *max_h = yp;
                xp = 0;
                line_height = 0;
                line_start = -1;
            }
            else {

                if( xp==0 && line_start<0 ) line_start = m_len(glyph_spec_m );
                specs = m_add( glyph_spec_m );
                text_code_ent = m_add( text_code_m );
                text_code_ent->unicode = unicode;
                info = & text_code_ent->info;
                unicode_to_glyphspec($, unicode, specs, info );
                specs->x = xp;
                specs->y = yp;
                xp += info->xOff;
                yp += info->yOff;
                int h = specs->font->height;
                if( h > line_height ) line_height = h;
                h = specs->font->ascent;
                if( h > max_ascent ) max_ascent = h;
            }


        TRACE(4,"size: %dx%d", *max_w, *max_h );

        int y_off, x_off;
        int line_end = 0;
        int cp, cp_opt, cp_end;
        if(do_draw && old_w && old_w < *max_w ) { /* enable word wrap */
            /* goto end of line,
               check line width
               search backward for breakpoint
            */
            y_off = 0;
            struct line_info_st *ent;
            p=0;
            while( p < m_len(line_info_m) ) {
                if( p ) {
                    ent = mls(line_info_m, p-1);
                    line_end = ent->endp+1;
                }
                ent = mls(line_info_m, p);
                p++;
                TRACE(4, "checking line %d", p );
                x_off = 0;
                if( ent->endp < 0 ) continue;
                specs = mls( glyph_spec_m, ent->endp );
                if( specs->x < old_w ) continue;
                /* search backward until glyph line_end is reached */
                cp = ent->endp; cp_opt = -1;
                while( cp-- > line_end ) {
                    specs = mls( glyph_spec_m, cp );
                    text_code_ent = mls(text_code_m, cp);
                    if(cp_opt && text_code_ent->unicode == 32 ) /* break here */
                        {
                            cp_opt = cp; break;
                        }
                    if(cp_opt<0) { if( specs->x < old_w ) cp_opt = cp-1; }
                }
                /* break not found! why not? */
                if( cp_opt < 0 ) /* break not possible */
                    continue;
                TRACE(4, "break at %d", cp_opt );

                /* ab hier nur line break */
                cp = cp_opt;
                cp_end = ent->endp;
                ent->endp = cp;
                y_off = ent->height;
                line_height = ent->height;
                /* neue zeile */
                cp++;
                specs = mls( glyph_spec_m, cp );
                x_off = specs->x;
                m_ins(line_info_m, p, 1);
                ent = mls(line_info_m, p);

                /* height und endp bestimmen */
                ent->height = line_height;
                ent->endp = cp_end;

                /* alle koordinaten umrechnen */
                while( cp <= cp_end ) {
                    specs = mls( glyph_spec_m, cp );
                    specs->x -= x_off;
                    specs->y += y_off;
                    text_code_ent = mls(text_code_m, cp);
                    if( text_code_ent->info.height > line_height )
                        line_height = text_code_ent->info.height;
                    cp++;
                }
                /* y - koordinaten des restlichen texts */
                cp_end = m_len(text_code_m);
                while( cp < cp_end ) {
                    specs = mls( glyph_spec_m, cp );
                    specs->y += y_off;
                    cp++;
                }
            }

            m_foreach( glyph_spec_m, p, specs ) {
                printf("%dx%d ",specs->x, specs->y );
            }
            printf("\n\n");
        }



        if( do_draw ) {
            XftDrawGlyphFontSpec( $draw, $xcol+FG,
                                  m_buf( glyph_spec_m ),
                                  m_len( glyph_spec_m ) );
        }
        m_free( glyph_spec_m );
        m_free( text_code_m );
        m_free( line_info_m );
}
