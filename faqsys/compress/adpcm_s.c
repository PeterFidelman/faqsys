/***********************************************************
Copyright 1992 by Stichting Mathematisch Centrum, Amsterdam, The
Netherlands.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the names of Stichting Mathematisch
Centrum or CWI not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior permission.

STICHTING MATHEMATISCH CENTRUM DISCLAIMS ALL WARRANTIES WITH REGARD TO
THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS, IN NO EVENT SHALL STICHTING MATHEMATISCH CENTRUM BE LIABLE
FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

******************************************************************/

/*
** Intel/DVI ADPCM coder/decoder.
**
** The algorithm for this coder was taken from the IMA Compatability Project
** proceedings, Vol 2, Number 2; May 1992.
**
** Version 1.0, 7-Jul-92.
*/

struct adpcm_state {
    short      valprev_l;        /* Previous output value */
    char       index_l;          /* Index into stepsize table */
    short      valprev_r;        /* Previous output value */
    char       index_r;          /* Index into stepsize table */
};

#ifndef __STDC__
#define signed
#endif

#define NODIVMUL

/* Intel ADPCM step variation table */
static long indexTable[16] = {
    -1, -1, -1, -1, 2, 4, 6, 8,
    -1, -1, -1, -1, 2, 4, 6, 8,
};

