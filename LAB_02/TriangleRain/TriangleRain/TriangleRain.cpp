#include <GL/glew.h>
#include <GL/glut.h>
#include <iostream>
#include <vector>
#include <random>

const int MAX_TRIANGOLI = 8;
const float QUADRATO_SPEED = 0.05f;
const float TRIANGOLO_SIZE = 0.2f; // Dimensione massima dei triangoli
const float TRIANGOLO_MIN_SIZE = 0.05f; // Misura minima dei triangoli
const float TRIANGOLO_MAX_H_VELOCITY = 0.05f;

int punteggio = 0;
bool game_over = false;

struct Triangolo {
    float x, y;
    float velocity;
    float width, height;
    float direction;

    Triangolo(float _x, float _y, float _velocity, float _width, float _height, float _direction)
        : x(_x), y(_y), velocity(_velocity), width(_width), height(_height), direction(_direction) {}
};

std::vector<Triangolo> triangoli;

float quadratoX = 0.0f;
float quadratoY = -0.9f;
const float quadratoWidth = 0.1f;
const float quadratoHeight = 0.1f;

GLuint quadratoVao, triangoloVao;
GLuint quadratoVbo, triangoloVbo;

void generaTriangolo() {
    float x = (static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f;
    float velocity = (static_cast<float>(rand()) / RAND_MAX) * 0.02f + 0.01f;
    float width = (static_cast<float>(rand()) / RAND_MAX) * TRIANGOLO_SIZE;
    float direction = (static_cast<float>(rand()) / RAND_MAX) * 0.01;

    // Assicurati che la larghezza del triangolo sia maggiore o uguale alla misura minima
    width = std::max(width, TRIANGOLO_MIN_SIZE);
    direction = std::min(direction, TRIANGOLO_MAX_H_VELOCITY);

    float height = width * 2.0f; // Triangolo isoscele con la punta verso il basso
    float y = 1.0f + height; // Coordinate y al di sopra dello schermo
    triangoli.push_back(Triangolo(x, y, velocity, width, height, direction));
}

void collisionDetection() {
    for (auto it = triangoli.begin(); it != triangoli.end();) {
        if (it->y <= quadratoY + quadratoHeight && it->y >= quadratoY &&
            it->x <= quadratoX + quadratoWidth && it->x >= quadratoX) {
            game_over = true;
            break;
        }
        else if (it->y < -1.0f) {
            it->y = 1.0f + it->height; // Riporta il triangolo sopra lo schermo
            punteggio += 10;;
            continue;
        }
        ++it;
    }
}

void update(int value) {
    if (!game_over) {
        for (auto& triangolo : triangoli) {
            triangolo.y -= triangolo.velocity;
            triangolo.x += triangolo.direction;
            if (triangolo.x >= 1.0f || triangolo.x <= -1.0f) {
                triangolo.direction = -triangolo.direction;
            }
        }

        collisionDetection();
        // Genera nuovi triangoli se il numero corrente è inferiore al massimo
        if (triangoli.size() < MAX_TRIANGOLI) {
            generaTriangolo();
        }

    }


    if (punteggio >= 1000) {
        game_over = true;
    }

    // Aggiorna la posizione del quadrato
    if (quadratoX < -1.0f)
        quadratoX = -1.0f;
    else if (quadratoX > 1.0f - quadratoWidth)
        quadratoX = 1.0f - quadratoWidth;

    // Ridisegna il quadrato
    glBindVertexArray(quadratoVao);
    glBindBuffer(GL_ARRAY_BUFFER, quadratoVbo);

    GLfloat quadratoVertices[] = {
        quadratoX, quadratoY,
        quadratoX + quadratoWidth, quadratoY,
        quadratoX + quadratoWidth, quadratoY + quadratoHeight,
        quadratoX, quadratoY + quadratoHeight
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(quadratoVertices), quadratoVertices, GL_STATIC_DRAW);

    // Richiede il ridisegno della scena
    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

void initializeVaoVbo() {
    // Inizializza il VAO e il VBO per il quadrato
    glGenVertexArrays(1, &quadratoVao);
    glBindVertexArray(quadratoVao);

    glGenBuffers(1, &quadratoVbo);
    glBindBuffer(GL_ARRAY_BUFFER, quadratoVbo);

    GLfloat quadratoVertices[] = {
        quadratoX, quadratoY,
        quadratoX + quadratoWidth, quadratoY,
        quadratoX + quadratoWidth, quadratoY + quadratoHeight,
        quadratoX, quadratoY + quadratoHeight
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(quadratoVertices), quadratoVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    // Inizializza il VAO e il VBO per il triangolo
    glGenVertexArrays(1, &triangoloVao);
    glBindVertexArray(triangoloVao);

    glGenBuffers(1, &triangoloVbo);
    glBindBuffer(GL_ARRAY_BUFFER, triangoloVbo);

    GLfloat triangoloVertices[] = {
        -0.5f, 0.0f,
        0.5f, 0.0f,
        0.0f, -1.0f
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(triangoloVertices), triangoloVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    glBindVertexArray(quadratoVao);
    glColor3f(1.0f, 1.0f, 0.0f);
    glDrawArrays(GL_QUADS, 0, 4);

    glBindVertexArray(triangoloVao);
    glColor3f(0.6f, 0.3f, 0.0f);
    for (const auto& triangolo : triangoli) {
        glLoadIdentity();
        glTranslatef(triangolo.x, triangolo.y, 0.0f);
        glScalef(triangolo.width, triangolo.height, 1.0f);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    std::cout << "Punteggio: " << punteggio << std::endl;

    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'a':
    case 'A':
        quadratoX -= QUADRATO_SPEED;
        break;
    case 'd':
    case 'D':
        quadratoX += QUADRATO_SPEED;
        break;
    }
}


void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(400, 400);
    glutCreateWindow("TriangleRain");

    glewInit();
    initializeVaoVbo();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(0, update, 0);

    srand(static_cast<unsigned int>(time(0)));
    for (int i = 0; i < MAX_TRIANGOLI; ++i) {
        generaTriangolo();
    }

    glutMainLoop();
    return 0;
}