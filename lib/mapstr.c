

#include "mls.h"
#include <stdio.h>
#include <stdlib.h>

#include "mapstr.h"

vpool_t *vp_create()
{
    vpool_t *vp = malloc( sizeof(vpool_t) );
    vp->data = m_create(300,sizeof(int));
    int dummy = -42; m_put(vp->data,&dummy);
    vp->map  = MapAg_New();    
    return vp;
}

void vp_free(vpool_t *vp)
{
    if( vp ) {
	MapAg_Free(vp->map);
	m_free(vp->data);
	free(vp);
    }
}

int* vp_qaddr(vpool_t *vp, XrmQuark var, XrmQuark group)
{
    int d = (int) MapAg_Find( vp->map, var, group,0 );
    if( !d ) {
	d = m_new(vp->data,1);
	MapAg_Define(vp->map, var,group,0, d  );
    }
    return mls(vp->data, d);
}

int* vp_addr(vpool_t *vp, char *name, char *group_name )
{
    XrmQuark var    = XrmStringToQuark(name); 
    XrmQuark group  = XrmStringToQuark(group_name); 
    return vp_qaddr(vp, var, group);
}


