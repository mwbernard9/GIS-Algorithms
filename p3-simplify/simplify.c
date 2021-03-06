#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

#include "grid.h"
#include "rtimer.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#define NORMAL 0
#define SIMULATION 1
#define TIN_THREED 2
#define TIN_TWOD 3

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
SimpGrid *simp;

int viewType = TIN_THREED;
float epsilon = 100;
int simIsDone = 0;


//forward declarations 
void getArgs(int argc, char *argv[], char **file, char **output);
void directions();
void getColor(float elev, Grid *grid, GLfloat *color);
void gridRender(Grid *grid);
void tinRender3D(SimpGrid *simp);
void tinRender2D(SimpGrid *simp);
void display(void);
int max(int a, int b);
void keypress(unsigned char key, int x, int y);


void getArgs(int argc, char *argv[], char **file, char **output)
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

    if (strcmp(argv[i], "-output") == 0) {
      *output = argv[++i];
      continue;
    }
    
    if (strcmp(argv[i], "-epsilon") == 0)
    {
      if (sscanf(argv[++i], "%f", &epsilon) != 1) {
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
    printf("ERROR: must enter value for input file\n");
    exit(1);
  }

  if (*output == NULL) {
    printf("ERROR: must enter value for output file\n");
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
  printf("\ngridRender: \n");
  printf("\t-input <input file>\n");
  printf("\t-output <output file>\n");
  printf("\t-epsilon <epsilon value>\n");
}

void getColor(float elev, Grid *grid, GLfloat *color)
{
  
  if (grid->max == grid->noDataValue) {
    //black
    color[0] = 0.0;
    color[1] = 0.0;
    color[2] = 0.0;
    return;
  }
  
  if (grid->max == grid->min) {
    //yellow
    color[0] = 1.0;
    color[1] = 1.0;
    color[2] = 0.0;
    return;
  }
  
  float interval = (grid->max - grid->min) / NUMCOLORS;
  
  int genColor = (int)((elev - grid->min) / interval);
  
  float shade = .18 + (.75 * ((elev - grid->min) - (genColor * interval)) / interval);
  
  switch (genColor) {
    //blue
  case 0:
    color[0] = 0.0;
    color[1] = 0.0;
    color[2] = shade;
    break;
    //cyan
  case 1:
    color[0] = 0.0;
    color[1] = shade;
    color[2] = shade;
    break;
    //green
  case 2:
    color[0] = 0.0;
    color[1] = shade;
    color[2] = 0.0;
    break;
    //yellow
  case 3:
    color[0] = shade;
    color[1] = shade;
    color[2] = 0.0;
    break;
  }
}



int main(int argc, char * argv[])
{
  char* gridFileName = NULL;  //input grid file 
  char* outputFileName = NULL;
  getArgs(argc, argv, &gridFileName, &outputFileName);
  
  printf("reading %s into memory...", gridFileName);
  Rtimer rt1; 
  rt_start(rt1);

  grid = (Grid*)malloc(sizeof(Grid));
  assert(grid);
  readGridFromFile(gridFileName, grid);
  int totalPoints = grid->nrow * grid->ncol;
  findExtrema(grid);
  mag = (grid->max - grid->min) * 4;
  //fflush(stdout); 

  simp = (SimpGrid*)malloc(sizeof(SimpGrid));

  initSimpGrid(simp, grid);

  MaxHeap hp = initMaxHeap(grid->nrow * grid->ncol);
  initTin(grid, simp, &hp);

  rt_stop(rt1); 
  char buf [1024]; 
  rt_sprint(buf,rt1);
  printf("done\n");
  printf("total time: %s\n", buf);
  printf("---------\n");
  fflush(stdout);

  printf("starting simplification\n");
  printf("n = %d\n", totalPoints);
  printf("...\n");
  Rtimer rt2; 
  rt_start(rt2);
  simplify(simp, grid, &hp, epsilon);
  rt_stop(rt2); 
  rt_sprint(buf,rt2);
  printf("done. n' = %d (%.2f%% of %d)\n", simp->numPoints, (((float)simp->numPoints) / totalPoints) * 100, totalPoints);
  printf("total time: %s\n", buf);
  printf("---------\n");
  printf("writing TIN to file\n");
  fflush(stdout);

  tinToFile(simp, outputFileName);
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


void gridRender(Grid *grid)
{
  assert(grid);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 

  GLfloat *color = malloc(3 * sizeof(GLfloat));
  
  

  for (int i = 0; i < grid->nrow - res; i += res) {
    for (int j = 0; j < grid->ncol - res2; j += res2) { 
      float h1 = grid->data[i][j];
      float h2 = grid->data[i+res][j];
      float h3 = grid->data[i][j+res2];
      float h4 = grid->data[i+res][j+res2];
      
      if (h1 == grid->noDataValue){
	h1 = grid->min;
      }
      if (h2 == grid->noDataValue) {
	h2 = grid->min;
      }
      if (h3 == grid->noDataValue) {
	h3 = grid->min;
      }
      if (h4 == grid->noDataValue) {
	h4 = grid->min;
      }

      getColor(grid->data[i][j], grid, color);
      glColor3fv(color);

      glBegin(GL_TRIANGLES);
      glVertex3f(j, grid->nrow - 1 - i, h1 / mag); 
      glVertex3f(j + res2, grid->nrow - 1 - i, h2 / mag);
      glVertex3f(j, grid->nrow - 1 - i - res, h3 / mag);
      glEnd();
      
      glBegin(GL_TRIANGLES);
      glVertex3f(j + res2, grid->nrow - 1 - i - res, h4 / mag); 
      glVertex3f(j + res2, grid->nrow - 1 - i, h2 / mag);
      glVertex3f(j, grid->nrow - 1 - i - res, h3 / mag);
      glEnd();
    }
  }
}

void tinRender3D(SimpGrid *simp)
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 
  	GLfloat *color = malloc(3 * sizeof(GLfloat));

	for (int i = 0; i < simp->numTriangles; i++)
	{
		getColor(simp->tin[i].a.z, &simp->newGrid, color);
		glColor3fv(color);

		glBegin(GL_TRIANGLES);
      	glVertex3f(simp->tin[i].a.x, simp->newGrid.nrow - 1 - simp->tin[i].a.y, simp->tin[i].a.z / mag); 

      	getColor(simp->tin[i].b.z, &simp->newGrid, color);
		glColor3fv(color);


      	glVertex3f(simp->tin[i].b.x, simp->newGrid.nrow - 1 - simp->tin[i].b.y, simp->tin[i].b.z / mag);

      	getColor(simp->tin[i].c.z, &simp->newGrid, color);
		glColor3fv(color);


      	glVertex3f(simp->tin[i].c.x, simp->newGrid.nrow - 1 - simp->tin[i].c.y, simp->tin[i].c.z / mag);
      	glEnd();
	}
}

void tinRender2D(SimpGrid *simp)
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 
  	GLfloat *color = malloc(3 * sizeof(GLfloat));

	for (int i = 0; i < simp->numTriangles; i++)
	{
		getColor(simp->tin[i].a.z, &simp->newGrid, color);
		glColor3fv(color);

		glBegin(GL_TRIANGLES);
      	glVertex2f(simp->tin[i].a.x, simp->newGrid.nrow - 1 - simp->tin[i].a.y); 

      	getColor(simp->tin[i].b.z, &simp->newGrid, color);
		glColor3fv(color);


      	glVertex2f(simp->tin[i].b.x, simp->newGrid.nrow - 1 - simp->tin[i].b.y);

      	getColor(simp->tin[i].c.z, &simp->newGrid, color);
		glColor3fv(color);


      	glVertex2f(simp->tin[i].c.x, simp->newGrid.nrow - 1 - simp->tin[i].c.y);
      	glEnd();
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


  if (viewType == NORMAL)
  {
  	gridRender(grid);
  } 
  else if (viewType == SIMULATION)
  {
    if (simIsDone == 0)
    {
      simIsDone = 1;
      computeEstimateGrid(simp, grid);
    }
  	gridRender(&simp->newGrid);
  }
  else if (viewType == TIN_THREED)
  {
  	tinRender3D(simp);
  }
  else if (viewType == TIN_TWOD)
  {
  	tinRender2D(simp);
  }
  
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
  	viewType = NORMAL;
  	glutPostRedisplay();
  	break;

  case 'h':
  	viewType = SIMULATION;
  	glutPostRedisplay();
  	break;

  case 'j':
  	viewType = TIN_THREED;
  	glutPostRedisplay();
  	break;

  case 'k':
  	viewType = TIN_TWOD;
  	glutPostRedisplay();
  	break;
  } 
  
}


