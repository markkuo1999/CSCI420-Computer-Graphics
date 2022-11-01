// assign2.cpp : Defines the entry point for the console application.
//

/*
	CSCI 480 Computer Graphics
	Assignment 2: Simulating a Roller Coaster
	C++ starter code
	Zong-Ying Kuo
*/

#include "stdafx.h"
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <iostream>
#include <GL/glu.h>
#include <GL/glut.h>
#include "opencv2/core/core.hpp"
#include "opencv2/core/utility.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgcodecs/imgcodecs.hpp"
using namespace std;
using namespace cv;
/* Object where you can load an image */
cv::Mat3b imageBGR;
GLuint texName[3];
int index = 0;

/* represents one control point along the spline */
struct point {
	double x;
	double y;
	double z;
};

/* spline struct which contains how many control points, and an array of control points */
struct spline {
	int numControlPoints;
	struct point *points;
};

/* the spline array */
struct spline *g_Splines;

/*represent each point on the spline*/
class EverySplinePoint {
public:
	double X, Y, Z;
	double N[3], T[3], B[3];
};

class EverySplinePoint P[2410];

/* total number of splines */
int g_iNumOfSplines;

int g_vMousePos[2] = { 0, 0 };
int g_iLeftMouseButton = 0;    /* 1 if pressed, 0 if not */
int g_iMiddleMouseButton = 0;
int g_iRightMouseButton = 0;

/* state of the world */
float g_vLandRotate[3] = { 0.0, 0.0, 0.0 };
float g_vLandTranslate[3] = { 0.0, 0.0, 0.0 };
float g_vLandScale[3] = { 1.0, 1.0, 1.0 };

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROLSTATE;

CONTROLSTATE g_ControlState = ROTATE;

int loadSplines(char *argv) {
	char *cName = (char *)malloc(128 * sizeof(char));
	FILE *fileList;
	FILE *fileSpline;
	int iType, i = 0, j, iLength;

	/* load the track file */
	fileList = fopen(argv, "r");
	if (fileList == NULL) {
		printf ("can't open file\n");
		exit(1);
	}
	/* stores the number of splines in a global variable */
	fscanf(fileList, "%d", &g_iNumOfSplines);
	g_Splines = (struct spline *)malloc(g_iNumOfSplines * sizeof(struct spline));

	/* reads through the spline files */
	for (j = 0; j < g_iNumOfSplines; j++) {
		i = 0;
		fscanf(fileList, "%s", cName);
		
		fileSpline = fopen(cName, "r");

		if (fileSpline == NULL) {
			printf ("can't open file\n");
			exit(1);
		}

		/* gets length for spline file */
		fscanf(fileSpline, "%d %d", &iLength, &iType);

		/* allocate memory for all the points */
		g_Splines[j].points = (struct point *)malloc(iLength * sizeof(struct point));
		g_Splines[j].numControlPoints = iLength;
		/* saves the data to the struct */
		while (fscanf(fileSpline, "%lf %lf %lf", 
			&g_Splines[j].points[i].x, 
			&g_Splines[j].points[i].y, 
			&g_Splines[j].points[i].z) != EOF) {
			i++;
		}
	}

	free(cName);

	return 0;
}

