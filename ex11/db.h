#ifndef DB_H
#define DB_H

#include <stdio.h>

struct d3_album {
    int name;
    int path;
    int interpret;
    int song_m;
};

struct d3_song {
    int name;
    int file;
};

int d3_open( char *file );
void d3_close(int h);
char* d3_album_name(int h, int n);
char* d3_album_interpret(int h, int n);
int d3_album_song_m(int n);
int d3_song_name(int n, int p);
int d3_album_song_filename( int h, int n, int p );

int m_readln(int buf, FILE *fp);


struct d3_album *album(int i,int p);

#endif
