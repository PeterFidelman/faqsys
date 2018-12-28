/*==============================================================================

    texmc3.c
    (C) Willi Geiger 1997. All rights reserved.

    Texture machine for CGI.
    Uses code from NCSA.

==============================================================================*/



#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <float.h>
#include <string.h>
#include <time.h>
#include <Wit.h>



/*------------------------------------------------------------------------------
   NCSA declarations
------------------------------------------------------------------------------*/

#define MAX_ENTRIES 128

typedef struct {
    char *name;
    char *val;
} entry;

char *makeword(char *line, char stop);
char *fmakeword(FILE *f, char stop, int *len);
char x2c(char *what);
void unescape_url(char *url);
void plustospace(char *str);


/*------------------------------------------------------------------------------
    My globals
------------------------------------------------------------------------------*/


typedef enum {
  CosineShape, SawtoothShape, TriangleShape, SquareShape
} PatternShape;

typedef enum {
  RoundShape, SoftShape, GlowShape, BubbleShape
} ParticleShape;

typedef enum {
  ApBpCpD, AmBmCmD, AmBpCpD, ApBpCmD, AmBpCmD, ApBmCpD
} MixType;


/* my functions within this module */
static void   texmc(entry[], int);
static void   ParseInput(entry [], int);
static void   MakeColours(void);
static void   MakeTexture(void);
static float *MakeParticles(void);
static float  Turbulence(float, float, float, float);
static float  Noise(float, float);
static float  HorizPattern(float);
static float  VertPattern(float);
static float  ShapeSample(float, PatternShape);
static float  Spline(float, const float [], int);


/* store the texture parameters as global vars */
static int width = 256, height = 256;
static float stereo = 0;
static float tvol = 1, vvol = 0, hvol = 0, pvol = 0;
static PatternShape vshape = CosineShape, hshape = CosineShape;
static ParticleShape pshape = RoundShape;
static int tmagic = 0;
static int vfreq = 1, hfreq = 1, pnum = 64;
static float tdetail = 0, vdetail = 0, hdetail = 0;
static int psize = 4;
static MixType mix = ApBpCpD;
static float mod = 0;
static float bias = 0.5, gain = 0.5;
static float red[6]   = { -0.33, 0.00, 0.33, 0.67, 1.0, 1.3 };
static float green[6] = { -0.33, 0.00, 0.33, 0.67, 1.0, 1.3 };
static float blue[6]  = { -0.33, 0.00, 0.33, 0.67, 1.0, 1.3 };



/*------------------------------------------------------------------------------
   Mainly NCSA code.
------------------------------------------------------------------------------*/

main(int argc, char *argv[]) {
    entry entries[MAX_ENTRIES];
    register int x,m=0;
    int cl;

    printf("Content-type: text/html%c%c",10,10);

    if(strcmp(getenv("REQUEST_METHOD"),"POST")) {
        printf("This script should be referenced with a METHOD of POST.\n");
        printf("If you don't understand this, see this ");
        printf("<A HREF=\"http://www.ncsa.uiuc.edu/SDG/Software/Mosaic/Docs/fill-out-forms/overview.html\">forms overview</A>.%c",10);
        exit(1);
    }

    if(strcmp(getenv("CONTENT_TYPE"),"application/x-www-form-urlencoded")) {
        printf("This script can only be used to decode form results. \n");
        exit(1);
    }

    cl = atoi(getenv("CONTENT_LENGTH"));

    for(x=0;cl && (!feof(stdin));x++) {
        m=x;
        entries[x].val = fmakeword(stdin,'&',&cl);
        plustospace(entries[x].val);
        unescape_url(entries[x].val);
        entries[x].name = makeword(entries[x].val,'=');
    }


    texmc(entries, m);
}




/*------------------------------------------------------------------------------
    My declarations
------------------------------------------------------------------------------*/

