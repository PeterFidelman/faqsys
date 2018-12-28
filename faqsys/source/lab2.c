#include <stdio.h>
#include <gl/gl.h>
#include <gl/device.h>
#include <math.h>

#define WINSIZE 4 
#define BUFSIZE 50
#define RECT	1
#define CIRCLE	2
#define TRIANGLE	3
#define X	0
#define Y	1
#define XY	2
#define A	0
#define B	1
#define C	3 
#define CIRCX	0.0
#define CIRCY	2.0
#define RADIUS 1.0 
#define BACKROUND 0x000000
#define black 0x000000
#define white 0xffffff
#define red 0x0000ff
#define green 0x00ff00
#define yellow 0x00ffff
#define cyan 0xffff00
#define magenta 0xff00ff
#define blue 0xff0000
#define gray 0x555555
#define TRIAX 	1.5 
#define TRIAY	-3.0	 
#define TRIBX	3.5 
#define TRIBY	-3.0 
#define TRICX	2.5 
#define TRICY	-1.27 
#define SQX1	-3.5
#define SQY1	-3.0
#define SQX2	-1.5
#define SQY2	-1.0
#define RGB 3


struct Rect
{
	float lowerLeftX, lowerLeftY, upperRightX, upperRightY;
};
typedef struct Rect Rect;

struct Triangle
{
	float points[3][2];
};
typedef struct Triangle Triangle;

struct Rect Main_Rect;

struct Triangle Main_Triangle;

void draw_rect(Rect Obj)
{
  /* Draw clockwise */
	frontbuffer(TRUE);
	sboxf(Obj.lowerLeftX,Obj.lowerLeftY,Obj.upperRightX,Obj.upperRightY);
}

void erase_rect(Rect Obj)
{
   cpack(BACKROUND);
   draw_rect(Obj);
}

void draw_triangle(Triangle Obj)
{
	cpack(BACKROUND);
  bgnpolygon();
    v2f(Obj.points[0]);
    v2f(Obj.points[1]);
    v2f(Obj.points[2]);
  endpolygon();
}

void erase_triangle(Triangle Obj)
{
   cpack(BACKROUND);
	draw_triangle(Obj);
}

float distant_AB(float p1X, float p1Y, float p2X, float p2Y)
{
	float return_value;
	float X_dist;
	float Y_dist;

   X_dist = fabs(p2X - p1X);
   Y_dist = fabs(p2Y - p1Y);

  return_value = (sqrt(pow(X_dist, 2) + pow(Y_dist, 2)));
	return return_value;
}

float Rect_width(Rect Object)
{
   return (Object.upperRightX - Object.lowerLeftX);
}
 
float Rect_length(Rect Object)
{
   return (Object.upperRightY - Object.lowerLeftY);
}

void fract_Rect(Rect Obj)
{
   Rect temp = Obj;
   int i,j;
   static long status = 1;
   float Y_dis = Rect_length(Obj) / 3.0;
   float X_dis = Rect_width(Obj) / 3.0;
 
/* MAY WANT TO PLAY WITH THIS VALUE */
   if ((Y_dis < 0.02) || (X_dis < 0.02))
      /* It's small enough, we don't do anything */
      return;
   /* Erase the rectangle in the middle, call fract_Rect on the other eight
      rectangles */
   temp.lowerLeftX = Obj.lowerLeftX + X_dis;
   temp.lowerLeftY = Obj.lowerLeftY + Y_dis;
   temp.upperRightX = Obj.upperRightX - X_dis;
   temp.upperRightY = Obj.upperRightY - Y_dis;
 
   erase_rect(temp);
 
   for(i = 0; i < 3; i++)
      for (j = 0; j < 3; j++)
         if ((i != 1) || (j != 1))
         {
            temp.lowerLeftX = Obj.lowerLeftX + (i*X_dis);
            temp.lowerLeftY = Obj.lowerLeftY + (j*Y_dis);
            temp.upperRightX = temp.lowerLeftX + X_dis;
            temp.upperRightY = temp.lowerLeftY + Y_dis;
 
            fract_Rect(temp);
         }
}

