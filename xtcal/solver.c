#include "solver.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

static void print_matrix(char *name,
                         void *B, int rows, int cols)
{
#ifdef DEBUG
    double *b = B;
    int i,j;

    printf("------------- %s ----------------------------\n", name );

    for( i=0;i<rows;i++) {
        for(j=0;j<cols;j++)
            printf("%c%5.2f", ' ', *b++);
        printf("\n");
    }

    for(i=0;i<cols;i++)
        printf("------");
    printf("\n\n");
#endif
}


/* gauss solver:
   B   : double [ eq ] [eq+1]
   res : double [ eq ]

   example:
    a + 2b + 3c = 2
    a +  b +  c = 2
   3a + 3b +  c = 0

   eq=3
   B:                    res:
   ||  1 2 3 | 2  ||     |  5  |
   ||  1 1 1 | 2  ||     | -6  |
   ||  3 3 1 | 0  ||     |  3  |

   B=[ 1 2 3 2  1 1 1 2  3 3 1 0 ]
   gauss1(B,res,3)
   res=[ 5 -6 3 ]
*/
void gauss1(void *b, void *res, int eq )
{
#define MX(a,y,x) (((double*)a)[ (x) + (y) * (n+1)])
    int i,n,k,j;
    double t,temp;
    double *B = b;
    double *a = res;

    // n=-0.5 + sqrt(0.25+m_len(b));
    n=eq;
    print_matrix( "INPUT", B,n,n+1);

    for (i=0;i<n;i++)
        for (k=i+1;k<n;k++)
            if ( MX(B,i,i) < MX( B,k,i) )
                for (j=0;j<=n;j++)
                {
                    temp=MX(B,i,j);
                    MX(B,i,j) = MX(B,k,j);
                    MX(B,k,j) = temp;
                }

    print_matrix("", B,n,n+1);

    /* gauss elimination */
    for (i=0;i<n-1;i++) {
        for (k=i+1;k<n;k++)
            {
                t=MX(B,k,i)/MX(B,i,i);
                /* make the elements below the pivot elements
                   equal to zero or elimnate the variables
                */
                for (j=0;j<=n;j++)
                    MX(B,k,j)=MX(B,k,j) - t * MX(B,i,j);
            }
    }

    for (i=n-1;i>=0;i--)        // back-substitution
        {
            a[i]=MX(B,i,n); /* make the variable to be calculated equal
                             to the rhs of the last equation */
            for (j=0;j<n;j++)
                if (j!=i) /* then subtract all the lhs values except
                             the coefficient of the
                             variable whose value is being calculated */
                    a[i]=a[i]-MX(B,i,j)*a[j];
            a[i]=a[i]/MX(B,i,i); /* now finally divide the rhs by the coefficient
                                  of the variable to be calculated */
        }

    print_matrix("result",a,1,3);
    #undef MX
}

/*  normieren
    double xy_return[ 4 * cnt ];
*/
void normalize_points( struct pt *points, int cnt, int width, int height, double *xy_return )
{
    for(int i=0; i<cnt; i++) {
        double *m = xy_return + i*4;
        struct pt *p = points+i;
        m[0] = p->xt * 1.0 / width;
        m[1] = p->yt * 1.0 / height;
        m[2] = p->xp * 1.0 / width;
        m[3] = p->yp * 1.0 / height;
    }
}

#define CLEAR(a)  memset( a  , 0, sizeof a );

/* double xy[ 4 * cnt ]
   double coeff_return[9]
*/
static void ctm_fitting(double *xy, int cnt, double *coeff_return )
{
    int i;
    double abc[3], def[3];
    double ata[9], atx[3], aty[3];
    CLEAR(ata);
    CLEAR(atx);
    CLEAR(aty);
    CLEAR(def);
    CLEAR(abc);

    print_matrix( "XY", xy, cnt, 4 );

    for( i=0; i<cnt; i++ ) {
        double *xy1 = xy + 4 * i;
        ata[0] += xy1[0] * xy1[0]; /* sigma xt^2 */
        ata[1] += xy1[0] * xy1[1];  /* sigma xt*yt */
        ata[2] += xy1[0];          /* sigma xt */
        ata[4] += xy1[1] * xy1[1]; /* sigma yt^2 */
        ata[5] += xy1[1];          /* sigma yt */

        atx[0] += xy1[0]*xy1[2]; /* sigma xt*xp */
        atx[1] += xy1[1]*xy1[2]; /* sigma yt*xp */
        atx[2] += xy1[2];        /* sigma xp */

        aty[0] += xy1[0]*xy1[3]; /* sigma xt*yp */
        aty[1] += xy1[1]*xy1[3]; /* sigma yt*yp */
        aty[2] += xy1[3];        /* sigma yp */
    }

    ata[3] = ata[1];
    ata[6] = ata[2];
    ata[7] = ata[5];
    ata[8] = cnt;

    print_matrix( "ata", ata, 3,3 );
    print_matrix( "atx", atx, 3,1 );
    print_matrix( "aty", aty, 3,1 );

    // abc
    double bgauss1[ 4 * 3 ], bgauss2[ 4 * 3 ];
    for(i=0;i<3;i++) {
        double *b = bgauss1 + i*4;
        double *a = ata     + i*3;
        double *x = atx + i;
        b[0] = a[0]; b[1] = a[1]; b[2] = a[2]; b[3] = x[0];
    }
    // def
    for(i=0;i<3;i++) {
        double *b = bgauss2 + i*4;
        double *a = ata     + i*3;
        double *x = aty + i;
        b[0] = a[0]; b[1] = a[1]; b[2] = a[2]; b[3] = x[0];
    }

    print_matrix( "ata\\atx", bgauss1, 3,4 );
    print_matrix( "ata\\aty", bgauss2, 3,4 );

    gauss1(bgauss1, abc, 3 );
    gauss1(bgauss2, def, 3 );

    print_matrix( "abc", abc, 3,1 );
    print_matrix( "def", def, 3,1 );

#define MX(y,x,d) coeff_return[3*y+x]=d
    /* generate CTM */
    for(i=0;i<3;i++) {
        MX(0,i, abc[i] );
        MX(1,i, def[i] );
    }
    MX(2,0,0); MX(2,1,0); MX(2,2,1);
}

void compute_ctm( struct pt *points, int cnt, int width, int height,  float *ctm_return )
{
    double xy[ 4 * cnt ];
    double ctm[9];
    normalize_points(points,cnt,width,height,xy);
    ctm_fitting(xy,cnt,ctm);
    for(int i=0;i<9;i++) ctm_return[i] = ctm[i];
}
