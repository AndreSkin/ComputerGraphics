#include"ShaderMaker.h"
#include <iostream>
#include <vector>
#include <random>

#define X 0
#define Y 1
#define Z 2
#define R 3
#define G 4
#define B 5
#define A 6


const int MAX_TRIANGOLI = 8;
const float QUADRATO_SPEED = 0.05f;
const float TRIANGOLO_SIZE = 0.27f; // Dimensione massima dei triangoli
const float TRIANGOLO_MIN_SIZE = 0.05f; // Misura minima dei triangoli
const float TRIANGOLO_MAX_H_VELOCITY = 0.05f;
const int winscore = 1000;

static unsigned int programId;
bool paused = false;

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

GLfloat triangoloVertices[MAX_TRIANGOLI * 3 * (3 + 4)];


GLuint quadratoVao, triangoloVao;
GLuint quadratoVbo, triangoloVbo;

Triangolo generaTriangolo()
{
    float x = (static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f;
    float velocity = (static_cast<float>(rand()) / RAND_MAX) * 0.02f + 0.01f;
    float width = (static_cast<float>(rand()) / RAND_MAX) * TRIANGOLO_SIZE;
    float direction = (static_cast<float>(rand()) / RAND_MAX) * 0.01;

    // Assicurati che la larghezza del triangolo sia maggiore o uguale alla misura minima
    width = std::max(width, TRIANGOLO_MIN_SIZE);
    direction = std::min(direction, TRIANGOLO_MAX_H_VELOCITY);

    float height = width * 2.0f; // Triangolo isoscele con la punta verso il basso
    float y = 1.0f + height; // Coordinate y al di sopra dello schermo

    return Triangolo(x, y, velocity, width, height, direction);
}


void generaTriangoli() {

    Triangolo T = generaTriangolo();
    triangoli.push_back(T);

    // Aggiungi i dati del triangolo all'array di vertici
    int index = (triangoli.size() - 1) * 3 * (3 + 4);
    // Primo vertice
    triangoloVertices[index + X] = T.x - T.width / 2;
    triangoloVertices[index + Y] = T.y;
    triangoloVertices[index + Z] = 0.0f;
    // Imposta il colore del primo vertice a marrone chiaro (HEX #A18d6f ; RGB(161,141,111)
    triangoloVertices[index + R] = 0.63f;
    triangoloVertices[index + G] = 0.55f;
    triangoloVertices[index + B] = 0.44f;
    triangoloVertices[index + A] = 1.0f;
    // Secondo vertice
    index += (3 + 4);
    triangoloVertices[index + X] = T.x + T.width / 2;
    triangoloVertices[index + Y] = T.y;
    triangoloVertices[index + Z] = 0.0f;
    // Marrone più scuro HEX #836357; RGB(131,99,87)
    triangoloVertices[index + R] = 0.51f;
    triangoloVertices[index + G] = 0.39f;
    triangoloVertices[index + B] = 0.44f;
    triangoloVertices[index + A] = 1.0f;
    // Terzo vertice
    index += (3 + 4);
    triangoloVertices[index + X] = T.x;
    triangoloVertices[index + Y] = T.y - T.height;
    triangoloVertices[index + Z] = 0.0f;
    // Marrone più scuro HEX #634B47 RGB(99,75,71)
    triangoloVertices[index + R] = 0.39f;
    triangoloVertices[index + G] = 0.29f;
    triangoloVertices[index + B] = 0.28f;
    triangoloVertices[index + A] = 1.0f;
}


void collisionDetection() {
    for (auto it = triangoli.begin(); it != triangoli.end();) {
        if (it->y >= quadratoY - quadratoHeight && it->y <= quadratoY &&
            it->x <= quadratoX + quadratoWidth && it->x >= quadratoX) {
            game_over = true;
            break;
        }
        else if (it->y < -1.0f) {
            punteggio += 10;
            // Rimuovi il triangolo che ha oltrepassato il bordo inferiore
            it = triangoli.erase(it);

            triangoli.push_back(generaTriangolo());

            // Esci dal ciclo dopo aver generato il nuovo triangolo
            break;
        }
        ++it;
    }
}

void update(int value) {
    if (!paused && !game_over)
    {
        if (!game_over) {
            for (int i = 0; i < triangoli.size(); i++) {
                auto& triangolo = triangoli[i];
                triangolo.y -= triangolo.velocity;
                triangolo.x += triangolo.direction;
                if (triangolo.x >= 1.0f || triangolo.x <= -1.0f) {
                    triangolo.direction = -triangolo.direction;
                }

                // Aggiorna i dati di posizione del triangolo nell'array di vertici
                int index = i * 3 * (3 + 4);
                // Primo vertice
                triangoloVertices[index + X] = triangolo.x - triangolo.width / 2;
                triangoloVertices[index + Y] = triangolo.y;
                // Secondo vertice
                index += (3 + 4);
                triangoloVertices[index + X] = triangolo.x + triangolo.width / 2;
                triangoloVertices[index + Y] = triangolo.y;
                // Terzo vertice
                index += (3 + 4);
                triangoloVertices[index + X] = triangolo.x;
                triangoloVertices[index + Y] = triangolo.y - triangolo.height;
            }

            collisionDetection();
        }

        if (punteggio >= winscore) {
            game_over = true;
            std::cout << "HAI VINTO!" << std::endl << "Punteggio finale : " << punteggio << std::endl;
            exit(0);
        }

        // Aggiorna la posizione del quadrato
        if (quadratoX < -1.0f)
            quadratoX = -1.0f;
        else if (quadratoX > 1.0f - quadratoWidth)
            quadratoX = 1.0f - quadratoWidth;

        GLfloat quadratoVertices[] = {
            quadratoX, quadratoY, 0.0f,// vertice in basso a sinistra
            0.96f, 0.73f, 0.04f, 1.0f, // Giallo (HEX #f6ba0a ; RGB(246,186,10))

            quadratoX + quadratoWidth, quadratoY, 0.0f, // vertice in basso a destra 
            1.00f, 0.85f, 0.00f, 1.0f, // Giallo (HEX #FFD700  ; RGB(255, 215, 0))

            quadratoX + quadratoWidth, quadratoY + quadratoHeight, 0.0f,// vertice in alto a destra
            0.96f, 0.73f, 0.04f, 1.0f, // Giallo (HEX #f6ba0a ; RGB(246,186,10))

            quadratoX, quadratoY + quadratoHeight, 0.0f,// vertice in alto a sinistra
            1.00f, 0.85f, 0.00f, 1.0f // Giallo (HEX #FFD700  ; RGB(255, 215, 0))
        };

        glBindBuffer(GL_ARRAY_BUFFER, quadratoVbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quadratoVertices), quadratoVertices);

        glBindBuffer(GL_ARRAY_BUFFER, triangoloVbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(triangoloVertices), triangoloVertices);

    }
    // Richiede il ridisegno della scena
    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}


