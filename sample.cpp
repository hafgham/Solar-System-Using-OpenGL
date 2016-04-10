#include <stdio.h>
// yes, I know stdio.h is not good C++, but I like the *printf( )
#include <stdlib.h>
#include <ctype.h>

#define _USE_MATH_DEFINES
#include <math.h>

#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#include "glew.h"
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include "glut.h"
#include "glui.h"

//
//
//	This is a sample OpenGL / GLUT / GLUI program
//
//	The objective is to draw a 3d object and change the color of the axes
//		with radio buttons
//
//	The left mouse button allows rotation
//	The middle mouse button allows scaling
//	The glui window allows:
//		1. The 3d object to be transformed
//		2. The projection to be changed
//		3. The color of the axes to be changed
//		4. The axes to be turned on and off
//		5. The transformations to be reset
//		6. The program to quit
//
//	Author:			Joe Graphics
//
//  Latest update:	March 27, 2013
//


//
// constants:
//
// NOTE: There are a bunch of good reasons to use const variables instead
// of #define's.  However, Visual C++ does not allow a const variable
// to be used as an array size or as the case in a switch( ) statement.  So in
// the following, all constants are const variables except those which need to
// be array sizes or cases in switch( ) statements.  Those are #defines.
//
//


// title of these windows:

const char *WINDOWTITLE = { "Solar System" };
const char *GLUITITLE   = { "User Interface Window" };


// what the glui package defines as true and false:

const int GLUITRUE  = { true  };
const int GLUIFALSE = { false };


// the escape key:

#define ESCAPE		0x1b


// initial window size:

const int INIT_WINDOW_SIZE = { 600 };


// size of the box:

const float BOXSIZE = { 2.f };



// multiplication factors for input interaction:
//  (these are known from previous experience)

const float ANGFACT = { 1. };
const float SCLFACT = { 0.005f };


// able to use the left mouse for either rotation or scaling,
// in case have only a 2-button mouse:

enum LeftButton
{
	ROTATE,
	SCALE
};


// minimum allowable scale factor:

const float MINSCALE = { 0.05f };


// active mouse buttons (or them together):

const int LEFT   = { 4 };
const int MIDDLE = { 2 };
const int RIGHT  = { 1 };


// which projection:

enum Projections
{
	ORTHO,
	PERSP
};


// which button:

enum ButtonVals
{
	RESET,
	QUIT
};


// window background color (rgba):

const float BACKCOLOR[ ] = { 0., 0., 0., 0. };


// line width for the axes:

const GLfloat AXES_WIDTH   = { 3. };


// the color numbers:
// this order must match the radio button order


//const int	All ;
const int Mercury = 0;
const int	Venus =1; 
const int	Earth = 2;
const int	Mars = 3;
const int	Jupiter = 4;
const int	Saturn = 5;
const int	Uranus = 6;
const int	Nebtune = 7;
const int	Pluto = 8;
const int	Sun = 9;


int freezebox;


// the color definitions:
// this order must match the radio button order



const int	EducationalSolar = 0;
const int	Solar = 1;
const int	PlanetTop= 2;
const int	Planet = 3;
const int	PlanetSurrounding = 4;



// the color definitions:
// this order must match the radio button order

const GLfloat Planets[ ][3] = 
{
	0,		// Solar
	1,		// PlanetTop
	2,		// Planet

};



// fog parameters:

const GLfloat FOGCOLOR[4] = { .0, .0, .0, 1. };
const GLenum  FOGMODE     = { GL_LINEAR };
const GLfloat FOGDENSITY  = { 0.30f };
const GLfloat FOGSTART    = { 1.5 };
const GLfloat FOGEND      = { 4. };



//
// non-constant global variables:
//

int	ActiveButton;			// current button that is down
GLuint	AxesList;			// list to hold the axes
int	AxesOn;					// != 0 means to draw the axes
int	DebugOn;				// != 0 means to print debugging info
int	DepthCueOn;				// != 0 means to use intensity depth cueing
GLUI *	Glui;				// instance of glui window
int	GluiWindow;				// the glut id for the glui window
int	LeftButton;				// either ROTATE or SCALE
GLuint	BoxList;			// object display list
int	MainWindow;				// window id for main graphics window
GLfloat	RotMatrix[4][4];	// set by glui rotation widget
float	Scale, Scale2;		// scaling factors
int	WhichColor;				// index into Colors[ ]
int WhichCamera;			// Index of Cameras positions
int WhichPlanet;
int SaturnRingTrans;
int	WhichProjection;		// ORTHO or PERSP
int	Xmouse, Ymouse;			// mouse values
float	Xrot, Yrot;			// rotation angles in degrees
float	TransXYZ[3];		// set by glui translation widgets
int		Texture_Options;

//int CameraPoistion;
int LinesEnabled;
bool Freeze = true;
//bool CameraPoistion = true;
bool CameraPoistionEarth = false;
bool CameraPoistionMoon = false;
bool Light1On = true;
const int MS_PER_CYCLE = 365;
float Time;

//
// function prototypes:
//

void	Animate( void );
void	Buttons( int );
void	Display( void );
void	DoRasterString( float, float, float, char * );
void	DoStrokeString( float, float, float, float, char * );
float	ElapsedSeconds( void );
void	InitGlui( void );
void	InitGraphics( void );
void	InitLists( void );
void	Keyboard( unsigned char, int, int );
void	MouseButton( int, int, int, int );
void	MouseMotion( int, int );
void	Reset( void );
void	Resize( int, int );
void	Visibility( int );

void	Arrow( float [3], float [3] );
void	Cross( float [3], float [3], float [3] );
float	Dot( float [3], float [3] );
float	Unit( float [3], float [3] );
void	Axes( float );
void	HsvRgb( float[3], float [3] );


float White[ ] = { 1.,1.,1.,1. };
float Green[ ] = { 0.,1.,0.,1. };


float *
	MulArray3( float factor, float array0[3] )
{
	static float array[4];
	array[0] = factor * array0[0];
	array[1] = factor * array0[1];
	array[2] = factor * array0[2];
	array[3] = 1.;
	return array;
}

float *
	Array3( float a, float b, float c )
{
	static float array[4];
	array[0] = a;
	array[1] = b;
	array[2] = c;
	array[3] = 1.;
	return array;
}


//Animation Control 

float TimeControl= 1.0;



float EducationalSolarJupiture_distance = 140;
float EducationalSolarVenus_distance = 40;
float EducationalSolarEarth_distance = 60 ;
float EducationalSolarMars_distance = 80;
float EducationalSolarSaturn_distance = 220;
float EducationalSolarUranus_distance = 260;
float EducationalSolarNeptune_distance = 300;
float EducationalSolarPluto_dostance = 340;


float DistanceScaler;

float  SizeScaler = .4;
float  SunSize = 109;
float  SpaceSize = 100000;

// Distance Astronimal Units, Size and Orbital peroids
float MercurySize = 0.383 ;
float mercurydistance =  0.4 ;
float mercury;
float mercurySpin;

float VenusSize = 0.949 ;
float Venus_distance =  0.7 ;
float venus;
float venusSpin;

float Earth_distance = 1 ;
float EarthSize = 1 ;
float earth; 
float earthMoonDistance = 0.00257  ;
float earthMoonSize = 0.374;
float earthMoon;
float earthSpin;

float Mars_distance = 1.5 ;
float MarsSize = 0.533 ;
float mars;
float marsSpin;
float marsdiemosmoonDistance = 0.000015;
float marsdiemosmoonSize = 0.0017;
float marsdiemosmoon;
float marsphobosmoonDistance = 0.00015;
float marsphobosmoonSize = 0.00097;
float marsphobosmoon;

float Jupiture_distance = 5.2;
float JupitureSize = 11.209;
float jupiter;
float jupiterSpin;
float jupiteriomoonDistance = 0.00282 ;
float jupiteriomoonSize = 0.245;
float jupiteriomoon;
float jupitereuropamoonDistance  = 0.00449 ;
float jupitereuropamoonSize = .245;
float jupitereuropamoon;
float jupitercallistomoonDistance  = 0.0126;
float jupitercallistomoonSize = .377;
float jupitercallistomoon;
float jupiterganymedemoonDistance  = 0.00716;
float jupiterganymedemoonSize = .413;
float jupiterganymedemoon;
//float JupiterMoon;

float Saturn_distance = 9.54 ;
float SaturnSize = 9.449 ;
float SaturnRingOuterRadius = 22.0;
float SaturnRingInnerRadius = 17;
float saturn;
float saturnSpin;
float saturndionemoonDistance =  0.00257;
float saturndionemoonSize = 0.0881;
float saturndionemoon;
float saturnenceladusmoonDistance = 0.0016;
float saturnenceladusmoonSize = 0.0395;
float saturnenceladusmoon;
float saturnlapetusmoonDistance = 0.0238;
float saturnlapetusmoonSize = 0.1154;
float saturnlapetusmoon;
float saturnmimasmooonDistance = 0.0012;
float saturnmimasmooonSize = 0.03;
float saturnmimasmooon;
float saturnrheamoonDistance = 0.112;
float saturnrheamoonSize = 0.0035;
float saturnrheamoon;
float saturntitanmoonDistance = 0.0081;
float saturntitanmoonSize = 0.40;
float saturntitanmoon;
float saturntrlhysmoonDistance = 0.02;
float saturntrlhysmoonSize = 0.0836;
float saturntrlhysmoon;

float Uranus_distance = 19.6 ;
float UranusSize = 4.007 ;
float uranus;
float uranusSpin;
float uranustitaniamoonDistance = 0.003;
float uranustitaniamoonSize = 0.123;
float uranustitaniamoon;
float uranusoberonmoonDistance = 0.0039;
float uranusoberonmoonSize = 0.1195;
float uranusoberonmoon;
float uranusumbrielmoonDistance = 0.001;
float uranusumbrielmoonSize = 0.1088;
float uranusumbrielmoon;

float Neptune_distance = 30;
float NeptuneSize = 4.007 ;
float nebtune;
float nebtuneSpin;
float nebtunemoontritonDistance = 0.0024;
float nebtunemoontritonSize = 0.212;
float nebtunemoontriton;

float Pluto_distance = 36;
float PlutoSize = 3.883 ;
float pluto;
float plutoSpin;
float plutocmoonharonDistance = 0.0017;
float plutomooncharonSize = 0.094;
float plutomooncharon;


// Textures Dimensions
int sun_width = 512, sun_height = 256;

int space_width = 8192, space_height = 4096;

int earth_width = 1024, earth_height = 512;
int moon_width = 1024, moon_height = 512;

int mercury_width = 1024, mercury_height = 512;
int venus_width = 2048, venus_height = 1024;

int mars_width = 1024, mars_height = 512;
int diemos_height =320, diemos_width =  620;
int phobos_height =320, phobos_width= 620;

int jupiter_width = 1024, jupiter_height = 512;
int io_width = 4096, io_height = 2048;
int europa_width = 4096, europa_height = 2048;
int callisto_width = 1800, callisto_height = 900;
int ganymede_width = 2048, ganymede_height = 1024;

int saturn_width = 1800, saturn_height = 900;
int saturn_ring_width = 64, saturn_ring_height = 1024 ;
int saturn_ring_width2 = 64, saturn_ring_height2 = 1024 ;
int dione_width = 690, dione_height = 480;
int enceladus_width = 1024, enceladus_height = 512;
int lapetus_width = 1800, lapetus_height = 900;
int mimas_width = 512, mimas_height = 256;
int rhea_width = 8192, rhea_height = 4096;
int titan_width = 4096, titan_height = 2048;
int trlhys_width = 11520, trlhys_height = 5760;

int uranus_width = 2048, uranus_height = 1024;
int titania_width = 2048, titania_height = 1024;
int oberon_width = 2048, oberon_height = 1024;
int umbriel_width = 2048, umbriel_height = 1024;

int nebtune_width = 1024, nebtune_height = 512;
int triton_width = 4096, triton_height = 2024;

int pluto_width = 4096, pluto_height = 2048;
int charon_width = 1800, charon_height = 900;

// Texture Char
unsigned char *Sun_Texture;

unsigned char *Space_Texture;

unsigned char *Venus_Texture;

unsigned char *Mercury_Texture;

unsigned char *Earth_Texture;
unsigned char *Moon_Texture;

unsigned char *Jupiter_Texture;
unsigned char *Io_Texture;
unsigned char *Europa_Texture;
unsigned char *Callisto_Texture;
unsigned char *Ganymede_Texture;

unsigned char *Mars_Texture;
unsigned char *Diemos_Texture;
unsigned char *Phobos_Texture;

unsigned char *Saturn_Texture;
unsigned char *Saturn_Ring_Texture;
unsigned char *Saturn_Ring_Texture2;
unsigned char *Saturn_Dione_Texture;
unsigned char *Saturn_Enceladus_Texture;
unsigned char *Saturn_Lapetus_Texture;
unsigned char *Saturn_Mimas_Texture;
unsigned char *Saturn_Rhea_Texture;
unsigned char *Saturn_Titan_Texture;
unsigned char *Saturn_Trlhys_Texture;

unsigned char *Uranus_Texture;
unsigned char *Titania_Texture;
unsigned char *Oberon_Texture;
unsigned char *Umbriel_Texture;

unsigned char *Nebtune_Texture;
unsigned char *Triton_Texture;

unsigned char *Pluto_Texture;
unsigned char *Charon_Texture;


static GLuint Sun_Tex, Space_Tex, Venus_Tex, Mercury_Tex;

static GLuint Mars_Tex, Diemos_Tex, Phobos_Tex ;

static GLuint Earth_Tex, Moon_Tex;

static GLuint Jupiter_Tex, Io_Tex, Europa_Tex, Callisto_Tex, Ganymede_Tex;

static GLuint Saturn_Ring_Tex ,Saturn_Ring_Tex2, Saturn_Tex, Dione_Tex, Enceladus_Tex, Lapetus_Tex, Mimas_Tex, Rhea_Tex, Titan_Tex, Trlhys_Tex;

static GLuint Uranus_Tex, Titania_Tex, Oberon_Tex, Umbriel_Tex;

static GLuint Nebtune_Tex, Triton_Tex;

static GLuint Pluto_Tex, Charon_Tex;

int	ReadInt( FILE * );
short	ReadShort( FILE * );


struct bmfh
{
	short bfType;
	int bfSize;
	short bfReserved1;
	short bfReserved2;
	int bfOffBits;
} FileHeader;

struct bmih
{
	int biSize;
	int biWidth;
	int biHeight;
	short biPlanes;
	short biBitCount;
	int biCompression;
	int biSizeImage;
	int biXPelsPerMeter;
	int biYPelsPerMeter;
	int biClrUsed;
	int biClrImportant;
} InfoHeader;

const int birgb = { 0 };



/**
** read a BMP file into a Texture:
**/

unsigned char *
	BmpToTexture( char *filename, int *width, int *height )
{

	int s, t, e;		// counters
	int numextra;		// # extra bytes each line in the file is padded with
	FILE *fp;
	unsigned char *texture;
	int nums, numt;
	unsigned char *tp;


	fp = fopen( filename, "rb" );
	if( fp == NULL )
	{
		fprintf( stderr, "Cannot open Bmp file '%s'\n", filename );
		return NULL;
	}

	FileHeader.bfType = ReadShort( fp );


	// if bfType is not 0x4d42, the file is not a bmp:

	if( FileHeader.bfType != 0x4d42 )
	{
		fprintf( stderr, "Wrong type of file: 0x%0x\n", FileHeader.bfType );
		fclose( fp );
		return NULL;
	}


	FileHeader.bfSize = ReadInt( fp );
	FileHeader.bfReserved1 = ReadShort( fp );
	FileHeader.bfReserved2 = ReadShort( fp );
	FileHeader.bfOffBits = ReadInt( fp );


	InfoHeader.biSize = ReadInt( fp );
	InfoHeader.biWidth = ReadInt( fp );
	InfoHeader.biHeight = ReadInt( fp );

	nums = InfoHeader.biWidth;
	numt = InfoHeader.biHeight;

	InfoHeader.biPlanes = ReadShort( fp );
	InfoHeader.biBitCount = ReadShort( fp );
	InfoHeader.biCompression = ReadInt( fp );
	InfoHeader.biSizeImage = ReadInt( fp );
	InfoHeader.biXPelsPerMeter = ReadInt( fp );
	InfoHeader.biYPelsPerMeter = ReadInt( fp );
	InfoHeader.biClrUsed = ReadInt( fp );
	InfoHeader.biClrImportant = ReadInt( fp );


	// fprintf( stderr, "Image size found: %d x %d\n", ImageWidth, ImageHeight );


	texture = new unsigned char[ 3 * nums * numt ];
	if( texture == NULL )
	{
		fprintf( stderr, "Cannot allocate the texture array!\b" );
		return NULL;
	}


	// extra padding bytes:

	numextra =  4*(( (3*InfoHeader.biWidth)+3)/4) - 3*InfoHeader.biWidth;


	// we do not support compression:

	if( InfoHeader.biCompression != birgb )
	{
		fprintf( stderr, "Wrong type of image compression: %d\n", InfoHeader.biCompression );
		fclose( fp );
		return NULL;
	}



	rewind( fp );
	fseek( fp, 14+40, SEEK_SET );

	if( InfoHeader.biBitCount == 24 )
	{
		for( t = 0, tp = texture; t < numt; t++ )
		{
			for( s = 0; s < nums; s++, tp += 3 )
			{
				*(tp+2) = fgetc( fp );		// b
				*(tp+1) = fgetc( fp );		// g
				*(tp+0) = fgetc( fp );		// r
			}

			for( e = 0; e < numextra; e++ )
			{
				fgetc( fp );
			}
		}
	}

	fclose( fp );

	*width = nums;
	*height = numt;
	return texture;
}



int
	ReadInt( FILE *fp )
{
	unsigned char b3, b2, b1, b0;
	b0 = fgetc( fp );
	b1 = fgetc( fp );
	b2 = fgetc( fp );
	b3 = fgetc( fp );
	return ( b3 << 24 )  |  ( b2 << 16 )  |  ( b1 << 8 )  |  b0;
}


short
	ReadShort( FILE *fp )
{
	unsigned char b1, b0;
	b0 = fgetc( fp );
	b1 = fgetc( fp );
	return ( b1 << 8 )  |  b0;
}


struct point {
	float x, y, z;		// coordinates
	float nx, ny, nz;	// surface normal
	float s, t;		// texture coords
};

int		NumLngs, NumLats;
struct point *	Pts;

struct point *
	PtsPointer( int lat, int lng )
{
	if( lat < 0 )	lat += (NumLats-1);
	if( lng < 0 )	lng += (NumLngs-1);
	if( lat > NumLats-1 )	lat -= (NumLats-1);
	if( lng > NumLngs-1 )	lng -= (NumLngs-1);
	return &Pts[ NumLngs*lat + lng ];
}



void
	DrawPoint( struct point *p )
{
	glNormal3f( p->nx, p->ny, p->nz );
	glTexCoord2f( p->s, p->t );
	glVertex3f( p->x, p->y, p->z );
}

void
	MjbSphere( float radius, int slices, int stacks )
{
	struct point top, bot;		// top, bottom points
	struct point *p;

	// set the globals:

	NumLngs = slices;
	NumLats = stacks;

	if( NumLngs < 3 )
		NumLngs = 3;

	if( NumLats < 3 )
		NumLats = 3;

	// allocate the point data structure:

	Pts = new struct point[ NumLngs * NumLats ];


	// fill the Pts structure:

	for( int ilat = 0; ilat < NumLats; ilat++ )
	{
		float lat = -M_PI/2.  +  M_PI * (float)ilat / (float)(NumLats-1);
		float xz = cos( lat );
		float y = sin( lat );
		for( int ilng = 0; ilng < NumLngs; ilng++ )
		{
			float lng = -M_PI  +  2. * M_PI * (float)ilng / (float)(NumLngs-1);
			float x =  xz * cos( lng ); //Center
			float z = -xz * sin( lng ); // Center
			p = PtsPointer( ilat, ilng ); // set Limits
			p->x  = radius * x; // To give x distance in the sphere
			p->y  = radius * y; // To give y distance in the sphere
			p->z  = radius * z; // To give z distance in the sphere
			p->nx = x;   // Give X normal
			p->ny = y;   // Give y normal
			p->nz = z;   // Give z normal


			p->s =  (( lng + M_PI    ) / ( 2.*M_PI ));
			p->t =  (( lat + M_PI/2. ) / M_PI);
		}
	}

	top.x =  0.;		top.y  = radius;	top.z = 0.;
	top.nx = 0.;		top.ny = 1.;		top.nz = 0.;
	top.s  = 0.;		top.t  = 1.;

	bot.x =  0.;		bot.y  = -radius;	bot.z = 0.;
	bot.nx = 0.;		bot.ny = -1.;		bot.nz = 0.;
	bot.s  = 0.;		bot.t  =  0.;


	// connect the north pole to the latitude NumLats-2:

	glBegin( GL_QUADS );
	for( int ilng = 0; ilng < NumLngs-1; ilng++ )
	{
		p = PtsPointer( NumLats-1, ilng );
		DrawPoint( p );

		p = PtsPointer( NumLats-2, ilng );
		DrawPoint( p );

		p = PtsPointer( NumLats-2, ilng+1 );
		DrawPoint( p );

		p = PtsPointer( NumLats-1, ilng+1 );
		DrawPoint( p );
	}
	glEnd( );

	// connect the south pole to the latitude 1:

	glBegin( GL_QUADS );
	for( int ilng = 0; ilng < NumLngs-1; ilng++ )
	{
		p = PtsPointer( 0, ilng );
		DrawPoint( p );

		p = PtsPointer( 0, ilng+1 );
		DrawPoint( p );

		p = PtsPointer( 1, ilng+1 );
		DrawPoint( p );

		p = PtsPointer( 1, ilng );
		DrawPoint( p );
	}
	glEnd( );


	// connect the other 4-sided polygons:

	glBegin( GL_QUADS );
	for( int ilat = 2; ilat < NumLats-1; ilat++ )
	{
		for( int ilng = 0; ilng < NumLngs-1; ilng++ )
		{
			p = PtsPointer( ilat-1, ilng );
			DrawPoint( p );

			p = PtsPointer( ilat-1, ilng+1 );
			DrawPoint( p );

			p = PtsPointer( ilat, ilng+1 );
			DrawPoint( p );

			p = PtsPointer( ilat, ilng );
			DrawPoint( p );
		}
	}
	glEnd( );

	delete [ ] Pts;
	Pts = NULL;
}

void
	SetPointLight( int ilight, float x, float y, float z, float r, float g, float b )
{
	//glMaterialfv( GL_FRONT, GL_EMISSION, Array3( 0., 0., 0. ) );
	glLightfv( ilight, GL_POSITION, Array3( x, y, z ) );
	glLightfv( ilight, GL_AMBIENT, Array3( 0., 0., 0. ) );
	glLightfv( ilight, GL_DIFFUSE, Array3( r, g, b ) );
	glLightfv( ilight, GL_SPECULAR, Array3( r, g, b ) );
	glLightf ( ilight, GL_CONSTANT_ATTENUATION, 1. );
	glLightf ( ilight, GL_LINEAR_ATTENUATION, 0. );
	glLightf ( ilight, GL_QUADRATIC_ATTENUATION, 0. );
	//glEnable( ilight );
}


void
	SetMaterial( float r, float g, float b, float shininess )
{
	glMaterialfv( GL_BACK, GL_EMISSION, Array3( 0., 0., 0. ) );
	glMaterialfv( GL_BACK, GL_AMBIENT, MulArray3( .4f, White ) );
	glMaterialfv( GL_BACK, GL_DIFFUSE, MulArray3( 1., White ) );
	glMaterialfv( GL_BACK, GL_SPECULAR, Array3( 0., 0., 0. ) );
	glMaterialf ( GL_BACK, GL_SHININESS, 2.f );
	glMaterialfv( GL_FRONT, GL_EMISSION, Array3( 0., 0., 0. ) );
	glMaterialfv( GL_FRONT, GL_AMBIENT, Array3( r, g, b ) );
	glMaterialfv( GL_FRONT, GL_DIFFUSE, Array3( r, g, b ) );
	glMaterialfv( GL_FRONT, GL_SPECULAR, MulArray3( .8f, White ) );
	glMaterialf ( GL_FRONT, GL_SHININESS, shininess );
}

void circle(float x, float y, float z, float rayon)
{
	glBegin(GL_LINE_LOOP);
	for(float k=0;k<(M_PI*2);k+=M_PI/180){
		x=sin(k)*rayon;
		z=cos(k)*rayon;
		glColor3f( 1., 1., 1. );
		glVertex3f(x,y,z);
	}
	glEnd();
}



void DrawCircle(float cx, float cy, float r, int num_segments) 
{ 
	float theta = 2 * 3.1415926 / float(num_segments); 
	float c = cosf(theta);//precalculate the sine and cosine
	float s = sinf(theta);
	float t;

	float x = r;//we start at angle = 0 
	float y = 0; 

	glBegin(GL_LINE_LOOP); 
	for(int ii = 0; ii < num_segments; ii++) 
	{ 
		glVertex2f(x + cx, y + cy);//output vertex 

		//apply the rotation matrix
		t = x;
		x = c * x - s * y;
		y = s * t + c * y;
	} 
	glEnd(); 
}


void drawDisk(double radiusX, double eccentricity, int inRadius, int sideSmooth)
{   
	double radiusY = 0.01;
	double pi = M_PI;
	double texY1 = 1.0f/double(inRadius);
	double lat1 = 1.0f*pi/double(inRadius)-0.5f*pi;
	double r1 = cosf(lat1);
	double z1 = sinf(lat1);
	for(int i = 2; i < inRadius; ++i)
	{
		double r0 = r1;
		double z0 = z1;
		double texY0 = texY1;
		texY1 = double(i)/double(inRadius);
		lat1 = double(i)*pi/double(inRadius)-0.5f*pi;
		r1 = cosf(lat1);
		z1 = sinf(lat1);

		glBegin(GL_QUAD_STRIP);
		for(int j=0; j <= sideSmooth; ++j)
		{
			double texX = double(j)/double(sideSmooth);
			double lng = double(j)*(2.0f*pi)/double(sideSmooth);
			double x1 = cosf(lng) * r1;
			double y1 = sinf(lng) * r1;
			glNormal3f(x1,y1,z1);
			glTexCoord2f(texX,texY1);
			glVertex3f(x1*radiusX,y1*radiusX*eccentricity,z1*radiusX*radiusY);
			double x0 = cosf(lng)*r0;
			double y0 = sinf(lng)*r0;
			glNormal3f(x0,y0,z0);
			glTexCoord2f(texX,texY0);
			glVertex3f(x0*radiusX,y0*radiusX*eccentricity,z0*radiusX*radiusY);
		}
		glEnd();
	}

}

//
// main program:
//


// ALL Planets Functions Are here

// Space Object
//void SpaceObject(float SpaceSize 4500)
void SpaceObject(float SpaceSize)
{
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();

	glBindTexture( GL_TEXTURE_2D, Space_Tex );
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
	glTranslatef(0.0,0.0,0.0);
	glColor3f( 1., 1., 1. );
	MjbSphere(SpaceSize,50,50);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
}

// Sun Object
//void SunObject(float SunSize 20.0)
void SunObject(float SunSize)
{
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
	glBindTexture( GL_TEXTURE_2D, Sun_Tex );
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
	glTranslatef(0.0,0.0,0.0);
	glColor3f( 1., 1., 1. );
	MjbSphere(SunSize,50,50);
	glDisable(GL_LIGHTING);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);

}


// Jupiter Object
//void JubiterObject(float JubiterSize 10, float JubiterLocation 140, float IoSize 0.6, float IoLocation12, float EuropaSize .5, float EuropaLocation 14, float GanymedeSize .8, float GanymedeLocation 16, float CallistoSize .8, float CallistoLocation 20)
void JupiterObject(float JubiterSize, float JubiterLocation, float IoSize, float IoLocation, float EuropaSize, float EuropaLocation, float GanymedeSize, float GanymedeLocation, float CallistoSize, float CallistoLocation)
{
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();

	glBindTexture( GL_TEXTURE_2D, Jupiter_Tex);
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glRotatef( jupiter, 0.0, 1.0, 0.0 );
	glTranslatef( JubiterLocation, 0.0, 0.0 );
	glDisable( GL_LIGHTING );
	glColor3f( 1., 0.86, 0.80 );
	DoRasterString( 0., 20., 0., "Jupiter" );
	glEnable( GL_LIGHTING );
	glPushMatrix();
	// Jupiter Spinning
	glRotatef( jupiterSpin, 0., 1.0, 0.0 );
	MjbSphere(JubiterSize,50,50);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
	glBindTexture( GL_TEXTURE_2D, Io_Tex);
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glDisable(GL_LIGHTING);
	if (LinesEnabled)
	{
		glPushMatrix();
		gluLookAt( 0.0000001, 0., 0.,     0., 0., 0.,     0., 0., .000000001 );
		DrawCircle(0.0, 0.0, IoLocation, 1000);
		glPopMatrix();
	}
	glEnable( GL_LIGHTING );
	glColor3f(1.,1.,1.);
	glRotatef( jupiteriomoon, 0.0, 1.0, 0.0 );
	glTranslatef( IoLocation, 0.0, 0.0 );
	MjbSphere(IoSize,50,50);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glBindTexture( GL_TEXTURE_2D, Europa_Tex);
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glDisable(GL_LIGHTING);
	if (LinesEnabled)
	{
		glPushMatrix();
		gluLookAt( 0.0000001, 0., 0.,     0., 0., 0.,     0., 0., .000000001 );
		DrawCircle(0.0, 0.0, EuropaLocation, 1000);
		glPopMatrix();
	}
	glEnable( GL_LIGHTING );
	glColor3f(1.,1.,1.);
	glRotatef( jupitereuropamoon, 0.0, 1.0, 0.0 );
	glTranslatef( EuropaLocation, 0.0, 0.0 );
	MjbSphere(EuropaSize,50,50);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glBindTexture( GL_TEXTURE_2D, Ganymede_Tex);
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glDisable(GL_LIGHTING);
	if (LinesEnabled)
	{
		glPushMatrix();
		gluLookAt( 0.0000001, 0., 0.,     0., 0., 0.,     0., 0., .000000001 );
		DrawCircle(0.0, 0.0, GanymedeLocation, 1000);
		glPopMatrix();
	}
	glEnable( GL_LIGHTING );
	glColor3f(1.,1.,1.);
	glRotatef( jupitercallistomoon, 0.0, 1.0, 0.0 );
	glTranslatef( GanymedeLocation, 0.0, 0.0 );
	MjbSphere(GanymedeSize,50,50);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glBindTexture( GL_TEXTURE_2D, Callisto_Tex);
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glDisable(GL_LIGHTING);
	if (LinesEnabled)
	{
		glPushMatrix();
		gluLookAt( 0.0000001, 0., 0.,     0., 0., 0.,     0., 0., .000000001 );
		DrawCircle(0.0, 0.0, CallistoLocation, 1000);
		glPopMatrix();
	}
	glEnable( GL_LIGHTING );
	glColor3f(1.,1.,1.);
	glRotatef( jupiterganymedemoon, 0.0, 1.0, 0.0 );
	glTranslatef( CallistoLocation, 0.0, 0.0 );
	MjbSphere(CallistoSize,50,50);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();	
	glDisable(GL_TEXTURE_2D);
}	

// Mercury Object
//void MercuryObject(float MercurySize 1.0, float MercuryLocation 40)
void MercuryObject(float MercurySize, float MercuryLocation)
{	glEnable(GL_TEXTURE_2D);
glPushMatrix();
glBindTexture( GL_TEXTURE_2D, Mercury_Tex);
glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
glRotatef( mercury, 0.0, 1.0, 0.0 );
glTranslatef( MercuryLocation, 0.0, 0.0 );
glDisable( GL_LIGHTING );
glColor3f( 0.58, 0.29, 0.04 );
DoRasterString( 0., 5., 0., "Mercury" );
glEnable( GL_LIGHTING );
glPushMatrix();
// Venus Spinning
glRotatef( -mercurySpin, 0., 1.0, 0.0 );
MjbSphere(MercurySize,50,50);
glPopMatrix();
// restore old position
glPopMatrix();
glDisable(GL_TEXTURE_2D);
}


// Venus Object
//void VenusObject(float VenusSize 1.0, float VenusLocation 40)
void VenusObject(float VenusSize, float VenusLocation)
{	
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();

	glBindTexture( GL_TEXTURE_2D, Venus_Tex);
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glRotatef( venus, 0.0, 1.0, 0.0 );
	glTranslatef( VenusLocation, 0.0, 0.0 );
	glDisable( GL_LIGHTING );
	glColor3f( 0.58, 0.29, 0.04 );
	DoRasterString( 0., 5., 0., "Venus" );
	glEnable( GL_LIGHTING );
	glPushMatrix();
	// Venus Spinning
	glRotatef( venusSpin, 0., 1.0, 0.0 );
	MjbSphere(VenusSize,50,50);
	glPopMatrix();
	// restore old position
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
}

// Earth Object
//void EarthObject(float EarthSize 5, float EarthLocation 60.0, float MoonSize .5, float Moon_Location 7.0)

void EarthObject(float EarthSize, float EarthLocation, float MoonSize, float Moon_Location)
{
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();	
	glBindTexture( GL_TEXTURE_2D, Earth_Tex);
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	// Earth Roatation
	glRotatef( earth, 0.0, 1.0, 0.0 );
	glTranslatef( EarthLocation, 0.0, 0.0 );
	glDisable( GL_LIGHTING );
	glColor3f( 0.0, 0.58, 0.93 );
	DoRasterString( 0., 5., 0., "    Earth" );
	glEnable( GL_LIGHTING );
	glPushMatrix();
	// Earth Spinning
	glRotatef( earthSpin, 0., 1.0, 0.0 );
	MjbSphere(EarthSize,100,100);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();

	glBindTexture( GL_TEXTURE_2D, Moon_Tex);
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glDisable(GL_LIGHTING);
	if (LinesEnabled)
	{
		glPushMatrix();
		gluLookAt( 0.0000001, 0., 0.,     0., 0., 0.,     0., 0., .000000001 );
		DrawCircle(0.0, 0.0, Moon_Location, 1000);
		glPopMatrix();
	}
	glEnable( GL_LIGHTING );
	glColor3f(1.,1.,1.);
	glRotatef( earthMoon, 0.0, 1.0, 0.0 );
	glTranslatef( Moon_Location, 0.0, 0.0 );
	MjbSphere(MoonSize,50,50);

	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	//glDisable(GL_TEXTURE_2D);
}

