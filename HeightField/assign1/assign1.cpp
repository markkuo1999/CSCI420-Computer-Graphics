// assign1.cpp : Defines the entry point for the console application.
//

/*
  CSCI 420 Computer Graphics
  Assignment 1: Height Fields
  <Zong-Ying Kuo>
*/



#include "stdafx.h"
#include <windows.h>
#include <stdlib.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include "./CImg-2.3.5/CImg.h"
#include <iostream>

using namespace cimg_library;
using namespace std;
int g_iMenuId;

int g_vMousePos[2] = { 0, 0 };
int g_iLeftMouseButton = 0;    /* 1 if pressed, 0 if not */
int g_iMiddleMouseButton = 0;
int g_iRightMouseButton = 0;

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROLSTATE;
typedef enum { VERTEXS, LINES, TRIANGLES } DISPLAYSTATE; //choose display mode

CONTROLSTATE g_ControlState = ROTATE;
DISPLAYSTATE g_Shape = VERTEXS;//default display mode : GL_POINTS

/* state of the world */
float g_vLandRotate[3] = { 0.0, 0.0, 0.0 };
float g_vLandTranslate[3] = { -0.5, -0.5, 0.0 };
float g_vLandScale[3] = { 1.0, 1.0, 1.0 };

CImg<unsigned char>* g_pHeightData;

void reshape(int w, int h)
{
	GLfloat aspect = (GLfloat)w / (GLfloat)h;
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, 1.0 * aspect, 0.01, 1000.0);
	glMatrixMode(GL_MODELVIEW);
}

/* This line is required for CImg to be able to read jpg/png format files. */
/* Please install ImageMagick and replace the path below to the correct path to convert.exe on your computer */
void initializeImageMagick()
{
	cimg::imagemagick_path("convert.exe", true);
}


/* Write a screenshot to the specified filename */
void saveScreenshot(char* filename)
{
	int i, j;

	if (filename == NULL)
		return;

	/* Allocate a picture buffer */
	CImg<unsigned char> in(640, 480, 1, 3, 0);

	printf("File to save to: %s\n", filename);

	for (i = 479; i >= 0; i--) {
		glReadPixels(0, 479 - i, 640, 1, GL_RGB, GL_UNSIGNED_BYTE,
			in.data());
	}

	if (in.save_jpeg(filename))
		printf("File saved Successfully\n");
	else
		printf("Error in Saving\n");

}

void myinit()
{
	/* setup gl view here */
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glShadeModel(GL_SMOOTH);
}

