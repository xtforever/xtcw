#define NUM_OF_DOTS 32768

#define _eps 1.0e-6
#define qFabs(x) fabs(x)
#define qExp(x) exp(x)


#define MDE_SCALE_EXTERN 0
#define MDE_AUTO_SCALE 1
#define MDE_AUTO_ONLY_EXPANDING 2
#define MAX_XYCURVES 10


typedef struct _Range
{
	double MaxY;
	double MinY;
	double MaxX;
	double MinX;			
}XYRange;


typedef struct _Curve
{
	double *XValues;
	double *YValues;
	int PointsDrawn;
	int Points;
	int Size;
	char CurveName[32];
	XYRange Range;
	GC gc_draw;
}XYCurve;

