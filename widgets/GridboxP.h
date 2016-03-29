/* $Id: GridboxP.h,v 1.2 1999/07/01 16:33:33 falk Exp $
 *
 * GridboxP.h - Private definitions for Gridbox widget
 */

#ifndef _GridboxP_h
#define _GridboxP_h

#include "Gridbox.h"


typedef struct {
    XtPointer	extension ;
} GridboxClassPart;

typedef struct _GridboxClassRec {
    CoreClassPart	core_class;
    CompositeClassPart	composite_class;
    ConstraintClassPart	constraint_class;
    GridboxClassPart	gridbox_class;
} GridboxClassRec;

extern GridboxClassRec gridboxClassRec;


/* Notes:
 *  nx,ny are the dimensions of the grid, in cells
 *  maxgw, maxgh are the sizes of the largest cells in the grid.
 *
 *  max_wids, max_hgts are the maximum preferred child widths for each
 *	column and heights for each row, respectively.  This does
 *	*not* mean that the rows & columns are actually this size.
 *
 * wids, hgts are the actual widths and heights of the columns and rows.
 *
 * total_wid, total_hgt are the sums of the max_wids, max_hgts arrays
 * total_weightx, total_weighty are the sums of the maximum weights
 *	of the child widgets.
 */

typedef struct _GridboxPart {
    /* resources */
    int		defaultDistance;	/* default distance between children */

    /* private state */
    int		nx, ny ;
    Dimension	maxgw, maxgh ;
    Dimension	*max_wids, *max_hgts ;
    Dimension	*wids, *hgts ;
    int		*max_weightx, *max_weighty ;
    Dimension	total_wid, total_hgt ;
    int		total_weightx, total_weighty ;
    Bool	needs_layout ;
} GridboxPart;

typedef struct _GridboxRec {
    CorePart		core;
    CompositePart	composite;
    ConstraintPart	constraint;
    GridboxPart		gridbox;
} GridboxRec;

typedef struct _GridboxConstraintsPart {
/*
 * Constraint Resources.
 */
    Position	gridx, gridy ;	/* position in the grid			*/
    Dimension	gridWidth,	/* size in cells			*/
    		gridHeight ;
    FillType	fill ;		/* how to expand to fill cell		*/
    int		gravity ;	/* position within cell			*/
    int		weightx,	/* how much excess space to absorb	*/
    		weighty ;
    int		margin ;
/*
 * Private contstraint resources.
 */
    Dimension	prefWidth, prefHeight ;	/* what the child wants to be	*/
    Bool	queried ;
} GridboxConstraintsPart;

typedef struct _GridboxConstraintsRec {
    GridboxConstraintsPart	gridbox;
} GridboxConstraintsRec, *GridboxConstraints;

typedef GridboxConstraintsPart GridboxConstraintPart; 

#endif /* _GridboxP_h */