void fract_Triangle(Triangle Obj)
{
	float Mid_AB[XY];
	float Mid_BC[XY];
	float Mid_AC[XY];
	float EdgeAB, EdgeAC, EdgeBC;
   static long status = 0;
   Triangle sub_triag1, sub_triag2, sub_triag3, sub_triag4;

/* MAY WANT TO TRY fabs HERE */
   EdgeAB = fabs(Obj.points[1][X] - Obj.points[0][X]);
   EdgeAC = distant_AB(Obj.points[0][X],Obj.points[0][Y], Obj.points[2][X],Obj.points[2][Y]);
   EdgeBC = distant_AB(Obj.points[1][X],Obj.points[1][Y], Obj.points[2][X],Obj.points[2][Y]);
 
	Mid_AB[X] = (Obj.points[0][X] + Obj.points[1][X]) / 2.0;
   Mid_AB[Y] = (Obj.points[0][Y] + Obj.points[1][Y]) / 2.0;

   Mid_BC[X] = (Obj.points[1][X] + Obj.points[2][X]) / 2.0;
   Mid_BC[Y] = (Obj.points[1][Y] + Obj.points[2][Y]) / 2.0;

   Mid_AC[X] = (Obj.points[0][X] + Obj.points[2][X]) / 2.0;
   Mid_AC[Y] = (Obj.points[0][Y] + Obj.points[2][Y]) / 2.0;

 
   if (EdgeAB <= 0.1 || EdgeAC <= 0.1 || EdgeBC <= 0.1)
     return;
 
   sub_triag1.points[0][X] = Mid_AB[X];
   sub_triag1.points[0][Y] = Mid_AB[Y];
 
   sub_triag1.points[1][X] = Mid_BC[X];
   sub_triag1.points[1][Y] = Mid_BC[Y];
 
   sub_triag1.points[2][X] = Mid_AC[X];
   sub_triag1.points[2][Y] = Mid_AC[Y];
 
   erase_triangle(sub_triag1);
   sub_triag2.points[1][X] = Mid_AB[X];
   sub_triag2.points[1][Y] = Mid_AB[Y];
 
   sub_triag2.points[2][X] = Mid_AC[X];
   sub_triag2.points[2][Y] = Mid_AC[Y];

	sub_triag2.points[0][X] = Obj.points[0][X];
   sub_triag2.points[0][Y] = Obj.points[0][Y];
   fract_Triangle(sub_triag2);
 
   sub_triag3.points[0][X] = Mid_AB[X];
   sub_triag3.points[0][Y] = Mid_AB[Y];
 
   sub_triag3.points[2][X] = Mid_BC[X];
   sub_triag3.points[2][Y] = Mid_BC[Y];

   sub_triag3.points[1][X] = Obj.points[1][X];
   sub_triag3.points[1][Y] = Obj.points[1][Y];
   fract_Triangle(sub_triag3);

   sub_triag4.points[0][X] = Mid_AC[X];
   sub_triag4.points[0][Y] = Mid_AC[Y];
 
   sub_triag4.points[1][X] = Mid_BC[X];
   sub_triag4.points[1][Y] = Mid_BC[Y];

   sub_triag4.points[2][X] = Obj.points[2][X];
   sub_triag4.points[2][Y] = Obj.points[2][Y];

   fract_Triangle(sub_triag4);
}

/*************************************************************************/

/* Points for the triangle */
float parray1[3][2] = {
	{TRIAX, TRIAY},
	{TRIBX, TRIBY},
	{TRICX, TRICY}
};

float midarray[3][2] = {
	{-1.0, -1.735},
	{1.0, -1.735},
	{0.0, 0.0}
};

float v[8][3] = {
    {-1.0, -1.0, -1.0},
    {-1.0, -1.0,  1.0},
    {-1.0,  1.0,  1.0},
    {-1.0,  1.0, -1.0},
    { 1.0, -1.0, -1.0},
    { 1.0, -1.0,  1.0},
    { 1.0,  1.0,  1.0},
    { 1.0,  1.0, -1.0},
};

struct object {
	float pos[XY];
	float lastpos[XY];
	float delta[XY];
	int col;
};

float parray2[3][2];
int mainmenu;
int colormenu;
int objmenu;
int curcolor;
int curobj;
struct object ball, square, tri;
long xorg, yorg;
long xsize, ysize;

unsigned short curs2[16] = {
	0x1ff0, 0x1ff0, 0x0820, 0x0820,
	0x0820, 0x0c60, 0x0c60, 0x0100,
	0x0100, 0x06c0, 0x0c60, 0x0820,
	0x0820, 0x0820, 0x1ff0, 0x1ff0
};

