#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/freeglut_ext.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include "tga.h"

#define __OPENCV__ 1

#ifdef __OPENCV__
	#include "opencv2/opencv.hpp"
	using namespace cv;
	void trackFrame();
	void onExit();
	int trackerInit();
	IplImage* imgTracking = 0;
	IplImage* frame = 0;
	CvCapture* capture = 0;
#endif

#define STARTING_X (1.5f);
#define STARTING_Y (1.5f);
#define STARTING_DIRECTION (0.0f);
#define SPIN 0
#define ENTER 1
#define NAV 2

float player_x = STARTING_X ;
float player_y = STARTING_Y ;
float player_h = STARTING_DIRECTION ;
float player_s = 0.0f;
float player_m = 1.0f;
float player_t = 0.0f;
float player_b = 0.0f;
float forwardSpeed = 0.2f;
float turnSpeed = 3.5f;
int walllist=0;
int mazelist=0;
int balllist=0;
void spinmaze();
void entermaze();
void navmaze();
void lighting(int);
void setup();
int Window_Width = 720;
int Window_Height = 720;
const int EscKey = 27;
const int SpaceKey = 32;

const int numTextures = 8;
GLuint *textures = new GLuint[numTextures];

bool enabletexture = false;
bool ortho = false;

std::stringstream sstm;
std::string file;

int mazeHeight, mazeWidth, level = 8;
std::vector<std::string> mazedata;

static int state = SPIN;
static float texcoordX=0.0f;
static float spin=720.0f;
static float p=0.0f;

int lastX = -1;
int lastY = -1;
int maxX, minX, maxY, minY, midX, midY;
int lowerH=47;
int upperH=112;
int lowerS=115;
int upperS=224;
int lowerV=40;
int upperV=180;
bool firstValue = false;
bool track = false;
bool fullScreen = false;
int board[3][3];
GLdouble left, right, bottom, top, fov = 60.0;
GLdouble zNearOrtho = -600.0, zFarOrtho = 600.0, zNearPersp = 0.1, zFarPersp= 600.0;
GLdouble aspect = 1.0;

void drawSquares(GLenum mode)
{
	GLuint i, j;
	for (i = 0; i < 3; i++) {
		if (mode == GL_SELECT)
			glLoadName (i);
		for (j = 0; j < 3; j ++) {
			if (mode == GL_SELECT)
				glPushName (j);
			glColor3f ((GLfloat) i/3.0, (GLfloat) j/3.0, 
			(GLfloat) board[i][j]/3.0);
			glRecti (i, j, i+1, j+1);
			if (mode == GL_SELECT)
				glPopName ();
		}
	}
}
void processHits (GLint hits, GLuint buffer[])
{
	unsigned int i, j;
	GLuint ii, jj, names, *ptr;

	printf ("hits = %d\n", hits);

	ptr = (GLuint *) buffer;

	for (i = 0; i < hits; i++) { /*  for each hit  */
		names = *ptr;
		printf (" number of names for this hit = %d\n", names);
		ptr++;
		printf(" z1 is %g;", (float) *ptr/0x7fffffff); ptr++;
		printf(" z2 is %g\n", (float) *ptr/0x7fffffff); ptr++;
		printf (" names are ");
      
		for (j = 0; j < names; j++) { /*  for each name */
			printf ("%d ", *ptr);
			if (j == 0)  /*  set row and column  */
				ii = *ptr;
			else if (j == 1)
				jj = *ptr;
		 
			ptr++;
		}
		printf ("row is %d ", ii);
		printf ("col is %d ", jj);

		#ifdef __OPENCV__
			if (ii == 0 && jj == 1 ) //left
				player_t = -2.5*turnSpeed;
			else if (ii == 2 && jj == 1)//right
				player_t = 2.5*turnSpeed;
			else if(ii == 1) {
				if (jj == 2) //up
					player_s = 2.0*forwardSpeed;
				else if (jj == 0) //down
					player_s = -2.0*forwardSpeed;
			}
		#else
				if (ii == 0 && jj == 1 ) //left
				player_t = -turnSpeed;
			else if (ii == 2 && jj == 1)//right
				player_t = turnSpeed;
			else if(ii == 1) {
				if (jj == 2) //up
					player_s = forwardSpeed;
				else if (jj == 0) //down
					player_s = -forwardSpeed;
			}
		#endif
		printf ("\n");
		board[ii][jj] = (board[ii][jj] + 1) % 3;
	}
}

