#ifndef WLIST_COMMON_H
#define WLIST_COMMON_H

typedef enum line_states { 
      state_norm1 = 0, 
      state_norm2 = 1,
      state_sel = 2, 
      state_hi = 3,
      state_max = 4 
} line_states;

typedef struct { int x,y,width,height; } rect32_t;

typedef struct { int pos, max, state; char *data; } wlist_entry_t;


#endif
