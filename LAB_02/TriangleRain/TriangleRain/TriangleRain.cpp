#include <GL/glut.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <iostream>
#include <string>

#define MAX_TRIANGLES 10 // Numero massimo di triangoli
#define TRIANGLE_SIZE 60 // Dimensione massima triangoli
#define SQUARE_SIZE 30 // Dimensione del quadrato
#define WINDOW_SIZE 400 // Dimensioni della finestra
#define PI 3.14159265358979323846

using namespace std;

// Struct per definire i triangoli
typedef struct {
    float x;
    float y;
    float size;
    float xVelocity;
} Triangle;

// Definizione delle variabili globali
bool debug = false; // Attiva debug mode (mostra bordi rossi)
bool showHitbox = false; // Indica se mostrare la hitbox dei triangoli e del quadrato
bool gameOver = false; // Indica se il gioco è terminato

Triangle triangles[MAX_TRIANGLES]; // Array di triangoli
int numTriangles = 0; // Numero di triangoli attualmente in gioco
const float triangleXVelocity = 0.5; // Velocità orizzontale dei triangoli
const int minTriangleSize = 15; // Dimensione minima dei triangolo

float squareX = WINDOW_SIZE / 2; // Coordinata x del quadrato
float squareY = SQUARE_SIZE / 2; // Coordinata y del quadrato
const int platform_y = 20; // Coordinata y della piattaforma

int score = 0; // Punteggio del giocatore
float yVelocity = 1; // Valore del timer
const int recall_timer = 15; //Ogni quanto aggiornare lo stato

// Funzione per inizializzare i triangoli
void initTriangles() 
{
    srand(time(0)); // Inizializza il generatore di numeri casuali
    for (int i = 0; i < MAX_TRIANGLES; i++) 
    {
        triangles[i].x = rand() % (WINDOW_SIZE - TRIANGLE_SIZE) + TRIANGLE_SIZE / 2; // Imposta la coordinata x del triangolo in modo casuale
        triangles[i].y = WINDOW_SIZE + TRIANGLE_SIZE / 2; // Imposta la coordinata y del triangolo al di sopra della finestra di gioco
        triangles[i].size = rand() % TRIANGLE_SIZE + minTriangleSize; // Imposta la dimensione del triangolo in modo casuale
        triangles[i].xVelocity = rand()%2 == 0? triangleXVelocity: -1*triangleXVelocity; // Imposta la velocità orizzontale del triangolo
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

// Funzione per disegnare un triangolo sullo schermo
void drawTriangle(Triangle t) 
{
    glColor4f(0.73, 0.62, 0.5, 1.0); // Imposta il colore del triangolo a marrone chiaro (HEX #BC9E82 ; RGB(188,158,130))
    glBegin(GL_TRIANGLES); // Inizia a disegnare un triangolo
    glVertex2f(t.x, t.y - t.size / 2); // Imposta il primo vertice del triangolo in basso al centro
    glVertex2f(t.x - t.size / 2, t.y + t.size / 2); // Imposta il secondo vertice del triangolo in alto a sinistra
    glVertex2f(t.x + t.size / 2, t.y + t.size / 2); // Imposta il terzo vertice del triangolo in alto a destra
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
        glVertex2f(t.x - t.size / 2, t.y + t.size / 2); // Imposta il secondo vertice del contorno in alto a sinistra
        glVertex2f(t.x + t.size / 2, t.y + t.size / 2); // Imposta il terzo vertice del contorno in alto a destra
        glEnd();
    }
}

// Funzione per disegnare il contenuto della finestra di gioco
void display() 
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
        radius -= 10; // Riduce il raggio del cerchio successivo
        modifier -= 0.025; // Riduce il modificatore di colore del cerchio successivo
        yOffset += 5; // Aumenta l'offset y del cerchio successivo

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

    if (!gameOver) 
    { // Se il gioco non è terminato
        drawScore(); // Disegna il punteggio sullo schermo
        drawSquare(); // Disegna il quadrato e la piattaforma sullo schermo
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
    if (!gameOver) { // Se il gioco non è terminato
        for (int i = 0; i < numTriangles; i++) 
        { // Per ogni triangolo in gioco
            triangles[i].y -= value; // Aggiorna la coordinata y del triangolo
            triangles[i].x += triangles[i].xVelocity; // Aggiorna la coordinata x del triangolo

            if (triangles[i].x < TRIANGLE_SIZE / 2 || triangles[i].x > WINDOW_SIZE - TRIANGLE_SIZE / 2) 
            { // Se il triangolo tocca il bordo destro o sinistro della finestra di gioco
                triangles[i].xVelocity = -triangles[i].xVelocity; // Inverti la velocità orizzontale del triangolo
            }

            if (triangles[i].y < -TRIANGLE_SIZE / 2) 
            { // Se il triangolo esce dal bordo inferiore della finestra di gioco
                score += 5; // Incrementa il punteggio del giocatore
                triangles[i].x = rand() % (WINDOW_SIZE - TRIANGLE_SIZE) + TRIANGLE_SIZE / 2; // Imposta la coordinata x del triangolo in modo casuale
                triangles[i].y = WINDOW_SIZE + TRIANGLE_SIZE / 2; // Imposta la coordinata y del triangolo al di sopra della finestra di gioco
                triangles[i].size = rand() % TRIANGLE_SIZE + minTriangleSize; // Imposta la dimensione del triangolo in modo casuale
                triangles[i].xVelocity = rand() % 2 == 0 ? triangleXVelocity : -1 * triangleXVelocity; // Imposta la velocità orizzontale del triangolo
            }
            // Se il triangolo collide con il quadrato
            if (triangles[i].x > squareX - SQUARE_SIZE / 2 && triangles[i].x < squareX + SQUARE_SIZE / 2 && triangles[i].y > squareY - SQUARE_SIZE / 2 && triangles[i].y < squareY + SQUARE_SIZE / 2)
            { 
                gameOver = true; // Termina il gioco
            }
        }
        yVelocity += 0.001; // Incrementa il valore di yVelocity (i triangoli scendono più velocemente)
    }
    else // Se il gioco è terminato
    {
        cout << "Punteggio finale: " << score << endl; // Stampa il punteggio finale sul terminale
        exit(0); // Termina l'esecuzione del programma
    }

    glutPostRedisplay(); // Richiede un aggiornamento della finestra di gioco
    glutTimerFunc(recall_timer, update_scene, yVelocity); // Richiama la funzione timer dopo un intervallo di tempo specificato
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

    glutDisplayFunc(display); 
    glutKeyboardFunc(keyboard);
    glutTimerFunc(recall_timer, update_scene, 1); // Chiama update_scene(1)

    glutMainLoop();

    return 0;
}