#define BUFSIZE 512

void pickSquares(int button, int state, int x, int y)
{
	GLuint selectBuf[BUFSIZE];
	GLint hits;
	GLint viewport[4];

	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP){
		player_t = 0.0f;
		player_s = 0.0f;
	}

	if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN)
		return;

	glGetIntegerv (GL_VIEWPORT, viewport);

	glSelectBuffer (BUFSIZE, selectBuf);
	(void) glRenderMode (GL_SELECT);

	glInitNames();
	glPushName(0);

	glMatrixMode (GL_PROJECTION);
	glPushMatrix ();
	glLoadIdentity();
	/*  create 5x5 pixel picking region near cursor location      */
	gluPickMatrix ((GLdouble) x, (GLdouble) (viewport[3] - y), 
	5.0, 5.0, viewport);
	gluOrtho2D (0.0, 3.0, 0.0, 3.0);
	drawSquares (GL_SELECT);

	glMatrixMode (GL_PROJECTION);
	glPopMatrix ();
	glFlush ();

	hits = glRenderMode (GL_RENDER);
	processHits (hits, selectBuf);
	glutPostRedisplay();
} 

std::vector<std::string> readmaze(std::string filename)
{
	mazeHeight = 0, mazeWidth = 0;
	std::ifstream fileStream(filename);
	if(!fileStream)
	{
		std::cout<<"Error opening output file"<< std::endl;
		system("pause");
		exit(EXIT_FAILURE);
	}
	std::vector<std::string> lines;
	for (std::string line; std::getline( fileStream, line ); /**/ ){
		lines.push_back( line );
		mazeHeight++; mazeWidth = line.length();
	}
	return lines;
}

int wall(int x,int y) {
	return (x>=0 && y>=0 && x<mazeWidth && y<mazeHeight && ' ' != mazedata[y][x]);
}

int openList(int x,int y){
	assert(wall(x,y));
	return(mazedata[y][x]=='.');
}
void closeList(int x,int y) {
	assert(wall(x,y));
	assert(openList(x,y));
	mazedata[y][x]= 'X';
}
int adjacent(int x,int y,int w,int *nx,int *ny){
	switch(w) {
	case 0:
		*nx = x-1; *ny=y;   break;
	case 1:
		*nx = x;   *ny=y+1; break;
	case 2:
		*nx = x+1; *ny=y;   break;
	case 3:
		*nx = x;   *ny=y-1; break;
	default:
		assert(0);
	}
	return wall(*nx,*ny);
}

int diagonal(int x,int y,int w,int *nx,int *ny){
	switch(w) {
	case 0:
		*nx = x-1; *ny=y-1; break;
	case 1:
		*nx = x-1; *ny=y+1; break;
	case 2:
		*nx = x+1; *ny=y+1; break;
	case 3:
		*nx = x+1; *ny=y-1; break;
	default:
		assert(0);
	}
	return wall(*nx,*ny);
}

float normal[6][3] = {
	{ -1.0f, 0.0f,0.0f},
	{  0.0f, 1.0f,0.0f},
	{  1.0f, 0.0f,0.0f},
	{  0.0f,-1.0f,0.0f},
	{  0.0f, 0.0f,1.0f},
	{  0.0f, 0.0f,-1.0f},
};

float color[4][3] = {
	{  1.0f, 0.0f,0.0f},
	{  0.0f, 1.0f,0.0f},
	{  0.0f, 0.0f,1.0f},
	{  1.0f, 1.0f,0.0f},
};

int quadStrip(int x,int y,int p) {
	int w=p;
	closeList(x,y);
	do{
		int x2,y2;
		if(adjacent(x,y,w,&x2,&y2)) {
			if(openList(x2,y2)) {
				quadStrip(x2,y2,(w+3)%4);
			}
			else {
				assert((w+1)%4 ==p);
				return 1;
			}
		}
		else {
			float fx;
			float fy;
			if(diagonal(x,y,w,&x2,&y2) && openList(x2,y2)) {
				quadStrip(x2,y2,(w+2)%4);
			}
			glNormal3fv(normal[w] );
			glColor3fv(color[w] );
			texcoordX=(texcoordX<0.5)?1.0f:0.0f;
			fx = (float)x+((w==1||w==2)?1.0f:0.0f);
			fy = (float)y+((w==0||w==1)?1.0f:0.0f);
			glTexCoord2f(texcoordX,0.0f);
			glVertex3f(fx,fy,0.0f);
			glTexCoord2f(texcoordX,1.0f);
			glVertex3f(fx,fy,1.0f);
		}  
		w++;w%=4;
	}while (w!=p);
	return 1;
}

