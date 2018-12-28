
// Date: Sat, 19 Apr 1997 18:38:25 -0200
// From: Alessandro Moure <moure@sercomtel.com.br>
// Subject: Cubic spline code.
//
// Beta_sp4.c

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<math.h>
#include<conio.h>
#include<dos.h>

//                           Natural Cubic Spline Zoom Code
//                                        By
//                           Cesar Antonio Caggiano Santos
//                                Sandra Mara Tutida
//                                 Alessandro Moure
//                                       1997
//

void crt_img(char *);        // The "main" function
void read_matrix(char *);    // Read the matrix file
void save_spline(char *);    // Save the new matrix
void drv_l_sigma(void);      //***********************
void drv_c_sigma(void);      //*                     *
void drv_l_di(void);         //*                     *
void drv_c_di(void);         //*                     *
void drv_l_k(void);          //*                     *
void drv_c_k(void);          //* These functions are *
void drv_l_z(void);          //* all math functions! *
void drv_c_z(void);          //*                     *
void drv_l_mu(void);         //*                     *
void drv_c_mu(void);         //*                     *
void drv_l_spl(void);        //*                     *
void drv_c_spl(void);        //***********************

   float matrix[55][55],     // The original matrix
         lsigma[55][55],     // *********************
         csigma[55][560],    // *                   *
         ldi[55][55],        // *                   *
         cdi[55][560],       // *                   *
         lk[55][55],         // * Variables for the *
         ck[55][560],        // *   math functions  *
         lz[55][55],         // *                   *
         cz[55][560],        // *                   *
         lmu[55][55],        // *                   *
         cmu[55][560],       // *                   *
         spl_mtrx[55][560],  // *********************
         spline[560][560];   // The final matrix


   int   cnt1,               // generic loop counter
         cnt2,               // generic loop counter
         dim_x,              // dimension of the original matrix (axis x)
         dim_y,              // dimension of the original matrix (axis y)
         iamp_x,             // number of points to add to original matrix (axis x) (+1)  ie: 1 point of original matrix will be expanded to iamp_x points in the final matrix
         iamp_y;             // number of points to add to original matrix (axis y) (+1)  ie: 1 point of original matrix will be expanded to iamp_y points in the final matrix

   FILE  *swpin;             // pointer to read/write files

   float swp,                // generic purpose variable
         amp_x,              // same of iamp_x
         amp_y;              // same of iamp_y

void crt_img(char *flnm)
{
   read_matrix(flnm);
   drv_l_sigma();
   drv_l_di();
   drv_l_k();
   drv_l_z();
   drv_l_mu();
   drv_l_spl();
   drv_c_sigma();
   drv_c_di();
   drv_c_k();
   drv_c_z();
   drv_c_mu();
   drv_c_spl();
   save_spline(flnm);
}

void save_spline(char *flnm)
{
   int  k = 0;
   char nflnm[12];

   do
   {
     k++;
     strncpy(nflnm, flnm, k);
   } while(!strstr(nflnm,"."));
   strcat(nflnm, "spl");
   swpin = fopen(nflnm, "wb");
   fprintf(swpin, "%d %d \n", dim_y*iamp_y, dim_x*iamp_x);
   for(cnt1 = 0; cnt1 < dim_y*iamp_y; cnt1++)
   {
      for(cnt2 = 0; cnt2 < dim_x*iamp_x; cnt2++)
          fprintf(swpin, "%f ", spline[cnt1][cnt2]);
      fprintf(swpin, "\n");
   }
   fclose(swpin);
}

