/*******************************************************************************************
*
*
*   Welcome to raylib!
*
*   
*
********************************************************************************************/
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include <stdlib.h>
#include <stdio.h>//pour les printf
#include <vector>

#include "sol.h"
#include "nuages.h"
#define RLIGHTS_IMPLEMENTATION
#if defined(_WIN32) || defined(_WIN64)
#include "include/shaders/rlights.h"
#elif defined(__linux__)
#include "include/shaders/rlights.h"
#endif

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            330//120//si c'est 100 ça ouvre pas les autres shaders
#endif
#define GRID_SIZE 10
#define NBHERBE 25
#define MAX_LIGHTS 4 // Max dynamic lights supported by shader
#define SHADOWMAP_RESOLUTION 4096 //la resolution de la shadowmap

#define MODE_NORMAL 0
#define MODE_TEMPERATURE 1
#define MODE_HUMIDITE 2
int viewMode = MODE_NORMAL;

#define VIDE  CLITERAL(Color){ 0, 0, 0, 0 }   // Light Gray
const float PENTE_SEUIL = 0.20f; //valeur de la pente max

float timeOfDay = 12.0f; // L'heure du jour (de 0 à 24)
const float pI = 3.14159265359f;

Color GetSunColor(float timeOfDay) {
    if (timeOfDay < 6.0f || timeOfDay >= 20.0f) {
        return (Color){ 10, 10, 30, 255 }; // Nuit profonde - bleu très foncé
    } else if (timeOfDay < 7.0f) {
        float t = (timeOfDay - 6.0f); // Transition de 6h à 7h
        return (Color){
            (unsigned char)(255 * t),        // Rouge augmente
            (unsigned char)(165 * t),  // Vert augmente255,165,0
            (unsigned char)(10 * t),   // Bleu diminue
            255
        }; // Lever du soleil - transition du bleu foncé au rose
    } else if (timeOfDay < 8.0f) {
        float t = (timeOfDay - 7.0f); // Transition de 7h à 8h
        return (Color){
            255,                     // Rouge constant
            (unsigned char)(200 + t * 55),   // Vert augmente légèrement
            (unsigned char)(100 - t * 50),   // Bleu diminue
            255
        }; // Lever du soleil - transition du rose au doré
    } else if (timeOfDay < 17.0f) {
        return (Color){ 255, 255, 255, 255 }; // Journée - blanc éclatant
    } else if (timeOfDay < 18.0f) {
        float t = (timeOfDay - 17.0f); // Transition de 17h à 18h
        return (Color){
            (unsigned char)(255 - t * 5),  // Rouge diminue
            (unsigned char)(255 - t * 55),   // Vert diminue légèrement
            (unsigned char)(100 + t * 50),   // Bleu augmente
            255
        }; // Coucher du soleil - transition du blanc au doré
    } else if (timeOfDay < 19.0f) {
        float t = (timeOfDay - 18.0f); // Transition de 18h à 19h
        return (Color){
            255,  // Rouge constant
            (unsigned char)(105 + t * 45),  // Vert augmente légèrement
            (unsigned char)(180 - t * 80),  // Bleu diminue
            255
        }; // Transition vers la couleur 255,105,180
        t = (timeOfDay - 18.0f); // Transition de 18h à 19h
        return (Color){
            (unsigned char)(255 - t * 155),  // Rouge diminue
            (unsigned char)(150 - t * 100),  // Vert diminue
            (unsigned char)(100 + t * 50),   // Bleu augmente
            255
        }; // Coucher du soleil - transition du doré au bleu foncé
    } else {
        float t = (timeOfDay - 19.0f); // Transition de 19h à 20h
        return (Color){
            (unsigned char)(100 - t * 90),   // Rouge diminue
            (unsigned char)(50 - t * 40),    // Vert diminue
            (unsigned char)(150 + t * 10),   // Bleu augmente légèrement
            255
        }; // Fin du coucher du soleil - transition vers la nuit
    }
}
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

//pour dessiner la scene 
void dessine_scene(Camera camera, Image image_sol, Vector3 taille_terrain, Model model_sol, Model model_buisson_europe, Model model_acacia, Model model_sapin, Model model_mort, Model emptyModel, Plante buisson, Plante accacia, Plante sapin, Plante plante_morte, Plante herbe, std::vector<Plante> plantes, std::vector<std::vector<GridCell>> grille, int viewMode, int &minTemp, int &maxTemp, int &minHum, int &maxHum, Vector3 mapPosition);

