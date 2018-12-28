/*
 * dither -- use a reduced set of grey values to represent an image
 *
 * Copyright (C) 1988 by Dale Schumacher.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  This software is provided "as is" without express or
 * implied warranty.
 *
 *
 * Notes:
 *
 * [1] Floyd-Steinberg dither:
 *  I should point out that the actual fractions we used were, assuming
 *  you are at X, moving left to right:
 *
 *		    X     7/16
 *	     3/16   5/16  1/16    
 *
 *  Note that the error goes to four neighbors, not three.  I think this
 *  will probably do better (at least for black and white) than the
 *  3/8-3/8-1/4 distribution, at the cost of greater processing.  I have
 *  seen the 3/8-3/8-1/4 distribution described as "our" algorithm before,
 *  but I have no idea who the credit really belongs to.

 *  Also, I should add that if you do zig-zag scanning (see my immediately
 *  previous message), it is sufficient (but not quite as good) to send
 *  half the error one pixel ahead (e.g. to the right on lines you scan
 *  left to right), and half one pixel straight down.  Again, this is for
 *  black and white;  I've not tried it with color.
 *  -- 
 *					    Lou Steinberg
 */

static char	_Program[] = "dither";
static char	_Version[] = "1.3";
static char	_Copyright[] = "Copyright 1988 by Dale Schumacher";

#include <stdio.h>
#include <ctype.h>
#include "pxm.h"

#define	DEBUG(x)	if(0)/* x */

#define	range	255		/* maximum pixel output value */

int	levels = 2;		/* number of output levels */

banner()
	{
	printf("%s v%s -- %s\n", _Program, _Version, _Copyright);
	}

usage()
	{
	fprintf(stderr,
		"usage: %s [-n levels] inpxm outpxm\n",
		_Program);
	exit(1);
	}

main(argc, argv)
	int argc;
	char *argv[];
	{
	register PX_DEF *ipx, *opx;
	FILE *f;
	register int c;
	register char *p;
	extern int optind;
	extern char *optarg;

	while((c = getopt(argc, argv, "n:V")) != EOF)
		{
		switch(c)
			{
			case 'n':
				levels = atoi(optarg);
				break;
			case 'V':
				banner();
				exit(0);
			case '?':
			default:
				usage();
			}
		}
	if((argc - optind) < 2)
		usage();
	else
		{
		ipx = px_ropen(argv[optind++]);
		opx = px_wopen(argv[optind++],
			(PXT_PIX | PXT_BIN),
			ipx->px_width, ipx->px_height, 8, NULL);
		dither(ipx, opx);
		px_close(ipx);
		px_close(opx);
		}
	exit(0);
	}

dither(ipx, opx)
	PX_DEF *ipx, *opx;
	{
	unsigned int xsize, ysize;
	unsigned int y, x;
	register int xslop, *yslop, dslop;
	register int i, j, k, t, q;
	register PX_BYTE *ibuf, *obuf;

	xsize = ipx->px_width;
	ysize = ipx->px_height;
	ibuf = px_rowalloc(xsize, ipx->px_psize);
	obuf = px_rowalloc(xsize, opx->px_psize);
	yslop = (int *) px_alloc(xsize * sizeof(int));

	t = ((range + 1) * 2) / levels;		/* threshold factor */
	q = range / (levels - 1);		/* quantization factor */
	j = (9 * range) / 32;
	for(x=0; x<xsize; ++x)
		yslop[x] = j;

	for(y=0; y<ysize; ++y)
		{
		xslop = (7 * range) / 32;
		dslop = range / 32;
		px_rrow(ipx, ibuf);
		for(x=0; x<xsize; ++x)
			{
			i = px_rgrey(ipx, ibuf, x);
			i += xslop + yslop[x];
			j = (i / t) * q;
			if(j > range)		/* quick hack to fix overflow */
				j = range;	/* which shouldn't happen :-) */
			px_wgrey(opx, obuf, x, j);
			i = i - j;
			k = (i >> 4);		/* (i / 16) */
			xslop = 7 * k;
			yslop[x] = (5 * k) + dslop;
			if(x > 0)
				yslop[x-1] += 3 * k;
			dslop = i - (15 * k);
			}
		px_wrow(opx, obuf);
		}
	}