void read_matrix(char *flnm)
{
   clrscr();
   printf("Entre amp_x: ");
   scanf("%d", &iamp_x);
   amp_x = iamp_x;
   printf("Entre amp_y: ");
   scanf("%d", &iamp_y);
   amp_y = iamp_y;
   swpin = fopen(flnm, "r");
   fscanf(swpin, "%d", &dim_y);
   fscanf(swpin, "%d", &dim_x);
   for(cnt1 = 0; cnt1 < dim_y; cnt1+=2)         //******************************************
   {                                            //* You may need to change this routine!!! *
      for(cnt2 = 0; cnt2 < dim_x; cnt2++)       //******************************************
      {
          fscanf(swpin, "%f", &swp);
          matrix[cnt1][cnt2] = swp;
      }
      for(cnt2 = dim_x-1; cnt2 >= 0; cnt2--)
      {
          fscanf(swpin, "%f", &swp);
          matrix[cnt1+1][cnt2] = swp;

      }
   }
   fclose(swpin);
}

void drv_l_sigma(void)
{
   for(cnt1 = 0; cnt1 < dim_y; cnt1++)
       for(cnt2 = 1; cnt2 < dim_x; cnt2++)
       {
          lsigma[cnt1][cnt2] = ((matrix[cnt1][cnt2]-matrix[cnt1][cnt2-1])/amp_x);
       }


}

void drv_c_sigma(void)
{
   for(cnt1 = 0; cnt1 < (dim_x*iamp_x); cnt1++)
       for(cnt2 = 1; cnt2 < dim_y; cnt2++)
       {
          csigma[cnt2][cnt1] = ((spl_mtrx[cnt2][cnt1]-spl_mtrx[cnt2-1][cnt1])/amp_y);
       }
}

void drv_l_di(void)
{
  for(cnt1 = 0; cnt1 < dim_y; cnt1++)
  {
      for(cnt2 = 1; cnt2 < (dim_x-1); cnt2++)
      {
         ldi[cnt1][cnt2] = ((6 * (lsigma[cnt1][cnt2+1] - lsigma[cnt1][cnt2])) / (2 * amp_x));
      }
      ldi[cnt1][0] = ldi[cnt1][dim_x-1] = 0;
  }
}

void drv_c_di(void)
{
  for(cnt1 = 0; cnt1 < (dim_x-1)*iamp_x; cnt1++)
  {
      for(cnt2 = 1; cnt2 < (dim_y-1); cnt2++)
      {
         cdi[cnt2][cnt1] = ((6 * (csigma[cnt2+1][cnt1] - csigma[cnt2][cnt1])) / (2 * amp_y));
      }
      ldi[0][cnt1] = ldi[dim_y-1][cnt1] = 0;
  }
}

void drv_l_k(void)
{
   for(cnt1 = 0; cnt1 < dim_y; cnt1++)
   {
      lk[cnt1][0] = 2;
      for(cnt2 = 1; cnt2 < (dim_x-1); cnt2++)
         lk[cnt1][cnt2] = 2 - (0.25 / lk[cnt1][cnt2-1]);
   }
}

void drv_c_k(void)
{
   for(cnt1 = 0; cnt1 < (dim_x-1) * iamp_x; cnt1++)
   {
      ck[0][cnt1] = 2;
      for(cnt2 = 1; cnt2 < (dim_y-1); cnt2++)
         ck[cnt2][cnt1] = 2 - (0.25 / ck[cnt2-1][cnt1]);
   }
}

void drv_l_z(void)
{
   for(cnt1 = 0; cnt1 < dim_y; cnt1++)
   {
      lz[cnt1][1] = 0;
      lz[cnt1][2] = ((0.5 * ldi[cnt1][1]) / lk[cnt1][0]);
      for(cnt2 = 3; cnt2 < dim_x; cnt2++)
         lz[cnt1][cnt2] = (((0.5*ldi[cnt1][cnt2-1]) - (0.5*lz[cnt1][cnt2-1])) / lk[cnt1][cnt2-2]);
   }
}

void drv_c_z(void)
{
   for(cnt1 = 0; cnt1 < (dim_x-1) * iamp_x; cnt1++)
   {
      cz[1][cnt1] = 0;
      cz[2][cnt1] = ((0.5 * cdi[1][cnt1]) / ck[0][cnt1]);
      for(cnt2 = 3; cnt2 < dim_y; cnt2++)
         cz[cnt2][cnt1] = (((0.5*cdi[cnt2-1][cnt1]) - (0.5*cz[cnt2-1][cnt1])) / ck[cnt2-2][cnt1]);
   }
}

