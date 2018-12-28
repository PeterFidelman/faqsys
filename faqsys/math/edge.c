From - Fri Jun  5 11:05:16 1998
Path: ifi.uio.no!news-feed.ifi.uio.no!fu-berlin.de!news-peer.gip.net!news.gsl.net!gip.net!news-peer.sprintlink.net!news-backup-west.sprintlink.net!news.sprintlink.net!194.168.4.5!news7-gui.server.cableol.net!newsfeed.cableol.net!newsfeed1.cableol.net!cableol.net!newsfeed.cableol.net!usenet
From: "Osama M. Shaban" <osama.shaban@diamond.co.uk>
Newsgroups: comp.graphics.algorithms
Subject: Re: Edge detect algorithm
Date: Mon, 25 May 1998 21:54:43 +0000
Organization: Cable OnLine Limited
Lines: 438
Message-ID: <3569E8A3.2F7E@diamond.co.uk>
References: <6kback$h7$1@lgdx06.lg.ehu.es>
NNTP-Posting-Host: p17-bellbird-gri.tch.cableol.net
Mime-Version: 1.0
Content-Type: multipart/mixed; boundary="------------28A31E4B522C"
X-Mailer: Mozilla 2.02E-KIT  (Win95; I)
To: Ander Lozano Perez <jtalopea@bi.ehu.es>
Xref: ifi.uio.no comp.graphics.algorithms:69143

This is a multi-part message in MIME format.

--------------28A31E4B522C
Content-Type: text/plain; charset=us-ascii
Content-Transfer-Encoding: 7bit

Hi;

Try my own code for Robert edge detector.

Regards

Shaban

Ander Lozano Perez wrote:
> 
> --
> I'm looking for an edge detect algorithm. Can some one help me?
> 
>         Thanks.
> 
>         Hasta pronto
> 
>                 Agaliaretph.

--------------28A31E4B522C
Content-Type: text/plain; charset=us-ascii
Content-Transfer-Encoding: 7bit
Content-Disposition: inline; filename="Nrobert.cpp"

#include<stdio.h>
#include <stdlib.h>
#include <math.h>
#include <conio.h>

static double sqrarg;

#define SQR(a) ((sqrarg=(a)) == 0.0 ? 0.0 : sqrarg*sqrarg)

#define   MAX(a,b) ((a) > (b) ? (a) : (b))

void put(int *x,int *temp);int round(double num);
FILE *open_file(char *msga, char *msgwr);
void fint_write(int *ax,FILE *fout,char *oimage);
void fchar_write(int *ax,FILE *fout,char *oimage);
unsigned char *allocate_char(int n);
int *allocate_int(int n);
void fill_zero(int *ptr);
double *allocate_double(int n);
void abslt(int *s);
void convert_char(int *a,unsigned char *pixel);
void convert_int(unsigned char *pixel,int *a);
void normalize(int *axy,double *dxy,double fctr );
void transform(int *tmp,int *x,int w[2][2],int *a);
void transform1(int *x,int w[2][2],int *Gxy);
void transform2(int *x,int *y,int w[2][2],int *Gxy);
void magnitude(double *dx,double *dy, double *xd, double *yd, int *ax);
void angle(double *dx,double *dy,int *ax);
void normalize255(int *ax);
void error_rd(char *ptr);
void sum(int *ah,int *av);
void error_wr(char *ptr);

int width;