int drawwalls() {
	int dl;
	glNewList(dl=glGenLists(1),GL_COMPILE);
	glBegin(GL_QUAD_STRIP);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 1.0f);
	quadStrip(0,0,0);
	glEnd();
	glEndList();
	return dl;
}

int drawball()
{
	int dl;
	glNewList(dl=glGenLists(1),GL_COMPILE);
	glColor3f(1.0,0.0,0.0);
	glutSolidSphere(0.3f,15,15);
	glEndList();
	return dl;
}

int drawtop() {
	int x,y,dl;
	glNewList(dl=glGenLists(1),GL_COMPILE);
	glPushAttrib(GL_TEXTURE_BIT | GL_LIGHTING_BIT);
	glColor3f(1.0f,1.0f,1.0f);
	glBegin(GL_QUADS);
	for(y=0;y<mazeHeight;y++) {
		for(x=0;x<mazeWidth;x++) {
			if(wall(x,y)) {
				//bottomside:
				glNormal3fv(normal[5]);
				glTexCoord2f(0.0f, 0.0f); glVertex3f(x+0.0f ,y+0.0f ,0.0f );
				glNormal3fv(normal[5]);
				glTexCoord2f(0.0f, 1.0f); glVertex3f(x+0.0f ,y+1.0f ,0.0f );
				glNormal3fv(normal[5]);
				glTexCoord2f(1.0f, 1.0f); glVertex3f(x+1.0f ,y+1.0f ,0.0f );
				glNormal3fv(normal[5]);
				glTexCoord2f(1.0f, 0.0f); glVertex3f(x+1.0f ,y+0.0f ,0.0f );

				// topside:
				glNormal3fv(normal[4]);
				glTexCoord2f(0.0f, 1.0f); glVertex3f(x+0.0f ,y+0.0f ,1.0f );
				glNormal3fv(normal[4]);
				glTexCoord2f(1.0f, 1.0f); glVertex3f(x+1.0f ,y+0.0f ,1.0f );
				glNormal3fv(normal[4]);
				glTexCoord2f(1.0f, 0.0f); glVertex3f(x+1.0f ,y+1.0f ,1.0f );
				glNormal3fv(normal[4]);
				glTexCoord2f(0.0f, 0.0f); glVertex3f(x+0.0f ,y+1.0f ,1.0f );
				}
		}
	}
	glEnd();
	glPopAttrib();
	glEndList();
	return(dl);
}

int forward(float px,float py,float bf) {
	int x = ((int)player_x);
	int y = ((int)player_y);
	int h=0;
	if((px> x+1.0f - bf) && wall(x+1,y)) {
		px = (float)(x)+1.0f-bf;
		h++;
	}
	if(py> y+1.0f-bf && wall(x,y+1))  {
		py = (float)(y)+1.0f-bf;
		h++;
	}
	if(px< x+bf && wall(x-1,y)) {
		px = (float)(x)+bf;
		h++;
	}
	if(py< y+bf && wall(x,y-1)) {
		py = (float)(y)+bf;
		h++;
	}
	player_x=px;
	player_y=py;
	return h;
}

void update(int value)
{
	if(state == SPIN) {
		if(level == 256)
			spin -= 5.0f;
		else
			spin -= 3.0f;
	} else if (state == ENTER) {
		p+=0.01f;
	} else if (state == NAV) {
		forward(player_x+player_m*player_s*(float)sin(player_h*3.14/180),
		player_y+player_m*player_s*(float)cos(player_h*3.14/180),0.5f);
		player_h+=player_t;
		player_b = 3*player_b/4 + player_t/4;
	}
	glutPostRedisplay();
	glutTimerFunc(10,update,0);
}