//Mars Object
//void MarsObject(float MarsSize 5.0, float MarsLocation 40, float PhobosSize .5, float PhobosLocation 6.0, float DiemosSize .4 , float DiemosLocaion 8.0)
void MarsObject(float MarsSize, float MarsLocation, float PhobosSize, float PhobosLocation , float DiemosSize , float DiemosLocaion)
{
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();	

	glBindTexture( GL_TEXTURE_2D, Mars_Tex);
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glRotatef( mars, 0.0, 1.0, 0.0 );
	glTranslatef( MarsLocation, 0.0, 0.0 );
	glDisable( GL_LIGHTING );
	glColor3f( 0.45, 0.22, 0.22 );
	DoRasterString( 0., 8., 0., "  Mars" );
	glEnable(GL_LIGHTING);
	// glPopMatrix();
	glPushMatrix();
	// Mars Spinning
	glRotatef( marsSpin, 0., 1.0, 0.0 );
	MjbSphere(MarsSize,100,100);
	glPopMatrix();
	//MjbSphere( 3.0,50,50 );
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();

	glBindTexture( GL_TEXTURE_2D, Phobos_Tex);
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glDisable(GL_LIGHTING);
	if (LinesEnabled)
	{
		glPushMatrix();
		gluLookAt( 0.0000001, 0., 0.,     0., 0., 0.,     0., 0., .000000001 );
		DrawCircle(0.0, 0.0, PhobosLocation, 1000);
		glPopMatrix();
	}
	glEnable( GL_LIGHTING );
	glColor3f(1.,1.,1.);
	glRotatef( marsphobosmoon, 0.0, 1.0, 0.0 );
	glTranslatef( PhobosLocation, 0.0, 0.0 );
	MjbSphere(PhobosSize,50,50);

	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();

	glBindTexture( GL_TEXTURE_2D, Diemos_Tex);
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glDisable(GL_LIGHTING);
	if (LinesEnabled)
	{
		glPushMatrix();
		gluLookAt( 0.0000001, 0., 0.,     0., 0., 0.,     0., 0., .000000001 );
		DrawCircle(0.0, 0.0, DiemosLocaion, 1000);
		glPopMatrix();
	}
	glEnable( GL_LIGHTING );
	glColor3f(1.,1.,1.);
	glRotatef( marsdiemosmoon, 0.0, 1.0, 0.0 );
	glTranslatef( DiemosLocaion, 0.0, 0.0 );
	MjbSphere(DiemosSize,50,50);

	glPopMatrix();
	glDisable(GL_TEXTURE_2D);

	glPopMatrix();
	glDisable(GL_TEXTURE_2D);

}


//Saturn Object
//void SaturnObejct(float SaturnSize 7.0, float SaturnLocation 220, float MimasSize .4, float MimasLocation 7.0, float EnceladusSize 0.5, float EnceladusLocation 8.0, float TrlhysSize 0.55, float TrlhysLocation 9.0, float DioneSize 0.65, float DioneLocation 11.0, float RheaSize 0.75, float RheaLocation 12.5, float TitanSize 1.2, float TitanLocation 16.0, float LapetusSize 0.75, float LapetusLocation 20.0)
void SaturnObejct(float SaturnSize, float SaturnLocation, float MimasSize, float MimasLocation, float EnceladusSize, float EnceladusLocation, float TrlhysSize, float TrlhysLocation, float DioneSize, float DioneLocation, float RheaSize, float RheaLocation, float TitanSize, float TitanLocation, float LapetusSize, float LapetusLocation)
{
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();

	glBindTexture( GL_TEXTURE_2D, Saturn_Tex);
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glRotatef( saturn, 0.0, 1.0, 0.0 );
	glTranslatef( SaturnLocation, 0.0, 0.0 );
	glDisable( GL_LIGHTING );
	glColor3f( 0.58, 0.29, 0.04 );
	DoRasterString( 0., 5., 0., "     Saturn" );
	glEnable( GL_LIGHTING );
	glPushMatrix();
	glRotatef( saturnSpin, 0., 1.0, 0.0 );
	MjbSphere(SaturnSize,50,50);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);  /// The problem might Be Here
	glEnable(GL_TEXTURE_2D);
	//Mimas_Tex
	glPushMatrix();

	glBindTexture( GL_TEXTURE_2D, Mimas_Tex);
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glDisable(GL_LIGHTING);
	if (LinesEnabled)
	{
		glPushMatrix();
		gluLookAt( 0.0000001, 0., 0.,     0., 0., 0.,     0., 0., .000000001 );
		DrawCircle(0.0, 0.0, MimasLocation, 1000);
		glPopMatrix();
	}
	glEnable( GL_LIGHTING );
	glColor3f(1.,1.,1.);
	glRotatef( saturnmimasmooon, 0.0, 1.0, 0.0 );
	glTranslatef( MimasLocation, 0.0, 0.0 );
	MjbSphere(MimasSize,50,50);

	glPopMatrix();	
	glDisable(GL_TEXTURE_2D);

	//Enceladus_moon
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();

	glBindTexture( GL_TEXTURE_2D, Enceladus_Tex);
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glDisable(GL_LIGHTING);
	if (LinesEnabled)
	{
		glPushMatrix();
		gluLookAt( 0.0000001, 0., 0.,     0., 0., 0.,     0., 0., .000000001 );
		DrawCircle(0.0, 0.0, EnceladusLocation, 1000);
		glPopMatrix();
	}
	glEnable( GL_LIGHTING );
	glColor3f(1.,1.,1.);
	glRotatef( saturnenceladusmoon, 0.0, 1.0, 0.0 );
	glTranslatef( EnceladusLocation, 0.0, 0.0 );
	MjbSphere(EnceladusSize,50,50);

	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	//Tetlhys_Tex
	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glBindTexture( GL_TEXTURE_2D, Trlhys_Tex);
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glDisable(GL_LIGHTING);
	if (LinesEnabled)
	{
		glPushMatrix();
		gluLookAt( 0.0000001, 0., 0.,     0., 0., 0.,     0., 0., .000000001 );
		DrawCircle(0.0, 0.0, TrlhysLocation, 1000);
		glPopMatrix();
	}
	glEnable( GL_LIGHTING );
	glColor3f(1.,1.,1.);
	glRotatef( saturntrlhysmoon, 0.0, 1.0, 0.0 );
	glTranslatef( TrlhysLocation, 0.0, 0.0 );
	MjbSphere(TrlhysSize,50,50);

	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	//Dione_moon
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();

	glBindTexture( GL_TEXTURE_2D, Dione_Tex);
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glDisable(GL_LIGHTING);
	if (LinesEnabled)
	{
		glPushMatrix();
		gluLookAt( 0.0000001, 0., 0.,     0., 0., 0.,     0., 0., .000000001 );
		DrawCircle(0.0, 0.0, DioneLocation, 1000);
		glPopMatrix();
	}
	glEnable( GL_LIGHTING );
	glColor3f(1.,1.,1.);
	glRotatef( saturndionemoon, 0.0, 1.0, 0.0 );
	glTranslatef( DioneLocation, 0.0, 0.0 );
	MjbSphere(DioneSize,50,50);

	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	//Rhea_Tex
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();

	glBindTexture( GL_TEXTURE_2D, Rhea_Tex);
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glDisable(GL_LIGHTING);
	if (LinesEnabled)
	{
		glPushMatrix();
		gluLookAt( 0.0000001, 0., 0.,     0., 0., 0.,     0., 0., .000000001 );
		DrawCircle(0.0, 0.0, RheaLocation, 1000);
		glPopMatrix();
	}
	glEnable( GL_LIGHTING );
	glColor3f(1.,1.,1.);
	glRotatef( saturnrheamoon, 0.0, 1.0, 0.0 );
	glTranslatef( RheaLocation, 0.0, 0.0 );
	MjbSphere(RheaSize,50,50);

	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	//Titan_Tex
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();

	glBindTexture( GL_TEXTURE_2D, Titan_Tex);
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glDisable(GL_LIGHTING);
	if (LinesEnabled)
	{
		glPushMatrix();
		gluLookAt( 0.0000001, 0., 0.,     0., 0., 0.,     0., 0., .000000001 );
		DrawCircle(0.0, 0.0, TitanLocation, 1000);
		glPopMatrix();
	}
	glEnable( GL_LIGHTING );
	glColor3f(1.,1.,1.);
	glRotatef( saturntitanmoon, 0.0, 1.0, 0.0 );
	glTranslatef( TitanLocation, 0.0, 0.0 );
	MjbSphere(TitanSize,50,50);

	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	//Lapetus_Tex
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();

	glBindTexture( GL_TEXTURE_2D, Lapetus_Tex);
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	//gluLookAt( 0.0, 0., 0.,     0.15, .0, -0.15,     0., 0., .1 );
	glDisable(GL_LIGHTING);
	if (LinesEnabled)
	{
		glPushMatrix();
		gluLookAt( 0.0000001, 0., 0.,     0, .0, 0,     0., 0., .000000001 );
		DrawCircle(0.0, 0.0, LapetusLocation, 1000);
		glPopMatrix();
	}
	glEnable( GL_LIGHTING );
	glColor3f(1.,1.,1.);
	//gluLookAt( -0.015, .0, 0.015,     0., .0, 0.,     0., 0.00000001, .0 );
	glRotatef( saturnlapetusmoon, 0.0, 1.0, 0.0 );
	glTranslatef( LapetusLocation, 0.0, 0.0 );
	MjbSphere(LapetusSize,50,50);

	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
}

// Saturn Ring
//void SaturnRing(float RingSize 15, float RingLocation 220 , float RingWidth 1.2, float RingInnerRadious 6, float SideSmooth 100)
void SaturnRing(float RingOuterRadious, float RingLocation, float Eccentricity, float RingInnerRadious, float SideSmooth)
{

	glEnable(GL_TEXTURE_2D);
	glPushMatrix();

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable (GL_BLEND);
	if (SaturnRingTrans == 0)
	{
		glBlendFunc (GL_ONE, GL_ZERO    );
	}
	else if (SaturnRingTrans == 1)
	{
		glBlendFunc (GL_ONE, GL_ONE   );
	}
	else if (SaturnRingTrans == 2)
	{
		glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_COLOR      );
	}	
	else if (SaturnRingTrans == 3)
	{
		glBlendFunc (GL_ONE, GL_DST_COLOR   );
	}	
	else if (SaturnRingTrans == 4)
	{
		glBlendFunc (GL_ONE, GL_ONE_MINUS_DST_COLOR    );
	}	
	else if (SaturnRingTrans == 5)
	{
		glBlendFunc (GL_ONE, GL_SRC_ALPHA    );
	}	


	glPushMatrix();
	glRotatef(0.0, 0.0, 0.0, 1.0);
	glRotatef( saturn, 0.0, 1.0, 0.0);
	glTranslatef(RingLocation, 0.0, 0.0 );
	glRotatef( -90, 1.0, 0.0, 0.0 );
	glBindTexture(GL_TEXTURE_2D, Saturn_Ring_Tex);
	//	glBindTexture(GL_TEXTURE_2D, Saturn_Ring_Tex2);
	//glScalef(1,1,1);
	// RingOuterRadious, the eccentricity, the RingInnerRadious, the SideSmooth
	drawDisk(RingOuterRadious, Eccentricity, RingInnerRadious, SideSmooth);	

	//drawDisk( radiusX,  eccentricity, inRadius,  sideSmooth)
	//MjbSphere(20.0,50,50);
	glPopMatrix();
	glDisable (GL_BLEND);

	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	//glPopMatrix();

}

// Uranus Object
//void UranusObject(float UranusSize 6.0, float UranusLocation 260, float UmbrielSize .5, float UmbrielLocation 10.0, float TitaniaSize .9, float TitaniaLocation 12.0, float OberonSize .8, float OberonLocation 14.0)

void UranusObject(float UranusSize, float UranusLocation, float UmbrielSize, float UmbrielLocation, float TitaniaSize, float TitaniaLocation, float OberonSize, float OberonLocation)
{
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();

	glBindTexture( GL_TEXTURE_2D, Uranus_Tex);
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glRotatef( uranus, 0.0, 1.0, 0.0 );
	glTranslatef( UranusLocation, 0.0, 0.0 );
	glDisable( GL_LIGHTING );
	glColor3f( 0.58, 0.29, 0.04 );
	DoRasterString( 0., 5., 0., "     Uranus" );
	glEnable( GL_LIGHTING );
	glPushMatrix();
	// Venus Spinning
	glRotatef( uranusSpin, 0., 1.0, 0.0 );
	MjbSphere(UranusSize,50,50);

	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();

	glBindTexture( GL_TEXTURE_2D, Umbriel_Tex);
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glDisable(GL_LIGHTING);
	if (LinesEnabled)
	{
		glPushMatrix();
		gluLookAt( 0.0000001, 0., 0.,     0., 0., 0.,     0., 0., .000000001 );
		DrawCircle(0.0, 0.0, UmbrielLocation, 1000);
		glPopMatrix();
	}
	glEnable( GL_LIGHTING );
	glColor3f(1.,1.,1.);
	glRotatef( uranusumbrielmoon, 0.0, 1.0, 0.0 );
	glTranslatef( UmbrielLocation, 0.0, 0.0 );
	MjbSphere(UmbrielSize,50,50);

	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();

	glBindTexture( GL_TEXTURE_2D, Titania_Tex);
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glDisable(GL_LIGHTING);
	if (LinesEnabled)
	{
		glPushMatrix();
		gluLookAt( 0.0000001, 0., 0.,     0., 0., 0.,     0., 0., .000000001 );
		DrawCircle(0.0, 0.0, TitaniaLocation, 1000);
		glPopMatrix();
	}
	glEnable( GL_LIGHTING );
	glColor3f(1.,1.,1.);
	glRotatef( uranustitaniamoon, 0.0, 1.0, 0.0 );
	glTranslatef( TitaniaLocation, 0.0, 0.0 );
	MjbSphere(TitaniaSize,50,50);

	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();

	glBindTexture( GL_TEXTURE_2D, Oberon_Tex);
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glDisable(GL_LIGHTING);
	if (LinesEnabled)
	{
		glPushMatrix();
		gluLookAt( 0.0000001, 0., 0.,     0., 0., 0.,     0., 0., .000000001 );
		DrawCircle(0.0, 0.0, OberonLocation, 1000);
		glPopMatrix();
	}
	glEnable( GL_LIGHTING );
	glColor3f(1.,1.,1.);
	glRotatef( uranusoberonmoon, 0.0, 1.0, 0.0 );
	glTranslatef( OberonLocation, 0.0, 0.0 );
	MjbSphere(OberonSize,50,50);

	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
}

// Nebtune Object
//void NebtuneObject(float NebtuneSize 5.0, float NebtuneLocation 300)
void NebtuneObject(float NebtuneSize, float NebtuneLocation, float TritonSize, float TritonLocation )
{	
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
	glBindTexture( GL_TEXTURE_2D, Nebtune_Tex);
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glRotatef( nebtune, 0.0, 1.0, 0.0 );
	glTranslatef( NebtuneLocation, 0.0, 0.0 );
	glDisable( GL_LIGHTING );
	glColor3f( 0.58, 0.29, 0.04 );
	DoRasterString( 0., 5., 0., "     Nebtune" );
	glEnable( GL_LIGHTING );
	glPushMatrix();
	// Venus Spinning
	glRotatef( nebtuneSpin, 0., 1.0, 0.0 );
	MjbSphere(NebtuneSize,50,50);
	glPopMatrix();
	glPushMatrix();

	glBindTexture( GL_TEXTURE_2D, Oberon_Tex);
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glDisable(GL_LIGHTING);
	if (LinesEnabled)
	{
		glPushMatrix();
		gluLookAt( 0.0000001, 0., 0.,     0., 0., 0.,     0., 0., .000000001 );
		DrawCircle(0.0, 0.0, TritonLocation, 1000);
		glPopMatrix();
	}
	glEnable( GL_LIGHTING );
	glColor3f(1.,1.,1.);
	glRotatef( nebtunemoontriton, 0.0, 1.0, 0.0 );
	glTranslatef( TritonLocation, 0.0, 0.0 );
	MjbSphere(TritonSize,50,50);

	glPopMatrix();
	glDisable(GL_TEXTURE_2D);

	// restore old position
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
}

//Pluto Object
//void PlutoObject(float PlutoSize 6.0, float PlutoLocation 340, float CharonSize 2.0, float CharonLocation 10)

void PlutoObject(float PlutoSize, float PlutoLocation, float CharonSize, float CharonLocation)
{
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();

	glBindTexture( GL_TEXTURE_2D, Pluto_Tex);
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glRotatef( pluto, 0.0, 1.0, 0.0 );
	glTranslatef( PlutoLocation, 0.0, 0.0 );
	glDisable( GL_LIGHTING );
	glColor3f( 0.58, 0.29, 0.04 );
	DoRasterString( 0., 5., 0., "     Pluto" );
	glEnable( GL_LIGHTING );
	glPushMatrix();
	glRotatef( plutoSpin, 0., 1.0, 0.0 );
	MjbSphere(PlutoSize,50,50);

	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();

	glBindTexture( GL_TEXTURE_2D, Charon_Tex);
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glDisable(GL_LIGHTING);
	if (LinesEnabled)
	{
		glPushMatrix();
		gluLookAt( 0.0000001, 0., 0.,     0., 0., 0.,     0., 0., .000000001 );
		DrawCircle(0.0, 0.0, CharonLocation, 1000);
		glPopMatrix();
	}
	glEnable( GL_LIGHTING );
	glColor3f(1.,1.,1.);
	glRotatef( plutomooncharon, 0.0, 1.0, 0.0 );
	glTranslatef( CharonLocation, 0.0, 0.0 );
	MjbSphere(CharonSize,50,50);

	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);



}


void PlanetsOrbits(float Mercury_distance1, float Jupiture_distance1, float Venus_distance1, float Earth_distance1, float Mars_distance1, float Saturn_distance1, float Uranus_distance1, float Neptune_distance1, float Pluto_dostance1)
{
	if (LinesEnabled)
	{	glPushMatrix();
	glColor3f( 1., 1., 1. );
	gluLookAt( 0.0000001, 0., 0.,     0., 0., 0.,     0., 0., .000000001 );
	DrawCircle(0.0, 0.0, Mercury_distance1, 1000);
	DrawCircle(0.0, 0.0, Jupiture_distance1, 1000);
	DrawCircle(0.0, 0.0, Venus_distance1, 1000);
	DrawCircle(0.0, 0.0, Earth_distance1, 1000);
	DrawCircle(0.0, 0.0, Mars_distance1, 1000);
	DrawCircle(0.0, 0.0, Saturn_distance1, 1000);
	DrawCircle(0.0, 0.0, Uranus_distance1, 1000);
	DrawCircle(0.0, 0.0, Neptune_distance1, 1000);
	DrawCircle(0.0, 0.0, Pluto_dostance1, 1000);
	glPopMatrix();
	}
}



void RemoveAllTexutreAtExit()
{
	glDeleteTextures( 1, &Sun_Tex );

	glDeleteTextures( 1, &Earth_Tex );
	glDeleteTextures( 1, &Moon_Tex );

	glDeleteTextures( 1, &Space_Tex );

	glDeleteTextures( 1, &Venus_Tex );

	glDeleteTextures( 1, &Mars_Tex );
	glDeleteTextures( 1, &Diemos_Tex );
	glDeleteTextures( 1, &Phobos_Tex );

	glDeleteTextures( 1, &Jupiter_Tex );
	glDeleteTextures( 1, &Io_Tex );
	glDeleteTextures( 1, &Europa_Tex );
	glDeleteTextures( 1, &Callisto_Tex );
	glDeleteTextures( 1, &Ganymede_Tex );

	glDeleteTextures( 1, &Saturn_Tex );
	glDeleteTextures( 1, &Dione_Tex );
	glDeleteTextures( 1, &Enceladus_Tex );
	glDeleteTextures( 1, &Lapetus_Tex );
	glDeleteTextures( 1, &Mimas_Tex );
	glDeleteTextures( 1, &Rhea_Tex );
	glDeleteTextures( 1, &Titan_Tex );
	glDeleteTextures( 1, &Trlhys_Tex );
	glDeleteTextures( 1, &Saturn_Ring_Tex );

	glDeleteTextures( 1, &Uranus_Tex );
	glDeleteTextures( 1, &Titania_Tex );
	glDeleteTextures( 1, &Oberon_Tex );
	glDeleteTextures( 1, &Umbriel_Tex );

	glDeleteTextures( 1, &Nebtune_Tex );

	glDeleteTextures( 1, &Pluto_Tex );
	glDeleteTextures( 1, &Charon_Tex );

	Sun_Texture = NULL; 

	Earth_Texture = NULL;
	Moon_Texture = NULL; 

	Space_Texture = NULL;

	// Venus Texture = null
	Venus_Texture = NULL; 

	// Mars Texture = null
	Mars_Texture = NULL;
	Diemos_Texture = NULL; 
	Phobos_Texture = NULL; 

	// Jupiter Texture = null
	Jupiter_Texture = NULL;
	Io_Texture = NULL;
	Europa_Texture = NULL;
	Callisto_Texture = NULL;
	Ganymede_Texture = NULL; 

	//Saturn Texture = null
	Saturn_Texture = NULL; 
	Saturn_Ring_Texture = NULL;
	Saturn_Dione_Texture = NULL; 
	Saturn_Enceladus_Texture = NULL;
	Saturn_Lapetus_Texture = NULL; 
	Saturn_Mimas_Texture = NULL;
	Saturn_Rhea_Texture = NULL; 
	Saturn_Titan_Texture = NULL;
	Saturn_Trlhys_Texture = NULL; 

	// Uranus Texture = null
	Uranus_Texture = NULL; 
	Titania_Texture = NULL; 
	Oberon_Texture = NULL;
	Umbriel_Texture = NULL;
	// Nebtune Texture = null
	Nebtune_Texture = NULL;

	// Pluto Texture = null
	Pluto_Texture = NULL;
	Charon_Texture = NULL;

	Sun_Texture = NULL;

	Earth_Texture = NULL;
	Moon_Texture = NULL;

	Space_Texture = NULL;

	// Venus Texture
	Venus_Texture = NULL;

	// Mars Texture
	Mars_Texture = NULL;
	Diemos_Texture = NULL;
	Phobos_Texture = NULL;

	// Jupiter Texture
	Jupiter_Texture = NULL;
	Io_Texture = NULL;
	Europa_Texture = NULL;
	Callisto_Texture = NULL;
	Ganymede_Texture = NULL;

	//Saturn Texture
	Saturn_Texture = NULL;
	Saturn_Ring_Texture = NULL;
	Saturn_Dione_Texture = NULL;
	Saturn_Enceladus_Texture = NULL;
	Saturn_Lapetus_Texture = NULL;
	Saturn_Mimas_Texture = NULL;
	Saturn_Rhea_Texture = NULL;
	Saturn_Titan_Texture = NULL;
	Saturn_Trlhys_Texture = NULL; 

	// Uranus Texture
	Uranus_Texture = NULL;
	Titania_Texture = NULL;
	Oberon_Texture = NULL;
	Umbriel_Texture = NULL;
	// Nebtune Texture
	Nebtune_Texture = NULL;

	// Pluto Texture
	Pluto_Texture = NULL;
	Charon_Texture = NULL;
}


int
	main( int argc, char *argv[ ] )
{
	// turn on the glut package:
	// (do this before checking argc and argv since it might
	// pull some command line arguments out)

	glutInit( &argc, argv );


	// setup all the graphics stuff:

	InitGraphics( );


	// create the display structures that will not change:

	InitLists( );


	// init all the global variables used by Display( ):
	// this will also post a redisplay
	// it is important to call this before InitGlui( )
	// so that the variables that glui will control are correct
	// when each glui widget is created

	Reset( );


	// setup all the user interface stuff:

	InitGlui( );


	// draw the scene once and wait for some interaction:
	// (this will never return)

	glutMainLoop( );

	//LinesEnabled = 0;
	// this is here to make the compiler happy:

	return 0;
}



//
// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display( ) from here -- let glutMainLoop( ) do it
//
//float Time;
void
	Animate( void )
{
	// put animation stuff in here -- change some global variables
	// for Display( ) to find:
	// force a call to Display( ) next time it is convenient:
	//	int ms = glutGet( GLUT_ELAPSED_TIME );


	// Control the solar system rotation

	//mercury plaent and moons
	mercury += 4.147 * TimeControl;

	if( mercury > 360.0 )
		mercury -= 360.0;

	mercurySpin += 0.0173 * TimeControl;
	if( mercurySpin > 360.0 )
		mercurySpin -= 360.0;

	//venus plaent and moons
	venus += 1.62 * TimeControl;

	if( venus > 360.0 )
		venus -= 360.0;

	// Clockwise Rotation
	venusSpin -= 2.5 * TimeControl;
	//venusSpin -= 0.0041 * TimeControl;
	if( venusSpin <= -360.0 )
		venusSpin = 0.0;

	//Earth plaent and moons
	earth += 1.0 * TimeControl;
	if( earth > 360.0 )
		earth -= 360.0;

	earthSpin += 1.0 * TimeControl;
	if( earthSpin > 360.0 )
		earthSpin -= 360.0;

	earthMoon += 13.36 * TimeControl;
	if( earthMoon > 360.0 )
		earthMoon -= 360.0;

	//Mars plaent and moons
	mars += .531 * TimeControl;
	if (mars > 360.0)
		mars -= 360.0;

	marsSpin += .98 * TimeControl;
	if( marsSpin > 360.0 )
		marsSpin -= 360.0;

	marsdiemosmoon += 289.2 * TimeControl;
	if( marsdiemosmoon > 360.0 )
		marsdiemosmoon -= 360.0;

	marsphobosmoon += 1144 * TimeControl;
	if( marsphobosmoon > 360.0 )
		marsphobosmoon -= 360.0;

	//jupiter plaent and moons
	jupiter += 0.0843 * TimeControl;

	if( jupiter > 360.0 )
		jupiter -= 360.0;

	jupiterSpin += 2.421 * TimeControl;
	if( jupiterSpin > 360.0 )
		jupiterSpin -= 360.0;

	jupiteriomoon += 206.2 * TimeControl;
	if( jupiteriomoon > 360.0 )
		jupiteriomoon -= 360.0;

	jupitereuropamoon += 102.8 * TimeControl;
	if( jupitereuropamoon > 360.0 )
		jupitereuropamoon -= 360.0;

	jupitercallistomoon += 51.04 * TimeControl;
	if( jupitercallistomoon > 360.0 )
		jupitercallistomoon -= 360.0;

	jupiterganymedemoon += 21.86 * TimeControl;
	if( jupiterganymedemoon > 360.0 )
		jupiterganymedemoon -= 360.0;


	//saturn plaent and moons
	saturn += 0.0339 * TimeControl;

	if( saturn > 360.0 )
		saturn -= 360.0;

	saturnSpin += 2.252 * TimeControl;
	if( saturnSpin > 360.0 )
		saturnSpin -= 360.0;

	saturndionemoon += 133.3 * TimeControl;
	if( saturndionemoon > 360.0 )
		saturndionemoon -= 360.0;

	saturnenceladusmoon += 266.42 * TimeControl;
	if( saturnenceladusmoon > 360.0 )
		saturnenceladusmoon -= 360.0;

	saturnlapetusmoon += 4.601 * TimeControl;
	if( saturnlapetusmoon > 360.0 )
		saturnlapetusmoon -= 360.0;

	saturnmimasmooon += 387.4 * TimeControl;
	if( saturnmimasmooon > 360.0 )
		saturnmimasmooon -= 360.0;

	saturnrheamoon += 80.78 * TimeControl;
	if( saturnrheamoon > 360.0 )
		saturnrheamoon -= 360.0;

	saturntitanmoon += 22.88 * TimeControl;
	if( saturntitanmoon > 360.0 )
		saturntitanmoon -= 360.0;

	saturntrlhysmoon += 139.3 * TimeControl;
	if( saturntrlhysmoon > 360.0 )
		saturntrlhysmoon -= 360.0;


	//uranus plaent and moons
	uranus += 0.0119 * TimeControl;

	if( uranus > 360.0 )
		uranus -= 360.0;

	// Clockwise Rotation
	uranusSpin -= 2.39 * TimeControl;
	if( uranusSpin <= -360.0 )
		uranusSpin = 0.0;

	uranustitaniamoon += 41.92 * TimeControl;
	if( uranustitaniamoon > 360.0 )
		uranustitaniamoon -= 360.0;

	uranusoberonmoon += 27.11 * TimeControl;
	if( uranusoberonmoon > 360.0 )
		uranusoberonmoon -= 360.0;

	uranusumbrielmoon += 88.08 * TimeControl;
	if( uranusumbrielmoon > 360.0 )
		uranusumbrielmoon -= 360.0;


	//nebtune plaent and moons
	nebtune += 0.006 * TimeControl;
	//	nebtune += 0.06 * TimeControl;
	if( nebtune > 360.0 )
		nebtune -= 360.0;

	nebtuneSpin += 1.49 * TimeControl;
	if( nebtuneSpin > 360.0 )
		nebtuneSpin -= 360.0;

	nebtunemoontriton += 62.10 * TimeControl;
	if( nebtunemoontriton > 360.0 )
		nebtunemoontriton -= 360.0;


	//pluto plaent and moons
	pluto += 0.004 * TimeControl;
	//pluto += 0.04 * TimeControl;
	if( pluto > 360.0 )
		pluto -= 360.0;

	// Clockwise Rotation
	plutoSpin -= 1.565 * TimeControl;
	if( plutoSpin <= -360.0 )
		plutoSpin = 0.0;

	plutomooncharon += 57.14 * TimeControl;
	if( plutomooncharon > 360.0 )
		plutomooncharon -= 360.0;

	// force a call to Display( ) next time it is convenient:

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}




//
// glui buttons callback:
//

void
	Buttons( int id )
{
	switch( id )
	{
	case RESET:
		Reset( );
		Glui->sync_live( );
		glutSetWindow( MainWindow );
		glutPostRedisplay( );
		break;

	case QUIT:
		// gracefully close the glui window:
		// gracefully close out the graphics:
		// gracefully close the graphics window:
		// gracefully exit the program:
		RemoveAllTexutreAtExit();
		Glui->close( );
		glutSetWindow( MainWindow );
		glFinish( );	
		glutDestroyWindow( MainWindow );

		exit( 0 );
		break;

	default:
		fprintf( stderr, "Don't know what to do with Button ID %d\n", id );
	}

}


//
// draw the complete scene:
//




