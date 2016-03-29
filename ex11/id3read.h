#ifndef ID3READ_H
#define ID3READ_H

#include "mls.h"
#include <tag_c.h>

#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int get_id3_tag(char *path);
void id3db_import( int db, char *root );
void id3db_free(int db);
void id3db_print(int db);
int id3db_add( int db, char *path );
int id3db_create(void);
void scan_dir( char *path, void (*get_file_props) (char*path,void *cb), void *cb );
int scan_files( int list, const char *path, 
		void (*get_file_props) (char*path,void *cb), void *cb );

struct id3ent {
    char *path;
    int vs;
};


#endif