#define NumColours    256
#define Size           16
#define MaxOctaves      8
#define MaxWidth      720
#define MaxHeight     576
#define MaxAmplitude    8
#define MaxParticles 1024
#define MaxStereo      32


const float Pi = 3.1415926536;

/* static array of colours */
struct {
  unsigned char r, g, b;
}
colours[NumColours];


#define Bias(t, a) (t)/((1.0/(a) - 2.0) * (1.0 - (t)) + 1.0)


#define Gain(t, a) ((t) < 0.5) ?\
  (t)/((1.0/(a) - 2.0) * (1.0 - 2.0*(t)) + 1.0) :\
  ((1.0/(a) - 2.0) * (1.0 - 2.0*(t)) - (t))/((1.0/(a) - 2.0) * (1.0 - 2.0*(t)) - 1.0)


/*------------------------------------------------------------------------------
    texmc
--------------------------------------------------------------------------------
    Inputs:       the decoded input
    Returns:      int - program return status
    Side-effects: none
    Description:  My main routine.
------------------------------------------------------------------------------*/

static void texmc(entry entries[], int num)
{
  ParseInput(entries, num);
  MakeColours();
  MakeTexture();
}



/*------------------------------------------------------------------------------
    ParseInput
--------------------------------------------------------------------------------
    Inputs:       entries - the translated data from the form
                  the number of entries
    Returns:      none
    Side-effects: Sets the values of the global texture vars.
    Description:  Parses the input and sets the texture vars from this.
------------------------------------------------------------------------------*/

