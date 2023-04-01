#include <GL/glut.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <iostream>
#include <string>
#include <cstdlib>

#define MAX_TRIANGLES 8 // Numero massimo di triangoli
#define TRIANGLE_SIZE 60 // Dimensione massima triangoli
#define MAX_PARTICLES 50 //Numero massimo particelle
#define SQUARE_SIZE 30 // Dimensione del quadrato
#define WINDOW_SIZE 400 // Dimensioni della finestra
#define PI 3.14159265358979323846

using namespace std;

// Struct per definire i triangoli
typedef struct 
{
    float x;
    float y;
    float size;
    float xVelocity;
    float yVelocity;
} Triangle;

// Struct per definire le particelle
typedef struct 
{
    float x;
    float y;
    float xVelocity;
    float yVelocity;
} Particle;

// Definizione delle variabili globali
bool debug = false; // Attiva debug mode (mostra bordi rossi)
bool showHitbox = false; // Indica se mostrare la hitbox dei triangoli e del quadrato
bool gameOver = false; // Indica se il gioco è terminato
bool paused = false; // Indica se il gioco è in pausa

Triangle triangles[MAX_TRIANGLES]; // Array di triangoli
int numTriangles = 0; // Numero di triangoli attualmente in gioco
const float triangleXVelocity = 0.5; // Velocità orizzontale dei triangoli
const int minTriangleSize = 15; // Dimensione minima dei triangolo

float squareX = WINDOW_SIZE / 2; // Coordinata x del quadrato
float squareY = SQUARE_SIZE / 2; // Coordinata y del quadrato
const int platform_y = 20; // Coordinata y della piattaforma

int score = 0; // Punteggio del giocatore
float dropVelocity = 1; // Velocità di caduta base dei triangoli
const int recall_timer = 15; //Ogni quanto aggiornare lo stato
const int winScore = 500; // Punti per vincere

Particle particles[MAX_PARTICLES]; // Array di particelle

void drawSfondo()
{
    // Disegna cerchi concentrici sullo sfondo
    float radius = WINDOW_SIZE; // Imposta il raggio iniziale del cerchio più grande
    float modifier = 1.0; // Imposta il modificatore di colore iniziale
    float yOffset = 0; // Imposta l'offset y iniziale
    while (radius > 0) // Finché il raggio è maggiore di zero
    {
        glColor4f(0.5 * modifier, 0.33 * modifier, 0.16 * modifier, 1.0); // Imposta il colore del cerchio (HEX #7E5429 ; RGB(126, 84, 41))
        glBegin(GL_TRIANGLE_FAN); // Inizia a disegnare un cerchio
        glVertex2f(WINDOW_SIZE / 2, WINDOW_SIZE / 2 - yOffset); // Imposta il centro del cerchio al centro della finestra di gioco e all'offset y corrente
        for (int i = 0; i <= 360; i++)
        { // Per ogni grado da 0 a 360 aggiunge un vertice al cerchio
            glVertex2f(WINDOW_SIZE / 2 + radius * cos(i * PI / 180), WINDOW_SIZE / 2 - yOffset + radius * sin(i * PI / 180));
        }
        glEnd();
        radius -= 20; // Riduce il raggio del cerchio successivo
        modifier -= 0.035; // Riduce il modificatore di colore del cerchio successivo
        yOffset += 10; // Aumenta l'offset y del cerchio successivo

        if (debug)
        { // Se sono in debug mode
            glColor3f(1.0, 0.0, 0.0); // Imposta il colore del contorno a rosso
            glBegin(GL_LINE_LOOP); // Inizia a disegnare una linea chiusa
            for (int i = 0; i <= 360; i++)
            { // Per ogni grado da 0 a 360 aggiunge un vertice al contorno del cerchio
                glVertex2f(WINDOW_SIZE / 2 + radius * cos(i * PI / 180), WINDOW_SIZE / 2 - yOffset + radius * sin(i * PI / 180));
            }
            glEnd();
        }
    }
}

// Funzione per inizializzare i triangoli
void initTriangles() 
{
    srand(time(0)); // Inizializza il generatore di numeri casuali
    for (int i = 0; i < MAX_TRIANGLES; i++) 
    {
        triangles[i].x = rand() % (WINDOW_SIZE - TRIANGLE_SIZE) + TRIANGLE_SIZE / 2; // Imposta la coordinata x del triangolo in modo casuale
        triangles[i].y = WINDOW_SIZE + TRIANGLE_SIZE / 2; // Imposta la coordinata y del triangolo al di sopra della finestra di gioco
        triangles[i].size = rand() % TRIANGLE_SIZE + minTriangleSize; // Imposta la dimensione del triangolo in modo casuale
        triangles[i].xVelocity = rand() % 2 == 0 ? triangleXVelocity : -1 * triangleXVelocity; // Imposta la velocità orizzontale del triangolo
        triangles[i].yVelocity = rand() % 5;
    }
}