int main(void)
{
   clrscr();
   int i,r,x[512],ax[512],ay[512],tmp[512];//,n;
   int xa[512], ya[512];
   double dx[512],dy[512], xd[512], yd[512], fctr;
   unsigned char ipixel[512],opixel[512];
   char i_image[50],o_image[50],a_image[50];
   FILE *fin,*fout,*fa;  //*fax,*fay;
   int wx[2][2] ={1,0,0,-1}; //{0,1,-1,0};
   int wy[2][2] ={0,1,-1,0}; //{1,0,0,-1};
   int xw[2][2] ={0,0,-1,1};
   int yw[2][2] ={1,0,-1,0};

   printf("\n\n\n\n\n\n");
   printf("               enter input image file..............>");
   scanf("%s",i_image);
   printf("               enter differential image file.......>");
   scanf("%s",o_image);
   printf("               enter angle image file..............>");
   scanf("%s",a_image);
//   printf("               enter input image width.............>");
//   scanf("%d",&width);
//   printf("               enter normalization factor (2.0)....>");
//   scanf("%lf",&fctr);
   fctr=1.0;
   width=512;

   clrscr();
   printf("\n\n   Processing.....");
   //n=width;
   fin = open_file(i_image,"rb");
   fout= open_file(o_image,"wb");
   fa= open_file(a_image,"wb");
   //fax= open_file("lena.ax","wb");  fay= open_file("lena.ay","wb") ;

   if (fread(ipixel,1,width,fin) != width)     error_rd(i_image);
   convert_int(ipixel,tmp);
   for (r=1; r<=width; r++)  {
	  if (r == width) // do not read input image file.
	fill_zero(x);
	  else {
	if (fread(ipixel,1,width,fin) != width)  error_rd(i_image);
	convert_int(ipixel,x);
	  }
	  transform(tmp,x,wx,ax);
	  transform(tmp,x,wy,ay);
	  transform(tmp,x,xw,xa);
	  transform(tmp,x,yw,ya);

	  normalize(ax,dx,fctr);        //ax[i] is int, dx[i] is double normalzd o/p.
	  normalize(ay,dy,fctr);        //ay[i] is int, dy[i] is double normalzd o/p.
	  normalize(xa,xd,fctr);        //ay[i] is int, dy[i] is double normalzd o/p.
	  normalize(ya,yd,fctr);        //ay[i] is int, dy[i] is double normalzd o/p.

	  abslt(ax);abslt(ay);
	  abslt(xa);abslt(ya);
/*
	  printf("\n\n\n\nbefore dx, dy");
	  printf("\n%.3f   %.3f   %.3f   %.3f   %.3f@", dx[0], dx[1], dx[2], dx[3], dx[4]);
	  printf("\n%.3f   %.3f   %.3f   %.3f   %.3f@", dy[0], dy[1], dy[2], dy[3], dy[4]);
	  printf("\n\nbefore xd, yd");
	  printf("\n%.3f   %.3f   %.3f   %.3f   %.3f@", xd[0], xd[1], xd[2], xd[3], xd[4]);
	  printf("\n%.3f   %.3f   %.3f   %.3f   %.3f@", yd[0], yd[1], yd[2], yd[3], yd[4]);
*/
	  magnitude(dx,dy,xd,yd,ax);
/*
	  printf("\n\n\n\nAfter dx, dy");
	  printf("\n%.3f   %.3f   %.3f   %.3f   %.3f@", dx[0], dx[1], dx[2], dx[3], dx[4]);
	  printf("\n%.3f   %.3f   %.3f   %.3f   %.3f@", dy[0], dy[1], dy[2], dy[3], dy[4]);
	  printf("\n\nAfter xd, yd");
	  printf("\n%.3f   %.3f   %.3f   %.3f   %.3f@", xd[0], xd[1], xd[2], xd[3], xd[4]);
	  printf("\n%.3f   %.3f   %.3f   %.3f   %.3f@", yd[0], yd[1], yd[2], yd[3], yd[4]);

	  getch();
	  clrscr();
*/
	  normalize255(ax);
	  //max(ax,ay);
	  //theta(dx,dy,ay);            //ay[i] is int o/p angle.
	  angle(dx,dy,ay);
//	  angle(xd,yd,ya);

//	  printf("\n%.3f",ay[g]);
//	  for(int g=0; g<512; g++) {
//	  ay[g]=MAX(abs(round(ay[g])),abs(round(ay[g])));
//	  }
	  fchar_write(ay,fa,a_image);    //for writing the angle output.
	  fchar_write(ax,fout,o_image);  //for writing the magnitude output.
	  put(x,tmp);
   }
   clrscr();
   printf("\nFinished succecifuly !!!!!"); getch();
   fclose(fin);
   fclose(fout);
   fclose(fa);
   //fclose(fax);fclose(fay);
   return 1;
}


FILE *open_file(char *msga, char *msgwr)
{

	FILE *fo;
	fo=fopen(msga, msgwr);
	if (fo == NULL) {
	  printf("unable to open %s file", msga);
	  exit(1);
	}
	return fo;
}

void fchar_write(int *ax,FILE *fout,char *oimage)
{
	unsigned char opixel[512];

	convert_char(ax,opixel);
	if (fwrite(opixel,1,width,fout) != width)   error_wr(oimage);
}

void fint_write(int *ax,FILE *fout,char *oimage)
{
	if (fwrite(ax,sizeof(int),width,fout) != width)   error_wr(oimage);
}


void put(int *x,int *temp)
{
  int i;
  for (i=0; i<width; i++)
	 temp[i]=x[i];
}


int round(double x)
{
   if (x > 0.0) return (int)(x + 0.5);
   else         return (int)(x - 0.5);
}

void fill_zero(int *ptr)
{
   int i;
   for (i=0; i<width; i++)
	  ptr[i]=0;
}


void normalize(int *axy,double *dxy,double fctr )
{
   int i;

   for (i=0; i<width; i++) {
	 dxy[i]=(double)axy[i]/fctr;
	 axy[i]=round(dxy[i]);
   }
}

void normalize255(int *ax)
{
  int i;

  for (i=0; i<width; i++) {
	 if (ax[i] > 255)
	   ax[i] = 255;
  }
}

unsigned char *allocate_char(int n)
{

  unsigned char *ptr;
  if((ptr=(unsigned char*) malloc(n))==NULL){
	printf("couldn't allocate required space\n");
	exit(1);
  }
  return ptr;
}

