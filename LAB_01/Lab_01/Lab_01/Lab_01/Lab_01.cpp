/*
 * Lab-01_students.c
 *
 *     This program draws straight lines connecting dots placed with mouse clicks.
 *
 * Usage:
 *   Left click to place a control point.
 *		Maximum number of control points allowed is currently set at 64.
 *	 Press "f" to remove the first control point
 *	 Press "l" to remove the last control point.
 *	 Press escape to exit.
 */


#include <iostream>
#include "ShaderMaker.h"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <vector>

#include <math.h>
#include <string>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

static unsigned int programId;

unsigned int VAO;
unsigned int VBO;

unsigned int VAO_2;
unsigned int VBO_2;
unsigned int VAO_3;
unsigned int VBO_3;
unsigned int VAO_4;
unsigned int VBO_4;

using namespace glm;

#define MaxNumPts 300
float PointArray[MaxNumPts][2];
float catmull_rom_points[MaxNumPts][2];
float CurveArray[MaxNumPts][2];
float interp_curve[MaxNumPts * 10][2];

bool trascinamento = false;
float tolleranza_trascinamento = 0.1;

bool show_points = false;

int NumPts = 0;

// Window size in pixels
int		width = 500;
int		height = 500;

/* Prototypes */
void addNewPoint(float x, float y);
int main(int argc, char** argv);
void removeFirstPoint();
void removeLastPoint();
void decasteljau(float t, float* result);
void Point_is_dragged(int x, int y);
void DragPoint(int x, int y);
void catmull_rom(float t, float* result_2, int y);


void decasteljau(float t, float* result)
{
	float array_bezier[MaxNumPts][MaxNumPts][2];
	//Copio pointarray
	for (int i = 0; i < NumPts; i++) {
		array_bezier[0][i][0] = PointArray[i][0];
		array_bezier[0][i][1] = PointArray[i][1];
	}

	//Applico l'algoritmo
	for (int i = 1; i < NumPts; i++) {
		for (int j = 0; j < NumPts -i; j++) {
			array_bezier[i][j][0] = ((1 - t) * array_bezier[i - 1][j][0]) + (t * array_bezier[i - 1][j + 1][0]);
			array_bezier[i][j][1] = ((1 - t) * array_bezier[i - 1][j][1]) + (t * array_bezier[i - 1][j + 1][1]);
		}
		
	}
	//Salvo i risultati
	result[0] = array_bezier[NumPts - 1][0][0];
	result[1] = array_bezier[NumPts - 1][0][1];
}

void catmull_rom(float t, float* result_2, int y) 
{
	float CurveArray_catmull_rom[MaxNumPts][MaxNumPts][2];
	CurveArray_catmull_rom[0][0][0] = PointArray[y][0]; //punto 1
	CurveArray_catmull_rom[0][0][1] = PointArray[y][1];

	float m_x = (PointArray[y + 1][0] - PointArray[y - 1][0]) / 2;
	float m_y = (PointArray[y + 1][1] - PointArray[y - 1][1]) / 2;
	CurveArray_catmull_rom[0][1][0] = PointArray[y][0] + (m_x / 3); //punto 2
	CurveArray_catmull_rom[0][1][1] = PointArray[y][1] + (m_y / 3);
	//sopra i primi due punti, sotto il 3 e 4

	CurveArray_catmull_rom[0][3][0] = PointArray[y + 1][0]; // punto 4
	CurveArray_catmull_rom[0][3][1] = PointArray[y + 1][1];

	m_x = (PointArray[y + 1][0] - PointArray[y][0]) / 2; // inizio punto 3
	m_y = (PointArray[y + 1][1] - PointArray[y][1]) / 2;
	if (y != NumPts - 1) {
		m_x = (PointArray[y + 2][0] - PointArray[y][0]) / 2;
		m_y = (PointArray[y + 2][1] - PointArray[y][1]) / 2;
	}

	CurveArray_catmull_rom[0][2][0] = PointArray[y + 1][0] - (m_x / 3); // punto 3
	CurveArray_catmull_rom[0][2][1] = PointArray[y + 1][1] - (m_y / 3);

	for (int i = 1; i < 4; i++) { //bezier applicato ai 4 punti di controllo
		for (int j = 0; j < (4 - i); j++) {
			CurveArray_catmull_rom[i][j][0] = ((1 - t) * CurveArray_catmull_rom[i - 1][j][0]) + (t * CurveArray_catmull_rom[i - 1][j + 1][0]);
			CurveArray_catmull_rom[i][j][1] = ((1 - t) * CurveArray_catmull_rom[i - 1][j][1]) + (t * CurveArray_catmull_rom[i - 1][j + 1][1]);
		}

	}


	catmull_rom_points[y * 2][0] = CurveArray_catmull_rom[0][1][0];
	catmull_rom_points[y * 2][1] = CurveArray_catmull_rom[0][1][1];
	catmull_rom_points[(y * 2) + 1][0] = CurveArray_catmull_rom[0][2][0];
	catmull_rom_points[(y * 2) + 1][1] = CurveArray_catmull_rom[0][2][1];
	result_2[0] = CurveArray_catmull_rom[3][0][0];
	result_2[1] = CurveArray_catmull_rom[3][0][1];
}