static void ParseInput(entry entries[], int num)
{

  register int i;
  char vshapestr[16], hshapestr[16], pshapestr[16], mixstr[16];


  /* fill in the values of the texture parameters */
  for (i = 0; i <= num; i++) {
    if (!strcmp(entries[i].name, "width"))
      width = (int) atof(entries[i].val);
    else if (!strcmp(entries[i].name, "height"))
      height = (int) atof(entries[i].val);
    else if (!strcmp(entries[i].name, "stereo"))
      stereo = atof(entries[i].val);
    else if (!strcmp(entries[i].name, "vshape"))
      strcpy(vshapestr, entries[i].val);
    else if (!strcmp(entries[i].name, "hshape"))
      strcpy(hshapestr, entries[i].val);
    else if (!strcmp(entries[i].name, "pshape"))
      strcpy(pshapestr, entries[i].val);
    else if (!strcmp(entries[i].name, "tvol"))
      tvol = atof(entries[i].val);
    else if (!strcmp(entries[i].name, "vvol"))
      vvol = atof(entries[i].val);
    else if (!strcmp(entries[i].name, "hvol"))
      hvol = atof(entries[i].val);
    else if (!strcmp(entries[i].name, "pvol"))
      pvol = atof(entries[i].val);
    else if (!strcmp(entries[i].name, "tmagic"))
      tmagic = (int) atof(entries[i].val);
    else if (!strcmp(entries[i].name, "vfreq"))
      vfreq = (int) atof(entries[i].val);
    else if (!strcmp(entries[i].name, "hfreq"))
      hfreq = (int) atof(entries[i].val);
    else if (!strcmp(entries[i].name, "pnum"))
      pnum = (int) atof(entries[i].val);
    else if (!strcmp(entries[i].name, "tdetail"))
      tdetail = atof(entries[i].val);
    else if (!strcmp(entries[i].name, "vdetail"))
      vdetail = atof(entries[i].val);
    else if (!strcmp(entries[i].name, "hdetail"))
      hdetail = atof(entries[i].val);
    else if (!strcmp(entries[i].name, "psize"))
      psize = atof(entries[i].val);
    else if (!strcmp(entries[i].name, "mix"))
      strcpy(mixstr, entries[i].val);
    else if (!strcmp(entries[i].name, "mod"))
      mod = atof(entries[i].val);
    else if (!strcmp(entries[i].name, "bias"))
      bias = atof(entries[i].val);
    else if (!strcmp(entries[i].name, "gain"))
      gain = atof(entries[i].val);
    else if (!strcmp(entries[i].name, "red1"))
      red[1] = atof(entries[i].val);
    else if (!strcmp(entries[i].name, "green1"))
      green[1] = atof(entries[i].val);
    else if (!strcmp(entries[i].name, "blue1"))
      blue[1] = atof(entries[i].val);
    else if (!strcmp(entries[i].name, "red2"))
      red[2] = atof(entries[i].val);
    else if (!strcmp(entries[i].name, "green2"))
      green[2] = atof(entries[i].val);
    else if (!strcmp(entries[i].name, "blue2"))
      blue[2] = atof(entries[i].val);
    else if (!strcmp(entries[i].name, "red3"))
      red[3] = atof(entries[i].val);
    else if (!strcmp(entries[i].name, "green3"))
      green[3] = atof(entries[i].val);
    else if (!strcmp(entries[i].name, "blue3"))
      blue[3] = atof(entries[i].val);
    else if (!strcmp(entries[i].name, "red4"))
      red[4] = atof(entries[i].val);
    else if (!strcmp(entries[i].name, "green4"))
      green[4] = atof(entries[i].val);
    else if (!strcmp(entries[i].name, "blue4"))
      blue[4] = atof(entries[i].val);
  }


  /* set the options */
  if (!strcmp(vshapestr, "cosine"))
    vshape = CosineShape;
  else if (!strcmp(vshapestr, "sawtooth"))
    vshape = SawtoothShape;
  else if (!strcmp(vshapestr, "triangle"))
    vshape = TriangleShape;
  else if (!strcmp(vshapestr, "square"))
    vshape = SquareShape;

  if (!strcmp(hshapestr, "cosine"))
    hshape = CosineShape;
  else if (!strcmp(hshapestr, "sawtooth"))
    hshape = SawtoothShape;
  else if (!strcmp(hshapestr, "triangle"))
    hshape = TriangleShape;
  else if (!strcmp(hshapestr, "square"))
    hshape = SquareShape;

  if (!strcmp(pshapestr, "round"))
    pshape = RoundShape;
  else if (!strcmp(pshapestr, "soft"))
    pshape = SoftShape;
  else if (!strcmp(pshapestr, "glow"))
    pshape = GlowShape;
  else if (!strcmp(pshapestr, "bubble"))
    pshape = BubbleShape;

  if (!strcmp(mixstr, "A+B+C+D"))
    mix = ApBpCpD;
  else if (!strcmp(mixstr, "A*B*C*D"))
    mix = AmBmCmD;
  else if (!strcmp(mixstr, "A*(B+C+D)"))
    mix = AmBpCpD;
  else if (!strcmp(mixstr, "(A+B+C)*D"))
    mix = ApBpCmD;
  else if (!strcmp(mixstr, "A*(B+C)*D"))
    mix = AmBpCmD;
  else if (!strcmp(mixstr, "A+(B*C)+D"))
    mix = ApBmCpD;


  /* check all the values */
  if (width < 1) width = 1;
  if (width > MaxWidth) width = MaxWidth;
  if (height < 1) height = 1;
  if (height > MaxHeight) height = MaxHeight;

  if (stereo < -1) stereo = -1;
  if (stereo >  1) stereo =  1;

  if (fabs(tvol) > MaxAmplitude) tvol = MaxAmplitude;
  if (fabs(vvol) > MaxAmplitude) vvol = MaxAmplitude;
  if (fabs(hvol) > MaxAmplitude) hvol = MaxAmplitude;
  if (fabs(pvol) > MaxAmplitude) pvol = MaxAmplitude;

  if (fabs(hfreq) > width/2)  hfreq = width/2;
  if (fabs(vfreq) > height/2) vfreq = height/2;

  if (tdetail < 0) tdetail = 0;
  if (tdetail > 1) tdetail = 1;
  tdetail = tdetail * MaxOctaves + FLT_EPSILON;
  if (vdetail < 0) vdetail = 0;
  if (vdetail > 1) vdetail = 1;
  vdetail = vdetail * MaxOctaves + FLT_EPSILON;
  if (hdetail < 0) hdetail = 0;
  if (hdetail > 1) hdetail = 1;
  hdetail = hdetail * MaxOctaves + FLT_EPSILON;

  if (pnum < 0) pnum = 0;
  if (pnum > MaxParticles) pnum = MaxParticles;
  if (psize < 1) psize = 1;
  if (psize > width/2) psize = width/2;
  if (psize > height/2) psize = height/2;
  
  if (bias < FLT_EPSILON) bias = FLT_EPSILON;
  if (bias > 1 - FLT_EPSILON) bias = 1 - FLT_EPSILON;
  if (gain < FLT_EPSILON) gain = FLT_EPSILON;
  if (gain > 1 - FLT_EPSILON) gain = 1 - FLT_EPSILON;
  gain = 1.0 - gain;

  if (red[1] < 0) red[1] = 0;
  if (red[1] > 1) red[1] = 1;
  if (green[1] < 0) green[1] = 0;
  if (green[1] > 1) green[1] = 1;
  if (blue[1] < 0) blue[1] = 0;
  if (blue[1] > 1) blue[1] = 1;
  if (red[2] < 0) red[2] = 0;
  if (red[2] > 1) red[2] = 1;
  if (green[2] < 0) green[2] = 0;
  if (green[2] > 1) green[2] = 1;
  if (blue[2] < 0) blue[2] = 0;
  if (blue[2] > 1) blue[2] = 1;
  if (red[3] < 0) red[3] = 0;
  if (red[3] > 1) red[3] = 1;
  if (green[3] < 0) green[3] = 0;
  if (green[3] > 1) green[3] = 1;
  if (blue[3] < 0) blue[3] = 0;
  if (blue[3] > 1) blue[3] = 1;
  if (red[4] < 0) red[4] = 0;
  if (red[4] > 1) red[4] = 1;
  if (green[4] < 0) green[4] = 0;
  if (green[4] > 1) green[4] = 1;
  if (blue[4] < 0) blue[4] = 0;
  if (blue[4] > 1) blue[4] = 1;
  red[0] = 2*red[1] - red[2], red[5] = 2*red[4] - red[3];
  green[0] = 2*green[1] - green[2], green[5] = 2*green[4] - green[3];
  blue[0] = 2*blue[1] - blue[2], blue[5] = 2*blue[4] - blue[3];

}



