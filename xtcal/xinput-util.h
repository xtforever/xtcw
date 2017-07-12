#ifndef XINPUT_UTIL_H
#define XINPUT_UTIL_H

#include <X11/Xlib.h>

typedef struct Matrix {
    float m[9];
} Matrix;

void xi_apply_matrix(Display *dpy, long deviceid, Matrix *ctm);
void xi_apply_matrix_string(Display *dpy, long deviceid, char *str);
void xi_apply_unity(Display *dpy, long deviceid);
void xi_get_matrix(Display *dpy, long deviceid, Matrix *return_matrix);
long xi_get_deviceid(Display *dpy, char *device_name);
void xi_list_devices(Display *display);

#endif