/* Write a screenshot to the specified filename */
void saveScreenshot(char *filename)
{
	if (filename == NULL)
		return;

	// Allocate a picture buffer // 
	cv::Mat3b bufferRGB = cv::Mat::zeros(480, 640, CV_8UC3); //rows, cols, 3-channel 8-bit.
	printf("File to save to: %s\n", filename);

	//use fast 4-byte alignment (default anyway) if possible
	glPixelStorei(GL_PACK_ALIGNMENT, (bufferRGB.step & 3) ? 1 : 4);
	//set length of one complete row in destination data (doesn't need to equal img.cols)
	glPixelStorei(GL_PACK_ROW_LENGTH, bufferRGB.step / bufferRGB.elemSize());
	glReadPixels(0, 0, bufferRGB.cols, bufferRGB.rows, GL_RGB, GL_UNSIGNED_BYTE, bufferRGB.data);
	//flip to account for GL 0,0 at lower left
	cv::flip(bufferRGB, bufferRGB, 0);
	//convert RGB to BGR
	cv::Mat3b bufferBGR(bufferRGB.rows, bufferRGB.cols, CV_8UC3);
	cv::Mat3b out[] = { bufferBGR };
	// rgb[0] -> bgr[2], rgba[1] -> bgr[1], rgb[2] -> bgr[0]
	int from_to[] = { 0,2, 1,1, 2,0 };
	mixChannels(&bufferRGB, 1, out, 1, from_to, 3);

	if (cv::imwrite(filename, bufferBGR)) {
		printf("File saved Successfully\n");
	}
	else {
		printf("Error in Saving\n");
	}
}

/* Function to get a pixel value. Use like PIC_PIXEL macro. 
Note: OpenCV images are in channel order BGR. 
This means that:
chan = 0 returns BLUE, 
chan = 1 returns GREEN, 
chan = 2 returns RED. */
unsigned char getPixelValue(cv::Mat3b& image, int x, int y, int chan)
{
	return image.at<cv::Vec3b>(y, x)[chan];
}

/* Function that does nothing but demonstrates looping through image coordinates.*/
void loopImage(cv::Mat3b& image)
{
	for (int r = 0; r < image.rows; r++) { // y-coordinate
		for (int c = 0; c < image.cols; c++) { // x-coordinate
			for (int channel = 0; channel < 3; channel++) {
				// DO SOMETHING... example usage
				unsigned char blue = getPixelValue(image, c, r, 0);
				unsigned char green = getPixelValue(image, c, r, 1); 
				unsigned char red = getPixelValue(image, c, r, 2);
			}
		}
	}
}

/* Read an image into memory. 
Set argument displayOn to true to make sure images are loaded correctly.
One image loaded, set to false so it doesn't interfere with OpenGL window.*/
int readImage(char *filename, cv::Mat3b& image, bool displayOn)
{
	//std::cout << "reading image: " << filename << std::endl;
	image = cv::imread(filename);
	if (!image.data) // Check for invalid input                    
	{
		std::cout << "Could not open or find the image." << std::endl;
		return 1;
	}

	if (displayOn)
	{
		cv::imshow("TestWindow", image);
		cv::waitKey(0); // Press any key to enter. 
	}
	return 0;
}

void init()
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	// glShadeModel (GL_FLAT);
	glShadeModel(GL_SMOOTH);
}

void initTexture()
{
	
	readImage("snow.jpg", imageBGR, false);
	
	glGenTextures(1, &texName[0]);
	
	// create placeholder for texture
	 // must declare a global variable in program header: GLUint texName
	glBindTexture(GL_TEXTURE_2D, texName[0]); // make texture ¡§texName¡¨ the currently active texture

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	// specify texture parameters (they affect whatever texture is active) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	// repeat pattern in s
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// repeat pattern in t

	// use linear filter both for magnification and minification
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	
	// load image data stored at pointer ¡§pointerToImage¡¨ into the currently active texture(¡§texName¡¨)
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageBGR.cols, imageBGR.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, imageBGR.data);

	readImage("sky2.jpg", imageBGR, false);
	glGenTextures(1, &texName[1]);

	// create placeholder for texture
	 // must declare a global variable in program header: GLUint texName
	glBindTexture(GL_TEXTURE_2D, texName[1]); // make texture ¡§texName¡¨ the currently active texture

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	// specify texture parameters (they affect whatever texture is active) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	// repeat pattern in s
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// repeat pattern in t

	// use linear filter both for magnification and minification
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// load image data stored at pointer ¡§pointerToImage¡¨ into the currently active texture(¡§texName¡¨)
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageBGR.cols, imageBGR.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, imageBGR.data);
}



void reshape(int w, int h)
{
	GLfloat aspect = (GLfloat)w / (GLfloat)h;
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, 1.0 * aspect, 0.01, 1000.0);
	glMatrixMode(GL_MODELVIEW);
}

