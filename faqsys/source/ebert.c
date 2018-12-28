/*
 *************************************************************************
 *                               Ebert.c                                 *
 *************************************************************************
 * This file contains C code for procedures in my chapters of "Texturing *
 * and Modeling: A Procedural Approach". This software may be used for   *
 * non-profit use as long as the original author is credited.  If you    *
 * find any bugs in this software, send email.                           *
 *************************************************************************
 * Author: David S. Ebert (ebert@cs.umbc.edu)                            *
 * Copyright 1994 David S. Ebert                                         *
 *************************************************************************
*/

/*
 *************************************************************************
 *                           WRITE_NOISE.C                               *
 *  This procedure generates a noise function file for solid texturing   *
 *                      by David S. Ebert                                *
 *************************************************************************
 * This noise file needs to be read in to your program for the rest of   *
 * the procedures in this file to work.                                  *
 *************************************************************************
 */

#include <math.h>
#include <stdio.h>
#include "ebert.h"
#include "macro.h"

#define SIZE           64  
#define SIZE_1         65  
#define POW_TABLE_SIZE 10000
#define RAMP_SIZE      200
#define OFFSET_SIZE    1024
#define SWIRL_FRAMES   126 
#define SWIRL          0.04986655               /* 2*PI/126  */
#define SWIRL_AMOUNT   0.04986655               /* 2*PI/126  */
#define DOWN           0.011904762
#define SPEED          .012
#define DOWN_AMOUNT    .0095
#define DOWN_SMOKE     .015
#define RAD1           .10
#define RAD2           .14
#define SWIRL_SMOKE    .062831853              /* 2*PI/100  */
#define SWIRL_FRAMES_SMOKE 100                 

double drand48();

float  calc_noise();
float  turbulence();
rgb_td marble_color();
double ease();
void   transform_XYZ();

main()
{
  float   tmp,u,v;
  long    i,j, k, ii,jj,kk;
  float   noise[SIZE+1][SIZE+1][SIZE+1];
  FILE    *noise_file;
 
 noise_file = fopen("noise.data","w");
 
 for (i=0; i<SIZE; i++)
   for (j=0; j<SIZE; j++)
     for (k=0; k<SIZE; k++)
       {
         noise[i][j][k] = (float)drand48();
       }

/* This is a hack, but it works. Remember this is only done once.
 */

 for (i=0; i<SIZE+1; i++)
   for (j=0; j<SIZE+1; j++)
     for (k=0; k<SIZE+1; k++)
       {
         ii = (i == SIZE)? 0:  i;
         jj = (j == SIZE)? 0:  j;
         kk = (k == SIZE)? 0:  k;
         noise[i][j][k] = noise[ii][jj][kk];
       }

 fwrite(noise,sizeof(float),(SIZE+1)*(SIZE+1)*(SIZE+1), noise_file);
 fclose(noise_file);
}

/*
 ***********************************************************************
 * The procedures below use the noise file written by the above        *
 * write_noise program. Read the noise file into a variable called     *
 * noise. These procedures also assume that the renderer has a global  *
 * int variable frame_num  that contains the current frame number.     *
 ***********************************************************************
 */


float   noise[SIZE+1][SIZE+1][SIZE+1];
int     frame_num;
/*
 ***********************************************************************
 *                            Calc_noise                               *
 ***********************************************************************
 * This is basically how the trilinear interpolation works. I lerp     *
 * down the left front edge of the cube, then the right front edge of  *
 * the cube(p_l, p_r). Then I lerp down the left back and right back   *
 * edges of the  cube (p_l2, p_r2). Then I lerp across the front face  *
 * between p_l  and p_r (p_face1). The I lerp across the back face     *
 * between p_l2 and p_r2 (p_face2). Now I lerp along the line between  *
 * p_face1 and  p_face2.                                               *
 ***********************************************************************
 */

float
calc_noise(pnt)
     xyz_td  pnt;
{
  float  t1;
  float  p_l,p_l2,    /* value lerped down left side of face1 & face 2 */
         p_r,p_r2,    /* value lerped down left side of face1 & face 2 */
         p_face1,     /* value lerped across face 1 (x-y plane ceil of z) */
         p_face2,     /* value lerped across face 2 (x-y plane floor of z) */
         p_final;     /* value lerped through cube (in z)                  */

  extern float noise[SIZE_1][SIZE_1][SIZE_1];
  float   tnoise;
  register int      x, y, z,px,py,pz;
  
  px = (int)pnt.x;
  py = (int)pnt.y;
  pz = (int)pnt.z;
  x = px &(SIZE-1); /* make sure the values are in the table           */
  y = py &(SIZE-1); /* Effectively, replicates the table thought space */
  z = pz &(SIZE-1);
  
  t1 = pnt.y - py;
  p_l  = noise[x][y][z+1]+t1*(noise[x][y+1][z+1]-noise[x][y][z+1]);  
  p_r  =noise[x+1][y][z+1]+t1*(noise[x+1][y+1][z+1]-noise[x+1][y][z+1]);
  p_l2 = noise[x][y][z]+ t1*( noise[x][y+1][z] - noise[x][y][z]);     
  p_r2 = noise[x+1][y][z]+ t1*(noise[x+1][y+1][z] - noise[x+1][y][z]);
  
  t1 = pnt.x - px; 
  p_face1 = p_l + t1 * (p_r - p_l);
  p_face2 = p_l2 + t1 * (p_r2 -p_l2);
  
  t1 = pnt.z - pz;
  p_final =  p_face2 + t1*(p_face1 -p_face2);
  
  return(p_final);
}


/*
 **********************************************************************
 *                   TURBULENCE                                       *
 **********************************************************************
 */

