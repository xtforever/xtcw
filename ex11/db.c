#include "db.h"
#include "mls.h"


void *m_last(int m) {
    if( m_len(m) == 0 ) return NULL;
    return mls(m, m_len(m)-1 );
}

int d3_open( char *file );
void d3_close(int h);
char* d3_album_name(int h, int n);
char* d3_album_interpret(int h, int n);

int d3_song_name(int n, int p);
int d3_album_song_filename( int h, int n, int p );

struct d3_album *album(int i,int p) { return mls(i,p); }

char* d3_album_name(int h, int n)
{
    return m_buf(album(h,n)->name);
}

char* d3_album_interpret(int h, int n)
{
    return m_buf(album(h,n)->interpret);
}

static struct d3_album * add_album(int root, FILE *fp, int buf)
{
    struct d3_album album;
    album.path = m_dub( buf );
    album.name = m_create(10,1);
    album.interpret = m_create(10,1);
    album.song_m = m_create(10,sizeof(struct d3_song));
    if( m_fscan( album.name, 10, fp ) != 10 ) goto error;
    if( m_fscan( album.interpret, 10, fp ) != 10 ) goto error;
    m_put( root, &album );
    return m_last(root);

 error:
    m_free( album.path );
    m_free( album.name );
    m_free( album.interpret );
    return NULL;
}

static void add_song(struct d3_album *a, FILE *fp, int buf)
{
    struct d3_song song;
    song.file = s_copy( buf, 1, -1 );
    song.name = m_create(10,1);
    if( m_fscan( song.name, 10, fp ) != 10 ) goto error;
    m_put( a->song_m, &song );
    return;

 error:
    m_free(song.file);
    m_free(song.name);
}


int m_readln(int buf, FILE *fp)
{
    m_clear(buf);
    return m_fscan(buf, 10, fp);
}


int d3_open( char *file )
{
    int root = m_create(10,sizeof(struct d3_album));
    struct d3_album *album;
    FILE *fp = fopen(file, "r");
    int buf = m_create(50,1);
    while( m_readln(buf, fp) == 10 )
        {
            if( m_len(buf) < 2 ) continue;
            if( CHAR(buf,0) == '#' ) continue;
            if( CHAR(buf,0) == ' ' ) add_song(album, fp, buf);
            else album = add_album(root, fp, buf);

        }
    m_free(buf);
    return root;
}


void dmp_album(struct d3_album *a)
{
    printf("T: %s\nF: %s\nN: %d\n",
           (char*)m_buf(a->name),(char*) m_buf(a->path), m_len(a->song_m));
    struct d3_song *s;
    int p;
    m_foreach(a->song_m, p, s)
        printf( " %s %s\n",(char*) m_buf(s->name),(char*) m_buf(s->file) );
    printf("\n");
}


void d3_dump(int db)
{
    int p; struct d3_album *a;
    m_foreach(db,p,a)
        dmp_album(a);
}



void free_album(struct d3_album *a)
{
    struct d3_song *s;
    int p;
    m_foreach(a->song_m, p, s)
        {
            m_free(s->name);
            m_free(s->file);
        }
    m_free(a->song_m);
    m_free( a->path );
    m_free( a->name );
    m_free( a->interpret );

}


void d3_close(int db)
{
    int p; struct d3_album *a;
    m_foreach(db,p,a)
        free_album(a);
    m_free(db);
}


#ifdef test_db
int main()
{
    m_init();
    int db = d3_open( "database.dat" );
    d3_dump(db);
    d3_close(db);
    m_destruct();
}
#endif