void
	Display( void )
{
	if( DebugOn != 0 )
	{
		fprintf( stderr, "Display\n" );
	}


	// set which window we want to do the graphics into:

	glutSetWindow( MainWindow );


	// erase the background:

	glDrawBuffer( GL_BACK );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable( GL_DEPTH_TEST );


	// specify shading to be flat:

	glShadeModel( GL_FLAT );


	// set the viewport to a square centered in the window:

	GLsizei vx = glutGet( GLUT_WINDOW_WIDTH );
	GLsizei vy = glutGet( GLUT_WINDOW_HEIGHT );
	GLsizei v = vx < vy ? vx : vy;			// minimum dimension
	GLint xl = ( vx - v ) / 2;
	GLint yb = ( vy - v ) / 2;
	glViewport( xl, yb,  v, v );


	// set the viewing volume:
	// remember that the Z clipping  values are actually
	// given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D( ) IF YOU ARE DOING 2D !

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	gluPerspective( 90., 1.,	0.1, 3000000. );


	// place the objects into the scene:

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );

	//RemoveAllTexutreAtExit();
	if (freezebox == 0)
		glutIdleFunc( NULL );
	else
		glutIdleFunc( Animate );

	// set the eye position, look-at position, and up-vector:
	// IF DOING 2D, REMOVE THIS -- OTHERWISE ALL YOUR 2D WILL DISAPPEAR !



	if (WhichCamera == EducationalSolar)
	{
		gluLookAt( 0., 40., 200.,     0., 0., 0.,     0., 1., 0. );


		// translate the objects in the scene:
		// note the minus sign on the z value
		// this is to make the appearance of the glui z translate
		// widget more intuitively match the translate behavior
		// DO NOT TRANSLATE IN Z IF YOU ARE DOING 2D !

		glTranslatef( (GLfloat)TransXYZ[0], (GLfloat)TransXYZ[1], -(GLfloat)TransXYZ[2] );


		// rotate the scene:
		// DO NOT ROTATE (EXCEPT ABOUT Z) IF YOU ARE DOING 2D !

		glRotatef( (GLfloat)Yrot, 0., 1., 0. );
		glRotatef( (GLfloat)Xrot, 1., 0., 0. );
		glMultMatrixf( (const GLfloat *) RotMatrix );


		// uniformly scale the scene:

		glScalef( (GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale );
		GLfloat scale2 = 1.0 + Scale2;		// because glui translation starts at 0.
		if( scale2 < MINSCALE )
			scale2 = MINSCALE;
		glScalef( (GLfloat)scale2, (GLfloat)scale2, (GLfloat)scale2 );


		// set the fog parameters:
		// DON'T NEED THIS IF DOING 2D !


		glDisable( GL_FOG );

		glEnable( GL_NORMALIZE );


		if (LinesEnabled)
		{
			//void PlanetsOrbits(float Jupiture_distance1, float Venus_distance1, float Earth_distance1, float Mars_distance1, float Saturn_distance1, float Uranus_distance1, float Neptune_distance1, float Pluto_dostance1)

			PlanetsOrbits(30, EducationalSolarJupiture_distance,  EducationalSolarVenus_distance,  EducationalSolarEarth_distance,  EducationalSolarMars_distance,  EducationalSolarSaturn_distance,  EducationalSolarUranus_distance,  EducationalSolarNeptune_distance,  EducationalSolarPluto_dostance);
		}

		// Sun Light			
		glEnable( GL_LIGHTING );
		glShadeModel(GL_SMOOTH);
		SetPointLight(GL_LIGHT1,0.0,0.0,0.0,1,1,.9);
		//SetMaterial(.5,.5,.5,-2);

		// Space Object
		SpaceObject( 4500);

		// Sun Object
		SunObject( 20.0);

		// Jupiter Object
		JupiterObject(  10,   140,   0.6,  12,   .5,   14,   .8,   16,   .8,   20);

		// Mercury Object
		MercuryObject(  0.6,   30);

		// Venus Object
		VenusObject(  1.0,   40);

		// Earth Object
		EarthObject(  5,   60.0,   .5,   7.0);

		//Mars Object
		MarsObject(  0.9,   80,   .1,   2.0,   .08 ,   3.);

		//Saturn Object
		SaturnObejct(  8.0,   220,   .2,   9.0,   0.25,   10.0,   0.275,   11.0,   0.325,   14.0,   0.352,   15.5,   0.52,   20.0,   0.5,   22.0);

		// Saturn Ring
		SaturnRing(  20,   220 ,   1.0,   5,   100);

		// Uranus Object
		UranusObject(  6.0,   260,   .5,   10.0,   .9,   12.0,   .8,   14.0);

		// Nebtune Object
		NebtuneObject(  5.0,   300, .5, 7.0);

		//Pluto Object
		PlutoObject(  .4,   340,   .05,   1.2);


	}

	else if (WhichCamera == Solar)

	{
		glMatrixMode( GL_PROJECTION );
		glLoadIdentity( );
		gluPerspective( 90., 1.,	0.1, 3000000 );


		// place the objects into the scene:

		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity( );

		gluLookAt( 0., 400, 900,     0., 0., 0.,     0., 1., 0. );


		// translate the objects in the scene:
		// note the minus sign on the z value
		// this is to make the appearance of the glui z translate
		// widget more intuitively match the translate behavior
		// DO NOT TRANSLATE IN Z IF YOU ARE DOING 2D !

		glTranslatef( (GLfloat)TransXYZ[0], (GLfloat)TransXYZ[1], -(GLfloat)TransXYZ[2] );


		// rotate the scene:
		// DO NOT ROTATE (EXCEPT ABOUT Z) IF YOU ARE DOING 2D !

		glRotatef( (GLfloat)Yrot, 0., 1., 0. );
		glRotatef( (GLfloat)Xrot, 1., 0., 0. );
		glMultMatrixf( (const GLfloat *) RotMatrix );


		// uniformly scale the scene:

		glScalef( (GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale );
		GLfloat scale2 = 1.0 + Scale2;		// because glui translation starts at 0.
		if( scale2 < MINSCALE )
			scale2 = MINSCALE;
		glScalef( (GLfloat)scale2, (GLfloat)scale2, (GLfloat)scale2 );


		// set the fog parameters:
		// DON'T NEED THIS IF DOING 2D !

		DistanceScaler = 500;
		glDisable( GL_FOG );

		glEnable( GL_NORMALIZE );


		if (LinesEnabled)
		{
			PlanetsOrbits(mercurydistance*DistanceScaler, Jupiture_distance*DistanceScaler,  Venus_distance*DistanceScaler,  Earth_distance * DistanceScaler,  Mars_distance * DistanceScaler,  Saturn_distance*DistanceScaler,  Uranus_distance*DistanceScaler,  Neptune_distance*DistanceScaler,  Pluto_distance*DistanceScaler);

		}

		// Sun Light			
		glEnable( GL_LIGHTING );
		glShadeModel(GL_SMOOTH);
		SetPointLight(GL_LIGHT1,0.0,0.0,0.0,1,1,.9);
		//SetMaterial(.5,.5,.5,-2);

		// Space Object
		//SpaceObject(float SpaceSize)
		SpaceObject( 200000 );

		// Sun Object
		//SunObject(float SunSize)
		SunObject( SunSize * SizeScaler);

		//Mercury Object
		//MercuryObject(float MercurySize, float MercuryLocation)
		MercuryObject(MercurySize*SizeScaler, mercurydistance*DistanceScaler);

		// Venus Object
		//VenusObject(float VenusSize, float VenusLocation)
		VenusObject(  VenusSize * SizeScaler,   Venus_distance*DistanceScaler);

		// Earth Object
		//EarthObject(float EarthSize, float EarthLocation, float MoonSize, float Moon_Location)
		EarthObject(  EarthSize * SizeScaler,   Earth_distance*DistanceScaler,   earthMoonSize*SizeScaler,   earthMoonDistance*DistanceScaler);

		//Mars Object
		//MarsObject(float MarsSize, float MarsLocation, float PhobosSize, float PhobosLocation , float DiemosSize , float DiemosLocaion)
		MarsObject(  MarsSize * SizeScaler,   Mars_distance*DistanceScaler,   marsdiemosmoonSize* SizeScaler,   marsdiemosmoonDistance * DistanceScaler,   marsphobosmoonSize *SizeScaler,   marsphobosmoonDistance* DistanceScaler);

		// Jupiter Object
		//JupiterObject(float JubiterSize, float JubiterLocation, float IoSize, float IoLocation, float EuropaSize, float EuropaLocation, float GanymedeSize, float GanymedeLocation, float CallistoSize, float CallistoLocation)
		JupiterObject(  JupitureSize * SizeScaler,   Jupiture_distance*DistanceScaler,   jupiteriomoonSize *SizeScaler,  jupiteriomoonDistance*DistanceScaler,   jupitereuropamoonSize * SizeScaler,   jupitereuropamoonDistance*DistanceScaler,   jupiterganymedemoonSize * SizeScaler,   jupiterganymedemoonDistance*DistanceScaler,   jupitercallistomoonSize  * SizeScaler,   jupitercallistomoonDistance*DistanceScaler);

		//Saturn Object
		//void SaturnObejct(float SaturnSize, float SaturnLocation, float MimasSize, float MimasLocation, float EnceladusSize, float EnceladusLocation, float TrlhysSize, float TrlhysLocation, float DioneSize, float DioneLocation, float RheaSize, float RheaLocation, float TitanSize, float TitanLocation, float LapetusSize, float LapetusLocation)
		SaturnObejct(  SaturnSize * SizeScaler,   Saturn_distance*DistanceScaler,   saturnmimasmooonSize*SizeScaler,   saturnmimasmooonDistance*DistanceScaler,   saturnenceladusmoonSize*SizeScaler,   saturnenceladusmoonDistance*DistanceScaler,   saturntrlhysmoonSize*SizeScaler,   saturntrlhysmoonDistance*DistanceScaler,   saturndionemoonSize*SizeScaler,   saturndionemoonDistance*DistanceScaler,   saturnrheamoonSize*SizeScaler,   saturnrheamoonDistance*DistanceScaler,   saturntitanmoonSize*SizeScaler,   saturntitanmoonDistance*DistanceScaler,   saturnlapetusmoonSize*SizeScaler,   saturnlapetusmoonDistance*DistanceScaler);

		// Saturn Ring
		//void SaturnRing(float RingOuterRadious, float RingLocation, float Eccentricity, float RingInnerRadious, float SideSmooth)
		SaturnRing(  SaturnRingOuterRadius * SizeScaler,   Saturn_distance*DistanceScaler ,   1.0,   SaturnRingInnerRadius * SizeScaler,   100);

		// Uranus Object
		//UranusObject(float UranusSize, float UranusLocation, float UmbrielSize, float UmbrielLocation, float TitaniaSize, float TitaniaLocation, float OberonSize, float OberonLocation)
		UranusObject(  UranusSize * SizeScaler,   Uranus_distance*DistanceScaler,   uranusumbrielmoonSize*SizeScaler,   uranusumbrielmoonDistance*DistanceScaler,   uranustitaniamoonSize*SizeScaler,   uranustitaniamoonDistance*DistanceScaler,   uranusoberonmoonSize*SizeScaler,   uranusoberonmoonDistance*DistanceScaler);

		// Nebtune Object
		//NebtuneObject(float NebtuneSize, float NebtuneLocation, float TritonSize, float TritonLocation )
		NebtuneObject(  NeptuneSize * SizeScaler,   Neptune_distance*DistanceScaler, nebtunemoontritonSize *SizeScaler, nebtunemoontritonDistance*DistanceScaler);

		//Pluto Object
		//PlutoObject(float PlutoSize, float PlutoLocation, float CharonSize, float CharonLocation)
		PlutoObject(  PlutoSize * SizeScaler,   Pluto_distance*DistanceScaler,   plutomooncharonSize*SizeScaler,   plutocmoonharonDistance*DistanceScaler);



	}

	else if (WhichCamera == PlanetTop)
	{

		glDisable( GL_FOG );

		glEnable( GL_NORMALIZE );

		// Sun Light			
		glEnable( GL_LIGHTING );
		glShadeModel(GL_SMOOTH);
		// Disable Planets Orbits 

		if (LinesEnabled == 0 || LinesEnabled == 1 )
		{
			PlanetsOrbits(0,0,0,0,0,0,0,0,0);
		}
		DistanceScaler = 3200;
		// Jupiter Object
		if (WhichPlanet == Jupiter)
		{

			//Caculate the earth postion
			GLfloat JupiterPos[3] = {Jupiture_distance*DistanceScaler * cos(-jupiter * M_PI / 180), 0, Jupiture_distance*DistanceScaler * sin(-jupiter * M_PI / 180)};
			//Caculate the Camera Position
			GLfloat cameraPos[3] = {Jupiture_distance*DistanceScaler * cos(-jupiter * M_PI / 180), (16*SizeScaler), Jupiture_distance*DistanceScaler * sin(-jupiter * M_PI / 180)};
			//Setup the camear on the top of the moon pointing 
			gluLookAt(cameraPos[0], cameraPos[1], cameraPos[2], JupiterPos[0], JupiterPos[1], JupiterPos[2]-(20*SizeScaler), 0, 0, -1);
			// Jupiter Planet

			SetPointLight(GL_LIGHT1,0.0,0.0,0.0,1,1,.9);
			//SetMaterial(1,1,1,.2);
			JupiterObject(  JupitureSize * SizeScaler,   Jupiture_distance*DistanceScaler,   jupiteriomoonSize *SizeScaler,  jupiteriomoonDistance*DistanceScaler,   jupitereuropamoonSize * SizeScaler,   jupitereuropamoonDistance*DistanceScaler,   jupiterganymedemoonSize * SizeScaler,   jupiterganymedemoonDistance*DistanceScaler,   jupitercallistomoonSize  * SizeScaler,   jupitercallistomoonDistance*DistanceScaler);


			//JupiterObject(  10,   1400,   0.6,  12,   .5,   14,   .8,   16,   .8,   20);
			// Space Object
			//SpaceObject(float SpaceSize)
			SpaceObject( 200000 );

			// Sun Object
			//SunObject(float SunSize)
			SunObject( SunSize * SizeScaler);

			//Mercury Object
			//MercuryObject(float MercurySize, float MercuryLocation)
			MercuryObject(MercurySize*SizeScaler, mercurydistance*DistanceScaler);

			// Venus Object
			//VenusObject(float VenusSize, float VenusLocation)
			VenusObject(  VenusSize * SizeScaler,   Venus_distance*DistanceScaler);

			// Earth Object
			//EarthObject(float EarthSize, float EarthLocation, float MoonSize, float Moon_Location)
			EarthObject(  EarthSize * SizeScaler,   Earth_distance*DistanceScaler,   earthMoonSize*SizeScaler,   earthMoonDistance*DistanceScaler);

			//Mars Object
			//MarsObject(float MarsSize, float MarsLocation, float PhobosSize, float PhobosLocation , float DiemosSize , float DiemosLocaion)
			MarsObject(  MarsSize * SizeScaler,   Mars_distance*DistanceScaler,   marsdiemosmoonSize* SizeScaler,   marsdiemosmoonDistance * DistanceScaler,   marsphobosmoonSize *SizeScaler,   marsphobosmoonDistance* DistanceScaler);

			// Jupiter Object
			//JupiterObject(float JubiterSize, float JubiterLocation, float IoSize, float IoLocation, float EuropaSize, float EuropaLocation, float GanymedeSize, float GanymedeLocation, float CallistoSize, float CallistoLocation)
			//JupiterObject(  JupitureSize * SizeScaler,   Jupiture_distance*DistanceScaler,   jupiteriomoonSize *SizeScaler,  jupiteriomoonDistance*DistanceScaler,   jupitereuropamoonSize * SizeScaler,   jupitereuropamoonDistance*DistanceScaler,   jupiterganymedemoonSize * SizeScaler,   jupiterganymedemoonDistance*DistanceScaler,   jupitercallistomoonSize  * SizeScaler,   jupitercallistomoonDistance*DistanceScaler);

			//Saturn Object
			//void SaturnObejct(float SaturnSize, float SaturnLocation, float MimasSize, float MimasLocation, float EnceladusSize, float EnceladusLocation, float TrlhysSize, float TrlhysLocation, float DioneSize, float DioneLocation, float RheaSize, float RheaLocation, float TitanSize, float TitanLocation, float LapetusSize, float LapetusLocation)
			SaturnObejct(  SaturnSize * SizeScaler,   Saturn_distance*DistanceScaler,   saturnmimasmooonSize*SizeScaler,   saturnmimasmooonDistance*DistanceScaler,   saturnenceladusmoonSize*SizeScaler,   saturnenceladusmoonDistance*DistanceScaler,   saturntrlhysmoonSize*SizeScaler,   saturntrlhysmoonDistance*DistanceScaler,   saturndionemoonSize*SizeScaler,   saturndionemoonDistance*DistanceScaler,   saturnrheamoonSize*SizeScaler,   saturnrheamoonDistance*DistanceScaler,   saturntitanmoonSize*SizeScaler,   saturntitanmoonDistance*DistanceScaler,   saturnlapetusmoonSize*SizeScaler,   saturnlapetusmoonDistance*DistanceScaler);

			// Saturn Ring
			//float SaturnRingOuterRadius = 21.469;
			//float SaturnRingInnerRadius = 9.85;
			//void SaturnRing(float RingOuterRadious, float RingLocation, float Eccentricity, float RingInnerRadious, float SideSmooth)
			SaturnRing(  SaturnRingOuterRadius * SizeScaler,   Saturn_distance*DistanceScaler ,   1.0,   SaturnRingInnerRadius * SizeScaler,   100);

			// Uranus Object
			//UranusObject(float UranusSize, float UranusLocation, float UmbrielSize, float UmbrielLocation, float TitaniaSize, float TitaniaLocation, float OberonSize, float OberonLocation)
			UranusObject(  UranusSize * SizeScaler,   Uranus_distance*DistanceScaler,   uranusumbrielmoonSize*SizeScaler,   uranusumbrielmoonDistance*DistanceScaler,   uranustitaniamoonSize*SizeScaler,   uranustitaniamoonDistance*DistanceScaler,   uranusoberonmoonSize*SizeScaler,   uranusoberonmoonDistance*DistanceScaler);

			// Nebtune Object
			//NebtuneObject(float NebtuneSize, float NebtuneLocation, float TritonSize, float TritonLocation )
			NebtuneObject(  NeptuneSize * SizeScaler,   Neptune_distance*DistanceScaler, nebtunemoontritonSize *SizeScaler, nebtunemoontritonDistance*DistanceScaler);

			//Pluto Object
			//PlutoObject(float PlutoSize, float PlutoLocation, float CharonSize, float CharonLocation)
			PlutoObject(  PlutoSize * SizeScaler,   Pluto_distance*DistanceScaler,   plutomooncharonSize*SizeScaler,   plutocmoonharonDistance*DistanceScaler);
		}

		// Venus Object
		else if (WhichPlanet == Venus)
		{


			//Caculate the earth postion
			GLfloat JupiterPos[3] = {Venus_distance*DistanceScaler * cos(-venus * M_PI / 180), 0, Venus_distance*DistanceScaler * sin(-venus * M_PI / 180)};
			//Caculate the Camera Position
			GLfloat cameraPos[3] = {Venus_distance*DistanceScaler * cos(-venus * M_PI / 180), (1.4*SizeScaler), Venus_distance*DistanceScaler * sin(-venus * M_PI / 180)};
			//Setup the camear on the top of the moon pointing 
			gluLookAt(cameraPos[0], cameraPos[1], cameraPos[2], JupiterPos[0], JupiterPos[1], JupiterPos[2]-(5*SizeScaler), 0, 0, -1);
			// Jupiter Planet

			SetPointLight(GL_LIGHT1,0.0,0.0,0.0,1,1,.9);
			//SetMaterial(1,1,1,.2);
			VenusObject(  VenusSize * SizeScaler,   Venus_distance*DistanceScaler);

			// Space Object
			//SpaceObject(float SpaceSize)
			SpaceObject( 200000 );

			// Sun Object
			//SunObject(float SunSize)
			SunObject( SunSize * SizeScaler);

			//Mercury Object
			//MercuryObject(float MercurySize, float MercuryLocation)
			MercuryObject(MercurySize*SizeScaler, mercurydistance*DistanceScaler);

			// Venus Object
			//VenusObject(float VenusSize, float VenusLocation)
			//VenusObject(  VenusSize * SizeScaler,   Venus_distance*DistanceScaler);

			// Earth Object
			//EarthObject(float EarthSize, float EarthLocation, float MoonSize, float Moon_Location)
			EarthObject(  EarthSize * SizeScaler,   Earth_distance*DistanceScaler,   earthMoonSize*SizeScaler,   earthMoonDistance*DistanceScaler);

			//Mars Object
			//MarsObject(float MarsSize, float MarsLocation, float PhobosSize, float PhobosLocation , float DiemosSize , float DiemosLocaion)
			MarsObject(  MarsSize * SizeScaler,   Mars_distance*DistanceScaler,   marsdiemosmoonSize* SizeScaler,   marsdiemosmoonDistance * DistanceScaler,   marsphobosmoonSize *SizeScaler,   marsphobosmoonDistance* DistanceScaler);

			// Jupiter Object
			//JupiterObject(float JubiterSize, float JubiterLocation, float IoSize, float IoLocation, float EuropaSize, float EuropaLocation, float GanymedeSize, float GanymedeLocation, float CallistoSize, float CallistoLocation)
			JupiterObject(  JupitureSize * SizeScaler,   Jupiture_distance*DistanceScaler,   jupiteriomoonSize *SizeScaler,  jupiteriomoonDistance*DistanceScaler,   jupitereuropamoonSize * SizeScaler,   jupitereuropamoonDistance*DistanceScaler,   jupiterganymedemoonSize * SizeScaler,   jupiterganymedemoonDistance*DistanceScaler,   jupitercallistomoonSize  * SizeScaler,   jupitercallistomoonDistance*DistanceScaler);

			//Saturn Object
			//void SaturnObejct(float SaturnSize, float SaturnLocation, float MimasSize, float MimasLocation, float EnceladusSize, float EnceladusLocation, float TrlhysSize, float TrlhysLocation, float DioneSize, float DioneLocation, float RheaSize, float RheaLocation, float TitanSize, float TitanLocation, float LapetusSize, float LapetusLocation)
			SaturnObejct(  SaturnSize * SizeScaler,   Saturn_distance*DistanceScaler,   saturnmimasmooonSize*SizeScaler,   saturnmimasmooonDistance*DistanceScaler,   saturnenceladusmoonSize*SizeScaler,   saturnenceladusmoonDistance*DistanceScaler,   saturntrlhysmoonSize*SizeScaler,   saturntrlhysmoonDistance*DistanceScaler,   saturndionemoonSize*SizeScaler,   saturndionemoonDistance*DistanceScaler,   saturnrheamoonSize*SizeScaler,   saturnrheamoonDistance*DistanceScaler,   saturntitanmoonSize*SizeScaler,   saturntitanmoonDistance*DistanceScaler,   saturnlapetusmoonSize*SizeScaler,   saturnlapetusmoonDistance*DistanceScaler);

			// Saturn Ring
			//void SaturnRing(float RingOuterRadious, float RingLocation, float Eccentricity, float RingInnerRadious, float SideSmooth)
			SaturnRing(  SaturnRingOuterRadius * SizeScaler,   Saturn_distance*DistanceScaler ,   1.0,   SaturnRingInnerRadius * SizeScaler,   100);

			// Uranus Object
			//UranusObject(float UranusSize, float UranusLocation, float UmbrielSize, float UmbrielLocation, float TitaniaSize, float TitaniaLocation, float OberonSize, float OberonLocation)
			UranusObject(  UranusSize * SizeScaler,   Uranus_distance*DistanceScaler,   uranusumbrielmoonSize*SizeScaler,   uranusumbrielmoonDistance*DistanceScaler,   uranustitaniamoonSize*SizeScaler,   uranustitaniamoonDistance*DistanceScaler,   uranusoberonmoonSize*SizeScaler,   uranusoberonmoonDistance*DistanceScaler);

			// Nebtune Object
			//NebtuneObject(float NebtuneSize, float NebtuneLocation, float TritonSize, float TritonLocation )
			NebtuneObject(  NeptuneSize * SizeScaler,   Neptune_distance*DistanceScaler, nebtunemoontritonSize *SizeScaler, nebtunemoontritonDistance*DistanceScaler);

			//Pluto Object
			//PlutoObject(float PlutoSize, float PlutoLocation, float CharonSize, float CharonLocation)
			PlutoObject(  PlutoSize * SizeScaler,   Pluto_distance*DistanceScaler,   plutomooncharonSize*SizeScaler,   plutocmoonharonDistance*DistanceScaler);

		}

		// Earth Object
		else if (WhichPlanet == Earth)
		{


			//Caculate the earth postion
			GLfloat JupiterPos[3] = {Earth_distance*DistanceScaler * cos(-earth * M_PI / 180), 0, Earth_distance*DistanceScaler * sin(-earth * M_PI / 180)};
			//Caculate the Camera Position
			GLfloat cameraPos[3] = {Earth_distance*DistanceScaler * cos(-earth * M_PI / 180), (1.5*SizeScaler), Earth_distance*DistanceScaler * sin(-earth * M_PI / 180)};
			//Setup the camear on the top of the moon pointing 
			gluLookAt(cameraPos[0], cameraPos[1], cameraPos[2], JupiterPos[0], JupiterPos[1], JupiterPos[2]-(2.2*SizeScaler), 0, 0, -1);
			// Jupiter Planet

			SetPointLight(GL_LIGHT1,0.0,0.0,0.0,1,1,.9);
			// Earth Object
			EarthObject(  EarthSize * SizeScaler,   Earth_distance*DistanceScaler,   earthMoonSize*SizeScaler,   earthMoonDistance*DistanceScaler);


			// Space Object
			//SpaceObject(float SpaceSize)
			SpaceObject( 200000 );

			// Sun Object
			//SunObject(float SunSize)
			SunObject( SunSize * SizeScaler);

			//Mercury Object
			//MercuryObject(float MercurySize, float MercuryLocation)
			MercuryObject(MercurySize*SizeScaler, mercurydistance*DistanceScaler);

			// Venus Object
			//VenusObject(float VenusSize, float VenusLocation)
			VenusObject(  VenusSize * SizeScaler,   Venus_distance*DistanceScaler);

			// Earth Object
			//EarthObject(float EarthSize, float EarthLocation, float MoonSize, float Moon_Location)
			//EarthObject(  EarthSize * SizeScaler,   Earth_distance*DistanceScaler,   earthMoonSize*SizeScaler,   earthMoonDistance*DistanceScaler);

			//Mars Object
			//MarsObject(float MarsSize, float MarsLocation, float PhobosSize, float PhobosLocation , float DiemosSize , float DiemosLocaion)
			MarsObject(  MarsSize * SizeScaler,   Mars_distance*DistanceScaler,   marsdiemosmoonSize* SizeScaler,   marsdiemosmoonDistance * DistanceScaler,   marsphobosmoonSize *SizeScaler,   marsphobosmoonDistance* DistanceScaler);

			// Jupiter Object
			//JupiterObject(float JubiterSize, float JubiterLocation, float IoSize, float IoLocation, float EuropaSize, float EuropaLocation, float GanymedeSize, float GanymedeLocation, float CallistoSize, float CallistoLocation)
			JupiterObject(  JupitureSize * SizeScaler,   Jupiture_distance*DistanceScaler,   jupiteriomoonSize *SizeScaler,  jupiteriomoonDistance*DistanceScaler,   jupitereuropamoonSize * SizeScaler,   jupitereuropamoonDistance*DistanceScaler,   jupiterganymedemoonSize * SizeScaler,   jupiterganymedemoonDistance*DistanceScaler,   jupitercallistomoonSize  * SizeScaler,   jupitercallistomoonDistance*DistanceScaler);

			//Saturn Object
			//void SaturnObejct(float SaturnSize, float SaturnLocation, float MimasSize, float MimasLocation, float EnceladusSize, float EnceladusLocation, float TrlhysSize, float TrlhysLocation, float DioneSize, float DioneLocation, float RheaSize, float RheaLocation, float TitanSize, float TitanLocation, float LapetusSize, float LapetusLocation)
			SaturnObejct(  SaturnSize * SizeScaler,   Saturn_distance*DistanceScaler,   saturnmimasmooonSize*SizeScaler,   saturnmimasmooonDistance*DistanceScaler,   saturnenceladusmoonSize*SizeScaler,   saturnenceladusmoonDistance*DistanceScaler,   saturntrlhysmoonSize*SizeScaler,   saturntrlhysmoonDistance*DistanceScaler,   saturndionemoonSize*SizeScaler,   saturndionemoonDistance*DistanceScaler,   saturnrheamoonSize*SizeScaler,   saturnrheamoonDistance*DistanceScaler,   saturntitanmoonSize*SizeScaler,   saturntitanmoonDistance*DistanceScaler,   saturnlapetusmoonSize*SizeScaler,   saturnlapetusmoonDistance*DistanceScaler);

			// Saturn Ring
			//void SaturnRing(float RingOuterRadious, float RingLocation, float Eccentricity, float RingInnerRadious, float SideSmooth)
			SaturnRing(  SaturnRingOuterRadius * SizeScaler,   Saturn_distance*DistanceScaler ,   1.0,   SaturnRingInnerRadius * SizeScaler,   100);

			// Uranus Object
			//UranusObject(float UranusSize, float UranusLocation, float UmbrielSize, float UmbrielLocation, float TitaniaSize, float TitaniaLocation, float OberonSize, float OberonLocation)
			UranusObject(  UranusSize * SizeScaler,   Uranus_distance*DistanceScaler,   uranusumbrielmoonSize*SizeScaler,   uranusumbrielmoonDistance*DistanceScaler,   uranustitaniamoonSize*SizeScaler,   uranustitaniamoonDistance*DistanceScaler,   uranusoberonmoonSize*SizeScaler,   uranusoberonmoonDistance*DistanceScaler);

			// Nebtune Object
			//NebtuneObject(float NebtuneSize, float NebtuneLocation, float TritonSize, float TritonLocation )
			NebtuneObject(  NeptuneSize * SizeScaler,   Neptune_distance*DistanceScaler, nebtunemoontritonSize *SizeScaler, nebtunemoontritonDistance*DistanceScaler);

			//Pluto Object
			//PlutoObject(float PlutoSize, float PlutoLocation, float CharonSize, float CharonLocation)
			PlutoObject(  PlutoSize * SizeScaler,   Pluto_distance*DistanceScaler,   plutomooncharonSize*SizeScaler,   plutocmoonharonDistance*DistanceScaler);

		}
		//Mars Object
		else if (WhichPlanet == Mars)
		{

			//Caculate the earth postion
			GLfloat MarsPos[3] = {Mars_distance*DistanceScaler * cos(-mars * M_PI / 180), 0, Mars_distance*DistanceScaler * sin(-mars * M_PI / 180)};
			//Caculate the Camera Position
			GLfloat cameraPos[3] = {Mars_distance*DistanceScaler * cos(-mars * M_PI / 180), (1*SizeScaler), Mars_distance*DistanceScaler * sin(-mars * M_PI / 180)};
			//Setup the camear on the top of the moon pointing 
			gluLookAt(cameraPos[0], cameraPos[1], cameraPos[2], MarsPos[0], MarsPos[1], MarsPos[2]-(1.2*SizeScaler), 0, 0, -1);


			SetPointLight(GL_LIGHT1,0.0,0.0,0.0,1,1,.9);
			// Mars Object
			MarsObject(  MarsSize * SizeScaler,   Mars_distance*DistanceScaler,   marsdiemosmoonSize* SizeScaler,   marsdiemosmoonDistance * DistanceScaler,   marsphobosmoonSize *SizeScaler,   marsphobosmoonDistance* DistanceScaler);

			// Space Object
			//SpaceObject(float SpaceSize)
			SpaceObject( 200000 );

			// Sun Object
			//SunObject(float SunSize)
			SunObject( SunSize * SizeScaler);

			//Mercury Object
			//MercuryObject(float MercurySize, float MercuryLocation)
			MercuryObject(MercurySize*SizeScaler, mercurydistance*DistanceScaler);

			// Venus Object
			//VenusObject(float VenusSize, float VenusLocation)
			VenusObject(  VenusSize * SizeScaler,   Venus_distance*DistanceScaler);

			// Earth Object
			//EarthObject(float EarthSize, float EarthLocation, float MoonSize, float Moon_Location)
			EarthObject(  EarthSize * SizeScaler,   Earth_distance*DistanceScaler,   earthMoonSize*SizeScaler,   earthMoonDistance*DistanceScaler);

			//Mars Object
			//MarsObject(float MarsSize, float MarsLocation, float PhobosSize, float PhobosLocation , float DiemosSize , float DiemosLocaion)
			//MarsObject(  MarsSize * SizeScaler,   Mars_distance*DistanceScaler,   marsdiemosmoonSize* SizeScaler,   marsdiemosmoonDistance * DistanceScaler,   marsphobosmoonSize *SizeScaler,   marsphobosmoonDistance* DistanceScaler);

			// Jupiter Object
			//JupiterObject(float JubiterSize, float JubiterLocation, float IoSize, float IoLocation, float EuropaSize, float EuropaLocation, float GanymedeSize, float GanymedeLocation, float CallistoSize, float CallistoLocation)
			JupiterObject(  JupitureSize * SizeScaler,   Jupiture_distance*DistanceScaler,   jupiteriomoonSize *SizeScaler,  jupiteriomoonDistance*DistanceScaler,   jupitereuropamoonSize * SizeScaler,   jupitereuropamoonDistance*DistanceScaler,   jupiterganymedemoonSize * SizeScaler,   jupiterganymedemoonDistance*DistanceScaler,   jupitercallistomoonSize  * SizeScaler,   jupitercallistomoonDistance*DistanceScaler);

			//Saturn Object
			//void SaturnObejct(float SaturnSize, float SaturnLocation, float MimasSize, float MimasLocation, float EnceladusSize, float EnceladusLocation, float TrlhysSize, float TrlhysLocation, float DioneSize, float DioneLocation, float RheaSize, float RheaLocation, float TitanSize, float TitanLocation, float LapetusSize, float LapetusLocation)
			SaturnObejct(  SaturnSize * SizeScaler,   Saturn_distance*DistanceScaler,   saturnmimasmooonSize*SizeScaler,   saturnmimasmooonDistance*DistanceScaler,   saturnenceladusmoonSize*SizeScaler,   saturnenceladusmoonDistance*DistanceScaler,   saturntrlhysmoonSize*SizeScaler,   saturntrlhysmoonDistance*DistanceScaler,   saturndionemoonSize*SizeScaler,   saturndionemoonDistance*DistanceScaler,   saturnrheamoonSize*SizeScaler,   saturnrheamoonDistance*DistanceScaler,   saturntitanmoonSize*SizeScaler,   saturntitanmoonDistance*DistanceScaler,   saturnlapetusmoonSize*SizeScaler,   saturnlapetusmoonDistance*DistanceScaler);

			// Saturn Ring
			//void SaturnRing(float RingOuterRadious, float RingLocation, float Eccentricity, float RingInnerRadious, float SideSmooth)
			SaturnRing(  SaturnRingOuterRadius * SizeScaler,   Saturn_distance*DistanceScaler ,   1.0,   SaturnRingInnerRadius * SizeScaler,   100);

			// Uranus Object
			//UranusObject(float UranusSize, float UranusLocation, float UmbrielSize, float UmbrielLocation, float TitaniaSize, float TitaniaLocation, float OberonSize, float OberonLocation)
			UranusObject(  UranusSize * SizeScaler,   Uranus_distance*DistanceScaler,   uranusumbrielmoonSize*SizeScaler,   uranusumbrielmoonDistance*DistanceScaler,   uranustitaniamoonSize*SizeScaler,   uranustitaniamoonDistance*DistanceScaler,   uranusoberonmoonSize*SizeScaler,   uranusoberonmoonDistance*DistanceScaler);

			// Nebtune Object
			//NebtuneObject(float NebtuneSize, float NebtuneLocation, float TritonSize, float TritonLocation )
			NebtuneObject(  NeptuneSize * SizeScaler,   Neptune_distance*DistanceScaler, nebtunemoontritonSize *SizeScaler, nebtunemoontritonDistance*DistanceScaler);

			//Pluto Object
			//PlutoObject(float PlutoSize, float PlutoLocation, float CharonSize, float CharonLocation)
			PlutoObject(  PlutoSize * SizeScaler,   Pluto_distance*DistanceScaler,   plutomooncharonSize*SizeScaler,   plutocmoonharonDistance*DistanceScaler);

		}
		//Saturn Object
		else if (WhichPlanet == Saturn)
		{


			//Caculate the earth postion
			GLfloat SaturnPos[3] = {Saturn_distance*DistanceScaler * cos(-saturn * M_PI / 180), 0, Saturn_distance*DistanceScaler * sin(-saturn * M_PI / 180)};
			//Caculate the Camera Position
			GLfloat cameraPos[3] = {Saturn_distance*DistanceScaler * cos(-saturn * M_PI / 180), (7), Saturn_distance*DistanceScaler * sin(-saturn * M_PI / 180)};
			//Setup the camear on the top of the moon pointing 
			gluLookAt(cameraPos[0], cameraPos[1], cameraPos[2], SaturnPos[0], SaturnPos[1], SaturnPos[2]-(12), 0, 0, -1);
			// Jupiter Planet

			// Space Object
			//SpaceObject(float SpaceSize)
			SpaceObject( 200000 );

			SetPointLight(GL_LIGHT1,0.0,0.0,0.0,1,1,.9);
			//SetMaterial(1,1,1,.2);
			//Saturn Object
			//void SaturnObejct(float SaturnSize, float SaturnLocation, float MimasSize, float MimasLocation, float EnceladusSize, float EnceladusLocation, float TrlhysSize, float TrlhysLocation, float DioneSize, float DioneLocation, float RheaSize, float RheaLocation, float TitanSize, float TitanLocation, float LapetusSize, float LapetusLocation)
			SaturnObejct(  SaturnSize * SizeScaler,   Saturn_distance*DistanceScaler,   saturnmimasmooonSize*SizeScaler,   saturnmimasmooonDistance*DistanceScaler,   saturnenceladusmoonSize*SizeScaler,   saturnenceladusmoonDistance*DistanceScaler,   saturntrlhysmoonSize*SizeScaler,   saturntrlhysmoonDistance*DistanceScaler,   saturndionemoonSize*SizeScaler,   saturndionemoonDistance*DistanceScaler,   saturnrheamoonSize*SizeScaler,   saturnrheamoonDistance*DistanceScaler,   saturntitanmoonSize*SizeScaler,   saturntitanmoonDistance*DistanceScaler,   saturnlapetusmoonSize*SizeScaler,   saturnlapetusmoonDistance*DistanceScaler);

			// Saturn Ring
			//void SaturnRing(float RingOuterRadious, float RingLocation, float Eccentricity, float RingInnerRadious, float SideSmooth)
			SaturnRing(  SaturnRingOuterRadius * SizeScaler,   Saturn_distance*DistanceScaler ,   1.0,   SaturnRingInnerRadius * SizeScaler,   100);

			// Sun Object
			//SunObject(float SunSize)
			SunObject( SunSize * SizeScaler);

			//Mercury Object
			//MercuryObject(float MercurySize, float MercuryLocation)
			MercuryObject(MercurySize*SizeScaler, mercurydistance*DistanceScaler);

			// Venus Object
			//VenusObject(float VenusSize, float VenusLocation)
			VenusObject(  VenusSize * SizeScaler,   Venus_distance*DistanceScaler);

			// Earth Object
			//EarthObject(float EarthSize, float EarthLocation, float MoonSize, float Moon_Location)
			EarthObject(  EarthSize * SizeScaler,   Earth_distance*DistanceScaler,   earthMoonSize*SizeScaler,   earthMoonDistance*DistanceScaler);

			//Mars Object
			//MarsObject(float MarsSize, float MarsLocation, float PhobosSize, float PhobosLocation , float DiemosSize , float DiemosLocaion)
			MarsObject(  MarsSize * SizeScaler,   Mars_distance*DistanceScaler,   marsdiemosmoonSize* SizeScaler,   marsdiemosmoonDistance * DistanceScaler,   marsphobosmoonSize *SizeScaler,   marsphobosmoonDistance* DistanceScaler);

			// Jupiter Object
			//JupiterObject(float JubiterSize, float JubiterLocation, float IoSize, float IoLocation, float EuropaSize, float EuropaLocation, float GanymedeSize, float GanymedeLocation, float CallistoSize, float CallistoLocation)
			JupiterObject(  JupitureSize * SizeScaler,   Jupiture_distance*DistanceScaler,   jupiteriomoonSize *SizeScaler,  jupiteriomoonDistance*DistanceScaler,   jupitereuropamoonSize * SizeScaler,   jupitereuropamoonDistance*DistanceScaler,   jupiterganymedemoonSize * SizeScaler,   jupiterganymedemoonDistance*DistanceScaler,   jupitercallistomoonSize  * SizeScaler,   jupitercallistomoonDistance*DistanceScaler);

			//Saturn Object
			//void SaturnObejct(float SaturnSize, float SaturnLocation, float MimasSize, float MimasLocation, float EnceladusSize, float EnceladusLocation, float TrlhysSize, float TrlhysLocation, float DioneSize, float DioneLocation, float RheaSize, float RheaLocation, float TitanSize, float TitanLocation, float LapetusSize, float LapetusLocation)
			//SaturnObejct(  SaturnSize * SizeScaler,   Saturn_distance*DistanceScaler,   saturnmimasmooonSize*SizeScaler,   saturnmimasmooonDistance*DistanceScaler,   saturnenceladusmoonSize*SizeScaler,   saturnenceladusmoonDistance*DistanceScaler,   saturntrlhysmoonSize*SizeScaler,   saturntrlhysmoonDistance*DistanceScaler,   saturndionemoonSize*SizeScaler,   saturndionemoonDistance*DistanceScaler,   saturnrheamoonSize*SizeScaler,   saturnrheamoonDistance*DistanceScaler,   saturntitanmoonSize*SizeScaler,   saturntitanmoonDistance*DistanceScaler,   saturnlapetusmoonSize*SizeScaler,   saturnlapetusmoonDistance*DistanceScaler);

			// Saturn Ring
			//void SaturnRing(float RingOuterRadious, float RingLocation, float Eccentricity, float RingInnerRadious, float SideSmooth)
			//SaturnRing(  SaturnRingOuterRadius * SizeScaler,   Saturn_distance*DistanceScaler ,   1.0,   SaturnRingInnerRadius * SizeScaler,   100);

			// Uranus Object
			//UranusObject(float UranusSize, float UranusLocation, float UmbrielSize, float UmbrielLocation, float TitaniaSize, float TitaniaLocation, float OberonSize, float OberonLocation)
			UranusObject(  UranusSize * SizeScaler,   Uranus_distance*DistanceScaler,   uranusumbrielmoonSize*SizeScaler,   uranusumbrielmoonDistance*DistanceScaler,   uranustitaniamoonSize*SizeScaler,   uranustitaniamoonDistance*DistanceScaler,   uranusoberonmoonSize*SizeScaler,   uranusoberonmoonDistance*DistanceScaler);

			// Nebtune Object
			//NebtuneObject(float NebtuneSize, float NebtuneLocation, float TritonSize, float TritonLocation )
			NebtuneObject(  NeptuneSize * SizeScaler,   Neptune_distance*DistanceScaler, nebtunemoontritonSize *SizeScaler, nebtunemoontritonDistance*DistanceScaler);

			//Pluto Object
			//PlutoObject(float PlutoSize, float PlutoLocation, float CharonSize, float CharonLocation)
			PlutoObject(  PlutoSize * SizeScaler,   Pluto_distance*DistanceScaler,   plutomooncharonSize*SizeScaler,   plutocmoonharonDistance*DistanceScaler);

		}
		// Uranus Object
		else if (WhichPlanet == Uranus)
		{


			//Caculate the earth postion
			long double UranusPos[3] = {Uranus_distance*DistanceScaler * cos(-uranus * M_PI / 180), 0, Uranus_distance*DistanceScaler * sin(-uranus * M_PI / 180)};
			//Caculate the Camera Position
			long double cameraPos[3] = {Uranus_distance*DistanceScaler * cos(-uranus * M_PI / 180), (5*SizeScaler), Uranus_distance*DistanceScaler * sin(-uranus * M_PI / 180)};
			//Setup the camear on the top of the moon pointing 
			gluLookAt(cameraPos[0], cameraPos[1], cameraPos[2], UranusPos[0], UranusPos[1], UranusPos[2]-(6*SizeScaler), 0, 0, -1);

			SetPointLight(GL_LIGHT1,0.0,0.0,0.0,1,1,.9);
			//SetMaterial(1,1,1,.2);
			//Saturn Object
			// Uranus Planet
			UranusObject(  UranusSize * SizeScaler,   Uranus_distance*DistanceScaler,   uranusumbrielmoonSize*SizeScaler,   uranusumbrielmoonDistance*DistanceScaler,   uranustitaniamoonSize*SizeScaler,   uranustitaniamoonDistance*DistanceScaler,   uranusoberonmoonSize*SizeScaler,   uranusoberonmoonDistance*DistanceScaler);

			// Space Object
			//SpaceObject(float SpaceSize)
			SpaceObject( 200000 );



			// Sun Object
			//SunObject(float SunSize)
			SunObject( SunSize * SizeScaler);

			//Mercury Object
			//MercuryObject(float MercurySize, float MercuryLocation)
			MercuryObject(MercurySize*SizeScaler, mercurydistance*DistanceScaler);

			// Venus Object
			//VenusObject(float VenusSize, float VenusLocation)
			VenusObject(  VenusSize * SizeScaler,   Venus_distance*DistanceScaler);

			// Earth Object
			//EarthObject(float EarthSize, float EarthLocation, float MoonSize, float Moon_Location)
			EarthObject(  EarthSize * SizeScaler,   Earth_distance*DistanceScaler,   earthMoonSize*SizeScaler,   earthMoonDistance*DistanceScaler);

			//Mars Object
			//MarsObject(float MarsSize, float MarsLocation, float PhobosSize, float PhobosLocation , float DiemosSize , float DiemosLocaion)
			MarsObject(  MarsSize * SizeScaler,   Mars_distance*DistanceScaler,   marsdiemosmoonSize* SizeScaler,   marsdiemosmoonDistance * DistanceScaler,   marsphobosmoonSize *SizeScaler,   marsphobosmoonDistance* DistanceScaler);

			// Jupiter Object
			//JupiterObject(float JubiterSize, float JubiterLocation, float IoSize, float IoLocation, float EuropaSize, float EuropaLocation, float GanymedeSize, float GanymedeLocation, float CallistoSize, float CallistoLocation)
			JupiterObject(  JupitureSize * SizeScaler,   Jupiture_distance*DistanceScaler,   jupiteriomoonSize *SizeScaler,  jupiteriomoonDistance*DistanceScaler,   jupitereuropamoonSize * SizeScaler,   jupitereuropamoonDistance*DistanceScaler,   jupiterganymedemoonSize * SizeScaler,   jupiterganymedemoonDistance*DistanceScaler,   jupitercallistomoonSize  * SizeScaler,   jupitercallistomoonDistance*DistanceScaler);

			//Saturn Object
			//void SaturnObejct(float SaturnSize, float SaturnLocation, float MimasSize, float MimasLocation, float EnceladusSize, float EnceladusLocation, float TrlhysSize, float TrlhysLocation, float DioneSize, float DioneLocation, float RheaSize, float RheaLocation, float TitanSize, float TitanLocation, float LapetusSize, float LapetusLocation)
			SaturnObejct(  SaturnSize * SizeScaler,   Saturn_distance*DistanceScaler,   saturnmimasmooonSize*SizeScaler,   saturnmimasmooonDistance*DistanceScaler,   saturnenceladusmoonSize*SizeScaler,   saturnenceladusmoonDistance*DistanceScaler,   saturntrlhysmoonSize*SizeScaler,   saturntrlhysmoonDistance*DistanceScaler,   saturndionemoonSize*SizeScaler,   saturndionemoonDistance*DistanceScaler,   saturnrheamoonSize*SizeScaler,   saturnrheamoonDistance*DistanceScaler,   saturntitanmoonSize*SizeScaler,   saturntitanmoonDistance*DistanceScaler,   saturnlapetusmoonSize*SizeScaler,   saturnlapetusmoonDistance*DistanceScaler);

			// Saturn Ring
			//void SaturnRing(float RingOuterRadious, float RingLocation, float Eccentricity, float RingInnerRadious, float SideSmooth)
			SaturnRing(  SaturnRingOuterRadius * SizeScaler,   Saturn_distance*DistanceScaler ,   1.0,   SaturnRingInnerRadius * SizeScaler,   100);

			// Uranus Object
			//UranusObject(float UranusSize, float UranusLocation, float UmbrielSize, float UmbrielLocation, float TitaniaSize, float TitaniaLocation, float OberonSize, float OberonLocation)
			//UranusObject(  UranusSize * SizeScaler,   Uranus_distance*DistanceScaler,   uranusumbrielmoonSize*SizeScaler,   uranusumbrielmoonDistance*DistanceScaler,   uranustitaniamoonSize*SizeScaler,   uranustitaniamoonDistance*DistanceScaler,   uranusoberonmoonSize*SizeScaler,   uranusoberonmoonDistance*DistanceScaler);

			// Nebtune Object
			//NebtuneObject(float NebtuneSize, float NebtuneLocation, float TritonSize, float TritonLocation )
			NebtuneObject(  NeptuneSize * SizeScaler,   Neptune_distance*DistanceScaler, nebtunemoontritonSize *SizeScaler, nebtunemoontritonDistance*DistanceScaler);

			//Pluto Object
			//PlutoObject(float PlutoSize, float PlutoLocation, float CharonSize, float CharonLocation)
			PlutoObject(  PlutoSize * SizeScaler,   Pluto_distance*DistanceScaler,   plutomooncharonSize*SizeScaler,   plutocmoonharonDistance*DistanceScaler);


		}
		// Nebtune Object
		else if (WhichPlanet == Nebtune)
		{


			//Caculate the earth postion
			GLfloat NebtunePos[3] = {Neptune_distance*DistanceScaler * cos(-nebtune * M_PI / 180), 0, Neptune_distance*DistanceScaler * sin(-nebtune * M_PI / 180)};
			//Caculate the Camera Position
			GLfloat cameraPos[3] = {Neptune_distance*DistanceScaler * cos(-nebtune * M_PI / 180), (7*SizeScaler), Neptune_distance*DistanceScaler * sin(-nebtune * M_PI / 180)};
			//Setup the camear on the top of the moon pointing 
			gluLookAt(cameraPos[0], cameraPos[1], cameraPos[2], NebtunePos[0], NebtunePos[1], NebtunePos[2]-(6*SizeScaler), 0, 0, -1);

			SetPointLight(GL_LIGHT1,0.0,0.0,0.0,1,1,.9);
			//SetMaterial(1,1,1,.2);

			// Nebtun Planet
			//NebtuneObject(float NebtuneSize, float NebtuneLocation, float TritonSize, float TritonLocation )
			NebtuneObject(  NeptuneSize * SizeScaler,   Neptune_distance*DistanceScaler, nebtunemoontritonSize *SizeScaler, nebtunemoontritonDistance*DistanceScaler);


			// Space Object
			//SpaceObject(float SpaceSize)
			SpaceObject( 200000 );

			// Sun Object
			//SunObject(float SunSize)
			SunObject( SunSize * SizeScaler);

			//Mercury Object
			//MercuryObject(float MercurySize, float MercuryLocation)
			MercuryObject(MercurySize*SizeScaler, mercurydistance*DistanceScaler);

			// Venus Object
			//VenusObject(float VenusSize, float VenusLocation)
			VenusObject(  VenusSize * SizeScaler,   Venus_distance*DistanceScaler);

			// Earth Object
			//EarthObject(float EarthSize, float EarthLocation, float MoonSize, float Moon_Location)
			EarthObject(  EarthSize * SizeScaler,   Earth_distance*DistanceScaler,   earthMoonSize*SizeScaler,   earthMoonDistance*DistanceScaler);

			//Mars Object
			//MarsObject(float MarsSize, float MarsLocation, float PhobosSize, float PhobosLocation , float DiemosSize , float DiemosLocaion)
			MarsObject(  MarsSize * SizeScaler,   Mars_distance*DistanceScaler,   marsdiemosmoonSize* SizeScaler,   marsdiemosmoonDistance * DistanceScaler,   marsphobosmoonSize *SizeScaler,   marsphobosmoonDistance* DistanceScaler);

			// Jupiter Object
			//JupiterObject(float JubiterSize, float JubiterLocation, float IoSize, float IoLocation, float EuropaSize, float EuropaLocation, float GanymedeSize, float GanymedeLocation, float CallistoSize, float CallistoLocation)
			JupiterObject(  JupitureSize * SizeScaler,   Jupiture_distance*DistanceScaler,   jupiteriomoonSize *SizeScaler,  jupiteriomoonDistance*DistanceScaler,   jupitereuropamoonSize * SizeScaler,   jupitereuropamoonDistance*DistanceScaler,   jupiterganymedemoonSize * SizeScaler,   jupiterganymedemoonDistance*DistanceScaler,   jupitercallistomoonSize  * SizeScaler,   jupitercallistomoonDistance*DistanceScaler);

			//Saturn Object
			//void SaturnObejct(float SaturnSize, float SaturnLocation, float MimasSize, float MimasLocation, float EnceladusSize, float EnceladusLocation, float TrlhysSize, float TrlhysLocation, float DioneSize, float DioneLocation, float RheaSize, float RheaLocation, float TitanSize, float TitanLocation, float LapetusSize, float LapetusLocation)
			SaturnObejct(  SaturnSize * SizeScaler,   Saturn_distance*DistanceScaler,   saturnmimasmooonSize*SizeScaler,   saturnmimasmooonDistance*DistanceScaler,   saturnenceladusmoonSize*SizeScaler,   saturnenceladusmoonDistance*DistanceScaler,   saturntrlhysmoonSize*SizeScaler,   saturntrlhysmoonDistance*DistanceScaler,   saturndionemoonSize*SizeScaler,   saturndionemoonDistance*DistanceScaler,   saturnrheamoonSize*SizeScaler,   saturnrheamoonDistance*DistanceScaler,   saturntitanmoonSize*SizeScaler,   saturntitanmoonDistance*DistanceScaler,   saturnlapetusmoonSize*SizeScaler,   saturnlapetusmoonDistance*DistanceScaler);

			// Saturn Ring
			//void SaturnRing(float RingOuterRadious, float RingLocation, float Eccentricity, float RingInnerRadious, float SideSmooth)
			SaturnRing(  SaturnRingOuterRadius * SizeScaler,   Saturn_distance*DistanceScaler ,   1.0,   SaturnRingInnerRadius * SizeScaler,   100);

			// Uranus Object
			//UranusObject(float UranusSize, float UranusLocation, float UmbrielSize, float UmbrielLocation, float TitaniaSize, float TitaniaLocation, float OberonSize, float OberonLocation)
			UranusObject(  UranusSize * SizeScaler,   Uranus_distance*DistanceScaler,   uranusumbrielmoonSize*SizeScaler,   uranusumbrielmoonDistance*DistanceScaler,   uranustitaniamoonSize*SizeScaler,   uranustitaniamoonDistance*DistanceScaler,   uranusoberonmoonSize*SizeScaler,   uranusoberonmoonDistance*DistanceScaler);

			// Nebtune Object
			//NebtuneObject(float NebtuneSize, float NebtuneLocation, float TritonSize, float TritonLocation )
			//NebtuneObject(  NeptuneSize * SizeScaler,   Neptune_distance*DistanceScaler, nebtunemoontritonSize *SizeScaler, nebtunemoontritonDistance*DistanceScaler);

			//Pluto Object
			//PlutoObject(float PlutoSize, float PlutoLocation, float CharonSize, float CharonLocation)
			PlutoObject(  PlutoSize * SizeScaler,   Pluto_distance*DistanceScaler,   plutomooncharonSize*SizeScaler,   plutocmoonharonDistance*DistanceScaler);

		}
		//Pluto Object
		else if (WhichPlanet == Pluto)
		{


			//Caculate the earth postion
			GLfloat NebtunePos[3] = {Pluto_distance*DistanceScaler * cos(-pluto * M_PI / 180), 0, Pluto_distance*DistanceScaler * sin(-pluto * M_PI / 180)};
			//Caculate the Camera Position
			GLfloat cameraPos[3] = {Pluto_distance*DistanceScaler * cos(-pluto * M_PI / 180), (7*SizeScaler), Pluto_distance*DistanceScaler * sin(-pluto * M_PI / 180)};
			//Setup the camear on the top of the moon pointing 
			gluLookAt(cameraPos[0], cameraPos[1], cameraPos[2], NebtunePos[0], NebtunePos[1], NebtunePos[2]-(6*SizeScaler), 0, 0, -1);

			SetPointLight(GL_LIGHT1,0.0,0.0,0.0,1,1,.9);
			//SetMaterial(1,1,1,.2);

			//Pluto Object
			PlutoObject(  PlutoSize * SizeScaler,   Pluto_distance*DistanceScaler,   plutomooncharonSize*SizeScaler,   plutocmoonharonDistance*DistanceScaler);

			// Space Object
			//SpaceObject(float SpaceSize)
			SpaceObject( 200000 );

			// Sun Object
			//SunObject(float SunSize)
			SunObject( SunSize * SizeScaler);

			//Mercury Object
			//MercuryObject(float MercurySize, float MercuryLocation)
			MercuryObject(MercurySize*SizeScaler, mercurydistance*DistanceScaler);

			// Venus Object
			//VenusObject(float VenusSize, float VenusLocation)
			VenusObject(  VenusSize * SizeScaler,   Venus_distance*DistanceScaler);

			// Earth Object
			//EarthObject(float EarthSize, float EarthLocation, float MoonSize, float Moon_Location)
			EarthObject(  EarthSize * SizeScaler,   Earth_distance*DistanceScaler,   earthMoonSize*SizeScaler,   earthMoonDistance*DistanceScaler);

			//Mars Object
			//MarsObject(float MarsSize, float MarsLocation, float PhobosSize, float PhobosLocation , float DiemosSize , float DiemosLocaion)
			MarsObject(  MarsSize * SizeScaler,   Mars_distance*DistanceScaler,   marsdiemosmoonSize* SizeScaler,   marsdiemosmoonDistance * DistanceScaler,   marsphobosmoonSize *SizeScaler,   marsphobosmoonDistance* DistanceScaler);

			// Jupiter Object
			//JupiterObject(float JubiterSize, float JubiterLocation, float IoSize, float IoLocation, float EuropaSize, float EuropaLocation, float GanymedeSize, float GanymedeLocation, float CallistoSize, float CallistoLocation)
			JupiterObject(  JupitureSize * SizeScaler,   Jupiture_distance*DistanceScaler,   jupiteriomoonSize *SizeScaler,  jupiteriomoonDistance*DistanceScaler,   jupitereuropamoonSize * SizeScaler,   jupitereuropamoonDistance*DistanceScaler,   jupiterganymedemoonSize * SizeScaler,   jupiterganymedemoonDistance*DistanceScaler,   jupitercallistomoonSize  * SizeScaler,   jupitercallistomoonDistance*DistanceScaler);

			//Saturn Object
			//void SaturnObejct(float SaturnSize, float SaturnLocation, float MimasSize, float MimasLocation, float EnceladusSize, float EnceladusLocation, float TrlhysSize, float TrlhysLocation, float DioneSize, float DioneLocation, float RheaSize, float RheaLocation, float TitanSize, float TitanLocation, float LapetusSize, float LapetusLocation)
			SaturnObejct(  SaturnSize * SizeScaler,   Saturn_distance*DistanceScaler,   saturnmimasmooonSize*SizeScaler,   saturnmimasmooonDistance*DistanceScaler,   saturnenceladusmoonSize*SizeScaler,   saturnenceladusmoonDistance*DistanceScaler,   saturntrlhysmoonSize*SizeScaler,   saturntrlhysmoonDistance*DistanceScaler,   saturndionemoonSize*SizeScaler,   saturndionemoonDistance*DistanceScaler,   saturnrheamoonSize*SizeScaler,   saturnrheamoonDistance*DistanceScaler,   saturntitanmoonSize*SizeScaler,   saturntitanmoonDistance*DistanceScaler,   saturnlapetusmoonSize*SizeScaler,   saturnlapetusmoonDistance*DistanceScaler);

			// Saturn Ring
			//void SaturnRing(float RingOuterRadious, float RingLocation, float Eccentricity, float RingInnerRadious, float SideSmooth)
			SaturnRing(  SaturnRingOuterRadius * SizeScaler,   Saturn_distance*DistanceScaler ,   1.0,   SaturnRingInnerRadius * SizeScaler,   100);

			// Uranus Object
			//UranusObject(float UranusSize, float UranusLocation, float UmbrielSize, float UmbrielLocation, float TitaniaSize, float TitaniaLocation, float OberonSize, float OberonLocation)
			UranusObject(  UranusSize * SizeScaler,   Uranus_distance*DistanceScaler,   uranusumbrielmoonSize*SizeScaler,   uranusumbrielmoonDistance*DistanceScaler,   uranustitaniamoonSize*SizeScaler,   uranustitaniamoonDistance*DistanceScaler,   uranusoberonmoonSize*SizeScaler,   uranusoberonmoonDistance*DistanceScaler);

			// Nebtune Object
			//NebtuneObject(float NebtuneSize, float NebtuneLocation, float TritonSize, float TritonLocation )
			NebtuneObject(  NeptuneSize * SizeScaler,   Neptune_distance*DistanceScaler, nebtunemoontritonSize *SizeScaler, nebtunemoontritonDistance*DistanceScaler);

			//Pluto Object
			//PlutoObject(float PlutoSize, float PlutoLocation, float CharonSize, float CharonLocation)
			//PlutoObject(  PlutoSize * SizeScaler,   Pluto_distance*DistanceScaler,   plutomooncharonSize*SizeScaler,   plutocmoonharonDistance*DistanceScaler);

		}
		else if (WhichPlanet == Mercury)
		{

			//Caculate the earth postion
			GLfloat MercuryPos[3] = {mercurydistance*DistanceScaler * cos(-mercury * M_PI / 180), 0, mercurydistance*DistanceScaler * sin(-mercury * M_PI / 180)};
			//Caculate the Camera Position
			GLfloat cameraPos[3] = {mercurydistance*DistanceScaler * cos(-mercury * M_PI / 180), (.5*SizeScaler), mercurydistance*DistanceScaler * sin(-mercury * M_PI / 180)};
			//Setup the camear on the top of the moon pointing 
			gluLookAt(cameraPos[0], cameraPos[1], cameraPos[2], MercuryPos[0], MercuryPos[1], MercuryPos[2]-(1*SizeScaler), 0, 0, -1);
			// Jupiter Planet

			SetPointLight(GL_LIGHT1,0.0,0.0,0.0,1,1,.9);

			MercuryObject(MercurySize*SizeScaler, mercurydistance*DistanceScaler);

			// Space Object
			//SpaceObject(float SpaceSize)
			SpaceObject( 200000 );

			// Sun Object
			//SunObject(float SunSize)
			SunObject( SunSize * SizeScaler);

			//Mercury Object
			//MercuryObject(float MercurySize, float MercuryLocation)
			//MercuryObject(MercurySize*SizeScaler, mercurydistance*DistanceScaler);

			// Venus Object
			//VenusObject(float VenusSize, float VenusLocation)
			VenusObject(  VenusSize * SizeScaler,   Venus_distance*DistanceScaler);

			// Earth Object
			//EarthObject(float EarthSize, float EarthLocation, float MoonSize, float Moon_Location)
			EarthObject(  EarthSize * SizeScaler,   Earth_distance*DistanceScaler,   earthMoonSize*SizeScaler,   earthMoonDistance*DistanceScaler);

			//Mars Object
			//MarsObject(float MarsSize, float MarsLocation, float PhobosSize, float PhobosLocation , float DiemosSize , float DiemosLocaion)
			MarsObject(  MarsSize * SizeScaler,   Mars_distance*DistanceScaler,   marsdiemosmoonSize* SizeScaler,   marsdiemosmoonDistance * DistanceScaler,   marsphobosmoonSize *SizeScaler,   marsphobosmoonDistance* DistanceScaler);

			// Jupiter Object
			//JupiterObject(float JubiterSize, float JubiterLocation, float IoSize, float IoLocation, float EuropaSize, float EuropaLocation, float GanymedeSize, float GanymedeLocation, float CallistoSize, float CallistoLocation)
			JupiterObject(  JupitureSize * SizeScaler,   Jupiture_distance*DistanceScaler,   jupiteriomoonSize *SizeScaler,  jupiteriomoonDistance*DistanceScaler,   jupitereuropamoonSize * SizeScaler,   jupitereuropamoonDistance*DistanceScaler,   jupiterganymedemoonSize * SizeScaler,   jupiterganymedemoonDistance*DistanceScaler,   jupitercallistomoonSize  * SizeScaler,   jupitercallistomoonDistance*DistanceScaler);

			//Saturn Object
			//void SaturnObejct(float SaturnSize, float SaturnLocation, float MimasSize, float MimasLocation, float EnceladusSize, float EnceladusLocation, float TrlhysSize, float TrlhysLocation, float DioneSize, float DioneLocation, float RheaSize, float RheaLocation, float TitanSize, float TitanLocation, float LapetusSize, float LapetusLocation)
			SaturnObejct(  SaturnSize * SizeScaler,   Saturn_distance*DistanceScaler,   saturnmimasmooonSize*SizeScaler,   saturnmimasmooonDistance*DistanceScaler,   saturnenceladusmoonSize*SizeScaler,   saturnenceladusmoonDistance*DistanceScaler,   saturntrlhysmoonSize*SizeScaler,   saturntrlhysmoonDistance*DistanceScaler,   saturndionemoonSize*SizeScaler,   saturndionemoonDistance*DistanceScaler,   saturnrheamoonSize*SizeScaler,   saturnrheamoonDistance*DistanceScaler,   saturntitanmoonSize*SizeScaler,   saturntitanmoonDistance*DistanceScaler,   saturnlapetusmoonSize*SizeScaler,   saturnlapetusmoonDistance*DistanceScaler);

			// Saturn Ring
			//void SaturnRing(float RingOuterRadious, float RingLocation, float Eccentricity, float RingInnerRadious, float SideSmooth)
			SaturnRing(  SaturnRingOuterRadius * SizeScaler,   Saturn_distance*DistanceScaler ,   1.0,   SaturnRingInnerRadius * SizeScaler,   100);

			// Uranus Object
			//UranusObject(float UranusSize, float UranusLocation, float UmbrielSize, float UmbrielLocation, float TitaniaSize, float TitaniaLocation, float OberonSize, float OberonLocation)
			UranusObject(  UranusSize * SizeScaler,   Uranus_distance*DistanceScaler,   uranusumbrielmoonSize*SizeScaler,   uranusumbrielmoonDistance*DistanceScaler,   uranustitaniamoonSize*SizeScaler,   uranustitaniamoonDistance*DistanceScaler,   uranusoberonmoonSize*SizeScaler,   uranusoberonmoonDistance*DistanceScaler);

			// Nebtune Object
			//NebtuneObject(float NebtuneSize, float NebtuneLocation, float TritonSize, float TritonLocation )
			NebtuneObject(  NeptuneSize * SizeScaler,   Neptune_distance*DistanceScaler, nebtunemoontritonSize *SizeScaler, nebtunemoontritonDistance*DistanceScaler);

			//Pluto Object
			//PlutoObject(float PlutoSize, float PlutoLocation, float CharonSize, float CharonLocation)
			PlutoObject(  PlutoSize * SizeScaler,   Pluto_distance*DistanceScaler,   plutomooncharonSize*SizeScaler,   plutocmoonharonDistance*DistanceScaler);



		}
	}



	else if (WhichCamera == Planet)

	{
		glDisable( GL_FOG );

		glEnable( GL_NORMALIZE );

		// Sun Light			
		glEnable( GL_LIGHTING );
		glShadeModel(GL_SMOOTH);

		if (LinesEnabled == 0 || LinesEnabled == 1 )
		{
			PlanetsOrbits(0,0,0,0,0,0,0,0,0);
		}
		DistanceScaler = 3500;
		// Jupiter Object
		if (WhichPlanet == Jupiter)
		{
			//Caculate the earth postion
			GLfloat JupiterPos[3] = {Jupiture_distance*DistanceScaler * cos(-jupiter * M_PI / 180), 0, Jupiture_distance*DistanceScaler * sin(-jupiter * M_PI / 180)};
			//Caculate the Camera Position
			GLfloat cameraPos[3] = {Jupiture_distance*DistanceScaler * cos(-jupiter * M_PI / 180), (16*SizeScaler), Jupiture_distance*DistanceScaler * sin(-jupiter * M_PI / 180)};
			//Setup the camear on the top of the moon pointing 
			gluLookAt(cameraPos[0], cameraPos[1], cameraPos[2]+(20*SizeScaler), JupiterPos[0], JupiterPos[1], JupiterPos[2]-(20*SizeScaler), 0, 0, -1);
			// Jupiter Planet

			SetPointLight(GL_LIGHT1,0.0,0.0,0.0,1,1,.9);
			//SetMaterial(1,1,1,.2);
			JupiterObject(  JupitureSize * SizeScaler,   Jupiture_distance*DistanceScaler,   jupiteriomoonSize *SizeScaler,  jupiteriomoonDistance*DistanceScaler,   jupitereuropamoonSize * SizeScaler,   jupitereuropamoonDistance*DistanceScaler,   jupiterganymedemoonSize * SizeScaler,   jupiterganymedemoonDistance*DistanceScaler,   jupitercallistomoonSize  * SizeScaler,   jupitercallistomoonDistance*DistanceScaler);


			//JupiterObject(  10,   1400,   0.6,  12,   .5,   14,   .8,   16,   .8,   20);
			// Space Object
			//SpaceObject(float SpaceSize)
			SpaceObject( 200000 );

			// Sun Object
			//SunObject(float SunSize)
			SunObject( SunSize * SizeScaler);

			//Mercury Object
			//MercuryObject(float MercurySize, float MercuryLocation)
			MercuryObject(MercurySize*SizeScaler, mercurydistance*DistanceScaler);

			// Venus Object
			//VenusObject(float VenusSize, float VenusLocation)
			VenusObject(  VenusSize * SizeScaler,   Venus_distance*DistanceScaler);

			// Earth Object
			//EarthObject(float EarthSize, float EarthLocation, float MoonSize, float Moon_Location)
			EarthObject(  EarthSize * SizeScaler,   Earth_distance*DistanceScaler,   earthMoonSize*SizeScaler,   earthMoonDistance*DistanceScaler);

			//Mars Object
			//MarsObject(float MarsSize, float MarsLocation, float PhobosSize, float PhobosLocation , float DiemosSize , float DiemosLocaion)
			MarsObject(  MarsSize * SizeScaler,   Mars_distance*DistanceScaler,   marsdiemosmoonSize* SizeScaler,   marsdiemosmoonDistance * DistanceScaler,   marsphobosmoonSize *SizeScaler,   marsphobosmoonDistance* DistanceScaler);

			// Jupiter Object
			//JupiterObject(float JubiterSize, float JubiterLocation, float IoSize, float IoLocation, float EuropaSize, float EuropaLocation, float GanymedeSize, float GanymedeLocation, float CallistoSize, float CallistoLocation)
			//JupiterObject(  JupitureSize * SizeScaler,   Jupiture_distance*DistanceScaler,   jupiteriomoonSize *SizeScaler,  jupiteriomoonDistance*DistanceScaler,   jupitereuropamoonSize * SizeScaler,   jupitereuropamoonDistance*DistanceScaler,   jupiterganymedemoonSize * SizeScaler,   jupiterganymedemoonDistance*DistanceScaler,   jupitercallistomoonSize  * SizeScaler,   jupitercallistomoonDistance*DistanceScaler);

			//Saturn Object
			//void SaturnObejct(float SaturnSize, float SaturnLocation, float MimasSize, float MimasLocation, float EnceladusSize, float EnceladusLocation, float TrlhysSize, float TrlhysLocation, float DioneSize, float DioneLocation, float RheaSize, float RheaLocation, float TitanSize, float TitanLocation, float LapetusSize, float LapetusLocation)
			SaturnObejct(  SaturnSize * SizeScaler,   Saturn_distance*DistanceScaler,   saturnmimasmooonSize*SizeScaler,   saturnmimasmooonDistance*DistanceScaler,   saturnenceladusmoonSize*SizeScaler,   saturnenceladusmoonDistance*DistanceScaler,   saturntrlhysmoonSize*SizeScaler,   saturntrlhysmoonDistance*DistanceScaler,   saturndionemoonSize*SizeScaler,   saturndionemoonDistance*DistanceScaler,   saturnrheamoonSize*SizeScaler,   saturnrheamoonDistance*DistanceScaler,   saturntitanmoonSize*SizeScaler,   saturntitanmoonDistance*DistanceScaler,   saturnlapetusmoonSize*SizeScaler,   saturnlapetusmoonDistance*DistanceScaler);

			// Saturn Ring
			//void SaturnRing(float RingOuterRadious, float RingLocation, float Eccentricity, float RingInnerRadious, float SideSmooth)
			SaturnRing(  SaturnRingOuterRadius * SizeScaler,   Saturn_distance*DistanceScaler ,   1.0,   SaturnRingInnerRadius * SizeScaler,   100);

			// Uranus Object
			//UranusObject(float UranusSize, float UranusLocation, float UmbrielSize, float UmbrielLocation, float TitaniaSize, float TitaniaLocation, float OberonSize, float OberonLocation)
			UranusObject(  UranusSize * SizeScaler,   Uranus_distance*DistanceScaler,   uranusumbrielmoonSize*SizeScaler,   uranusumbrielmoonDistance*DistanceScaler,   uranustitaniamoonSize*SizeScaler,   uranustitaniamoonDistance*DistanceScaler,   uranusoberonmoonSize*SizeScaler,   uranusoberonmoonDistance*DistanceScaler);

			// Nebtune Object
			//NebtuneObject(float NebtuneSize, float NebtuneLocation, float TritonSize, float TritonLocation )
			NebtuneObject(  NeptuneSize * SizeScaler,   Neptune_distance*DistanceScaler, nebtunemoontritonSize *SizeScaler, nebtunemoontritonDistance*DistanceScaler);

			//Pluto Object
			//PlutoObject(float PlutoSize, float PlutoLocation, float CharonSize, float CharonLocation)
			PlutoObject(  PlutoSize * SizeScaler,   Pluto_distance*DistanceScaler,   plutomooncharonSize*SizeScaler,   plutocmoonharonDistance*DistanceScaler);

		}
		// Venus Object
		else if (WhichPlanet == Venus)
		{


			//Caculate the earth postion
			GLfloat VenusPos[3] = {Venus_distance*DistanceScaler * cos(-venus * M_PI / 180), 0, Venus_distance*DistanceScaler * sin(-venus * M_PI / 180)};
			//Caculate the Camera Position
			GLfloat cameraPos[3] = {Venus_distance*DistanceScaler * cos(-venus * M_PI / 180), (1.4*SizeScaler), Venus_distance*DistanceScaler * sin(-venus * M_PI / 180)};
			//Setup the camear on the top of the moon pointing 
			gluLookAt(cameraPos[0], cameraPos[1], cameraPos[2]+(2.2*SizeScaler), VenusPos[0], VenusPos[1], VenusPos[2]-(5*SizeScaler), 0, 0, -1);
			// Jupiter Planet

			SetPointLight(GL_LIGHT1,0.0,0.0,0.0,1,1,.9);
			//SetMaterial(1,1,1,.2);
			VenusObject(  VenusSize * SizeScaler,   Venus_distance*DistanceScaler);

			// Space Object
			//SpaceObject(float SpaceSize)
			SpaceObject( 200000 );

			// Sun Object
			//SunObject(float SunSize)
			SunObject( SunSize * SizeScaler);

			//Mercury Object
			//MercuryObject(float MercurySize, float MercuryLocation)
			MercuryObject(MercurySize*SizeScaler, mercurydistance*DistanceScaler);

			// Venus Object
			//VenusObject(float VenusSize, float VenusLocation)
			//VenusObject(  VenusSize * SizeScaler,   Venus_distance*DistanceScaler);

			// Earth Object
			//EarthObject(float EarthSize, float EarthLocation, float MoonSize, float Moon_Location)
			EarthObject(  EarthSize * SizeScaler,   Earth_distance*DistanceScaler,   earthMoonSize*SizeScaler,   earthMoonDistance*DistanceScaler);

			//Mars Object
			//MarsObject(float MarsSize, float MarsLocation, float PhobosSize, float PhobosLocation , float DiemosSize , float DiemosLocaion)
			MarsObject(  MarsSize * SizeScaler,   Mars_distance*DistanceScaler,   marsdiemosmoonSize* SizeScaler,   marsdiemosmoonDistance * DistanceScaler,   marsphobosmoonSize *SizeScaler,   marsphobosmoonDistance* DistanceScaler);

			// Jupiter Object
			//JupiterObject(float JubiterSize, float JubiterLocation, float IoSize, float IoLocation, float EuropaSize, float EuropaLocation, float GanymedeSize, float GanymedeLocation, float CallistoSize, float CallistoLocation)
			JupiterObject(  JupitureSize * SizeScaler,   Jupiture_distance*DistanceScaler,   jupiteriomoonSize *SizeScaler,  jupiteriomoonDistance*DistanceScaler,   jupitereuropamoonSize * SizeScaler,   jupitereuropamoonDistance*DistanceScaler,   jupiterganymedemoonSize * SizeScaler,   jupiterganymedemoonDistance*DistanceScaler,   jupitercallistomoonSize  * SizeScaler,   jupitercallistomoonDistance*DistanceScaler);

			//Saturn Object
			//void SaturnObejct(float SaturnSize, float SaturnLocation, float MimasSize, float MimasLocation, float EnceladusSize, float EnceladusLocation, float TrlhysSize, float TrlhysLocation, float DioneSize, float DioneLocation, float RheaSize, float RheaLocation, float TitanSize, float TitanLocation, float LapetusSize, float LapetusLocation)
			SaturnObejct(  SaturnSize * SizeScaler,   Saturn_distance*DistanceScaler,   saturnmimasmooonSize*SizeScaler,   saturnmimasmooonDistance*DistanceScaler,   saturnenceladusmoonSize*SizeScaler,   saturnenceladusmoonDistance*DistanceScaler,   saturntrlhysmoonSize*SizeScaler,   saturntrlhysmoonDistance*DistanceScaler,   saturndionemoonSize*SizeScaler,   saturndionemoonDistance*DistanceScaler,   saturnrheamoonSize*SizeScaler,   saturnrheamoonDistance*DistanceScaler,   saturntitanmoonSize*SizeScaler,   saturntitanmoonDistance*DistanceScaler,   saturnlapetusmoonSize*SizeScaler,   saturnlapetusmoonDistance*DistanceScaler);

			// Saturn Ring
			//void SaturnRing(float RingOuterRadious, float RingLocation, float Eccentricity, float RingInnerRadious, float SideSmooth)
			SaturnRing(  SaturnRingOuterRadius * SizeScaler,   Saturn_distance*DistanceScaler ,   1.0,   SaturnRingInnerRadius * SizeScaler,   100);

			// Uranus Object
			//UranusObject(float UranusSize, float UranusLocation, float UmbrielSize, float UmbrielLocation, float TitaniaSize, float TitaniaLocation, float OberonSize, float OberonLocation)
			UranusObject(  UranusSize * SizeScaler,   Uranus_distance*DistanceScaler,   uranusumbrielmoonSize*SizeScaler,   uranusumbrielmoonDistance*DistanceScaler,   uranustitaniamoonSize*SizeScaler,   uranustitaniamoonDistance*DistanceScaler,   uranusoberonmoonSize*SizeScaler,   uranusoberonmoonDistance*DistanceScaler);

			// Nebtune Object
			//NebtuneObject(float NebtuneSize, float NebtuneLocation, float TritonSize, float TritonLocation )
			NebtuneObject(  NeptuneSize * SizeScaler,   Neptune_distance*DistanceScaler, nebtunemoontritonSize *SizeScaler, nebtunemoontritonDistance*DistanceScaler);

			//Pluto Object
			//PlutoObject(float PlutoSize, float PlutoLocation, float CharonSize, float CharonLocation)
			PlutoObject(  PlutoSize * SizeScaler,   Pluto_distance*DistanceScaler,   plutomooncharonSize*SizeScaler,   plutocmoonharonDistance*DistanceScaler);

		}

		// Earth Object
		else if (WhichPlanet == Earth)
		{

			//Caculate the earth postion
			GLfloat JupiterPos[3] = {Earth_distance*DistanceScaler * cos(-earth * M_PI / 180), 0, Earth_distance*DistanceScaler * sin(-earth * M_PI / 180)};
			//Caculate the Camera Position
			GLfloat cameraPos[3] = {Earth_distance*DistanceScaler * cos(-earth * M_PI / 180), (1.5*SizeScaler), Earth_distance*DistanceScaler * sin(-earth * M_PI / 180)};
			//Setup the camear on the top of the moon pointing 
			gluLookAt(cameraPos[0], cameraPos[1], cameraPos[2]+ (2*SizeScaler), JupiterPos[0], JupiterPos[1], JupiterPos[2]-(2.2*SizeScaler), 0, 0, -1);
			// Jupiter Planet

			SetPointLight(GL_LIGHT1,0.0,0.0,0.0,1,1,.9);
			// Earth Object
			EarthObject(  EarthSize * SizeScaler,   Earth_distance*DistanceScaler,   earthMoonSize*SizeScaler,   earthMoonDistance*DistanceScaler);


			// Space Object
			//SpaceObject(float SpaceSize)
			SpaceObject( 200000 );

			// Sun Object
			//SunObject(float SunSize)
			SunObject( SunSize * SizeScaler);

			//Mercury Object
			//MercuryObject(float MercurySize, float MercuryLocation)
			MercuryObject(MercurySize*SizeScaler, mercurydistance*DistanceScaler);

			// Venus Object
			//VenusObject(float VenusSize, float VenusLocation)
			VenusObject(  VenusSize * SizeScaler,   Venus_distance*DistanceScaler);

			// Earth Object
			//EarthObject(float EarthSize, float EarthLocation, float MoonSize, float Moon_Location)
			//EarthObject(  EarthSize * SizeScaler,   Earth_distance*DistanceScaler,   earthMoonSize*SizeScaler,   earthMoonDistance*DistanceScaler);

			//Mars Object
			//MarsObject(float MarsSize, float MarsLocation, float PhobosSize, float PhobosLocation , float DiemosSize , float DiemosLocaion)
			MarsObject(  MarsSize * SizeScaler,   Mars_distance*DistanceScaler,   marsdiemosmoonSize* SizeScaler,   marsdiemosmoonDistance * DistanceScaler,   marsphobosmoonSize *SizeScaler,   marsphobosmoonDistance* DistanceScaler);

			// Jupiter Object
			//JupiterObject(float JubiterSize, float JubiterLocation, float IoSize, float IoLocation, float EuropaSize, float EuropaLocation, float GanymedeSize, float GanymedeLocation, float CallistoSize, float CallistoLocation)
			JupiterObject(  JupitureSize * SizeScaler,   Jupiture_distance*DistanceScaler,   jupiteriomoonSize *SizeScaler,  jupiteriomoonDistance*DistanceScaler,   jupitereuropamoonSize * SizeScaler,   jupitereuropamoonDistance*DistanceScaler,   jupiterganymedemoonSize * SizeScaler,   jupiterganymedemoonDistance*DistanceScaler,   jupitercallistomoonSize  * SizeScaler,   jupitercallistomoonDistance*DistanceScaler);

			//Saturn Object
			//void SaturnObejct(float SaturnSize, float SaturnLocation, float MimasSize, float MimasLocation, float EnceladusSize, float EnceladusLocation, float TrlhysSize, float TrlhysLocation, float DioneSize, float DioneLocation, float RheaSize, float RheaLocation, float TitanSize, float TitanLocation, float LapetusSize, float LapetusLocation)
			SaturnObejct(  SaturnSize * SizeScaler,   Saturn_distance*DistanceScaler,   saturnmimasmooonSize*SizeScaler,   saturnmimasmooonDistance*DistanceScaler,   saturnenceladusmoonSize*SizeScaler,   saturnenceladusmoonDistance*DistanceScaler,   saturntrlhysmoonSize*SizeScaler,   saturntrlhysmoonDistance*DistanceScaler,   saturndionemoonSize*SizeScaler,   saturndionemoonDistance*DistanceScaler,   saturnrheamoonSize*SizeScaler,   saturnrheamoonDistance*DistanceScaler,   saturntitanmoonSize*SizeScaler,   saturntitanmoonDistance*DistanceScaler,   saturnlapetusmoonSize*SizeScaler,   saturnlapetusmoonDistance*DistanceScaler);

			// Saturn Ring
			//void SaturnRing(float RingOuterRadious, float RingLocation, float Eccentricity, float RingInnerRadious, float SideSmooth)
			SaturnRing(  SaturnRingOuterRadius * SizeScaler,   Saturn_distance*DistanceScaler ,   1.0,   SaturnRingInnerRadius * SizeScaler,   100);

			// Uranus Object
			//UranusObject(float UranusSize, float UranusLocation, float UmbrielSize, float UmbrielLocation, float TitaniaSize, float TitaniaLocation, float OberonSize, float OberonLocation)
			UranusObject(  UranusSize * SizeScaler,   Uranus_distance*DistanceScaler,   uranusumbrielmoonSize*SizeScaler,   uranusumbrielmoonDistance*DistanceScaler,   uranustitaniamoonSize*SizeScaler,   uranustitaniamoonDistance*DistanceScaler,   uranusoberonmoonSize*SizeScaler,   uranusoberonmoonDistance*DistanceScaler);

			// Nebtune Object
			//NebtuneObject(float NebtuneSize, float NebtuneLocation, float TritonSize, float TritonLocation )
			NebtuneObject(  NeptuneSize * SizeScaler,   Neptune_distance*DistanceScaler, nebtunemoontritonSize *SizeScaler, nebtunemoontritonDistance*DistanceScaler);

			//Pluto Object
			//PlutoObject(float PlutoSize, float PlutoLocation, float CharonSize, float CharonLocation)
			PlutoObject(  PlutoSize * SizeScaler,   Pluto_distance*DistanceScaler,   plutomooncharonSize*SizeScaler,   plutocmoonharonDistance*DistanceScaler);

		}
		//Mars Object
		else if (WhichPlanet == Mars)
		{

			//Caculate the earth postion
			GLfloat MarsPos[3] = {Mars_distance*DistanceScaler * cos(-mars * M_PI / 180), 0, Mars_distance*DistanceScaler * sin(-mars * M_PI / 180)};
			//Caculate the Camera Position
			GLfloat cameraPos[3] = {Mars_distance*DistanceScaler * cos(-mars * M_PI / 180), (.6*SizeScaler), Mars_distance*DistanceScaler * sin(-mars * M_PI / 180)};
			//Setup the camear on the top of the moon pointing 
			gluLookAt(cameraPos[0], cameraPos[1], cameraPos[2]+(1*SizeScaler), MarsPos[0], MarsPos[1], MarsPos[2]-(1*SizeScaler), 0, 0, -1);


			SetPointLight(GL_LIGHT1,0.0,0.0,0.0,1,1,.9);
			// Mars Object
			MarsObject(  MarsSize * SizeScaler,   Mars_distance*DistanceScaler,   marsdiemosmoonSize* SizeScaler,   marsdiemosmoonDistance * DistanceScaler,   marsphobosmoonSize *SizeScaler,   marsphobosmoonDistance* DistanceScaler);

			// Space Object
			//SpaceObject(float SpaceSize)
			SpaceObject( 200000 );

			// Sun Object
			//SunObject(float SunSize)
			SunObject( SunSize * SizeScaler);

			//Mercury Object
			//MercuryObject(float MercurySize, float MercuryLocation)
			MercuryObject(MercurySize*SizeScaler, mercurydistance*DistanceScaler);

			// Venus Object
			//VenusObject(float VenusSize, float VenusLocation)
			VenusObject(  VenusSize * SizeScaler,   Venus_distance*DistanceScaler);

			// Earth Object
			//EarthObject(float EarthSize, float EarthLocation, float MoonSize, float Moon_Location)
			EarthObject(  EarthSize * SizeScaler,   Earth_distance*DistanceScaler,   earthMoonSize*SizeScaler,   earthMoonDistance*DistanceScaler);

			//Mars Object
			//MarsObject(float MarsSize, float MarsLocation, float PhobosSize, float PhobosLocation , float DiemosSize , float DiemosLocaion)
			//MarsObject(  MarsSize * SizeScaler,   Mars_distance*DistanceScaler,   marsdiemosmoonSize* SizeScaler,   marsdiemosmoonDistance * DistanceScaler,   marsphobosmoonSize *SizeScaler,   marsphobosmoonDistance* DistanceScaler);

			// Jupiter Object
			//JupiterObject(float JubiterSize, float JubiterLocation, float IoSize, float IoLocation, float EuropaSize, float EuropaLocation, float GanymedeSize, float GanymedeLocation, float CallistoSize, float CallistoLocation)
			JupiterObject(  JupitureSize * SizeScaler,   Jupiture_distance*DistanceScaler,   jupiteriomoonSize *SizeScaler,  jupiteriomoonDistance*DistanceScaler,   jupitereuropamoonSize * SizeScaler,   jupitereuropamoonDistance*DistanceScaler,   jupiterganymedemoonSize * SizeScaler,   jupiterganymedemoonDistance*DistanceScaler,   jupitercallistomoonSize  * SizeScaler,   jupitercallistomoonDistance*DistanceScaler);

			//Saturn Object
			//void SaturnObejct(float SaturnSize, float SaturnLocation, float MimasSize, float MimasLocation, float EnceladusSize, float EnceladusLocation, float TrlhysSize, float TrlhysLocation, float DioneSize, float DioneLocation, float RheaSize, float RheaLocation, float TitanSize, float TitanLocation, float LapetusSize, float LapetusLocation)
			SaturnObejct(  SaturnSize * SizeScaler,   Saturn_distance*DistanceScaler,   saturnmimasmooonSize*SizeScaler,   saturnmimasmooonDistance*DistanceScaler,   saturnenceladusmoonSize*SizeScaler,   saturnenceladusmoonDistance*DistanceScaler,   saturntrlhysmoonSize*SizeScaler,   saturntrlhysmoonDistance*DistanceScaler,   saturndionemoonSize*SizeScaler,   saturndionemoonDistance*DistanceScaler,   saturnrheamoonSize*SizeScaler,   saturnrheamoonDistance*DistanceScaler,   saturntitanmoonSize*SizeScaler,   saturntitanmoonDistance*DistanceScaler,   saturnlapetusmoonSize*SizeScaler,   saturnlapetusmoonDistance*DistanceScaler);

			// Saturn Ring
			//void SaturnRing(float RingOuterRadious, float RingLocation, float Eccentricity, float RingInnerRadious, float SideSmooth)
			SaturnRing(  SaturnRingOuterRadius * SizeScaler,   Saturn_distance*DistanceScaler ,   1.0,   SaturnRingInnerRadius * SizeScaler,   100);

			// Uranus Object
			//UranusObject(float UranusSize, float UranusLocation, float UmbrielSize, float UmbrielLocation, float TitaniaSize, float TitaniaLocation, float OberonSize, float OberonLocation)
			UranusObject(  UranusSize * SizeScaler,   Uranus_distance*DistanceScaler,   uranusumbrielmoonSize*SizeScaler,   uranusumbrielmoonDistance*DistanceScaler,   uranustitaniamoonSize*SizeScaler,   uranustitaniamoonDistance*DistanceScaler,   uranusoberonmoonSize*SizeScaler,   uranusoberonmoonDistance*DistanceScaler);

			// Nebtune Object
			//NebtuneObject(float NebtuneSize, float NebtuneLocation, float TritonSize, float TritonLocation )
			NebtuneObject(  NeptuneSize * SizeScaler,   Neptune_distance*DistanceScaler, nebtunemoontritonSize *SizeScaler, nebtunemoontritonDistance*DistanceScaler);

			//Pluto Object
			//PlutoObject(float PlutoSize, float PlutoLocation, float CharonSize, float CharonLocation)
			PlutoObject(  PlutoSize * SizeScaler,   Pluto_distance*DistanceScaler,   plutomooncharonSize*SizeScaler,   plutocmoonharonDistance*DistanceScaler);

		}

		else if (WhichPlanet == Saturn)


		{

			//Caculate the earth postion
			GLfloat SaturnPos[3] = {Saturn_distance*DistanceScaler * cos(-saturn * M_PI / 180), 0, Saturn_distance*DistanceScaler * sin(-saturn * M_PI / 180)};
			//Caculate the Camera Position
			GLfloat cameraPos[3] = {Saturn_distance*DistanceScaler * cos(-saturn * M_PI / 180), (4), Saturn_distance*DistanceScaler * sin(-saturn * M_PI / 180)};
			//Setup the camear on the top of the moon pointing 
			gluLookAt(cameraPos[0], cameraPos[1], cameraPos[2]+(18*SizeScaler), SaturnPos[0], SaturnPos[1], SaturnPos[2]-(10), 0, 0, -1);
			// Jupiter Planet

			// Space Object
			//SpaceObject(float SpaceSize)
			SpaceObject( 200000 );

			SetPointLight(GL_LIGHT1,0.0,0.0,0.0,1,1,.9);
			//SetMaterial(1,1,1,.2);
			//Saturn Object
			//void SaturnObejct(float SaturnSize, float SaturnLocation, float MimasSize, float MimasLocation, float EnceladusSize, float EnceladusLocation, float TrlhysSize, float TrlhysLocation, float DioneSize, float DioneLocation, float RheaSize, float RheaLocation, float TitanSize, float TitanLocation, float LapetusSize, float LapetusLocation)
			SaturnObejct(  SaturnSize * SizeScaler,   Saturn_distance*DistanceScaler,   saturnmimasmooonSize*SizeScaler,   saturnmimasmooonDistance*DistanceScaler,   saturnenceladusmoonSize*SizeScaler,   saturnenceladusmoonDistance*DistanceScaler,   saturntrlhysmoonSize*SizeScaler,   saturntrlhysmoonDistance*DistanceScaler,   saturndionemoonSize*SizeScaler,   saturndionemoonDistance*DistanceScaler,   saturnrheamoonSize*SizeScaler,   saturnrheamoonDistance*DistanceScaler,   saturntitanmoonSize*SizeScaler,   saturntitanmoonDistance*DistanceScaler,   saturnlapetusmoonSize*SizeScaler,   saturnlapetusmoonDistance*DistanceScaler);

			// Saturn Ring
			//void SaturnRing(float RingOuterRadious, float RingLocation, float Eccentricity, float RingInnerRadious, float SideSmooth)
			SaturnRing(  SaturnRingOuterRadius * SizeScaler,   Saturn_distance*DistanceScaler ,   1.0,   SaturnRingInnerRadius * SizeScaler,   100);

			// Sun Object
			//SunObject(float SunSize)
			SunObject( SunSize * SizeScaler);

			//Mercury Object
			//MercuryObject(float MercurySize, float MercuryLocation)
			MercuryObject(MercurySize*SizeScaler, mercurydistance*DistanceScaler);

			// Venus Object
			//VenusObject(float VenusSize, float VenusLocation)
			VenusObject(  VenusSize * SizeScaler,   Venus_distance*DistanceScaler);

			// Earth Object
			//EarthObject(float EarthSize, float EarthLocation, float MoonSize, float Moon_Location)
			EarthObject(  EarthSize * SizeScaler,   Earth_distance*DistanceScaler,   earthMoonSize*SizeScaler,   earthMoonDistance*DistanceScaler);

			//Mars Object
			//MarsObject(float MarsSize, float MarsLocation, float PhobosSize, float PhobosLocation , float DiemosSize , float DiemosLocaion)
			MarsObject(  MarsSize * SizeScaler,   Mars_distance*DistanceScaler,   marsdiemosmoonSize* SizeScaler,   marsdiemosmoonDistance * DistanceScaler,   marsphobosmoonSize *SizeScaler,   marsphobosmoonDistance* DistanceScaler);

			// Jupiter Object
			//JupiterObject(float JubiterSize, float JubiterLocation, float IoSize, float IoLocation, float EuropaSize, float EuropaLocation, float GanymedeSize, float GanymedeLocation, float CallistoSize, float CallistoLocation)
			JupiterObject(  JupitureSize * SizeScaler,   Jupiture_distance*DistanceScaler,   jupiteriomoonSize *SizeScaler,  jupiteriomoonDistance*DistanceScaler,   jupitereuropamoonSize * SizeScaler,   jupitereuropamoonDistance*DistanceScaler,   jupiterganymedemoonSize * SizeScaler,   jupiterganymedemoonDistance*DistanceScaler,   jupitercallistomoonSize  * SizeScaler,   jupitercallistomoonDistance*DistanceScaler);

			//Saturn Object
			//void SaturnObejct(float SaturnSize, float SaturnLocation, float MimasSize, float MimasLocation, float EnceladusSize, float EnceladusLocation, float TrlhysSize, float TrlhysLocation, float DioneSize, float DioneLocation, float RheaSize, float RheaLocation, float TitanSize, float TitanLocation, float LapetusSize, float LapetusLocation)
			//SaturnObejct(  SaturnSize * SizeScaler,   Saturn_distance*DistanceScaler,   saturnmimasmooonSize*SizeScaler,   saturnmimasmooonDistance*DistanceScaler,   saturnenceladusmoonSize*SizeScaler,   saturnenceladusmoonDistance*DistanceScaler,   saturntrlhysmoonSize*SizeScaler,   saturntrlhysmoonDistance*DistanceScaler,   saturndionemoonSize*SizeScaler,   saturndionemoonDistance*DistanceScaler,   saturnrheamoonSize*SizeScaler,   saturnrheamoonDistance*DistanceScaler,   saturntitanmoonSize*SizeScaler,   saturntitanmoonDistance*DistanceScaler,   saturnlapetusmoonSize*SizeScaler,   saturnlapetusmoonDistance*DistanceScaler);

			// Saturn Ring
			//void SaturnRing(float RingOuterRadious, float RingLocation, float Eccentricity, float RingInnerRadious, float SideSmooth)
			//SaturnRing(  SaturnRingOuterRadius * SizeScaler,   Saturn_distance*DistanceScaler ,   1.0,   SaturnRingInnerRadius * SizeScaler,   100);

			// Uranus Object
			//UranusObject(float UranusSize, float UranusLocation, float UmbrielSize, float UmbrielLocation, float TitaniaSize, float TitaniaLocation, float OberonSize, float OberonLocation)
			UranusObject(  UranusSize * SizeScaler,   Uranus_distance*DistanceScaler,   uranusumbrielmoonSize*SizeScaler,   uranusumbrielmoonDistance*DistanceScaler,   uranustitaniamoonSize*SizeScaler,   uranustitaniamoonDistance*DistanceScaler,   uranusoberonmoonSize*SizeScaler,   uranusoberonmoonDistance*DistanceScaler);

			// Nebtune Object
			//NebtuneObject(float NebtuneSize, float NebtuneLocation, float TritonSize, float TritonLocation )
			NebtuneObject(  NeptuneSize * SizeScaler,   Neptune_distance*DistanceScaler, nebtunemoontritonSize *SizeScaler, nebtunemoontritonDistance*DistanceScaler);

			//Pluto Object
			//PlutoObject(float PlutoSize, float PlutoLocation, float CharonSize, float CharonLocation)
			PlutoObject(  PlutoSize * SizeScaler,   Pluto_distance*DistanceScaler,   plutomooncharonSize*SizeScaler,   plutocmoonharonDistance*DistanceScaler);

		}

		else if (WhichPlanet == Uranus)
		{


			//Caculate the earth postion
			GLfloat UranusPos[3] = {Uranus_distance*DistanceScaler * cos(-uranus * M_PI / 180), 0, Uranus_distance*DistanceScaler * sin(-uranus * M_PI / 180)};
			//Caculate the Camera Position
			GLfloat cameraPos[3] = {Uranus_distance*DistanceScaler * cos(-uranus * M_PI / 180), (8*SizeScaler), Uranus_distance*DistanceScaler * sin(-uranus * M_PI / 180)};
			//Setup the camear on the top of the moon pointing 
			gluLookAt(cameraPos[0], cameraPos[1], cameraPos[2]+(8*SizeScaler), UranusPos[0], UranusPos[1], UranusPos[2]-(11*SizeScaler), 0, 0, -1);

			SetPointLight(GL_LIGHT1,0.0,0.0,0.0,1,1,.9);
			//SetMaterial(1,1,1,.2);
			//Saturn Object
			// Uranus Planet
			UranusObject(  UranusSize * SizeScaler,   Uranus_distance*DistanceScaler,   uranusumbrielmoonSize*SizeScaler,   uranusumbrielmoonDistance*DistanceScaler,   uranustitaniamoonSize*SizeScaler,   uranustitaniamoonDistance*DistanceScaler,   uranusoberonmoonSize*SizeScaler,   uranusoberonmoonDistance*DistanceScaler);

			// Space Object
			//SpaceObject(float SpaceSize)
			SpaceObject( 200000 );

			// Sun Object
			//SunObject(float SunSize)
			SunObject( SunSize * SizeScaler);

			//Mercury Object
			//MercuryObject(float MercurySize, float MercuryLocation)
			MercuryObject(MercurySize*SizeScaler, mercurydistance*DistanceScaler);

			// Venus Object
			//VenusObject(float VenusSize, float VenusLocation)
			VenusObject(  VenusSize * SizeScaler,   Venus_distance*DistanceScaler);

			// Earth Object
			//EarthObject(float EarthSize, float EarthLocation, float MoonSize, float Moon_Location)
			EarthObject(  EarthSize * SizeScaler,   Earth_distance*DistanceScaler,   earthMoonSize*SizeScaler,   earthMoonDistance*DistanceScaler);

			//Mars Object
			//MarsObject(float MarsSize, float MarsLocation, float PhobosSize, float PhobosLocation , float DiemosSize , float DiemosLocaion)
			MarsObject(  MarsSize * SizeScaler,   Mars_distance*DistanceScaler,   marsdiemosmoonSize* SizeScaler,   marsdiemosmoonDistance * DistanceScaler,   marsphobosmoonSize *SizeScaler,   marsphobosmoonDistance* DistanceScaler);

			// Jupiter Object
			//JupiterObject(float JubiterSize, float JubiterLocation, float IoSize, float IoLocation, float EuropaSize, float EuropaLocation, float GanymedeSize, float GanymedeLocation, float CallistoSize, float CallistoLocation)
			JupiterObject(  JupitureSize * SizeScaler,   Jupiture_distance*DistanceScaler,   jupiteriomoonSize *SizeScaler,  jupiteriomoonDistance*DistanceScaler,   jupitereuropamoonSize * SizeScaler,   jupitereuropamoonDistance*DistanceScaler,   jupiterganymedemoonSize * SizeScaler,   jupiterganymedemoonDistance*DistanceScaler,   jupitercallistomoonSize  * SizeScaler,   jupitercallistomoonDistance*DistanceScaler);

			//Saturn Object
			//void SaturnObejct(float SaturnSize, float SaturnLocation, float MimasSize, float MimasLocation, float EnceladusSize, float EnceladusLocation, float TrlhysSize, float TrlhysLocation, float DioneSize, float DioneLocation, float RheaSize, float RheaLocation, float TitanSize, float TitanLocation, float LapetusSize, float LapetusLocation)
			SaturnObejct(  SaturnSize * SizeScaler,   Saturn_distance*DistanceScaler,   saturnmimasmooonSize*SizeScaler,   saturnmimasmooonDistance*DistanceScaler,   saturnenceladusmoonSize*SizeScaler,   saturnenceladusmoonDistance*DistanceScaler,   saturntrlhysmoonSize*SizeScaler,   saturntrlhysmoonDistance*DistanceScaler,   saturndionemoonSize*SizeScaler,   saturndionemoonDistance*DistanceScaler,   saturnrheamoonSize*SizeScaler,   saturnrheamoonDistance*DistanceScaler,   saturntitanmoonSize*SizeScaler,   saturntitanmoonDistance*DistanceScaler,   saturnlapetusmoonSize*SizeScaler,   saturnlapetusmoonDistance*DistanceScaler);

			// Saturn Ring
			//void SaturnRing(float RingOuterRadious, float RingLocation, float Eccentricity, float RingInnerRadious, float SideSmooth)
			SaturnRing(  SaturnRingOuterRadius * SizeScaler,   Saturn_distance*DistanceScaler ,   1.0,   SaturnRingInnerRadius * SizeScaler,   100);

			// Uranus Object
			//UranusObject(float UranusSize, float UranusLocation, float UmbrielSize, float UmbrielLocation, float TitaniaSize, float TitaniaLocation, float OberonSize, float OberonLocation)
			//UranusObject(  UranusSize * SizeScaler,   Uranus_distance*DistanceScaler,   uranusumbrielmoonSize*SizeScaler,   uranusumbrielmoonDistance*DistanceScaler,   uranustitaniamoonSize*SizeScaler,   uranustitaniamoonDistance*DistanceScaler,   uranusoberonmoonSize*SizeScaler,   uranusoberonmoonDistance*DistanceScaler);

			// Nebtune Object
			//NebtuneObject(float NebtuneSize, float NebtuneLocation, float TritonSize, float TritonLocation )
			NebtuneObject(  NeptuneSize * SizeScaler,   Neptune_distance*DistanceScaler, nebtunemoontritonSize *SizeScaler, nebtunemoontritonDistance*DistanceScaler);

			//Pluto Object
			//PlutoObject(float PlutoSize, float PlutoLocation, float CharonSize, float CharonLocation)
			PlutoObject(  PlutoSize * SizeScaler,   Pluto_distance*DistanceScaler,   plutomooncharonSize*SizeScaler,   plutocmoonharonDistance*DistanceScaler);

		}
		// Nebtune Object
		else if (WhichPlanet == Nebtune)
		{


			//Caculate the earth postion
			GLfloat NebtunePos[3] = {Neptune_distance*DistanceScaler * cos(-nebtune * M_PI / 180), 0, Neptune_distance*DistanceScaler * sin(-nebtune * M_PI / 180)};
			//Caculate the Camera Position
			GLfloat cameraPos[3] = {Neptune_distance*DistanceScaler * cos(-nebtune * M_PI / 180), (7*SizeScaler), Neptune_distance*DistanceScaler * sin(-nebtune * M_PI / 180)};
			//Setup the camear on the top of the moon pointing 
			gluLookAt(cameraPos[0], cameraPos[1], cameraPos[2]+(8 *SizeScaler ), NebtunePos[0], NebtunePos[1], NebtunePos[2]-(6*SizeScaler), 0, 0, -1);

			SetPointLight(GL_LIGHT1,0.0,0.0,0.0,1,1,.9);
			//SetMaterial(1,1,1,.2);

			// Nebtun Planet
			//NebtuneObject(float NebtuneSize, float NebtuneLocation, float TritonSize, float TritonLocation )
			NebtuneObject(  NeptuneSize * SizeScaler,   Neptune_distance*DistanceScaler, nebtunemoontritonSize *SizeScaler, nebtunemoontritonDistance*DistanceScaler);


			// Space Object
			//SpaceObject(float SpaceSize)
			SpaceObject( 200000 );

			// Sun Object
			//SunObject(float SunSize)
			SunObject( SunSize * SizeScaler);

			//Mercury Object
			//MercuryObject(float MercurySize, float MercuryLocation)
			MercuryObject(MercurySize*SizeScaler, mercurydistance*DistanceScaler);

			// Venus Object
			//VenusObject(float VenusSize, float VenusLocation)
			VenusObject(  VenusSize * SizeScaler,   Venus_distance*DistanceScaler);

			// Earth Object
			//EarthObject(float EarthSize, float EarthLocation, float MoonSize, float Moon_Location)
			EarthObject(  EarthSize * SizeScaler,   Earth_distance*DistanceScaler,   earthMoonSize*SizeScaler,   earthMoonDistance*DistanceScaler);

			//Mars Object
			//MarsObject(float MarsSize, float MarsLocation, float PhobosSize, float PhobosLocation , float DiemosSize , float DiemosLocaion)
			MarsObject(  MarsSize * SizeScaler,   Mars_distance*DistanceScaler,   marsdiemosmoonSize* SizeScaler,   marsdiemosmoonDistance * DistanceScaler,   marsphobosmoonSize *SizeScaler,   marsphobosmoonDistance* DistanceScaler);

			// Jupiter Object
			//JupiterObject(float JubiterSize, float JubiterLocation, float IoSize, float IoLocation, float EuropaSize, float EuropaLocation, float GanymedeSize, float GanymedeLocation, float CallistoSize, float CallistoLocation)
			JupiterObject(  JupitureSize * SizeScaler,   Jupiture_distance*DistanceScaler,   jupiteriomoonSize *SizeScaler,  jupiteriomoonDistance*DistanceScaler,   jupitereuropamoonSize * SizeScaler,   jupitereuropamoonDistance*DistanceScaler,   jupiterganymedemoonSize * SizeScaler,   jupiterganymedemoonDistance*DistanceScaler,   jupitercallistomoonSize  * SizeScaler,   jupitercallistomoonDistance*DistanceScaler);

			//Saturn Object
			//void SaturnObejct(float SaturnSize, float SaturnLocation, float MimasSize, float MimasLocation, float EnceladusSize, float EnceladusLocation, float TrlhysSize, float TrlhysLocation, float DioneSize, float DioneLocation, float RheaSize, float RheaLocation, float TitanSize, float TitanLocation, float LapetusSize, float LapetusLocation)
			SaturnObejct(  SaturnSize * SizeScaler,   Saturn_distance*DistanceScaler,   saturnmimasmooonSize*SizeScaler,   saturnmimasmooonDistance*DistanceScaler,   saturnenceladusmoonSize*SizeScaler,   saturnenceladusmoonDistance*DistanceScaler,   saturntrlhysmoonSize*SizeScaler,   saturntrlhysmoonDistance*DistanceScaler,   saturndionemoonSize*SizeScaler,   saturndionemoonDistance*DistanceScaler,   saturnrheamoonSize*SizeScaler,   saturnrheamoonDistance*DistanceScaler,   saturntitanmoonSize*SizeScaler,   saturntitanmoonDistance*DistanceScaler,   saturnlapetusmoonSize*SizeScaler,   saturnlapetusmoonDistance*DistanceScaler);

			// Saturn Ring
			//void SaturnRing(float RingOuterRadious, float RingLocation, float Eccentricity, float RingInnerRadious, float SideSmooth)
			SaturnRing(  SaturnRingOuterRadius * SizeScaler,   Saturn_distance*DistanceScaler ,   1.0,   SaturnRingInnerRadius * SizeScaler,   100);

			// Uranus Object
			//UranusObject(float UranusSize, float UranusLocation, float UmbrielSize, float UmbrielLocation, float TitaniaSize, float TitaniaLocation, float OberonSize, float OberonLocation)
			UranusObject(  UranusSize * SizeScaler,   Uranus_distance*DistanceScaler,   uranusumbrielmoonSize*SizeScaler,   uranusumbrielmoonDistance*DistanceScaler,   uranustitaniamoonSize*SizeScaler,   uranustitaniamoonDistance*DistanceScaler,   uranusoberonmoonSize*SizeScaler,   uranusoberonmoonDistance*DistanceScaler);

			// Nebtune Object
			//NebtuneObject(float NebtuneSize, float NebtuneLocation, float TritonSize, float TritonLocation )
			//NebtuneObject(  NeptuneSize * SizeScaler,   Neptune_distance*DistanceScaler, nebtunemoontritonSize *SizeScaler, nebtunemoontritonDistance*DistanceScaler);

			//Pluto Object
			//PlutoObject(float PlutoSize, float PlutoLocation, float CharonSize, float CharonLocation)
			PlutoObject(  PlutoSize * SizeScaler,   Pluto_distance*DistanceScaler,   plutomooncharonSize*SizeScaler,   plutocmoonharonDistance*DistanceScaler);

		}
		//Pluto Object
		else if (WhichPlanet == Pluto)
		{


			//Caculate the earth postion
			GLfloat NebtunePos[3] = {Pluto_distance*DistanceScaler * cos(-pluto * M_PI / 180), 0, Pluto_distance*DistanceScaler * sin(-pluto * M_PI / 180)};
			//Caculate the Camera Position
			GLfloat cameraPos[3] = {Pluto_distance*DistanceScaler * cos(-pluto * M_PI / 180), (7*SizeScaler), Pluto_distance*DistanceScaler * sin(-pluto * M_PI / 180)};
			//Setup the camear on the top of the moon pointing 
			gluLookAt(cameraPos[0], cameraPos[1], cameraPos[2]+(12*SizeScaler), NebtunePos[0], NebtunePos[1], NebtunePos[2]-(6*SizeScaler), 0, 0, -1);

			SetPointLight(GL_LIGHT1,0.0,0.0,0.0,1,1,.9);
			//SetMaterial(1,1,1,.2);

			//Pluto Object
			PlutoObject(  PlutoSize * SizeScaler,   Pluto_distance*DistanceScaler,   plutomooncharonSize*SizeScaler,   plutocmoonharonDistance*DistanceScaler);

			// Space Object
			//SpaceObject(float SpaceSize)
			SpaceObject( 200000 );

			// Sun Object
			//SunObject(float SunSize)
			SunObject( SunSize * SizeScaler);

			//Mercury Object
			//MercuryObject(float MercurySize, float MercuryLocation)
			MercuryObject(MercurySize*SizeScaler, mercurydistance*DistanceScaler);

			// Venus Object
			//VenusObject(float VenusSize, float VenusLocation)
			VenusObject(  VenusSize * SizeScaler,   Venus_distance*DistanceScaler);

			// Earth Object
			//EarthObject(float EarthSize, float EarthLocation, float MoonSize, float Moon_Location)
			EarthObject(  EarthSize * SizeScaler,   Earth_distance*DistanceScaler,   earthMoonSize*SizeScaler,   earthMoonDistance*DistanceScaler);

			//Mars Object
			//MarsObject(float MarsSize, float MarsLocation, float PhobosSize, float PhobosLocation , float DiemosSize , float DiemosLocaion)
			MarsObject(  MarsSize * SizeScaler,   Mars_distance*DistanceScaler,   marsdiemosmoonSize* SizeScaler,   marsdiemosmoonDistance * DistanceScaler,   marsphobosmoonSize *SizeScaler,   marsphobosmoonDistance* DistanceScaler);

			// Jupiter Object
			//JupiterObject(float JubiterSize, float JubiterLocation, float IoSize, float IoLocation, float EuropaSize, float EuropaLocation, float GanymedeSize, float GanymedeLocation, float CallistoSize, float CallistoLocation)
			JupiterObject(  JupitureSize * SizeScaler,   Jupiture_distance*DistanceScaler,   jupiteriomoonSize *SizeScaler,  jupiteriomoonDistance*DistanceScaler,   jupitereuropamoonSize * SizeScaler,   jupitereuropamoonDistance*DistanceScaler,   jupiterganymedemoonSize * SizeScaler,   jupiterganymedemoonDistance*DistanceScaler,   jupitercallistomoonSize  * SizeScaler,   jupitercallistomoonDistance*DistanceScaler);

			//Saturn Object
			//void SaturnObejct(float SaturnSize, float SaturnLocation, float MimasSize, float MimasLocation, float EnceladusSize, float EnceladusLocation, float TrlhysSize, float TrlhysLocation, float DioneSize, float DioneLocation, float RheaSize, float RheaLocation, float TitanSize, float TitanLocation, float LapetusSize, float LapetusLocation)
			SaturnObejct(  SaturnSize * SizeScaler,   Saturn_distance*DistanceScaler,   saturnmimasmooonSize*SizeScaler,   saturnmimasmooonDistance*DistanceScaler,   saturnenceladusmoonSize*SizeScaler,   saturnenceladusmoonDistance*DistanceScaler,   saturntrlhysmoonSize*SizeScaler,   saturntrlhysmoonDistance*DistanceScaler,   saturndionemoonSize*SizeScaler,   saturndionemoonDistance*DistanceScaler,   saturnrheamoonSize*SizeScaler,   saturnrheamoonDistance*DistanceScaler,   saturntitanmoonSize*SizeScaler,   saturntitanmoonDistance*DistanceScaler,   saturnlapetusmoonSize*SizeScaler,   saturnlapetusmoonDistance*DistanceScaler);

			// Saturn Ring
			//void SaturnRing(float RingOuterRadious, float RingLocation, float Eccentricity, float RingInnerRadious, float SideSmooth)
			SaturnRing(  SaturnRingOuterRadius * SizeScaler,   Saturn_distance*DistanceScaler ,   1.0,   SaturnRingInnerRadius * SizeScaler,   100);

			// Uranus Object
			//UranusObject(float UranusSize, float UranusLocation, float UmbrielSize, float UmbrielLocation, float TitaniaSize, float TitaniaLocation, float OberonSize, float OberonLocation)
			UranusObject(  UranusSize * SizeScaler,   Uranus_distance*DistanceScaler,   uranusumbrielmoonSize*SizeScaler,   uranusumbrielmoonDistance*DistanceScaler,   uranustitaniamoonSize*SizeScaler,   uranustitaniamoonDistance*DistanceScaler,   uranusoberonmoonSize*SizeScaler,   uranusoberonmoonDistance*DistanceScaler);

			// Nebtune Object
			//NebtuneObject(float NebtuneSize, float NebtuneLocation, float TritonSize, float TritonLocation )
			NebtuneObject(  NeptuneSize * SizeScaler,   Neptune_distance*DistanceScaler, nebtunemoontritonSize *SizeScaler, nebtunemoontritonDistance*DistanceScaler);

			//Pluto Object
			//PlutoObject(float PlutoSize, float PlutoLocation, float CharonSize, float CharonLocation)
			//PlutoObject(  PlutoSize * SizeScaler,   Pluto_distance*DistanceScaler,   plutomooncharonSize*SizeScaler,   plutocmoonharonDistance*DistanceScaler);

		}

		else if (WhichPlanet == Mercury)
		{

			//Caculate the earth postion
			GLfloat MercuryPos[3] = {mercurydistance*DistanceScaler * cos(-mercury * M_PI / 180), 0, mercurydistance*DistanceScaler * sin(-mercury * M_PI / 180)};
			//Caculate the Camera Position
			GLfloat cameraPos[3] = {mercurydistance*DistanceScaler * cos(-mercury * M_PI / 180), (.5*SizeScaler), mercurydistance*DistanceScaler * sin(-mercury * M_PI / 180)};
			//Setup the camear on the top of the moon pointing 
			gluLookAt(cameraPos[0], cameraPos[1], cameraPos[2]+(1*SizeScaler), MercuryPos[0], MercuryPos[1], MercuryPos[2]-(1*SizeScaler), 0, 0, -1);

			SetPointLight(GL_LIGHT1,0.0,0.0,0.0,1,1,.9);

			MercuryObject(MercurySize*SizeScaler, mercurydistance*DistanceScaler);

			// Space Object
			//SpaceObject(float SpaceSize)
			SpaceObject( 200000 );

			// Sun Object
			//SunObject(float SunSize)
			SunObject( SunSize * SizeScaler);

			//Mercury Object
			//MercuryObject(float MercurySize, float MercuryLocation)
			//MercuryObject(MercurySize*SizeScaler, mercurydistance*DistanceScaler);

			// Venus Object
			//VenusObject(float VenusSize, float VenusLocation)
			VenusObject(  VenusSize * SizeScaler,   Venus_distance*DistanceScaler);

			// Earth Object
			//EarthObject(float EarthSize, float EarthLocation, float MoonSize, float Moon_Location)
			EarthObject(  EarthSize * SizeScaler,   Earth_distance*DistanceScaler,   earthMoonSize*SizeScaler,   earthMoonDistance*DistanceScaler);

			//Mars Object
			//MarsObject(float MarsSize, float MarsLocation, float PhobosSize, float PhobosLocation , float DiemosSize , float DiemosLocaion)
			MarsObject(  MarsSize * SizeScaler,   Mars_distance*DistanceScaler,   marsdiemosmoonSize* SizeScaler,   marsdiemosmoonDistance * DistanceScaler,   marsphobosmoonSize *SizeScaler,   marsphobosmoonDistance* DistanceScaler);

			// Jupiter Object
			//JupiterObject(float JubiterSize, float JubiterLocation, float IoSize, float IoLocation, float EuropaSize, float EuropaLocation, float GanymedeSize, float GanymedeLocation, float CallistoSize, float CallistoLocation)
			JupiterObject(  JupitureSize * SizeScaler,   Jupiture_distance*DistanceScaler,   jupiteriomoonSize *SizeScaler,  jupiteriomoonDistance*DistanceScaler,   jupitereuropamoonSize * SizeScaler,   jupitereuropamoonDistance*DistanceScaler,   jupiterganymedemoonSize * SizeScaler,   jupiterganymedemoonDistance*DistanceScaler,   jupitercallistomoonSize  * SizeScaler,   jupitercallistomoonDistance*DistanceScaler);

			//Saturn Object
			//void SaturnObejct(float SaturnSize, float SaturnLocation, float MimasSize, float MimasLocation, float EnceladusSize, float EnceladusLocation, float TrlhysSize, float TrlhysLocation, float DioneSize, float DioneLocation, float RheaSize, float RheaLocation, float TitanSize, float TitanLocation, float LapetusSize, float LapetusLocation)
			SaturnObejct(  SaturnSize * SizeScaler,   Saturn_distance*DistanceScaler,   saturnmimasmooonSize*SizeScaler,   saturnmimasmooonDistance*DistanceScaler,   saturnenceladusmoonSize*SizeScaler,   saturnenceladusmoonDistance*DistanceScaler,   saturntrlhysmoonSize*SizeScaler,   saturntrlhysmoonDistance*DistanceScaler,   saturndionemoonSize*SizeScaler,   saturndionemoonDistance*DistanceScaler,   saturnrheamoonSize*SizeScaler,   saturnrheamoonDistance*DistanceScaler,   saturntitanmoonSize*SizeScaler,   saturntitanmoonDistance*DistanceScaler,   saturnlapetusmoonSize*SizeScaler,   saturnlapetusmoonDistance*DistanceScaler);

			// Saturn Ring
			//void SaturnRing(float RingOuterRadious, float RingLocation, float Eccentricity, float RingInnerRadious, float SideSmooth)
			SaturnRing(  SaturnRingOuterRadius * SizeScaler,   Saturn_distance*DistanceScaler ,   1.0,   SaturnRingInnerRadius * SizeScaler,   100);

			// Uranus Object
			//UranusObject(float UranusSize, float UranusLocation, float UmbrielSize, float UmbrielLocation, float TitaniaSize, float TitaniaLocation, float OberonSize, float OberonLocation)
			UranusObject(  UranusSize * SizeScaler,   Uranus_distance*DistanceScaler,   uranusumbrielmoonSize*SizeScaler,   uranusumbrielmoonDistance*DistanceScaler,   uranustitaniamoonSize*SizeScaler,   uranustitaniamoonDistance*DistanceScaler,   uranusoberonmoonSize*SizeScaler,   uranusoberonmoonDistance*DistanceScaler);

			// Nebtune Object
			//NebtuneObject(float NebtuneSize, float NebtuneLocation, float TritonSize, float TritonLocation )
			NebtuneObject(  NeptuneSize * SizeScaler,   Neptune_distance*DistanceScaler, nebtunemoontritonSize *SizeScaler, nebtunemoontritonDistance*DistanceScaler);

			//Pluto Object
			//PlutoObject(float PlutoSize, float PlutoLocation, float CharonSize, float CharonLocation)
			PlutoObject(  PlutoSize * SizeScaler,   Pluto_distance*DistanceScaler,   plutomooncharonSize*SizeScaler,   plutocmoonharonDistance*DistanceScaler);

		}
	}

	else if (WhichCamera == PlanetSurrounding)
	{

		glDisable( GL_FOG );

		glEnable( GL_NORMALIZE );

		// Sun Light			
		glEnable( GL_LIGHTING );
		glShadeModel(GL_SMOOTH);
		// Disable Planets Orbits 

		if (LinesEnabled == 0 || LinesEnabled == 1 )
		{
			PlanetsOrbits(0,0,0,0,0,0,0,0,0);
		}

		DistanceScaler = 3000;

		// Jupiter Object
		if (WhichPlanet == Jupiter)
		{
			//Caculate the earth postion
			GLfloat JupiterPos[3] = {Jupiture_distance*DistanceScaler * cos(-jupiter * M_PI / 180), 0, Jupiture_distance*DistanceScaler * sin(-jupiter * M_PI / 180)};
			//Caculate the Camera Position
			GLfloat cameraPos[3] = {Jupiture_distance*DistanceScaler * cos(-jupiter * M_PI / 180), (16*SizeScaler), Jupiture_distance*DistanceScaler * sin(-jupiter * M_PI / 180)};
			//Setup the camear on the top of the moon pointing 
			gluLookAt(cameraPos[0], cameraPos[1], cameraPos[2]+(50*SizeScaler), JupiterPos[0], JupiterPos[1], JupiterPos[2]-(20*SizeScaler), 0, 0, -1);
			// Jupiter Planet

			SetPointLight(GL_LIGHT1,0.0,0.0,0.0,1,1,.9);
			//SetMaterial(1,1,1,.2);
			JupiterObject(  JupitureSize * SizeScaler,   Jupiture_distance*DistanceScaler,   jupiteriomoonSize *SizeScaler,  jupiteriomoonDistance*DistanceScaler,   jupitereuropamoonSize * SizeScaler,   jupitereuropamoonDistance*DistanceScaler,   jupiterganymedemoonSize * SizeScaler,   jupiterganymedemoonDistance*DistanceScaler,   jupitercallistomoonSize  * SizeScaler,   jupitercallistomoonDistance*DistanceScaler);


			//JupiterObject(  10,   1400,   0.6,  12,   .5,   14,   .8,   16,   .8,   20);
			// Space Object
			//SpaceObject(float SpaceSize)
			SpaceObject( 200000 );

			// Sun Object
			//SunObject(float SunSize)
			SunObject( SunSize * SizeScaler);

			//Mercury Object
			//MercuryObject(float MercurySize, float MercuryLocation)
			MercuryObject(MercurySize*SizeScaler, mercurydistance*DistanceScaler);

			// Venus Object
			//VenusObject(float VenusSize, float VenusLocation)
			VenusObject(  VenusSize * SizeScaler,   Venus_distance*DistanceScaler);

			// Earth Object
			//EarthObject(float EarthSize, float EarthLocation, float MoonSize, float Moon_Location)
			EarthObject(  EarthSize * SizeScaler,   Earth_distance*DistanceScaler,   earthMoonSize*SizeScaler,   earthMoonDistance*DistanceScaler);

			//Mars Object
			//MarsObject(float MarsSize, float MarsLocation, float PhobosSize, float PhobosLocation , float DiemosSize , float DiemosLocaion)
			MarsObject(  MarsSize * SizeScaler,   Mars_distance*DistanceScaler,   marsdiemosmoonSize* SizeScaler,   marsdiemosmoonDistance * DistanceScaler,   marsphobosmoonSize *SizeScaler,   marsphobosmoonDistance* DistanceScaler);

			// Jupiter Object
			//JupiterObject(float JubiterSize, float JubiterLocation, float IoSize, float IoLocation, float EuropaSize, float EuropaLocation, float GanymedeSize, float GanymedeLocation, float CallistoSize, float CallistoLocation)
			//JupiterObject(  JupitureSize * SizeScaler,   Jupiture_distance*DistanceScaler,   jupiteriomoonSize *SizeScaler,  jupiteriomoonDistance*DistanceScaler,   jupitereuropamoonSize * SizeScaler,   jupitereuropamoonDistance*DistanceScaler,   jupiterganymedemoonSize * SizeScaler,   jupiterganymedemoonDistance*DistanceScaler,   jupitercallistomoonSize  * SizeScaler,   jupitercallistomoonDistance*DistanceScaler);

			//Saturn Object
			//void SaturnObejct(float SaturnSize, float SaturnLocation, float MimasSize, float MimasLocation, float EnceladusSize, float EnceladusLocation, float TrlhysSize, float TrlhysLocation, float DioneSize, float DioneLocation, float RheaSize, float RheaLocation, float TitanSize, float TitanLocation, float LapetusSize, float LapetusLocation)
			SaturnObejct(  SaturnSize * SizeScaler,   Saturn_distance*DistanceScaler,   saturnmimasmooonSize*SizeScaler,   saturnmimasmooonDistance*DistanceScaler,   saturnenceladusmoonSize*SizeScaler,   saturnenceladusmoonDistance*DistanceScaler,   saturntrlhysmoonSize*SizeScaler,   saturntrlhysmoonDistance*DistanceScaler,   saturndionemoonSize*SizeScaler,   saturndionemoonDistance*DistanceScaler,   saturnrheamoonSize*SizeScaler,   saturnrheamoonDistance*DistanceScaler,   saturntitanmoonSize*SizeScaler,   saturntitanmoonDistance*DistanceScaler,   saturnlapetusmoonSize*SizeScaler,   saturnlapetusmoonDistance*DistanceScaler);

			// Saturn Ring
			//void SaturnRing(float RingOuterRadious, float RingLocation, float Eccentricity, float RingInnerRadious, float SideSmooth)
			SaturnRing(  SaturnRingOuterRadius * SizeScaler,   Saturn_distance*DistanceScaler ,   1.0,   SaturnRingInnerRadius * SizeScaler,   100);

			// Uranus Object
			//UranusObject(float UranusSize, float UranusLocation, float UmbrielSize, float UmbrielLocation, float TitaniaSize, float TitaniaLocation, float OberonSize, float OberonLocation)
			UranusObject(  UranusSize * SizeScaler,   Uranus_distance*DistanceScaler,   uranusumbrielmoonSize*SizeScaler,   uranusumbrielmoonDistance*DistanceScaler,   uranustitaniamoonSize*SizeScaler,   uranustitaniamoonDistance*DistanceScaler,   uranusoberonmoonSize*SizeScaler,   uranusoberonmoonDistance*DistanceScaler);

			// Nebtune Object
			//NebtuneObject(float NebtuneSize, float NebtuneLocation, float TritonSize, float TritonLocation )
			NebtuneObject(  NeptuneSize * SizeScaler,   Neptune_distance*DistanceScaler, nebtunemoontritonSize *SizeScaler, nebtunemoontritonDistance*DistanceScaler);

			//Pluto Object
			//PlutoObject(float PlutoSize, float PlutoLocation, float CharonSize, float CharonLocation)
			PlutoObject(  PlutoSize * SizeScaler,   Pluto_distance*DistanceScaler,   plutomooncharonSize*SizeScaler,   plutocmoonharonDistance*DistanceScaler);

		}
		// Venus Object
		else if (WhichPlanet == Venus)
		{

			//Caculate the earth postion
			GLfloat VenusPos[3] = {Venus_distance*DistanceScaler * cos(-venus * M_PI / 180), 0, Venus_distance*DistanceScaler * sin(-venus * M_PI / 180)};
			//Caculate the Camera Position
			GLfloat cameraPos[3] = {Venus_distance*DistanceScaler * cos(-venus * M_PI / 180), (1.4*SizeScaler), Venus_distance*DistanceScaler * sin(-venus * M_PI / 180)};
			//Setup the camear on the top of the moon pointing 
			gluLookAt(cameraPos[0], cameraPos[1], cameraPos[2]+(4*SizeScaler), VenusPos[0], VenusPos[1], VenusPos[2]-(5*SizeScaler), 0, 0, -1);
			// Jupiter Planet

			SetPointLight(GL_LIGHT1,0.0,0.0,0.0,1,1,.9);
			//SetMaterial(1,1,1,.2);
			VenusObject(  VenusSize * SizeScaler,   Venus_distance*DistanceScaler);

			// Space Object
			//SpaceObject(float SpaceSize)
			SpaceObject( 200000 );

			// Sun Object
			//SunObject(float SunSize)
			SunObject( SunSize * SizeScaler);

			//Mercury Object
			//MercuryObject(float MercurySize, float MercuryLocation)
			MercuryObject(MercurySize*SizeScaler, mercurydistance*DistanceScaler);

			// Venus Object
			//VenusObject(float VenusSize, float VenusLocation)
			//VenusObject(  VenusSize * SizeScaler,   Venus_distance*DistanceScaler);

			// Earth Object
			//EarthObject(float EarthSize, float EarthLocation, float MoonSize, float Moon_Location)
			EarthObject(  EarthSize * SizeScaler,   Earth_distance*DistanceScaler,   earthMoonSize*SizeScaler,   earthMoonDistance*DistanceScaler);

			//Mars Object
			//MarsObject(float MarsSize, float MarsLocation, float PhobosSize, float PhobosLocation , float DiemosSize , float DiemosLocaion)
			MarsObject(  MarsSize * SizeScaler,   Mars_distance*DistanceScaler,   marsdiemosmoonSize* SizeScaler,   marsdiemosmoonDistance * DistanceScaler,   marsphobosmoonSize *SizeScaler,   marsphobosmoonDistance* DistanceScaler);

			// Jupiter Object
			//JupiterObject(float JubiterSize, float JubiterLocation, float IoSize, float IoLocation, float EuropaSize, float EuropaLocation, float GanymedeSize, float GanymedeLocation, float CallistoSize, float CallistoLocation)
			JupiterObject(  JupitureSize * SizeScaler,   Jupiture_distance*DistanceScaler,   jupiteriomoonSize *SizeScaler,  jupiteriomoonDistance*DistanceScaler,   jupitereuropamoonSize * SizeScaler,   jupitereuropamoonDistance*DistanceScaler,   jupiterganymedemoonSize * SizeScaler,   jupiterganymedemoonDistance*DistanceScaler,   jupitercallistomoonSize  * SizeScaler,   jupitercallistomoonDistance*DistanceScaler);

			//Saturn Object
			//void SaturnObejct(float SaturnSize, float SaturnLocation, float MimasSize, float MimasLocation, float EnceladusSize, float EnceladusLocation, float TrlhysSize, float TrlhysLocation, float DioneSize, float DioneLocation, float RheaSize, float RheaLocation, float TitanSize, float TitanLocation, float LapetusSize, float LapetusLocation)
			SaturnObejct(  SaturnSize * SizeScaler,   Saturn_distance*DistanceScaler,   saturnmimasmooonSize*SizeScaler,   saturnmimasmooonDistance*DistanceScaler,   saturnenceladusmoonSize*SizeScaler,   saturnenceladusmoonDistance*DistanceScaler,   saturntrlhysmoonSize*SizeScaler,   saturntrlhysmoonDistance*DistanceScaler,   saturndionemoonSize*SizeScaler,   saturndionemoonDistance*DistanceScaler,   saturnrheamoonSize*SizeScaler,   saturnrheamoonDistance*DistanceScaler,   saturntitanmoonSize*SizeScaler,   saturntitanmoonDistance*DistanceScaler,   saturnlapetusmoonSize*SizeScaler,   saturnlapetusmoonDistance*DistanceScaler);

			// Saturn Ring
			//void SaturnRing(float RingOuterRadious, float RingLocation, float Eccentricity, float RingInnerRadious, float SideSmooth)
			SaturnRing(  SaturnRingOuterRadius * SizeScaler,   Saturn_distance*DistanceScaler ,   1.0,   SaturnRingInnerRadius * SizeScaler,   100);

			// Uranus Object
			//UranusObject(float UranusSize, float UranusLocation, float UmbrielSize, float UmbrielLocation, float TitaniaSize, float TitaniaLocation, float OberonSize, float OberonLocation)
			UranusObject(  UranusSize * SizeScaler,   Uranus_distance*DistanceScaler,   uranusumbrielmoonSize*SizeScaler,   uranusumbrielmoonDistance*DistanceScaler,   uranustitaniamoonSize*SizeScaler,   uranustitaniamoonDistance*DistanceScaler,   uranusoberonmoonSize*SizeScaler,   uranusoberonmoonDistance*DistanceScaler);

			// Nebtune Object
			//NebtuneObject(float NebtuneSize, float NebtuneLocation, float TritonSize, float TritonLocation )
			NebtuneObject(  NeptuneSize * SizeScaler,   Neptune_distance*DistanceScaler, nebtunemoontritonSize *SizeScaler, nebtunemoontritonDistance*DistanceScaler);

			//Pluto Object
			//PlutoObject(float PlutoSize, float PlutoLocation, float CharonSize, float CharonLocation)
			PlutoObject(  PlutoSize * SizeScaler,   Pluto_distance*DistanceScaler,   plutomooncharonSize*SizeScaler,   plutocmoonharonDistance*DistanceScaler);

		}

		// Earth Object
		else if (WhichPlanet == Earth)
		{

			//Caculate the earth postion
			GLfloat JupiterPos[3] = {Earth_distance*DistanceScaler * cos(-earth * M_PI / 180), 0, Earth_distance*DistanceScaler * sin(-earth * M_PI / 180)};
			//Caculate the Camera Position
			GLfloat cameraPos[3] = {Earth_distance*DistanceScaler * cos(-earth * M_PI / 180), (1.5*SizeScaler), Earth_distance*DistanceScaler * sin(-earth * M_PI / 180)};
			//Setup the camear on the top of the moon pointing 
			gluLookAt(cameraPos[0], cameraPos[1], cameraPos[2]+ (6*SizeScaler), JupiterPos[0], JupiterPos[1], JupiterPos[2]-(2.2*SizeScaler), 0, 0, -1);
			// Jupiter Planet

			SetPointLight(GL_LIGHT1,0.0,0.0,0.0,1,1,.9);
			// Earth Object
			EarthObject(  EarthSize * SizeScaler,   Earth_distance*DistanceScaler,   earthMoonSize*SizeScaler,   earthMoonDistance*DistanceScaler);


			// Space Object
			//SpaceObject(float SpaceSize)
			SpaceObject( 200000 );

			// Sun Object
			//SunObject(float SunSize)
			SunObject( SunSize * SizeScaler);

			//Mercury Object
			//MercuryObject(float MercurySize, float MercuryLocation)
			MercuryObject(MercurySize*SizeScaler, mercurydistance*DistanceScaler);

			// Venus Object
			//VenusObject(float VenusSize, float VenusLocation)
			VenusObject(  VenusSize * SizeScaler,   Venus_distance*DistanceScaler);

			// Earth Object
			//EarthObject(float EarthSize, float EarthLocation, float MoonSize, float Moon_Location)
			//EarthObject(  EarthSize * SizeScaler,   Earth_distance*DistanceScaler,   earthMoonSize*SizeScaler,   earthMoonDistance*DistanceScaler);

			//Mars Object
			//MarsObject(float MarsSize, float MarsLocation, float PhobosSize, float PhobosLocation , float DiemosSize , float DiemosLocaion)
			MarsObject(  MarsSize * SizeScaler,   Mars_distance*DistanceScaler,   marsdiemosmoonSize* SizeScaler,   marsdiemosmoonDistance * DistanceScaler,   marsphobosmoonSize *SizeScaler,   marsphobosmoonDistance* DistanceScaler);

			// Jupiter Object
			//JupiterObject(float JubiterSize, float JubiterLocation, float IoSize, float IoLocation, float EuropaSize, float EuropaLocation, float GanymedeSize, float GanymedeLocation, float CallistoSize, float CallistoLocation)
			JupiterObject(  JupitureSize * SizeScaler,   Jupiture_distance*DistanceScaler,   jupiteriomoonSize *SizeScaler,  jupiteriomoonDistance*DistanceScaler,   jupitereuropamoonSize * SizeScaler,   jupitereuropamoonDistance*DistanceScaler,   jupiterganymedemoonSize * SizeScaler,   jupiterganymedemoonDistance*DistanceScaler,   jupitercallistomoonSize  * SizeScaler,   jupitercallistomoonDistance*DistanceScaler);

			//Saturn Object
			//void SaturnObejct(float SaturnSize, float SaturnLocation, float MimasSize, float MimasLocation, float EnceladusSize, float EnceladusLocation, float TrlhysSize, float TrlhysLocation, float DioneSize, float DioneLocation, float RheaSize, float RheaLocation, float TitanSize, float TitanLocation, float LapetusSize, float LapetusLocation)
			SaturnObejct(  SaturnSize * SizeScaler,   Saturn_distance*DistanceScaler,   saturnmimasmooonSize*SizeScaler,   saturnmimasmooonDistance*DistanceScaler,   saturnenceladusmoonSize*SizeScaler,   saturnenceladusmoonDistance*DistanceScaler,   saturntrlhysmoonSize*SizeScaler,   saturntrlhysmoonDistance*DistanceScaler,   saturndionemoonSize*SizeScaler,   saturndionemoonDistance*DistanceScaler,   saturnrheamoonSize*SizeScaler,   saturnrheamoonDistance*DistanceScaler,   saturntitanmoonSize*SizeScaler,   saturntitanmoonDistance*DistanceScaler,   saturnlapetusmoonSize*SizeScaler,   saturnlapetusmoonDistance*DistanceScaler);

			// Saturn Ring
			//void SaturnRing(float RingOuterRadious, float RingLocation, float Eccentricity, float RingInnerRadious, float SideSmooth)
			SaturnRing(  SaturnRingOuterRadius * SizeScaler,   Saturn_distance*DistanceScaler ,   1.0,   SaturnRingInnerRadius * SizeScaler,   100);

			// Uranus Object
			//UranusObject(float UranusSize, float UranusLocation, float UmbrielSize, float UmbrielLocation, float TitaniaSize, float TitaniaLocation, float OberonSize, float OberonLocation)
			UranusObject(  UranusSize * SizeScaler,   Uranus_distance*DistanceScaler,   uranusumbrielmoonSize*SizeScaler,   uranusumbrielmoonDistance*DistanceScaler,   uranustitaniamoonSize*SizeScaler,   uranustitaniamoonDistance*DistanceScaler,   uranusoberonmoonSize*SizeScaler,   uranusoberonmoonDistance*DistanceScaler);

			// Nebtune Object
			//NebtuneObject(float NebtuneSize, float NebtuneLocation, float TritonSize, float TritonLocation )
			NebtuneObject(  NeptuneSize * SizeScaler,   Neptune_distance*DistanceScaler, nebtunemoontritonSize *SizeScaler, nebtunemoontritonDistance*DistanceScaler);

			//Pluto Object
			//PlutoObject(float PlutoSize, float PlutoLocation, float CharonSize, float CharonLocation)
			PlutoObject(  PlutoSize * SizeScaler,   Pluto_distance*DistanceScaler,   plutomooncharonSize*SizeScaler,   plutocmoonharonDistance*DistanceScaler);

		}
		//Mars Object
		else if (WhichPlanet == Mars)
		{
			//Caculate the earth postion
			GLfloat MarsPos[3] = {Mars_distance*DistanceScaler * cos(-mars * M_PI / 180), 0, Mars_distance*DistanceScaler * sin(-mars * M_PI / 180)};
			//Caculate the Camera Position
			GLfloat cameraPos[3] = {Mars_distance*DistanceScaler * cos(-mars * M_PI / 180), (.6*SizeScaler), Mars_distance*DistanceScaler * sin(-mars * M_PI / 180)};
			//Setup the camear on the top of the moon pointing 
			gluLookAt(cameraPos[0], cameraPos[1], cameraPos[2]+(2*SizeScaler), MarsPos[0], MarsPos[1], MarsPos[2]-(1*SizeScaler), 0, 0, -1);


			SetPointLight(GL_LIGHT1,0.0,0.0,0.0,1,1,.9);
			// Mars Object
			MarsObject(  MarsSize * SizeScaler,   Mars_distance*DistanceScaler,   marsdiemosmoonSize* SizeScaler,   marsdiemosmoonDistance * DistanceScaler,   marsphobosmoonSize *SizeScaler,   marsphobosmoonDistance* DistanceScaler);

			// Space Object
			//SpaceObject(float SpaceSize)
			SpaceObject( 200000 );

			// Sun Object
			//SunObject(float SunSize)
			SunObject( SunSize * SizeScaler);

			//Mercury Object
			//MercuryObject(float MercurySize, float MercuryLocation)
			MercuryObject(MercurySize*SizeScaler, mercurydistance*DistanceScaler);

			// Venus Object
			//VenusObject(float VenusSize, float VenusLocation)
			VenusObject(  VenusSize * SizeScaler,   Venus_distance*DistanceScaler);

			// Earth Object
			//EarthObject(float EarthSize, float EarthLocation, float MoonSize, float Moon_Location)
			EarthObject(  EarthSize * SizeScaler,   Earth_distance*DistanceScaler,   earthMoonSize*SizeScaler,   earthMoonDistance*DistanceScaler);

			//Mars Object
			//MarsObject(float MarsSize, float MarsLocation, float PhobosSize, float PhobosLocation , float DiemosSize , float DiemosLocaion)
			//MarsObject(  MarsSize * SizeScaler,   Mars_distance*DistanceScaler,   marsdiemosmoonSize* SizeScaler,   marsdiemosmoonDistance * DistanceScaler,   marsphobosmoonSize *SizeScaler,   marsphobosmoonDistance* DistanceScaler);

			// Jupiter Object
			//JupiterObject(float JubiterSize, float JubiterLocation, float IoSize, float IoLocation, float EuropaSize, float EuropaLocation, float GanymedeSize, float GanymedeLocation, float CallistoSize, float CallistoLocation)
			JupiterObject(  JupitureSize * SizeScaler,   Jupiture_distance*DistanceScaler,   jupiteriomoonSize *SizeScaler,  jupiteriomoonDistance*DistanceScaler,   jupitereuropamoonSize * SizeScaler,   jupitereuropamoonDistance*DistanceScaler,   jupiterganymedemoonSize * SizeScaler,   jupiterganymedemoonDistance*DistanceScaler,   jupitercallistomoonSize  * SizeScaler,   jupitercallistomoonDistance*DistanceScaler);

			//Saturn Object
			//void SaturnObejct(float SaturnSize, float SaturnLocation, float MimasSize, float MimasLocation, float EnceladusSize, float EnceladusLocation, float TrlhysSize, float TrlhysLocation, float DioneSize, float DioneLocation, float RheaSize, float RheaLocation, float TitanSize, float TitanLocation, float LapetusSize, float LapetusLocation)
			SaturnObejct(  SaturnSize * SizeScaler,   Saturn_distance*DistanceScaler,   saturnmimasmooonSize*SizeScaler,   saturnmimasmooonDistance*DistanceScaler,   saturnenceladusmoonSize*SizeScaler,   saturnenceladusmoonDistance*DistanceScaler,   saturntrlhysmoonSize*SizeScaler,   saturntrlhysmoonDistance*DistanceScaler,   saturndionemoonSize*SizeScaler,   saturndionemoonDistance*DistanceScaler,   saturnrheamoonSize*SizeScaler,   saturnrheamoonDistance*DistanceScaler,   saturntitanmoonSize*SizeScaler,   saturntitanmoonDistance*DistanceScaler,   saturnlapetusmoonSize*SizeScaler,   saturnlapetusmoonDistance*DistanceScaler);

			// Saturn Ring
			//void SaturnRing(float RingOuterRadious, float RingLocation, float Eccentricity, float RingInnerRadious, float SideSmooth)
			SaturnRing(  SaturnRingOuterRadius * SizeScaler,   Saturn_distance*DistanceScaler ,   1.0,   SaturnRingInnerRadius * SizeScaler,   100);

			// Uranus Object
			//UranusObject(float UranusSize, float UranusLocation, float UmbrielSize, float UmbrielLocation, float TitaniaSize, float TitaniaLocation, float OberonSize, float OberonLocation)
			UranusObject(  UranusSize * SizeScaler,   Uranus_distance*DistanceScaler,   uranusumbrielmoonSize*SizeScaler,   uranusumbrielmoonDistance*DistanceScaler,   uranustitaniamoonSize*SizeScaler,   uranustitaniamoonDistance*DistanceScaler,   uranusoberonmoonSize*SizeScaler,   uranusoberonmoonDistance*DistanceScaler);

			// Nebtune Object
			//NebtuneObject(float NebtuneSize, float NebtuneLocation, float TritonSize, float TritonLocation )
			NebtuneObject(  NeptuneSize * SizeScaler,   Neptune_distance*DistanceScaler, nebtunemoontritonSize *SizeScaler, nebtunemoontritonDistance*DistanceScaler);

			//Pluto Object
			//PlutoObject(float PlutoSize, float PlutoLocation, float CharonSize, float CharonLocation)
			PlutoObject(  PlutoSize * SizeScaler,   Pluto_distance*DistanceScaler,   plutomooncharonSize*SizeScaler,   plutocmoonharonDistance*DistanceScaler);

		}
		//Saturn Object
		else if (WhichPlanet == Saturn)
		{


			//Caculate the earth postion
			GLfloat SaturnPos[3] = {Saturn_distance*DistanceScaler * cos(-saturn * M_PI / 180), 0, Saturn_distance*DistanceScaler * sin(-saturn * M_PI / 180)};
			//Caculate the Camera Position
			GLfloat cameraPos[3] = {Saturn_distance*DistanceScaler * cos(-saturn * M_PI / 180), (4), Saturn_distance*DistanceScaler * sin(-saturn * M_PI / 180)};
			//Setup the camear on the top of the moon pointing 
			gluLookAt(cameraPos[0], cameraPos[1], cameraPos[2]+(80*SizeScaler), SaturnPos[0], SaturnPos[1], SaturnPos[2]-(10), 0, 0, -1);
			// Jupiter Planet

			// Space Object
			//SpaceObject(float SpaceSize)
			SpaceObject( 200000 );

			SetPointLight(GL_LIGHT1,0.0,0.0,0.0,1,1,.9);
			//SetMaterial(1,1,1,.2);
			//Saturn Object
			//void SaturnObejct(float SaturnSize, float SaturnLocation, float MimasSize, float MimasLocation, float EnceladusSize, float EnceladusLocation, float TrlhysSize, float TrlhysLocation, float DioneSize, float DioneLocation, float RheaSize, float RheaLocation, float TitanSize, float TitanLocation, float LapetusSize, float LapetusLocation)
			SaturnObejct(  SaturnSize * SizeScaler,   Saturn_distance*DistanceScaler,   saturnmimasmooonSize*SizeScaler,   saturnmimasmooonDistance*DistanceScaler,   saturnenceladusmoonSize*SizeScaler,   saturnenceladusmoonDistance*DistanceScaler,   saturntrlhysmoonSize*SizeScaler,   saturntrlhysmoonDistance*DistanceScaler,   saturndionemoonSize*SizeScaler,   saturndionemoonDistance*DistanceScaler,   saturnrheamoonSize*SizeScaler,   saturnrheamoonDistance*DistanceScaler,   saturntitanmoonSize*SizeScaler,   saturntitanmoonDistance*DistanceScaler,   saturnlapetusmoonSize*SizeScaler,   saturnlapetusmoonDistance*DistanceScaler);

			// Saturn Ring
			//void SaturnRing(float RingOuterRadious, float RingLocation, float Eccentricity, float RingInnerRadious, float SideSmooth)
			SaturnRing(  SaturnRingOuterRadius * SizeScaler,   Saturn_distance*DistanceScaler ,   1.0,   SaturnRingInnerRadius * SizeScaler,   100);

			// Sun Object
			//SunObject(float SunSize)
			SunObject( SunSize * SizeScaler);

			//Mercury Object
			//MercuryObject(float MercurySize, float MercuryLocation)
			MercuryObject(MercurySize*SizeScaler, mercurydistance*DistanceScaler);

			// Venus Object
			//VenusObject(float VenusSize, float VenusLocation)
			VenusObject(  VenusSize * SizeScaler,   Venus_distance*DistanceScaler);

			// Earth Object
			//EarthObject(float EarthSize, float EarthLocation, float MoonSize, float Moon_Location)
			EarthObject(  EarthSize * SizeScaler,   Earth_distance*DistanceScaler,   earthMoonSize*SizeScaler,   earthMoonDistance*DistanceScaler);

			//Mars Object
			//MarsObject(float MarsSize, float MarsLocation, float PhobosSize, float PhobosLocation , float DiemosSize , float DiemosLocaion)
			MarsObject(  MarsSize * SizeScaler,   Mars_distance*DistanceScaler,   marsdiemosmoonSize* SizeScaler,   marsdiemosmoonDistance * DistanceScaler,   marsphobosmoonSize *SizeScaler,   marsphobosmoonDistance* DistanceScaler);

			// Jupiter Object
			//JupiterObject(float JubiterSize, float JubiterLocation, float IoSize, float IoLocation, float EuropaSize, float EuropaLocation, float GanymedeSize, float GanymedeLocation, float CallistoSize, float CallistoLocation)
			JupiterObject(  JupitureSize * SizeScaler,   Jupiture_distance*DistanceScaler,   jupiteriomoonSize *SizeScaler,  jupiteriomoonDistance*DistanceScaler,   jupitereuropamoonSize * SizeScaler,   jupitereuropamoonDistance*DistanceScaler,   jupiterganymedemoonSize * SizeScaler,   jupiterganymedemoonDistance*DistanceScaler,   jupitercallistomoonSize  * SizeScaler,   jupitercallistomoonDistance*DistanceScaler);

			//Saturn Object
			//void SaturnObejct(float SaturnSize, float SaturnLocation, float MimasSize, float MimasLocation, float EnceladusSize, float EnceladusLocation, float TrlhysSize, float TrlhysLocation, float DioneSize, float DioneLocation, float RheaSize, float RheaLocation, float TitanSize, float TitanLocation, float LapetusSize, float LapetusLocation)
			//SaturnObejct(  SaturnSize * SizeScaler,   Saturn_distance*DistanceScaler,   saturnmimasmooonSize*SizeScaler,   saturnmimasmooonDistance*DistanceScaler,   saturnenceladusmoonSize*SizeScaler,   saturnenceladusmoonDistance*DistanceScaler,   saturntrlhysmoonSize*SizeScaler,   saturntrlhysmoonDistance*DistanceScaler,   saturndionemoonSize*SizeScaler,   saturndionemoonDistance*DistanceScaler,   saturnrheamoonSize*SizeScaler,   saturnrheamoonDistance*DistanceScaler,   saturntitanmoonSize*SizeScaler,   saturntitanmoonDistance*DistanceScaler,   saturnlapetusmoonSize*SizeScaler,   saturnlapetusmoonDistance*DistanceScaler);

			// Saturn Ring
			//void SaturnRing(float RingOuterRadious, float RingLocation, float Eccentricity, float RingInnerRadious, float SideSmooth)
			//SaturnRing(  SaturnRingOuterRadius * SizeScaler,   Saturn_distance*DistanceScaler ,   1.0,   SaturnRingInnerRadius * SizeScaler,   100);

			// Uranus Object
			//UranusObject(float UranusSize, float UranusLocation, float UmbrielSize, float UmbrielLocation, float TitaniaSize, float TitaniaLocation, float OberonSize, float OberonLocation)
			UranusObject(  UranusSize * SizeScaler,   Uranus_distance*DistanceScaler,   uranusumbrielmoonSize*SizeScaler,   uranusumbrielmoonDistance*DistanceScaler,   uranustitaniamoonSize*SizeScaler,   uranustitaniamoonDistance*DistanceScaler,   uranusoberonmoonSize*SizeScaler,   uranusoberonmoonDistance*DistanceScaler);

			// Nebtune Object
			//NebtuneObject(float NebtuneSize, float NebtuneLocation, float TritonSize, float TritonLocation )
			NebtuneObject(  NeptuneSize * SizeScaler,   Neptune_distance*DistanceScaler, nebtunemoontritonSize *SizeScaler, nebtunemoontritonDistance*DistanceScaler);

			//Pluto Object
			//PlutoObject(float PlutoSize, float PlutoLocation, float CharonSize, float CharonLocation)
			PlutoObject(  PlutoSize * SizeScaler,   Pluto_distance*DistanceScaler,   plutomooncharonSize*SizeScaler,   plutocmoonharonDistance*DistanceScaler);

		}
		// Uranus Object
		else if (WhichPlanet == Uranus)
		{

			//Caculate the earth postion
			GLfloat UranusPos[3] = {Uranus_distance*DistanceScaler * cos(-uranus * M_PI / 180), 0, Uranus_distance*DistanceScaler * sin(-uranus * M_PI / 180)};
			//Caculate the Camera Position
			GLfloat cameraPos[3] = {Uranus_distance*DistanceScaler * cos(-uranus * M_PI / 180), (8*SizeScaler), Uranus_distance*DistanceScaler * sin(-uranus * M_PI / 180)};
			//Setup the camear on the top of the moon pointing 
			gluLookAt(cameraPos[0], cameraPos[1], cameraPos[2]+(20*SizeScaler), UranusPos[0], UranusPos[1], UranusPos[2]-(11*SizeScaler), 0, 0, -1);

			SetPointLight(GL_LIGHT1,0.0,0.0,0.0,1,1,.9);
			//SetMaterial(1,1,1,.2);
			//Saturn Object
			// Uranus Planet
			UranusObject(  UranusSize * SizeScaler,   Uranus_distance*DistanceScaler,   uranusumbrielmoonSize*SizeScaler,   uranusumbrielmoonDistance*DistanceScaler,   uranustitaniamoonSize*SizeScaler,   uranustitaniamoonDistance*DistanceScaler,   uranusoberonmoonSize*SizeScaler,   uranusoberonmoonDistance*DistanceScaler);

			// Space Object
			//SpaceObject(float SpaceSize)
			SpaceObject( 200000 );

			// Sun Object
			//SunObject(float SunSize)
			SunObject( SunSize * SizeScaler);

			//Mercury Object
			//MercuryObject(float MercurySize, float MercuryLocation)
			MercuryObject(MercurySize*SizeScaler, mercurydistance*DistanceScaler);

			// Venus Object
			//VenusObject(float VenusSize, float VenusLocation)
			VenusObject(  VenusSize * SizeScaler,   Venus_distance*DistanceScaler);

			// Earth Object
			//EarthObject(float EarthSize, float EarthLocation, float MoonSize, float Moon_Location)
			EarthObject(  EarthSize * SizeScaler,   Earth_distance*DistanceScaler,   earthMoonSize*SizeScaler,   earthMoonDistance*DistanceScaler);

			//Mars Object
			//MarsObject(float MarsSize, float MarsLocation, float PhobosSize, float PhobosLocation , float DiemosSize , float DiemosLocaion)
			MarsObject(  MarsSize * SizeScaler,   Mars_distance*DistanceScaler,   marsdiemosmoonSize* SizeScaler,   marsdiemosmoonDistance * DistanceScaler,   marsphobosmoonSize *SizeScaler,   marsphobosmoonDistance* DistanceScaler);

			// Jupiter Object
			//JupiterObject(float JubiterSize, float JubiterLocation, float IoSize, float IoLocation, float EuropaSize, float EuropaLocation, float GanymedeSize, float GanymedeLocation, float CallistoSize, float CallistoLocation)
			JupiterObject(  JupitureSize * SizeScaler,   Jupiture_distance*DistanceScaler,   jupiteriomoonSize *SizeScaler,  jupiteriomoonDistance*DistanceScaler,   jupitereuropamoonSize * SizeScaler,   jupitereuropamoonDistance*DistanceScaler,   jupiterganymedemoonSize * SizeScaler,   jupiterganymedemoonDistance*DistanceScaler,   jupitercallistomoonSize  * SizeScaler,   jupitercallistomoonDistance*DistanceScaler);

			//Saturn Object
			//void SaturnObejct(float SaturnSize, float SaturnLocation, float MimasSize, float MimasLocation, float EnceladusSize, float EnceladusLocation, float TrlhysSize, float TrlhysLocation, float DioneSize, float DioneLocation, float RheaSize, float RheaLocation, float TitanSize, float TitanLocation, float LapetusSize, float LapetusLocation)
			SaturnObejct(  SaturnSize * SizeScaler,   Saturn_distance*DistanceScaler,   saturnmimasmooonSize*SizeScaler,   saturnmimasmooonDistance*DistanceScaler,   saturnenceladusmoonSize*SizeScaler,   saturnenceladusmoonDistance*DistanceScaler,   saturntrlhysmoonSize*SizeScaler,   saturntrlhysmoonDistance*DistanceScaler,   saturndionemoonSize*SizeScaler,   saturndionemoonDistance*DistanceScaler,   saturnrheamoonSize*SizeScaler,   saturnrheamoonDistance*DistanceScaler,   saturntitanmoonSize*SizeScaler,   saturntitanmoonDistance*DistanceScaler,   saturnlapetusmoonSize*SizeScaler,   saturnlapetusmoonDistance*DistanceScaler);

			// Saturn Ring
			//void SaturnRing(float RingOuterRadious, float RingLocation, float Eccentricity, float RingInnerRadious, float SideSmooth)
			SaturnRing(  SaturnRingOuterRadius * SizeScaler,   Saturn_distance*DistanceScaler ,   1.0,   SaturnRingInnerRadius * SizeScaler,   100);

			// Uranus Object
			//UranusObject(float UranusSize, float UranusLocation, float UmbrielSize, float UmbrielLocation, float TitaniaSize, float TitaniaLocation, float OberonSize, float OberonLocation)
			//UranusObject(  UranusSize * SizeScaler,   Uranus_distance*DistanceScaler,   uranusumbrielmoonSize*SizeScaler,   uranusumbrielmoonDistance*DistanceScaler,   uranustitaniamoonSize*SizeScaler,   uranustitaniamoonDistance*DistanceScaler,   uranusoberonmoonSize*SizeScaler,   uranusoberonmoonDistance*DistanceScaler);

			// Nebtune Object
			//NebtuneObject(float NebtuneSize, float NebtuneLocation, float TritonSize, float TritonLocation )
			NebtuneObject(  NeptuneSize * SizeScaler,   Neptune_distance*DistanceScaler, nebtunemoontritonSize *SizeScaler, nebtunemoontritonDistance*DistanceScaler);

			//Pluto Object
			//PlutoObject(float PlutoSize, float PlutoLocation, float CharonSize, float CharonLocation)
			PlutoObject(  PlutoSize * SizeScaler,   Pluto_distance*DistanceScaler,   plutomooncharonSize*SizeScaler,   plutocmoonharonDistance*DistanceScaler);

		}
		// Nebtune Object
		else if (WhichPlanet == Nebtune)
		{

			//Caculate the earth postion
			GLfloat NebtunePos[3] = {Neptune_distance*DistanceScaler * cos(-nebtune * M_PI / 180), 0, Neptune_distance*DistanceScaler * sin(-nebtune * M_PI / 180)};
			//Caculate the Camera Position
			GLfloat cameraPos[3] = {Neptune_distance*DistanceScaler * cos(-nebtune * M_PI / 180), (7*SizeScaler), Neptune_distance*DistanceScaler * sin(-nebtune * M_PI / 180)};
			//Setup the camear on the top of the moon pointing 
			gluLookAt(cameraPos[0], cameraPos[1], cameraPos[2]+(20 *SizeScaler ), NebtunePos[0], NebtunePos[1], NebtunePos[2]-(6*SizeScaler), 0, 0, -1);

			SetPointLight(GL_LIGHT1,0.0,0.0,0.0,1,1,.9);
			//SetMaterial(1,1,1,.2);

			// Nebtun Planet
			//NebtuneObject(float NebtuneSize, float NebtuneLocation, float TritonSize, float TritonLocation )
			NebtuneObject(  NeptuneSize * SizeScaler,   Neptune_distance*DistanceScaler, nebtunemoontritonSize *SizeScaler, nebtunemoontritonDistance*DistanceScaler);


			// Space Object
			//SpaceObject(float SpaceSize)
			SpaceObject( 200000 );

			// Sun Object
			//SunObject(float SunSize)
			SunObject( SunSize * SizeScaler);

			//Mercury Object
			//MercuryObject(float MercurySize, float MercuryLocation)
			MercuryObject(MercurySize*SizeScaler, mercurydistance*DistanceScaler);

			// Venus Object
			//VenusObject(float VenusSize, float VenusLocation)
			VenusObject(  VenusSize * SizeScaler,   Venus_distance*DistanceScaler);

			// Earth Object
			//EarthObject(float EarthSize, float EarthLocation, float MoonSize, float Moon_Location)
			EarthObject(  EarthSize * SizeScaler,   Earth_distance*DistanceScaler,   earthMoonSize*SizeScaler,   earthMoonDistance*DistanceScaler);

			//Mars Object
			//MarsObject(float MarsSize, float MarsLocation, float PhobosSize, float PhobosLocation , float DiemosSize , float DiemosLocaion)
			MarsObject(  MarsSize * SizeScaler,   Mars_distance*DistanceScaler,   marsdiemosmoonSize* SizeScaler,   marsdiemosmoonDistance * DistanceScaler,   marsphobosmoonSize *SizeScaler,   marsphobosmoonDistance* DistanceScaler);

			// Jupiter Object
			//JupiterObject(float JubiterSize, float JubiterLocation, float IoSize, float IoLocation, float EuropaSize, float EuropaLocation, float GanymedeSize, float GanymedeLocation, float CallistoSize, float CallistoLocation)
			JupiterObject(  JupitureSize * SizeScaler,   Jupiture_distance*DistanceScaler,   jupiteriomoonSize *SizeScaler,  jupiteriomoonDistance*DistanceScaler,   jupitereuropamoonSize * SizeScaler,   jupitereuropamoonDistance*DistanceScaler,   jupiterganymedemoonSize * SizeScaler,   jupiterganymedemoonDistance*DistanceScaler,   jupitercallistomoonSize  * SizeScaler,   jupitercallistomoonDistance*DistanceScaler);

			//Saturn Object
			//void SaturnObejct(float SaturnSize, float SaturnLocation, float MimasSize, float MimasLocation, float EnceladusSize, float EnceladusLocation, float TrlhysSize, float TrlhysLocation, float DioneSize, float DioneLocation, float RheaSize, float RheaLocation, float TitanSize, float TitanLocation, float LapetusSize, float LapetusLocation)
			SaturnObejct(  SaturnSize * SizeScaler,   Saturn_distance*DistanceScaler,   saturnmimasmooonSize*SizeScaler,   saturnmimasmooonDistance*DistanceScaler,   saturnenceladusmoonSize*SizeScaler,   saturnenceladusmoonDistance*DistanceScaler,   saturntrlhysmoonSize*SizeScaler,   saturntrlhysmoonDistance*DistanceScaler,   saturndionemoonSize*SizeScaler,   saturndionemoonDistance*DistanceScaler,   saturnrheamoonSize*SizeScaler,   saturnrheamoonDistance*DistanceScaler,   saturntitanmoonSize*SizeScaler,   saturntitanmoonDistance*DistanceScaler,   saturnlapetusmoonSize*SizeScaler,   saturnlapetusmoonDistance*DistanceScaler);

			// Saturn Ring
			//void SaturnRing(float RingOuterRadious, float RingLocation, float Eccentricity, float RingInnerRadious, float SideSmooth)
			SaturnRing(  SaturnRingOuterRadius * SizeScaler,   Saturn_distance*DistanceScaler ,   1.0,   SaturnRingInnerRadius * SizeScaler,   100);

			// Uranus Object
			//UranusObject(float UranusSize, float UranusLocation, float UmbrielSize, float UmbrielLocation, float TitaniaSize, float TitaniaLocation, float OberonSize, float OberonLocation)
			UranusObject(  UranusSize * SizeScaler,   Uranus_distance*DistanceScaler,   uranusumbrielmoonSize*SizeScaler,   uranusumbrielmoonDistance*DistanceScaler,   uranustitaniamoonSize*SizeScaler,   uranustitaniamoonDistance*DistanceScaler,   uranusoberonmoonSize*SizeScaler,   uranusoberonmoonDistance*DistanceScaler);

			// Nebtune Object
			//NebtuneObject(float NebtuneSize, float NebtuneLocation, float TritonSize, float TritonLocation )
			//NebtuneObject(  NeptuneSize * SizeScaler,   Neptune_distance*DistanceScaler, nebtunemoontritonSize *SizeScaler, nebtunemoontritonDistance*DistanceScaler);

			//Pluto Object
			//PlutoObject(float PlutoSize, float PlutoLocation, float CharonSize, float CharonLocation)
			PlutoObject(  PlutoSize * SizeScaler,   Pluto_distance*DistanceScaler,   plutomooncharonSize*SizeScaler,   plutocmoonharonDistance*DistanceScaler);

		}
		//Pluto Object
		else if (WhichPlanet == Pluto)
		{

			//Caculate the earth postion
			GLfloat NebtunePos[3] = {Pluto_distance*DistanceScaler * cos(-pluto * M_PI / 180), 0, Pluto_distance*DistanceScaler * sin(-pluto * M_PI / 180)};
			//Caculate the Camera Position
			GLfloat cameraPos[3] = {Pluto_distance*DistanceScaler * cos(-pluto * M_PI / 180), (7*SizeScaler), Pluto_distance*DistanceScaler * sin(-pluto * M_PI / 180)};
			//Setup the camear on the top of the moon pointing 
			gluLookAt(cameraPos[0], cameraPos[1], cameraPos[2]+(20*SizeScaler), NebtunePos[0], NebtunePos[1], NebtunePos[2]-(6*SizeScaler), 0, 0, -1);

			SetPointLight(GL_LIGHT1,0.0,0.0,0.0,1,1,.9);
			//SetMaterial(1,1,1,.2);

			//Pluto Object
			PlutoObject(  PlutoSize * SizeScaler,   Pluto_distance*DistanceScaler,   plutomooncharonSize*SizeScaler,   plutocmoonharonDistance*DistanceScaler);

			// Space Object
			//SpaceObject(float SpaceSize)
			SpaceObject( 200000 );

			// Sun Object
			//SunObject(float SunSize)
			SunObject( SunSize * SizeScaler);

			//Mercury Object
			//MercuryObject(float MercurySize, float MercuryLocation)
			MercuryObject(MercurySize*SizeScaler, mercurydistance*DistanceScaler);

			// Venus Object
			//VenusObject(float VenusSize, float VenusLocation)
			VenusObject(  VenusSize * SizeScaler,   Venus_distance*DistanceScaler);

			// Earth Object
			//EarthObject(float EarthSize, float EarthLocation, float MoonSize, float Moon_Location)
			EarthObject(  EarthSize * SizeScaler,   Earth_distance*DistanceScaler,   earthMoonSize*SizeScaler,   earthMoonDistance*DistanceScaler);

			//Mars Object
			//MarsObject(float MarsSize, float MarsLocation, float PhobosSize, float PhobosLocation , float DiemosSize , float DiemosLocaion)
			MarsObject(  MarsSize * SizeScaler,   Mars_distance*DistanceScaler,   marsdiemosmoonSize* SizeScaler,   marsdiemosmoonDistance * DistanceScaler,   marsphobosmoonSize *SizeScaler,   marsphobosmoonDistance* DistanceScaler);

			// Jupiter Object
			//JupiterObject(float JubiterSize, float JubiterLocation, float IoSize, float IoLocation, float EuropaSize, float EuropaLocation, float GanymedeSize, float GanymedeLocation, float CallistoSize, float CallistoLocation)
			JupiterObject(  JupitureSize * SizeScaler,   Jupiture_distance*DistanceScaler,   jupiteriomoonSize *SizeScaler,  jupiteriomoonDistance*DistanceScaler,   jupitereuropamoonSize * SizeScaler,   jupitereuropamoonDistance*DistanceScaler,   jupiterganymedemoonSize * SizeScaler,   jupiterganymedemoonDistance*DistanceScaler,   jupitercallistomoonSize  * SizeScaler,   jupitercallistomoonDistance*DistanceScaler);

			//Saturn Object
			//void SaturnObejct(float SaturnSize, float SaturnLocation, float MimasSize, float MimasLocation, float EnceladusSize, float EnceladusLocation, float TrlhysSize, float TrlhysLocation, float DioneSize, float DioneLocation, float RheaSize, float RheaLocation, float TitanSize, float TitanLocation, float LapetusSize, float LapetusLocation)
			SaturnObejct(  SaturnSize * SizeScaler,   Saturn_distance*DistanceScaler,   saturnmimasmooonSize*SizeScaler,   saturnmimasmooonDistance*DistanceScaler,   saturnenceladusmoonSize*SizeScaler,   saturnenceladusmoonDistance*DistanceScaler,   saturntrlhysmoonSize*SizeScaler,   saturntrlhysmoonDistance*DistanceScaler,   saturndionemoonSize*SizeScaler,   saturndionemoonDistance*DistanceScaler,   saturnrheamoonSize*SizeScaler,   saturnrheamoonDistance*DistanceScaler,   saturntitanmoonSize*SizeScaler,   saturntitanmoonDistance*DistanceScaler,   saturnlapetusmoonSize*SizeScaler,   saturnlapetusmoonDistance*DistanceScaler);

			// Saturn Ring
			//void SaturnRing(float RingOuterRadious, float RingLocation, float Eccentricity, float RingInnerRadious, float SideSmooth)
			SaturnRing(  SaturnRingOuterRadius * SizeScaler,   Saturn_distance*DistanceScaler ,   1.0,   SaturnRingInnerRadius * SizeScaler,   100);

			// Uranus Object
			//UranusObject(float UranusSize, float UranusLocation, float UmbrielSize, float UmbrielLocation, float TitaniaSize, float TitaniaLocation, float OberonSize, float OberonLocation)
			UranusObject(  UranusSize * SizeScaler,   Uranus_distance*DistanceScaler,   uranusumbrielmoonSize*SizeScaler,   uranusumbrielmoonDistance*DistanceScaler,   uranustitaniamoonSize*SizeScaler,   uranustitaniamoonDistance*DistanceScaler,   uranusoberonmoonSize*SizeScaler,   uranusoberonmoonDistance*DistanceScaler);

			// Nebtune Object
			//NebtuneObject(float NebtuneSize, float NebtuneLocation, float TritonSize, float TritonLocation )
			NebtuneObject(  NeptuneSize * SizeScaler,   Neptune_distance*DistanceScaler, nebtunemoontritonSize *SizeScaler, nebtunemoontritonDistance*DistanceScaler);

			//Pluto Object
			//PlutoObject(float PlutoSize, float PlutoLocation, float CharonSize, float CharonLocation)
			//PlutoObject(  PlutoSize * SizeScaler,   Pluto_distance*DistanceScaler,   plutomooncharonSize*SizeScaler,   plutocmoonharonDistance*DistanceScaler);

		}
		else if (WhichPlanet == Mercury)
		{

			//Caculate the earth postion
			GLfloat MercuryPos[3] = {mercurydistance*DistanceScaler * cos(-mercury * M_PI / 180), 0, mercurydistance*DistanceScaler * sin(-mercury * M_PI / 180)};
			//Caculate the Camera Position
			GLfloat cameraPos[3] = {mercurydistance*DistanceScaler * cos(-mercury * M_PI / 180), (.5*SizeScaler), mercurydistance*DistanceScaler * sin(-mercury * M_PI / 180)};
			//Setup the camear on the top of the moon pointing 
			gluLookAt(cameraPos[0], cameraPos[1], cameraPos[2]+(3*SizeScaler), MercuryPos[0], MercuryPos[1], MercuryPos[2]-(1*SizeScaler), 0, 0, -1);

			SetPointLight(GL_LIGHT1,0.0,0.0,0.0,1,1,.9);

			MercuryObject(MercurySize*SizeScaler, mercurydistance*DistanceScaler);

			// Space Object
			//SpaceObject(float SpaceSize)
			SpaceObject( 200000 );

			// Sun Object
			//SunObject(float SunSize)
			SunObject( SunSize * SizeScaler);

			//Mercury Object
			//MercuryObject(float MercurySize, float MercuryLocation)
			//MercuryObject(MercurySize*SizeScaler, mercurydistance*DistanceScaler);

			// Venus Object
			//VenusObject(float VenusSize, float VenusLocation)
			VenusObject(  VenusSize * SizeScaler,   Venus_distance*DistanceScaler);

			// Earth Object
			//EarthObject(float EarthSize, float EarthLocation, float MoonSize, float Moon_Location)
			EarthObject(  EarthSize * SizeScaler,   Earth_distance*DistanceScaler,   earthMoonSize*SizeScaler,   earthMoonDistance*DistanceScaler);

			//Mars Object
			//MarsObject(float MarsSize, float MarsLocation, float PhobosSize, float PhobosLocation , float DiemosSize , float DiemosLocaion)
			MarsObject(  MarsSize * SizeScaler,   Mars_distance*DistanceScaler,   marsdiemosmoonSize* SizeScaler,   marsdiemosmoonDistance * DistanceScaler,   marsphobosmoonSize *SizeScaler,   marsphobosmoonDistance* DistanceScaler);

			// Jupiter Object
			//JupiterObject(float JubiterSize, float JubiterLocation, float IoSize, float IoLocation, float EuropaSize, float EuropaLocation, float GanymedeSize, float GanymedeLocation, float CallistoSize, float CallistoLocation)
			JupiterObject(  JupitureSize * SizeScaler,   Jupiture_distance*DistanceScaler,   jupiteriomoonSize *SizeScaler,  jupiteriomoonDistance*DistanceScaler,   jupitereuropamoonSize * SizeScaler,   jupitereuropamoonDistance*DistanceScaler,   jupiterganymedemoonSize * SizeScaler,   jupiterganymedemoonDistance*DistanceScaler,   jupitercallistomoonSize  * SizeScaler,   jupitercallistomoonDistance*DistanceScaler);

			//Saturn Object
			//void SaturnObejct(float SaturnSize, float SaturnLocation, float MimasSize, float MimasLocation, float EnceladusSize, float EnceladusLocation, float TrlhysSize, float TrlhysLocation, float DioneSize, float DioneLocation, float RheaSize, float RheaLocation, float TitanSize, float TitanLocation, float LapetusSize, float LapetusLocation)
			SaturnObejct(  SaturnSize * SizeScaler,   Saturn_distance*DistanceScaler,   saturnmimasmooonSize*SizeScaler,   saturnmimasmooonDistance*DistanceScaler,   saturnenceladusmoonSize*SizeScaler,   saturnenceladusmoonDistance*DistanceScaler,   saturntrlhysmoonSize*SizeScaler,   saturntrlhysmoonDistance*DistanceScaler,   saturndionemoonSize*SizeScaler,   saturndionemoonDistance*DistanceScaler,   saturnrheamoonSize*SizeScaler,   saturnrheamoonDistance*DistanceScaler,   saturntitanmoonSize*SizeScaler,   saturntitanmoonDistance*DistanceScaler,   saturnlapetusmoonSize*SizeScaler,   saturnlapetusmoonDistance*DistanceScaler);

			// Saturn Ring
			//void SaturnRing(float RingOuterRadious, float RingLocation, float Eccentricity, float RingInnerRadious, float SideSmooth)
			SaturnRing(  SaturnRingOuterRadius * SizeScaler,   Saturn_distance*DistanceScaler ,   1.0,   SaturnRingInnerRadius * SizeScaler,   100);

			// Uranus Object
			//UranusObject(float UranusSize, float UranusLocation, float UmbrielSize, float UmbrielLocation, float TitaniaSize, float TitaniaLocation, float OberonSize, float OberonLocation)
			UranusObject(  UranusSize * SizeScaler,   Uranus_distance*DistanceScaler,   uranusumbrielmoonSize*SizeScaler,   uranusumbrielmoonDistance*DistanceScaler,   uranustitaniamoonSize*SizeScaler,   uranustitaniamoonDistance*DistanceScaler,   uranusoberonmoonSize*SizeScaler,   uranusoberonmoonDistance*DistanceScaler);

			// Nebtune Object
			//NebtuneObject(float NebtuneSize, float NebtuneLocation, float TritonSize, float TritonLocation )
			NebtuneObject(  NeptuneSize * SizeScaler,   Neptune_distance*DistanceScaler, nebtunemoontritonSize *SizeScaler, nebtunemoontritonDistance*DistanceScaler);

			//Pluto Object
			//PlutoObject(float PlutoSize, float PlutoLocation, float CharonSize, float CharonLocation)
			PlutoObject(  PlutoSize * SizeScaler,   Pluto_distance*DistanceScaler,   plutomooncharonSize*SizeScaler,   plutocmoonharonDistance*DistanceScaler);

		}
	}


	// Camera on at the moon



	// draw some gratuitous text that just rotates on top of the scene:

	glDisable( GL_DEPTH_TEST );
	glColor3f( 0., 1., 1. );
	//	DoRasterString( 0., 1., 0., "Text That Moves" );


	// draw some gratuitous text that is fixed on the screen:
	//
	// the projection matrix is reset to define a scene whose
	// world coordinate system goes from 0-100 in each axis
	//
	// this is called "percent units", and is just a convenience
	//
	// the modelview matrix is reset to identity as we don't
	// want to transform these coordinates

	if( Light1On )
		glEnable( GL_LIGHT1 );
	else
		glDisable( GL_LIGHT1 );
	glDisable( GL_LIGHTING );

	glDisable( GL_DEPTH_TEST );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	gluOrtho2D( 0., 100.,     0., 100. );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );
	glColor3f( 1., 1., 1. );
	DoRasterString( 0., 3.0, 0., "The Sun" );


	// swap the double-buffered framebuffers:

	glutSwapBuffers( );


	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush( ) here, not glFinish( ) !

	glFlush( );

}