float turbulence(pnt, pixel_size)
    xyz_td   pnt;
    float    pixel_size;
{
  float t, scale;

  t=0;
  for(scale=1.0; scale >pixel_size; scale/=2.0)
    {
      pnt.x = pnt.x/scale; pnt.y = pnt.y/scale; pnt.z = pnt.z/scale;
      t+=calc_noise(pnt)* scale ; 
    }
  return(t);
}


basic_gas(pnt,density,parms)
     xyz_td  pnt; 
     float   *density,*parms;
{
  float turb;
  int   i;
  static float pow_table[POW_TABLE_SIZE];
  static int calcd=1;
  
  if(calcd)
    { calcd=0;
      for(i=POW_TABLE_SIZE-1; i>=0; i--)
	pow_table[i] = (float)pow(((double)(i))/(POW_TABLE_SIZE-1)*
				  parms[1]*2.0,(double)parms[2]);
    }
  turb =turbulence(pnt);
  *density = pow_table[(int)(turb*(.5*(POW_TABLE_SIZE-1)))];
}

/* 
 *
 *change to make veins of gas 
 *
 */
/*       turb =(1.0 +sin(turbulence(pnt)*M_PI*5))*.5;*/




steam_slab1(pnt, pnt_world, density,parms, vol)
         xyz_td  pnt, pnt_world;
         float   *density,*parms;
         vol_td  vol;
{
  float        turb, dist_sq,density_max;
  int          i, indx;
  xyz_td       diff;
  static float pow_table[POW_TABLE_SIZE], ramp[RAMP_SIZE], 
               offset[OFFSET_SIZE];
  static int   calcd=1;

      if(calcd)
        { calcd=0;
          for(i=POW_TABLE_SIZE-1; i>=0; i--)
             pow_table[i] = (float)pow(((double)(i))/(POW_TABLE_SIZE-1)*
                             parms[1]*2.0,(double)parms[2]);
          make_tables(ramp);
        }
      turb =turbulence(pnt,.1);
      *density = pow_table[(int)(turb*0.5*(POW_TABLE_SIZE-1))];

     /* determine distance from center of the slab ^2. */
      XYZ_SUB(diff,vol.shape.center, pnt_world);
      dist_sq = DOT_XYZ(diff,diff);
      density_max = dist_sq*vol.shape.inv_rad_sq.y; 
      indx = (int)((pnt.x+pnt.y+pnt.z)*100) & (OFFSET_SIZE -1);
      density_max += parms[3]*offset[indx];

      if(density_max >= .25) /* ramp off if > 25% from center */
        { i = (density_max -.25)*4/3*RAMP_SIZE; /* get table index 0:RAMP_SIZE-1 */
          i=MIN(i,RAMP_SIZE-1);
          density_max = ramp[i];
          *density *=density_max;
        }
     }

     make_tables(ramp, offset)
          float *ramp, *offset;
     {
       int   i;
       float dist, result;
       srand48(42);
       for(i=0; i < OFFSET_SIZE; i++)
        {
         offset[i]=(float)drand48();
        }
      for(i = 0; i < RAMP_SIZE; i++)
        { dist =i/(RAMP_SIZE -1.0);
          ramp[i]=(cos(dist*M_PI) +1.0)/2.0;
        }
     }







/* Addition needed to sleam_slab1 to get final steam rising from a teacup
 */

/* dist = pnt_world.y - vol.shape.center.y;
 if(dist > 0.0)
   { dist = (dist +offset[indx]*.1)*vol.shape.inv_rad.y;
     if(dist > .05)
       { offset2 = (dist -.05)*1.111111;
         offset2 = 1 - (exp(offset2)-1.0)/1.718282;
         offset2 *=parms[1];
         *density *= offset;
       }
     }
     */


/*
 *********************************************************************
 *                           Smoke_stream                            *
 *********************************************************************
 * parms[1] = Maximum density value - density scaling factor         *
 * parms[2] = height for 0 density (end of ramping it off)           *
 * parms[3] = height to start adding turbulence                      *
 * parms[4] = height(length) for maximum turbulence;                 *
 * parms[5] = height to start ramping density off                    *
 * parms[6] = center.y                                               *
 * parms[7] = speed for rising                                       *
 * parms[8] = radius                                                 *
 * parms[9] = max radius of swirling                                 *
 *********************************************************************
 */

