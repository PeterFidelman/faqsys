/***********************************************/
/* gred_ellipse.c - draw ellipses		*
/* algorithm from IEEE CG&A Sept 1984 p.24 	*
/***********************************************/
#include <stdio.h>
#include <sunwindow/window_hs.h>

gred_ellipse (cx, cy, radius_x, radius_y, line_width, pw, pattern_pr)
int   			cx, cy;
int   			radius_x, radius_y;
int			line_width;
struct pixwin 		*pw;
struct pixrect 		*pattern_pr;
{
    struct pixrect 	*pr;
    int			inner_radius_x;
    int			inner_radius_y;
    int			outer_radius_x;
    int			outer_radius_y;

/*******************************************************************/
/* Calculate the inner, outer radiuses of the ellipse              */
/*******************************************************************/
    /* if one pixel wide, radiuses are same */
    if (line_width < 2)
    {
	inner_radius_x = outer_radius_x = radius_x;
	inner_radius_y = outer_radius_y = radius_y;
    }
    else
    {
	outer_radius_x = radius_x + (line_width >> 1);
	outer_radius_y = radius_y + (line_width >> 1);
	inner_radius_x = outer_radius_x - line_width + 1;
	inner_radius_y = outer_radius_y - line_width + 1;
    }

/*******************************************************************/
/* Create a pixrect to build the ellipse in.                       */
/*******************************************************************/
    pr = mem_create ((outer_radius_x << 1) + 1, (outer_radius_y << 1) + 1, 1);
    /* now clear it */
    pr_rop (pr,	0, 0, pr -> pr_size.x, pr -> pr_size.y,
		PIX_CLR, (struct pixrect *) 0, 0, 0);

/*******************************************************************/
/* If the inner_radius > 0 then outline the inner edge.            */
/* Then if the outer radius of the ellipse is > than the inner     */
/* radius, call the fill ellipse routine with the outer radius.    */
/*******************************************************************/
    if ((inner_radius_x > 0)
    	    && (inner_radius_y > 0))
        outline_ellipse (pr, outer_radius_x, outer_radius_y, 
		inner_radius_x, inner_radius_y);

    if (line_width >=  2)
	fill_ellipse(pr, outer_radius_x, outer_radius_y, 
		outer_radius_x, outer_radius_y);
    
/*******************************************************************/
/* Now write the ellipse out to the window.                        */
/*******************************************************************/
    inner_radius_x = inner_radius_y = 0;	/* normal source offset */
    if ((cx -= outer_radius_x) < 0)	/* ellipse extends beyond left edge? */
    {
	inner_radius_x = -cx;
	cx = 0;
    }
    if ((cy -= outer_radius_y) < 0)	/* above top edge? */
    {
    	inner_radius_y = -cy;
	cy = 0;
    }

    if (pattern_pr == NULL)
        pw_write (pw, cx, cy, pr->pr_size.x-inner_radius_x, pr->pr_size.y-inner_radius_y,
	    PIX_SRC ^ PIX_DST, pr, inner_radius_x, inner_radius_y);
    else
	pw_stencil (pw, cx, cy, pr -> pr_size.x, pr -> pr_size.y, 
	    PIX_SRC, pr, inner_radius_x, inner_radius_y, pattern_pr, 0, 0);

    pr_destroy (pr);
}


