#ifndef MAPSTR_H
#define MAPSTR_H
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xresource.h>
#include "MapAg.h"

typedef struct vpool_st {
    MapAg map;
    int   data;
} vpool_t;

vpool_t *vp_create();
void vp_free(vpool_t *vp);
int* vp_qaddr(vpool_t *vp, XrmQuark var, XrmQuark group);
int* vp_addr(vpool_t *vp, char *name, char *group_name );

#endif
