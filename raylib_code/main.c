/*******************************************************************************************
*
*   raylib [core] example - Basic window
*
*   Welcome to raylib!
*
*   
*
********************************************************************************************/
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <stdlib.h>
#define RLIGHTS_IMPLEMENTATION
#if defined(_WIN32) || defined(_WIN64)
#include "shaders/rlights.h"
#elif defined(__linux__)
#include "shaders/rlights.h"
#endif

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif
#define GRID_SIZE 5
#define VIDE  CLITERAL(Color){ 0, 0, 0, 0 }   // Light Gray



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

// Variables globales pour stocker les angles de rotation
float angleX = 0.0f; // Rotation autour de l'axe X
float angleY = 0.0f; // Rotation autour de l'axe Y
float distance = 10.0f; // Distance entre la caméra et la cible

// Variable pour activer/désactiver la rotation
bool isRotating = false;


typedef struct {
    GridCell *cell;
    float depth;
} TransparentObject;

int CompareDepth(const void *a, const void *b) {
    TransparentObject *objA = (TransparentObject *)a;
    TransparentObject *objB = (TransparentObject *)b;
    return (objA->depth < objB->depth) - (objA->depth > objB->depth); // Tri décroissant
}

void SortTransparentObjects(Camera camera, TransparentObject *transparentObjects, int count) {
    for (int i = 0; i < count; i++) {
        transparentObjects[i].depth = Vector3Distance(camera.position, transparentObjects[i].cell->position);
    }
    qsort(transparentObjects, count, sizeof(TransparentObject), CompareDepth);
}

//terrain avec hauteur
float GetHeightFromTerrain(Vector3 position, Image heightmap, Vector3 terrainSize) {
    int mapX = (int)((position.x + terrainSize.x / 2.0f) * heightmap.width / terrainSize.x);
    int mapZ = (int)((position.z + terrainSize.z / 2.0f) * heightmap.height / terrainSize.z);

    mapX = Clamp(mapX, 0, heightmap.width - 1);
    mapZ = Clamp(mapZ, 0, heightmap.height - 1);

    Color pixel = GetImageColor(heightmap, mapX, mapZ);
    return (pixel.r / 255.0f) * terrainSize.y;
}


int main(void) {
    // Initialisation
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib - Grille avec objets 3D");
    rlDisableBackfaceCulling();//pour voir l'arriere des objets
    rlEnableDepthTest();

    rlEnableScissorTest();
    rlEnableColorBlend();

    // Caméra pour visualiser la scène
    Camera camera = { 
    .position = (Vector3){ 0.0f, 0.0f, 0.0f },
    .target = (Vector3){ 0.0f, 0.0f, 0.0f },
    .up = (Vector3){ 0.0f, 1.0f, 0.0f },
    .fovy = 85.0f,
    .projection = CAMERA_PERSPECTIVE
    };
    

    //test sol
    Image image_sol = LoadImage("heightmap.png");     // Load heightmap image (RAM)
    Texture2D texture_sol = LoadTextureFromImage(image_sol);        // Convert image to texture (VRAM)

    Mesh mesh_sol = GenMeshHeightmap(image_sol, (Vector3){ 16, 8, 16 }); // Generate heightmap mesh (RAM and VRAM)
    Model model_sol = LoadModelFromMesh(mesh_sol);                  // Load model from generated mesh

    model_sol.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture_sol; // Set map diffuse texture
    Vector3 mapPosition = { -8.0f, 0.0f, -8.0f };           // Define model position

    //UnloadImage(image_sol);
    //fin test sol

    // Load basic lighting shader
    Shader shader = LoadShader(TextFormat("resources/shaders/glsl%i/lighting.vs", GLSL_VERSION), TextFormat("resources/shaders/glsl%i/lighting.fs", GLSL_VERSION));
    
    // Charger le modèle et la texture test commentaire
    Model model = LoadModel("pine_tree/scene.gltf");
    Texture2D texture = LoadTexture("pine_tree/textures/Leavs_baseColor.png");
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
    
    // Initialisation de la grille
    GridCell grid[GRID_SIZE][GRID_SIZE];

   // Initialisation de la grille
    for (int x = 0; x < GRID_SIZE; x++) {
        for (int z = 0; z < GRID_SIZE; z++) {
            float posX = x * 4.0f - 8.0f;  // Ajustez selon votre terrain
            float posZ = z * 4.0f - 8.0f;

            // Obtenir la hauteur du terrain pour cette cellule
            float height = GetHeightFromTerrain((Vector3){ posX, 0.0f, posZ }, image_sol, (Vector3){ 16, 8, 16 });

            // Positionner la cellule en fonction de la hauteur du terrain
            grid[x][z].position = (Vector3){ posX, height, posZ };
            grid[x][z].model = model;
            float taille = random_flottant(3.5f,5.15f);
            Matrix transform = MatrixIdentity();

            // Appliquer l'échelle pour réduire ou agrandir le modèle
            transform = MatrixMultiply(transform, MatrixScale(taille, taille, taille));
            // Rotation pour orienter l'arbre vers le haut (si nécessaire)
            transform = MatrixMultiply(transform, MatrixRotateX(-(PI / 2.0f))); // Exemple pour une rotation X
        
            grid[x][z].model.transform = transform;
            grid[x][z].active = true;
        }
    }


    DisableCursor();// Limit cursor to relative movement inside the window

    SetTargetFPS(5000);
    

    // Boucle principale
    while (!WindowShouldClose()) {
         // Activer/désactiver la rotation avec le clic droit
        if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) isRotating = true;
        if (IsMouseButtonReleased(MOUSE_RIGHT_BUTTON)) isRotating = false;

        // Capture des mouvements de la souris
        if (isRotating) {
            Vector2 mouseDelta = GetMouseDelta();
            angleX -= mouseDelta.y * 0.2f; // Sensibilité verticale
            angleY -= mouseDelta.x * 0.2f; // Sensibilité horizontale
        }
        // Gestion du zoom avec la molette de la souris
        distance -= GetMouseWheelMove() * 0.5f; // Ajustez le facteur (0.5f) pour contrôler la sensibilité du zoom
        if (distance < 2.0f) distance = 2.0f;   // Distance minimale
        if (distance > 50.0f) distance = 50.0f; // Distance maximale


        // Limiter les angles X pour éviter une rotation complète
        if (angleX > 89.0f) angleX = 89.0f;
        if (angleX < -89.0f) angleX = -89.0f;

        // Calcul de la position de la caméra en coordonnées sphériques
        float radAngleX = DEG2RAD * angleX;
        float radAngleY = DEG2RAD * angleY;

        camera.position.x = distance * cos(radAngleX) * sin(radAngleY);
        camera.position.y = distance * sin(radAngleX);
        camera.position.z = distance * cos(radAngleX) * cos(radAngleY);

        DisableCursor();//pour pas avoir le curseur qui sort de l'ecran
        
        // Dessiner
        BeginDrawing();
        ClearBackground(SKYBLUE);

        BeginMode3D(camera);
        
        BeginShaderMode(shader);
        DrawCube(Vector3Zero(), 2.0, 4.0, 2.0, WHITE);


        // Dessiner les objets de la grille
        for (int x = 0; x < GRID_SIZE; x++) {
            for (int z = 0; z < GRID_SIZE; z++) {
                if (grid[x][z].active) {
                    DrawModel(grid[x][z].model, grid[x][z].position, 0.5f, WHITE);
                }
                variation_hauteur(grid[x][z]);
            }
        }

        DrawModel(model_sol, mapPosition, 1.0f, RED);

        DrawGrid(20, 1.0f);

        EndShaderMode();
        

        EndMode3D();
        

        DrawText("Grille d'objets 3D - Utilisez la souris pour naviguer", 10, 10, 20, DARKGRAY);
        DrawText("Maintenez le clic droit pour tourner la scène", 10, 25, 20, DARKGRAY);
        DrawFPS(10, 40);

        EndDrawing();
    }

    // Désallocation des ressources
    UnloadModel(model);
    UnloadTexture(texture);
    UnloadShader(shader);   // Unload shader
    UnloadModel(model_sol);
    UnloadTexture(texture_sol);


    CloseWindow();

    return 0;
}


