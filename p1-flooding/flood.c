#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "grid.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

GLfloat red[3] = {1.0, 0.0, 0.0};
GLfloat green[3] = {0.0, 1.0, 0.0};
GLfloat blue[3] = {0.0, 0.0, 1.0};
GLfloat black[3] = {0.0, 0.0, 0.0};
GLfloat white[3] = {1.0, 1.0, 1.0};
GLfloat gray[3] = {0.5, 0.5, 0.5};
GLfloat yellow[3] = {1.0, 1.0, 0.0};
GLfloat magenta[3] = {1.0, 0.0, 1.0};
GLfloat cyan[3] = {0.0, 1.0, 1.0};

GLint fillmode = 0;

const int WINDOWSIZE = 500;
const int NUMCOLORS = 4;
GLfloat pos[3] = {0, 0, 0};
GLfloat theta[3] = {0, 0, 0};
int res = 1;
int res2 = 1;
int mag = 528;

Grid *grid;
float seaLevel = 0.0;

float rise = 1.0;




//forward declarations 
void getArgs(int argc, char *argv[], char **file);
void directions();
void getColor(float elev, Grid *grid, GLfloat *color);
void gridRender(Grid *grid, float s);
void display(void);
int max(int a, int b);
void keypress(unsigned char key, int x, int y);


void getArgs(int argc, char *argv[], char **file)
{
  if (argc <= 1) {
    directions();
    exit(1);
  }
  
  for (int i = 1; i < argc; ++i) {
    
    //check for options
    
    if (i + 1 >= argc) {printf("No argument provided to %s.\nExiting.\n", argv[i]); exit(1);}
    
    if (strcmp(argv[i], "-input") == 0) {
      
      *file = argv[++i];
      if (access(*file, F_OK) == -1) {
	
        printf("No such input file: %s\n", *file);
        exit(1);
      }
      continue;
    }
    if (strcmp(argv[i], "-rise") == 0)
    {
      if (sscanf(argv[++i], "%f", &rise) != 1) {
      printf("error\n");
      exit(1);
      }  
      continue;
    }
    
    printf("Unknown argument '%s'\nExiting.", argv[i]);
    
    directions();
    exit(1);
    
  }
  
  
  //check arguments 
  if (*file == NULL) {
    printf("ERROR: must enter value for file\n");
    exit(1);
  }
}

int max(int a, int b)
{
  if (a >= b) {
    return a;
  }
  return b;
}

void directions()
{
  printf("\nflood: \n");
  printf("\t-input <input file>\n");
  printf("\t-rise <sea level rise increment>\n");
}

void getColor(float elev, Grid *grid, GLfloat *color)
{
	if (elev == grid->noDataValue)
	{
		color[0] = 0.0;
		color[1] = 1.0;
		color[2] = 1.0;
		return;
	}
  
  if (grid->max == grid->noDataValue) {
    //blue
    //whole grid is water
    color[0] = 0.0;
    color[1] = 0.0;
    color[2] = 1.0;
    return;
  }
  
  if (grid->max == grid->min) {
    //gray
    color[0] = 0.5;
    color[1] = 0.5;
    color[2] = 0.5;
    return;
  }
  
  float shade = (elev - grid->min) / (grid->max - grid->min);

  color[0] = shade;
  color[1] = shade;
  color[2] = shade;

}



int main(int argc, char * argv[])
{
  char* gridFileName;  //input grid file 
  
  getArgs(argc, argv, &gridFileName);
  
  grid = (Grid*)malloc(sizeof(Grid));
  assert(grid);
  readGridFromFile(gridFileName, grid);
  findExtrema(grid);
  mag = (grid->max - grid->min) * 4;
  findWater(grid);
  fflush(stdout); 
  
  /* initialize GLUT  */
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(WINDOWSIZE, WINDOWSIZE);
  glutInitWindowPosition(100,100);
  glutCreateWindow(argv[0]);

  //set up projection  
  glMatrixMode(GL_PROJECTION); 
  glLoadIdentity(); 
  /* the frustrum is from z=-1 to z=-10 */ /* camera is at (0,0,0) looking along negative y axis */
  gluPerspective(60, 1 /* aspect */, .1, 10.0); 
  
  /* register callback functions */
  glutDisplayFunc(display); 
  glutKeyboardFunc(keypress);
  
  /* init GL */
  /* set background color black*/
  glClearColor(0, 0, 0, 0);  
  /* here we can enable depth testing and double buffering and so
     on */
  /* give control to event handler */
  glutMainLoop();
  
  return 0;
}