void spinmaze() {
	state = SPIN;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glPushMatrix();
	glTranslatef(0.0f,0.0f,-(float)level-12.0f );
	glRotatef(spin, 0.0f, 1.5f, 1.5f);
	glTranslatef(-mazeWidth/2.0f,-mazeHeight/2.0f,0.0f);
	glCallList(walllist);
	glCallList(mazelist);
	glPopMatrix();
	glutSwapBuffers();
	if(spin <=0.0f) {
		spin = 720.0f;
		state = ENTER;
		glutIdleFunc(entermaze);
		player_x = STARTING_X ;
		player_y = STARTING_Y ;
		player_h = STARTING_DIRECTION ;
	}
}

void entermaze() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glPushMatrix();
	glRotatef(-90.0f*p,1.0f,0.0f,0.0f);
	glRotatef(360.0f*p,0.0f,0.0f,1.0f);
	glRotatef(player_h*p,0.0f,0.0f,1.0f);
	glTranslatef(-(player_x*p + mazeWidth /2.0f*(1-p)),
	-(player_y*p + mazeHeight/2.0f*(1-p)),
	-(0.5f*p + (0.22*level+13.23f)*(1-p)));
	glCallList(walllist);
	glCallList(mazelist);
	glPopMatrix();
	glutSwapBuffers();
	if(p>=1.0f) {
		p=0.0f;
		state = NAV;
		glutIdleFunc(navmaze);
	}
}

void nextLevel(){
	state = SPIN;
	p=0.0f;
	if(level < 32)
		level += 4;
	else if(level == 32)
		level = 256;
	else
		level = 8;;
	sstm.str("");
	sstm << "MAZE" << level << ".txt";
	file = sstm.str();
	mazedata = readmaze(file);
	ortho = false;
	setup();
}

void navmaze()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if(ortho){
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho( left, right, bottom, top, zNearOrtho, zFarOrtho );
		glMatrixMode(GL_MODELVIEW);
		glDisable(GL_TEXTURE_2D);
		glCallList(balllist);
		((enabletexture)?glEnable:glDisable)(GL_TEXTURE_2D);
		glLoadIdentity();
		glPushMatrix();
		glRotatef(-20.0f,1.0f,0.0f,0.0f);
	}else{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(fov, aspect, zNearPersp, zFarPersp);
		glMatrixMode(GL_MODELVIEW);

		glLoadIdentity();
		glPushMatrix();
		glRotatef(-90.0f,1.0f,0.0f,0.0f);
	}
	glRotatef(player_h,0.0f,0.0f,1.0f);
	glTranslatef(-player_x,-player_y,-0.5f);
	glCallList(walllist);
	glCallList(mazelist);
	glPopMatrix();
	glutSwapBuffers();
	#ifdef __OPENCV__
		if(capture)
			trackFrame();
	#endif
	if(player_x>mazeWidth || player_y>mazeHeight) {
		nextLevel();
	}	
}

void readTexture(GLuint texture, const char* filename){
  TgaImage cubeImage;
  if (!cubeImage.loadTGA(filename))
    {
      printf("Error loading image file\n");
      exit(1);
    }

  glBindTexture( GL_TEXTURE_2D, texture );
  glTexImage2D(GL_TEXTURE_2D, 0, 4, cubeImage.width, cubeImage.height, 0,
	       (cubeImage.byteCount == 3) ? GL_BGR : GL_BGRA,
	       GL_UNSIGNED_BYTE, cubeImage.data );

  glGenerateMipmap(GL_TEXTURE_2D);    
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
}


