#ifndef IF_MPG123_H
#define IF_MPG123_H
#include <X11/Intrinsic.h>
#include "mrb.h"

extern int trace_m123;
#define M123_DISCONNECT 0
#define M123_STOP  1
#define M123_PLAY  2
#define M123_PAUSE 3



int m123_status(void);
int m123_playpos(void);
void m123_set_playpos(int relative );

int m123_init(Widget top,char *server, char* port,void (*func) (int old, int new));
void m123_destroy(void);

int m123_load(char *fn);
void m123_pause(void);
#endif