void crossProduct(double v_A[], double v_B[], double c_P[]) {
	c_P[0] = v_A[1] * v_B[2] - v_A[2] * v_B[1];
	c_P[1] = -(v_A[0] * v_B[2] - v_A[2] * v_B[0]);
	c_P[2] = v_A[0] * v_B[1] - v_A[1] * v_B[0];
	double Normalizefactor = sqrt(c_P[0] * c_P[0] + c_P[1] * c_P[1] + c_P[2] * c_P[2]);
	c_P[0] /= Normalizefactor;
	c_P[1] /= Normalizefactor;
	c_P[2] /= Normalizefactor;
}


void drawsplines() {
	int i, j;
	int count;

	double h[4][3], s;
	double M[4][4], UM[4], V[3];
	double u;
	double Normalizefactor;
	double TubeV0[3], TubeV1[3], TubeV2[3], TubeV3[3], alpha;
	alpha = 1;
	count = 0;
	s = 0.5;
	M[0][0] = 2;
	M[0][1] = -2;
	M[0][2] = 1;
	M[0][3] = 1;
	M[1][0] = -3;
	M[1][1] = 3;
	M[1][2] = -2;
	M[1][3] = -1;
	M[2][0] = 0;
	M[2][1] = 0;
	M[2][2] = 1;
	M[2][3] = 0;
	M[3][0] = 1;
	M[3][1] = 0;
	M[3][2] = 0;
	M[3][3] = 0;
	V[0] = 1;
	V[1] = 1;
	V[2] = 1;
	double max = -1000;
	for (j = 0; j < g_iNumOfSplines; j++) {
		for (i = 1; i < g_Splines[j].numControlPoints; i++) {
			h[0][0] = g_Splines[j].points[i].x;
			h[0][1] = g_Splines[j].points[i].y;
			h[0][2] = g_Splines[j].points[i].z;

			h[1][0] = g_Splines[j].points[i + 1].x;
			h[1][1] = g_Splines[j].points[i + 1].y;
			h[1][2] = g_Splines[j].points[i + 1].z;

			h[2][0] = s * (g_Splines[j].points[i + 1].x - g_Splines[j].points[i - 1].x);
			h[2][1] = s * (g_Splines[j].points[i + 1].y - g_Splines[j].points[i - 1].y);
			h[2][2] = s * (g_Splines[j].points[i + 1].z - g_Splines[j].points[i - 1].z);

			h[3][0] = s * (g_Splines[j].points[i + 2].x - g_Splines[j].points[i].x);
			h[3][1] = s * (g_Splines[j].points[i + 2].y - g_Splines[j].points[i].y);
			h[3][2] = s * (g_Splines[j].points[i + 2].z - g_Splines[j].points[i].z);

			u = 0;
			while (u < 1) {
				/*Compute u * M * h*/
				UM[0] = u * u * u * M[0][0] + u * u * M[1][0] + u * M[2][0] + M[3][0];
				UM[1] = u * u * u * M[0][1] + u * u * M[1][1] + u * M[2][1] + M[3][1];
				UM[2] = u * u * u * M[0][2] + u * u * M[1][2] + u * M[2][2] + M[3][2];
				UM[3] = u * u * u * M[0][3] + u * u * M[1][3] + u * M[2][3] + M[3][3];
				P[count].X = UM[0] * h[0][0] + UM[1] * h[1][0] + UM[2] * h[2][0] + UM[3] * h[3][0] - 5.0;
				P[count].Y = UM[0] * h[0][1] + UM[1] * h[1][1] + UM[2] * h[2][1] + UM[3] * h[3][1] - 20.0;
				P[count].Z = UM[0] * h[0][2] + UM[1] * h[1][2] + UM[2] * h[2][2] + UM[3] * h[3][2] - 47.5;
				if (P[count].Z > max)
					max = P[count].Z;
				
				UM[0] = 3 * u * u * M[0][0] + 2 * u * M[1][0] + M[2][0];
				UM[1] = 3 * u * u * M[0][1] + 2 * u * M[1][1] + M[2][1];
				UM[2] = 3 * u * u * M[0][2] + 2 * u * M[1][2] + M[2][2];
				UM[3] = 3 * u * u * M[0][3] + 2 * u * M[1][3] + M[2][3];
				/*Compute Tangent, Normal and B Vector*/
				P[count].T[0] = UM[0] * h[0][0] + UM[1] * h[1][0] + UM[2] * h[2][0] + UM[3] * h[3][0];
				P[count].T[1] = UM[0] * h[0][1] + UM[1] * h[1][1] + UM[2] * h[2][1] + UM[3] * h[3][1];
				P[count].T[2] = UM[0] * h[0][2] + UM[1] * h[1][2] + UM[2] * h[2][2] + UM[3] * h[3][2];
				Normalizefactor = sqrt(P[count].T[0] * P[count].T[0] + P[count].T[1] * P[count].T[1] + P[count].T[2] * P[count].T[2]);
				P[count].T[0] /= Normalizefactor;
				P[count].T[1] /= Normalizefactor;
				P[count].T[2] /= Normalizefactor;
				if (u == 0 && i == 1) {
					crossProduct(P[count].T, V, P[count].N);
					crossProduct(P[count].T, P[count].N, P[count].B);
				}
				else {
					crossProduct(P[count - 1].B, P[count].T, P[count].N);
					crossProduct(P[count].T, P[count].N, P[count].B);
				}
				/*Compute the cross-section*/
				TubeV0[0] = P[count].X + alpha * (-P[count].N[0] + P[count].B[0]); TubeV0[1] = P[count].Y + alpha * (-P[count].N[1] + P[count].B[1]); TubeV0[2] = P[count].Z + alpha * (-P[count].N[2] + P[count].B[2]);
				TubeV1[0] = P[count].X + alpha * (P[count].N[0] + P[count].B[0]); TubeV1[1] = P[count].Y + alpha * (P[count].N[1] + P[count].B[1]); TubeV1[2] = P[count].Z + alpha * (P[count].N[2] + P[count].B[2]);
				TubeV2[0] = P[count].X + alpha * (P[count].N[0] - P[count].B[0]); TubeV2[1] = P[count].Y + alpha * (P[count].N[1] - P[count].B[1]); TubeV2[2] = P[count].Z + alpha * (P[count].N[2] - P[count].B[2]);
				TubeV3[0] = P[count].X + alpha * (-P[count].N[0] - P[count].B[0]); TubeV3[1] = P[count].Y + alpha * (-P[count].N[1] - P[count].B[1]); TubeV3[2] = P[count].Z + alpha * (-P[count].N[2] - P[count].B[2]);
				glLineWidth(10.0f);
				glBegin(GL_LINES);
				glColor3f(1.0, 0.0, 0.0);
				glVertex3f(TubeV0[0], TubeV0[1], TubeV0[2]);
				glColor3f(0.0, 1.0, 0.0);
				glVertex3f(TubeV1[0], TubeV1[1], TubeV1[2]);
				glColor3f(0.0, 1.0, 0.0);
				glVertex3f(TubeV1[0], TubeV1[1], TubeV1[2]);
				glColor3f(0.0, 0.0, 1.0);
				glVertex3f(TubeV2[0], TubeV2[1], TubeV2[2]);
				glColor3f(0.0, 0.0, 1.0);
				glVertex3f(TubeV2[0], TubeV2[1], TubeV2[2]);
				glColor3f(1.0, 1.0, 0.0);
				glVertex3f(TubeV3[0], TubeV3[1], TubeV3[2]);
				glColor3f(1.0, 1.0, 0.0);
				glVertex3f(TubeV3[0], TubeV3[1], TubeV3[2]);
				glColor3f(1.0, 0.0, 0.0);
				glVertex3f(TubeV0[0], TubeV0[1], TubeV0[2]);
				glEnd();
				/*Compute camera velocity*/
				u += 0.01 * sqrt(2 * 98 * fabs(-41.4 - P[count].Z)) / Normalizefactor;
				count++;
			}
		}
	}
}