/* draw ellipse incrementally */
static outline_ellipse (pr, center_x, center_y, rx, ry)
struct pixrect *pr;
int    center_x, center_y;
int    rx, ry;
{
	/* intermediate terms to speed up loop */
	long t1 = rx*rx, t2 = t1<<1, t3 = t2<<1;
	long t4 = ry*ry, t5 = t4<<1, t6 = t5<<1;
	long t7 = rx*t5, t8 = t7<<1, t9 = 0L;
	long d1 = t2 - t7 + (t4>>1);	/* error terms */
	long d2 = (t1>>1) - t8 + t5;

	register int x = rx, y = 0;	/* ellipse points */

	while (d2 < 0)			/* til slope = -1 */
	{
		/* draw 4 points using symmetry */
	    	pr_put (pr, center_x + x, center_y + y, 1);
	    	pr_put (pr, center_x + x, center_y - y, 1);
	    	pr_put (pr, center_x - x, center_y + y, 1);
	    	pr_put (pr, center_x - x, center_y - y, 1);

		y++;		/* always move up here */
		t9 += t3;	
		if (d1 < 0)	/* move straight up */
		{
			d1 += t9 + t2;
			d2 += t9;
		}
		else		/* move up and left */
		{
			x--;
			t8 -= t6;
			d1 += t9 + t2 - t8;
			d2 += t9 + t5 - t8;
		}
	}

	do 				/* rest of top right quadrant */
	{
		/* draw 4 points using symmetry */
	    	pr_put (pr, center_x + x, center_y + y, 1);
	    	pr_put (pr, center_x + x, center_y - y, 1);
	    	pr_put (pr, center_x - x, center_y + y, 1);
	    	pr_put (pr, center_x - x, center_y - y, 1);

		x--;		/* always move left here */
		t8 -= t6;	
		if (d2 < 0)	/* move up and left */
		{
			y++;
			t9 += t3;
			d2 += t9 + t5 - t8;
		}
		else		/* move straight left */
			d2 += t5 - t8;
	} while (x>=0);
}


static fill_ellipse (pr, center_x, center_y, rx, ry)
struct pixrect *pr;
int    center_x, center_y;
int    rx, ry;
{
	long t1 = rx*rx, t2 = t1<<1, t3 = t2<<1;
	long t4 = ry*ry, t5 = t4<<1, t6 = t5<<1;
	long t7 = rx*t5, t8 = t7<<1, t9 = 0;
	long d1 = t2 - t7 + (t4>>1);	/* error terms */
	long d2 = (t1>>1) - t8 + t5;
	register int x = rx, y = 0;	/* ellipse points */
        register int t;			/* used in fill operation */
	int wid;			/* width of fill */

	while (d2 < 0)			/* til slope = -1 */
	{
	 	/* fill in leftward to inner ellipse */
	    	for (t=x; (!pr_get(pr, center_x+t, center_y+y)) && t; t--);
		wid = x - t + 1;
		pr_rop (pr, center_x+t, center_y+y, wid, 1,
			PIX_SET | PIX_DONTCLIP, NULL, 0, 0);
		pr_rop (pr, center_x-x, center_y+y, wid, 1,
			PIX_SET | PIX_DONTCLIP, NULL, 0, 0);
		pr_rop (pr, center_x+t, center_y-y, wid, 1,
			PIX_SET | PIX_DONTCLIP, NULL, 0, 0);
		pr_rop (pr, center_x-x, center_y-y, wid, 1,
			PIX_SET | PIX_DONTCLIP, NULL, 0, 0);

		y++;		/* always move up here */
		t9 += t3;	
		if (d1 < 0)	/* move straight up */
		{
			d1 += t9 + t2;
			d2 += t9;
		}
		else		/* move up and left */
		{
			x--;
			t8 -= t6;
			d1 += t9 + t2 - t8;
			d2 += t9 + t5 - t8;
		}
	}

	do 				/* rest of top right quadrant */
	{
		/* fill in downward to inner ellipse */
	    	for (t=y; (!pr_get(pr, center_x+x, center_y+t)) && t; t--);
		wid = y - t + 1;
		pr_rop (pr, center_x+x, center_y+t, 1, wid,
			PIX_SET | PIX_DONTCLIP, NULL, 0, 0);
		pr_rop (pr, center_x+x, center_y-y, 1, wid,
			PIX_SET | PIX_DONTCLIP, NULL, 0, 0);
		pr_rop (pr, center_x-x, center_y+t, 1, wid,
			PIX_SET | PIX_DONTCLIP, NULL, 0, 0);
		pr_rop (pr, center_x-x, center_y-y, 1, wid,
			PIX_SET | PIX_DONTCLIP, NULL, 0, 0);

		x--;		/* always move left here */
		t8 -= t6;	
		if (d2 < 0)	/* move up and left */
		{
			y++;
			t9 += t3;
			d2 += t9 + t5 - t8;
		}
		else		/* move straight left */
		{
			d2 += t5 - t8;
		}
	} while (x>=0);
}
