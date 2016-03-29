#ifndef M_STRING_H
#define M_STRING_H

#include <stdarg.h>
#include "mls.h"


/* string utils */
int s_strlen(int m);
int s_new(void);
void s_free(int m);
int s_app(int m, ...);
int s_index(int m, int p, char ch);
int s_printf(int m, int p, char *format, ...);
int s_cp(int m, char*s );
#endif