// Funzione per disegnare un triangolo sullo schermo
void drawTriangle(Triangle t) 
{
    glBegin(GL_TRIANGLES); // Inizia a disegnare un triangolo

    // Imposta il colore del primo vertice a marrone chiaro (HEX #A18d6f ; RGB(161,141,111)
    glColor4f(0.63, 0.55, 0.44, 1.0);
    glVertex2f(t.x, t.y - t.size / 2); // Imposta il primo vertice del triangolo in basso al centro

    // Marrone più scuro HEX #836357; RGB(131,99,87)
    glColor4f(0.51, 0.39, 0.34, 1.0);
    glVertex2f(t.x - t.size / 3, t.y + t.size / 2); // Imposta il secondo vertice del triangolo in alto a sinistra

    // Marrone più scuro HEX #634B47 RGB(99,75,71)
    glColor4f(0.39, 0.29, 0.28, 1.0);
    glVertex2f(t.x + t.size / 3, t.y + t.size / 2); // Imposta il terzo vertice del triangolo in alto a destra

    glEnd();

    if (showHitbox) 
    { // Se la hitbox è abilitata
        glColor3f(1.0, 1.0, 1.0); // Imposta il colore della hitbox a bianco
        glBegin(GL_LINES); // Inizia a disegnare delle linee
        glVertex2f(t.x - 5, t.y); // Disegna una linea orizzontale al centro del triangolo
        glVertex2f(t.x + 5, t.y); 
        glVertex2f(t.x, t.y + 5); // Disegna una linea verticale al centro del triangolo
        glVertex2f(t.x, t.y - 5); 
        glEnd();
    }

    if (debug)
    { // Se sono in debug mode
        glColor3f(1.0, 0.0, 0.0); // Imposta il colore del contorno a rosso
        glBegin(GL_LINE_LOOP); // Inizia a disegnare una linea chiusa
        glVertex2f(t.x, t.y - t.size / 2); // Imposta il primo vertice del contorno in basso al centro
        glVertex2f(t.x - t.size / 3, t.y + t.size / 2); // Imposta il secondo vertice del contorno in alto a sinistra
        glVertex2f(t.x + t.size / 3, t.y + t.size / 2); // Imposta il terzo vertice del contorno in alto a destra
        glEnd();
    }
}

Triangle restoreTriangle(Triangle t)
{
    t.x = rand() % (WINDOW_SIZE - TRIANGLE_SIZE) + TRIANGLE_SIZE / 2; // Imposta la coordinata x del triangolo in modo casuale
    t.y = WINDOW_SIZE + TRIANGLE_SIZE / 2; // Imposta la coordinata y del triangolo al di sopra della finestra di gioco
    t.size = rand() % TRIANGLE_SIZE + minTriangleSize; // Imposta la dimensione del triangolo in modo casuale
    t.xVelocity = rand() % 2 == 0 ? triangleXVelocity : -1 * triangleXVelocity; // Imposta la velocità orizzontale del triangolo
    t.yVelocity = rand() % 5;
    return t;
}

