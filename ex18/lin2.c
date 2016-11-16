
#include "mls.h"

#include <math.h>
#include <stdio.h>

/* gauss solver:
   B   : matrix [ eq ] [eq+1]
   res : vector [ eq ]
*/
void gauss1(void *b, void *res, int eq );


void print_matrix(char *name,
                  void *B, int rows, int cols)
{
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
}


struct pt {
    int xt,yt, /* touch */
        xp,yp; /* disp */
};


void touch_cal(struct pt *points, int cnt, int width, int height)
{
    struct pt *p;
    /* normieren */
    double xy[ 4 * cnt ];
    int i; int t;
    for( i=0; i<cnt; i++) {
        p=points+i;
        t=i*4;
        xy[0+t] = p->xt * 1.0 / width;
        xy[1+t] = p->yt * 1.0 / height;
        xy[2+t] = p->xp * 1.0 / width;
        xy[3+t] = p->yp * 1.0 / height;
    }

    print_matrix( "XY", xy, cnt, 4 );

    double ata[9], atx[3], aty[3];
    memset( ata, 0, sizeof ata );
    memset( atx, 0, sizeof atx );
    memset( aty, 0, sizeof aty );

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

    double abc[3], def[3];

    gauss1(bgauss1, abc, 3 );
    gauss1(bgauss2, def, 3 );

    print_matrix( "abc", abc, 3,1 );
    print_matrix( "def", def, 3,1 );
}

void test_touch_cal(void)
{
    struct pt points[] = {
        { 358,794, 192,192  },
        { 323,209, 1088,192 },
        { 911,205, 1088,832 },
        { 921,795, 193,833  } };

    touch_cal( points, 4, 1280, 1024 );

}




int fitting( double x[], double y[], int points, int degree, double a[] )
{
    int i,j,k,N, n;

    N = points;
    n = degree;

    double X[2*n+1];   // N, sigma(xi),sigma(xi^2),sigma(xi^3)....sigma(xi^2n)
    for (i=0;i<2*n+1;i++)
    {
        X[i]=0; for(j=0;j<N;j++) X[i]=X[i]+pow(x[j],i);
    }

    // a[n+1] final coefficients
    // B is the Normal matrix
    double B[n+1][n+2];
    for (i=0;i<=n;i++)
        for (j=0;j<=n;j++)
            B[i][j]=X[i+j];

     // sigma(yi),sigma(xi*yi),sigma(xi^2*yi)...sigma(xi^n*yi)
    double Y[n+1];
    for (i=0;i<n+1;i++)
    {
        Y[i]=0; for(j=0;j<N;j++) Y[i]=Y[i]+pow(x[j],i)*y[j];
    }

    // copy Y to last column of B(Normal Matrix)
    for (i=0;i<=n;i++)
        B[i][n+1]=Y[i];

    // Gaussian Elimination:: (n+1) Equations
    n=n+1;

    for (i=0;i<n;i++)
        for (k=i+1;k<n;k++)
            if (B[i][i]<B[k][i])
                for (j=0;j<=n;j++)
                {
                    double temp=B[i][j];
                    B[i][j]=B[k][j];
                    B[k][j]=temp;
                }

    // gauss elimination
    for (i=0;i<n-1;i++)
        for (k=i+1;k<n;k++)
            {
                double t=B[k][i]/B[i][i];
                // make the elements below the pivot elements equal to zero or elimnate the variables
                for (j=0;j<=n;j++)
                    B[k][j]=B[k][j]-t*B[i][j];
            }
    for (i=n-1;i>=0;i--)        //back-substitution
        {  //x is an array whose values correspond to the values of x,y,z..
            a[i]=B[i][n]; //make the variable to be calculated equal to the rhs of the last equation
            for (j=0;j<n;j++)
                if (j!=i) // then subtract all the lhs values except the coefficient of the
                    // variable whose value is being calculated
                    a[i]=a[i]-B[i][j]*a[j];
            a[i]=a[i]/B[i][i]; //now finally divide the rhs by the coefficient
            // of the variable to be calculated
        }

    for (i=0;i<n;i++)
        printf( "%e x^%u\n", a[i], i );

    return 0;
}