static long stepsizeTable[89] = {
    7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
    19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
    50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
    130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
    337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
    876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
    2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
    5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

void
adpcm_coder(indata, outdata, len, state)
    short indata[];
    char outdata[];
    int len;
    struct adpcm_state *state;
{
    short *inp;                        /* Input buffer pointer */
    signed char *outp;         /* output buffer pointer */
    long val;                   /* Current input sample value */
    long sign;                  /* Current adpcm sign bit */
    long delta;                 /* Current adpcm output value */
    long step_l;                /* Stepsize */
    long step_r;                /* Stepsize */
    long valprev_l;             /* virtual previous output value */
    long valprev_r;             /* virtual previous output value */
    long vpdiff;                        /* Current change to valprev */
    long index_l;               /* Current step change index */
    long index_r;
    long outputbuffer;          /* place to keep previous 4-bit value */

    outp = (signed char *)outdata;
    inp = indata;

    valprev_l = state->valprev_l;
    index_l = state->index_l;
    step_l = stepsizeTable[index_l];
    valprev_r = state->valprev_r;
    index_r = state->index_r;
    step_r = stepsizeTable[index_r];

    for ( ; len > 0 ; len-- )
    {
       /****************/
       /* LEFT CHANNEL */
       /****************/
       val = *inp++;
       /* Step 1 - compute difference with previous value */
       delta = val - valprev_l;
       sign = (delta < 0) ? 8 : 0;
       if ( sign ) delta = (-delta);

       /* Step 2 - Divide and clamp */
#ifdef NODIVMUL
        {
           long tmp = 0;

           vpdiff = 0;
           if ( delta > step_l ) {
               tmp = 4;
               delta -= step_l;
               vpdiff = step_l;
           }
           step_l >>= 1;
           if ( delta > step_l ) {
               tmp |= 2;
               delta -= step_l;
               vpdiff += step_l;
           }
           step_l >>= 1;
           if ( delta > step_l ) {
               tmp |= 1;
               vpdiff += step_l;
           }
           delta = tmp;
       }
#else
       delta = (delta<<2) / step_l;
       if ( delta > 7 ) delta = 7;

       vpdiff = (delta*step_l) >> 2;
#endif

       /* Step 3 - Update previous value */
       if ( sign )
         valprev_l -= vpdiff;
       else
         valprev_l += vpdiff;

       /* Step 4 - Clamp previous value to 16 bits */
       if ( valprev_l > 32767 )
         valprev_l = 32767;
       else if ( valprev_l < -32768 )
         valprev_l = -32768;

       /* Step 5 - Assemble value, update index and step values */
       delta |= sign;

       index_l += indexTable[delta];
       if ( index_l < 0 ) index_l = 0;
       if ( index_l > 88 ) index_l = 88;
       step_l = stepsizeTable[index_l];

       /* Step 6 - Output value */
       outputbuffer = (delta << 4) & 0xf0;
       /****************/
       /* RIGHT CHANNEL */
       /****************/
       val = *inp++;
       /* Step 1 - compute difference with previous value */
       delta = val - valprev_r;
       sign = (delta < 0) ? 8 : 0;
       if ( sign ) delta = (-delta);

       /* Step 2 - Divide and clamp */
#ifdef NODIVMUL
        {
           long tmp = 0;

           vpdiff = 0;
           if ( delta > step_r ) {
               tmp = 4;
               delta -= step_r;
               vpdiff = step_r;
           }
           step_r >>= 1;
           if ( delta > step_r ) {
               tmp |= 2;
               delta -= step_r;
               vpdiff += step_r;
           }
           step_r >>= 1;
           if ( delta > step_r ) {
               tmp |= 1;
               vpdiff += step_r;
           }
           delta = tmp;
       }
#else
       delta = (delta<<2) / step_r;
       if ( delta > 7 ) delta = 7;

       vpdiff = (delta*step_r) >> 2;
#endif

       /* Step 3 - Update previous value */
       if ( sign )
         valprev_r -= vpdiff;
       else
         valprev_r += vpdiff;

       /* Step 4 - Clamp previous value to 16 bits */
       if ( valprev_r > 32767 )
         valprev_r = 32767;
       else if ( valprev_r < -32768 )
         valprev_r = -32768;

       /* Step 5 - Assemble value, update index and step values */
       delta |= sign;

       index_r += indexTable[delta];
       if ( index_r < 0 ) index_r = 0;
       if ( index_r > 88 ) index_r = 88;
       step_r = stepsizeTable[index_r];

       /* Step 6 - Output value */
       *outp++ = (delta & 0x0f) | outputbuffer;
    }

    state->valprev_l = valprev_l;
    state->index_l = index_l;
    state->valprev_r = valprev_r;
    state->index_r = index_r;
}

void
adpcm_decoder(indata, outdata, len, state)
    char indata[];
    short outdata[];
    int len;
    struct adpcm_state *state;
{
    signed char *inp;          /* Input buffer pointer */
    short *outp;               /* output buffer pointer */
    long sign;                  /* Current adpcm sign bit */
    long delta;                 /* Current adpcm output value */
    long step_l;                /* Stepsize */
    long step_r;                /* Stepsize */
    long valprev_l;             /* virtual previous output value */
    long valprev_r;             /* virtual previous output value */
    long vpdiff;                        /* Current change to valprev */
    long index_l;               /* Current step change index */
    long index_r;               /* Current step change index */
    long inputbuffer;           /* place to keep next 4-bit value */

    outp = outdata;
    inp = (signed char *)indata;

    valprev_l = state->valprev_l;
    index_l = state->index_l;
    step_l = stepsizeTable[index_l];
    valprev_r = state->valprev_r;
    index_r = state->index_r;
    step_r = stepsizeTable[index_r];

    for ( ; len > 0 ; len-- )
    {
       /****************/
       /* LEFT CHANNEL */
       /****************/
       /* Step 1 - get the delta value and compute next index */
       inputbuffer = *inp++;
       delta = (inputbuffer >> 4) & 0xf;

        /* Step 2 - Find new index value (for later) */
        index_l += indexTable[delta];
        if ( index_l < 0 )
        {
            index_l = 0;
        }
        else if ( index_l > 88 )
        {
            index_l = 88;
        }

       /* Step 3 - Separate sign and magnitude */
       sign = delta & 8;
       delta = delta & 7;

       /* Step 4 - update output value */
#ifdef NODIVMUL
        vpdiff = 0;
        if ( delta & 4 )
        {
            vpdiff  = (step_l << 2);
        }
        if ( delta & 2 )
        {
            vpdiff += (step_l << 1);
        }
        if ( delta & 1 )
        {
            vpdiff += step_l;
        }
        vpdiff >>= 2;
#else
        vpdiff = (delta*step_l) >> 2;
#endif
       if ( sign )
         valprev_l -= vpdiff;
       else
         valprev_l += vpdiff;

       /* Step 5 - clamp output value */
       if ( valprev_l > 32767 )
         valprev_l = 32767;
       else if ( valprev_l < -32768 )
         valprev_l = -32768;

       /* Step 6 - Update step value */
       step_l = stepsizeTable[index_l];

       /* Step 7 - Output value */
       *outp++ = valprev_l;
       /*****************/
       /* RIGHT CHANNEL */
       /*****************/
       /* Step 1 - get the delta value and compute next index */
       delta = inputbuffer & 0xf;

        /* Step 2 - Find new index value (for later) */
        index_r += indexTable[delta];
        if ( index_r < 0 )
        {
            index_r = 0;
        }
        else if ( index_r > 88 )
        {
            index_r = 88;
        }

       /* Step 3 - Separate sign and magnitude */
       sign = delta & 8;
       delta = delta & 7;

       /* Step 4 - update output value */
#ifdef NODIVMUL
        vpdiff = 0;
        if ( delta & 4 )
        {
            vpdiff  = (step_r << 2);
        }
        if ( delta & 2 )
        {
            vpdiff += (step_r << 1);
        }
        if ( delta & 1 )
        {
            vpdiff += step_r;
        }
        vpdiff >>= 2;
#else
        vpdiff = (delta*step_r) >> 2;
#endif
       if ( sign )
         valprev_r -= vpdiff;
       else
         valprev_r += vpdiff;

       /* Step 5 - clamp output value */
       if ( valprev_r > 32767 )
         valprev_r = 32767;
       else if ( valprev_r < -32768 )
         valprev_r = -32768;

       /* Step 6 - Update step value */
       step_r = stepsizeTable[index_r];

       /* Step 7 - Output value */
       *outp++ = valprev_r;
    }

    state->valprev_l = valprev_l;
    state->index_l = index_l;
    state->valprev_r = valprev_r;
    state->index_r = index_r;
}
