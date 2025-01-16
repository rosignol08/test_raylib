#include "raylib.h"
#include <stdlib.h> // Pour rand()

#define CUBE_COUNT 50000

typedef struct {
    Vector3 position;
    Color color;
} Cube;

int main(void) {
    // Initialisation de la fenêtre et de la caméra
    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "Raylib - 50,000 Cubes Benchmark");
    
    Camera camera = { 0 };
    camera.position = (Vector3){ 100.0f, 100.0f, 100.0f }; // Position de la caméra
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };         // Où la caméra regarde
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };             // Direction "up" de la caméra
    camera.fovy = 45.0f;                                   // Champ de vision vertical
    camera.projection = CAMERA_PERSPECTIVE;               // Projection perspective

    // Génération des cubes
    Cube cubes[CUBE_COUNT];
    for (int i = 0; i < CUBE_COUNT; i++) {
        cubes[i].position = (Vector3){
            (float)(rand() % 200 - 100)*0.1f, // Position X entre -10 et 10
            (float)(rand() % 200 - 100)*0.1f, // Position Y entre -10 et 10
            (float)(rand() % 200 - 100)*0.1f  // Position Z entre -10 et 10
        };
        cubes[i].color = (Color){
            rand() % 256, // Rouge
            rand() % 256, // Vert
            rand() % 256, // Bleu
            255           // Opacité
        };
    }

    SetTargetFPS(6000); // Limite des FPS à 60
    // Boucle principale
    while (!WindowShouldClose()) {
        // Déplacement de la caméra
        UpdateCamera(&camera, CAMERA_PERSPECTIVE);
        EnableCursor(); // Activer le curseur pour la rotation de la caméra
        // Rendu
        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);

        // Affichage des cubes
        for (int i = 0; i < CUBE_COUNT; i++) {
            DrawCube(cubes[i].position, 1.0f, 1.0f, 1.0f, cubes[i].color);  // Taille 1x1x1
        }

        EndMode3D();

        DrawFPS(10, 10); // Affichage des FPS en haut à gauche

        EndDrawing();
    }

    // Libération des ressources
    CloseWindow();

    return 0;
}
