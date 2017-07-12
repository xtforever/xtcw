#include "xinput-util.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <X11/Xutil.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>

static void matrix_print(void *f);
static void matrix_set(Matrix *m, int row, int col, float val);
static void matrix_set_unity(Matrix *m);

static void matrix_set(Matrix *m, int row, int col, float val)
{
    unsigned int i = row*3+col;
    assert( i < 9);
    m->m[i] = val;
}

static void matrix_set_unity(Matrix *m)
{
    memset(m, 0, sizeof(m->m));
    matrix_set(m, 0, 0, 1);
    matrix_set(m, 1, 1, 1);
    matrix_set(m, 2, 2, 1);
}

static void xi_show(Display *d,  XDeviceInfo    *info )
{
    printf("%ld\t%s\n", info->id, info->name );
}

void xi_list_devices(Display *display)
{
    XDeviceInfo         *info;
    int                 loop;
    int                 num_devices;

    info = XListInputDevices(display, &num_devices);
    for(loop=0; loop<num_devices; loop++) {
        xi_show(display, info+loop);
    }
}

long xi_get_deviceid(Display *display, char *device_name)
{
    XDeviceInfo         *info;
    int                 loop;
    int                 num_devices;

    if( ! (device_name && *device_name)) return -1;
    info = XListInputDevices(display, &num_devices);
    for(loop=0; loop<num_devices; loop++) {
        if( strcmp(device_name, info[loop].name)==0 )
            return info[loop].id;
    }
    return -1;
}


static void matrix_print(void *f)
{
#ifdef DEBUG
    Matrix *m = f;
    printf("[ %3.3f %3.3f %3.3f ]\n", m->m[0], m->m[1], m->m[2]);
    printf("[ %3.3f %3.3f %3.3f ]\n", m->m[3], m->m[4], m->m[5]);
    printf("[ %3.3f %3.3f %3.3f ]\n", m->m[6], m->m[7], m->m[8]);
#endif
}

static void
apply_matrix(Display *dpy, long deviceid, Matrix *ctm, Matrix *return_matrix)
{
    union {
        unsigned char *c;
        float *f;
    } data;
    int format_return;
    Atom type_return;
    Atom prop_float, prop_matrix;
    unsigned long nitems;
    unsigned long bytes_after;
    int rc;

    prop_float = XInternAtom(dpy, "FLOAT", False);
    prop_matrix = XInternAtom(dpy, "Coordinate Transformation Matrix", False);

    if (!prop_float)
    {
        fprintf(stderr, "Float atom not found. This server is too old.\n");
        exit( EXIT_FAILURE );
    }
    if (!prop_matrix)
    {
        fprintf(stderr, "Coordinate transformation matrix not found. This "
                "server is too old\n");
        exit( EXIT_FAILURE );
    }

    rc = XIGetProperty(dpy, deviceid, prop_matrix, 0, 9, False, prop_float,
                       &type_return, &format_return, &nitems, &bytes_after,
                       &data.c);
    if (rc != Success || prop_float != type_return || format_return != 32 ||
        nitems != 9 || bytes_after != 0)
    {
        fprintf(stderr, "Failed to retrieve current property values\n");
        exit( EXIT_FAILURE );
    }

    matrix_print(data.f);

    if( return_matrix )
        memcpy(return_matrix->m, data.f, sizeof(return_matrix->m));

    if( ctm ) {
        memcpy(data.f, ctm->m, sizeof(ctm->m));
        XIChangeProperty(dpy, deviceid, prop_matrix, prop_float,
                         format_return, PropModeReplace, data.c, nitems);
        XFlush(dpy); /* important! else data will be discarded on exit */
    }

    XFree(data.c);
    //    return EXIT_SUCCESS;
}


void string_to_matrix(char *s, Matrix *return_matrix )
{
    int e; float *m = return_matrix->m;
    e=sscanf(s,"%f %f %f %f %f %f %f %f %f",
           &m[0],&m[1],&m[2],
           &m[3],&m[4],&m[5],
           &m[6],&m[7],&m[8] );

    if( e != 9 ) {
        fprintf(stderr,"Matrix: %s\nError:"
                "Need 9 floats, but could only parse %d numbers.\n",
                s, e );
        exit(EXIT_FAILURE);
    }
}

void xi_get_matrix(Display *dpy, long deviceid, Matrix *return_matrix)
{
    apply_matrix(dpy,deviceid,NULL,return_matrix);
}

void xi_apply_matrix(Display *dpy, long deviceid, Matrix *ctm)
{
    apply_matrix(dpy,deviceid,ctm,NULL);
}

void xi_apply_matrix_string(Display *dpy, long deviceid, char *str)
{
    Matrix m;
    string_to_matrix(str,&m);
    xi_apply_matrix(dpy,deviceid,&m);
}

void xi_apply_unity(Display *dpy, long id)
{
    Matrix m;
    matrix_set_unity(&m);
    xi_apply_matrix(dpy, id, &m );
}