// Funzione per disegnare il quadrato e la piattaforma sullo schermo
void drawSquare()
{
    // Disegna la piattaforma
    glColor3f(0.0, 0.0, 1.0); // Imposta il colore della piattaforma a blu
    glBegin(GL_LINES); // Inizia a disegnare una linea
    glVertex2f(0, platform_y); // Imposta il primo punto della linea al bordo sinistro della finestra di gioco e alla coordinata y della piattaforma
    glVertex2f(WINDOW_SIZE, platform_y); // Imposta il secondo punto della linea al bordo destro della finestra di gioco e alla coordinata y della piattaforma
    glEnd(); // Termina di disegnare la linea

    // Disegna il quadrato
    glColor3f(0.96, 0.73, 0.04); // Imposta il colore del quadrato a giallo (HEX #f6ba0a ; RGB(246,186,10))
    glBegin(GL_POLYGON); // Inizia a disegnare un poligono
    glVertex2f(squareX - SQUARE_SIZE / 2, squareY - SQUARE_SIZE / 2 + 10); // Imposta il primo vertice del quadrato in basso a sinistra
    glVertex2f(squareX + SQUARE_SIZE / 2, squareY - SQUARE_SIZE / 2 + 10); // Imposta il secondo vertice del quadrato in basso a destra
    glVertex2f(squareX + SQUARE_SIZE / 2, squareY + SQUARE_SIZE / 2 + 10); // Imposta il terzo vertice del quadrato in alto a destra
    glVertex2f(squareX - SQUARE_SIZE / 2, squareY + SQUARE_SIZE / 2 + 10); // Imposta il quarto vertice del quadrato in alto a sinistra
    glEnd();

    if (showHitbox)
    { // Se la hitbox è abilitata
        glColor3f(1.0, 1.0, 1.0); // Imposta il colore della hitbox a bianco
        glBegin(GL_LINES); // Inizia a disegnare delle linee
        glVertex2f(squareX - 5, squareY + 10); // Disegna una linea orizzontale al centro del quadrato
        glVertex2f(squareX + 5, squareY + 10);
        glVertex2f(squareX, squareY + 5 + 10); // Disegna una linea verticale al centro del quadrato
        glVertex2f(squareX, squareY - 5 + 10);
        glEnd();
    }

    if (debug)
    { // Se sono in debug mode
        glColor3f(1.0, 0.0, 0.0); // Imposta il colore del contorno a rosso
        glBegin(GL_LINE_LOOP); // Inizia a disegnare una linea chiusa
        glVertex2f(squareX - SQUARE_SIZE / 2, squareY - SQUARE_SIZE / 2 + 10); // Imposta il primo vertice del contorno in basso a sinistra
        glVertex2f(squareX + SQUARE_SIZE / 2, squareY - SQUARE_SIZE / 2 + 10); // Imposta il secondo vertice del contorno in basso a destra
        glVertex2f(squareX + SQUARE_SIZE / 2, squareY + SQUARE_SIZE / 2 + 10); // Imposta il terzo vertice del contorno in alto a destra
        glVertex2f(squareX - SQUARE_SIZE / 2, squareY + SQUARE_SIZE / 2 + 10); // Imposta il quarto vertice del contorno in alto a sinistra
        glEnd();
    }
}

void initParticles()
{
    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        particles[i].x = rand() % WINDOW_SIZE; // Imposta la coordinata x della particella in modo casuale
        particles[i].y = WINDOW_SIZE - 1; // Imposta la coordinata y della particella all'inizio della finestra di gioco
        particles[i].xVelocity = 0.1; // Imposta la velocità orizzontale della particella
        particles[i].yVelocity = rand() % 5 + 3; // Imposta la velocità verticale della particella in modo casuale
    }
}

void drawParticles()
{
    // Marrone scuro HEX #634B47 RGB(99,75,71)
    glColor4f(0.39, 0.29, 0.28, 1.0);
    glPointSize(5.0); // Imposta la dimensione delle particelle

    glBegin(GL_POINTS); // Inizia a disegnare le particelle

    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        glVertex2f(particles[i].x, particles[i].y); // Disegna la particella
    }

    glEnd();
}

void updateParticles()
{
    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        particles[i].x += particles[i].xVelocity; // Aggiorna la posizione orizzontale della particella
        particles[i].y -= particles[i].yVelocity; // Aggiorna la posizione verticale della particella

        if (particles[i].y < 1) // Se la particella è uscita dalla finestra di gioco
        {
            particles[i].x = rand() % WINDOW_SIZE; // Imposta la coordinata x della particella in modo casuale
            particles[i].y = WINDOW_SIZE - 1; // Imposta la coordinata y della particella all'inizio della finestra di gioco
            particles[i].xVelocity = 0.1; // Imposta la velocità orizzontale della particella
            particles[i].yVelocity = rand() % 5 + 3; // Imposta la velocità verticale della particella in modo casuale
        }
    }
}

// Funzione per disegnare il punteggio sullo schermo
// https://stackoverflow.com/questions/2183270/what-is-the-easiest-way-to-print-text-to-screen-in-opengl
void drawScore()
{
    string scoreText = "Punteggio: " + to_string(score); // Crea il testo del punteggio
    glColor3f(1.0, 1.0, 1.0); // Imposta il colore del testo a bianco
    glRasterPos2f(10, WINDOW_SIZE - 20); // Imposta la posizione del testo in alto a sinistra
    for (char c : scoreText)
    { // Per ogni carattere nel testo del punteggio
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c); // Disegna il carattere sullo schermo
    }
}

// Funzione per disegnare il contenuto della finestra di gioco
void display() 
{
    drawSfondo();

    if (!gameOver) 
    { // Se il gioco non è terminato
        drawScore(); // Disegna il punteggio sullo schermo
        drawSquare(); // Disegna il quadrato e la piattaforma sullo schermo
        drawParticles();
        for (int i = 0; i < numTriangles; i++) 
        { // Per ogni triangolo in gioco
            drawTriangle(triangles[i]); // Disegna il triangolo sullo schermo
        }
    }

    glutSwapBuffers(); // Scambia i buffer per visualizzare il contenuto disegnato sullo schermo
}

