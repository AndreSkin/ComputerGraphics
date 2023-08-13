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

unsigned int control_poly_VAO;
unsigned int control_poly_VBO;

unsigned int bezier_VAO;
unsigned int bezier_VBO;
unsigned int catmull_VAO;
unsigned int catmull_VBO;
unsigned int catmull_internal_VAO;
unsigned int catmull_internal_VBO;

using namespace glm;

#define MaxNumPts 300
float PointArray[MaxNumPts][2];
float catmull_rom_points[MaxNumPts][2];
float bezier_curve[MaxNumPts][2];
float catmull_curve[MaxNumPts * 10][2];

bool trascinamento = false;
float tolleranza_trascinamento = 0.1;

bool show_catmull_points = false;
bool show_segments = false;

int NumPts = 0;
int catmull_pointsToDraw = 50;

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


void decasteljau(float t, float* bezier_result)
{
	float array_bezier[MaxNumPts][MaxNumPts][2];
	//Copio pointarray
	for (int i = 0; i < NumPts; i++) {
		array_bezier[0][i][0] = PointArray[i][0];
		array_bezier[0][i][1] = PointArray[i][1];
	}

	//Applico l'algoritmo
	for (int i = 1; i < NumPts; i++) {
		for (int j = 0; j < NumPts - i; j++) {
			array_bezier[i][j][0] = ((1 - t) * array_bezier[i - 1][j][0]) + (t * array_bezier[i - 1][j + 1][0]);
			array_bezier[i][j][1] = ((1 - t) * array_bezier[i - 1][j][1]) + (t * array_bezier[i - 1][j + 1][1]);
		}
		
	}
	//Salvo i risultati
	bezier_result[0] = array_bezier[NumPts - 1][0][0];
	bezier_result[1] = array_bezier[NumPts - 1][0][1];
}

