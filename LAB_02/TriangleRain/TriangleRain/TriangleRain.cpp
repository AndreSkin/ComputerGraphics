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
#define M_PI 3.1415

//Dimensioni della finestra
const int GAME_SIZE = 400;
const float WINDOW_SIZE = 2.0f;

const int MAX_NUM_CIRCLES = 25; //Numero massimo di cerchi che compongono lo sfondo
int num_circles = 5; //Numero iniziale (e attuale) di cerchi
float circleRadius = WINDOW_SIZE * 1.5; //Raggio del primo cerchio

const int MAX_TRIANGOLI = 8; //Numero massimo di triangoli
const float TRIANGOLO_MAX_SIZE = 0.25f; //Dimensione massima dei triangoli
const float TRIANGOLO_MIN_SIZE = 0.05f; //Misura minima dei triangoli
const float TRIANGOLO_MAX_H_VELOCITY = 0.02f; //Massima velocità orizzontale dei triangoli

const float QUADRATO_SPEED = 0.08f; //Velocità del giocatore
//Posizione iniziale del giocatore
float quadratoX = 0.0f; 
float quadratoY = -0.98f;
//Dimensioni del giocatore
const float quadratoWidth = 0.15f;
const float quadratoHeight = 0.15f;

const int winscore = 1000; //Punteggio necessario per vincere

static unsigned int programId;

bool paused = false; //Gioco in pausa
bool game_over = false; //Gioco terminato

int schivati = 0; //Triangoli schivati
int punteggio = 0; //Punteggio attuale

//Definizione di triangolo
struct Triangolo
{
    float x, y; //Coordinate
    float Yvelocity; //Velocità verticale    
    float Xvelocity; //Velocità orizzontale
    float width, height; //Dimensioni

    //Costruttore
    Triangolo(float _x, float _y, float _velocity, float _width, float _height, float _direction)
        : x(_x), y(_y), Yvelocity(_velocity), width(_width), height(_height), Xvelocity(_direction) {}
};

//Vettore di triangoli
std::vector<Triangolo> triangoli;

//Vettore dei vertici dei triangoli
//3 vertici * (3 coordinate + 4 rgba)
GLfloat triangoloVertices[MAX_TRIANGOLI * 3 * (3 + 4)];

//Vettore dei vertici dei cerchi
std::vector<float> circleVertices;

//VAO e VBO
GLuint quadratoVao, triangoloVao;
GLuint quadratoVbo, triangoloVbo;
GLuint circleVao[MAX_NUM_CIRCLES], circleVbo[MAX_NUM_CIRCLES];

//Inizializza VAO e VBO per i cerchi di sfondo
void initCircles(int numero_circles)
{

    float circleOffset = circleRadius / numero_circles; //Grandezza cerchi
    float yOffset = 1.0f; //Sposta i cerchi di 1 unità verso il basso

    for (int i = numero_circles - 1; i >= 0; --i) {
        float radius = circleRadius - i * circleOffset; //Raggio decrescente
        int numVertices = static_cast<int>(2 * M_PI * radius * 20); //Numero di vertici
        float angleIncrement = 2 * M_PI / numVertices;

        for (int j = 0; j < numVertices; ++j) {
            float angle = j * angleIncrement;
            float x = radius * cos(angle);
            float y = radius * sin(angle) - yOffset;
            float color = 1.0f - (static_cast<float>(i) / numero_circles); //Colore più scuro
            circleVertices.push_back(x);
            circleVertices.push_back(y);
            circleVertices.push_back(0.0f);
            float r = 0.6f * color;
            float g = 0.3f * color;
            float b = 0.0f;
            circleVertices.push_back(r);
            circleVertices.push_back(g);
            circleVertices.push_back(b);
            circleVertices.push_back(1.0f);
        }
        //Un VAO e un VBO per cerchio
        glGenVertexArrays(1, &circleVao[i]);
        glBindVertexArray(circleVao[i]);

        glGenBuffers(1, &circleVbo[i]);
        glBindBuffer(GL_ARRAY_BUFFER, circleVbo[i]);

        //Inserisco xyz e rgba di un cerchio
        glBufferData(GL_ARRAY_BUFFER, circleVertices.size() * sizeof(float), circleVertices.data(), GL_STATIC_DRAW);

        //Configura l'attributo posizione
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        //Configura il colore
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        //Svuota il vettore circleVertices per il prossimo ciclo
        circleVertices.clear();
    }
}