/* Draw the cursor as an hourglass */
void newCurs()
{
   curstype(C16X1);
   defcursor(1,curs2);
   setcursor(1,0,0);
}

/* Restore the original cursor */
void oldCurs()
{
   setcursor(0,0,0);
}

void setTriCol()
{
   switch(tri.col)
   {
      case 1:
         cpack(red);
         break;
      case 2:
         cpack(green);
         break;
      case 3:
         cpack(yellow);
         break;
      case 4:
         cpack(blue);
         break;
      case 5:
         cpack(magenta);
         break;
      case 6:
         cpack(cyan);
         break;
      case 7:
         cpack(white);
         break;
		case 8:
			cpack(gray);
			break;
   }
}

void setCircleCol()
{
   switch(ball.col)
   {
      case 1:
         cpack(red);
         break;
      case 2:
         cpack(green);
         break;
      case 3:
         cpack(yellow);
         break;
      case 4:
         cpack(blue);
         break;
      case 5:
         cpack(magenta);
         break;
      case 6:
         cpack(cyan);
         break;
      case 7:
         cpack(white);
         break;
		case 8:
			cpack(gray);
			break;
   }
}

void setSquareCol()
{
   switch(square.col)
   {
      case 1:
         cpack(red);
         break;
      case 2:
         cpack(green);
         break;
      case 3:
         cpack(yellow);
         break;
      case 4:
         cpack(blue);
         break;
      case 5:
         cpack(magenta);
         break;
      case 6:
         cpack(cyan);
         break;
      case 7:
         cpack(white);
         break;
		case 8:
			cpack(gray);
			break;
   }
}

void drawSquare()
{
	setSquareCol();
    bgnpolygon();
	    v3f(v[5]);
  		 v3f(v[1]);
  		 v3f(v[2]);
		 v3f(v[6]);
    endpolygon();
}

void drawTri()
{
   setTriCol();
    bgnpolygon();
       v3f(v[5]);
       v3f(v[1]);
       v3f(v[2]);
    endpolygon();
}

void drawit()
{
	frontbuffer(TRUE);
	loadname(1);
	cpack(cyan);
	RGBwritemask(0,0,255);
	sboxf(Main_Rect.lowerLeftX,Main_Rect.lowerLeftY,Main_Rect.upperRightX,Main_Rect.upperRightY);
	loadname(2);
	cpack(yellow);
   RGBwritemask(0,255,0);
	circf(CIRCX,CIRCY,RADIUS);
	loadname(3);
	cpack(magenta);
   RGBwritemask(255,0,0);
   bgnpolygon();
       v2f(parray1[0]);
       v2f(parray1[1]);
       v2f(parray1[2]);
   endpolygon();
	RGBwritemask(255,255,255);
}

void setcolor(n)
int n;
{
	frontbuffer(TRUE);
	curcolor = n;
	switch(curobj)
	{
		case 1:
			square.col = n;
			setSquareCol();
			sboxf(Main_Rect.lowerLeftX,Main_Rect.lowerLeftY,Main_Rect.upperRightX,Main_Rect.upperRightY);
			break;
		case 2:
			ball.col = n;
			setCircleCol();
			circf(CIRCX,CIRCY,RADIUS);
			break;
		case 3:
			tri.col = n;
			setTriCol();
		   bgnpolygon();
				v2f(parray1[0]);
       		v2f(parray1[1]);
				v2f(parray1[2]);
			endpolygon();
			break;
	}
}

setobj(n)
int n;
{
	setpup(objmenu,curobj,PUP_BOX);
   curobj = n;
	setpup(objmenu,curobj,PUP_CHECK);
   return -1;
}

short getobj(buffer,hits)
	int buffer[];
	long hits;
{
	if (hits > 0)
		return(buffer[0]);
	else
		return(0);
}

void drawOrig()
{
	frontbuffer(TRUE);
	cpack(BACKROUND);
	clear();
	cpack(cyan);
	square.col = 6;
	sboxf(Main_Rect.lowerLeftX,Main_Rect.lowerLeftY,Main_Rect.upperRightX,Main_Rect.upperRightY);
	cpack(yellow);
	ball.col = 3;
	circf(CIRCX,CIRCY,RADIUS);
	cpack(magenta);
	tri.col = 5;
   bgnpolygon();
       v2f(parray1[0]);
       v2f(parray1[1]);
       v2f(parray1[2]);
   endpolygon();

}

