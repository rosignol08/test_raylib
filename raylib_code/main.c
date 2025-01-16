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
#include <stdio.h>//pour les printf
#define RLIGHTS_IMPLEMENTATION
#if defined(_WIN32) || defined(_WIN64)
#include "include/shaders/rlights.h"
#elif defined(__linux__)
#include "include/shaders/rlights.h"
#endif

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif
#define GRID_SIZE 50
#define VIDE  CLITERAL(Color){ 0, 0, 0, 0 }   // Light Gray
const float PENTE_SEUIL = 0.20f; //valeur de la pente max



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

// Structure pour stocker les informations d'un objet 3D dans la scène
typedef struct {
    Vector3 position;    // Position de l'objet
    Model *model;        // Modèle 3D
    float depth;         // Distance caméra → objet
} SceneObject;


int CompareSceneObjects(const void *a, const void *b) {
    SceneObject *objA = (SceneObject *)a;
    SceneObject *objB = (SceneObject *)b;
    return (objA->depth < objB->depth) - (objA->depth > objB->depth); // Tri décroissant
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
    const int screenWidth = 1920;//800;
    const int screenHeight = 1080;//450;

    InitWindow(screenWidth, screenHeight, "raylib - Grille avec objets 3D");
    rlDisableBackfaceCulling();//pour voir l'arriere des objets
    /*
    rlEnableDepthTest();
    rlEnableDepthMask();
    rlEnableScissorTest();
    rlEnableColorBlend();
    glEnable(GL_DEPTH_TEST);
    */
    //rlEnableDepthTest();    // Activer le test de profondeur
    //rlEnableDepthMask();    // Activer l'écriture dans le tampon de profondeur
    //glBlendFunc(RL_SRC_ALPHA, RL_ONE_MINUS_SRC_ALPHA);
    rlEnableColorBlend(); // Activer le blending
    rlSetBlendMode(RL_BLEND_ALPHA);
    // Caméra pour visualiser la scène
    Camera camera = { 
    .position = (Vector3){ 0.0f, 0.0f, 0.0f },
    .target = (Vector3){ 0.0f, 0.0f, 0.0f },
    .up = (Vector3){ 0.0f, 1.0f, 0.0f },
    .fovy = 85.0f,
    .projection = CAMERA_PERSPECTIVE
    };
    

    //test sol
    Image image_sol = LoadImage("ressources/heightmap.png");     // Load heightmap image (RAM)
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
    Model model_sapin = LoadModel("models/pine_tree/scene.gltf");
    Texture2D texture_sapin = LoadTexture("models/pine_tree/textures/Leavs_baseColor.png");
    model_sapin.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture_sapin;
    
    Model model_buisson_europe = LoadModel("models/buisson/foret_classique/scene.gltf");
    Texture2D texture_buisson_europe = LoadTexture("models/buisson/foret_classique/textures/gbushy_baseColor.png");
    model_buisson_europe.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture_buisson_europe;

    Model model_acacia = LoadModel("models/acacia2/untitled.glb");
    //Texture2D texture_acacia = LoadTexture("models/acacia2/Acacia_Dry_Green__Mature__Acacia_Trunk_baked_Color.png");
    /*
    Texture2D texture_acacia2 = LoadTexture("models/acacia2/Maps/Acacia_Dry_Green__Mature__Acacia_Leaves_1_baked_Color.png");
    Texture2D texture_acacia3 = LoadTexture("models/acacia2/Maps/Acacia_Dry_Green__Mature__Acacica_Leaves_2_baked_Color.png");
    Texture2D texture_acacia4 = LoadTexture("models/acacia2/Maps/Acacia_Dry_Green__Mature__Acacica_Leaves_3_baked_Color.png");
    model_acacia.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture_acacia;
    */
    /*
    model_acacia.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = texture_acacia2;
    model_acacia.materials[2].maps[MATERIAL_MAP_DIFFUSE].texture = texture_acacia3;
    model_acacia.materials[3].maps[MATERIAL_MAP_DIFFUSE].texture = texture_acacia4;
    */

    // Initialisation de la grille
    GridCell grid[GRID_SIZE][GRID_SIZE];
    float taille_min = 0;
    float taille_max = 0;
    int besoin_retourner = 0;
    
   // Initialisation de la grille
    for (int x = 0; x < GRID_SIZE; x++) {
        for (int z = 0; z < GRID_SIZE; z++) {
            float posX = x * 0.30f - 8.0f;  // Ajustez selon votre terrain
            float posZ = z * 0.30f - 8.0f;

            
            // Ajouter une irrégularité aux positions X et Z
            float offsetX = random_flottant(-0.1f, 0.1f); // Décalage aléatoire pour X
            float offsetZ = random_flottant(-0.1f, 0.1f); // Décalage aléatoire pour Z

            posX += offsetX;
            posZ += offsetZ;

            // Obtenir la hauteur du terrain pour cette cellule
            float height = GetHeightFromTerrain((Vector3){ posX, 0.0f, posZ }, image_sol, (Vector3){ 16, 8, 16 });

            // Calcul des hauteurs des cellules voisines
            float heightLeft = GetHeightFromTerrain((Vector3){ posX - 0.3f, 0.0f, posZ }, image_sol, (Vector3){ 16, 8, 16 });
            float heightRight = GetHeightFromTerrain((Vector3){ posX + 0.3f, 0.0f, posZ }, image_sol, (Vector3){ 16, 8, 16 });
            float heightUp = GetHeightFromTerrain((Vector3){ posX, 0.0f, posZ - 0.3f }, image_sol, (Vector3){ 16, 8, 16 });
            float heightDown = GetHeightFromTerrain((Vector3){ posX, 0.0f, posZ + 0.3f }, image_sol, (Vector3){ 16, 8, 16 });

            // Calcul des variations de hauteur
            float deltaLeft = fabs(height - heightLeft);
            float deltaRight = fabs(height - heightRight);
            float deltaUp = fabs(height - heightUp);
            float deltaDown = fabs(height - heightDown);

            // Vérifier si la cellule est sur une pente
            bool pente = (deltaLeft > PENTE_SEUIL || deltaRight > PENTE_SEUIL || deltaUp > PENTE_SEUIL || deltaDown > PENTE_SEUIL);

            if (!(pente)){
                grid[x][z].model = model_sapin;
                taille_min = 0.05f;
                taille_max = 0.15f;
                besoin_retourner = 1;
            }
            else{
                grid[x][z].model = model_buisson_europe;
                taille_min = 0.00005f;
                taille_max = 0.0005f;
                besoin_retourner = 0;
            }
            
            // Positionner la cellule en fonction de la hauteur du terrain
            grid[x][z].position = (Vector3){ posX, height, posZ };
            //grid[x][z].model = model_sapin;
            float taille = random_flottant(taille_min, taille_max);
            Matrix transform = MatrixIdentity();

            // Appliquer l'échelle pour réduire ou agrandir le modèle
            transform = MatrixMultiply(transform, MatrixScale(taille, taille, taille));
            if (besoin_retourner == 1){
                // Rotation pour orienter l'arbre vers le haut (si nécessaire)
                transform = MatrixMultiply(transform, MatrixRotateX(-(PI / 2.0f))); // Exemple pour une rotation X
            }else if (besoin_retourner == 2){
                // Rotation pour orienter l'arbre vers le haut (si nécessaire)
                transform = MatrixMultiply(transform, MatrixRotateX(PI / 2.0f)); // Exemple pour une rotation X
            }
            
            grid[x][z].model.transform = transform;
            grid[x][z].active = true;
        }
    }

    //on collecte les objets

    
    DisableCursor();// Limit cursor to relative movement inside the window

    SetTargetFPS(165);
    

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
        
        //DrawCube(Vector3Zero(), 2.0, 4.0, 2.0, WHITE);
        //DrawModel(model_sol, mapPosition, 1.0f, RED);
        // Dessiner les objets de la grille
        /*
        for (int x = 0; x < GRID_SIZE; x++) {
            for (int z = 0; z < GRID_SIZE; z++) {
                if (grid[x][z].active) {
                    DrawModel(grid[x][z].model, grid[x][z].position, 0.5f, WHITE);
                }
                variation_hauteur(grid[x][z]);
            }
        }
        */
       DrawModel(model_sol, mapPosition, 1.0f, MAROON);
        SceneObject sceneObjects[GRID_SIZE * GRID_SIZE + 1]; // +1 pour inclure le sol
int objectCount = 0;

// Ajouter le sol à la liste
sceneObjects[objectCount].position = mapPosition;
sceneObjects[objectCount].model = &model_sol;
sceneObjects[objectCount].depth = Vector3Distance(camera.position, mapPosition);
objectCount++;

// Ajouter les arbres à la liste
for (int x = 0; x < GRID_SIZE; x++) {
    for (int z = 0; z < GRID_SIZE; z++) {
        if (grid[x][z].active) {
            sceneObjects[objectCount].position = grid[x][z].position;
            sceneObjects[objectCount].model = &grid[x][z].model;
            sceneObjects[objectCount].depth = Vector3Distance(camera.position, grid[x][z].position);
            objectCount++;
        }
    }
}
// Trier les objets par profondeur
qsort(sceneObjects, objectCount, sizeof(SceneObject), CompareSceneObjects);

// Dessiner les objets dans l'ordre trié
for (int i = 0; i < objectCount; i++) {
    DrawModel(*sceneObjects[i].model, sceneObjects[i].position, 1.0f, WHITE);
}
/* ou alors ça
// Rendre les faces arrière des feuilles
rlSetCullFace(RL_CULL_FACE_FRONT); // Ne pas dessiner les faces avant
for (int i = 0; i < objectCount; i++) {
    DrawModel(*sceneObjects[i].model, sceneObjects[i].position, 1.0f, WHITE);
}

// Rendre les faces avant des feuilles
rlSetCullFace(RL_CULL_FACE_BACK); // Ne pas dessiner les faces arrière
for (int i = 0; i < objectCount; i++) {
    DrawModel(*sceneObjects[i].model, sceneObjects[i].position, 1.0f, WHITE);
}

// Réinitialiser le mode de culling
rlSetCullFace(RL_CULL_FACE_BACK);
    */    

        

        DrawGrid(20, 1.0f);
        EndShaderMode();
        

        EndMode3D();
        
        DrawText("Grille d'objets 3D - Utilisez la souris pour naviguer", 10, 10, 20, DARKGRAY);
        DrawText("Maintenez le clic droit pour tourner la scène", 10, 25, 20, DARKGRAY);
        DrawFPS(10, 40);
        EndDrawing();
    }

    // Désallocation des ressources
    UnloadModel(model_sapin);
    UnloadTexture(texture_sapin);
    UnloadModel(model_buisson_europe);
    UnloadTexture(texture_buisson_europe);
    //UnloadModel(model_acacia);
    //UnloadTexture(texture_acacia);
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