void initializeVaoVbo() {

    GLfloat quadratoVertices[] = {
        quadratoX, quadratoY, 0.0f,// vertice in basso a sinistra
        0.96f, 0.73f, 0.04f, 1.0f, // Giallo (HEX #f6ba0a ; RGB(246,186,10))

        quadratoX + quadratoWidth, quadratoY, 0.0f, // vertice in basso a destra 
        1.00f, 0.85f, 0.00f, 1.0f, // Giallo (HEX #FFD700  ; RGB(255, 215, 0))

        quadratoX + quadratoWidth, quadratoY + quadratoHeight, 0.0f,// vertice in alto a destra
        0.96f, 0.73f, 0.04f, 1.0f, // Giallo (HEX #f6ba0a ; RGB(246,186,10))

        quadratoX, quadratoY + quadratoHeight, 0.0f,// vertice in alto a sinistra
        1.00f, 0.85f, 0.00f, 1.0f // Giallo (HEX #FFD700  ; RGB(255, 215, 0))
    };

    // Inizializza il VAO e il VBO per il quadrato
    glGenVertexArrays(1, &quadratoVao);
    glBindVertexArray(quadratoVao);

    glGenBuffers(1, &quadratoVbo);
    glBindBuffer(GL_ARRAY_BUFFER, quadratoVbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(quadratoVertices), quadratoVertices, GL_STATIC_DRAW);
    // Configura l'attributo posizione
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Configura l'attributo colore
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

   
    // Inizializza il VAO e il VBO per i triangoli
    glGenVertexArrays(1, &triangoloVao);
    glBindVertexArray(triangoloVao);

    glGenBuffers(1, &triangoloVbo);
    glBindBuffer(GL_ARRAY_BUFFER, triangoloVbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(triangoloVertices), triangoloVertices, GL_DYNAMIC_DRAW);
    // Configura l'attributo posizione
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Configura l'attributo colore
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //set background color
    //glClearColor(1.0, 0.5, 0.0, 1.0);
}



void display() {

    if (!paused && !game_over)
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glLoadIdentity();

        glBindVertexArray(quadratoVao);
        glDrawArrays(GL_QUADS, 0, 4);

        glBindVertexArray(triangoloVao);
        glDrawArrays(GL_TRIANGLES, 0, MAX_TRIANGOLI * 3);

        std::cout << "Punteggio: " << punteggio << std::endl;


        glutSwapBuffers();
    }
}


void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'a':
    case 'A':
        if (!paused)
        {
            quadratoX -= QUADRATO_SPEED;
        }

        break;
    case 'd':
    case 'D':
        if (!paused)
        {
            quadratoX += QUADRATO_SPEED;
        }       
        break;

    case 'p':
        paused = !paused;
        break;
    case 'P':
        paused = !paused;
        break;

        // Esce dal gioco
    case 27:
        cout << "Punteggio finale: " << punteggio << endl;
        exit(0);
        break;

    default:
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

void initShader(void)
{
    GLenum ErrorCheckValue = glGetError();

    char* vertexShader = (char*)"vertexShader.glsl";
    char* fragmentShader = (char*)"fragmentShader.glsl";

    programId = ShaderMaker::createProgram(vertexShader, fragmentShader);
    glUseProgram(programId);

}

int main(int argc, char** argv) {
    glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(400, 400);
    glutCreateWindow("TriangleRain");

    glewInit();
    initShader();
    initializeVaoVbo();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(0, update, 0);

    srand(static_cast<unsigned int>(time(0)));
    for (int i = 0; i < MAX_TRIANGOLI; ++i) {
        generaTriangoli();
    }

    glutMainLoop();
    return 0;
}