void redraw()
{
	frontbuffer(TRUE);
   cpack(BACKROUND);
   clear();
	setSquareCol();
   sboxf(Main_Rect.lowerLeftX,Main_Rect.lowerLeftY,Main_Rect.upperRightX,Main_Rect.upperRightY);
	setCircleCol();
   circf(CIRCX,CIRCY,RADIUS);
	setTriCol();
   bgnpolygon();
       v2f(parray1[0]);
       v2f(parray1[1]);
       v2f(parray1[2]);
   endpolygon();

}

void initialize()
{
	prefsize(700,700);
	winopen("Lab 2");
	getsize(&xsize, &ysize);
	mmode(MVIEWING);
	ortho(-4.0,4.0,-4.0,4.0,-4.0,4.0);
	qdevice(LEFTMOUSE);
	qdevice(MENUBUTTON);
	getorigin(&xorg, &yorg);
	RGBmode();
	doublebuffer();
	gconfig();
	cpack(BACKROUND);
	clear();
}

void fractalize()
{
	Triangle temp;

	cpack(BACKROUND);
	clear();
	newCurs();
	setTriCol();
   bgnpolygon();
       v2f(parray1[0]);
       v2f(parray1[1]);
       v2f(parray1[2]);
   endpolygon();
   setCircleCol();
   circf(CIRCX,CIRCY,RADIUS);
	setSquareCol();
	draw_rect(Main_Rect);
	switch(curobj)
	{
		case 1:
			fract_Rect(Main_Rect); 
			sleep(2);
			break;
		case 2:
			break;
		case 3:
			fract_Triangle(Main_Triangle);
			sleep(2);
			break;
	}
	redraw();
	oldCurs();
}

void floatObj()
{
short val;
Angle xrot, yrot, zrot;

	newCurs();
	qdevice(ESCKEY);
	switch(curobj)
	{
		case 1:
			xrot = yrot = zrot = 0;
			pushmatrix();
			cpack(BACKROUND);
			clear();
			while (!(qtest() && qread(&val) == ESCKEY && val == 0))
			{
				pushmatrix();
  		      rotate(xrot, 'x');
       		rotate(yrot, 'y');
       		rotate(zrot, 'z');
       		cpack(BACKROUND);
       		clear();
      		zbuffer(FALSE);
       		pushmatrix();
      			scale(1.2, 1.2, 1.2);
      			translate(0.3, 0.2, 0.2);
      			drawSquare();
     			popmatrix();
   			popmatrix();
				swapbuffers();
   			xrot += 11;
			   yrot += 15;
			   if (xrot + yrot > 3500)
       			zrot += 23;
			   if (xrot > 3600)
       			xrot -= 3600;
			   if (yrot > 3600)
       			yrot -= 3600;
			   if (zrot > 3600)
       			zrot -= 3600;
			}
			popmatrix();
			redraw();
			break;
		case 2:
			break;
		case 3:
         xrot = yrot = zrot = 0;
         pushmatrix();
         cpack(BACKROUND);
         clear();
         while (!(qtest() && qread(&val) == ESCKEY && val == 0))
         {
            pushmatrix();
            rotate(xrot, 'x');
            rotate(yrot, 'y');
            rotate(zrot, 'z');
            cpack(BACKROUND);
            clear();
            zbuffer(FALSE);
            pushmatrix();
               scale(1.2, 1.2, 1.2);
               translate(0.3, 0.2, 0.2);
               drawTri();
            popmatrix();
            popmatrix();
            swapbuffers();
            xrot += 11;
            yrot += 15;
            if (xrot + yrot > 3500)
               zrot += 23;
            if (xrot > 3600)
               xrot -= 3600;
            if (yrot > 3600)
               yrot -= 3600;
            if (zrot > 3600)
               zrot -= 3600;
         }
			popmatrix();
			redraw();
			break;
	}
	unqdevice(ESCKEY);
	oldCurs();
}