void movecamera() {
	P[index].N[0] = 0; P[index].N[1] = 0; P[index].N[2] = 1;
	gluLookAt(P[index].X, P[index].Y, P[index].Z, P[index].X + P[index].T[0], P[index].Y + P[index].T[1], P[index].Z + P[index].T[2], P[index].N[0], P[index].N[1], P[index].N[2]);
	index++;
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear buffer
	glLoadIdentity();
	glScalef(g_vLandScale[0], g_vLandScale[1], g_vLandScale[2]);
	glTranslatef(g_vLandTranslate[0], g_vLandTranslate[1], g_vLandTranslate[2]);
	glRotatef(g_vLandRotate[0], 1, 0, 0);
	glRotatef(g_vLandRotate[1], 0, 1, 0);
	glRotatef(g_vLandRotate[2], 0, 0, 1);
	glLineWidth(3.0f);
	if (index < 522) {
		movecamera();
		Sleep(66.67);
	}
	drawsplines();
	// no modulation of texture color with lighting; use texture color directly
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	// turn on texture mapping (this disables standard OpenGL lighting, unless in GL_MODULATE mode)
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texName[0]); // make texture ¡§texName¡¨ the currently active texture
	/*Ground Texture*/
	glBegin(GL_QUADS); // draw a textured quad
    glTexCoord2f(0.0, 0.0); glVertex3f(-150.0, 150.0, -50.0);//3
	glTexCoord2f(0.0, 1.0); glVertex3f(-150.0, -150.0, -50.0);//0
	glTexCoord2f(1.0, 0.0); glVertex3f(150.0, -150.0, -50.0);//1
	glTexCoord2f(1.0, 1.0); glVertex3f(150.0, 150.0, -50.0);//2
	glEnd();
	/*Sky Texture using a sphere*/
	glBindTexture(GL_TEXTURE_2D, texName[1]);
	GLUquadric* qobj = gluNewQuadric();
	gluQuadricTexture(qobj, GL_TRUE);
	glTranslatef(0, 0, -35);
	gluSphere(qobj, 300, 100, 100);
	gluDeleteQuadric(qobj);
	glBindTexture(GL_TEXTURE_2D, texName[2]);
	// turn off texture mapping
	glDisable(GL_TEXTURE_2D);
	glutSwapBuffers();
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
	/*restart the roller coaster*/
	if (key == 'e' || key == 'E') {
		index = 0;
	}
	else
		glutIdleFunc(doIdle);
}