//
// use glut to display a string of characters using a raster font:
//

void
	DoRasterString( float x, float y, float z, char *s )
{
	char c;			// one character to print

	glRasterPos3f( (GLfloat)x, (GLfloat)y, (GLfloat)z );
	for( ; ( c = *s ) != '\0'; s++ )
	{
		glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, c );
	}
}



//
// use glut to display a string of characters using a stroke font:
//

void
	DoStrokeString( float x, float y, float z, float ht, char *s )
{
	char c;			// one character to print

	glPushMatrix( );
	glTranslatef( (GLfloat)x, (GLfloat)y, (GLfloat)z );
	float sf = ht / ( 119.05 + 33.33 );
	glScalef( (GLfloat)sf, (GLfloat)sf, (GLfloat)sf );
	for( ; ( c = *s ) != '\0'; s++ )
	{
		glutStrokeCharacter( GLUT_STROKE_ROMAN, c );
	}
	glPopMatrix( );
}



//
// return the number of seconds since the start of the program:
//

float
	ElapsedSeconds( void )
{
	// get # of milliseconds since the start of the program:

	int ms = glutGet( GLUT_ELAPSED_TIME );

	// convert it to seconds:

	return (float)ms / 1000.;
}



//
// initialize the glui window:
//

void
	InitGlui( void )
{
	GLUI_Panel *panel;
	GLUI_RadioGroup *group;
	GLUI_Rotation *rot;
	GLUI_Translation *trans, *scale;


	// setup the glui window:

	glutInitWindowPosition( INIT_WINDOW_SIZE + 50, 0 );
	Glui = GLUI_Master.create_glui( (char *) GLUITITLE );


	Glui->add_statictext( (char *) GLUITITLE );
	Glui->add_separator( );


	Glui->add_checkbox( "Start Solar Animation", &freezebox );
	Glui->add_checkbox( "Turn Off Orbits Lines", &LinesEnabled );
	panel = Glui->add_panel(  "Choose Camera Position" );
	group = Glui->add_radiogroup_to_panel( panel, &WhichCamera );
	Glui->add_radiobutton_to_group( group, "Estimated Solar System" );
	Glui->add_radiobutton_to_group( group, "Propotional Solar System" );
	Glui->add_radiobutton_to_group( group, "Planet Top" );
	Glui->add_radiobutton_to_group( group, "Planet View" );
	Glui->add_radiobutton_to_group( group, "Planet and Moons View" );

	//Glui->add_checkbox( "Intensity Depth Cue", &DepthCueOn );
	Glui->add_column_to_panel(panel, GLUIFALSE);
	panel = Glui->add_panel(  "Animation Control" );
	GLUI_Spinner *kval = Glui->add_spinner_to_panel(panel, "Rotation Speed", GLUI_SPINNER_FLOAT, &TimeControl);
	kval->set_float_limits(-100.f, 100.f);
	kval->set_speed(0.20f);


	panel = Glui->add_panel(  "Planet" );
	group = Glui->add_radiogroup_to_panel( panel, &WhichPlanet );
	Glui->add_radiobutton_to_group( group, "Mercury" );
	Glui->add_radiobutton_to_group( group, "Venus" );
	Glui->add_radiobutton_to_group( group, "Earth" );
	Glui->add_radiobutton_to_group( group, "Mars" );
	Glui->add_radiobutton_to_group( group, "Jupiter" );
	Glui->add_radiobutton_to_group( group, "Saturn" );
	Glui->add_radiobutton_to_group( group, "Uranus" );
	Glui->add_radiobutton_to_group( group, "Nebtune" );
	Glui->add_radiobutton_to_group( group, "Pluto" );


	//Glui->add_radiobutton_to_group( group, "All Planets" );
	panel = Glui->add_panel(  " Saturn Ring Transparency " );
	group = Glui->add_radiogroup_to_panel( panel, &SaturnRingTrans );
	Glui->add_radiobutton_to_group( group, "No Transparency" );
	Glui->add_radiobutton_to_group( group, "Transparency 1" );
	Glui->add_radiobutton_to_group( group, "Transparency 2" );
	Glui->add_radiobutton_to_group( group, "Transparency 3" );
	Glui->add_radiobutton_to_group( group, "Transparency 4" );
	Glui->add_radiobutton_to_group( group, "Transparency 5" );

	panel = Glui->add_panel(  "View Control" );
	rot = Glui->add_rotation_to_panel( panel, "Rotation", (float *) RotMatrix );

	// allow the object to be spun via the glui rotation widget:

	rot->set_spin( 1.0 );



	Glui->add_column_to_panel( panel, GLUIFALSE );
	scale = Glui->add_translation_to_panel( panel, "Scale",  GLUI_TRANSLATION_Y , &Scale2 );
	scale->set_speed( 0.005f );

	Glui->add_column_to_panel( panel, GLUIFALSE );
	trans = Glui->add_translation_to_panel( panel, "Trans XY", GLUI_TRANSLATION_XY, &TransXYZ[0] );
	trans->set_speed( 1.0f );

	Glui->add_column_to_panel( panel, GLUIFALSE );
	trans = Glui->add_translation_to_panel( panel, "Trans Z",  GLUI_TRANSLATION_Z , &TransXYZ[2] );
	trans->set_speed( 1.0f );

	Glui->add_checkbox( "Debug", &DebugOn );


	panel = Glui->add_panel( "", GLUIFALSE );


	Glui->add_button_to_panel( panel, "Reset", RESET, (GLUI_Update_CB) Buttons );

	Glui->add_column_to_panel( panel, GLUIFALSE );

	Glui->add_button_to_panel( panel, "Quit", QUIT, (GLUI_Update_CB) Buttons );


	// tell glui what graphics window it needs to post a redisplay to:

	Glui->set_main_gfx_window( MainWindow );


	// set the graphics window's idle function if needed:

	GLUI_Master.set_glutIdleFunc( NULL );
}