void animate()
{
int i, j, x, y,w;
Boolean cont = TRUE;
short val;

	qdevice(ESCKEY);
	switch(curobj)
	{
		case 1:
			square.pos[X] = (SQX2 + SQX1) / 2.0;
			square.pos[Y] = (SQY1 + SQY2) / 2.0;
			square.delta[X] = 0.1;
			square.delta[Y] = 0.05;
         while(!(qtest() && qread(&val) == ESCKEY && val == 0))
         {
				newCurs();
            for (j=0; j < XY; j++)
            {
               square.lastpos[j] = square.pos[j];
               square.pos[j] += square.delta[j];
               if ((square.pos[j] >= 3.0) || (square.pos[j] <= -3.0))
                  square.delta[j] = -square.delta[j];
            }
            cpack(BACKROUND);
				clear();
				setTriCol();	
			   bgnpolygon();
       			v2f(parray1[0]);
       			v2f(parray1[1]);
       			v2f(parray1[2]);
			   endpolygon();
				setCircleCol();
            circf(CIRCX,CIRCY,RADIUS);
				setSquareCol();
				sboxf(square.pos[X]-1.0,square.pos[Y]-1.0,square.pos[X]+1.0,square.pos[Y]+1.0);
            swapbuffers();
         }
			oldCurs();
         redraw();
			break;

		case 2:
			ball.pos[X] = CIRCX;
			ball.pos[Y] = CIRCY;
			ball.delta[X] = 0.07;
			ball.delta[Y] = 0.05;
			while(!(qtest() && qread(&val) == ESCKEY && val == 0))
			{
				newCurs();
				ball.lastpos[X] = ball.pos[X];
				ball.pos[X] += ball.delta[X];
/* This makes the ball bounce off the walls */
				for (j=0; j < XY; j++)
				{
					ball.lastpos[j] = ball.pos[j];
					ball.pos[j] += ball.delta[j];
					if ((ball.pos[j] >= (WINSIZE - RADIUS)) || (ball.pos[j] <= -3.0))
						ball.delta[j] = -ball.delta[j];
				}
				cpack(BACKROUND);
				clear();
				setTriCol();
			   bgnpolygon();
       			v2f(parray1[0]);
					v2f(parray1[1]);
			      v2f(parray1[2]);
			   endpolygon();
				setSquareCol();
			   sboxf(Main_Rect.lowerLeftX,Main_Rect.lowerLeftY,Main_Rect.upperRightX,Main_Rect.upperRightY); 
				setCircleCol();
				circf(ball.pos[X], ball.pos[Y], RADIUS);
				swapbuffers(); 
			}
			oldCurs();
			redraw();
			break;
		case 3:
         tri.pos[X] = (TRIAX + TRIBX) / 2.0;
         tri.pos[Y] = (TRIAY + TRICY) / 2.0;
         tri.delta[X] = 0.01;
         tri.delta[Y] = 0.05;

         while(!(qtest() && qread(&val) == ESCKEY && val == 0))
         {
				newCurs();
				tri.lastpos[X] = tri.pos[X];
				tri.pos[X] += tri.delta[X];
				if ((tri.pos[X] >= 3.0) || (tri.pos[X] <= -3.0))
					tri.delta[X] = -tri.delta[X];
            tri.lastpos[Y] = tri.pos[Y];
            tri.pos[Y] += tri.delta[Y];
            if ((tri.pos[Y] >= 3.135) || (tri.pos[Y] <= -3.135))
               tri.delta[Y] = -tri.delta[Y];

            cpack(BACKROUND);
				clear();
				setTriCol();
				parray2[0][0] = tri.pos[X] - 1.0;
				parray2[0][1] = tri.pos[Y] - 0.865;
            parray2[1][0] = tri.pos[X] + 1.0;
            parray2[1][1] = tri.pos[Y] - 0.865;
            parray2[2][0] = tri.pos[X];
            parray2[2][1] = tri.pos[Y] + 0.865;

			   bgnpolygon();
       			v2f(parray2[0]);
			      v2f(parray2[1]);
       			v2f(parray2[2]);
			   endpolygon();
				setSquareCol();
				sboxf(Main_Rect.lowerLeftX,Main_Rect.lowerLeftY,Main_Rect.upperRightX,Main_Rect.upperRightY);
				setCircleCol();
            circf(CIRCX,CIRCY,RADIUS);
				setSquareCol();
            swapbuffers();
         }
			oldCurs();
			redraw();
			break;
	}
	unqdevice(ESCKEY);
}

int rand_gen(int low, int high)
{
	long j,random;
	float temp2;
	double temp;

	temp = pow(2,15)-1;
	random = rand();
	temp2 = random/temp;
	j = (temp2*(high-low)+low);
	return j;
}