void drv_l_mu(void)
{
   for(cnt1 = 0; cnt1 < dim_y; cnt1++)
   {
      lmu[cnt1][0] = lmu[cnt1][dim_x-1] = 0;
      for(cnt2 = (dim_x-2); cnt2 > 0; cnt2--)
         lmu[cnt1][cnt2] = ((ldi[cnt1][cnt2]-(0.5*lmu[cnt1][cnt2+1])-lz[cnt1][cnt2])/lk[cnt1][cnt2-1]);
   }
}

void drv_c_mu(void)
{
   for(cnt1 = 0; cnt1 < (dim_x-1) * iamp_x; cnt1++)
   {
      cmu[0][cnt1] = cmu[dim_y-1][cnt1] = 0;
      for(cnt2 = (dim_y-2); cnt2 > 0; cnt2--)
         cmu[cnt2][cnt1] = ((cdi[cnt2][cnt1]-(0.5*cmu[cnt2+1][cnt1])-cz[cnt2][cnt1])/ck[cnt2-1][cnt1]);
   }
}

void drv_l_spl(void)
{
   float cnt4,
         d_next,
         d_prev;

   int   cnt3;

   float f_swp1,
         f_swp2,
         f_swp3,
         f_swp4,
         spldata;

   for(cnt1 = 0; cnt1 < dim_y; cnt1++)
   {
      for(cnt2 = 1; cnt2 < dim_x - 1; cnt2++)
      {
         spl_mtrx[cnt1][(cnt2+1)*iamp_x] = matrix[cnt1][cnt2];
         for(cnt3 = 1; cnt3 < iamp_x; cnt3++)
         {
            d_next = amp_x-cnt3;
            d_prev = cnt3;
            f_swp1 = lmu[cnt1][cnt2-1]*(pow(d_next,3)/(6*amp_x));
            f_swp2 = lmu[cnt1][cnt2]*(pow(d_prev,3)/(6*amp_x));
            f_swp3 = (matrix[cnt1][cnt2-1]-((lmu[cnt1][cnt2-1]*amp_x*amp_x)/6))*(d_next/amp_x);
            f_swp4 = (matrix[cnt1][cnt2]-((lmu[cnt1][cnt2]*amp_x*amp_x)/6))*(d_prev/amp_x);
            spldata = f_swp1 + f_swp2 + f_swp3 + f_swp4;
            spl_mtrx[cnt1][(cnt2*iamp_x)+cnt3] = spldata;
         }
      }
   }
}

void drv_c_spl(void)
{
   float cnt4,
         d_next,
         d_prev;

   int   cnt3;

   float f_swp1,
         f_swp2,
         f_swp3,
         f_swp4,
         spldata;

   for(cnt1 = 0; cnt1 < dim_x*iamp_x; cnt1++)
   {
      for(cnt2 = 1; cnt2 < dim_y - 1; cnt2++)
      {
         spline[(cnt2+1)*iamp_y][cnt1] = spl_mtrx[cnt2][cnt1];
         for(cnt3 = 1; cnt3 < iamp_y; cnt3++)
         {
            d_next = amp_y-cnt3;
            d_prev = cnt3;
            f_swp1 = (cmu[cnt2-1][cnt1]*(pow(d_next,3)/(6*amp_y)));
            f_swp2 = (cmu[cnt2][cnt1]*(pow(d_prev,3)/(6*amp_y)));
            f_swp3 = (spl_mtrx[cnt2-1][cnt1]-((cmu[cnt2-1][cnt1]*amp_y*amp_y)/6))*(d_next/amp_y);
            f_swp4 = (spl_mtrx[cnt2][cnt1]-((cmu[cnt2][cnt1]*amp_y*amp_y)/6))*(d_prev/amp_y);
            spldata = f_swp1 + f_swp2 + f_swp3 + f_swp4;
            spline[(cnt2*iamp_y)+cnt3][cnt1] = spldata;
         }
      }
   }
}