void gridRender(Grid *grid, float s)
{
	assert(grid);

	flood(grid, seaLevel);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 

	GLfloat *color = malloc(3 * sizeof(GLfloat));

  for (int i = 0; i < grid->nrow - res; i += res) 
  {
    for (int j = 0; j < grid->ncol - res2; j += res2)
    {
    	getColor(grid->data[i][j], grid, color);
    	glColor3fv(color);
      glBegin(GL_TRIANGLES);
      glVertex3f(j, grid->nrow - 1 - i, isWater(grid->data[i][j], grid->noDataValue, s) / mag); 
      glVertex3f(j + res2, grid->nrow - 1 - i, isWater(grid->data[i+res][j], grid->noDataValue, s) / mag);
      glVertex3f(j, grid->nrow - 1 - i - res, isWater(grid->data[i][j+res2], grid->noDataValue, s) / mag);
      glEnd();

      glBegin(GL_TRIANGLES);
      glVertex3f(j + res2, grid->nrow - 1 - i - res, isWater(grid->data[i+res][j+res2], grid->noDataValue, s) / mag); 
      glVertex3f(j + res2, grid->nrow - 1 - i, isWater(grid->data[i+res][j], grid->noDataValue, s) / mag);
      glVertex3f(j, grid->nrow - 1 - i - res, isWater(grid->data[i][j+res], grid->noDataValue, s) / mag);
      glEnd();
    }
  }
}


/* ****************************** */
void display(void) {
  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW); 
  glLoadIdentity(); //clear the matrix
  
  //look from above (z=2) and back (y=-2)
  gluLookAt(0,-2,2, /* eye position */   
	    0,0,0, /* position of reference point indicating center of the scene*/  
	    0,0,1 /* direction of up vector */); 
  
  
  /* The default GL window is [-1,1]x[-1,1] with the origin in the
     center. 
     
     The points are in the range (0,0) to (WINSIZE,WINSIZE), so they
     need to be mapped to [-1,1]x [-1,1]x */
  glScalef(1.5/max(grid->nrow, grid->ncol), 1.5/max(grid->nrow, grid->ncol), 1.0);  
  glTranslatef(-grid->ncol/2, -grid->nrow/2, 0); 
  glTranslatef(pos[0], pos[1] , pos[2]);
  glRotatef(theta[0], 1,0,0); 
  glRotatef(theta[1], 0,1,0);  
  glRotatef(theta[2], 0,0,1);  
  gridRender(grid, seaLevel);
  
  
  /* execute the drawing commands */
  glFlush();
}

void keypress(unsigned char key, int x, int y) { 
  switch(key) { 
  case 'q': 
    exit(0); 
    break; 
    
  case 'd': 
    //move left  
    pos[0] -= 5;  //move left 5 cols  
    //glTranslatef(-5,0,0 );  
    glutPostRedisplay();  
    break;  
    
  case 'a':  
    //move right 
    pos[0] += 5; //move right 5 cols  
    // glTranslatef(5,0,0 );  
    glutPostRedisplay();  
    break; 
    
  case 's':  
    //move up  
    pos[1] += 5; //move up 5 rows  
    //glTranslatef(0,5,0 );  
    glutPostRedisplay();  
    break;  
    
  case 'w':  
    //move down  
    pos[1] -= 5; //move down 5 rows  
    //glTranslatef(0,-5,0);  
    glutPostRedisplay();  
    break;
    
  case 'f':  
    pos[2] += .2;  
    glutPostRedisplay();  
    break; 
    
  case 'b':
    pos[2] -= .2;  
    glutPostRedisplay();  
    break; 
    
  case 'x':  
    //glRotatef(5, 1,0,0);  
    theta[0] += .05;  
    glutPostRedisplay();  
    break;  
  case 'X':  
    theta[0] -= .05;  
    //glRotatef(-1, 1,0,0);  
    glutPostRedisplay();  
    break;  
    
  case 'y':  
    theta[1] += .05;  
    //glRotatef(5, 0,1,0);  
    glutPostRedisplay();  
    break;  
    
  case 'Y': 
    theta[1] -= .05;  
    //glRotatef(-5, 0,1,0);  
    glutPostRedisplay();  
    break;  
    
  case 'z': 
    //rotate around z 
    theta[2] += 5;  
    /* we are in object system of coordinates; the default rotation is 
       wrt the origin (ie lower left corner of the grid), but we want 
       to rotate wrt the middle of the grid 
    */ 
    //glTranslatef(ncols/2, nrows/2, 0);  
    //glRotatef(5, 0,0,1);  
    //glTranslatef(-ncols/2, -nrows/2, 0);  
    glutPostRedisplay();  
    break;  
    
  case 'Z': 
    //rotate around z 
    theta[2] -= 5;  
    //glTranslatef(ncols/2, nrows/2, 0);  
    //glRotatef(5, 0,0,-1);  
    //glTranslatef(-ncols/2, -nrows/2, 0);  
    glutPostRedisplay();  
    break;  
    
  case 'r':
    //decrease resolution
    res += grid->nrow/100;
    res2 += grid->ncol/100;
    glutPostRedisplay();
    break;
    
  case 'R':
    // increase resolution
    if (res >= grid->nrow/100) {
      res -= grid->nrow/100;
      res2 -= grid->ncol/100;
    }
    glutPostRedisplay();
    break;
    
  case 'm':
    //decrease magnitude of elevation values
    mag *= 1.5;
    glutPostRedisplay();
    break;
    
  case 'M':
    //increase the magnitude of elevation values
    if (mag >= 1.5) {
      mag /= 1.5;
    }
    glutPostRedisplay();
    break;

  case 'g':
  	//increase sea level
  	seaLevel += rise;
  	glutPostRedisplay();
  	break;

  } 
  
}