void DrawImage() {

	int width = g_pHeightData->width();
	int height = g_pHeightData->height();
	float r, c;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (g_Shape == VERTEXS) {
		glBegin(GL_POINTS);
		for (r = 0; r < height; r++) {
			for (c = 0; c < width; c++) {
				glColor3f(0.5, 0.5, (float)*g_pHeightData->data(c, r) / 255);
				glVertex3f(c / 256, r / 256, (float)*g_pHeightData->data(c, r) / 255 / 10);//(float)*g_pHeightData->data(c, r) means get the grayscale value of point(r, c)
			}
		}
		glEnd();
	}
	else if (g_Shape == LINES) {
		glBegin(GL_LINES);
		for (r = 0; r < height - 1; r++) {
			for (c = 0; c < width - 1; c++) {
				/*Draw 5 lines(wireframe for triangles) for each 2 triangles*/

				/*Draw a line between point(c, r) and point(c + 1, r)*/
				glColor3f(0.5, 0.5, (float)*g_pHeightData->data(c, r) / 255);
				glVertex3f(c / 256, r / 256, (float)*g_pHeightData->data(c, r) / 255 / 10);
				glColor3f(0.5, 0.5, (float)*g_pHeightData->data(c + 1, r) / 255);
				glVertex3f((c + 1) / 256, r / 256, (float)*g_pHeightData->data(c + 1, r) / 255 / 10);
				/*Draw a line between point(c, r) and point(c, r + 1)*/
				glColor3f(0.5, 0.5, (float)*g_pHeightData->data(c, r) / 255);
				glVertex3f(c / 256, r / 256, (float)*g_pHeightData->data(c, r) / 255 / 10);
				glColor3f(0.5, 0.5, (float)*g_pHeightData->data(c, r + 1) / 255);
				glVertex3f(c / 256, (r + 1) / 256, (float)*g_pHeightData->data(c, r + 1) / 255 / 10);
				/*Draw a line between point(c + 1, r) and point(c, r + 1)*/
				glColor3f(0.5, 0.5, (float)*g_pHeightData->data(c + 1, r) / 255);
				glVertex3f((c + 1) / 256, r / 256, (float)*g_pHeightData->data(c + 1, r) / 255 / 10);
				glColor3f(0.5, 0.5, (float)*g_pHeightData->data(c, r + 1) / 255);
				glVertex3f(c / 256, (r + 1) / 256, (float)*g_pHeightData->data(c, r + 1) / 255 / 10);
				/*Draw a line between point(c + 1, r) and point(c + 1, r + 1)*/
				glColor3f(0.5, 0.5, (float)*g_pHeightData->data(c + 1, r) / 255);
				glVertex3f((c + 1) / 256, r / 256, (float)*g_pHeightData->data(c + 1, r) / 255 / 10);
				glColor3f(0.5, 0.5, (float)*g_pHeightData->data(c + 1, r + 1) / 255);
				glVertex3f((c + 1) / 256, (r + 1) / 256, (float)*g_pHeightData->data(c + 1, r + 1) / 255 / 10);
				/*Draw a line between point(c, r + 1) and point(c + 1, r + 1)*/
				glColor3f(0.5, 0.5, (float)*g_pHeightData->data(c, r + 1) / 255);
				glVertex3f(c / 256, (r + 1) / 256, (float)*g_pHeightData->data(c, r + 1) / 255 / 10);
				glColor3f(0.5, 0.5, (float)*g_pHeightData->data(c + 1, r + 1) / 255);
				glVertex3f((c + 1) / 256, (r + 1) / 256, (float)*g_pHeightData->data(c + 1, r + 1) / 255 / 10);
			}
		}
		glEnd();
	}
	else if (g_Shape == TRIANGLES) {
		glBegin(GL_TRIANGLES);
		for (r = 0; r < height - 1; r++) {
			for (c = 0; c < width - 1; c++) {
				/*Draw the first triangle*/
				glColor3f(0.5, 0.5, (float)*g_pHeightData->data(c, r) / 255);
				glVertex3f(c / 256, r / 256, (float)*g_pHeightData->data(c, r) / 255 / 10);
				glColor3f(0.5, 0.5, (float)*g_pHeightData->data(c + 1, r) / 255);
				glVertex3f((c + 1) / 256, r / 256, (float)*g_pHeightData->data(c + 1, r) / 255 / 10);
				glColor3f(0.5, 0.5, (float)*g_pHeightData->data(c, r + 1) / 255);
				glVertex3f((c) / 256, (r + 1) / 256, (float)*g_pHeightData->data(c, r + 1) / 255 / 10);
				/*Draw the second trinagle*/
				glColor3f(0.5, 0.5, (float)*g_pHeightData->data(c + 1, r) / 255);
				glVertex3f((c + 1) / 256, r / 256, (float)*g_pHeightData->data(c + 1, r) / 255 / 10);
				glColor3f(0.5, 0.5, (float)*g_pHeightData->data(c, r + 1) / 255);
				glVertex3f((c) / 256, (r + 1) / 256, (float)*g_pHeightData->data(c, r + 1) / 255 / 10);
				glColor3f(0.5, 0.5, (float)*g_pHeightData->data(c + 1, r + 1) / 255);
				glVertex3f((c + 1) / 256, (r + 1) / 256, (float)*g_pHeightData->data(c + 1, r + 1) / 255 / 10);
			}
		}
		glEnd();
	}

}

void display()
{
	/* draw 1x1 cube about origin */
	/* replace this code with your height field implementation */
	/* you may also want to precede it with your
  rotation/translation/scaling */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear buffer
	glLoadIdentity();
	gluLookAt(0, 0, 1, 0, 0, -1, 0, 1, 0);
	glScalef(g_vLandScale[0], g_vLandScale[1], g_vLandScale[2]);
	glTranslatef(g_vLandTranslate[0], g_vLandTranslate[1], g_vLandTranslate[2]);
	glRotatef(g_vLandRotate[0], 1, 0, 0);
	glRotatef(g_vLandRotate[1], 0, 1, 0);
	glRotatef(g_vLandRotate[2], 0, 0, 1);
	DrawImage();
	glutSwapBuffers();
}

void menufunc(int value)
{
	switch (value)
	{
	case 0:
		exit(0);
		break;
	}
}

void doIdle()
{
	/* do some stuff... */

	/* make the screen update */
	glutPostRedisplay();
}