smoke_stream(pnt,density,parms, pnt_world, vol)
     xyz_td  pnt, pnt_world;
     float   *density,*parms;
     vol_td  *vol;
{
 float           dist_sq;
 extern float    offset[OFFSET_SIZE];
 xyz_td          diff;
 xyz_td          hel_path, new_path, direction2, center;
 double          ease(), turb_amount, theta_swirl, cos_theta, sin_theta;
 static int      calcd=1;
 static float    cos_theta2, sin_theta2;
 static xyz_td   bottom;
 static double   rad_sq, max_turb_length, radius, big_radius,
                 st_d_ramp, d_ramp_length, end_d_ramp,
                 inv_max_turb_length;
 double          height, fast_turb, t_ease, path_turb, rad_sq2;

 if(calcd)
   { bottom.x=0; bottom.z = 0;
     bottom.y = parms[6];
     radius   = parms[8];
     big_radius = parms[9];
     rad_sq   = radius*radius;
     max_turb_length = parms[4];
     inv_max_turb_length = 1/max_turb_length;
     st_d_ramp = parms[5];
     end_d_ramp = parms[2];
     d_ramp_length = end_d_ramp - st_d_ramp;
     theta_swirl = 45.0*M_PI/180.0; /* swirling effect */
     cos_theta  = cos(theta_swirl);
     sin_theta  = sin(theta_swirl);
     cos_theta2 = .01*cos_theta;
     sin_theta2 = .0075*sin_theta;
     calcd=0;
   }


 height = pnt_world.y - bottom.y + calc_noise(pnt)*radius;
   /* We don't want smoke below the bottom of the column */
 if(height < 0)
   { *density =0;  return;}
 height -= parms[3];
 if (height < 0.0)
   height =0.0;

 /* calculate the eased turbulence, taking into account the value may be
  *  greater than 1, which ease won't handle.
  */
 t_ease = height* inv_max_turb_length;
 if(t_ease > 1.0)
   { t_ease = ((int) (t_ease)) + ease( (t_ease - ((int)t_ease)), .001, .999);
     if( t_ease > 2.5)
       t_ease = 2.5;
   }
 else
   t_ease = ease(t_ease, .5, .999); 

 /* Calculate the amount of turbulence to add in */	
 fast_turb= turbulence(pnt, .1);
 turb_amount = (fast_turb -0.875)* (.2 + .8*t_ease);
 path_turb = fast_turb*(.2 + .8*t_ease);

 /* add turbulence to the height and see if it is above the top */
 height +=0.1*turb_amount;
 if(height > end_d_ramp)
   { *density=0; return; }

 /* increase the radius of the column as the smoke rises */
 if(height <=0)
   rad_sq2 = rad_sq*.25;
 else if (height <=end_d_ramp)
   { rad_sq2 = (.5 + .5*(ease( height/(1.75*end_d_ramp), .5, .5)))*radius;
     rad_sq2 *=rad_sq2;
   }
 
/*
 **************************************************************************
 * move along a helical path 
 **************************************************************************
 *

 /* 
  * calculate the path based on the unperturbed flow: helical path 
  */
 hel_path.x = cos_theta2 *(1+ path_turb)*(1+cos(pnt_world.y*M_PI*2)*.11)
   *(1+ t_ease*.1)  + big_radius*path_turb;
 hel_path.z = sin_theta2 *(1+path_turb)*
   (1+sin(pnt_world.y*M_PI*2)*.085)* (1+ t_ease*.1) + .03*path_turb;
 hel_path.y =  - path_turb;
 XYZ_ADD(direction2, pnt_world, hel_path);

 /* adjusting the center point for ramping off the density based on the 
  * turbulence of the moved point 
  */
 turb_amount *= big_radius;
 center.x = bottom.x - turb_amount;
 center.z = bottom.z + .75*turb_amount; 

 /* calculate the radial distance from the center and ramp off the density
  * based on this distance squared.
  */
 diff.x = center.x - direction2.x;
 diff.z = center.z - direction2.z;
 dist_sq = diff.x*diff.x + diff.z*diff.z;
 if(dist_sq > rad_sq2)
   {*density=0; return;}
 *density = (1-dist_sq/rad_sq2 + fast_turb*.05) * parms[1];
if(height > st_d_ramp)
   *density *=  (1- ease( (height - st_d_ramp)/(d_ramp_length), .5 , .5));
}












     
rgb_td marble(pnt)
     xyz_td  pnt;
{
  float y;
  y = pnt.y + 3.0*turbulence(pnt, .0125);
  y = sin(y*M_PI); 
  return (marble_color(y));
}


rgb_td marble_color(x)
     float x;
{
  rgb_td  clr;
  x = sqrt(x+1.0)*.7071; 
  clr.g = .30 + .8*x;
  x=sqrt(x);
  clr.r = .30 + .6*x;
  clr.b = .60 + .4*x; 
  return (clr);
}




rgb_td marble_forming(pnt, frame_num, start_frame, end_frame)
     xyz_td  pnt;
     int     frame_num, start_frame, end_frame;
{
  float x, turb_percent, displacement;
  
  if(frame_num < start_frame)
    { turb_percent=0;
      displacement=0;
    }
  else if (frame_num >= end_frame)
    { turb_percent=1;
      displacement= 3;
    }
  else
    { turb_percent= ((float)(frame_num-start_frame))/ (end_frame-start_frame);
      displacement = 3*turb_percent;
    }
  
  x = pnt.x + turb_percent*3.0*turbulence(pnt, .0125) - displacement;
  x = sin(x*M_PI); 
  return (marble_color(x));
}

rgb_td marble_forming2(pnt, frame_num, start_frame, end_frame, heat_length)
     xyz_td  pnt;
     int     frame_num, start_frame, end_frame, heat_length;
{
  float       x, turb_percent, displacement, glow_percent;
  rgb_td      m_color;
  if(frame_num < (start_frame-heat_length/2) || 
     frame_num > end_frame+heat_length/2)
    glow_percent=0;
  else if (frame_num < start_frame + heat_length/2)
    glow_percent= 1.0 - ease( ((start_frame+heat_length/2-frame_num)/ 
			       heat_length),0.4,0.6); 
  else if (frame_num > end_frame-heat_length/2)
    glow_percent =  ease( ((frame_num-(end_frame-heat_length/2))/ 
			   heat_length),0.4,0.6);
  else 
    glow_percent=1.0;
  
  if(frame_num < start_frame)
    { turb_percent=0;
      displacement=0;
    }
  else if (frame_num >= end_frame)
    { turb_percent=1;
      displacement= 3;
    }
  else
    { turb_percent= ((float)(frame_num-start_frame))/(end_frame-start_frame);
      turb_percent=ease(turb_percent, 0.3, 0.7);
      displacement = 3*turb_percent;
    }
  
  x = pnt.y + turb_percent*3.0*turbulence(pnt, .0125) - displacement;
  x = sin(x*M_PI); 
  m_color=marble_color(x);
  glow_percent= .5* glow_percent;
  m_color.r= glow_percent*(1.0)+ (1-glow_percent)*m_color.r;	
  m_color.g= glow_percent*(0.4)+ (1-glow_percent)*m_color.g;	
  m_color.b= glow_percent*(0.8)+ (1-glow_percent)*m_color.b;	
  return(m_color);
}