//Genera un triangolo
Triangolo generaTriangolo()
{
    //Parametri casuali
    float x = (static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f;
    float Yvelocity = (static_cast<float>(rand()) / RAND_MAX) * 0.02f + 0.01f;
    float width = (static_cast<float>(rand()) / RAND_MAX) * TRIANGOLO_MAX_SIZE;
    float Xvelocity = (static_cast<float>(rand()) / RAND_MAX) * 0.01;

    //Larghezza del triangolo maggiore o uguale alla misura minima
    width = std::max(width, TRIANGOLO_MIN_SIZE);
    //Velocità orizzontale entro il massimo
    Xvelocity = std::min(Xvelocity, TRIANGOLO_MAX_H_VELOCITY);

    //Direzione positiva o negativa con il 50% di probabilità
    if (rand() % 2 == 0) {
        Xvelocity = -Xvelocity;
    }

    float height = width * 2.0f;
    float y = 1.0f + height; //Coordinate y al di sopra dello schermo

    return Triangolo(x, y, Yvelocity, width, height, Xvelocity);
}

//Inizializza VAO e VBO per i triangoli
void initTriangoli(int num_triangoli) 
{
    for (int i = 0; i < num_triangoli; i++)
    {
        //Crea un triangolo e lo aggiunge al vettore
        Triangolo T = generaTriangolo();
        triangoli.push_back(T);

        //Aggiungo i dati del triangolo all'array di vertici
        int index = (triangoli.size() - 1) * 3 * (3 + 4);
        //Primo vertice
        triangoloVertices[index + X] = T.x - T.width / 2;
        triangoloVertices[index + Y] = T.y;
        triangoloVertices[index + Z] = 0.0f;
        //Imposta il colore del primo vertice a marrone chiaro (HEX #A18d6f ; RGB(161,141,111)
        triangoloVertices[index + R] = 0.63f;
        triangoloVertices[index + G] = 0.55f;
        triangoloVertices[index + B] = 0.44f;
        triangoloVertices[index + A] = 1.0f;
        //Secondo vertice
        index += (3 + 4);
        triangoloVertices[index + X] = T.x + T.width / 2;
        triangoloVertices[index + Y] = T.y;
        triangoloVertices[index + Z] = 0.0f;
        //Marrone più scuro HEX #836357; RGB(131,99,87)
        triangoloVertices[index + R] = 0.51f;
        triangoloVertices[index + G] = 0.39f;
        triangoloVertices[index + B] = 0.44f;
        triangoloVertices[index + A] = 1.0f;
        //Terzo vertice
        index += (3 + 4);
        triangoloVertices[index + X] = T.x;
        triangoloVertices[index + Y] = T.y - T.height;
        triangoloVertices[index + Z] = 0.0f;
        //Marrone più scuro HEX #634B47 RGB(99,75,71)
        triangoloVertices[index + R] = 0.39f;
        triangoloVertices[index + G] = 0.29f;
        triangoloVertices[index + B] = 0.28f;
        triangoloVertices[index + A] = 1.0f;
    }
    glGenVertexArrays(1, &triangoloVao);
    glBindVertexArray(triangoloVao);

    glGenBuffers(1, &triangoloVbo);
    glBindBuffer(GL_ARRAY_BUFFER, triangoloVbo);

    //Tutti i triangoli
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangoloVertices), triangoloVertices, GL_DYNAMIC_DRAW);
    //Configura l'attributo posizione
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    //Configura l'attributo colore
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