void catmull_rom(float t, float* catmull_result, int catmull_point_index)
{
	// Crea un array per memorizzare i punti intermedi
	float catmull_curve_points[MaxNumPts][MaxNumPts][2];

	// Imposta il primo punto di controllo (P_i)
	catmull_curve_points[0][0][0] = PointArray[catmull_point_index][0];
	catmull_curve_points[0][0][1] = PointArray[catmull_point_index][1];	
	
	// Imposta il quarto punto di controllo (P_{i + 1})
	catmull_curve_points[0][3][0] = PointArray[catmull_point_index + 1][0];
	catmull_curve_points[0][3][1] = PointArray[catmull_point_index + 1][1];

	// Calcola il secondo punto di controllo (P+)
	//Stima della derivata
	float m_x = (PointArray[catmull_point_index + 1][0] - PointArray[catmull_point_index - 1][0]) / 2;
	float m_y = (PointArray[catmull_point_index + 1][1] - PointArray[catmull_point_index - 1][1]) / 2;
	catmull_curve_points[0][1][0] = PointArray[catmull_point_index][0] + (m_x / 3);
	catmull_curve_points[0][1][1] = PointArray[catmull_point_index][1] + (m_y / 3);

	// Calcola il terzo punto di controllo (P-)
	m_x = (PointArray[catmull_point_index + 1][0] - PointArray[catmull_point_index][0]) / 2;
	m_y = (PointArray[catmull_point_index + 1][1] - PointArray[catmull_point_index][1]) / 2;
	if (catmull_point_index != NumPts - 1) //Non l'ultimo punto
	{
		m_x = (PointArray[catmull_point_index + 2][0] - PointArray[catmull_point_index][0]) / 2;
		m_y = (PointArray[catmull_point_index + 2][1] - PointArray[catmull_point_index][1]) / 2;
	}
	catmull_curve_points[0][2][0] = PointArray[catmull_point_index + 1][0] - (m_x / 3);
	catmull_curve_points[0][2][1] = PointArray[catmull_point_index + 1][1] - (m_y / 3);

	// Applica l'algoritmo di De Casteljau ai quattro punti di controllo per calcolare il punto sulla curva di Catmull-Rom
	for (int i = 1; i < 4; i++) {
		for (int j = 0; j < (4 - i); j++) {
			catmull_curve_points[i][j][0] = ((1 - t) * catmull_curve_points[i - 1][j][0]) + (t * catmull_curve_points[i - 1][j + 1][0]);
			catmull_curve_points[i][j][1] = ((1 - t) * catmull_curve_points[i - 1][j][1]) + (t * catmull_curve_points[i - 1][j + 1][1]);
		}
	}

	// Salva i risultati
	catmull_rom_points[catmull_point_index * 2][0] = catmull_curve_points[0][1][0];
	catmull_rom_points[catmull_point_index * 2][1] = catmull_curve_points[0][1][1];
	catmull_rom_points[(catmull_point_index * 2) + 1][0] = catmull_curve_points[0][2][0];
	catmull_rom_points[(catmull_point_index * 2) + 1][1] = catmull_curve_points[0][2][1];
	catmull_result[0] = catmull_curve_points[3][0][0];
	catmull_result[1] = catmull_curve_points[3][0][1];
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
		//Normalizza coordinate
		float xPos = -1.0f + ((float)x) * 2 / ((float)(width));
		float yPos = -1.0f + ((float)(height - y)) * 2 / ((float)(height));

		for (int i = 0; i < NumPts; i++) 
		{
			//Se clicco abbastanza vicino al punto
			if ((abs(xPos - PointArray[i][0]) < tolleranza_trascinamento) && (abs(yPos - PointArray[i][1]) < tolleranza_trascinamento))
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
		show_catmull_points = !show_catmull_points;
		glutPostRedisplay();
		break;
	case 's':
		show_segments = !show_segments;
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
	glGenVertexArrays(1, &control_poly_VAO);
	glBindVertexArray(control_poly_VAO);
	glGenBuffers(1, &control_poly_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, control_poly_VBO);
	// VAO for bezier curve
	glGenVertexArrays(1, &bezier_VAO);
	glBindVertexArray(bezier_VAO);
	glGenBuffers(1, &bezier_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, bezier_VBO);

	// VAO for second curve catmull_rom
	glGenVertexArrays(1, &catmull_VAO);
	glBindVertexArray(catmull_VAO);
	glGenBuffers(1, &catmull_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, catmull_VBO);

	// punti del 5.A punti interni
	glGenVertexArrays(1, &catmull_internal_VAO);
	glBindVertexArray(catmull_internal_VAO);
	glGenBuffers(1, &catmull_internal_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, catmull_internal_VBO);

	// Background color
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glViewport(0, 0, 500, 500);
}

void drawScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	if (NumPts > 2) 
	{
		float bezier_result[2];
		for (int i = 0; i <= 100; i++) {
			decasteljau((GLfloat)i / 100, bezier_result);
			bezier_curve[i][0] = bezier_result[0];
			bezier_curve[i][1] = bezier_result[1];
		}


		glBindVertexArray(bezier_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, bezier_VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(bezier_curve), &bezier_curve[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glLineWidth(2.0);
		glDrawArrays(GL_LINE_STRIP, 0, 100 +1 );

		glBindVertexArray(0);
	}
	// punto 5.A
	if (NumPts > 2) {

		float catmull_result[2];
		int i = 0;

		for (int catmull_point_index = 0; catmull_point_index < NumPts - 1; catmull_point_index++) {
			for (i; i <= (catmull_pointsToDraw + (catmull_pointsToDraw * catmull_point_index)); i++) {
				catmull_rom((((GLfloat)i - (catmull_pointsToDraw * catmull_point_index)) / catmull_pointsToDraw), catmull_result, catmull_point_index);
				catmull_curve[i][0] = catmull_result[0];
				catmull_curve[i][1] = catmull_result[1];
			}
		}

		glBindVertexArray(catmull_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, catmull_VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(catmull_curve), &catmull_curve[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glLineWidth(1.0);
		glDrawArrays(GL_LINE_STRIP, 0, (catmull_pointsToDraw * (NumPts - 1)) +1);

		glBindVertexArray(0);
	}

	if ((NumPts > 3) && (show_catmull_points == true)) {

		glBindVertexArray(catmull_internal_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, catmull_internal_VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(catmull_rom_points), &catmull_rom_points[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glPointSize(10.0);
		glDrawArrays(GL_POINTS, 0, NumPts * 2 - 2);

		glBindVertexArray(0);
	}



	// Draw control polygon
	glBindVertexArray(control_poly_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, control_poly_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(PointArray), &PointArray[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// Draw the control points CP
	glPointSize(6.0);
	glDrawArrays(GL_POINTS, 0, NumPts);
	
	// Draw the line segments between CP
	if (show_segments)
	{
		glLineWidth(1.0);
		glDrawArrays(GL_LINE_STRIP, 0, NumPts);
	}


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

	//Dragging
	glutMotionFunc(DragPoint);
	glutPassiveMotionFunc(Point_is_dragged);

	glewExperimental = GL_TRUE;
	glewInit();

	initShader();
	init();

	glutMainLoop();
}