//
// initialize the glut and OpenGL libraries:
//	also setup display lists and callback functions
//

void
	InitGraphics( void )
{
	// setup the display mode:
	// ( *must* be done before call to glutCreateWindow( ) )
	// ask for color, double-buffering, and z-buffering:

	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );


	// set the initial window configuration:

	glutInitWindowPosition( 0, 0 );
	glutInitWindowSize( INIT_WINDOW_SIZE, INIT_WINDOW_SIZE );


	// open the window and set its title:

	MainWindow = glutCreateWindow( WINDOWTITLE );
	glutSetWindowTitle( WINDOWTITLE );


	// setup the clear values:

	glClearColor( BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3] );


	// setup the callback routines:


	// DisplayFunc -- redraw the window
	// ReshapeFunc -- handle the user resizing the window
	// KeyboardFunc -- handle a keyboard input
	// MouseFunc -- handle the mouse button going down or up
	// MotionFunc -- handle the mouse moving with a button down
	// PassiveMotionFunc -- handle the mouse moving with a button up
	// VisibilityFunc -- handle a change in window visibility
	// EntryFunc	-- handle the cursor entering or leaving the window
	// SpecialFunc -- handle special keys on the keyboard
	// SpaceballMotionFunc -- handle spaceball translation
	// SpaceballRotateFunc -- handle spaceball rotation
	// SpaceballButtonFunc -- handle spaceball button hits
	// ButtonBoxFunc -- handle button box hits
	// DialsFunc -- handle dial rotations
	// TabletMotionFunc -- handle digitizing tablet motion
	// TabletButtonFunc -- handle digitizing tablet button hits
	// MenuStateFunc -- declare when a pop-up menu is in use
	// TimerFunc -- trigger something to happen a certain time from now
	// IdleFunc -- what to do when nothing else is going on

	glutSetWindow( MainWindow );
	glutDisplayFunc( Display );
	glutReshapeFunc( Resize );
	glutKeyboardFunc( Keyboard );
	glutMouseFunc( MouseButton );
	glutMotionFunc( MouseMotion );
	glutPassiveMotionFunc( NULL );
	glutVisibilityFunc( Visibility );
	glutEntryFunc( NULL );
	glutSpecialFunc( NULL );
	glutSpaceballMotionFunc( NULL );
	glutSpaceballRotateFunc( NULL );
	glutSpaceballButtonFunc( NULL );
	glutButtonBoxFunc( NULL );
	glutDialsFunc( NULL );
	glutTabletMotionFunc( NULL );
	glutTabletButtonFunc( NULL );
	glutMenuStateFunc( NULL );
	glutTimerFunc( 0, NULL, 0 );

	// DO NOT SET THE GLUT IDLE FUNCTION HERE !!
	glutIdleFunc( Animate );

	Space_Texture = BmpToTexture("Space.bmp",&space_width,&space_height);

	Sun_Texture = BmpToTexture("sun.bmp",&sun_width,&sun_height);

	// Mercury Texture
	Mercury_Texture = BmpToTexture("mercury.bmp",&mercury_width,&mercury_height);

	// Venus Texture
	Venus_Texture = BmpToTexture("venus.bmp",&venus_width,&venus_height);

	Earth_Texture = BmpToTexture("worldtex.bmp",&earth_width,&earth_height);
	Moon_Texture = BmpToTexture("Moon.bmp",&moon_width,&moon_height);


	// Mars Texture
	Mars_Texture = BmpToTexture("mars.bmp",&mars_width,&mars_height);
	Diemos_Texture = BmpToTexture("diemos.bmp ",&diemos_width ,&diemos_height);
	Phobos_Texture = BmpToTexture("phobos.bmp",&phobos_width ,&phobos_height);

	// Jupiter Texture
	Jupiter_Texture = BmpToTexture("jupiter.bmp",&jupiter_width,&jupiter_height);
	Io_Texture = BmpToTexture("io.bmp",&io_width,&io_height);
	Europa_Texture = BmpToTexture("europa.bmp",&europa_width,&europa_height);
	Callisto_Texture = BmpToTexture("callisto.bmp",&callisto_width,&callisto_height);
	Ganymede_Texture = BmpToTexture("ganymede.bmp",&ganymede_width,&ganymede_height);

	//Saturn Texture
	Saturn_Texture = BmpToTexture("saturn.bmp",&saturn_width,&saturn_height);
	Saturn_Ring_Texture = BmpToTexture("saturn_ring.bmp",&saturn_ring_width,&saturn_ring_height);
	Saturn_Ring_Texture2 = BmpToTexture("saturn_ring2.bmp",&saturn_ring_width2,&saturn_ring_height2);
	Saturn_Dione_Texture = BmpToTexture("saturn_dione.bmp",&dione_width,&dione_height );
	Saturn_Enceladus_Texture = BmpToTexture("saturn_enceladus.bmp",&enceladus_width,&enceladus_height);
	Saturn_Lapetus_Texture = BmpToTexture("saturn_lapetus.bmp",&lapetus_width,&lapetus_height);
	Saturn_Mimas_Texture = BmpToTexture("saturn_mimas.bmp",&mimas_width,&mimas_height);
	Saturn_Rhea_Texture = BmpToTexture("saturn_rhea.bmp",&rhea_width,&rhea_height);
	Saturn_Titan_Texture = BmpToTexture("saturn_titan.bmp",&titan_width,&titan_height);
	Saturn_Trlhys_Texture = BmpToTexture("telhy_fix.bmp",&trlhys_width,&trlhys_height);

	// Uranus Texture
	Uranus_Texture = BmpToTexture("uranus.bmp",&uranus_width,&uranus_height);
	Titania_Texture = BmpToTexture("titania_fix.bmp",&titania_width,&titania_height);
	Oberon_Texture = BmpToTexture("oberon.bmp",&oberon_width,&oberon_height);
	Umbriel_Texture = BmpToTexture("umbriel.bmp",&umbriel_width,&umbriel_height);
	// Nebtune Texture
	Nebtune_Texture = BmpToTexture("nebtune.bmp",&nebtune_width,&nebtune_height);
	Triton_Texture = BmpToTexture("triton.bmp",&charon_width,&charon_height);
	// Pluto Texture
	Pluto_Texture = BmpToTexture("pluto.bmp",&pluto_width,&pluto_height);
	Charon_Texture = BmpToTexture("charon.bmp",&charon_width,&charon_height);

	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glGenTextures( 1, &Space_Tex );

	glGenTextures( 1, &Sun_Tex );

	glGenTextures( 1, &Mercury_Tex );

	glGenTextures( 1, &Venus_Tex );

	glGenTextures( 1, &Earth_Tex );
	glGenTextures( 1, &Moon_Tex );



	glGenTextures( 1, &Mars_Tex );
	glGenTextures( 1, &Diemos_Tex );
	glGenTextures( 1, &Phobos_Tex );

	glGenTextures( 1, &Jupiter_Tex );
	glGenTextures( 1, &Io_Tex );
	glGenTextures( 1, &Europa_Tex );
	glGenTextures( 1, &Callisto_Tex );
	glGenTextures( 1, &Ganymede_Tex );

	glGenTextures( 1, &Saturn_Tex );
	glGenTextures( 1, &Saturn_Ring_Tex );
	glGenTextures( 1, &Saturn_Ring_Tex2 );
	glGenTextures( 1, &Dione_Tex );
	glGenTextures( 1, &Enceladus_Tex );
	glGenTextures( 1, &Lapetus_Tex );
	glGenTextures( 1, &Mimas_Tex );
	glGenTextures( 1, &Rhea_Tex );
	glGenTextures( 1, &Titan_Tex );
	glGenTextures( 1, &Trlhys_Tex );


	glGenTextures( 1, &Uranus_Tex );
	glGenTextures( 1, &Titania_Tex );
	glGenTextures( 1, &Oberon_Tex );
	glGenTextures( 1, &Umbriel_Tex );

	glGenTextures( 1, &Nebtune_Tex );
	glGenTextures( 1, &Triton_Tex );

	glGenTextures( 1, &Pluto_Tex );
	glGenTextures( 1, &Charon_Tex );

	// Space Texture
	glBindTexture( GL_TEXTURE_2D, Space_Tex );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP  );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, space_width, space_height, 0, GL_RGB, GL_UNSIGNED_BYTE, Space_Texture );

	// Sun Texture
	glBindTexture( GL_TEXTURE_2D, Sun_Tex );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP  );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, sun_width, sun_height, 0, GL_RGB, GL_UNSIGNED_BYTE, Sun_Texture );

	// Venus Texture
	glBindTexture( GL_TEXTURE_2D, Mercury_Tex );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP  );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, mercury_width, mercury_height, 0, GL_RGB, GL_UNSIGNED_BYTE, Mercury_Texture );

	// Venus Texture
	glBindTexture( GL_TEXTURE_2D, Venus_Tex );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP  );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, venus_width, venus_height, 0, GL_RGB, GL_UNSIGNED_BYTE, Venus_Texture );

	// Earth Textures
	glBindTexture( GL_TEXTURE_2D, Earth_Tex );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP  );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, earth_width, earth_height, 0, GL_RGB, GL_UNSIGNED_BYTE, Earth_Texture );

	glBindTexture( GL_TEXTURE_2D, Moon_Tex );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP  );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, moon_width, moon_height, 0, GL_RGB, GL_UNSIGNED_BYTE, Moon_Texture );

	// Mars Textures
	glBindTexture( GL_TEXTURE_2D, Mars_Tex );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP  );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, mars_width, mars_height, 0, GL_RGB, GL_UNSIGNED_BYTE, Mars_Texture );
	glBindTexture( GL_TEXTURE_2D, Diemos_Tex );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP  );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, diemos_width, diemos_height, 0, GL_RGB, GL_UNSIGNED_BYTE, Diemos_Texture );
	glBindTexture( GL_TEXTURE_2D, Phobos_Tex );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP  );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, phobos_width, phobos_height, 0, GL_RGB, GL_UNSIGNED_BYTE, Phobos_Texture );

	// Jupiture Textures
	glBindTexture( GL_TEXTURE_2D, Jupiter_Tex );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP  );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, jupiter_width, jupiter_height, 0, GL_RGB, GL_UNSIGNED_BYTE, Jupiter_Texture );
	glBindTexture( GL_TEXTURE_2D, Io_Tex );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP  );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, io_width, io_height, 0, GL_RGB, GL_UNSIGNED_BYTE, Io_Texture );
	glBindTexture( GL_TEXTURE_2D, Europa_Tex );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP  );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, europa_width, europa_height, 0, GL_RGB, GL_UNSIGNED_BYTE, Europa_Texture );
	glBindTexture( GL_TEXTURE_2D, Callisto_Tex );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP  );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, callisto_width, callisto_height, 0, GL_RGB, GL_UNSIGNED_BYTE, Callisto_Texture );
	glBindTexture( GL_TEXTURE_2D, Ganymede_Tex );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP  );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, ganymede_width, ganymede_height, 0, GL_RGB, GL_UNSIGNED_BYTE, Ganymede_Texture );

	//Saturn Textures
	glBindTexture( GL_TEXTURE_2D, Saturn_Tex );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP  );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, saturn_width, saturn_height, 0, GL_RGB, GL_UNSIGNED_BYTE, Saturn_Texture );
	glBindTexture( GL_TEXTURE_2D, Saturn_Ring_Tex );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP  );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, saturn_ring_width, saturn_ring_height, 0, GL_RGB, GL_UNSIGNED_BYTE, Saturn_Ring_Texture );
	glBindTexture( GL_TEXTURE_2D, Saturn_Ring_Tex2 );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP  );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, saturn_ring_width2, saturn_ring_height2, 0, GL_RGB, GL_UNSIGNED_BYTE, Saturn_Ring_Texture2 );
	glBindTexture( GL_TEXTURE_2D, Dione_Tex );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP  );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, dione_width, dione_height, 0, GL_RGB, GL_UNSIGNED_BYTE, Saturn_Dione_Texture );
	glBindTexture( GL_TEXTURE_2D, Enceladus_Tex );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP  );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, enceladus_width, enceladus_height, 0, GL_RGB, GL_UNSIGNED_BYTE, Saturn_Enceladus_Texture );
	glBindTexture( GL_TEXTURE_2D, Lapetus_Tex );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP  );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, lapetus_width, lapetus_height, 0, GL_RGB, GL_UNSIGNED_BYTE, Saturn_Lapetus_Texture );
	glBindTexture( GL_TEXTURE_2D, Mimas_Tex );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP  );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, mimas_width, mimas_height, 0, GL_RGB, GL_UNSIGNED_BYTE, Saturn_Mimas_Texture );
	glBindTexture( GL_TEXTURE_2D, Rhea_Tex );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP  );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, rhea_width, rhea_height, 0, GL_RGB, GL_UNSIGNED_BYTE, Saturn_Rhea_Texture );
	glBindTexture( GL_TEXTURE_2D, Titan_Tex );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP  );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, titan_width, titan_height, 0, GL_RGB, GL_UNSIGNED_BYTE, Saturn_Titan_Texture );
	glBindTexture( GL_TEXTURE_2D, Trlhys_Tex );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP  );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, trlhys_width, trlhys_height, 0, GL_RGB, GL_UNSIGNED_BYTE, Saturn_Trlhys_Texture );

	// Uranus Textures
	glBindTexture( GL_TEXTURE_2D, Uranus_Tex );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP  );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, uranus_width, uranus_height, 0, GL_RGB, GL_UNSIGNED_BYTE, Uranus_Texture );
	glBindTexture( GL_TEXTURE_2D, Titania_Tex );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP  );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, titania_width, titania_height, 0, GL_RGB, GL_UNSIGNED_BYTE, Titania_Texture );
	glBindTexture( GL_TEXTURE_2D, Oberon_Tex );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP  );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, oberon_width, oberon_height, 0, GL_RGB, GL_UNSIGNED_BYTE, Oberon_Texture );
	glBindTexture( GL_TEXTURE_2D, Umbriel_Tex );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP  );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, umbriel_width, umbriel_height, 0, GL_RGB, GL_UNSIGNED_BYTE, Umbriel_Texture );

	// Nubtune Texture 
	glBindTexture( GL_TEXTURE_2D, Nebtune_Tex );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP  );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, nebtune_width, nebtune_height, 0, GL_RGB, GL_UNSIGNED_BYTE, Nebtune_Texture );
	glBindTexture( GL_TEXTURE_2D, Triton_Tex );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP  );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, triton_width, triton_height, 0, GL_RGB, GL_UNSIGNED_BYTE, Triton_Texture );
	// Pluto Textures
	glBindTexture( GL_TEXTURE_2D, Pluto_Tex );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP  );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, pluto_width, pluto_height, 0, GL_RGB, GL_UNSIGNED_BYTE, Pluto_Texture );
	glBindTexture( GL_TEXTURE_2D, Charon_Tex );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP  );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, charon_width, charon_height, 0, GL_RGB, GL_UNSIGNED_BYTE, Charon_Texture );




	//glGenTextures( 1, &Moon_Tex );

	// init glew (a window must be open to do this):