//lumiere
        //for (int i = 0; i < 1; i++)
        //        {
        //            if (lights[i].enabled) DrawSphereEx(lights[i].position, 0.2f, 8, 8, lights[i].color);
        //            else DrawSphereWires(lights[i].position, 0.2f, 8, 8, ColorAlpha(lights[i].color, 0.3f));
        //        }
        //DrawGrid(10, 1.0f);  // Grille visuelle

////////////////////////////////////test de profondeur marche pas à l'arrière/////////////////
    // Dessiner les objets opaques en premier
//    for (int x = 0; x < GRID_SIZE; x++) {
//        for (int z = 0; z < GRID_SIZE; z++) {
//            if (grid[x][z].active) {
//                DrawModel(grid[x][z].model, grid[x][z].position, 1.0f, WHITE);
//            }
//        }
//    }

    // Collecter les objets transparents
//    TransparentObject transparentObjects[GRID_SIZE * GRID_SIZE];
//    int transparentCount = 0;

//    for (int x = 0; x < GRID_SIZE; x++) {
//        for (int z = 0; z < GRID_SIZE; z++) {
//            if (grid[x][z].active) {
//                transparentObjects[transparentCount].cell = &grid[x][z];
//                transparentCount++;
//            }
//        }
//    }

    // Trier les objets transparents par profondeur
//    SortTransparentObjects(camera, transparentObjects, transparentCount);

    // Désactiver l'écriture dans le tampon de profondeur pour les objets transparents
//    rlDisableDepthMask();

    // Dessiner les objets transparents dans l'ordre trié
//    for (int i = 0; i < transparentCount; i++) {
//        DrawModel(transparentObjects[i].cell->model, transparentObjects[i].cell->position, 1.0f, WHITE);
//    }


        //for (int i = 0; i < 1; i++)
        //        {
        //            if (lights[i].enabled) DrawSphereEx(lights[i].position, 0.2f, 8, 8, lights[i].color);
        //            else DrawSphereWires(lights[i].position, 0.2f, 8, 8, ColorAlpha(lights[i].color, 0.3f));
        //        }
        //DrawGrid(10, 1.0f);  // Grille visuelle

        // Dessiner les objets de la grille
  //      for (int x = 0; x < GRID_SIZE; x++) {
//            for (int z = 0; z < GRID_SIZE; z++) {
                //if (grid[x][z].active) {
                //    DrawModel(grid[x][z].model, grid[x][z].position, 0.5f, WHITE);
              //  }
            //    variation_hauteur(grid[x][z]);
          //  }
        //}
// Réactiver l'écriture dans le tampon de profondeur
//rlEnableDepthMask();