rgb_td moving_marble(pnt, frame_num)
     xyz_td  pnt;
     int     frame_num;
{
  float        x, tmp, tmp2;
  static float down, theta, sin_theta, cos_theta;
  xyz_td       hel_path, direction;
  static int   calcd=1;
  
  if(calcd)
    { theta =(frame_num%SWIRL_FRAMES)*SWIRL_AMOUNT; /* swirling effect */
      cos_theta = RAD1 * cos(theta) + 0.5;
      sin_theta = RAD2 * sin(theta) - 2.0;
      down = (float)frame_num*DOWN_AMOUNT+2.0;
      calcd=0;
    }
  tmp = calc_noise(pnt); /* add some randomness */
  tmp2 = tmp*1.75;
  
  /* calculate the helical path */
  hel_path.y = cos_theta + tmp;
  hel_path.x = (- down)  + tmp2;
  hel_path.z = sin_theta - tmp2;
  XYZ_ADD(direction, pnt, hel_path);
  
  x = pnt.y + 3.0*turbulence(direction, .0125);
  x = sin(x*M_PI); 
  return (marble_color(x));
}




void fog(pnt, transp, frame_num)
     xyz_td  pnt;
     float   *transp;
     int      frame_num;
{
  float tmp;
  xyz_td direction,cyl;
  double theta;
  
  pnt.x += 2.0 +turbulence(pnt, .1);
  tmp = calc_noise(pnt);
  pnt.y +=  4+tmp;
  pnt.z += -2 - tmp; 
  
  theta =(frame_num%SWIRL_FRAMES)*SWIRL_AMOUNT; 
  cyl.x =RAD1 * cos(theta); 
  cyl.z =RAD2 * sin(theta);
  
  direction.x = pnt.x + cyl.x;
  direction.y = pnt.y - frame_num*DOWN_AMOUNT;
  direction.z = pnt.z + cyl.z;
  
  *transp = turbulence(direction, .015);
  *transp = (1.0 -(*transp)*(*transp)*.275);
  *transp =(*transp)*(*transp)*(*transp);
}


/*
 ****************************************
 * ANIMATING GASES
 ****************************************
 */

steam_moving(pnt, pnt_world, density,parms, vol)
     xyz_td  pnt, pnt_world;
     float   *density,*parms;
     vol_td  vol;
{
  float  noise_amt,turb, dist_sq, density_max, offset2, theta, dist;
  static float pow_table[POW_TABLE_SIZE], ramp[RAMP_SIZE], offset[OFFSET_SIZE];
  extern int frame_num; 
  xyz_td direction, diff;
  int i, indx;
  static int calcd=1;
  static float down, cos_theta, sin_theta;

  if(calcd)
    { calcd=0;
      /* determine how to move the point through the space (helical path) */
      theta =(frame_num%SWIRL_FRAMES)*SWIRL;       
      down = (float)frame_num*DOWN*3.0 +4.0;
      cos_theta = RAD1*cos(theta) +2.0;
      sin_theta = RAD2*sin(theta) -2.0;
      
      for(i=POW_TABLE_SIZE-1; i>=0; i--)
	pow_table[i] = (float)pow(((double)(i))/(POW_TABLE_SIZE-1)*
				  parms[1]*2.0,(double)parms[2]);
          make_tables(ramp);
    }
  
  /* move the point along the helical path */
  noise_amt = calc_noise(pnt);
  direction.x = pnt.x + cos_theta + noise_amt;
  direction.y = pnt.y - down + noise_amt;
  direction.z = pnt.z +sin_theta + noise_amt;

  turb =turbulence(direction, .1);
  *density = pow_table[(int)(turb*0.5*(POW_TABLE_SIZE-1))];
  
  /* determine distance from center of the slab ^2. */
  XYZ_SUB(diff,vol.shape.center, pnt_world);
  dist_sq = DOT_XYZ(diff,diff);
  density_max = dist_sq*vol.shape.inv_rad_sq.y; 
  indx = (int)((pnt.x+pnt.y+pnt.z)*100) & (OFFSET_SIZE -1);
  density_max += parms[3]*offset[indx];
  
  if(density_max >= .25) /* ramp off if > 25% from center */
    { i = (density_max -.25)*4/3*RAMP_SIZE; /* get table index 0:RAMP_SIZE-1 */
      i=MIN(i,RAMP_SIZE-1);
      density_max = ramp[i];
      *density *=density_max;
    }
  
  /* ramp it off vertically */
  dist = pnt_world.y - vol.shape.center.y;
  if(dist > 0.0)
    { dist = (dist +offset[indx]*.1)*vol.shape.inv_rad.y;
      if(dist > .05)
	{ offset2 = (dist -.05)*1.111111;
	  offset2 = 1 - (exp(offset2)-1.0)/1.718282;
	  offset2*=parms[1];
	  *density *= offset2;
	}
    }
}


