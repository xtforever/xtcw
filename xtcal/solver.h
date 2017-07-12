#ifndef SOLVER_H
#define SOLVER_H

struct pt {
    int xp,yp, /* disp */
        xt,yt; /* touch */

};

void compute_ctm( struct pt *points, int cnt, int width, int height,  float *ctm_return );

#endif