/*------------------------------------------------------------------------------
    MakeParticles
--------------------------------------------------------------------------------
    Inputs:       none
    Returns:      void
    Side-effects: fills in the colours in info
    Description:  Creates the array of pixel values for the colours.
------------------------------------------------------------------------------*/

static float *MakeParticles(void)
{

  int rad2 = psize * psize;
  int i;
  float *particles = (float *) malloc(width * height * sizeof(float));
  float pdecay = 0;

  if (!particles)
    return NULL;

  memset(particles, 0, pnum * sizeof(float));


  /* draw the particles */
  srand(tmagic);

  for (i = 0; i < pnum; i++) {

    int x0 = width  + rand() % width;
    int y0 = height + rand() % height;
    register int x, y, x1, y1;

    for (y = -psize; y < psize; y++) {
      y1 = (y0 + y) % height;
      for (x = -psize; x < psize; x++) {
        int d2 = x*x + y*y;
        if (d2 > rad2)
          continue;
        x1 = (x0 + x) % width;
        switch (pshape) {
          case RoundShape: particles[y1*width + x1] += 1.0; break;
          case SoftShape: particles[y1*width + x1] += 1.0/(1.0 + 2.0*d2/rad2); break;
          case GlowShape: particles[y1*width + x1] += 1.0/(16.0*d2/rad2); break;
          case BubbleShape:
            particles[y1*width + x1] += (1.0 - particles[y1*width + x1]) * 1.0 * d2/rad2;
            break;
          default: particles[y1*width + x1] += 0.0; break;
        }
      }
    }

  }


  return particles;

}