volume_fog_animation(pnt, pnt_world, density, parms, vol)
     xyz_td  pnt, pnt_world;
     float   *density,*parms;
     vol_td  *vol;
{
  float noise_amt, turb;
   extern int frame_num; 
   xyz_td direction;
   int    indx;
   static float pow_table[POW_TABLE_SIZE];
   int    i;
   static int calcd=1;
   static float down, cos_theta, sin_theta, theta;

   if(calcd)
     {
       down = (float)frame_num*SPEED*1.5 +2.0;
       theta =(frame_num%SWIRL_FRAMES)*SWIRL; /* get swirling effect */
       cos_theta = cos(theta)*.1 + 0.5;       /* use a radius of .1  */
       sin_theta = sin(theta)*.14 - 2.0;      /* use a radius of .14 */
       calcd=0;
       for(i=POW_TABLE_SIZE-1; i>=0; i--)
         {
          pow_table[i] = (float)pow(((double)(i))/(POW_TABLE_SIZE-1)*
                         parms[1]*4.0,(double)parms[2]);
         }
     }
     
   /* make it move horizontally and add some noise to the movement  */
   noise_amt = calc_noise(pnt); 
   direction.x = pnt.x - down + noise_amt*1.5;
   direction.y = pnt.y + cos_theta +noise_amt; 
   direction.z = pnt.z + sin_theta -noise_amt*1.5;

   /* base the turbulence on the new point */
   turb =turbulence(direction, .1);
   *density = pow_table[(int)((turb*turb)*(.25*(POW_TABLE_SIZE-1)))];
  
   /* make sure density isn't greater than 1 */	
   if(*density >1)
     *density=1;
}




/*
 *********************************************************************
 *                           Rising_smoke_stream                     *
 *********************************************************************
 * parms[1] = Maximum density value - density scaling factor         *
 * parms[2] = height for 0 density (end of ramping it off)           *
 * parms[3] = height to start adding turbulence                      *
 * parms[4] = height(length) for maximum turbulence;                 *
 * parms[5] = height to start ramping density off                    *
 * parms[6] = center.y                                               *
 * parms[7] = speed for rising                                       *
 * parms[8] = radius                                                 *
 * parms[9] = max radius of swirling                                 *
 *********************************************************************
 */

rising_smoke_stream(pnt,density,parms, pnt_world, vol)
     xyz_td  pnt, pnt_world;
     float   *density,*parms;
     vol_td  *vol;
{
  float  dist_sq;
  extern float offset[OFFSET_SIZE];
  extern int frame_num; 
  static int calcd=1;
  static float down, cos_theta2, sin_theta2;
  xyz_td  hel_path, center, diff, direction2;
  double ease(), turb_amount, theta_swirl, cos_theta, sin_theta;
  static xyz_td   bottom;
  static double   rad_sq, max_turb_length, radius, big_radius,
                  st_d_ramp, d_ramp_length, end_d_ramp, inv_max_turb_length, 
                  cos_theta3, down3, sin_theta3;
  double          height, fast_turb, t_ease, path_turb, rad_sq2;

  if(calcd)
    {
      bottom.x = 0; bottom.z = 0;
      bottom.y = parms[6];
      radius   = parms[8];
      big_radius = parms[9];
      rad_sq   = radius*radius;
      max_turb_length = parms[4];
      inv_max_turb_length = 1/max_turb_length;
      st_d_ramp = parms[5];
      st_d_ramp =MIN(st_d_ramp, end_d_ramp);
      end_d_ramp = parms[2];
      d_ramp_length = end_d_ramp - st_d_ramp;
      /* calculate the rotation about the helix axis based on the
       * frame_number 
       */
    theta_swirl =(frame_num%SWIRL_FRAMES_SMOKE)*SWIRL_SMOKE;/*swirling effect*/
    cos_theta  = cos(theta_swirl);
    sin_theta  = sin(theta_swirl);
    down = (float)(frame_num)*DOWN_SMOKE*.75 * parms[7];
      /* Calculate sine and cosine of the different radii of the
       * two helical helical paths
       */
    cos_theta2 = .01*cos_theta;
    sin_theta2 = .0075*sin_theta;
    cos_theta3= cos_theta2*2.25;
    sin_theta3= sin_theta2*4.5;
    down3= down*2.25;
      calcd=0;
    }

  height = pnt_world.y - bottom.y + calc_noise(pnt)*radius;
    /* We don't want smoke below the bottom of the column */
  if(height < 0)
    { *density =0;  return;}
  height -= parms[3];
  if (height < 0.0)
    height =0.0;
 
  /* calculate the eased turbulence, taking into account the value may be
   *  greater than 1, which ease won't handle.
   */
  t_ease = height* inv_max_turb_length;
  if(t_ease > 1.0)
    { t_ease = ((int) (t_ease)) + ease( (t_ease - ((int)t_ease)), .001, .999);
      if( t_ease > 2.5)
        t_ease = 2.5;
    }
  else
    t_ease = ease(t_ease, .5, .999); 

  /* move the point along the helical path before evaluating turbulence */
 pnt.x += cos_theta3;
 pnt.y -= down3;
 pnt.z += sin_theta3;

  fast_turb= turbulence(pnt, .1);
  turb_amount = (fast_turb -0.875)* (.2 + .8*t_ease);
  path_turb = fast_turb*(.2 + .8*t_ease);

 /* add turbulence to the height and see if it is above the top */
  height +=0.1*turb_amount;
  if(height > end_d_ramp)
   { *density=0; return; }

  /* increase the radius of the column as the smoke rises */
  if(height <=0)
    rad_sq2 = rad_sq*.25;
  else if (height <=end_d_ramp)
    {
      rad_sq2 = (.5 + .5*(ease( height/(1.75*end_d_ramp), .5, .5)))*radius;
      rad_sq2 *=rad_sq2;
    }
  else
    rad_sq2 = rad_sq;

 /*
  **************************************************************************
  * move along a helical path plus add the ability to use tables
  **************************************************************************
  *

  /* 
   * calculate the path base on the unperturbed flow: helical path 
   */
    hel_path.x = cos_theta2*(1+ path_turb)*(1+cos((pnt_world.y +down*.5)*
		M_PI*2)	*.11)* (1+ t_ease*.1)  + big_radius*path_turb;
  hel_path.z = sin_theta2 *(1+path_turb)*
    (1+sin((pnt_world.y +down*.5)*M_PI*2)*.085)* (1+ t_ease*.1) + .03*path_turb;

 hel_path.y = (- down) - path_turb;
  XYZ_ADD(direction2, pnt_world, hel_path);

  /* adjusting the center point for ramping off the density based on the 
   *  turbulence of the moved point 
   */
  turb_amount *= big_radius;
  center.x = bottom.x - turb_amount;
  center.z = bottom.z + .75*turb_amount; 

 /* calculate the radial distance from the center and ramp off the density
  * based on this distance squared.
  */
  diff.x = center.x - direction2.x;
  diff.z = center.z - direction2.z;
  dist_sq = diff.x*diff.x + diff.z*diff.z;
 if(dist_sq > rad_sq2)
   {*density=0; return;}
 *density = (1-dist_sq/rad_sq2 + fast_turb*.05) * parms[1];
if(height > st_d_ramp)
   *density *=  (1- ease( (height - st_d_ramp)/(d_ramp_length), .5 , .5));
}