//Controllo collisioni
void collisionDetection(int val)
{
    //Se non sono in pausa e sono in gioco
    if (!paused && !game_over)
    {
        //Itero tra i triangoli
        for (auto it = triangoli.begin(); it != triangoli.end();)
        {
            //Se uno tocca il quadrato finisce la partita
            if (it->y >= quadratoY - quadratoHeight && it->y <= quadratoY &&
                it->x <= quadratoX + quadratoWidth && it->x >= quadratoX)
            {
                std::cout << "\nHai perso, punteggio finale: " << punteggio;
                game_over = true;
                break;
            }
            //Se uno esce dal bordo inferiore
            else if (it->y < -1.0f)
            {
                //Aumento punteggio e schivati
                punteggio += 10;
                schivati += 1;

                //Dopo un po' il numero di cerchi aumenta per dare illusione di profondità
                if ((schivati % 10 == 0) && (punteggio != 0))
                {
                    num_circles = num_circles + 1 < MAX_NUM_CIRCLES ? num_circles + 1 : MAX_NUM_CIRCLES;
                    initCircles(num_circles);
                }

                std::cout << "\rPunteggio: " << punteggio;

                //Rimuove il triangolo che ha oltrepassato il bordo inferiore
                it = triangoli.erase(it);
                //Crea un nuovo triangolo casuale
                triangoli.push_back(generaTriangolo());

                break;
            }
            ++it;
        }
    }
    glutTimerFunc(16, collisionDetection, 0);
    
}

//Funzione di update
void update(int value) 
{
    //Se non sono in pausa e sono in gioco
    if (!paused && !game_over)
    {
        if (!game_over) 
        {
            //Itero tra i triangoli
            for (int i = 0; i < triangoli.size(); i++) 
            {
                auto& triangolo = triangoli[i];
                //Muovo il triangolo in basso e orizzontale in base ai suoi parametri
                triangolo.y -= triangolo.Yvelocity;
                triangolo.x += triangolo.Xvelocity;
                //Rimbalza se colpisce il muro
                if (triangolo.x >= 1.0f || triangolo.x <= -1.0f)
                {
                    triangolo.Xvelocity = -triangolo.Xvelocity;
                }

                //Aggiorna i dati di posizione del triangolo nell'array di vertici
                int index = i * 3 * (3 + 4); //3 vertici per (3 xyz + 4 rgba)
                //Primo vertice
                triangoloVertices[index + X] = triangolo.x - triangolo.width / 2;
                triangoloVertices[index + Y] = triangolo.y;
                //Secondo vertice
                index += (3 + 4);
                triangoloVertices[index + X] = triangolo.x + triangolo.width / 2;
                triangoloVertices[index + Y] = triangolo.y;
                //Terzo vertice
                index += (3 + 4);
                triangoloVertices[index + X] = triangolo.x;
                triangoloVertices[index + Y] = triangolo.y - triangolo.height;
            }
        }

        //Vittoria
        if (punteggio >= winscore)
        {
            game_over = true;
            std::cout << "\nHAI VINTO!" << std::endl << "Punteggio finale : " << punteggio;
            exit(0);
        }

        //Evita che il quadrato esca dall'area di gioco
        if (quadratoX < -1.0f)
        {
            quadratoX = -1.0f;
        }
        else if (quadratoX > 1.0f - quadratoWidth)
        {
            quadratoX = 1.0f - quadratoWidth;
        }

        //Reimposta i vertici del quadrato
        GLfloat quadratoVertices[] = {
            quadratoX, quadratoY, 0.0f,//vertice in basso a sinistra
            0.96f, 0.73f, 0.04f, 1.0f, //Giallo (HEX #f6ba0a ; RGB(246,186,10))

            quadratoX + quadratoWidth, quadratoY, 0.0f, //vertice in basso a destra 
            1.00f, 0.85f, 0.00f, 1.0f, //Giallo (HEX #FFD700  ; RGB(255, 215, 0))

            quadratoX + quadratoWidth, quadratoY + quadratoHeight, 0.0f,//vertice in alto a destra
            0.96f, 0.73f, 0.04f, 1.0f, //Giallo (HEX #f6ba0a ; RGB(246,186,10))

            quadratoX, quadratoY + quadratoHeight, 0.0f,//vertice in alto a sinistra
            1.00f, 0.85f, 0.00f, 1.0f //Giallo (HEX #FFD700  ; RGB(255, 215, 0))
        };

        glBindBuffer(GL_ARRAY_BUFFER, quadratoVbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quadratoVertices), quadratoVertices);

        glBindBuffer(GL_ARRAY_BUFFER, triangoloVbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(triangoloVertices), triangoloVertices);

    }
    //Richiede il ridisegno della scena
    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}