/*------------------------------------------------------------------------------
    MakeColours
--------------------------------------------------------------------------------
    Inputs:       none
    Returns:      void
    Side-effects: fills in the colours in info
    Description:  Creates the array of pixel values for the colours.
------------------------------------------------------------------------------*/

static void MakeColours(void)
{

  register int i;

  for (i = 0; i < NumColours; i++) {

    float shade = (float) i/(NumColours - 1);
    float r = 255 * Spline(shade, red,   6);
    float g = 255 * Spline(shade, green, 6);
    float b = 255 * Spline(shade, blue,  6);

    if (r < 0) r = 0;
    if (r > 255) r = 255;
    if (g < 0) g = 0;
    if (g > 255) g = 255;
    if (b < 0) b = 0;
    if (b > 255) b = 255;

    colours[i].r = (unsigned char) (r);
    colours[i].g = (unsigned char) (g);
    colours[i].b = (unsigned char) (b);

  }

}



/*------------------------------------------------------------------------------
    MakeTexture
--------------------------------------------------------------------------------
    Inputs:       none
    Returns:      void
    Side-effects: fills in the colours in info
    Description:  This is the function that actually makes the texture.
------------------------------------------------------------------------------*/

static void MakeTexture(void)
{

  register int x, y;
  register WitXFrame xframe = WitCreateXFrame(width, height);
  char basen[128], imgfn[128], imgpn[128];
  float *particles = NULL;
  float *texture = NULL;


  /* allocate memory to store the texture (so we can do stereo efficiently) */
  if (!(texture = (float *) malloc(width * height * sizeof(float)))) {
    printf("Error rendering texture - please try again later\n");
    exit(1);
  }


  if (!xframe) {
    printf("Error rendering texture - please try again later\n");
    exit(1);
  }


  /* create the particles */
  if (pnum > 0 && !(particles = MakeParticles())) {
    printf("Error rendering texture - please try again later\n");
    exit(1);
  }

  
  /* do all x and y */
  for (y = 0; y < height; y++) {
    for (x = 0; x < width; x++) {

      float xx = (float) x/width;
      float yy = (float) y/height;
      float a = 0, b = 0, c = 0, d = 0, shade = 0;
      unsigned int col = 0;


      /* turbulence */
      if (tvol > FLT_EPSILON)
        a = tvol * Turbulence(Size * xx, Size * yy, tdetail, 0.5);

      /* vertical pattern */
      if (abs(vfreq) > 0 && fabs(vvol) > FLT_EPSILON)
        b = vvol * VertPattern(yy + mod * (a + HorizPattern(xx)));

      /* horizontal pattern */
      if (abs(hfreq) > 0 && fabs(hvol) > FLT_EPSILON)
        c = hvol * HorizPattern(xx + mod * (a + VertPattern(yy)));

      /* particles */
      if (fabs(pvol) > 0 && pnum > 0) {
        int xp = (int) (x + mod * (a + b) * width  + 0.5);
        int yp = (int) (y + mod * (a + c) * height + 0.5);
        xp %= width, yp %= height;
        if (xp < 0) xp += width;
        if (yp < 0) yp += height;
        d = 2 * pvol * particles[yp * width + xp] - 1;
      }

      /* mix them all together to make the shade value */
      switch (mix) {
        case ApBpCpD:
          shade = a + b + c + d;
          break;
        case AmBmCmD:
          shade = 1;
          if (fabs(tvol) > FLT_EPSILON) shade *= (1 - fabs(tvol)) + a;
          if (fabs(vvol) > FLT_EPSILON) shade *= (1 - fabs(vvol)) + b;
          if (fabs(hvol) > FLT_EPSILON) shade *= (1 - fabs(hvol)) + c;
          if (fabs(pvol) > FLT_EPSILON) shade *= (1 - fabs(pvol)) + d;
          break;
        case AmBpCpD:
          shade = b + c + d;
          if (fabs(tvol) > FLT_EPSILON) shade *= (1 - fabs(tvol)) + a;
          break;
        case ApBpCmD:
          shade = a + b + c;
          if (fabs(pvol) > FLT_EPSILON) shade *= (1 - fabs(pvol)) + d;
          break;
        case AmBpCmD:
          shade = b + c;
          if (fabs(tvol) > FLT_EPSILON) shade *= (1 - fabs(tvol)) + a;
          if (fabs(pvol) > FLT_EPSILON) shade *= (1 - fabs(pvol)) + d;
          break;
        case ApBmCpD:
          shade = 1;
          if (fabs(vvol) > FLT_EPSILON) shade *= (1 - fabs(vvol)) + b;
          if (fabs(hvol) > FLT_EPSILON) shade *= (1 - fabs(hvol)) + c;
          shade += a + d;
          break;
        default:
           shade = 0;
           break;
      }
      shade = 0.5 * (shade + 1);

      /* clamp the shade value and apply bias and gain */
      if (shade < 0) shade = 0;
      if (shade > 1) shade = 1;
      shade = Bias(shade, bias);
      shade = Gain(shade, gain);


      /* store the texel in the texture memory */
      texture[y*width + x] = shade;

    }
  }

  free(particles);


  /* make the texture stereo if desired */
  if (fabs(stereo) > FLT_EPSILON) {
    for (y = 0; y < height; y++) {
      for (x = 0; x < width; x++) {
        float *texel = texture + y*width + x;
        int x1 = (x + (int) (stereo * MaxStereo * *texel + 0.5)) % width;
        *texel = 0.5 * *texel + 0.5 * texture[y * width + x1];
      }
    }
  }
        

  /* map the texture values on to colours in the XFrame */
  for (y = 0; y < height; y++) {
    for (x = 0; x < width; x++) {
        unsigned int col =
        (unsigned int) (texture[y*width + x] * (NumColours - 1));
      WitXFSetPixel24(xframe, x, y, colours[col].r,
                                    colours[col].g,
                                    colours[col].b);
    }
  }

  free(texture);


  /* remove any old textures and make a new unique filename */
  system("rm /usr/local/etc/httpd/htdocs/Staff/Willi/TextureMC/texmc*.jpg");
  sprintf(basen,  "%d", time(NULL));
  sprintf(imgfn,  "/Staff/Willi/TextureMC/texmc%s.jpg", basen);
  sprintf(imgpn,  "/usr/local/etc/httpd/htdocs%s", imgfn);


  /* write the WitXFrame as a JPEG file and destroy it */
  WitWriteImageFile(xframe, imgpn);
  WitFreeXFrame(xframe);


  /* output the HTML commands to show the texture tiling */
  printf("<html>\n");
  printf("<head>\n");
  printf("<title>The Texture Machine</title>\n");
  printf("</head>\n");
  printf("<body background=\"%s\" link=\"#00ffff\"\
          vlink=\"#00ffff\">\n", imgfn);
  printf("<center>\n");
  printf("<table cols=1 frame=box border=5>\n");
  printf("<tbody><tr><td>\n");
  printf("<a href=\"%s\">\
            <img src=\"%s\" border=3 alt=\"Download image...\"></a>\n",
            imgfn, imgfn);
  printf("</td></tr></tbody></table>\n");
  printf("</center>\n");
  printf("</body>\n");
  printf("</html>");    
  
}