#ifdef WIN32
	GLenum err = glewInit( );
	if( err != GLEW_OK )
	{
		fprintf( stderr, "glewInit Error\n" );
	}
	else
		fprintf( stderr, "GLEW initialized OK\n" );
	fprintf( stderr, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif

	// let glui take care of it in InitGlui( )
}


// initialize the display lists that will not change:
// (a display list is a way to store opengl commands in
//  memory so that they can be played back efficiently at a later time
//  with a call to glCallList( )

void
	InitLists( void )
{
	float dx = BOXSIZE / 2.;
	float dy = BOXSIZE / 2.;
	float dz = BOXSIZE / 2.;

	// create the object:

	BoxList = glGenLists( 1 );
	glNewList( BoxList, GL_COMPILE );

	glBegin( GL_QUADS );

	glColor3f( 0., 0., 1. );
	glNormal3f( 0., 0.,  1. );
	glVertex3f( -dx, -dy,  dz );
	glVertex3f(  dx, -dy,  dz );
	glVertex3f(  dx,  dy,  dz );
	glVertex3f( -dx,  dy,  dz );

	glNormal3f( 0., 0., -1. );
	glTexCoord2f( 0., 0. );
	glVertex3f( -dx, -dy, -dz );
	glTexCoord2f( 0., 1. );
	glVertex3f( -dx,  dy, -dz );
	glTexCoord2f( 1., 1. );
	glVertex3f(  dx,  dy, -dz );
	glTexCoord2f( 1., 0. );
	glVertex3f(  dx, -dy, -dz );

	glColor3f( 1., 0., 0. );
	glNormal3f(  1., 0., 0. );
	glVertex3f(  dx, -dy,  dz );
	glVertex3f(  dx, -dy, -dz );
	glVertex3f(  dx,  dy, -dz );
	glVertex3f(  dx,  dy,  dz );

	glNormal3f( -1., 0., 0. );
	glVertex3f( -dx, -dy,  dz );
	glVertex3f( -dx,  dy,  dz );
	glVertex3f( -dx,  dy, -dz );
	glVertex3f( -dx, -dy, -dz );

	glColor3f( 0., 1., 0. );
	glNormal3f( 0.,  1., 0. );
	glVertex3f( -dx,  dy,  dz );
	glVertex3f(  dx,  dy,  dz );
	glVertex3f(  dx,  dy, -dz );
	glVertex3f( -dx,  dy, -dz );

	glNormal3f( 0., -1., 0. );
	glVertex3f( -dx, -dy,  dz );
	glVertex3f( -dx, -dy, -dz );
	glVertex3f(  dx, -dy, -dz );
	glVertex3f(  dx, -dy,  dz );

	glEnd( );

	glEndList( );


	// create the axes:

	AxesList = glGenLists( 1 );
	glNewList( AxesList, GL_COMPILE );
	glLineWidth( AXES_WIDTH );
	Axes( 1.5 );
	glLineWidth( 1. );
	glEndList( );
}