/* OpenCV help:
Access number of rows of image (height): image.rows; 
Access number of columns of image (width): image.cols;
Pixel 0,0 is the upper left corner. Byte order for 3-channel images is BGR. 
*/

int _tmain(int argc, _TCHAR* argv[])
{
	// I've set the argv[1] to track.txt.
	// To change it, on the "Solution Explorer",
	// right click "assign1", choose "Properties",
	// go to "Configuration Properties", click "Debugging",
	// then type your track file name for the "Command Arguments"
	if (argc<2)
	{  
		printf ("usage: %s <trackfile>\n", argv[0]);
		exit(0);
	}

	loadSplines(argv[1]);
	//initTexture2();
	glutInit(&argc, (char**)argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(1920, 1080);
	glutInitWindowPosition(0, 0);
	glutCreateWindow(argv[0]);
	/* tells glut to use a particular display function to redraw */
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	/* replace with any animate code */
	glutIdleFunc(doIdle);
	/* callback for mouse drags */
	glutMotionFunc(mousedrag);
	/* callback for idle mouse movement */
	glutPassiveMotionFunc(mouseidle);
	/* callback for mouse button changes */
	glutMouseFunc(mousebutton);
	glutKeyboardFunc(keyboard);
	initTexture();
	init();
	glEnable(GL_DEPTH_TEST);
	glutMainLoop();
	
	return 0;
}