void lighting(int lightNumber)
{
	GLfloat mat_diffuse[] = {1.0, 0.0, 0.0, 1.0};
	GLfloat mat_specular[] = {0.0, 0.0, 1.0, 1.0};
	GLfloat mat_ambient[] = {0.0, 1.0, 0.0, 1.0};
	GLfloat mat_shininess[] = {100.0};

	GLfloat light_specular[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat light_diffuse[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat light_ambient[] = {0.0, 0.0, 0.0, 1.0};
	GLfloat light_position[] = {5.0, 5.0, 10.0, 0.0};

	GLfloat light_specular1[] = {0.5, 0.5, 0.5, 1.0};
	GLfloat light_diffuse1[] = {0.5, 0.5, 0.5, 1.0};
	GLfloat light_ambient1[] = {0.0, 0.0, 0.0, 1.0};

	GLfloat mat_diffuse1[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat mat_specular1[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat mat_ambient1[] = {1.0, 1.0, 1.0, 1.0};

	GLfloat light_specular2[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat light_diffuse2[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat light_ambient2[] = {1.0, 1.0, 1.0, 1.0};

	GLfloat mat_diffuse2[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat mat_specular2[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat mat_ambient2[] = {1.0, 1.0, 1.0, 1.0};

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glShadeModel(GL_SMOOTH);
	// z buffer enable
	glEnable(GL_DEPTH_TEST);

	// enable lighting
	glEnable(GL_LIGHTING);
	// set light property

	if(lightNumber == 0){
		glDisable(GL_LIGHT1);
		glDisable(GL_LIGHT2);

		glEnable(GL_LIGHT0);
		glLightfv(GL_LIGHT0, GL_POSITION, light_position);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
		glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
		glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);

		// set material property
		glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
		glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
		glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
		glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	} else if(lightNumber == 1){
		glDisable(GL_LIGHT0);
		glDisable(GL_LIGHT2);

		glEnable(GL_LIGHT1);
		glLightfv(GL_LIGHT1, GL_POSITION, light_position);
		glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse1);
		glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular1);
		glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient1);

		glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse1);
		glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular1);
		glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient1);
		glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	} else if(lightNumber == 2){
		glDisable(GL_LIGHT0);
		glDisable(GL_LIGHT1);

		glEnable(GL_LIGHT2);
		glLightfv(GL_LIGHT2, GL_POSITION, light_position);
		glLightfv(GL_LIGHT2, GL_DIFFUSE, light_diffuse2);
		glLightfv(GL_LIGHT2, GL_SPECULAR, light_specular2);
		glLightfv(GL_LIGHT2, GL_AMBIENT, light_ambient2);

		glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse2);
		glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular2);
		glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient2);
		glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	}
}

void displayCallback(){
	drawSquares(GL_RENDER);
}

void reshapeCallback( int width, int height )
{
    glViewport( 0, 0, width, height );
    aspect = (GLdouble) width / (GLdouble) height;

    if ( aspect < 1.0 ) {
         left = -(GLdouble) level;
         right = (GLdouble) level;
         bottom = -(GLdouble) level / aspect ;
         top = (GLdouble) level / aspect ;
    } else {
         left = -(GLdouble) level * aspect;
         right = (GLdouble) level * aspect;
         bottom = -(GLdouble) level;
         top = (GLdouble) level;
    }

	glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
	if(ortho)
		glOrtho( left, right, bottom, top, zNearOrtho, zFarOrtho );
	else
		gluPerspective(fov, aspect, zNearPersp, zFarPersp);
	glMatrixMode( GL_MODELVIEW );
}

void setup(){
	right = top = level;
	left = bottom = -level;
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glShadeModel(GL_SMOOTH);
	glDisable(GL_TEXTURE);
	//glEnable(GL_CULL_FACE);
	walllist = drawwalls();
	mazelist = drawtop();
	balllist = drawball();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fov, aspect, zNearPersp, zFarPersp);
	glMatrixMode(GL_MODELVIEW);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
	glLoadIdentity ();
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glGenTextures( numTextures, textures );
	if(level == 8){
		readTexture(textures[0], "crate.tga");
		lighting(0);
	}else if(level == 12)
		readTexture(textures[1], "furnace_top.tga");
	else if(level == 16)
		readTexture(textures[2], "mossy_stone.tga");
	else if(level == 20)
		readTexture(textures[3], "brick.tga");
	else if(level == 24)
		readTexture(textures[4], "wood.tga");
	else if(level == 28){
		readTexture(textures[5], "ice.tga");
		lighting(1);
	}else if(level == 32){
		readTexture(textures[6], "tunnel.tga");
		lighting(2);
	}else if(level == 256){
		readTexture(textures[7], "stone.tga");
		lighting(0);
	}
	glutIdleFunc(spinmaze);
}