/* converts mouse drags into information about
rotation/translation/scaling */
void mousedrag(int x, int y)
{
	int vMouseDelta[2] = { x - g_vMousePos[0], y - g_vMousePos[1] };

	switch (g_ControlState)
	{
	case TRANSLATE:
		if (g_iLeftMouseButton)
		{
			g_vLandTranslate[0] += vMouseDelta[0] * 0.01;
			g_vLandTranslate[1] -= vMouseDelta[1] * 0.01;
		}
		if (g_iMiddleMouseButton)
		{
			g_vLandTranslate[2] += vMouseDelta[1] * 0.01;
		}
		break;
	case ROTATE:
		if (g_iLeftMouseButton)
		{
			g_vLandRotate[0] += vMouseDelta[1];
			g_vLandRotate[1] += vMouseDelta[0];
		}
		if (g_iMiddleMouseButton)
		{
			g_vLandRotate[2] += vMouseDelta[1];
		}
		break;
	case SCALE:
		if (g_iLeftMouseButton)
		{
			g_vLandScale[0] *= 1.0 + vMouseDelta[0] * 0.01;
			g_vLandScale[1] *= 1.0 - vMouseDelta[1] * 0.01;
		}
		if (g_iMiddleMouseButton)
		{
			g_vLandScale[2] *= 1.0 - vMouseDelta[1] * 0.01;
		}
		break;
	}
	g_vMousePos[0] = x;
	g_vMousePos[1] = y;
}

void mouseidle(int x, int y)
{
	g_vMousePos[0] = x;
	g_vMousePos[1] = y;
}

void mousebutton(int button, int state, int x, int y)
{

	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		g_iLeftMouseButton = (state == GLUT_DOWN);
		break;
	case GLUT_MIDDLE_BUTTON:
		g_iMiddleMouseButton = (state == GLUT_DOWN);
		break;
	case GLUT_RIGHT_BUTTON:
		g_iRightMouseButton = (state == GLUT_DOWN);
		break;
	}

	switch (glutGetModifiers())
	{
	case GLUT_ACTIVE_CTRL:
		g_ControlState = TRANSLATE;
		break;
	case GLUT_ACTIVE_SHIFT:
		g_ControlState = SCALE;
		break;
	default:
		g_ControlState = ROTATE;
		break;
	}

	g_vMousePos[0] = x;
	g_vMousePos[1] = y;
}

void keyboard(unsigned char key, int x, int y)
{
	if (key == 'p' || key == 'P')/*change to point mode by pressing 'P'*/
		g_Shape = VERTEXS;
	if (key == 'l' || key == 'L')/*change to line mode by pressing 'L'*/
		g_Shape = LINES;
	if (key == 't' || key == 'T')/*change to solid triangle mode by pressing 'T'*/
		g_Shape = TRIANGLES;
	else
		glutIdleFunc(doIdle);;
}

int main(int argc, char* argv[])
{
	// I've set the argv[1] to spiral.jpg.
	// To change it, on the "Solution Explorer",
	// right click "assign1", choose "Properties",
	// go to "Configuration Properties", click "Debugging",
	// then type your texture name for the "Command Arguments"
	if (argc < 2)
	{
		printf("usage: %s heightfield.jpg\n", argv[0]);
		exit(1);
	}

	initializeImageMagick();

	g_pHeightData = new CImg<unsigned char>((char*)argv[1]);
	if (!g_pHeightData)
	{
		printf("error reading %s.\n", argv[1]);
		exit(1);
	}

	/*
		create a window here..should be double buffered and use depth testing

		the code past here will segfault if you don't have a window set up....
		replace the exit once you add those calls.


	*/
	glutInit(&argc, (char**)argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(640, 480);
	glutInitWindowPosition(100, 100);
	glutCreateWindow(argv[0]);
	/* tells glut to use a particular display function to redraw */
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	/* allow the user to quit using the right mouse button menu */
	g_iMenuId = glutCreateMenu(menufunc);
	glutSetMenu(g_iMenuId);
	glutAddMenuEntry("Quit", 0);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	/* replace with any animate code */
	glutIdleFunc(doIdle);

	/* callback for mouse drags */
	glutMotionFunc(mousedrag);
	/* callback for idle mouse movement */
	glutPassiveMotionFunc(mouseidle);
	/* callback for mouse button changes */
	glutMouseFunc(mousebutton);

	glutKeyboardFunc(keyboard);
	myinit();
	glEnable(GL_DEPTH_TEST);
	//saveScreenshot("GrandTeton-256-HF.jpg");
	glutMainLoop();
	/* do initialization */

	return 0;
}