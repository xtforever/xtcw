#ifndef INI_READ2_H
#define INI_READ2_H

/* parser */
void rc_add_chapter( char *s );
void rc_var_add( char *s );
void rc_var_set( char *s1, char *s2 );
void rc_var_define(char *name, char *s1);
/* end parser */

void rc_init(char *filename);


int rc_find_key(  char *chap, char *key);
void rc_print_key(char *chap, char *key);
void rc_dump(void);
void rc_free();
int rc_get_key(  char *chap, char *key);
int rc_get_pair( int key, int n,int *buf, int *value );

char *rc_key_value(char *chap, char *key );
int rc_find_nearest_pair( int key, int val, int *pos, int *bufp, int *value );



#endif