//fonction pour faire varier un parametre
void test_variation(GridCell * cellule){
    cellule->temperature = rand() % 100; // Assign a random temperature between 0 and 99
}
void update_grille(GridCell grille[GRID_SIZE][GRID_SIZE]){
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
float distance_cam = 5.0f; // Distance entre la caméra et la cible

// Variable pour activer/désactiver la rotation
bool isRotating = false;

// Structure pour stocker les informations d'un objet 3D dans la scène
typedef struct {
    Vector3 position;    // Position de l'objet
    Model *model;        // Modèle 3D
    float depth;         // Distance caméra → objet
    Color color = WHITE; // Couleur de l'objet, par défaut blanche
} SceneObject;
int CompareSceneObjects(const void *a, const void *b) {
    SceneObject *objA = (SceneObject *)a;
    SceneObject *objB = (SceneObject *)b;
    return (objA->depth < objB->depth) - (objA->depth > objB->depth); // Tri décroissant
}

//fonction pour vierifie quel plante peut vivre sous les conditions de sa case
void verifier_plante(std::vector<std::vector<GridCell>> &grille, GridCell *cellule, std::vector<Plante> plantes, Plante plante_morte, Plante vide, int minTemp, int maxTemp, int minHum, int maxHum, Color couleur_sante){
    if(cellule->plante.nom == "Morte" || cellule->plante.nom == "Vide"){//si la plante est morte
        if(cellule->plante.age >= cellule->plante.age_max){//si la plante est morte depuis trop longtemps
            Plante bestPlante = vide;
            float bestScore = 0;
            int nb_plantes_qui_peuvent_survivre = 0;
            float scoreTemperature = 0;
            float scoreHumidite = 0;
            float score = 0;
            for (Plante plante : plantes) {
                if (cellule->temperature >= plante.temperature_min && cellule->temperature <= plante.temperature_max &&
                    cellule->humidite >= plante.humidite_min && cellule->humidite <= plante.humidite_max && cellule->pente >= plante.pente_min &&
                    cellule->pente <= plante.pente_max) {
                    nb_plantes_qui_peuvent_survivre++;
                }
            }
            for (Plante plante : plantes) {
                if (cellule->temperature >= plante.temperature_min && cellule->temperature <= plante.temperature_max &&
                    cellule->humidite >= plante.humidite_min && cellule->humidite <= plante.humidite_max && cellule->pente >= plante.pente_min &&
                    cellule->pente <= plante.pente_max) {
                    scoreTemperature = fabs(cellule->temperature - (plante.temperature_max + plante.temperature_min)); //plus c'est proche de 0 mieux c'est
                    scoreHumidite = fabs(cellule->humidite - (plante.humidite_max + plante.humidite_min)); //plus c'est proche de 0 mieux c'est
                    score = scoreTemperature + scoreHumidite;
                    if (score < bestScore || bestScore == 0) {
                        bestScore = score;
                        bestPlante = plante;
                    }
                }
            }
            //cout << "la meilleure plante est : " << bestPlante.nom << endl;
            cellule->plante = bestPlante;
            cellule->plante.age = rand() % 500;
            cellule->plante.taille = bestPlante.taille;
            // Appliquer les influences sur les cases voisines
            for (int dx = -1; dx <= 1; dx++) {
                for (int dz = -1; dz <= 1; dz++) {
                    if (dx == 0 && dz == 0) continue; // Ignorer la cellule actuelle

                    int voisinX = (cellule->identifiant / GRID_SIZE) + dx;
                    int voisinZ = (cellule->identifiant % GRID_SIZE) + dz;

                    // Vérifier si les coordonnées sont valides
                    if (voisinX >= 0 && voisinX < GRID_SIZE && voisinZ >= 0 && voisinZ < GRID_SIZE) {
                        GridCell *voisin = &grille[voisinX][voisinZ];

                        // Appliquer les influences de la plante sur la cellule voisine
                        voisin->temperature += cellule->plante.influence_temperature;
                        voisin->humidite += cellule->plante.influence_humidite;

                        // Limiter les valeurs pour éviter les dépassements
                        //voisin->temperature = Clamp(voisin->temperature, minTemp, maxTemp);
                        voisin->humidite = Clamp(voisin->humidite, 0, 100); // Limiter l'humidité entre 0 et 100 parce que c'est un pourcentage
                    }
                }
            }

            return;
        }
        else{
            cellule->plante.age++;//bug ça incrémente pas l'age de la plante morte TODO
            return;
        }
    }
    else{//si la plante n'est pas morte
        cellule->plante.age++;
        if (cellule->plante.age >= cellule->plante.age_max) {
            cellule->plante.age = 0;
            float taille_actuelle = cellule->plante.taille;
            Plante planteMorteActuelle = plante_morte;
            planteMorteActuelle.taille = taille_actuelle;
            cellule->plante = planteMorteActuelle;
            return;
        }
        else{
            if(cellule->plante.nom != plante_morte.nom) {
                if (cellule->plante.sante <= 1){
                    cellule->plante.sante = 0;
                    cellule->plante.age = 0;
                    float taille_actuelle = cellule->plante.taille;
                    Plante planteMorteActuelle = plante_morte;
                    planteMorteActuelle.taille = taille_actuelle;
                    cellule->plante = planteMorteActuelle;
                    return;
                }
            //modifer pout ajouter un système de santée
                if (cellule->pente >= cellule->plante.pente_min &&
                    cellule->pente <= cellule->plante.pente_max) {//si elle peut survivre
                    //verifier si la plante à cette case est la meilleure sinon on baisse sa santée
                    Plante bestPlante = cellule->plante;
                    float bestScore = 0;
                    int nb_plantes_qui_peuvent_survivre = 0;
                    float scoreTemperature = 0;
                    float scoreHumidite = 0;
                    float score = 0;
                    for (Plante plante : plantes) {
                        if (cellule->temperature >= plante.temperature_min && cellule->temperature <= plante.temperature_max &&
                            cellule->humidite >= plante.humidite_min && cellule->humidite <= plante.humidite_max && cellule->pente >= plante.pente_min &&
                            cellule->pente <= plante.pente_max) {
                            nb_plantes_qui_peuvent_survivre++;
                        }
                    }
                    for (Plante plante : plantes) {
                        if (cellule->temperature >= plante.temperature_min && cellule->temperature <= plante.temperature_max &&
                            cellule->humidite >= plante.humidite_min && cellule->humidite <= plante.humidite_max && cellule->pente >= plante.pente_min &&
                            cellule->pente <= plante.pente_max) {
                            scoreTemperature = fabs(cellule->temperature - (plante.temperature_max + plante.temperature_min)); //plus c'est proche de 0 mieux c'est
                            scoreHumidite = fabs(cellule->humidite - (plante.humidite_max + plante.humidite_min)); //plus c'est proche de 0 mieux c'est
                            score = scoreTemperature + scoreHumidite;
                            if (score < bestScore || bestScore == 0) {
                                bestScore = score;
                                bestPlante = plante;
                            }
                        }
                    }
                    if(bestPlante.nom != cellule->plante.nom){
                        cellule->plante.sante -= 4;
                        couleur_sante = (Color){
                            (unsigned char)Clamp(cellule->plante.couleur.r - 10.0f, 0, 255),
                            (unsigned char)Clamp(cellule->plante.couleur.g - 10.0f, 0, 255),
                            (unsigned char)Clamp(cellule->plante.couleur.b - 10.0f, 0, 255),
                            cellule->plante.couleur.a
                        };
                        cellule->plante.couleur = couleur_sante;
                        //printf("santé : %d\n", cellule->plante.sante);
                    }
                    
                    // Augmenter la taille de la plante
                    if (cellule->plante.taille < cellule->plante.taille_max) {
                        cellule->plante.taille *= 1.005f;
                    }
                    return;
                }
            }
        }
    }
    return;
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

int minTemp = 0;
int maxTemp = 10;
//pour la temperature
Color GetTemperatureColor(int temperature, int minTemp, int maxTemp) {
    float normalizedTemp = (float)(temperature - minTemp) / (maxTemp - minTemp);
    
    // Interpolation entre bleu (froid) et rouge (chaud)
    Color coldColor = BLUE;
    Color hotColor = RED;
    
    unsigned char r = (unsigned char)(coldColor.r + (hotColor.r - coldColor.r) * normalizedTemp);
    unsigned char g = (unsigned char)(coldColor.g + (hotColor.g - coldColor.g) * normalizedTemp);
    unsigned char b = (unsigned char)(coldColor.b + (hotColor.b - coldColor.b) * normalizedTemp);
    
    return (Color){r, g, b, 255};
}

int minHum = 0;
int maxHum = 10;
//pour l'humidite
Color GetHumidityColor(int humidity, int minHum, int maxHum) {
    float normalizedHum = (float)(humidity - minHum) / (maxHum - minHum);
    
    // Interpolation entre bleu (sec) et vert (humide)
    Color dryColor = BLUE;
    Color wetColor = GREEN;
    
    unsigned char r = (unsigned char)(dryColor.r + (wetColor.r - dryColor.r) * normalizedHum);
    unsigned char g = (unsigned char)(dryColor.g + (wetColor.g - dryColor.g) * normalizedHum);
    unsigned char b = (unsigned char)(dryColor.b + (wetColor.b - dryColor.b) * normalizedHum);
    
    return (Color){r, g, b, 255};
}

int main(void) {
    // Initialisation
    const int screenWidth = 1280;//1920;
    const int screenHeight = 720;//1080;
    // ecrant de chargement : 0 = menu, 1 = chargement , 2 = jeu
    int currentScreen = 0;
    int loadingStage = 0;
    bool initializationDone = false;
    SetConfigFlags(FLAG_MSAA_4X_HINT); // Enable Multi Sampling Anti Aliasing 4x (if available)

    InitWindow(screenWidth, screenHeight, "raylib - Projet tutore");
    rlDisableBackfaceCulling();//pour voir l'arriere des objets
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
    // Load basic lighting shader_
    Shader shader = LoadShader(TextFormat("include/shaders/resources/shaders/glsl%i/lighting.vs", GLSL_VERSION),TextFormat("include/shaders/resources/shaders/glsl%i/lighting.fs", GLSL_VERSION));
    //les ombres
    Shader shadowShader = LoadShader(TextFormat("include/shaders/resources/shaders/glsl%i/shadowmap.vs", GLSL_VERSION),TextFormat("include/shaders/resources/shaders/glsl%i/shadowmap.fs", GLSL_VERSION));
    //l'herbe
    Shader herbe_shader = LoadShader("ressources/custom_shader/glsl330/herbe_shader.vs","ressources/custom_shader/glsl330/herbe_shader.fs");
    // Configurez les locations du shader de l'ombre
    shadowShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shadowShader, "viewPos");

    //pour l'ombre    
    Vector3 lightDir = Vector3Normalize((Vector3){ 0.35f, -1.0f, -0.35f });
    Color lightColor = WHITE;
    Vector4 lightColorNormalized = ColorNormalize(lightColor);
    int lightDirLoc = GetShaderLocation(shadowShader, "lightDir");
    int lightColLoc = GetShaderLocation(shadowShader, "lightColor");
    SetShaderValue(shadowShader, lightDirLoc, &lightDir, SHADER_UNIFORM_VEC3);
    SetShaderValue(shadowShader, lightColLoc, &lightColorNormalized, SHADER_UNIFORM_VEC4);

    //test TODO
    SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "lightPos"), &lightDir, SHADER_UNIFORM_VEC3);
    SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "lightColor"), &lightColorNormalized, SHADER_UNIFORM_VEC4);
    //SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "ambient"), &lightColorNormalized, SHADER_UNIFORM_VEC4);
    // Valeurs initiales des paramètres de vent
    float time = 0.0f;

    //init bruit de perlin
    Image noiseImage = GenImagePerlinNoise(1024, 1024, 0, 0, 100.0f);
    Texture2D noiseTexture = LoadTextureFromImage(noiseImage);
    UnloadImage(noiseImage);
    
    // Valeurs initiales
    float windTextureTileSize = 2.0f;
    float windVerticalStrength = 0.3f;
    Vector2 windHorizontalDirection = { 1.0f, 0.5f };
    
    // Stocker l'ID de la localisation de l'uniform noiseTexture dans le shader
    int noiseTextureLoc = GetShaderLocation(herbe_shader, "noiseTexture");
    int noiseScaleLoc = GetShaderLocation(herbe_shader, "noiseScale");

    int windTextureTileSizeLocation = GetShaderLocation(herbe_shader, "windTextureTileSize");
    int windVerticalStrengthLocation = GetShaderLocation(herbe_shader, "windVerticalStrength");
    int windHorizontalDirectionLocation = GetShaderLocation(herbe_shader, "windHorizontalDirection");

    int timeLocation = GetShaderLocation(herbe_shader, "time");
    int windStrengthLocation = GetShaderLocation(herbe_shader, "windStrength");
    int windSpeedLocation = GetShaderLocation(herbe_shader, "windSpeed");
    int isGrassLocation = GetShaderLocation(herbe_shader, "isGrass");

    int ambientLoc = GetShaderLocation(shadowShader, "ambient");
    int lightVPLoc = GetShaderLocation(shadowShader, "lightVP");
    int shadowMapLoc = GetShaderLocation(shadowShader, "shadowMap");
    int shadowMapResolution = SHADOWMAP_RESOLUTION;
    SetShaderValue(shadowShader, GetShaderLocation(shadowShader, "shadowMapResolution"), &shadowMapResolution, SHADER_UNIFORM_INT);
    SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "shadowMapResolution"), &shadowMapResolution, SHADER_UNIFORM_INT);
    
    
    //test sol
    Image image_sol = LoadImage("ressources/test.png");     // Load heightmap image (RAM)    
    //image de la temperature
    Image temperatureMap = GenImageColor(GRID_SIZE, GRID_SIZE, WHITE);
    Texture2D temperatureTexture = LoadTextureFromImage(temperatureMap);
    UnloadImage(temperatureMap);

    Mesh mesh_sol = GenMeshHeightmap(image_sol, (Vector3){ 40, 20, 40 }); // Generate heightmap mesh (RAM and VRAM)
    Model model_sol = LoadModelFromMesh(mesh_sol); // Load model from generated mesh
    Image image_texture_sol = LoadImage("ressources/rocky_terrain_02_diff_1k.jpg"); //rocky_terrain_02_diff_1k.jpg
    Texture2D texture_sol = LoadTextureFromImage(image_texture_sol); // Load map texture
    Shader shader_taille = LoadShader("include/shaders/resources/shaders/glsl100/base.vs", "include/shaders/resources/shaders/glsl100/base.fs");
    int uvScaleLoc = GetShaderLocation(shader_taille, "uvScale");
    Vector2 uvScale = {10.0f, 10.0f}; // Plus grand = texture plus petite et répétée
    SetShaderValue(shader_taille, uvScaleLoc, &uvScale, SHADER_UNIFORM_VEC2);

    model_sol.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture_sol; // Set map diffuse texture
    model_sol.materials[0].shader = shader_taille; // Assignez le shader au modèle

    Vector3 mapPosition = { -2.0f, 0.0f, -2.0f };// Define model position

    //fin test sol
    // Charger le modèle et la texture test commentaire
    Model model_mort  = LoadModel("models/arb_mort/scene.gltf");
    Model model_sapin = LoadModel("models/pine_tree/scene.glb");
    Model model_buisson_europe = LoadModel("models/buisson/foret_classique/scene.gltf");
    Texture2D texture_buisson_europe = LoadTexture("models/buisson/foret_classique/textures/gbushy_baseColor.png");
    model_buisson_europe.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture_buisson_europe;

    Model model_herbe = LoadModel("models/herbe/untitled.glb");
    
    //pour l'herbe du sol
    Model model_herbe_instance = LoadModel("models/herbe/lpherbe.glb");

    Model model_acacia = LoadModel("models/acacia/scene.gltf");
    Texture2D texture_acacia = LoadTexture("models/acacia/textures/Acacia_Dry_Green__Mature__Acacia_Leaves_1_baked_Color-Acacia_Dry_Green__Mature__Acacia_Leaves_1_baked_Opacity.png");
    model_acacia.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture_acacia;
    
    //on applique la lumière sur toutes les plantes
    model_sapin.materials[0].shader = shadowShader;
    for (int i = 0; i < model_sapin.materialCount; i++)
    {
        model_sapin.materials[i].shader = shadowShader;
    }
    model_buisson_europe.materials[0].shader = shadowShader;
    for (int i = 0; i < model_buisson_europe.materialCount; i++)
    {
        model_buisson_europe.materials[i].shader = shadowShader;
    }
    
    model_acacia.materials[0].shader = shadowShader;
    for (int i = 0; i < model_acacia.materialCount; i++)
    {
        model_acacia.materials[i].shader = shadowShader;
    }
    model_mort.materials[0].shader = shadowShader;
    for (int i = 0; i < model_mort.materialCount; i++)
    {
        model_mort.materials[i].shader = shadowShader;
    }
    
    model_herbe.materials[0].shader = shadowShader;
    for (int i = 0; i < model_herbe.materialCount; i++)
    {
        model_herbe.materials[i].shader = shadowShader;
    }
    model_herbe_instance.materials[0].shader = herbe_shader;
    for (int i = 0; i < model_herbe_instance.materialCount; i++)
    {
        model_herbe_instance.materials[i].shader = herbe_shader;
    }
    // Après avoir chargé le shader
    if (herbe_shader.id == 0) {
        printf("Erreur lors du chargement du shader d'herbe!\n");
    }

    model_sol.materials[0].shader = shadowShader;
    
    model_sapin.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
    model_buisson_europe.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
    model_acacia.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
    model_sol.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
    model_mort.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
    model_herbe.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
    model_herbe_instance.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
    // Create an empty model to represent an empty cell
    Mesh emptyMesh = GenMeshCube(0.0f, 0.0f, 0.0f); // Generate a cube with zero size
    Model emptyModel = LoadModelFromMesh(emptyMesh);
    emptyModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = BLANK; // Set the color to blank
    //la shadowmap
    RenderTexture2D shadowMap = LoadShadowmapRenderTexture(SHADOWMAP_RESOLUTION, SHADOWMAP_RESOLUTION);
    
    //la light camera
    Camera3D lightCam = { 0 };
    lightCam.position = Vector3Scale(lightDir, -15.0f);
    lightCam.target = Vector3Zero();
    lightCam.projection = CAMERA_ORTHOGRAPHIC;
    lightCam.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    lightCam.fovy = 20.0f;

    //shader pour le billboard
    
    // Création d'une plante
    /*
    string nom;
    int humidite_min;
    int humidite_max;
    int temperature_min;
    int temperature_max;
    int influence_humidite;
    int influence_temperature;
    float taille;
    float taille_max;
    float pente_min;
    float pente_max;
    int age;
    bool morte;
    int age_max;
    Model model;
    */
    Color couleur = WHITE;
    Plante herbe("Herbe", 100, 0, 100, -10, 40, 2, 1, 0.05f, 0.15f, 0.0f, 0.010f, 0, false, 1000, model_herbe, couleur);
    Plante buisson("Buisson", 100, 15, 30, 10 , 30, 3, 1, 0.05f, 0.1f, 0.01f, 0.5f, 0, false, 1000, model_buisson_europe, couleur);
    Plante accacia("Acacia", 100, 10, 20, 10, 30, 2, 1, 0.005f, 0.1f, 0.0f, 0.5f, 0, false, 1000, model_acacia, couleur);
    Plante plante_morte("Morte", 100, 0, 100, -50, 200, 0, 0, 0.000250f, 0.000250f, 0.0f, 1.0f, 0, true, 50, model_mort, couleur);
    Plante sapin("Sapin", 100, 0, 30, -30, 20, 1, 1, 0.005f, 0.1f, 0.0f , 0.3f, 0, false, 1000, model_sapin, couleur);
    Plante vide("Vide", 100, 0, 0, 0, 0, 0, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0, false, 100, emptyModel, couleur);
    std::vector<Plante> plantes = {buisson, accacia, sapin, herbe};
    // Initialisation de la grille
    /*Vector3 position;
    Model model;
    bool active;
    bool occupee;
    int temperature;
    int humidite;
    float pente;
    Plante plante;*/
    // Création d'une grille de cellules
    std::vector<std::vector<GridCell>> grille(GRID_SIZE, std::vector<GridCell>(GRID_SIZE, GridCell(0,{0,0,0}, vide.model, true, false, 20, 50, 0.0f, vide)));
    //ajoute la grille du sol d'herbe type SolHerbe
    //le terrain
    Vector3 taille_terrain = { 4, 2, 4 }; // Taille du terrain
    int humidite_moyenne = 0;
    int temperature_moyenne = 0;
   

    //pour stocker les trucs sur l'herbe
    std::vector<Model> models_herbe_vecteur(NBHERBE * NBHERBE, model_herbe_instance);
    std::vector<Vector3> position_herbe(NBHERBE * NBHERBE);
    
    DisableCursor();// Limit cursor to relative movement inside the window
    int frameCounter = 0;
    SetTargetFPS(60);
    Mesh sphere_test = GenMeshSphere(1.0f, 16, 16);
    Material material_test = LoadMaterialDefault();
    Texture2D argyleTexture = LoadTexture("ressources/argyle.png");
    material_test.maps[MATERIAL_MAP_DIFFUSE].texture = argyleTexture;
    material_test.shader = shadowShader;
    std::vector<Nuage> grandsNuages;
    float cloudThreshold = 0.6f; // Seuil initial
    float noiseScale = 10.0f; // Échelle initiale

    // Dans la boucle principale
    float temperature_modifieur = 0;
    float hum_modifieur = 0;


    //couleur qui va changer en fonction de la santé je la decale ici pour pas la declarer a chaque frame
    Color couleur_sante = WHITE;
    // Boucle principale
    float windStrength = 0.7f;//force du vent
    float windSpeed = 1.0f;//vitesse du vent

    //declaration de la variable chargées
    int herbeCount = 0;
    
    while (!WindowShouldClose()) {
        switch (currentScreen)
        {
            case 0:{
                // Écran de chargement
                if (!initializationDone) {
                    switch (loadingStage) {
                        case 0: {
                            // Initialisation de la grille
                            for (int x = 0; x < GRID_SIZE; x++) {
                                for (int z = 0; z < GRID_SIZE; z++) {                                
                                    float posX = x * (3.0f / GRID_SIZE) - 1.0f;  //espace entre les plantes pour 10 = 0.3f - 1.0f, pour 100 = 0.03f - 1.0f 
                                    float posZ = z * (3.0f / GRID_SIZE) - 1.0f;
                                    // Ajouter une irrégularité aux positions X et Z
                                    float offsetX = random_flottant(-0.1f, 0.1f); // Décalage aléatoire pour X
                                    float offsetZ = random_flottant(-0.1f, 0.1f); // Décalage aléatoire pour Z
                                
                                    posX += offsetX;
                                    posZ += offsetZ;
                                
                                    // Obtenir la hauteur du terrain pour cette cellule
                                    float height = GetHeightFromTerrain((Vector3){ posX, 0.0f, posZ }, image_sol, taille_terrain);
                                
                                    // Générer une température arbitraire pour cette cellule
                                    int temperature = (int) random_flottant(0, 20); // Température aléatoire entre TEMP_MIN et TEMP_MAX
                                    int humidite = (int) random_flottant(0, 30); // Humidité aléatoire entre HUM_MIN et HUM_MAX
                                    humidite_moyenne += humidite;
                                    temperature_moyenne += temperature;//TODO voir si c'est utile
                                    // Calcul des hauteurs des cellules voisines
                                    float heightLeft = GetHeightFromTerrain((Vector3){ posX - 0.3f, 0.0f, posZ }, image_sol, taille_terrain);
                                    float heightRight = GetHeightFromTerrain((Vector3){ posX + 0.3f, 0.0f, posZ }, image_sol, taille_terrain);
                                    float heightUp = GetHeightFromTerrain((Vector3){ posX, 0.0f, posZ - 0.3f }, image_sol, taille_terrain);
                                    float heightDown = GetHeightFromTerrain((Vector3){ posX, 0.0f, posZ + 0.3f }, image_sol, taille_terrain);
                                
                                    // Calcul des variations de hauteur
                                    float deltaLeft = fabs(height - heightLeft);
                                    float deltaRight = fabs(height - heightRight);
                                    float deltaUp = fabs(height - heightUp);
                                    float deltaDown = fabs(height - heightDown);
                                
                                    // Calcul du taux de pente
                                    float penteX = (deltaLeft + deltaRight) / 2.0f;
                                    float penteZ = (deltaUp + deltaDown) / 2.0f;
                                    float tauxPente = sqrt(penteX * penteX + penteZ * penteZ);
                                    //printf("taux de pente : %f\n", tauxPente);
                                
                                    // Vérifier si la cellule est sur une pente
                                    //bool pente = (deltaLeft > PENTE_SEUIL || deltaRight > PENTE_SEUIL || deltaUp > PENTE_SEUIL || deltaDown > PENTE_SEUIL);
                                
                                    // Positionner la cellule en fonction de la hauteur du terrain
                                    grille[x][z].position = (Vector3){ posX, height, posZ };
                                    grille[x][z].active = true;
                                    grille[x][z].occupee = false;
                                    grille[x][z].humidite = humidite;
                                    grille[x][z].temperature = temperature;//random_flottant(0, 30);
                                    grille[x][z].pente = tauxPente;
                                
                                    grille[x][z].plante = vide;
                                    grille[x][z].plante.age = 0;
                                    float taille = grille[x][z].plante.taille;
                                
                                    Matrix transform = MatrixIdentity();
                                
                                    // Appliquer l'échelle pour réduire ou agrandir le modèle
                                    transform = MatrixMultiply(transform, MatrixScale(taille, taille, taille));
                                
                                    float randomRotationX = random_flottant(0.0f, 2.0f * PI); // Rotation aléatoire autour de l'axe X
                                    //convertir en radians
                                    randomRotationX = DEG2RAD * randomRotationX;
                                    transform = MatrixMultiply(transform, MatrixRotateX(randomRotationX));
                                    grille[x][z].model.transform = transform;
                                    grille[x][z].identifiant = x*GRID_SIZE+z;
                                }
                            }
                            loadingStage++;
                        } break;        
                ////ecrant de chargement  Copyright (c) 2021-2025 Ramon Santamaria (@raysan5)
                //frameCounter++;    // Count frames
                //// Wait for 2 seconds (120 frames) before jumping to TITLE screen
                //if (frameCounter > 300)
                //{
                //    currentScreen = 1;
                //}
                        case 1: {
                                for (int x = 0; x < NBHERBE; x++) {
                                    for (int z = 0; z < NBHERBE; z++) {
                                    
                                        float posX = (float) x - taille_terrain.x / 2; 
                                        float posZ = (float) z - taille_terrain.z / 2;
                                    
                                        // Variables d'espacement pour les éléments
                                        float espacementX = 4.0f / NBHERBE; // Espacement entre les éléments sur l'axe X
                                        float espacementZ = 4.0f / NBHERBE; // Espacement entre les éléments sur l'axe Z
                                        // Ajuster les positions avec l'espacement
                                        posX = x * espacementX - taille_terrain.x / 2;
                                        posZ = z * espacementZ - taille_terrain.z / 2;
                                    
                                        float offsetX = random_flottant(-0.1f , 0.1f); // Décalage aléatoire pour X
                                        float offsetZ = random_flottant(-0.1f, 0.1f); // Décalage aléatoire pour Z
                                        posX += offsetX;
                                        posZ += offsetZ;
                                    
                                        float height = GetHeightFromTerrain((Vector3){ posX, 0.0f, posZ }, image_sol, taille_terrain);
                                        // Calcul des hauteurs des cellules voisines
                                        float heightLeft = GetHeightFromTerrain((Vector3){ posX - 0.3f, 0.0f, posZ }, image_sol, taille_terrain);
                                        float heightRight = GetHeightFromTerrain((Vector3){ posX + 0.3f, 0.0f, posZ }, image_sol, taille_terrain);
                                        float heightUp = GetHeightFromTerrain((Vector3){ posX, 0.0f, posZ - 0.3f }, image_sol, taille_terrain);
                                        float heightDown = GetHeightFromTerrain((Vector3){ posX, 0.0f, posZ + 0.3f }, image_sol, taille_terrain);
                                    
                                        // Calcul des variations de hauteur
                                        float deltaLeft = fabs(height - heightLeft);
                                        float deltaRight = fabs(height - heightRight);
                                        float deltaUp = fabs(height - heightUp);
                                        float deltaDown = fabs(height - heightDown);
                                    
                                        // Calcul du taux de pente
                                        float penteX = (deltaLeft + deltaRight) / 2.0f;
                                        float penteZ = (deltaUp + deltaDown) / 2.0f;
                                        float tauxPente = sqrt(penteX * penteX + penteZ * penteZ);
                                    
                                        if (tauxPente > 0.2f) {
                                            continue; // Skip this position if the slope is too steep
                                        }else{
                                            // Appliquer une rotation aléatoire autour de l'axe Y
                                            float randomRotationY = random_flottant(0.0f, 2.0f * PI);
                                        
                                            // Créer une copie du modèle AVANT de modifier transform
                                            Model modelHerbe = model_herbe_instance;
                                        
                                            // Appliquer la transformation
                                            Matrix transform = MatrixIdentity();
                                            //transform = MatrixMultiply(transform, MatrixTranslate(posX, height, posZ));
                                            transform = MatrixMultiply(transform, MatrixRotateY(randomRotationY));
                                        
                                            modelHerbe.transform = transform; // Appliquer la transformation à la copie
                                        
                                            // Stocker dans le vecteur
                                            models_herbe_vecteur[herbeCount] = modelHerbe;
                                            position_herbe[herbeCount] = (Vector3){ posX, height-0.0f, posZ };
                                        
                                            herbeCount++;
                                        }
                                    
                                    }
                                }
                            loadingStage++;
                        } break;
                        case 2:{
                            grandsNuages.push_back(GenererGrandNuage({-taille_terrain.x, 4.0f, 0.0f}, taille_terrain.x * 3.0f, taille_terrain.x * 3.0f, 1, cloudThreshold, noiseScale));
                            grandsNuages.push_back(GenererGrandNuage({-taille_terrain.x, 3.0f, 0.0f}, taille_terrain.x * 3.0f, taille_terrain.x * 3.0f, 1, cloudThreshold, noiseScale));
                            grandsNuages.push_back(GenererGrandNuage({-taille_terrain.x, 2.0f, 0.0f}, taille_terrain.x * 3.0f, taille_terrain.x * 3.0f, 1, cloudThreshold, noiseScale));                        
                            loadingStage++;
                            initializationDone = true;
                        }break;
                    }
                }
                frameCounter++;
    
                // Afficher les messages de chargement avec le pourcentage
                int loadingPercentage = (loadingStage * 100) / 4; // 4 étapes au total
                        
                BeginDrawing();
                ClearBackground(RAYWHITE);
                DrawText("CHARGEMENT", GetScreenWidth() / 2 - MeasureText("CHARGEMENT", 40) / 2, GetScreenHeight() / 2 - 60, 40, RAYWHITE);
                DrawText(TextFormat("%d%%", loadingPercentage), GetScreenWidth() / 2 - MeasureText("100%", 20) / 2, GetScreenHeight() / 2, 20, RAYWHITE);
                DrawText(TextFormat("Étape %d/4", loadingStage + 1), GetScreenWidth() / 2 - MeasureText("Étape 4/4", 20) / 2, GetScreenHeight() / 2 + 30, 20, DARKGRAY);
                EndDrawing();
                        
                // Passage à l'écran principal uniquement quand tout est initialisé
                if (initializationDone && frameCounter > 120) {
                    currentScreen = 1;
                }
            } break;
            case 1:{
            // Appliquer temperature_modifieur à toutes les cases une seule fois
            static int last_temp_modif = 0; // Stocker la dernière valeur appliquée
            int temp_modif = (int)temperature_modifieur;

            if (temp_modif != last_temp_modif) { // Vérifier si la valeur a changé
                int delta = temp_modif - last_temp_modif; // Calculer la différence
                for (int x = 0; x < GRID_SIZE; x++) {
                for (int z = 0; z < GRID_SIZE; z++) {
                    grille[x][z].temperature += delta; // Modifier la température de chaque case
                    if (grille[x][z].temperature < minTemp) minTemp = grille[x][z].temperature;
                    if (grille[x][z].temperature > maxTemp) maxTemp = grille[x][z].temperature;
                }
                }
                last_temp_modif = temp_modif; // Mettre à jour la dernière valeur appliquée
            }

            static int last_hum_modif = 0; // Stocker la dernière valeur appliquée
            int hum_modif = (int)hum_modifieur;

            if (hum_modif != last_hum_modif) { // Vérifier si la valeur a changé
                int delta = hum_modif - last_hum_modif; // Calculer la différence
                for (int x = 0; x < GRID_SIZE; x++) {
                    for (int z = 0; z < GRID_SIZE; z++) {
                        grille[x][z].humidite += delta; // Modifier l'humidite de chaque case

                        //comme ça l'humidité reste dans la plage 0-100
                        grille[x][z].humidite = Clamp(grille[x][z].humidite, 0, 100);
                    
                        if (grille[x][z].humidite < minHum) minHum = grille[x][z].humidite;
                        if (grille[x][z].humidite > maxHum) maxHum = grille[x][z].humidite;
                    }
                }
                last_hum_modif = hum_modif; // Mettre à jour la dernière valeur appliquée
            }

            float dt = GetFrameTime();

            static float accumulatedTime = 0.0f;
            accumulatedTime += dt * 0.5f; // Contrôle la vitesse d'animation des nuages
            float timeValue = accumulatedTime;

            const float driftSpeed = 0.2f; // Vitesse de déplacement des nuages

            // Faire dériver les nuages horizontalement
            for (auto& nuage : grandsNuages) {
                // Distance parcourue depuis la position initiale
                static float distanceParcourue = 0.0f;
                float distanceDeReset = taille_terrain.x * 3.0f; // La largeur du nuage

                // Déplacer le nuage
                for (size_t i = 0; i < nuage.positions.size(); i++) {
                    nuage.positions[i].x += dt * nuage.vitesseDefile;
                }

                // Mettre à jour la distance parcourue
                distanceParcourue += dt * nuage.vitesseDefile;

                // Vérifier si le nuage a parcouru sa propre largeur
                if (distanceParcourue >= distanceDeReset) {
                    // Réinitialiser la position du nuage
                    for (size_t i = 0; i < nuage.positions.size(); i++) {
                        nuage.positions[i].x = -taille_terrain.x; // Position initiale
                    }
                    // Réinitialiser le compteur de distance
                    distanceParcourue = 0.0f;
                }
            }
            // Update the shader with the camera view vector (points towards { 0.0f, 0.0f, 0.0f })
            Vector3 cameraPos = camera.position;
            SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], &cameraPos, SHADER_UNIFORM_VEC3);
            SetShaderValue(herbe_shader, shader.locs[SHADER_LOC_VECTOR_VIEW], &cameraPos, SHADER_UNIFORM_VEC3);
            //pour la temperature
            if (IsKeyPressed(KEY_T)) {
                viewMode = (viewMode == MODE_NORMAL) ? MODE_TEMPERATURE : MODE_NORMAL;

                // Changer la texture du sol en fonction du mode
                if (viewMode == MODE_TEMPERATURE) {
                    // Mode température : mettre à jour la texture de température
                    Image tempImage = GenImageColor(GRID_SIZE, GRID_SIZE, WHITE);
                    for (int x = 0; x < GRID_SIZE; x++) {
                        for (int z = 0; z < GRID_SIZE; z++) {
                            Color tempColor = GetTemperatureColor(grille[x][z].temperature, minTemp, maxTemp);
                            ImageDrawPixel(&tempImage, x, z, tempColor);
                        }
                    }
                    UpdateTexture(temperatureTexture, tempImage.data);
                    UnloadImage(tempImage);
                    model_sol.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = temperatureTexture;
                } else {
                    // Mode normal : remettre la texture normale
                    model_sol.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture_sol;
                }
            }
            if (IsKeyPressed(KEY_Y)) {
                viewMode = (viewMode == MODE_NORMAL) ? MODE_HUMIDITE : MODE_NORMAL;

                // Changer la texture du sol en fonction du mode
                if (viewMode == MODE_HUMIDITE) {
                    // Mode humidité : mettre à jour la texture d'humidité
                    Image humImage = GenImageColor(GRID_SIZE, GRID_SIZE, WHITE);
                    for (int x = 0; x < GRID_SIZE; x++) {
                        for (int z = 0; z < GRID_SIZE; z++) {
                            Color humColor = GetHumidityColor(grille[x][z].humidite, minHum, maxHum);
                            ImageDrawPixel(&humImage, x, z, humColor);
                        }
                    }
                    UpdateTexture(temperatureTexture, humImage.data);
                    UnloadImage(humImage);
                    model_sol.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = temperatureTexture;
                } else {
                    // Mode normal : remettre la texture normale
                    model_sol.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture_sol;
                }
            }
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
            distance_cam -= GetMouseWheelMove() * 0.5f; // Ajustez le facteur (0.5f) pour contrôler la sensibilité du zoom
            if (distance_cam < 2.0f) distance_cam = 2.0f;   // Distance minimale
            if (distance_cam > 200.0f) distance_cam = 200.0f; // Distance maximale


            // Limiter les angles X pour éviter une rotation complète
            if (angleX > 89.0f) angleX = 89.0f;
            if (angleX < -89.0f) angleX = -89.0f;

            // Calcul de la position de la caméra en coordonnées sphériques
            float radAngleX = DEG2RAD * angleX;
            float radAngleY = DEG2RAD * angleY;

            camera.position.x = distance_cam * cos(radAngleX) * sin(radAngleY);
            camera.position.y = distance_cam * sin(radAngleX);
            camera.position.z = distance_cam * cos(radAngleX) * cos(radAngleY);

            //DisableCursor();//pour pas avoir le curseur qui sort de l'ecran
            ShowCursor();//pour voir le curseur

            // Mise à jour des cellules
            for (int x = 0; x < GRID_SIZE; x++) {
                for (int z = 0; z < GRID_SIZE; z++) {
                    verifier_plante(grille, &grille[x][z], plantes, plante_morte, vide, minTemp, maxTemp, minHum, maxHum, couleur_sante);
                }
            }

            if (viewMode == MODE_TEMPERATURE) {
                Image tempImage = GenImageColor(GRID_SIZE, GRID_SIZE, WHITE);

                //l'image avec les couleurs de température
                for (int x = 0; x < GRID_SIZE; x++) {
                    for (int z = 0; z < GRID_SIZE; z++) {
                        Color tempColor = GetTemperatureColor(grille[x][z].temperature, minTemp, maxTemp);
                        ImageDrawPixel(&tempImage, x, z, tempColor);
                    }
                }

                //maj de la texture
                UpdateTexture(temperatureTexture, tempImage.data);
                UnloadImage(tempImage);

                //applique la texture de température au sol
                if (viewMode == MODE_TEMPERATURE) {
                    model_sol.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = temperatureTexture;
                } else {
                    model_sol.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture_sol;
                }
            }
            //petite actualisation de l'humidite
            if (viewMode == MODE_HUMIDITE) {
                Image humImage = GenImageColor(GRID_SIZE, GRID_SIZE, WHITE);

                //maj l'image avec les couleurs d'humidité
                for (int x = 0; x < GRID_SIZE; x++) {
                    for (int z = 0; z < GRID_SIZE; z++) {
                        Color humColor = GetHumidityColor(grille[x][z].humidite, minHum, maxHum);
                        ImageDrawPixel(&humImage, x, z, humColor);
                    }
                }

                //maj la texture
                UpdateTexture(temperatureTexture, humImage.data);
                UnloadImage(humImage);

                //applique la texture d'humidité au sol
                if (viewMode == MODE_HUMIDITE) {
                    model_sol.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = temperatureTexture;
                } else {
                    model_sol.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture_sol;
                }
            }

            float sunAngle = ((timeOfDay - 6.0f) / 12.0f) * PI; // -PI/2 à PI/2 (6h à 18h)

            //calcul de la direction de la lumière (normalisée)
            Vector3 lightDir = {
                cosf(sunAngle),           // X: Est-Ouest
                -sinf(sunAngle),          // Y: Hauteur du soleil
                0.0f                      // Z: Nord-Sud
            };
            lightDir = Vector3Normalize(lightDir);

            //on bouge aussi la camera de lumière
            lightCam.position = Vector3Scale(lightDir, -15.0f);
            lightCam.target = Vector3Zero();

            // l'intensité de la lumière en fonction de l'heure
            float lightIntensity = 1.0f;
            if (timeOfDay < 6.0f || timeOfDay > 18.0f) {
                lightIntensity = 0.0f; // Nuit
            } else if (timeOfDay < 8.0f || timeOfDay > 16.0f) {
                lightIntensity = 0.6f; // Lever/Coucher du soleil
            }

            lightColor = GetSunColor(timeOfDay);  // Utilisez votre fonction existante
            lightColorNormalized = ColorNormalize(lightColor);

            SetShaderValue(shadowShader, lightColLoc, &lightColorNormalized, SHADER_UNIFORM_VEC4);
            SetShaderValue(shadowShader, lightDirLoc, &lightDir, SHADER_UNIFORM_VEC3);

            //test temp TODO
            SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "lightPos"), &lightDir, SHADER_UNIFORM_VEC3);
            SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "lightColor"), &lightColorNormalized, SHADER_UNIFORM_VEC4);

            SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "lightDir"), &lightDir, SHADER_UNIFORM_VEC3);
            time += dt;  // Incrémenter le temps

            SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "time"), &time, SHADER_UNIFORM_FLOAT);

            SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "windStrength"), &windStrength, SHADER_UNIFORM_FLOAT);
            SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "windSpeed"), &windSpeed, SHADER_UNIFORM_FLOAT);
            SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "noiseScale"), &noiseScale, SHADER_UNIFORM_FLOAT);
            SetShaderValueTexture(herbe_shader, GetShaderLocation(herbe_shader, "noiseTexture"), noiseTexture);
            } break;
            default: break;
        }
        // Rendu final (vue normale)
        BeginDrawing();
        switch(currentScreen)
            {
                case 0:
                {
                }break;
                case 1:
                {
        //on dessine les ombres ici
        Matrix lightView;
        Matrix lightProj;
        BeginTextureMode(shadowMap);
        ClearBackground(SKYBLUE);

        BeginMode3D(lightCam);
            lightView = rlGetMatrixModelview();
            lightProj = rlGetMatrixProjection();
            dessine_scene(camera, image_sol, taille_terrain, model_sol, model_buisson_europe, model_acacia, model_sapin, model_mort, emptyModel, buisson, accacia, sapin, plante_morte, herbe, plantes, grille, viewMode, minTemp, maxTemp, minHum, maxHum, mapPosition);
            int isGrass = 1;

            SetShaderValue(herbe_shader, timeLocation, &time, SHADER_UNIFORM_FLOAT);
            SetShaderValue(herbe_shader, windStrengthLocation, &windStrength, SHADER_UNIFORM_FLOAT);
            SetShaderValue(herbe_shader, windSpeedLocation, &windSpeed, SHADER_UNIFORM_FLOAT);
            SetShaderValue(herbe_shader, isGrassLocation, &isGrass, SHADER_UNIFORM_INT);
            SetShaderValue(herbe_shader, windTextureTileSizeLocation, &windTextureTileSize, SHADER_UNIFORM_FLOAT);
            SetShaderValue(herbe_shader, windVerticalStrengthLocation, &windVerticalStrength, SHADER_UNIFORM_FLOAT);
            SetShaderValueTexture(herbe_shader, noiseTextureLoc, noiseTexture);
            SetShaderValue(herbe_shader, windHorizontalDirectionLocation, &windHorizontalDirection, SHADER_UNIFORM_VEC2);
            SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "isGrass"), &isGrass, SHADER_UNIFORM_INT);
            SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "time"), &time, SHADER_UNIFORM_FLOAT);

            //maj des uniformes pour le shader
            SetShaderValue(herbe_shader, noiseScaleLoc, &noiseScale, SHADER_UNIFORM_FLOAT);

            rlEnableBackfaceCulling();
            //DrawMesh(sphere_test, material_test, MatrixTranslate(0.0f, 2.0f, 0.0f));
            for (int i = 0; i < herbeCount ; i++){
          
                //regarde si la cellule de la grille correspond à la position de l'herbe
                int gridX = (int)((position_herbe[i].x + taille_terrain.x / 2) * GRID_SIZE / taille_terrain.x);
                int gridZ = (int)((position_herbe[i].z + taille_terrain.z / 2) * GRID_SIZE / taille_terrain.z);

                //Clamp les coordonnées de grille pour éviter les débordements
                gridX = Clamp(gridX, 0, GRID_SIZE - 1);
                gridZ = Clamp(gridZ, 0, GRID_SIZE - 1);

                //verifie si l'herbe peut pousser à cette temperature
                if (grille[gridX][gridZ].temperature > -20 && grille[gridX][gridZ].temperature < 50) {
                    Shader originalShader = models_herbe_vecteur[i].materials[0].shader;
            
                    // Utilisez un shader de shadow mapping simple pour générer l'ombre
                    models_herbe_vecteur[i].materials[0].shader = shadowShader; // Utilisez un shader simple pour les ombres
                    // Restaurez le shader original
                    DrawModel(models_herbe_vecteur[i], position_herbe[i], 0.05f, WHITE);
                    models_herbe_vecteur[i].materials[0].shader = originalShader;
                }
            }
            isGrass = 0;
            SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "isGrass"), &isGrass, SHADER_UNIFORM_INT);
            rlDisableBackfaceCulling();
            // Après avoir généré les nuages
            for (auto& nuage : grandsNuages) {
                for (size_t i = 0; i < nuage.plans.size(); i++) {
                    //config le matériau pour la transparence
                    nuage.plans[i].materials[0].shader = shadowShader;
                    nuage.plans[i].materials[0].maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
                }
            }
        
            // Dans la boucle principale, lors du rendu
            rlEnableColorBlend();
            for (auto& nuage : grandsNuages) {
                for (size_t i = 0; i < nuage.plans.size(); i++) {
                    DrawModelEx(nuage.plans[i], nuage.positions[i], (Vector3){0, 1, 0}, nuage.rotations[i], (Vector3){nuage.scales[i], nuage.scales[i], nuage.scales[i]}, WHITE);
                }
            }
            //DrawModelEx(grandsNuages[0].plans[0], grandsNuages[0].positions[0], (Vector3){0, 1, 0}, grandsNuages[0].rotations[0], (Vector3){grandsNuages[0].scales[0], grandsNuages[0].scales[0], grandsNuages[0].scales[0]}, WHITE);
            
        EndMode3D();
        EndTextureMode();
        Matrix lightViewProj = MatrixMultiply(lightView, lightProj);

        ClearBackground(SKYBLUE);

        SetShaderValueMatrix(shadowShader, lightVPLoc, lightViewProj);

        rlEnableShader(shadowShader.id);

        int slot = 10;
        rlActiveTextureSlot(10);
        rlEnableTexture(shadowMap.depth.id);
        rlSetUniform(shadowMapLoc, &slot, SHADER_UNIFORM_INT, 1);
        //Vector3 lightPos = { 0.0f, 10.0f, 0.0f }; // Ajustez selon votre lumière
        //Vector4 lightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
        //Vector4 ambientColor = { 1.0f, 1.0f, 1.0f, 1.0f };
        
        BeginMode3D(camera);
            dessine_scene(camera, image_sol, taille_terrain, model_sol, model_buisson_europe, model_acacia, model_sapin, model_mort, emptyModel, buisson, accacia, sapin, plante_morte, herbe, plantes, grille, viewMode, minTemp, maxTemp, minHum, maxHum, mapPosition);
            isGrass = 1;
            SetShaderValue(herbe_shader, timeLocation, &time, SHADER_UNIFORM_FLOAT);
            SetShaderValue(herbe_shader, windStrengthLocation, &windStrength, SHADER_UNIFORM_FLOAT);
            SetShaderValue(herbe_shader, windSpeedLocation, &windSpeed, SHADER_UNIFORM_FLOAT);
            SetShaderValue(herbe_shader, isGrassLocation, &isGrass, SHADER_UNIFORM_INT);
            SetShaderValue(herbe_shader, windTextureTileSizeLocation, &windTextureTileSize, SHADER_UNIFORM_FLOAT);
            SetShaderValue(herbe_shader, windVerticalStrengthLocation, &windVerticalStrength, SHADER_UNIFORM_FLOAT);
            SetShaderValueTexture(herbe_shader, noiseTextureLoc, noiseTexture);
            SetShaderValue(herbe_shader, windHorizontalDirectionLocation, &windHorizontalDirection, SHADER_UNIFORM_VEC2);
            SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "isGrass"), &isGrass, SHADER_UNIFORM_INT);
            SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "time"), &time, SHADER_UNIFORM_FLOAT);
            //maj des uniformes pour le shader
            SetShaderValue(herbe_shader, noiseScaleLoc, &noiseScale, SHADER_UNIFORM_FLOAT);


            for (int i = 0; i < herbeCount ; i++){
                //active backface culling ici
                int gridX = (int)((position_herbe[i].x + taille_terrain.x / 2) * GRID_SIZE / taille_terrain.x);
                int gridZ = (int)((position_herbe[i].z + taille_terrain.z / 2) * GRID_SIZE / taille_terrain.z);

                gridX = Clamp(gridX, 0, GRID_SIZE - 1);
                gridZ = Clamp(gridZ, 0, GRID_SIZE - 1);

                //verifie si l'herbe peut pousser à cette temperature
                if (grille[gridX][gridZ].temperature > -20 && grille[gridX][gridZ].temperature < 50) {
    
                    DrawModel(models_herbe_vecteur[i], position_herbe[i], 0.05f, WHITE);
                }
//                Vector3 lightPos = { 0.0f, 10.0f, 0.0f }; // Ajustez selon votre lumière
//                    Vector4 lightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
//                    Vector4 ambientColor = { 0.2f, 0.2f, 0.2f, 1.0f };
                    
            }
            isGrass = 0;
            SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "isGrass"), &isGrass, SHADER_UNIFORM_INT);
            rlDisableBackfaceCulling();
            
           //dessine le grand nuage
           for (auto& nuage : grandsNuages) {
            for (size_t i = 0; i < nuage.plans.size(); i++) {
                DrawModelEx(nuage.plans[i], nuage.positions[i], (Vector3){0, 1, 0}, nuage.rotations[i], (Vector3){nuage.scales[i], nuage.scales[i], nuage.scales[i]}, WHITE);
            }
        }
           //DrawModelEx(grandsNuages[0].plans[0], grandsNuages[0].positions[0], (Vector3){0, 1, 0}, grandsNuages[0].rotations[0], (Vector3){grandsNuages[0].scales[0], grandsNuages[0].scales[0], grandsNuages[0].scales[0]}, lightColor);
           
        EndMode3D();
        DrawGrid(20, 1.0f);
        EndMode3D();
        // Ajouter une légende pour le mode température
        if (viewMode == MODE_TEMPERATURE) {
            DrawText("Mode température - Appuyez sur T pour revenir", 10, 60, 20, BLACK);
            // Optionnel : afficher une échelle de température
            DrawText(TextFormat("Min: %d°C", minTemp), 10, 80, 20, BLUE);
            DrawText(TextFormat("Max: %d°C", maxTemp), 10, 100, 20, RED);
        }
        if (viewMode == MODE_HUMIDITE) {
            DrawText("Mode humidite - Appuyez sur Y pour revenir", 10, 60, 20, BLACK);
            // Optionnel : afficher une échelle de température
            DrawText(TextFormat("Min: %d", minHum), 10, 80, 20, BLUE);
            DrawText(TextFormat("Max: %d", maxHum), 10, 100, 20, RED);
        }
        DrawText(" d'objets 3D - Utilisez la souris pour naviguer", 10, 10, 20, DARKGRAY);
        DrawText("Maintenez le clic droit pour tourner la scène", 10, 25, 20, DARKGRAY);
        GuiSliderBar((Rectangle){ 100, 190, 200, 20 }, "Noise Scale", TextFormat("%.2f", noiseScale), &noiseScale, 1.0f, 20.0f);
        GuiSliderBar((Rectangle){ 100, 220, 200, 20 }, "Cloud Threshold", TextFormat("%.2f", cloudThreshold), &cloudThreshold, 0.0f, 1.5f);
        GuiSliderBar((Rectangle){ 100, 250, 200, 20 }, "Temperature", TextFormat("%d", temperature_modifieur), (float*)&temperature_modifieur, -30.0, 30.0);
        GuiSliderBar((Rectangle){ 100, 280, 200, 20 }, "Humidite", TextFormat("%d", hum_modifieur), &hum_modifieur, 0.0f, 100.0f);
        // Sliders for wind parameters
        GuiSliderBar((Rectangle){ 100, 310, 200, 20 }, "Wind Speed", TextFormat("%.2f", windSpeed), &windSpeed, 0.0f, 3.0f);
        GuiSliderBar((Rectangle){ 100, 340, 200, 20 }, "Wind Strength", TextFormat("%.2f", windStrength), &windStrength, 0.0f, 2.0f);
        // Si l'un des paramètres change, régénérer la texture
        static float lastCloudThreshold = cloudThreshold;
        static float lastNoiseScale = noiseScale;
        if (fabsf(cloudThreshold - lastCloudThreshold) > 0.01f || fabsf(noiseScale - lastNoiseScale) > 0.1f) {
            // Régénérer la texture avec les nouveaux paramètres

            // Update all cloud textures when parameters change
            for (auto& nuage : grandsNuages) {
                for (size_t i = 0; i < nuage.textures.size(); i++) {
                    UnloadTexture(nuage.textures[i]);
                    nuage.textures[i] = GenererTextureNuage(256, 256, GetRandomValue(0, 1000), cloudThreshold, noiseScale);
                    nuage.plans[i].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = nuage.textures[i];
                }
            }
            lastCloudThreshold = cloudThreshold;
            lastNoiseScale = noiseScale;
        }
        DrawFPS(10, 40);

        

        /*
        l'ui pour controler les paramètres
        */
        // Pour changer la direction de la lumière
        GuiSliderBar((Rectangle){ 100, 100, 200, 20 }, "Time of Day", TextFormat("%.0f:00", timeOfDay), &timeOfDay, 0.0f, 24.0f);

        // Affichage de l'heure
        DrawText(TextFormat("Time: %.0f:00", timeOfDay), 310, 10, 20, DARKGRAY);
            } break;
        default: break;
        }
        EndDrawing();
    }

    UnloadShader(shader);
    UnloadShader(shadowShader);
    UnloadShader(shader_taille);
    // Désallocation des ressources
    UnloadModel(model_mort);
    UnloadModel(model_sapin);
    //UnloadTexture(texture_sapin);
    UnloadModel(model_buisson_europe);
    UnloadTexture(texture_buisson_europe);
    UnloadModel(model_acacia);
    UnloadTexture(texture_acacia);
    UnloadModel(model_sol);
    UnloadTexture(texture_sol);
    UnloadTexture(temperatureTexture);
    UnloadShadowmapRenderTexture(shadowMap);
    models_herbe_vecteur.clear();
    position_herbe.clear();

    // Clear the memory of other resources
    UnloadModel(model_herbe_instance);
    printf("model herbe unload\n");
    UnloadModel(model_herbe);
    printf("model herbe unload\n");
    UnloadImage(image_sol);
    printf("image sol unload\n");
    UnloadImage(image_texture_sol);
    printf("image texture sol unload\n");
    CloseWindow();

    return 0;
}