spherical_attractor(point, ff, direction, density_scaling, velocity, 
		    percent_to_use)
     xyz_td       point, *direction;
     flow_func_td ff;
     float        *density_scaling, *velocity, *percent_to_use;
{
  float        dist, d2;
  
  /*calculate distance and direction from center of attractor  */
  XYZ_SUB(*direction, point, ff.center);
  dist=sqrt(DOT_XYZ(*direction,*direction));
  
  /* set the density scaling and the velocity to 1 */
  *density_scaling=1.0;
  *velocity=1.0;
  
  /* calculate the falloff factor (cosine) */
  if(dist > ff.distance)
    *percent_to_use=0;
  else if (dist < ff.falloff_start)
    *percent_to_use=1.0;
  else
    { d2 = (dist - ff.falloff_start)/(ff.distance - ff.falloff_start);
      *percent_to_use = (cos(d2*M_PI)+1.0)*.5;
    }
}

calc_vortex(pt, ff, direction,percent_to_use, frame_num, velocity)
     xyz_td       *pt, *direction;
     flow_func_td *ff;
     float        *percent_to_use, *velocity;
     int          frame_num;
{
  static tran_mat_td  mat={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
       xyz_td              dir, pt2, diff;
       float               theta, dist, d2, dist2;
       float               cos_theta,sin_theta,compl_cos, ratio_mult;

        /*calculate distance from center of vortex */
       XYZ_SUB(diff,(*pt), ff->center);
       dist=sqrt(DOT_XYZ(diff,diff));
       dist2 = dist/ff->distance;
        /* calculate angle of rotation about the axis */
       theta = (ff->parms[0]*(1+.001*(frame_num)))/
               (pow((.1+dist2*.9), ff->parms[1])); 

        /* calculate the matrix for rotating about the cylinder's axis */
       calc_rot_mat(theta, ff->axis, mat);
       transform_XYZ((long)1,mat,pt,&pt2);
       XYZ_SUB(dir,pt2,(*pt));
       direction->x = dir.x;
       direction->y = dir.y;
       direction->z = dir.z;
  
        /* Have the maximum strength increase from frame parms[4] to 
         * parms[5]to a maximum of parms[2]   */
       if(frame_num < ff->parms[4])
         ratio_mult=0;
       else if (frame_num <= ff->parms[5])
          ratio_mult = (frame_num - ff->parms[4])/
                       (ff->parms[5] - ff->parms[4])* ff->parms[2];
       else
         ratio_mult = ff->parms[2];

        /* calculate the falloff factor */
       if(dist > ff->distance)
         { *percent_to_use=0;
           *velocity=1;
         }
       else if (dist < ff->falloff_start)
         { *percent_to_use=1.0 *ratio_mult;
           /*calc velocity  */
           *velocity= 1.0+(1.0 - (dist/ff->falloff_start));
         }
       else
         { d2 = (dist - ff->falloff_start)/(ff->distance - ff->falloff_start);
           *percent_to_use = (cos(d2*M_PI)+1.0)*.5*ratio_mult;
           *velocity= 1.0+(1.0 - (dist/ff->falloff_start));
         }
}



/*
 ***************************************************************************
 * Move the Volume of the Steam 
 ***************************************************************************
 * the shifting is based on the height above the cup (parms[6]->parms[7])
 * and the frame range for increasing the strength of the attractor.
 *  This is gotten from ratio_mult that is calculated above in calc_vortex. 
 ***************************************************************************
 */

/* Have the maximum strength increase from frame parms[4] to 
 * parms[5]to a maximum of parms[2]   
 */
/* if(frame_num < ff->parms[4])
    ratio_mult=0;
   else if (frame_num <= ff->parms[5])
     ratio_mult = (frame_num - ff->parms[4])/(ff->parms[5] - ff->parms[4])
     * ff->parms[2];
     if(point.y < ff->parms[6])
       x_disp=0;
     else
      { if(point.y <= ff->parms[7])
          d2 =COS_ERP((point.y - ff->parms[6])/(ff->parms[7] -ff->parms[6]));
        else 
          d2=0;
        x_disp = (1-d2)*ratio_mult*parms[8]+calc_noise(point)*ff->parms[9];
      }
    return(x_disp);
*/


















/*
 *********************************************************************
 * parms[1] = Maximum density value: density scaling factor          *
 * parms[2] = exponent for  density scaling                          *
 * parms[3] = x resolution for Perlin's trick (0-640)                *
 * parms[8] = 1/radius of fuzzy area for perlin's trick (> 1.0)      *
 *********************************************************************
 */

molten_marble(pnt, density, parms,vol)
     xyz_td  pnt;
     float   *density,*parms;
     vol_td  vol;
{
  float  parms_scalar, turb_amount;
  
  turb_amount = solid_txt(pnt,vol);
  *density = (pow(turb_amount, parms[2]) )*0.35 +.65;
  /* Introduce a harder surface quicker. parms[3] is multiplied by 1/640 */
  *density *=parms[1]; 
  parms_scalar = (parms[3]*.0015625)*parms[8];
  *density= (*density-.5)*parms_scalar +.5;
  *density = MAX(0.2, MIN(1.0,*density));
}

/* 
 *=====================================================================*
 * --------------------------------------------------------------------*
 * ease-in/ease-out                                                    *
 * --------------------------------------------------------------------*
 * By Dr. Richard E. Parent, The Ohio State University                 *
 * (parent@cis.ohio-state.edu)                                         *
 * --------------------------------------------------------------------*
 * using parabolic blending at the end points                          *
 * first leg has constant acceleration from 0 to v during time 0 to t1 *
 * second leg has constant velocity of v during time t1 to t2          *
 * third leg has constant deceleration from v to 0 during time t2 to 1 *
 * these are integrated to get the 'distance' traveled at any time     *
 * --------------------------------------------------------------------*
 */
double ease(t,t1,t2)
double  t,t1,t2;
{
  double  ans,s,a,b,c,nt,rt;
  double  v,a1,a2;

  v = 2/(1+t2-t1);  /* constant velocity attained */
  a1 = v/t1;        /* acceleration of first leg */
  a2 = -v/(1-t2);   /* deceleration of last leg */

  if (t<t1) {
    rt = 0.5*a1*t*t;       /* pos = 1/2 * acc * t*t */
  }
  else if (t<t2) {
    a = 0.5*a1*t1*t1;      /* distance from first leg */
    b = v*(t-t1);            /* distance = vel * time  of second leg */
    rt = a + b;
  }
  else {
    a = 0.5*a1*t1*t1;      /* distance from first leg */
    b = v*(t2-t1);           /* distance from second leg */
    c = ((v + v + (t-t2)*a2)/2) * (t-t2);  /* distance = ave vel. * time */
    rt = a + b + c;
  }
  return(rt);
}



/* transform_xyz: transform an array of xyz_td and output cooresponding
 * xyz_td pnts which are transformed by the matrix input. 
 * It should actually only be be used for scale, rotate, and translate.
 */
 
void
transform_XYZ(num_pts, mat, inpts, outpts)

    long        num_pts;
    tran_mat_td mat;
    xyz_td      *inpts;
    xyz_td     *outpts;
    
{
    long i;
    float w;
    xyz_td *tmp, tmp2;

    if(num_pts==1)
      {
	tmp2.x = mat[0][0]*inpts[0].x + mat[1][0]*inpts[0].y
	         + mat[2][0]*inpts[0].z + mat[3][0];
	tmp2.y = mat[0][1]*inpts[0].x + mat[1][1]*inpts[0].y
                 + mat[2][1]*inpts[0].z + mat[3][1];
	tmp2.z = mat[0][2]*inpts[0].x + mat[1][2]*inpts[0].y
                 + mat[2][2]*inpts[0].z + mat[3][2];
	*outpts=tmp2;
	return;
      }
    /* else */
    tmp =(xyz_td *)malloc(sizeof(xyz_td)*num_pts);

    for (i = 0; i < num_pts; i++)
      {
	tmp[i].x = mat[0][0]*inpts[i].x + mat[1][0]*inpts[i].y
	            + mat[2][0]*inpts[i].z + mat[3][0];
	tmp[i].y = mat[0][1]*inpts[i].x + mat[1][1]*inpts[i].y
	            + mat[2][1]*inpts[i].z + mat[3][1];
	tmp[i].z = mat[0][2]*inpts[i].x + mat[1][2]*inpts[i].y
                    + mat[2][2]*inpts[i].z + mat[3][2];
	outpts[i].x = tmp[i].x;
	outpts[i].y = tmp[i].y;
	outpts[i].z = tmp[i].z;
      }
    
}

//************************ EBERT.H ****************************

/* EDGE standard header file for graphics programs
 * Stripped down version 3/11/94 
 */

#ifndef GRAPHICS_H
#define GRAPHICS_H
#endif

#define       ABS(a)          ((a) > 0 ? (a) : -(a))
double fabs(), sqrt(), sin(), cos(), tan();

  typedef struct xyz_td
          {
              float    x,
                       y,
                       z;
	  } xyz_td;



  typedef struct rgb_td
          {
              float    r,
                       g,
                       b;
	  } rgb_td;


typedef float tran_mat_td[4][4];


/*
 *****************************************************************************
 *                  FUNCTION FIELD DEFINITIONS
 *****************************************************************************
 */

typedef unsigned char flow_func_type;

typedef struct flow_func_td       
    {
      short         func_type;
      xyz_td        center, axis;
      float         distance;
      float         falloff_start;
      short         falloff_type;
      float         parms[20];
    } flow_func_td;

typedef struct vol_shape_td
    {
      xyz_td         center;
      xyz_td         inv_rad_sq;
      xyz_td         inv_rad;
    } vol_shape_td;

typedef struct vol_td
    {
      short          obj_type;
      float          y_obj_min,    /* min & max y per volume             */
	             y_obj_max;
      float          x_obj_min,    /* min & max x per volume             */
	             x_obj_max;
      int            shape_type;   /* 0=sphere, 1=box;                   */
      int            self_shadow;  /* 0=no, 1=yes                        */
      xyz_td         scale_obj;   /*how to scale obj to get into -1:1 space*/
      xyz_td         trans_obj;   /*how to trans obj to get into -1:1 space*/
      vol_shape_td   shape;
      int            funct_name;
      rgb_td         color;
      float          amb_coef;
      float          diff_coef;
      float          spec_coef;
      float          spec_power;
      int            illum_type;  /* illumination - phong, blinn, c-t, gas */
      int            color_type;  /* constant, solid*/
      float          indx_refrac;
    } vol_td;

//************************* MACRO.H **********************************

#define XYZ_ADD(c,a,b)   { (c).x = (a).x + (b).x; (c).y = (a).y + (b).y;\
                           (c).z = (a).z + (b).z;}