//
// the keyboard callback:
//

void
	Keyboard( unsigned char c, int x, int y )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Keyboard: '%c' (0x%0x)\n", c, c );

	switch( c )
	{
	case 'o':
	case 'O':
		WhichProjection = ORTHO;
		break;

	case 'p':
	case 'P':
		WhichProjection = PERSP;
		break;

	case 'q':
	case 'Q':
	case ESCAPE:
		Buttons( QUIT );	// will not return here
		break;			// happy compiler

	case '1':
		Light1On = ! Light1On;
		break;


	case '2':
		TimeControl -=.20;
		break;
	case '8':
		TimeControl +=.20;
		break;

	case 'r':
	case 'R':
		LeftButton = ROTATE;
		break;

	case 's':
	case 'S':
		LeftButton = SCALE;
		break;

	default:
		fprintf( stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c );
	}


	// synchronize the GLUI display with the variables:

	Glui->sync_live( );


	// force a call to Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}



//
// called when the mouse button transitions down or up:
//

void
	MouseButton( int button, int state, int x, int y )
{
	int b = 0;			// LEFT, MIDDLE, or RIGHT

	if( DebugOn != 0 )
		fprintf( stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y );


	// get the proper button bit mask:

	switch( button )
	{
	case GLUT_LEFT_BUTTON:
		b = LEFT;		break;

	case GLUT_MIDDLE_BUTTON:
		b = MIDDLE;		break;

	case GLUT_RIGHT_BUTTON:
		b = RIGHT;		break;

	default:
		b = 0;
		fprintf( stderr, "Unknown mouse button: %d\n", button );
	}


	// button down sets the bit, up clears the bit:

	if( state == GLUT_DOWN )
	{
		Xmouse = x;
		Ymouse = y;
		ActiveButton |= b;		// set the proper bit
	}
	else
	{
		ActiveButton &= ~b;		// clear the proper bit
	}
}