// Update della scena
void update_scene(int value) 
{
    if (!paused)
    {
        if (!gameOver) 
        { // Se il gioco non è terminato
            for (int i = 0; i < numTriangles; i++)
            { // Per ogni triangolo in gioco
                triangles[i].y -= (value + triangles[i].yVelocity); // Aggiorna la coordinata y del triangolo
                triangles[i].x += triangles[i].xVelocity; // Aggiorna la coordinata x del triangolo

                if (triangles[i].x - triangles[i].size / 3 < 0 || triangles[i].x + triangles[i].size / 3 > WINDOW_SIZE)
                { // Se il triangolo tocca il bordo destro o sinistro della finestra di gioco
                    triangles[i].xVelocity = -triangles[i].xVelocity; // Inverti la velocità orizzontale del triangolo
                }

                if (triangles[i].y < -TRIANGLE_SIZE / 2)
                { // Se il triangolo esce dal bordo inferiore della finestra di gioco
                    score += 5; // Incrementa il punteggio del giocatore
                    triangles[i] = restoreTriangle(triangles[i]);
                }

                // Se il triangolo collide con il quadrato
                if (triangles[i].x > squareX - SQUARE_SIZE / 2 && triangles[i].x < squareX + SQUARE_SIZE / 2 && triangles[i].y > squareY - SQUARE_SIZE / 2 && triangles[i].y < squareY + SQUARE_SIZE / 2)
                {
                    gameOver = true; // Termina il gioco
                }
            }
            dropVelocity += 0.001; // Incrementa il valore di dropVelocity (i triangoli scendono più velocemente)
            if (score >= winScore)
            {
                cout << "HAI VINTO!"<< endl<<"Punteggio finale : " << score << endl; // Stampa il punteggio finale sul terminale
                exit(0); // Termina l'esecuzione del programma
            }
        }
        else // Se il gioco è terminato
        {
            cout << "Punteggio finale: " << score << endl; // Stampa il punteggio finale sul terminale
            exit(0); // Termina l'esecuzione del programma
        }
        updateParticles(); // Aggiorna lo stato delle particelle
        glutPostRedisplay();
    }
        glutTimerFunc(recall_timer, update_scene, dropVelocity);
}

void keyboard(unsigned char key, int x, int y) 
{
    // Trasla il quadrato
    switch (key) 
    {
    case 'a':
        squareX -= SQUARE_SIZE;
        if (squareX < SQUARE_SIZE / 2) 
        {
            squareX = SQUARE_SIZE / 2;
        }
        break;
    case 'A':
        squareX -= SQUARE_SIZE;
        if (squareX < SQUARE_SIZE / 2) 
        {
            squareX = SQUARE_SIZE / 2;
        }
        break;
    case 'd':
        squareX += SQUARE_SIZE;
        if (squareX > WINDOW_SIZE - SQUARE_SIZE / 2) 
        {
            squareX = WINDOW_SIZE - SQUARE_SIZE / 2;
        }
        break;
    case 'D':
        squareX += SQUARE_SIZE;
        if (squareX > WINDOW_SIZE - SQUARE_SIZE / 2) 
        {
            squareX = WINDOW_SIZE - SQUARE_SIZE / 2;
        }
        break;
    // Mostra o nasconde le hitbox
    case 'b':
        showHitbox = !showHitbox;
        break;
    case 'B':
        showHitbox = !showHitbox;
        break;

    // Attiva visualizzazione debug
    case 's':
        debug = !debug;
        break;
    case 'S':
        debug = !debug;       
        break;

    // Attiva visualizzazione debug
    case 'p':
        paused = !paused;
        break;
    case 'P':
        paused = !paused;
        break;

    // Esce dal gioco
    case 27:
        cout << "Punteggio finale: " << score << endl; // Stampa il punteggio finale sul terminale
        exit(0);
        break;

    default:
        break;
    }
}

// Funzione principale del programma
int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); // Imposta display mode
    glutInitWindowSize(WINDOW_SIZE, WINDOW_SIZE); // Imposta la dimensione della finestra di gioco
    glutCreateWindow("Triangle rain"); // Crea la finestra di gioco
    
    //https://learn.microsoft.com/en-us/windows/win32/opengl/gluortho2d
    gluOrtho2D(0, WINDOW_SIZE, 0, WINDOW_SIZE); // Imposta il sistema di coordinate della finestra di gioco

    squareY = platform_y + SQUARE_SIZE/2 - 10; // Imposta la coordinata y iniziale del quadrato al di sopra della piattaforma
    numTriangles = MAX_TRIANGLES;
    initTriangles(); // Inizializza i triangoli
    initParticles(); // Inizializza le particelle

    glutDisplayFunc(display); 
    glutKeyboardFunc(keyboard);
    glutTimerFunc(recall_timer, update_scene, 1); // Chiama update_scene(1)

    glutMainLoop();

    return 0;
}