/*------------------------------------------------------------------------------
    Turbulence
--------------------------------------------------------------------------------
    Fractal turbulence using the noise function.
    Lacunarity is fixed at 2 so that the texture wraps nicely.
------------------------------------------------------------------------------*/

static float Turbulence(float x, float y, float octaves, float H)
{

  static int first = 1;
  static float exponents[MaxOctaves];
  float freq = 1, rem;
  register float val = 0;
  register int i;


  /* initialise the turbulence if necessary */
  if (first) {
    float freq = 1;
    first = 0;

    for (i = 0; i <= MaxOctaves; i++) {
      exponents[i] = pow(freq, -H);
      freq *= 2;
    }
  }


  /* calculate the actual turbulence value */
  if (octaves < 0 || octaves > MaxOctaves)
    octaves = MaxOctaves;

  for (i = 0; i < octaves; i++) {
    val += Noise(x, y) * exponents[i];
    x *= 2, y *= 2;
  }

  rem = octaves - (int) octaves;
  if (rem > FLT_EPSILON)
    val += rem * Noise(x, y) * exponents[i];

  return val;

}



/*------------------------------------------------------------------------------
    Noise
--------------------------------------------------------------------------------
    Perlin's noise function.
------------------------------------------------------------------------------*/

static float Noise(float x, float y)
{

  static int first = 1;
  static float gx[Size + Size + 2], gy[Size + Size + 2];
  static int perm[Size + Size + 2];
  int i, j, bx0, by0, bx1, by1, b00, b01, b10, b11;
  float t, rx0, ry0, rx1, ry1, wx, wy, u, v, a, b, c;


  /* initialise the noise if necessary */
  if (first) {
    first = 0;

    /* create an array of random gradient vectors on a unit circle */
    srand(tmagic);
    for (i = 0; i < Size; i++) {
      float a = 2.0 * Pi * rand()/RAND_MAX;
      gx[i] = sin(a);
      gy[i] = cos(a);
    }

    /* create a pseudo-random permutation of [1..Size] */
    for (i = 0; i < Size; i++)
      perm[i] = i;

    for (i = Size; i > 0; i -= 2) {
      int tmp = perm[i];
      int j = rand() % Size;
      perm[i] = perm[j];
      perm[j] = tmp;
    }

    /* extend the gradient and permutation arrays for faster indexing */
    for (i = 0; i < Size + 2; i++) {
      gx[Size + i] = gx[i];
      gy[Size + i] = gy[i];
      perm[Size + i] = perm[i];
    }

  }


  /* now calculate the actual noise value */
  t = x + 10000;
  bx0 = ((int) t) & (Size - 1);
  bx1 = (bx0 + 1) & (Size - 1);
  rx0 = t - (int) t;
  rx1 = rx0 - 1;

  t = y + 10000;
  by0 = ((int) t) & (Size - 1);
  by1 = (by0 + 1) & (Size - 1);
  ry0 = t - (int) t;
  ry1 = ry0 - 1;

  i = perm[bx0];
  j = perm[bx1];

  b00 = perm[i + by0];
  b10 = perm[j + by0];
  b01 = perm[i + by1];
  b11 = perm[j + by1];

  wx = rx0 * rx0 * (3 - 2 * rx0);
  wy = ry0 * ry0 * (3 - 2 * ry0);

  u = rx0 * gx[b00] + ry0 * gy[b00];
  v = rx1 * gy[b10] + ry0 * gy[b10];
  a = u + wx * (v - u);

  u = rx0 * gx[b01] + ry1 * gy[b01];
  v = rx1 * gy[b11] + ry1 * gy[b11];
  b = u + wx * (v - u);

  return 1.5 * (a + wy * (b - a));

}



