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
#ifdef __cplusplus
extern "C" {
#endif

#include <vector>

#ifdef __cplusplus
}
#endif

#include "sol.h"
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
#define GRID_SIZE 40
#define MAX_LIGHTS 4 // Max dynamic lights supported by shader
#define SHADOWMAP_RESOLUTION 512 //la resolution de la shadowmap

#define VIDE  CLITERAL(Color){ 0, 0, 0, 0 }   // Light Gray
const float PENTE_SEUIL = 0.20f; //valeur de la pente max

//les ombres
//by @TheManTheMythTheGameDev
RenderTexture2D LoadShadowmapRenderTexture(int width, int height);
void UnloadShadowmapRenderTexture(RenderTexture2D target);


RenderTexture2D LoadShadowmapRenderTexture(int width, int height)
{
    RenderTexture2D target = { 0 };

    target.id = rlLoadFramebuffer(); // Load an empty framebuffer
    target.texture.width = width;
    target.texture.height = height;

    if (target.id > 0)
    {
        rlEnableFramebuffer(target.id);

        // Create depth texture
        // We don't need a color texture for the shadowmap
        target.depth.id = rlLoadTextureDepth(width, height, false);
        target.depth.width = width;
        target.depth.height = height;
        target.depth.format = 19;       //DEPTH_COMPONENT_24BIT?
        target.depth.mipmaps = 1;

        // Attach depth texture to FBO
        rlFramebufferAttach(target.id, target.depth.id, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_TEXTURE2D, 0);

        // Check if fbo is complete with attachments (valid)
        if (rlFramebufferComplete(target.id)) TRACELOG(LOG_INFO, "FBO: [ID %i] Framebuffer object created successfully", target.id);

        rlDisableFramebuffer();
    }
    else TRACELOG(LOG_WARNING, "FBO: Framebuffer object can not be created");

    return target;
}

// Unload shadowmap render texture from GPU memory (VRAM)
void UnloadShadowmapRenderTexture(RenderTexture2D target)
{
    if (target.id > 0)
    {
        // NOTE: Depth texture/renderbuffer is automatically
        // queried and deleted before deleting framebuffer
        rlUnloadFramebuffer(target.id);
    }
}

// Structure pour stocker les informations d'un objet 3D dans la grille
typedef struct {
    Vector3 position;   // Position de l'objet
    Model model;        // Modèle 3D de l'objet
    bool active;        // Indique si l'objet est actif ou non
    int temperature;    // Température de l'objet
} GridCell;