int get_double(int m, int *p, double *d)
{
    char *s = mls(m,*p);
    char *endp;

    *d = strtod( s, &endp );
    if( s == endp ) return -1;
    *p += endp - s;
    return 0;
}



int load(char *name, int col)
{
    int i,m;

    int table = m_create( col, sizeof(int));
    for(i=0;i<col;i++) {
        m = m_create( 10, sizeof(double));
        m_put(table, &m );
    }

    FILE *fp = fopen( name, "r" );
    if(!fp) return table;

    m = m_create(10,1);
    double d[col];
    int p;
    while( m_fscan2(m, '\n', fp ) == '\n') {
        p=0;
        for(i=0;i<col;i++) {
            if( get_double(m,&p,d+i) ) goto skip_line;
            printf( " %f", d[i] );
        }

        for(i=0;i<col;i++) {
            m_put( INT(table,i), d+i );
        }

    skip_line:
        puts("");
        m_clear(m);

    }

    fclose(fp);
    m_free(m);
    return table;
}


void test_gauss(void);


int main (int argc, char *argv[])
{
    double res[3]; /* c b a */
    m_init();

    test_touch_cal(); exit(1);

    test_gauss(); exit(1);


    int table = load( "f3flowtab", 5 );


    printf("rows %d\n", m_len( INT(table,0)));
    puts("fit 2:4");

    fitting( m_buf(INT(table,1)),
             m_buf(INT(table,3)),
             m_len(INT(table,1)),
             2, res );

    int *d; int i;
    m_foreach(table,i,d) m_free(*d);
    m_free(table);

    m_destruct();
    return 0;
}


void pr_mx(void *B, int rows, int cols)
{
    double *b = B;
    int i,j;

    for( i=0;i<rows;i++) {
        for(j=0;j<cols;j++)
            printf("%c%5.2f", j==cols-1 ? '|' : ' ', *b++);
        printf("\n");
    }

    for(i=0;i<cols;i++)
        printf("------");
    printf("\n\n");
}

/* gauss solver:
   B   : matrix [ eq ] [eq+1]
   res : vector [ eq ]
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
    pr_mx( B,n,n+1);

    for (i=0;i<n;i++)
        for (k=i+1;k<n;k++)
            if ( MX(B,i,i) < MX( B,k,i) )
                for (j=0;j<=n;j++)
                {
                    temp=MX(B,i,j);
                    MX(B,i,j) = MX(B,k,j);
                    MX(B,k,j) = temp;
                }

    pr_mx(B,3,4);

    /* gauss elimination */
    for (i=0;i<n-1;i++) {

        for (k=i+1;k<n;k++)
            {
                t=MX(B,k,i)/MX(B,i,i);
                printf("%f\n", t );

                /* make the elements below the pivot elements
                   equal to zero or elimnate the variables
                */
                for (j=0;j<=n;j++)
                    MX(B,k,j)=MX(B,k,j) - t * MX(B,i,j);

                pr_mx(B,3,4);
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

    pr_mx(a,1,3);
    #undef MX
}


typedef struct point_st { double x0,y0, x1,y1; } point;

void test_gauss(void)
{
    int i;

    double B[3][4] = {
        {3,4,1,5.2 },
        {5,6,1,8.2 },
        {3,2,1,4.2 }
    };

    double a[3];

    gauss1( B, a, 3 );



    point p[4] =
        {
            { 20,5,       568,3686 },
            { 1260,10,     480, 350 },
            { 1280,1010,   3540,414 },
            { 5,1024,      3524,3668 }
        };


    B[0][2]=1;
    B[1][2]=1;
    B[2][2]=1;

    for(i=0;i<3;i++) {
        B[i][0]=p[i].x1;
        B[i][1]=p[i].y1;
        B[i][3]=p[i].x0;
    }

    gauss1(B,a,3);


    for(i=0;i<3;i++) {
        B[i][0]=p[i].x1;
        B[i][1]=p[i].y1;
        B[i][3]=p[i].y0;
    }

    gauss1(B,a,3);

}