int *allocate_int(int n)
{

  int *ptr;
  if((ptr=(int *) malloc(n*2))==NULL){
	printf("couldn't allocate required space\n");
	exit(1);
  }
  return ptr;
}

double *allocate_double(int n)
{

  double *ptr;
  if((ptr=(double *) malloc(n*8))==NULL){
	printf("couldn't allocate required space\n");
	exit(1);
  }
  return ptr;
}

void convert_char(int *a,unsigned char *pixel)
{

  int i;
  for (i=0; i<width; i++)
	 pixel[i] =(unsigned char)a[i];
}

void convert_int(unsigned char *pixel,int *a)
{

  int i;
  for (i=0; i<width; i++)
	 a[i] =(int)pixel[i];
}

void transform(int *tmp,int *x,int w[2][2],int *a)
{

   int i;

   for (i=0; i<width; i++) {
	 if (i == (width-1))
	   a[i]=(w[0][0]*tmp[i] + w[1][0]*x[i]);
	 else
	   a[i]=(w[0][0]*tmp[i] + w[0][1]*tmp[i+1] +
		 w[1][0]*x[i]   + w[1][1]*x[i+1]   );
//   printf("\na[%d] = %d", i, a[i]); getch();
   }
}

//void magnitude(double *dx,double *dy,int *ax)
void magnitude(double *dx,double *dy, double *xd, double *yd, int *ax)
{
   int i;
   double p,q,r,s,t,u;

   for (i=0; i<width; i++) {
	  p=q=r=s=t=u=0.0;
	  p=fabs(dx[i]);
	  q=fabs(dy[i]);
	  r=fabs(xd[i]);
	  s=fabs(yd[i]);
	 // mag=sqrt(SQR(p) + SQR(q));
	  t=sqrt(p*p+q*q);
	  u=sqrt(r*r+s*s);

	  if(u>t) {dx[i]=xd[i];
			   dy[i]=yd[i];}

	  ax[i]=round(sqrt(dx[i]*dx[i]+dy[i]*dy[i]));

   }
}


void angle(double *dx,double *dy,int *ay)
{

   int i;
   double radians,degrees,y,x;
   double abs_x,abs_y,sx,sy;

   for (i=0; i<width; i++) {
	  x=dx[i];
	  y=dy[i];
	  abs_x=fabs(x); abs_y=fabs(y);
	   if (x < 0.0)   sx = -1.0;
	   else           sx =  1.0;
	   if (y < 0.0)   sy = -1.0;
	   else           sy =  1.0;
	   if ((x == 0.0) && (y == 0.0))
	 radians=0.0;
	   else
	if (x == 0.0)
	  radians=1.570796327;
	else
	 radians=sx*sy*atan2(abs_y,abs_x);
	degrees=(180.0*radians)/3.141592654;
		if (degrees < 0.0)
		  degrees = degrees + 90.0;      //in the original was + 180.0
		ay[i]=round(degrees);
		//direction=quantize_angle(degrees);
   }
}
/*
void theta(double *px,double *py,int *pz)
{
   int  i,dx,dy,ax,ay,t,angle;

   for (i=0; i<width; i++) {
	  dx=round(px[i]);  ax=abs(dx);
	  dy=round(py[i]);  ay=abs(dy);
	  if ((dx == 0) && (dy == 0))
	t=0;
	  else
	t=dy/(ax+ay);
	  if (dx<0)
	t=2-t;
	  else
	  if (dy<0)
	t=2+t;
	  angle=t*90;
	  pz[i]=angle;
   }
}
*/
/*
int quantize_angle(double angle)
{
   int ang;
   if ((angle >= 0.0)  &&  (angle <=22.5))   ang=0;
   else
   if ((angle < 0.0)   &&  (angle >= -22.5)) ang=0;
   else
   if ((angle >22.5)   &&  (angle <= 67.5))  ang=45;
   else
   if ((angle >67.5)   &&  (angle <= 90.0))  ang=90;
   else
   if ((angle <-22.5)  &&  (angle >=-67.5))  ang=135;
   else
   if ((angle <-67.5 ) &&  (angle >=-90.0))  ang=90;

   return ang;
}
*/
/*
void MX(int *ah, int *av)
{

  int i;
  for (i=0; i<width; i++)
  {
	 ah[i]=MX(ah[i],av[i]);
  }
}
*/
void abslt(int *s)
{
  int i;
  for (i=0; i<width; i++)
	 s[i]=abs(s[i]);
}


void sum(int *ah,int *av)
{

  int i;
  for (i=0; i<width; i++)
	 ah[i] = ah[i]+av[i];
}



void error_rd(char *ptr)
{

   printf("can not read %s file\n",ptr);
   exit(1);
}

void error_wr(char *ptr)
{

   printf("can not write %s file\n",ptr);
   exit(1);
}

--------------28A31E4B522C--

