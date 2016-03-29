#ifndef MICRO_VARS
#define MICRO_VARS

#include "mapstr.h"

struct mv_data {
  int data;
  int signal;
  char locked;
  char *type;
};

struct mv_signal {
  void *d;
  void (*fn) (void*);
};

struct micro_vars {
  int data;
  MapAg map;  
};

typedef void (*voidfn_t) (void*);

void mv_init();
void mv_parse(int buf, int *p, char *group );
void mv_onwrite( int q_var, void (*fn) (void*), void *d, int remove );
int *mv_var(  int q_var );
int *mv_svar(  char *var );
void mv_write(  int q_var, int d );

/* init var with zero and return quark */
int mv_zerovar(char *s);
#endif




