#include <string.h>
#include "id3read.h"


void m_puts(int str)
{
    int len = m_len(str);
    if( !str || !len ) goto newline;
    if( CHAR(str, len-1 ) == 0 ) len--;
    if( len ) fwrite( m_buf(str), len,1, stdout );

 newline:
    putchar(10);
}


int m_dirname(int m, char *s)
{
    if( m ) m_clear(m);
    else m = m_create(10,1);

    char *p = strrchr( s, '/' );
    if( p )
        m_write( m, 0, s, p-s );
    else m_putc(m, '.' );
    m_putc( m, 0 );
    return m;
}

int m_basename(int m, char *s)
{
    if( m ) m_clear(m);
    else m = m_create(10,1);

    int len = strlen( s );
    char *p = strrchr( s, '/' );
    if( p ) {
        int offset = (p-s)+1;
        if( len - offset > 0 ) {
            m_write( m, 0, s + offset, len - offset +1 );
            return m;
        }
    }
    m_putc(m, 0 );
    return m;
}



/* first sort by album, second sort by track number */
int id3db_cmp_album_track_qsort(const void *a0, const void *b0, void *c0)
{
    const struct id3ent *a = a0;
    const struct id3ent *b = b0;
    int t0,t1;
    char *key = "album";
    char *s0;
    char *s1;
    s0 = v_get( a->vs, key, 1 );
    s1 = v_get( b->vs, key, 1 );

    int x = strcasecmp(s0,s1);
    if( !x ) {
        key = "track";
        t0 = atoi(v_get( a->vs, key, 1 ));
        t1 = atoi(v_get( b->vs, key, 1 ));
        x = t0 - t1;
    }
    return x;
}




void print_all(int db)
{
    qsort_r( mls(db,0), m_len(db), m_width(db),
             id3db_cmp_album_track_qsort, "" );

    char *cur_album = "";
    struct id3ent *d;
    int p;
    int tmp = m_create(250,1);

    m_foreach(db, p, d )
	{
            char *album = v_get(d->vs, "album", 1 );
            char *title = v_get(d->vs, "title", 1 );
            char *artist = v_get(d->vs, "artist", 1 );

            if( is_empty(album) ) continue;
            if( strcmp(album,cur_album) != 0 ) {
                printf( "\n" ); m_puts( m_dirname(tmp, d->path) );

                printf( "%s\n", album  );
                printf( "%s\n", artist );
                cur_album = album;
            }
            printf( " "); m_puts( m_basename(tmp, d->path));
            printf( " %s\n", title );
        }
    m_free(tmp);
}



int id3db_cmp_qsort(const void *a0, const void *b0, void *c0)
{
    const struct id3ent *a = a0;
    const struct id3ent *b = b0;
    char *key = c0;
    char *s0;
    char *s1;
    s0 = v_get( a->vs, key, 1 );
    s1 = v_get( b->vs, key, 1 );
    return strcasecmp(s0,s1);
}


void id3db_sort(int db, char *key )
{
    qsort_r( mls(db,0), m_len(db), m_width(db), id3db_cmp_qsort, key );
}

int main(int argc, char **argv)
{
    char *root;
    trace_level=2;
    m_init();

    if( argc != 2 ) {
        ERR("you must give a pathname for music with id3 tags");
    }

    root = argv[1];

    int db = id3db_create();
    id3db_import(db, root );


    /*
    id3db_sort(db, "album");
    id3db_print(db);

    id3db_sort(db, "title");
    id3db_print(db);
    */

    print_all(db);


    id3db_free(db);

    m_destruct();
    return 0;
}