void circ_particles(int low, int high, int radius)
{
   int i,temp,temp_col,new_col;
   float x,y,vert[XY],vert1[XY],vert2[XY],vert3[XY],vert4[XY];

	temp_col = ball.col;
	new_col = 1;
	ball.col = new_col; 
   for (i=0;i<=3000;i++)
   {
		setCircleCol();
      temp = rand_gen(low,high);
      vert1[X] = temp / 100.0;
      temp = rand_gen(low,high);
      vert1[Y] = temp / 100.0;
		if (sqrt(pow(vert1[X],2) + pow(vert1[Y],2)) <= radius)
		{
      	bgnpoint();
      	v2f(vert1);
      	endpoint();
		}
      temp = rand_gen(low,high);
      vert2[X] = temp / 100.0 * (-1);
      temp = rand_gen(low,high);
      vert2[Y] = temp / 100.0;
      if (sqrt(pow(vert2[X],2) + pow(vert2[Y],2)) <= radius)
      {
         bgnpoint();
         v2f(vert2);
         endpoint();
      }
      temp = rand_gen(low,high);
      vert3[X] =  temp / 100.0;
      temp = rand_gen(low,high);
      vert3[Y] = (temp / 100.0) * (-1);
      if (sqrt(pow(vert3[X],2) + pow(vert3[Y],2)) <= radius)
      {
         bgnpoint();
         v2f(vert3);
         endpoint();
      }
      temp = rand_gen(low,high);
      vert4[X] = (temp / 100.0) * (-1);
      temp = rand_gen(low,high);
      vert4[Y] = (temp / 100.0) * (-1);
      if (sqrt(pow(vert4[X],2) + pow(vert4[Y],2)) <= radius)
      {
         bgnpoint();
         v2f(vert4);
         endpoint();
      }
		new_col++;
		if (new_col == 8)
			new_col = 1;
		ball.col = new_col;
   }
	ball.col = temp_col;
}

void square_particles(int low, int high, float Xoffset, float Yoffset)
{
	int i,temp,temp_col,new_col;
	float x,y,vert[XY],vert1[XY],vert2[XY],vert3[XY],vert4[XY];

	for (i=0;i<=3000;i++)
	{
		temp = rand_gen(low,high);
		vert1[X] = temp / 100.0;
      temp = rand_gen(low,high);
		vert1[Y] = temp / 100.0;
		bgnpoint();
		v2f(vert1);
		endpoint();
		temp = rand_gen(low,high);
      vert2[X] = temp / 100.0 * (-1);  
      temp = rand_gen(low,high);
      vert2[Y] = temp / 100.0;
      bgnpoint();
      v2f(vert2);
      endpoint();
      temp = rand_gen(low,high);
      vert3[X] =  temp / 100.0;  
      temp = rand_gen(low,high);
      vert3[Y] = (temp / 100.0) * (-1);
      bgnpoint();
      v2f(vert3);
      endpoint();
      temp = rand_gen(low,high);
      vert4[X] = (temp / 100.0) * (-1);
      temp = rand_gen(low,high);
      vert4[Y] = (temp / 100.0) * (-1);
      bgnpoint();
      v2f(vert4);
      endpoint();
	}
}

float getSlope(float x1,float y1, float x2, float y2)
{
	return (y1 - y2)/(x1 - x2);
}

void tri_particles(int low, int high)
{
   int i,temp,temp_col,new_col;
   float x,y,vert[XY],vert1[XY],vert2[XY],vert3[XY],vert4[XY];
	float slope;

   for (i=0;i<=5000;i++)
   {
      temp = rand_gen(low,high);
      vert1[X] = temp / 100.0;
      temp = rand_gen(low,high);
      vert1[Y] = temp / 100.0;
		slope = getSlope(vert1[X],vert1[Y],0.0,0.0);
		if (slope >= 1.0)
		{
      	bgnpoint();
      	v2f(vert1);
      	endpoint();
		}	
      temp = rand_gen(low,high);
      vert2[X] = temp / 100.0 * (-1);
      temp = rand_gen(low,high);
      vert2[Y] = temp / 100.0;

      slope = getSlope(vert2[X],vert2[Y],0.0,0.0);
      if (slope <= -1.0)
      {
         bgnpoint();
         v2f(vert2);
         endpoint();
      }

      temp = rand_gen(low,high);
      vert3[X] =  temp / 100.0; 
      temp = rand_gen(low,high);
      vert3[Y] = (temp / 100.0) * (1);
      slope = getSlope(vert3[X],vert3[Y],0.0,0.0);
      if (slope >= 1.0)
      {
         bgnpoint();
         v2f(vert3);
         endpoint();
      } 
      temp = rand_gen(low,high);
      vert4[X] = (temp / 100.0) * (-1);
      temp = rand_gen(low,high);
      vert4[Y] = (temp / 100.0) * (1);
      slope = getSlope(vert4[X],vert4[Y],0.0,0.0);
      if (slope <= -1.0)
      {
         bgnpoint();
         v2f(vert4);
         endpoint();
      }

   }
}