void keyboardCallback(unsigned char key, int x, int y){
    switch( key )
	{
		case EscKey: case 'q': case 'Q':
			exit( EXIT_SUCCESS );
			break;
		case 'o': case 'O':
			ortho = !ortho;
			break;
		case 'c': case 'C':
			track = !track;
			if(!track){
				player_s = 0.0f;
				player_t = 0.0f;
			}
			break;
		case ' ':
			minX = lastX;
			maxX = lastX;
			minY = lastY;
			maxY = lastY;
			midX = 0.0f;
			midY = 0.0f;
			break;
		case 't': case 'T':
			enabletexture = !enabletexture;
			((enabletexture)?glEnable:glDisable)(GL_TEXTURE);
			((enabletexture)?glEnable:glDisable)(GL_TEXTURE_2D);
			break;
		case '+':
			if(ortho){
				left += 2.0;
				bottom += 2.0;
				top -= 2.0;
				right -= 2.0;
			}
			break;
		case '-':
			if(ortho){
				left -= 2.0;
				bottom -= 2.0;
				top += 2.0;
				right += 2.0;
			}
			break;
		case 'f': case 'F':
			fullScreen = !fullScreen;
			if(fullScreen){
				glutFullScreen();
			} else {
				glutReshapeWindow(Window_Width,Window_Height);
				glutPositionWindow(0,0);
			}
			break;
		case 'n': case 'N':
			nextLevel();
			break;
		case 's': case 'S':
			state = SPIN;
			p=0.0f;
			ortho = false;
			glutIdleFunc(spinmaze);
			break;
		case 'e': case 'E':
			state = ENTER;
			p=0.0f;
			ortho = false;
			glutIdleFunc(entermaze);
			break;
	}
}

void specialKey(int key, int x, int y)
{
#ifdef __OPENCV__
	switch( key )
	{
		case GLUT_KEY_UP:
			player_s = 2.0*forwardSpeed;
			break;
		case GLUT_KEY_DOWN:
			player_s = -2.0*forwardSpeed;
			break;
		case GLUT_KEY_LEFT:
			player_t = -2.5*turnSpeed;
			break;
		case GLUT_KEY_RIGHT:
			player_t = 2.5*turnSpeed;
			break;
		default:
			break;
	}
#else
	switch( key )
	{
		case GLUT_KEY_UP:
			player_s = forwardSpeed;
			break;
		case GLUT_KEY_DOWN:
			player_s = -forwardSpeed;
			break;
		case GLUT_KEY_LEFT:
			player_t = -turnSpeed;
			break;
		case GLUT_KEY_RIGHT:
			player_t = turnSpeed;
			break;
		default:
			break;
	}
#endif
	glutPostRedisplay();
}

void upSpecialKeyboard(int key,int x,int y)
{
	switch (key)
	{
		case GLUT_KEY_LEFT:
			if(player_t<0.0f) player_t = 0.0f;
			break;
		case GLUT_KEY_RIGHT:
			if(player_t>0.0f) player_t = 0.0f;
			break;
		case GLUT_KEY_UP:
			player_s = 0.0f;
			break;
		case GLUT_KEY_DOWN:
			player_s = 0.0f;
			break;
		default:break;
	}
	glutPostRedisplay();
}

int main (int argc, char ** argv){
	#ifdef __OPENCV__
		trackerInit();
	#endif
	sstm << "MAZE" << level << ".txt";
	file = sstm.str();
	mazedata = readmaze(file);

    glewExperimental = GL_TRUE;
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(Window_Width,Window_Height);
    glutCreateWindow("Project");
	glutPositionWindow(0,0);
	glutMouseFunc (pickSquares);
	glutReshapeFunc (reshapeCallback);
    glutKeyboardFunc(keyboardCallback);
	glutSpecialFunc(specialKey);
	glutSpecialUpFunc(upSpecialKeyboard);
	glutTimerFunc(10,update,0);

    glutDisplayFunc(displayCallback);
	glewInit();
	lighting(0);
    setup();
	#ifdef __OPENCV__
		atexit(onExit);
	#endif
    glutMainLoop();
    return 0;
}

#ifdef __OPENCV__

void onExit()
{
	  cvDestroyAllWindows() ;
      cvReleaseImage(&imgTracking);
      cvReleaseCapture(&capture);   
}

void setwindowSettings(){
	cvNamedWindow("Threshold");
	cvNamedWindow("Boundary");
	cvNamedWindow("Settings");

	cvCreateTrackbar("LowerH", "Settings", &lowerH, 256, NULL);
	cvCreateTrackbar("UpperH", "Settings", &upperH, 256, NULL);

	cvCreateTrackbar("LowerS", "Settings", &lowerS, 256, NULL);
	cvCreateTrackbar("UpperS", "Settings", &upperS, 256, NULL);

	cvCreateTrackbar("LowerV", "Settings", &lowerV, 256, NULL);
	cvCreateTrackbar("UpperV", "Settings", &upperV, 256, NULL);
}

