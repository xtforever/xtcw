#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>
#include <X11/Intrinsic.h>
#include <X11/Xft/Xft.h>

int m_isequal( int m, const char *s, int n);
int rc_get_int(char*chap, char *key, int *valp);
int getInt( char **s0, int *p );
int parse_variable_assignment_int(int vset, int line, int *p );
void copy_variable( int vset, char *dst, char *src );

int cut_id(int line, int *p, int buf );
int next_id( int line, int *p);
int rc_key_int(char*chap, char *key);
char *rc_key_str(char*chap, char *key);
int find_str(char **list, int n, char *key);


int strlist_line_width( char *p );
int isword(char a);
int strlist_cut_word( char *p, int *a, int *b );
ulong get_time_ms(void);

/* segmented text */
enum T_ATTRIB { T_HARD_BREAK = 1, T_CENTER = 2 };

typedef struct text_segment {
      uint8_t attribute;
      uint8_t format;
      XRectangle r;
      int start, end;      
} text_segment_t;

typedef struct text_attribute {
    char name;
    XftColor color;
    XftFont *font;
    int space_width;
} text_attribute_t;

typedef struct tseg_text {
    int text;
    int m_seg;
    int m_attr;    
} tseg_text_t;

void tseg_init(tseg_text_t *ts);
void tseg_free(tseg_text_t *ts);
void tseg_set_text(tseg_text_t *ts, char *string );


FILE *xopen_file( char *fn, char *mode );
FILE *xopen_write(char *fn);
FILE *xopen_read(char *fn);
#endif