/*------------------------------------------------------------------------------
    HorizPattern
--------------------------------------------------------------------------------
    Returns a sample of the horizontal pattern.
    Generates the whole pattern if necessary (first invocation).
------------------------------------------------------------------------------*/

static float HorizPattern(float x)
{

  static float *samples = NULL;
  register int x0;
  

  /* initialise the samples if necessary */
  if (samples == NULL) {

    samples = (float *) malloc(width * sizeof(float));
    if (samples == NULL) return 0;

    for (x0 = 0; x0 < width; x0++) {
      register int i;
      int f = hfreq;
      float a = 1;
      samples[x0] = 0;
      for (i = 0; i < hdetail; i++) {
        samples[x0] += a * ShapeSample(f * (float) x0/width, hshape);
        f *= 2;
        if (f > width/2) break;
        a *= 0.707;
      }
    }

  }


  /* make sure that the sample point is within range */
  x = fmod(x, 1);
  if (x < 0) x += 1;


  /* interpolate the value from the samples */
  x *= width;
  x0 = (int) x;
  x -= x0;
  return samples[x0] + x * (samples[(x0 + 1) % width] - samples[x0]);

}



/*------------------------------------------------------------------------------
    VertPattern
--------------------------------------------------------------------------------
    Returns a sample of the vertical pattern.
    Generates the whole pattern if necessary (fisrt invocation).
------------------------------------------------------------------------------*/