void Point_is_dragged(int x, int y) {

	if (glutGetModifiers() == GLUT_LEFT_BUTTON) 
	{
		trascinamento = true;
	}
	else 
	{
		trascinamento = false;
	}
}

void DragPoint(int x, int y) 
{
	if (trascinamento) 
	{
		float xPos = -1.0f + ((float)x) * 2 / ((float)(width));
		float yPos = -1.0f + ((float)(height - y)) * 2 / ((float)(height));

		for (int i = 0; i < NumPts; i++) 
		{
			//Se clicco abbastanza vicino al punto
			if ((xPos - PointArray[i][0] < tolleranza_trascinamento && xPos - PointArray[i][0] >  -tolleranza_trascinamento) &&
				(yPos - PointArray[i][1] < tolleranza_trascinamento && yPos - PointArray[i][1] >  -tolleranza_trascinamento))
			{
				//Sposta punto
				PointArray[i][0] = xPos;
				PointArray[i][1] = yPos;
				glutPostRedisplay();
				return;
			}
		}
	}
}


void myKeyboardFunc(unsigned char key, int x, int y)
{
	switch (key) {
	case 'f':
		removeFirstPoint();
		glutPostRedisplay();
		break;
	case 'l':
		removeLastPoint();
		glutPostRedisplay();
		break;
	case 'v':
		show_points = !show_points;
		glutPostRedisplay();
		break;
	case 27:			// Escape key
		exit(0);
		break;
	}
}
void removeFirstPoint() 
{
	int i;
	if (NumPts > 0)
	{
		// Remove the first point, slide the rest down
		NumPts--;
		for (i = 0; i < NumPts; i++) 
		{
			PointArray[i][0] = PointArray[i + 1][0];
			PointArray[i][1] = PointArray[i + 1][1];
		}
	}
}

void resizeWindow(int w, int h)
{
	height = (h > 1) ? h : 2;
	width = (w > 1) ? w : 2;
	gluOrtho2D(-1.0f, 1.0f, -1.0f, 1.0f);
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
}

// Left button presses place a new control point.
void myMouseFunc(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		// (x,y) viewport(0,width)x(0,height)   -->   (xPos,yPos) window(-1,1)x(-1,1)
		float xPos = -1.0f + ((float)x) * 2 / ((float)(width));
		float yPos = -1.0f + ((float)(height - y)) * 2 / ((float)(height));

		bool punto_spostato = false;
		//Sposto il punto
		for (int i = 0; i < NumPts; i++) 
		{
			if ((abs(xPos - PointArray[i][0]) < tolleranza_trascinamento) && (abs(yPos - PointArray[i][1]) < tolleranza_trascinamento))
			{
				punto_spostato = true;
			}
		}
		if (punto_spostato == false)
		{
			addNewPoint(xPos, yPos);
		}
		glutPostRedisplay();

	}
}