//
// called when the mouse moves while a button is down:
//

void
	MouseMotion( int x, int y )
{
	if( DebugOn != 0 )
		fprintf( stderr, "MouseMotion: %d, %d\n", x, y );


	int dx = x - Xmouse;		// change in mouse coords
	int dy = y - Ymouse;

	if( ( ActiveButton & LEFT ) != 0 )
	{
		switch( LeftButton )
		{
		case ROTATE:
			Xrot += ( ANGFACT*dy );
			Yrot += ( ANGFACT*dx );
			break;

		case SCALE:
			Scale += SCLFACT * (float) ( dx - dy );
			if( Scale < MINSCALE )
				Scale = MINSCALE;
			break;
		}
	}


	if( ( ActiveButton & MIDDLE ) != 0 )
	{
		Scale += SCLFACT * (float) ( dx - dy );

		// keep object from turning inside-out or disappearing:

		if( Scale < MINSCALE )
			Scale = MINSCALE;
	}

	Xmouse = x;			// new current position
	Ymouse = y;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}



// reset the transformations and the colors:
//
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene

void
	Reset( void )
{
	mercury = 0;
	mercurySpin = 0;
	venus = 0;
	venusSpin = 0;
	earth = 0;
	earthSpin = 0;
	earthMoon = 0;
	mars = 0;
	marsSpin = 0;
	marsdiemosmoon = 0;
	marsphobosmoon = 0;
	jupiter = 0;
	jupiterSpin = 0;
	jupiteriomoon = 0;
	jupitereuropamoon = 0;
	jupitercallistomoon = 0;
	jupiterganymedemoon = 0;
	saturn = 0;
	saturnSpin = 0;
	saturndionemoon = 0;
	saturnenceladusmoon = 0;
	saturnlapetusmoon = 0;
	saturnmimasmooon = 0;
	saturnrheamoon = 0;
	saturntitanmoon = 0;
	saturntrlhysmoon = 0;
	uranus = 0;
	uranusSpin = 0;
	uranustitaniamoon = 0;
	uranusoberonmoon = 0;
	uranusumbrielmoon = 0;
	nebtune = 0;
	nebtuneSpin = 0;
	nebtunemoontriton = 0;
	pluto = 0;
	plutoSpin = 0;
	plutomooncharon = 0;




	AxesOn = GLUITRUE;
	DebugOn = GLUIFALSE;
	DepthCueOn = GLUIFALSE;
	LeftButton = ROTATE;
	LinesEnabled = 1;
	freezebox = 0;
	Scale  = 1.0;
	Scale2 = 0.0;		// because we add 1. to it in Display( )
	//WhichColor = WHITE;
	WhichPlanet = 0;
	SaturnRingTrans = 3;
	WhichCamera = 0;
	//	WhichProjection = PERSP;
	Xrot = Yrot = 0.;
	TransXYZ[0] = TransXYZ[1] = TransXYZ[2] = 0.;

	RotMatrix[0][1] = RotMatrix[0][2] = RotMatrix[0][3] = 0.;
	RotMatrix[1][0]                   = RotMatrix[1][2] = RotMatrix[1][3] = 0.;
	RotMatrix[2][0] = RotMatrix[2][1]                   = RotMatrix[2][3] = 0.;
	RotMatrix[3][0] = RotMatrix[3][1] = RotMatrix[3][3]                   = 0.;
	RotMatrix[0][0] = RotMatrix[1][1] = RotMatrix[2][2] = RotMatrix[3][3] = 1.;

}



//
// called when user resizes the window:
//

void
	Resize( int width, int height )
{
	if( DebugOn != 0 )
		fprintf( stderr, "ReSize: %d, %d\n", width, height );

	// don't really need to do anything since window size is
	// checked each time in Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


//
// handle a change to the window's visibility:
//

void
	Visibility ( int state )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Visibility: %d\n", state );

	if( state == GLUT_VISIBLE )
	{
		glutSetWindow( MainWindow );
		glutPostRedisplay( );
	}
	else
	{
		// could optimize by keeping track of the fact
		// that the window is not visible and avoid
		// animating or redrawing it ...
	}
}




///////////////////////////////////////   HANDY UTILITIES:  //////////////////////////

// size of wings as fraction of length:

#define WINGS	0.10


// axes:

#define X	1
#define Y	2
#define Z	3


// x, y, z, axes:

static float axx[3] = { 1., 0., 0. };
static float ayy[3] = { 0., 1., 0. };
static float azz[3] = { 0., 0., 1. };


void
	Arrow( float tail[3], float head[3] )
{
	float u[3], v[3], w[3];		// arrow coordinate system

	// set w direction in u-v-w coordinate system:

	w[0] = head[0] - tail[0];
	w[1] = head[1] - tail[1];
	w[2] = head[2] - tail[2];


	// determine major direction:

	int axis = X;
	float mag = fabs( w[0] );
	if(  fabs( w[1] )  > mag  )
	{
		axis = Y;
		mag = fabs( w[1] );
	}
	if(  fabs( w[2] )  > mag  )
	{
		axis = Z;
		mag = fabs( w[2] );
	}


	// set size of wings and turn w into a Unit vector:

	float d = WINGS * Unit( w, w );


	// draw the shaft of the arrow:

	glBegin( GL_LINE_STRIP );
	glVertex3fv( tail );
	glVertex3fv( head );
	glEnd( );

	// draw two sets of wings in the non-major directions:

	float x, y, z;

	if( axis != X )
	{
		Cross( w, axx, v );
		(void) Unit( v, v );
		Cross( v, w, u  );
		x = head[0] + d * ( u[0] - w[0] );
		y = head[1] + d * ( u[1] - w[1] );
		z = head[2] + d * ( u[2] - w[2] );
		glBegin( GL_LINE_STRIP );
		glVertex3fv( head );
		glVertex3f( x, y, z );
		glEnd( );
		x = head[0] + d * ( -u[0] - w[0] );
		y = head[1] + d * ( -u[1] - w[1] );
		z = head[2] + d * ( -u[2] - w[2] );
		glBegin( GL_LINE_STRIP );
		glVertex3fv( head );
		glVertex3f( x, y, z );
		glEnd( );
	}


	if( axis != Y )
	{
		Cross( w, ayy, v );
		(void) Unit( v, v );
		Cross( v, w, u  );
		x = head[0] + d * ( u[0] - w[0] );
		y = head[1] + d * ( u[1] - w[1] );
		z = head[2] + d * ( u[2] - w[2] );
		glBegin( GL_LINE_STRIP );
		glVertex3fv( head );
		glVertex3f( x, y, z );
		glEnd( );
		x = head[0] + d * ( -u[0] - w[0] );
		y = head[1] + d * ( -u[1] - w[1] );
		z = head[2] + d * ( -u[2] - w[2] );
		glBegin( GL_LINE_STRIP );
		glVertex3fv( head );
		glVertex3f( x, y, z );
		glEnd( );
	}



	if( axis != Z )
	{
		Cross( w, azz, v );
		(void) Unit( v, v );
		Cross( v, w, u  );
		x = head[0] + d * ( u[0] - w[0] );
		y = head[1] + d * ( u[1] - w[1] );
		z = head[2] + d * ( u[2] - w[2] );
		glBegin( GL_LINE_STRIP );
		glVertex3fv( head );
		glVertex3f( x, y, z );
		glEnd( );
		x = head[0] + d * ( -u[0] - w[0] );
		y = head[1] + d * ( -u[1] - w[1] );
		z = head[2] + d * ( -u[2] - w[2] );
		glBegin( GL_LINE_STRIP );
		glVertex3fv( head );
		glVertex3f( x, y, z );
		glEnd( );
	}
}



float
	Dot( float v1[3], float v2[3] )
{
	return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}



void
	Cross( float v1[3], float v2[3], float vout[3] )
{
	float tmp[3];

	tmp[0] = v1[1]*v2[2] - v2[1]*v1[2];
	tmp[1] = v2[0]*v1[2] - v1[0]*v2[2];
	tmp[2] = v1[0]*v2[1] - v2[0]*v1[1];

	vout[0] = tmp[0];
	vout[1] = tmp[1];
	vout[2] = tmp[2];
}



float
	Unit( float vin[3], float vout[3] )
{
	float dist = vin[0]*vin[0] + vin[1]*vin[1] + vin[2]*vin[2];

	if( dist > 0.0 )
	{
		dist = sqrt( dist );
		vout[0] = vin[0] / dist;
		vout[1] = vin[1] / dist;
		vout[2] = vin[2] / dist;
	}
	else
	{
		vout[0] = vin[0];
		vout[1] = vin[1];
		vout[2] = vin[2];
	}

	return dist;
}



// the stroke characters 'X' 'Y' 'Z' :

static float xx[ ] = {
	0.f, 1.f, 0.f, 1.f
};

static float xy[ ] = {
	-.5f, .5f, .5f, -.5f
};

static int xorder[ ] = {
	1, 2, -3, 4
};


static float yx[ ] = {
	0.f, 0.f, -.5f, .5f
};

static float yy[ ] = {
	0.f, .6f, 1.f, 1.f
};

static int yorder[ ] = {
	1, 2, 3, -2, 4
};


static float zx[ ] = {
	1.f, 0.f, 1.f, 0.f, .25f, .75f
};

static float zy[ ] = {
	.5f, .5f, -.5f, -.5f, 0.f, 0.f
};

static int zorder[ ] = {
	1, 2, 3, 4, -5, 6
};


// fraction of the length to use as height of the characters:

const float LENFRAC = 0.10f;


// fraction of length to use as start location of the characters:

const float BASEFRAC = 1.10f;


//	Draw a set of 3D axes:
//	(length is the axis length in world coordinates)

void
	Axes( float length )
{
	glBegin( GL_LINE_STRIP );
	glVertex3f( length, 0., 0. );
	glVertex3f( 0., 0., 0. );
	glVertex3f( 0., length, 0. );
	glEnd( );
	glBegin( GL_LINE_STRIP );
	glVertex3f( 0., 0., 0. );
	glVertex3f( 0., 0., length );
	glEnd( );

	float fact = LENFRAC * length;
	float base = BASEFRAC * length;

	glBegin( GL_LINE_STRIP );
	for( int i = 0; i < 4; i++ )
	{
		int j = xorder[i];
		if( j < 0 )
		{

			glEnd( );
			glBegin( GL_LINE_STRIP );
			j = -j;
		}
		j--;
		glVertex3f( base + fact*xx[j], fact*xy[j], 0.0 );
	}
	glEnd( );

	glBegin( GL_LINE_STRIP );
	for( int i = 0; i < 5; i++ )
	{
		int j = yorder[i];
		if( j < 0 )
		{

			glEnd( );
			glBegin( GL_LINE_STRIP );
			j = -j;
		}
		j--;
		glVertex3f( fact*yx[j], base + fact*yy[j], 0.0 );
	}
	glEnd( );

	glBegin( GL_LINE_STRIP );
	for( int i = 0; i < 6; i++ )
	{
		int j = zorder[i];
		if( j < 0 )
		{

			glEnd( );
			glBegin( GL_LINE_STRIP );
			j = -j;
		}
		j--;
		glVertex3f( 0.0, fact*zy[j], base + fact*zx[j] );
	}
	glEnd( );

}




// routine to convert HSV to RGB
//
// Reference:  Foley, van Dam, Feiner, Hughes,
//		"Computer Graphics Principles and Practices,"


void
	HsvRgb( float hsv[3], float rgb[3] )
{
	float r, g, b;			// red, green, blue

	// guarantee valid input:

	float h = hsv[0] / 60.;
	while( h >= 6. )	h -= 6.;
	while( h <  0. ) 	h += 6.;

	float s = hsv[1];
	if( s < 0. )
		s = 0.;
	if( s > 1. )
		s = 1.;

	float v = hsv[2];
	if( v < 0. )
		v = 0.;
	if( v > 1. )
		v = 1.;


	// if sat==0, then is a gray:

	if( s == 0.0 )
	{
		rgb[0] = rgb[1] = rgb[2] = v;
		return;
	}


	// get an rgb from the hue itself:

	float i = floor( h );
	float f = h - i;
	float p = v * ( 1. - s );
	float q = v * ( 1. - s*f );
	float t = v * ( 1. - ( s * (1.-f) ) );

	switch( (int) i )
	{
	case 0:
		r = v;	g = t;	b = p;
		break;

	case 1:
		r = q;	g = v;	b = p;
		break;

	case 2:
		r = p;	g = v;	b = t;
		break;

	case 3:
		r = p;	g = q;	b = v;
		break;

	case 4:
		r = t;	g = p;	b = v;
		break;

	case 5:
		r = v;	g = p;	b = q;
		break;
	}


	rgb[0] = r;
	rgb[1] = g;
	rgb[2] = b;
}
