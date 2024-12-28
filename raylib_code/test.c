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
#define RLIGHTS_IMPLEMENTATION
//pour linux:
#include "/home/romaric/Bureau/3d_raylib_test/test_raylib/raylib/examples/shaders/rlights.h"
//pour windows:
#include "C:\Users\bunny\vcpkg\buildtrees\raylib\src\5.5-966575b391.clean\examples\shadersrlights.h"

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif
#define GRID_SIZE 5


// Light type
typedef enum {
    LIGHT_DIRECTIONAL = 0,
    LIGHT_POINT,
    LIGHT_SPOT
} LightType;

// Light data
typedef struct {
    int type;
    int enabled;
    Vector3 position;
    Vector3 target;
    float color[4];
    float intensity;

    // Shader light parameters locations
    int typeLoc;
    int enabledLoc;
    int positionLoc;
    int targetLoc;
    int colorLoc;
    int intensityLoc;
} Light;

static int lightCount = 0;
// Create a light and get shader locations
static Light CreateLight(int type, Vector3 position, Vector3 target, Color color, float intensity, Shader shader);

// Update light properties on shader
// NOTE: Light shader locations should be available
static void UpdateLight(Shader shader, Light light);

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

    // Load basic lighting shader
    Shader shader = LoadShader(TextFormat("resources/shaders/glsl%i/lighting.vs", GLSL_VERSION),
                               TextFormat("resources/shaders/glsl%i/lighting.fs", GLSL_VERSION));
    // Get some required shader locations
    //shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");
    // Create a light and get shader locations
    static Light CreateLight(int type, Vector3 position, Vector3 target, Color color, float intensity, Shader shader);

    // Update light properties on shader
    // NOTE: Light shader locations should be available
    static void UpdateLight(Shader shader, Light light);

    // NOTE: "matModel" location name is automatically assigned on shader loading, 
    // no need to get the location again if using that uniform name
    //shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(shader, "matModel");
    
    // Ambient light level (some basic lighting)
    //int ambientLoc = GetShaderLocation(shader, "direc");
    //SetShaderValue(shader, ambientLoc, (float[4]){ 0.0f, 0.0f, 0.0f, 1.0f }, SHADER_UNIFORM_VEC4);

    // Create lights
    //Light lights[1] = { 0 };
    //lights[0] = CreateLight(LIGHT_DIRECTIONAL, (Vector3){ 0, 1, -1 }, Vector3Zero(), RED, shader);

    // Charger le modèle et la texture test commentaire
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
            grid[x][z].model.transform = MatrixScale(taille,taille,taille); // Réduire la taille du modèle
            grid[x][z].active = true; // Activer tous les objets par défaut
        }
    }

    DisableCursor();// Limit cursor to relative movement inside the window

    SetTargetFPS(850);

    // Boucle principale
    while (!WindowShouldClose()) {
        // Mise à jour de la caméra
        UpdateCamera(&camera, CAMERA_FREE);
        
        //float cameraPos[3] = { camera.position.x, camera.position.y, camera.position.z };
        //SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);
        //if (IsKeyPressed(KEY_Y)) { lights[0].enabled = !lights[0].enabled; }
        //for (int i = 0; i < 1; i++) UpdateLightValues(shader, lights[i]);

        // Dessiner
        BeginDrawing();
        ClearBackground(SKYBLUE);

        BeginMode3D(camera);
        
        BeginShaderMode(shader);
        DrawPlane(Vector3Zero(), (Vector2) { 10.0, 10.0 }, WHITE);
        DrawCube(Vector3Zero(), 2.0, 4.0, 2.0, WHITE);
        //for (int i = 0; i < 1; i++)
        //        {
        //            if (lights[i].enabled) DrawSphereEx(lights[i].position, 0.2f, 8, 8, lights[i].color);
        //            else DrawSphereWires(lights[i].position, 0.2f, 8, 8, ColorAlpha(lights[i].color, 0.3f));
        //        }
        //DrawGrid(10, 1.0f);  // Grille visuelle

        // Dessiner les objets de la grille
        for (int x = 0; x < GRID_SIZE; x++) {
            for (int z = 0; z < GRID_SIZE; z++) {
                if (grid[x][z].active) {
                    DrawModel(grid[x][z].model, grid[x][z].position, 0.5f, GREEN);
                }
                variation_hauteur(grid[x][z]);
            }
        }
        
        EndShaderMode();
        
        EndMode3D();

        DrawText("Grille d'objets 3D - Utilisez la souris pour naviguer", 10, 10, 20, DARKGRAY);
        DrawFPS(10, 30);

        EndDrawing();
    }

    // Désallocation des ressources
    UnloadModel(model);
    UnloadTexture(texture);
    UnloadShader(shader);   // Unload shader

    CloseWindow();

    return 0;
}