void dessine_scene(Camera camera, Image image_sol, Vector3 taille_terrain, Model model_sol, Model model_buisson_europe, Model model_acacia, Model model_sapin, Model model_mort, Model emptyModel, Plante buisson, Plante accacia, Plante sapin, Plante plante_morte, Plante herbe, std::vector<Plante> plantes, std::vector<std::vector<GridCell>> grille, int viewMode, int &minTemp, int &maxTemp, int &minHum, int &maxHum, Vector3 mapPosition) {
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
            if (grille[x][z].active) {
                sceneObjects[objectCount].position = grille[x][z].position;
                sceneObjects[objectCount].model = &grille[x][z].plante.model;
                float scale = grille[x][z].plante.taille;
                sceneObjects[objectCount].model->transform = MatrixScale(scale, scale, scale);
                sceneObjects[objectCount].depth = Vector3Distance(camera.position, grille[x][z].position);
                sceneObjects[objectCount].color = grille[x][z].plante.couleur;
                objectCount++;
            }
        }
    }
    // update des températures min et max
    minTemp = 0;
    maxTemp = 1;
    for (int x = 0; x < GRID_SIZE; x++) {
        for (int z = 0; z < GRID_SIZE; z++) {
            if (grille[x][z].temperature < minTemp) minTemp = grille[x][z].temperature;
            if (grille[x][z].temperature > maxTemp) maxTemp = grille[x][z].temperature;
            // Update minHum and maxHum
            grille[x][z].humidite = Clamp(grille[x][z].humidite, 0, 100);
            if (grille[x][z].humidite < minHum) minHum = grille[x][z].humidite;
            if (grille[x][z].humidite > maxHum) maxHum = grille[x][z].humidite;
        }
    }
    // Trier les objets par profondeur
    qsort(sceneObjects, objectCount, sizeof(SceneObject), CompareSceneObjects);
    // Dessiner les objets dans l'ordre trié
    for (int i = 0; i < objectCount; i++) {
        if (viewMode == MODE_NORMAL) {
            if (sceneObjects[i].model == &model_sol) {
                //DrawCubeV((Vector3){ mapPosition.x, mapPosition.y - 0.1f, mapPosition.z }, taille_terrain, GRAY);
                //DrawCubeV((Vector3){0,0,0 }, (Vector3){taille_terrain.x, 0.2f, taille_terrain.z}, GRAY);
                DrawModel(*sceneObjects[i].model, sceneObjects[i].position, 0.1f, WHITE);
            } else {
                DrawModel(*sceneObjects[i].model, sceneObjects[i].position, 1.0f, sceneObjects[i].color);
                //printf("couleur : R=%d, G=%d, B=%d, A=%d\n", sceneObjects[i].color.r, sceneObjects[i].color.g, sceneObjects[i].color.b, sceneObjects[i].color.a);
            }
        } else if (viewMode == MODE_TEMPERATURE) {
            if (sceneObjects[i].model == &model_sol) {
                // Le sol utilise déjà la texture de température
                //DrawCubeV((Vector3){0,0,0 }, (Vector3){taille_terrain.x, 0.2f, taille_terrain.z}, GRAY);
                DrawModel(*sceneObjects[i].model, sceneObjects[i].position, 0.1f, WHITE);
            } else {
                // Pour les autres objets, utilisez la couleur de température
                for (int x = 0; x < GRID_SIZE; x++) {
                    for (int z = 0; z < GRID_SIZE; z++) {
                        if (Vector3Equals(grille[x][z].position, sceneObjects[i].position)) {
                            Color tempColor = GetTemperatureColor(grille[x][z].temperature, minTemp, maxTemp);
                            DrawModel(*sceneObjects[i].model, sceneObjects[i].position, 1.0f, tempColor);
                            break;
                        }
                    }
                }
            }
        } else if(viewMode == MODE_HUMIDITE){
            if (sceneObjects[i].model == &model_sol) {
                // Le sol utilise déjà la texture d'humidite
                //DrawCubeV((Vector3){0,0,0 }, (Vector3){taille_terrain.x, 0.2f, taille_terrain.z}, GRAY);
                DrawModel(*sceneObjects[i].model, sceneObjects[i].position, 0.1f, WHITE);
            } else {
                // Pour les autres objets, utilisez la couleur de l'humiité
                for (int x = 0; x < GRID_SIZE; x++) {
                    for (int z = 0; z < GRID_SIZE; z++) {
                        if (Vector3Equals(grille[x][z].position, sceneObjects[i].position)) {
                            Color humColor = GetHumidityColor(grille[x][z].humidite, minHum, maxHum);
                            DrawModel(*sceneObjects[i].model, sceneObjects[i].position, 1.0f, humColor);
                            break;
                        }
                    }
                }
            }
        }
    }
}