static float VertPattern(float y)
{

  static float *samples = NULL;
  register int y0;  

  /* initialise the samples if necessary */
  if (samples == NULL) {

    samples = (float *) malloc(height * sizeof(float));
    if (samples == NULL) return 0;

    for (y0 = 0; y0 < height; y0++) {
      register int i;
      int f = vfreq;
      float a = 1;
      samples[y0] = 0;
      for (i = 0; i < vdetail; i++) {
        samples[y0] += a * ShapeSample(f * (float) y0/height, vshape);
        f *= 2;
        if (f > height/2) break;
        a *= 0.707;
      }
    }

  }


  /* make sure that the sample point is within range */
  y = fmod(y, 1);
  if (y < 0) y += 1;


  /* interpolate the value from the samples */
  y *= height;
  y0 = (int) y;
  y -= y0;
  return samples[y0] + y * (samples[(y0 + 1) % height] - samples[y0]);

}
      



/*------------------------------------------------------------------------------
    ShapeSample
--------------------------------------------------------------------------------
    Returns a sample of a periodic function of a variable shape
------------------------------------------------------------------------------*/

static float ShapeSample(float t, PatternShape shape)
{

  switch (shape) {
    case CosineShape: return cos(2.0 * Pi * t); break;
    case SawtoothShape: return 2.0 * fmod(t, 1) - 1; break;
    case TriangleShape: return 4.0 * fabs(fmod(t, 1) - 0.5) - 1; break;
    case SquareShape: return ((fmod(t, 1) > 0.5) ? 1 : -1); break;
    default: return 0; break;
  }

}



/*------------------------------------------------------------------------------
    Spline
--------------------------------------------------------------------------------
    Catmull-Rom spline function
------------------------------------------------------------------------------*/

static float Spline(float x, const float knots[], int nk)
{

  const int MaxKnots = 64;
  int span;
  const float *k;
  float c0, c1, c2, c3;

  if (nk < 0 || nk > MaxKnots)
    return 0;

  if (x < 0) x = 0;
  if (x > 1) x = 1;
  x *= nk - 3;

  span = (int) x;
  if (span >= nk - 3)
    span = nk - 4;
  x -= span;
  k = knots + span;


  c3 = -0.5 * k[0] + 1.5 * k[1] - 1.5 * k[2] + 0.5 * k[3];
  c2 =  1.0 * k[0] - 2.5 * k[1] + 2.0 * k[2] - 0.5 * k[3];
  c1 = -0.5 * k[0] + 0.0 * k[1] + 0.5 * k[2] + 0.0 * k[3];
  c0 =  0.0 * k[0] + 1.0 * k[1] + 0.0 * k[2] + 0.0 * k[3];

  return ((c3 * x + c2) * x + c1) * x + c0;
  
}