void particle_sys()
{
	float x,y,vert[XY],vert1[XY], vert2[XY], vert3[XY], vert4[XY];
	float count, rad, old_rad;
	int i;
	int temp,temp_col,new_col;

	switch(curobj)
	{
		case 1:
			cpack(BACKROUND);
			clear();
			setSquareCol();
			sboxfi(-1.0,-1.0,1.0,1.0);
			sleep(1);
         cpack(BACKROUND);
         clear();
			setSquareCol();
			square_particles(1,100,-3.0,-3.0);
			square_particles(101,200,-3.0,-3.0);
			square_particles(201,300,-3.0,-3.0);
			square_particles(301,400,-3.0,-3.0);
			break;
		case 2:
			cpack(BACKROUND);
			clear();
			setCircleCol();	
			circf(0.0,0.0,1.0);
			sleep(1);
         cpack(BACKROUND);
         clear();
			setCircleCol();
         circ_particles(1,100,1);
         circ_particles(1,200,2);
         circ_particles(1,300,3);
         circ_particles(1,400,4);
			circ_particles(1,500,5);
			break;
		case 3:
         cpack(BACKROUND);
         clear();
			setTriCol();
   		bgnpolygon();
       		v2f(midarray[0]);
       		v2f(midarray[1]);
       		v2f(midarray[2]);
		   endpolygon();
         tri_particles(1,100);
         tri_particles(1,200);
         tri_particles(1,300);
         tri_particles(1,400);
         break;
	}
	sleep(1);
	redraw();
}

main()
{
	Boolean run;
	Device dev;
	short val;
	short buffer[BUFSIZE];
	long hits;
	int pupval, n,i,j;

	Main_Rect.lowerLeftX = SQX1;
	Main_Rect.lowerLeftY = SQY1;
	Main_Rect.upperRightX = SQX2;
	Main_Rect.upperRightY = SQY2;

	Main_Triangle.points[0][X] = TRIAX;
	Main_Triangle.points[0][Y] = TRIAY;
	Main_Triangle.points[1][X] = TRIBX;
	Main_Triangle.points[1][Y] = TRIBY;
	Main_Triangle.points[2][X] = TRICX;
	Main_Triangle.points[2][Y] = TRICY;

	initialize();
	drawOrig();
	objmenu = defpup("Objects %t %F|Rectangle|Circle|Triangle", setobj);
	colormenu = defpup("Colors %t %F|Gray %x8|Blue %x4|Yellow %x3|\
		Cyan %x6|Magenta %x5|Green %x2|White %x7|Red %x1", setcolor);
	mainmenu = defpup("Main %t|Set Color %m|Choose Object %m|Fractalize|\
		Bounce|Particle System|Float|Original|Exit", colormenu,objmenu);
	run = TRUE;
	curobj = 1;
	setpup(objmenu,1,PUP_CHECK);
	setpup(objmenu,2,PUP_BOX);
	setpup(objmenu,3,PUP_BOX);
	while(run)
	{
		switch(qread(&val))
		{
			case LEFTMOUSE:
				pick(buffer, BUFSIZE);
				ortho(-4.0,4.0,-4.0,4.0,-4.0,4.0);
					drawit();
				hits = endpick(buffer);
				ortho(-4.0,4.0,-4.0,4.0,-4.0,4.0);
				setpup(objmenu,curobj,PUP_BOX);
				curobj = getobj(buffer,hits);
				setpup(objmenu,curobj,PUP_CHECK);
				break;
			case MENUBUTTON:
				if(val)
				{
					pupval = dopup(mainmenu);
					switch(pupval)
					{
						case 3:
							fractalize();
							break;
						case 4:
							animate();
							break;
						case 5:
							particle_sys();
							break;
						case 6:
							floatObj();
							break;
						case 7:
                     drawOrig();
                     break;
						case 8:
                     gflush();
                     gexit();
					}
				}
		}
	}
}