void initializeVaoVbo() 
{
    
    //Cerchi
    initCircles(num_circles);

    //Qudrato
    GLfloat quadratoVertices[] = {
        quadratoX, quadratoY, 0.0f,//vertice in basso a sinistra
        0.96f, 0.73f, 0.04f, 1.0f, //Giallo (HEX #f6ba0a ; RGB(246,186,10))

        quadratoX + quadratoWidth, quadratoY, 0.0f, //vertice in basso a destra 
        1.00f, 0.85f, 0.00f, 1.0f, //Giallo (HEX #FFD700  ; RGB(255, 215, 0))

        quadratoX + quadratoWidth, quadratoY + quadratoHeight, 0.0f,//vertice in alto a destra
        0.96f, 0.73f, 0.04f, 1.0f, //Giallo (HEX #f6ba0a ; RGB(246,186,10))

        quadratoX, quadratoY + quadratoHeight, 0.0f,//vertice in alto a sinistra
        1.00f, 0.85f, 0.00f, 1.0f //Giallo (HEX #FFD700  ; RGB(255, 215, 0))
    };

    //Inizializza il VAO e il VBO per il quadrato
    glGenVertexArrays(1, &quadratoVao);
    glBindVertexArray(quadratoVao);

    glGenBuffers(1, &quadratoVbo);
    glBindBuffer(GL_ARRAY_BUFFER, quadratoVbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(quadratoVertices), quadratoVertices, GL_STATIC_DRAW);
    //Configura l'attributo posizione
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    //Configura l'attributo colore
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //Triangoli
    initTriangoli(MAX_TRIANGOLI);
}



void display() 
{
    float circleOffset = circleRadius / num_circles;

    if (!paused && !game_over)
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glLoadIdentity();

        //Disegna i cerchi
        for (int i = 0; i < num_circles; ++i)
        {
            glBindVertexArray(circleVao[i]);

            //Calcola il numero di vertici per questo cerchio
            int numVertices = static_cast<int>(2 * M_PI * (circleRadius - i * circleOffset) * 20);

            //Disegna il cerchio
            glDrawArrays(GL_TRIANGLE_FAN, 0, numVertices);
        }

        //Disegna il giocatore
        glBindVertexArray(quadratoVao);
        glDrawArrays(GL_QUADS, 0, 4);

        //Disegna i triangoli
        glBindVertexArray(triangoloVao);
        glDrawArrays(GL_TRIANGLES, 0, MAX_TRIANGOLI * 3);

        glutSwapBuffers();
    }
}


void keyboard(unsigned char key, int x, int y) 
{
    switch (key)
    {
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

        //Esce dal gioco
    case 27:
        cout << "\nPunteggio finale: " << punteggio;
        std::exit(0);
        break;

    default:
        break;
    }
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
    glutInitWindowSize(GAME_SIZE, GAME_SIZE);

    //Posiziona la finestra
    int screenWidth = glutGet(GLUT_SCREEN_WIDTH);
    int screenHeight = glutGet(GLUT_SCREEN_HEIGHT);
    glutInitWindowPosition((screenWidth - GAME_SIZE*2) / 2, (screenHeight - GAME_SIZE*2) / 2);

    glutCreateWindow("TriangleRain");

    glewInit();
    initShader();
    initializeVaoVbo();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(0, update, 0);
    glutTimerFunc(0, collisionDetection, 0);


    //Inizializza random
    srand(static_cast<unsigned int>(time(0)));


    glutMainLoop();
    return 0;
}
