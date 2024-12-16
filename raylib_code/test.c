/*******************************************************************************************
*
*   raylib [core] example - Basic window
*
*   Welcome to raylib!
*
*   To test examples, just press F6 and execute 'raylib_compile_execute' script
*   Note that compiled executable is placed in the same folder as .c file
*
*   To test the examples on Web, press F6 and execute 'raylib_compile_execute_web' script
*   Web version of the program is generated in the same folder as .c file
*
*   You can find all basic examples on C:\raylib\raylib\examples folder or
*   raylib official webpage: www.raylib.com
*
*   Enjoy using raylib. :)
*
*   Example originally created with raylib 1.0, last time updated with raylib 1.0
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2013-2024 Ramon Santamaria (@raysan5)
*
********************************************************************************************/
#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>

#define GRID_SIZE 5

// Structure pour stocker les informations d'un objet 3D dans la grille
typedef struct {
    Vector3 position;   // Position de l'objet
    Model model;        // Modèle 3D de l'objet
    bool active;        // Indique si l'objet est actif ou non
} GridCell;


float variation_hauteur(GridCell cellule) {
    float time = GetTime();
    return cos(cellule.position.x + time) + cos(cellule.position.z + time);
}

float random_flottant(float min, float max) {
    return min + (rand() / (float)RAND_MAX) * (max - min);
}

int main(void) {
    // Initialisation
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib - Grille avec objets 3D");

    // Caméra pour visualiser la scène
    Camera camera = { 0 };
    camera.position = (Vector3){ 10.0f, 10.0f, 10.0f };
    camera.target = (Vector3){ 0.0f, 8.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 10.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Charger le modèle et la texture
    Model model = LoadModel("pine_tree/scene.gltf");
    Texture2D texture = LoadTexture("textures/Leavs_baseColor.png");
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
    // Initialisation de la grille
    GridCell grid[GRID_SIZE][GRID_SIZE];
    for (int x = 0; x < GRID_SIZE; x++) {
        for (int z = 0; z < GRID_SIZE; z++) {
            grid[x][z].position = (Vector3){ x * 2.0f, 0.0f, z * 2.0f };
            grid[x][z].model = model;
            float taille = random_flottant(0.05f,0.01f);
            grid[x][z].model.transform = MatrixScale(taille,taille,taille);  // Réduire la taille du modèle
            grid[x][z].active = true;  // Activer tous les objets par défaut
        }
    }

    DisableCursor();                    // Limit cursor to relative movement inside the window

    SetTargetFPS(850);

    // Boucle principale
    while (!WindowShouldClose()) {
        // Mise à jour de la caméra
        UpdateCamera(&camera, CAMERA_FREE);

        // Dessiner
        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);

        DrawGrid(10, 1.0f);  // Grille visuelle

        // Dessiner les objets de la grille
        for (int x = 0; x < GRID_SIZE; x++) {
            for (int z = 0; z < GRID_SIZE; z++) {
                if (grid[x][z].active) {
                    DrawModel(grid[x][z].model, grid[x][z].position, 0.5f, WHITE);
                }
                variation_hauteur(grid[x][z]);
            }
        }

        EndMode3D();

        DrawText("Grille d'objets 3D - Utilisez la souris pour naviguer", 10, 10, 20, DARKGRAY);
        DrawFPS(10, 30);

        EndDrawing();
    }

    // Désallocation des ressources
    UnloadModel(model);
    UnloadTexture(texture);

    CloseWindow();

    return 0;
}