#define XYZ_SUB(c,a,b)   { (c).x = (a).x - (b).x; (c).y = (a).y - (b).y; \
                           (c).z = (a).z - (b).z;}
#define XYZ_MULT(c,a,b)  { (c).x = (a).x * (b).x; (c).y = (a).y * (b).y; \
                           (c).z = (a).z * (b).z;}
#define XYZ_DIV(c,a,b)   { (c).x = (a).x / (b).x; (c).y = (a).y / (b).y; \
                           (c).z = (a).z / (b).z;}

#define XYZ_INC(c,a)     { (c).x += (a).x; (c).y += (a).y;\
                           (c).z += (a).z;}
#define XYZ_DEC(c,a)     { (c).x -= (a).x; (c).y -= (a).y;\
                           (c).z -= (a).z;}
#define XYZ_ADDC(c,a,b)  { (c).x = (a).x + (b); (c).y = (a).y + (b);\
                           (c).z = (a).z + (b);}
#define XYZ_SUBC(c,a,b)  { (c).x = (a).x - (b); (c).y = (a).y - (b); \
                           (c).z = (a).z - (b);}
#define XYZ_MULTC(c,a,b) { (c).x = (a).x * (b); (c).y = (a).y * (b);\
                           (c).z = (a).z * (b);}