//fonction pour faire varier un parametre
void test_variation(GridCell * cellule){
    cellule->temperature = rand() % 100; // Assign a random temperature between 0 and 99
}
void undate_grille(GridCell grille[GRID_SIZE][GRID_SIZE]){
    for (int x = 0; x < GRID_SIZE; x++) {
        for (int z = 0; z < GRID_SIZE; z++) {
            if (grille[x][z].temperature % 2 == 0) {
                grille[x][z].active = true;
            } else {
                grille[x][z].active = false;
            }
        }
    }
}
//#include "monobjet.h"

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
    const int screenWidth = 800;//1920;
    const int screenHeight = 450;//1080;
    SetConfigFlags(FLAG_MSAA_4X_HINT); // Enable Multi Sampling Anti Aliasing 4x (if available)

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

    //Lumière directionnelle
    // Load basic lighting shader
    Shader shader = LoadShader(TextFormat("include/shaders/resources/shaders/glsl%i/lighting.vs", GLSL_VERSION),TextFormat("include/shaders/resources/shaders/glsl%i/lighting.fs", GLSL_VERSION));
    
    //les ombres
    Shader shadowShader = LoadShader(TextFormat("include/shaders/resources/shaders/glsl120/shadowmap.vs", GLSL_VERSION),TextFormat("include/shaders/resources/shaders/glsl120/shadowmap.fs", GLSL_VERSION));

    // Configurez les locations du shader de l'ombre
    shadowShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shadowShader, "viewPos");

    // cree la lumiere
    Light directionalLight = CreateLight(LIGHT_DIRECTIONAL, Vector3Zero(), (Vector3){ -1.0f, -1.0f, -1.0f }, WHITE, shader);
    //pour l'ombre
    /*
    Vector3 lightDir = Vector3Normalize((Vector3){ 0.35f, -1.0f, -0.35f });
    Color lightColor = WHITE;
    Vector4 lightColorNormalized = ColorNormalize(lightColor);
    int lightDirLoc = GetShaderLocation(shadowShader, "lightDir");
    int lightColLoc = GetShaderLocation(shadowShader, "lightColor");
    SetShaderValue(shadowShader, lightDirLoc, &lightDir, SHADER_UNIFORM_VEC3);
    SetShaderValue(shadowShader, lightColLoc, &lightColorNormalized, SHADER_UNIFORM_VEC4);
    int ambientLoc = GetShaderLocation(shadowShader, "ambient");
    float ambient[4] = {0.1f, 0.1f, 0.1f, 1.0f};
    SetShaderValue(shadowShader, ambientLoc, ambient, SHADER_UNIFORM_VEC4);
    int lightVPLoc = GetShaderLocation(shadowShader, "lightVP");
    int shadowMapLoc = GetShaderLocation(shadowShader, "shadowMap");
    int shadowMapResolution = SHADOWMAP_RESOLUTION;
    SetShaderValue(shadowShader, GetShaderLocation(shadowShader, "shadowMapResolution"), &shadowMapResolution, SHADER_UNIFORM_INT);
    */
    //test sol
    Image image_sol = LoadImage("ressources/heightmap.png");     // Load heightmap image (RAM)
    //Texture2D texture_sol = LoadTextureFromImage(image_sol);        // Convert image to texture (VRAM)

    Mesh mesh_sol = GenMeshHeightmap(image_sol, (Vector3){ 160, 80, 160 }); // Generate heightmap mesh (RAM and VRAM)
    Model model_sol = LoadModelFromMesh(mesh_sol); // Load model from generated mesh
    Image image_texture_sol = LoadImage("ressources/rocky_terrain_02_diff_1k.png");
    Texture2D texture_sol = LoadTextureFromImage(image_texture_sol); // Load map texture
    Shader shader_taille = LoadShader("include/shaders/resources/shaders/glsl100/base.vs", "include/shaders/resources/shaders/glsl100/base.fs");
    int uvScaleLoc = GetShaderLocation(shader_taille, "uvScale");
    Vector2 uvScale = {10.0f, 10.0f}; // Plus grand = texture plus petite et répétée
    SetShaderValue(shader_taille, uvScaleLoc, &uvScale, SHADER_UNIFORM_VEC2);

    model_sol.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture_sol; // Set map diffuse texture
    model_sol.materials[0].shader = shader_taille; // Assignez le shader au modèle

    Vector3 mapPosition = { -8.0f, 0.0f, -8.0f };           // Define model position

    //UnloadImage(image_sol);
    //fin test sol

    // Charger le modèle et la texture test commentaire
    Model model_sapin = LoadModel("models/pine_tree/scene.gltf");
    Texture2D texture_sapin = LoadTexture("models/pine_tree/textures/Leavs_baseColor.png");
    model_sapin.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture_sapin;
    
    Model model_buisson_europe = LoadModel("models/buisson/foret_classique/scene.gltf");
    Texture2D texture_buisson_europe = LoadTexture("models/buisson/foret_classique/textures/gbushy_baseColor.png");
    model_buisson_europe.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture_buisson_europe;

    
    //Model model_acacia = LoadModel("models/caca/scene.gltf");
    //Texture2D texture_acacia = LoadTexture("models/caca/textures/Acacia_Dry_Green__Mature__Acacia_Leaves_1_baked_Color-Acacia_Dry_Green__Mature__Acacia_Leaves_1_baked_Opacity.png");
    //model_acacia.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture_acacia;
    Model model_acacia = LoadModel("models/caca/New/scene.gltf");
    Texture2D texture_acacia = LoadTexture("models/caca/New/Acacia_Dry_Green__Mature__Acacia_Leaves_1_baked_Color-Acacia_Dry_Green__Mature__Acacia_Leaves_1_baked_Opacity.png");
    model_acacia.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture_acacia;

    Mesh cubeMesh = GenMeshCube(1.0f, 1.0f, 1.0f); // Génère un cube
    Model cubeModel = LoadModelFromMesh(cubeMesh);

    cubeModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = WHITE;

    model_sapin.materials[0].shader = shader;

    model_buisson_europe.materials[0].shader = shader;
    
    model_acacia.materials[0].shader = shadowShader;
    for (int i = 0; i < model_acacia.materialCount; i++)
    {
        model_acacia.materials[i].shader = shadowShader;
    }
    
    model_sol.materials[0].shader = shader;
    
    model_sapin.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
    model_buisson_europe.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
    model_acacia.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
    model_sol.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = WHITE;

    //la shadowmap
    RenderTexture2D shadowMap = LoadShadowmapRenderTexture(SHADOWMAP_RESOLUTION, SHADOWMAP_RESOLUTION);
    // Création d'une grille de cellules
    std::vector<std::vector<GridCell>> grille(GRID_SIZE, std::vector<GridCell>(GRID_SIZE, 
        GridCell({0,0,0}, LoadModel("models/caca/New/scene.gltf"), true, 20, 50)),0.);
    // Création d'une plante
    Plante buisson("Buisson", 30, 15, 3, 1, 0.00005f, 0.05f, 0, model_buisson_europe);
    Plante accacia("Acacia", 20, 10, 2, 1, 0.05f, 0.15f, 0, model_acacia);
    Plante sapin("Sapin", 10, 5, 1, 1, 0.05f, 0.01f, 0, model_sapin);
    // Initialisation de la grille
    //GridCell grid[GRID_SIZE][GRID_SIZE];
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

            // Générer une température arbitraire pour cette cellule
            int temperature = random_flottant(0, 30); // Température aléatoire entre TEMP_MIN et TEMP_MAX


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
            
            // Calcul du taux de pente
            float penteX = (deltaLeft + deltaRight) / 2.0f;
            float penteZ = (deltaUp + deltaDown) / 2.0f;
            float tauxPente = sqrt(penteX * penteX + penteZ * penteZ);
            
            // Vérifier si la cellule est sur une pente
            bool pente = (deltaLeft > PENTE_SEUIL || deltaRight > PENTE_SEUIL || deltaUp > PENTE_SEUIL || deltaDown > PENTE_SEUIL);
            /*
            if (!(pente)){
                grid[x][z].model = model_acacia;
                taille_min = 0.05f;
                taille_max = 0.15f;
                //besoin_retourner = 1;
            }
            else{
                grid[x][z].model = model_buisson_europe;
                taille_min = 0.00005f;
                taille_max = 0.0005f;
                besoin_retourner = 0;
            }
            */

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
            float randomRotationX = random_flottant(0.0f, 2.0f * PI); // Rotation aléatoire autour de l'axe X
            //convertir en radians
            randomRotationX = DEG2RAD * randomRotationX;
            transform = MatrixMultiply(transform, MatrixRotateX(randomRotationX));
            grid[x][z].model.transform = transform;
            grid[x][z].active = true;
        }
    }

    //on collecte les objets

    
    DisableCursor();// Limit cursor to relative movement inside the window
    
    // For the shadowmapping algorithm, we will be rendering everything from the light's point of view
    Camera3D lightCam = (Camera3D){ 0 };
    lightCam.position = Vector3Scale(lightDir, -15.0f);
    lightCam.target = Vector3Zero();

    // Use an orthographic projection for directional lights
    lightCam.projection = CAMERA_ORTHOGRAPHIC;
    lightCam.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    lightCam.fovy = 20.0f;

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
        //Lumière directionnelle
        // Update the shader with the camera view vector (points towards { 0.0f, 0.0f, 0.0f })
        float cameraPos[3] = { camera.position.x, camera.position.y, camera.position.z };
        SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], &cameraPos, SHADER_UNIFORM_VEC3);
        // Parcours de toute la grille pour mettre à jour les températures
        //for (int x = 0; x < GRID_SIZE; x++) {
        //    for (int z = 0; z < GRID_SIZE; z++) {
        //        test_variation(&grid[x][z]);
        //    }
        //}
        // Mise à jour des cellules
        for (int x = 0; x < GRID_SIZE; x++) {
            for (int z = 0; z < GRID_SIZE; z++) {
                grille[x][z].update(grille, x, z);
            }
        }
        // Mise à jour de la grille en fonction des nouvelles températures
        //undate_grille(grid);
        /*
        //l'ombre
        lightDir = Vector3Normalize(lightDir);
        lightCam.position = Vector3Scale(lightDir, -15.0f);
        SetShaderValue(shadowShader, shadowShader.locs[SHADER_LOC_VECTOR_VIEW], &cameraPos, SHADER_UNIFORM_VEC3);
        SetShaderValue(shadowShader, lightDirLoc, &lightDir, SHADER_UNIFORM_VEC3);
        // Rendu de la shadow map (vue depuis la lumière)
        BeginTextureMode(shadowMap);
        ClearBackground(WHITE);  // La shadow map stocke les profondeurs
        BeginMode3D(lightCam);
            DrawModel(cubeModel, (Vector3){ 1.0f, 1.0f, 2.0f }, 1.0f, WHITE);
            DrawModel(model_sol, mapPosition, 1.0f, MAROON);
            for (int x = 0; x < GRID_SIZE; x++) {
                for (int z = 0; z < GRID_SIZE; z++) {
                    if (grid[x][z].active) {
                        DrawModel(grid[x][z].model, grid[x][z].position, 1.0f, WHITE);
                    }
                }
            }
        EndMode3D();
        EndTextureMode();

        // Enregistrer les matrices de la lumière pour le shader
        Matrix lightView = rlGetMatrixModelview();
        Matrix lightProj = rlGetMatrixProjection();
        Matrix lightViewProj = MatrixMultiply(lightView, lightProj);
        SetShaderValueMatrix(shader, lightVPLoc, lightViewProj);
        */
        // Rendu final (vue normale)
        BeginDrawing();
        ClearBackground(SKYBLUE);

        BeginMode3D(camera);
        
        BeginShaderMode(shader);
        /*
        // Ajoutez les données de shadow map
        int shadowMapSlot = 10;
        rlActiveTextureSlot(shadowMapSlot);
        rlEnableTexture(shadowMap.depth.id);
        rlSetUniform(shadowMapLoc, &shadowMapSlot, SHADER_UNIFORM_INT, 1);

        // Passez les matrices de la lumière
        lightView = rlGetMatrixModelview();
        lightProj = rlGetMatrixProjection();
        lightViewProj = MatrixMultiply(lightView, lightProj);
        SetShaderValueMatrix(shader, lightVPLoc, lightViewProj);
        */
        
        //DrawModel(model_sol, mapPosition, 0.50f, MAROON);
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
            if (sceneObjects[i].model == &model_sol) {
                DrawModel(*sceneObjects[i].model, sceneObjects[i].position, 0.1f, WHITE);
            } else {
                DrawModel(*sceneObjects[i].model, sceneObjects[i].position, 1.0f, WHITE);
            }
        }
        //pour faire bouger la lumière
        directionalLight.position.x = 5.0f * cos(GetTime() * 0.5f);
        directionalLight.position.z = 5.0f * sin(GetTime() * 0.5f);
        //update la lumière
        UpdateLightValues(shader, directionalLight);
        
        EndShaderMode();
        DrawGrid(20, 1.0f);
        EndMode3D();
        
        DrawText("Grille d'objets 3D - Utilisez la souris pour naviguer", 10, 10, 20, DARKGRAY);
        DrawText("Maintenez le clic droit pour tourner la scène", 10, 25, 20, DARKGRAY);
        DrawFPS(10, 40);
        
        EndDrawing();
    }
    UnloadShader(shader);
    UnloadShader(shadowShader);
    UnloadShader(shader_taille);
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
    UnloadShadowmapRenderTexture(shadowMap);



    CloseWindow();

    return 0;
}