// Add a new point to the end of the list.  
// Remove the first point in the list if too many points.
void removeLastPoint() 
{
	if (NumPts > 0) 
	{
		NumPts--;
	}
}

// Add a new point to the end of the list.  
// Remove the first point in the list if too many points.
void addNewPoint(float x, float y) 
{
	if (NumPts >= MaxNumPts) 
	{
		removeFirstPoint();
	}
		PointArray[NumPts][0] = x;
		PointArray[NumPts][1] = y;
		NumPts++;
}
void initShader(void)
{
	GLenum ErrorCheckValue = glGetError();

	char* vertexShader = (char*)"vertexShader.glsl";
	char* fragmentShader = (char*)"fragmentShader.glsl";

	programId = ShaderMaker::createProgram(vertexShader, fragmentShader);
	glUseProgram(programId);
}


void init(void)
{
	// VAO for control polygon
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// VAO for bezier curve
	glGenVertexArrays(1, &VAO_2);
	glBindVertexArray(VAO_2);
	glGenBuffers(1, &VBO_2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_2);

	// VAO for second curve catmull_rom
	glGenVertexArrays(1, &VAO_3);
	glBindVertexArray(VAO_3);
	glGenBuffers(1, &VBO_3);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_3);

	// punti del 5.A punti interni
	glGenVertexArrays(1, &VAO_4);
	glBindVertexArray(VAO_4);
	glGenBuffers(1, &VBO_4);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_4);

	// Background color
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glViewport(0, 0, 500, 500);
}

void drawScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	if (NumPts > 1) 
	{
		float result[2];
		for (int i = 0; i <= 100; i++) {
			decasteljau((GLfloat)i / 100, result);
			CurveArray[i][0] = result[0];
			CurveArray[i][1] = result[1];
		}


		glBindVertexArray(VAO_2);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_2);
		glBufferData(GL_ARRAY_BUFFER, sizeof(CurveArray), &CurveArray[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glLineWidth(1.0);
		glDrawArrays(GL_LINE_STRIP, 0, 100);

		glBindVertexArray(0);
	}
	// punto 5.A
	if (NumPts > 2) {

		float result_2[2];
		int i = 0;

		for (int y = 0; y < NumPts - 1; y++) {
			for (i; i <= (50 + (50 * y)); i++) {
				catmull_rom((((GLfloat)i - (50 * y)) / 50), result_2, y);
				interp_curve[i][0] = result_2[0];
				interp_curve[i][1] = result_2[1];
			}
		}

		glBindVertexArray(VAO_3);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_3);
		glBufferData(GL_ARRAY_BUFFER, sizeof(interp_curve), &interp_curve[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glLineWidth(1.0);
		glDrawArrays(GL_LINE_STRIP, 0, 50 * (NumPts - 1));

		glBindVertexArray(0);
	}

	if ((NumPts > 3) && (show_points == true)) {

		glBindVertexArray(VAO_4);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_4);
		glBufferData(GL_ARRAY_BUFFER, sizeof(catmull_rom_points), &catmull_rom_points[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glPointSize(10.0);
		glDrawArrays(GL_POINTS, 0, NumPts * 2 - 2);

		glBindVertexArray(0);
	}



	// Draw control polygon
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(PointArray), &PointArray[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// Draw the control points CP
	glPointSize(6.0);
	glDrawArrays(GL_POINTS, 0, NumPts);
	// Draw the line segments between CP
	glLineWidth(2.0);
	glDrawArrays(GL_LINE_STRIP, 0, NumPts);

	glBindVertexArray(0);

	glutSwapBuffers();
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

	glutInitWindowSize(width, height);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Draw curves 2D");

	glutDisplayFunc(drawScene);
	glutReshapeFunc(resizeWindow);
	glutKeyboardFunc(myKeyboardFunc);
	glutMouseFunc(myMouseFunc);

	//aggiunti per il dragging, il primo serve a muovere, il secondo serve a verificare e settare un bool a true
	glutMotionFunc(DragPoint);
	glutPassiveMotionFunc(Point_is_dragged);

	glewExperimental = GL_TRUE;
	glewInit();

	initShader();
	init();

	glutMainLoop();
}