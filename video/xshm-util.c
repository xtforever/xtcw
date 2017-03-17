#include "xshm-util.h"
#include <malloc.h>


#ifndef ERR
#define ERR(n, a...) do { fprintf(stderr,"%d %s %s:", __LINE__, __FILE__, __FUNCTION__ ); fprintf(stderr, n , ## a  ); } while(0)
#endif

void shm_destroy(shimg_t *img)
{
    if(! (img && img->img) ) return;
    XShmDetach (img->d, & img->shmSegInfo );
    XDestroyImage (img->img);
    shmdt(img->shmSegInfo.shmaddr);
    shmctl(img->shmSegInfo.shmid, IPC_RMID, 0);
    free(img);
};

shimg_t *shm_create(Widget w, int width, int height )
{
    int ShmMajor,ShmMinor;
    Bool ShmPixmaps;

    Display *d = XtDisplay(w);
    Visual *v = DefaultVisual(d,DefaultScreen(d));

    // Screen *screen = DefaultScreenOfDisplay(d);
    // int screenNr   = DefaultScreen(screen);
    // int depth = DefaultDepth(d,screenNr);
    // fprintf(stderr,"depth=%d\n", depth );

    if(!XShmQueryVersion(d,&ShmMajor,&ShmMinor,&ShmPixmaps))
        return NULL; /* N/A */

    shimg_t *g = malloc(sizeof(shimg_t));
    if( !g ) ERR("out of memory");
    g->d = d;
    g->img=XShmCreateImage(d,v,24,ZPixmap,NULL,& g->shmSegInfo,
                           width,height );

    g->shmSegInfo.shmid = shmget (IPC_PRIVATE,
                g->img->bytes_per_line * g->img->height,
                IPC_CREAT | 0777 );
    g->shmSegInfo.shmaddr = g->img->data =
        shmat (g->shmSegInfo.shmid, 0, 0);
    g->shmSegInfo.readOnly = False;
    if(! XShmAttach(d,& g->shmSegInfo) )
        ERR("ShmAttach returns non-zero");
    return g;
}