int trackerInit(){     
	capture = cvCaptureFromCAM(0);
	cvSetCaptureProperty(capture, CV_CAP_PROP_FPS,1);
	//cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_WIDTH, 480 );
	//cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_HEIGHT, 480 );
	minX = 0.0;
	maxX = cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH);
	minY = 0.0;
	maxY = cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT);
	midX = maxX/2;
	midY = maxY/2;
	if(!capture){
		printf("Capture failure\n");
		return -1;
	}
	setwindowSettings();
	frame = cvQueryFrame(capture);           
	if(!frame)
		return -1;
  
	imgTracking=cvCreateImage(cvGetSize(frame),IPL_DEPTH_8U, 3);
	trackFrame();
	return 0;
}

IplImage* GetThresholdedImage(IplImage* imgHSV){
	IplImage* imgThresh=cvCreateImage(cvGetSize(imgHSV),IPL_DEPTH_8U, 1);
	cvInRangeS(imgHSV, cvScalar(lowerH,lowerS,lowerV), cvScalar(upperH,upperS,upperV), imgThresh);
	return imgThresh;
}

void trackObject(IplImage* imgThresh){
	CvMoments *moments = (CvMoments*)malloc(sizeof(CvMoments));
	cvMoments(imgThresh, moments, 1);
	double moment10 = cvGetSpatialMoment(moments, 1, 0);
	double moment01 = cvGetSpatialMoment(moments, 0, 1);
	double area = cvGetCentralMoment(moments, 0, 0);

	if(area>100){
		int posX = moment10/area;
		int posY = moment01/area;  
		if(lastX>=0 && lastY>=0 && posX>=0 && posY>=0)
		{
			cvLine(imgTracking, cvPoint(2*midX-posX, posY), cvPoint(2*midX-lastX, lastY), cvScalar(0,0,255), 5);
		}

		lastX = posX;
		lastY = posY;

		int threshold = 75;

		if(track){
			if( lastY < midY - threshold)
				player_s = 2.0*forwardSpeed;
			else if( lastY > midY + threshold)
				player_s = -2.0*forwardSpeed;
			else
				player_s = 0.0f;

			if( lastX < midX - threshold)
				player_t = 1.5*turnSpeed;
			else if( lastX > midX + threshold)
				player_t = -1.5*turnSpeed;
			else
				player_t = 0.0f;
		}

		cvLine(imgTracking, cvPoint(midX, minY), cvPoint(midX, maxY), cvScalar(0,0,0), 1);
		cvLine(imgTracking, cvPoint(midX + threshold, minY), cvPoint(midX + threshold, maxY), cvScalar(50,50,50), 1);
		cvLine(imgTracking, cvPoint(midX - threshold, minY), cvPoint(midX - threshold, maxY), cvScalar(50,50,50), 1);

		cvLine(imgTracking, cvPoint(minX, midY), cvPoint(maxX, midY), cvScalar(0,0,0), 1);
		cvLine(imgTracking, cvPoint(minX, midY + threshold), cvPoint(maxX, midY + threshold), cvScalar(50,50,50), 1);
		cvLine(imgTracking, cvPoint(minX, midY - threshold), cvPoint(maxX, midY - threshold), cvScalar(50,50,50), 1);
	}
	free(moments);
}

void trackFrame(){
	frame = cvQueryFrame(capture);
	imgTracking=cvCreateImage(cvGetSize(frame),IPL_DEPTH_8U, 3);
	if(!frame) return;
	frame=cvCloneImage(frame); 
            
	cvSmooth(frame, frame, CV_GAUSSIAN,3,3);

	IplImage* imgHSV = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 3); 
	cvCvtColor(frame, imgHSV, CV_BGR2HSV);
	IplImage* imgThresh = GetThresholdedImage(imgHSV);
          
	cvSmooth(imgThresh, imgThresh, CV_GAUSSIAN,3,3);
            
	trackObject(imgThresh);

	cvShowImage("Threshold", imgThresh);
	cvShowImage("Boundary", imgTracking);       
                 
	cvReleaseImage(&imgHSV);
	cvReleaseImage(&imgThresh);            
	cvReleaseImage(&frame);
	cvReleaseImage(&imgTracking);
}
#endif