#include <glew.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

// Structure pour stocker les informations des cubes
typedef struct {
    float x, y, z;   // Position du cube
    float r, g, b;   // Couleur du cube
} Cube;

// Nombre de cubes
#define CUBE_COUNT 50000
Cube cubes[CUBE_COUNT];

// Fonction pour initialiser les cubes avec des positions et couleurs aléatoires
void initializeCubes() {
    srand((unsigned int)time(NULL));
    for (int i = 0; i < CUBE_COUNT; i++) {
        cubes[i].x = (rand() % 200 - 100) * 0.1f; // Position X entre -10 et 10
        cubes[i].y = (rand() % 200 - 100) * 0.1f; // Position Y entre -10 et 10
        cubes[i].z = (rand() % 200 - 100) * 0.1f; // Position Z entre -10 et 10
        cubes[i].r = (float)(rand() % 256) / 255.0f; // Couleur rouge
        cubes[i].g = (float)(rand() % 256) / 255.0f; // Couleur verte
        cubes[i].b = (float)(rand() % 256) / 255.0f; // Couleur bleue
    }
}

// Fonction pour dessiner un cube à une position donnée avec une couleur donnée
void drawCube(float x, float y, float z, float r, float g, float b) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glColor3f(r, g, b);

    glBegin(GL_QUADS);

    // Face avant
    glVertex3f(-0.5f, -0.5f,  0.5f);
    glVertex3f( 0.5f, -0.5f,  0.5f);
    glVertex3f( 0.5f,  0.5f,  0.5f);
    glVertex3f(-0.5f,  0.5f,  0.5f);

    // Face arrière
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f,  0.5f, -0.5f);
    glVertex3f( 0.5f,  0.5f, -0.5f);
    glVertex3f( 0.5f, -0.5f, -0.5f);

    // Face gauche
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, -0.5f,  0.5f);
    glVertex3f(-0.5f,  0.5f,  0.5f);
    glVertex3f(-0.5f,  0.5f, -0.5f);

    // Face droite
    glVertex3f( 0.5f, -0.5f, -0.5f);
    glVertex3f( 0.5f,  0.5f, -0.5f);
    glVertex3f( 0.5f,  0.5f,  0.5f);
    glVertex3f( 0.5f, -0.5f,  0.5f);

    // Face supérieure
    glVertex3f(-0.5f,  0.5f, -0.5f);
    glVertex3f(-0.5f,  0.5f,  0.5f);
    glVertex3f( 0.5f,  0.5f,  0.5f);
    glVertex3f( 0.5f,  0.5f, -0.5f);

    // Face inférieure
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f( 0.5f, -0.5f, -0.5f);
    glVertex3f( 0.5f, -0.5f,  0.5f);
    glVertex3f(-0.5f, -0.5f,  0.5f);

    glEnd();
    glPopMatrix();
}

int main() {
    // Initialisation de GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Échec de l'initialisation de GLFW\n");
        return -1;
    }

    // Création de la fenêtre GLFW
    GLFWwindow* window = glfwCreateWindow(1280, 720, "OpenGL - 50,000 Cubes Benchmark", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Échec de la création de la fenêtre GLFW\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glewInit();

    // Configuration OpenGL
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 1280.0 / 720.0, 0.1, 10000.0);
    glMatrixMode(GL_MODELVIEW);

    // Initialiser les cubes
    initializeCubes();

    // Boucle principale
    while (!glfwWindowShouldClose(window)) {
        // Effacer l'écran
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();

        // Définir la position de la caméra
        gluLookAt(30.0, 30.0, 30.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

        // Dessiner les cubes
        for (int i = 0; i < CUBE_COUNT; i++) {
            drawCube(cubes[i].x, cubes[i].y, cubes[i].z, cubes[i].r, cubes[i].g, cubes[i].b);
        }

        // Afficher les FPS
        char title[64];
        sprintf(title, "OpenGL - 50,000 Cubes Benchmark - FPS: %.2f", 1.0 / glfwGetTime());
        glfwSetWindowTitle(window, title);
        glfwSetTime(0.0);

        // Échanger les buffers
        glfwSwapBuffers(window);

        // Gestion des événements
        glfwPollEvents();
    }

    // Nettoyage
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
