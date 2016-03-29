#ifndef COMMON_SIGXT_H
#define COMMON_SIGXT_H

enum VIRTUAL_INPUT_SYSTEM {
  VIS_DUMMY,		// sig 0 is forbidden
  VIS_SETUP_SLIDER,	// setup slider
  VIS_SET_SLIDER_VAL,
  VIS_REMOVE_SLIDER,    // slider not needed
  VIS_SLIDER_CHANGE,	// new value from slider
  VIS_FIRE,		// fire pressed (or btndown on slider)
  VIS_INCREMENT,	// increment or rotary right
  VIS_DECREMENT,	// decrement or rotary left
  VIS_SIGNAL_MAX	// last signal number +1
};

typedef struct vis_setup_slider {
  int min,max,cur, show_scale;
} vis_setup_slider_t;

#endif