#define XYZ_DIVC(c,a,b)  { (c).x = (a).x / (b); (c).y = (a).y / (b);\
                           (c).z = (a).z / (b);}
#define XYZ_COPY(b,a)    { (b).x = (a).x; (b).y = (a).y; (b).z = (a).z; }
#define XYZ_COPYC(b,a)   { (b).x = (a); (b).y = (a); (b).z = (a); }
#define  DOT_XYZ(p1, p2)  ((p1).x * (p2).x + (p1).y * (p2).y + (p1).z * (p2).z)

#define CROSS_XYZ(c,a,b) { (c).x = (a).y * (b).z - (a).z * (b).y; \
			   (c).y = (a).z * (b).x - (a).x * (b).z; \
			   (c).z = (a).x * (b).y - (a).y * (b).x; }
#define CROSS_3(c,a,b)   { (c)[0] = (a)[1] * (b)[2] - (a)[2] * (b)[1]; \
			   (c)[1] = (a)[2] * (b)[0] - (a)[0] * (b)[2]; \
			   (c)[2] = (a)[0] * (b)[1] - (a)[1] * (b)[0]; }

#define NORMALIZE_XYZ(v)  { float __tmpnormval; \
			    __tmpnormval = (double)sqrt((v).x*(v).x +  \
					    (v).y*(v).y + (v).z*(v).z); \
			      (v).x /= __tmpnormval; \
			      (v).y /= __tmpnormval;  \
			      (v).z /= __tmpnormval;}
#define R_NORMALIZE_XYZ(v)  { float __tmpnormval; \
			    (((__tmpnormval = (double)sqrt((v).x*(v).x +  \
					    (v).y*(v).y + (v).z*(v).z)) \
			      == 0.0) ? FALSE : ((v).x /= __tmpnormval,  \
						 (v).y /= __tmpnormval,  \
						 (v).z /= __tmpnormval,TRUE));}

#define RGB_ADD(c,a,e)   { (c).r = (a).r + (e).r; (c).g = (a).g + (e).g; \
                           (c).b = (a).b + (e).b; }
#define RGB_SUB(c,a,e)   { (c).r = (a).r - (e).r; (c).g = (a).g - (e).g; \
                           (c).b = (a).b - (e).b; }
#define RGB_MULT(c,a,e)  { (c).r = (a).r * (e).r; (c).g = (a).g * (e).g; \
                           (c).b = (a).b * (e).b; }
#define RGB_ADDC(c,a,e)  { (c).r = (a).r + (e); (c).g = (a).g + (e); \
                           (c).b = (a).b + (e); }
#define RGB_SUBC(c,a,e)  { (c).r = (a).r - (e); (c).g = (a).g - (e); \
                           (c).b = (a).b - (e); }
#define RGB_MULTC(c,a,e) { (c).r = (a).r * (e); (c).g = (a).g * (e); \
                           (c).b = (a).b * (e); }
#define RGB_DIVC(c,a,e)  { (c).r = (a).r / (e); (c).g = (a).g / (e); \
                           (c).b = (a).b / (e); }
#define RGB_COPY(c,a)    { (c).r = (a).r; (c).g = (a).g; (c).b = (a).b; }
#define RGB_COPYC(c,a)   { (c).r = (a); (c).g = (a); (c).b = (a); }

#undef MIN
#undef MAX
#define    MIN(a, b)       ((a) < (b) ? (a) : (b))
#define    MAX(a, b)       ((a) > (b) ? (a) : (b))

