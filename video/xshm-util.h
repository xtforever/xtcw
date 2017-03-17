#ifndef XSHM_UTIL_H
#define XSHM_UTIL_H

#include <X11/Intrinsic.h>
#include <X11/extensions/XShm.h>
#include <sys/ipc.h>
#include <sys/shm.h>

typedef struct shimg {
    Display *d;
    XImage *img;
    XShmSegmentInfo shmSegInfo;
} shimg_t;

shimg_t *shm_create(Widget w, int width, int height );
void shm_destroy(shimg_t *img);

#endif
