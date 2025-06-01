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
#include <chrono>

#include "sol.h"
#include "nuages.h"
#include "biome.h"
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

#define GRID_SIZE 30
#define SHADOWMAP_RESOLUTION 2048 //la resolution de la shadowmap

//pour la vitesse de simulation
float simulationSpeed = 1.0f;//facteur de vitesse de simulation (1.0 = vitesse normale)

// Génère un float aléatoire entre 0.0 et 1.0
float frand() {
    return (float)GetRandomValue(0, 10000) / 10000.0f;
}

float random_flottant(float min, float max) {
    return min + (rand() / (float)RAND_MAX) * (max - min);
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

//eau
#define GOUTE_PLUIE 1000 //nombre de gouttes de pluie
bool pleut = false;
float frequence_pluie = 0.0f;
float random_pluie = 1.0f;
//ça serait cool si ça influence la pluviométrie genre on a X quantitée de pluie et plus on en a plus ça change le biome
typedef struct PluieQuad {
    Vector3 position;
    float rotationX;      // Rotation autour de l'axe X (en radians)
    float rotationY;      // Rotation autour de l'axe Y (en radians)
    Vector2 size;         // Largeur / Hauteur du quad
    Vector3 velocite; //pour faire bouger la particule pour la pluie
    Color color;
} PluieQuad;

PluieQuad la_pluie[GOUTE_PLUIE];

void InitPluieParticules(Vector3 taille_terrain, Image image_sol){
for (int i = 0; i < GOUTE_PLUIE; i++) {
        //pour faire apparaitre la pluie
        float posX = frand() * taille_terrain.x - taille_terrain.x / 2;
        float posY = frand() * taille_terrain.y;
        float posZ = frand() * taille_terrain.z - taille_terrain.z / 2;
        //petit décalage random pour une distrib plus naturelle
        float offsetX = random_flottant(-0.1f, 0.1f);
        float offsetZ = random_flottant(-0.1f, 0.1f);
        posX += offsetX;
        posZ += offsetZ;
        //printf("herbe%d: (%f, %f)\n", i, posX, posZ);
        //recup la hauteur du terrain
        float height = GetHeightFromTerrain((Vector3){ posX, 0.0f, posZ }, image_sol, taille_terrain);
        
        //on set la position de l'herbe avec la hauteur calculée
        la_pluie[i].size = (Vector2){0.01f + frand() * 0.02f, 0.08f + frand() * 0.03f};
        la_pluie[i].position = (Vector3){ posX, height+la_pluie[i].size.y , posZ };
        la_pluie[i].rotationY = frand() * PI * 2.0f;
        la_pluie[i].velocite.y = -1.0f * (frand() * 0.50f);
        //grass[i].rotationX = random_flottant(-0.2f, 0.2f);
        
        float teinte = random_flottant(0.8f, 1.0f);
        la_pluie[i].color = (Color){
            (unsigned char)(6 * teinte),
            (unsigned char)(101 * teinte),
            (unsigned char)(224 * teinte),
            (unsigned char)(127 * teinte) //faut faire varier ça
        };
    }
}


// Dessine un quad à partir d’une position centrale, taille, et rotation X
void DrawPluieQuad(PluieQuad &p, Image terrainImage, Vector3 taille_terrain, bool pleut, float delta){
    
    if (pleut) {
        p.position.y += p.velocite.y;// * delta * simulationSpeed; voir si on peut ralentir la pluie
    }

    float w = p.size.x * 0.5f;
    float h = p.size.y;

    // Vecteurs du quad avant rotation
    Vector3 topLeft     = (Vector3){ -w, 0.0f, 0.0f };
    Vector3 topRight    = (Vector3){  w, 0.0f, 0.0f };
    Vector3 bottomRight = (Vector3){  w, -h, 0.0f };
    Vector3 bottomLeft  = (Vector3){ -w, -h, 0.0f };

    Matrix rot = MatrixRotateY(p.rotationY);
    topLeft     = Vector3Transform(topLeft, rot);
    topRight    = Vector3Transform(topRight, rot);
    bottomRight = Vector3Transform(bottomRight, rot);
    bottomLeft  = Vector3Transform(bottomLeft, rot);

    // Applique la position
    topLeft     = Vector3Add(topLeft, p.position);
    topRight    = Vector3Add(topRight, p.position);
    bottomRight = Vector3Add(bottomRight, p.position);
    bottomLeft  = Vector3Add(bottomLeft, p.position);
    
    if (pleut) {
        float height = GetHeightFromTerrain((Vector3){ p.position.x, 0.0f, p.position.z }, terrainImage, taille_terrain);
        // Check si elle touche le sol
        if (p.position.y <= height) {
            // Réinitialiser la position de la goutte de pluie en haut
            float posX = frand() * taille_terrain.x - taille_terrain.x / 2;
            float posZ = frand() * taille_terrain.z - taille_terrain.z / 2;
            // Petit décalage random pour une distrib plus naturelle
            float offsetX = random_flottant(-0.1f, 0.1f);
            float offsetZ = random_flottant(-0.1f, 0.1f);
            posX += offsetX;
            posZ += offsetZ;
            
            // Obtenir la hauteur à cette position
            float newHeight = GetHeightFromTerrain((Vector3){ posX, 0.0f, posZ }, terrainImage, taille_terrain);
            
            // Placer la goutte en haut du terrain
            p.position = (Vector3){ posX, newHeight + taille_terrain.y, posZ };
            
            // Générer une nouvelle vitesse de chute aléatoire
            p.velocite.y = -1.0f * (frand() * 0.50f + 0.2f); // Vitesse minimale de 0.2
        }
        
        Color finalColor = p.color;
        
        // Dessine manuellement
        rlBegin(RL_QUADS);
            rlColor4ub(p.color.r, p.color.g, p.color.b, p.color.a);
            rlVertex3f(topLeft.x, topLeft.y, topLeft.z);
            rlVertex3f(topRight.x, topRight.y, topRight.z);
            rlVertex3f(bottomRight.x, bottomRight.y, bottomRight.z);
            rlVertex3f(bottomLeft.x, bottomLeft.y, bottomLeft.z);
        rlEnd();
    } else {
        // Si il ne pleut pas, rendre les gouttes invisibles
        rlBegin(RL_QUADS);
            rlColor4ub(p.color.r, p.color.g, p.color.b, 0);
            rlVertex3f(topLeft.x, topLeft.y, topLeft.z);
            rlVertex3f(topRight.x, topRight.y, topRight.z);
            rlVertex3f(bottomRight.x, bottomRight.y, bottomRight.z);
            rlVertex3f(bottomLeft.x, bottomLeft.y, bottomLeft.z);
        rlEnd();
    }
}

//pour gerer le temps de la pluie
double start_time = 0;
bool chrono_lance = false;
void lancer_chrono(double &start_time){
    start_time = GetTime();
    chrono_lance = true;
}

bool is_time_expired(double time_limit, double start_time) {
    double current_time = GetTime();
    double elapsed = current_time - start_time;
    double adjusted_time_limit = time_limit / simulationSpeed;
    
    if (elapsed >= adjusted_time_limit) {
        //faut le stoper le chrono ici
        chrono_lance = false;
        return true;
    }else{
        return false;
    }
}
//particules d'herbe
#define MAX_GRASS 10000
#define PENTE_SEUIL 0.20f //valeur de la pente max pour l'herbe

typedef struct GrassQuad {
    Vector3 position;
    float rotationX;      // Rotation autour de l'axe X (en radians)
    float rotationY;      // Rotation autour de l'axe Y (en radians)
    Vector2 size;         // Largeur / Hauteur du quad
    Color color;
} GrassQuad;

GrassQuad grass[MAX_GRASS];

// Génère les particules
void InitGrassParticles(Vector3 taille_terrain, Image image_sol) {
    for (int i = 0; i < MAX_GRASS; i++) {
        //pour faire apparaitre l'herbe partout
        float posX = frand() * taille_terrain.x - taille_terrain.x / 2;
        float posZ = frand() * taille_terrain.z - taille_terrain.z / 2;
        
        //petit décalage random pour une distrib plus naturelle
        float offsetX = random_flottant(-0.1f, 0.1f);
        float offsetZ = random_flottant(-0.1f, 0.1f);
        posX += offsetX;
        posZ += offsetZ;
        //printf("herbe%d: (%f, %f)\n", i, posX, posZ);
        //recup la hauteur du terrain
        float height = GetHeightFromTerrain((Vector3){ posX, 0.0f, posZ }, image_sol, taille_terrain);
        float seuil_verification_pente = 0.2f; //seuil pour vérifier la pente
        // --- Calcul de la pente --- //
        float heightLeft  = GetHeightFromTerrain((Vector3){ posX - seuil_verification_pente, 0.0f, posZ }, image_sol, taille_terrain);
        float heightRight = GetHeightFromTerrain((Vector3){ posX + seuil_verification_pente, 0.0f, posZ }, image_sol, taille_terrain);
        float heightUp    = GetHeightFromTerrain((Vector3){ posX, 0.0f, posZ - seuil_verification_pente }, image_sol, taille_terrain);
        float heightDown  = GetHeightFromTerrain((Vector3){ posX, 0.0f, posZ + seuil_verification_pente }, image_sol, taille_terrain);

        float deltaX = fabsf(heightLeft - heightRight);
        float deltaZ = fabsf(heightUp - heightDown);
        float slope = sqrtf((deltaX * deltaX + deltaZ * deltaZ) / 2.0f);

        // Si la pente dépasse un seuil, on saute ce quad
        const float SLOPE_THRESHOLD = PENTE_SEUIL;
        if (slope > SLOPE_THRESHOLD) {
            i--; // Refaire ce quad
            continue;
        }

        //on set la position de l'herbe avec la hauteur calculée
        grass[i].size = (Vector2){0.01f + frand() * 0.02f, 0.04f + frand() * 0.03f};
        grass[i].position = (Vector3){ posX, height+grass[i].size.y, posZ };
        grass[i].rotationY = frand() * PI * 2.0f;
        grass[i].rotationX = random_flottant(-0.2f, 0.2f);
        //grass[i].color = GREEN;
        float teinte = random_flottant(0.5f, 1.0f);
        grass[i].color = (Color){
            (unsigned char)(34 * teinte),
            (unsigned char)(139 * teinte),
            (unsigned char)(34 * teinte),
            255
        };
    }
}



bool useTerrainColorForGrass = false;
Image terrainColorImage; //pour choper les couleur du terrain

Color GetGrassColorFromTime(float timeOfDay) {
    if (timeOfDay < 6.0f || timeOfDay >= 20.0f) {
        return (Color){ 10, 10, 30, 255 }; // Nuit
    } else if (timeOfDay < 7.0f) {
        float t = timeOfDay - 6.0f;
        return (Color){
            (unsigned char)(255 * t),
            (unsigned char)(165 * t),
            (unsigned char)(10 * t),
            255
        };
    } else if (timeOfDay < 8.0f) {
        float t = timeOfDay - 7.0f;
        return (Color){
            255,
            (unsigned char)(200 + t * 55),
            (unsigned char)(100 - t * 50),
            255
        };
    } else if (timeOfDay < 17.0f) {
        return (Color){ 255, 255, 255, 255 }; // Jour
    } else if (timeOfDay < 18.0f) {
        float t = timeOfDay - 17.0f;
        return (Color){
            (unsigned char)(255 - t * 5),
            (unsigned char)(255 - t * 55),
            (unsigned char)(100 + t * 50),
            255
        };
    } else if (timeOfDay < 19.0f) {
        float t = timeOfDay - 18.0f;
        return (Color){
            (unsigned char)(255 - t * 155),
            (unsigned char)(150 - t * 100),
            (unsigned char)(100 + t * 50),
            255
        };
    } else {
        float t = timeOfDay - 19.0f;
        return (Color){
            (unsigned char)(100 - t * 90),
            (unsigned char)(50 - t * 40),
            (unsigned char)(150 + t * 10),
            255
        };
    }
}
float timeOfDay = 12.0f; // L'heure du jour (de 0 à 24)

// Dessine un quad à partir d’une position centrale, taille, et rotation X
void DrawGrassQuad(GrassQuad g, Color baseColor, bool useTerrainColor, Image terrainImage, Vector3 taille_terrain) {
    
    float w = g.size.x * 0.5f;
    float h = g.size.y;

    // Vecteurs du quad avant rotation
    Vector3 topLeft     = (Vector3){ -w, 0.0f, 0.0f };
    Vector3 topRight    = (Vector3){  w, 0.0f, 0.0f };
    Vector3 bottomRight = (Vector3){  w, -h, 0.0f };
    Vector3 bottomLeft  = (Vector3){ -w, -h, 0.0f };

    Matrix rot = MatrixRotateY(g.rotationY);
    topLeft     = Vector3Transform(topLeft, rot);
    topRight    = Vector3Transform(topRight, rot);
    bottomRight = Vector3Transform(bottomRight, rot);
    bottomLeft  = Vector3Transform(bottomLeft, rot);

    rot = MatrixRotateX(g.rotationX);
    topLeft     = Vector3Transform(topLeft, rot);
    topRight    = Vector3Transform(topRight, rot);
    bottomRight = Vector3Transform(bottomRight, rot);
    bottomLeft  = Vector3Transform(bottomLeft, rot);

    // Applique la position
    topLeft     = Vector3Add(topLeft, g.position);
    topRight    = Vector3Add(topRight, g.position);
    bottomRight = Vector3Add(bottomRight, g.position);
    bottomLeft  = Vector3Add(bottomLeft, g.position);
    Color finalColor = g.color;

    if (useTerrainColorForGrass) {
    // Convertir les coordonnées du monde en coordonnées d'image
    // Pour une grille de GRID_SIZE x GRID_SIZE
    int ix = (int)((g.position.x + taille_terrain.x/2) * terrainImage.width / taille_terrain.x);
    int iz = (int)((g.position.z + taille_terrain.z/2) * terrainImage.height / taille_terrain.z);

    // Clamp pour rester dans les limites
    ix = Clamp(ix, 0, terrainImage.width - 1);
    iz = Clamp(iz, 0, terrainImage.height - 1);
    
    finalColor = GetImageColor(terrainImage, ix, iz);
}
    float light = 1.0f; 
    
    // Dessine manuellement
    rlBegin(RL_QUADS);
        // Couleur de base pour l'herbe (vert)
        Color grassBaseColor = g.color; // Vert forêt
        
        // Si on utilise la couleur du terrain, on mélange avec finalColor
        // Sinon, on mélange avec baseColor
        //Color blendColor = useTerrainColor ? finalColor : baseColor;
        // Calcul de la lumière (ex: entre 0.2 et 1.0 selon l’heure)
        
        if (timeOfDay < 6.0f || timeOfDay >= 20.0f) {
            light = 0.2f; // Nuit - lumière faible
        } else if (timeOfDay < 7.0f) {
            // Transition entre nuit et aube (6h-7h)
            float t = timeOfDay - 6.0f;
            light = 0.2f + t * 0.3f; // De 0.2 à 0.5
        } else if (timeOfDay < 8.0f) {
            // Transition entre aube et jour (7h-8h)
            float t = timeOfDay - 7.0f;
            light = 0.5f + t * 0.5f; // De 0.5 à 1.0
        } else if (timeOfDay < 17.0f) {
            light = 1.0f; // Plein jour - lumière maximale
        } else if (timeOfDay < 18.0f) {
            // Transition entre jour et crépuscule (17h-18h)
            float t = timeOfDay - 17.0f;
            light = 1.0f - t * 0.3f; // De 1.0 à 0.7
        } else if (timeOfDay < 19.0f) {
            // Transition entre crépuscule et soir (18h-19h)
            float t = timeOfDay - 18.0f;
            light = 0.7f - t * 0.3f; // De 0.7 à 0.4
        } else if (timeOfDay < 20.0f) {
            // Transition entre soir et nuit (19h-20h)
            float t = timeOfDay - 19.0f;
            light = 0.4f - t * 0.2f; // De 0.4 à 0.2
        }

        // Mélange linéaire entre les deux couleurs
        Color blendColor = useTerrainColor ? finalColor : baseColor;

        unsigned char r = (unsigned char)Clamp((g.color.r * (1 - 0.5f) + blendColor.r * 0.5f) * light, 0, 255);
        unsigned char v = (unsigned char)Clamp((g.color.g * (1 - 0.5f) + blendColor.g * 0.5f) * light, 0, 255);
        unsigned char b = (unsigned char)Clamp((g.color.b * (1 - 0.5f) + blendColor.b * 0.5f) * light, 0, 255);

        // Addition des couleurs (avec limitation à 255)
        //unsigned char r = (unsigned char)Clamp(grassBaseColor.r + blendColor.r * 0.5f, 0, 255);
        //unsigned char v = (unsigned char)Clamp(grassBaseColor.g + blendColor.g * 0.5f, 0, 255);
        //unsigned char b = (unsigned char)Clamp(grassBaseColor.b + blendColor.b * 0.5f, 0, 255);
        //test couleur
        
        rlColor4ub(r, v, b, 255);
        rlVertex3f(topLeft.x, topLeft.y, topLeft.z);
        rlVertex3f(topRight.x, topRight.y, topRight.z);
        rlVertex3f(bottomRight.x, bottomRight.y, bottomRight.z);
        rlVertex3f(bottomLeft.x, bottomLeft.y, bottomLeft.z);
    rlEnd();
}

#define MODE_NORMAL 0
#define MODE_TEMPERATURE 1
#define MODE_HUMIDITE 2
#define MODE_PLUVIOMETRIE 3
int viewMode = MODE_NORMAL;

#define VIDE  CLITERAL(Color){ 0, 0, 0, 0 }   // Light Gray




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
        float t = timeOfDay - 18.0f; // Transition de 18h à 19h
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
void dessine_scene(Camera camera, Image image_sol, Vector3 taille_terrain, Model model_sol, Model emptyModel, std::vector<Plante> plantes, std::vector<std::vector<GridCell>> grille, int viewMode, int &minTemp, int &maxTemp, int &minHum, int &maxHum, Vector3 mapPosition);

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

bool is_plant_morte(const std::string& nom, const std::vector<Plante>& plantes_mortes) {
    for (const Plante& plante : plantes_mortes) {
        if (plante.nom == nom) {
            return true;
        }
    }
    return false;
}

//fonction pour vierifie quel plante peut vivre sous les conditions de sa case
void verifier_plante(std::vector<std::vector<GridCell>> &grille, GridCell *cellule, std::vector<Plante> plantes, std::vector<Plante> plantes_mortes, Plante vide, int minTemp, int maxTemp, int minHum, int maxHum, Color couleur_sante, float delta){
    if(is_plant_morte(cellule->plante.nom, plantes_mortes) || cellule->plante.nom == "Vide"){//si la plante est morte
        if(cellule->plante.age >= cellule->plante.age_max){//si la plante est morte depuis trop longtemps
            Plante bestPlante = vide;
            float bestScore = 0;
            int nb_plantes_qui_peuvent_survivre = 0;
            float scoreTemperature = 0;
            float scoreHumidite = 0;
            float scorePluviometrie = 0;
            float score = 0;
            for (Plante plante : plantes) {
                if (cellule->temperature >= plante.temperature_min && cellule->temperature <= plante.temperature_max &&
                    cellule->humidite >= plante.humidite_min && cellule->humidite <= plante.humidite_max &&
                    cellule->pluviometrie >= plante.pluviometrie_min && cellule->pluviometrie <= plante.pluviometrie_max &&
                    cellule->pente >= plante.pente_min &&
                    cellule->pente <= plante.pente_max) {
                    nb_plantes_qui_peuvent_survivre++;
                }
            }
            for (Plante plante : plantes) {
                if (cellule->temperature >= plante.temperature_min && cellule->temperature <= plante.temperature_max &&
                    cellule->humidite >= plante.humidite_min && cellule->humidite <= plante.humidite_max && cellule->pente >= plante.pente_min &&
                    cellule->pente <= plante.pente_max) {
                    scoreTemperature = fabs(cellule->temperature - ((plante.temperature_max + plante.temperature_min) / 2.0f)); //fabs(cellule->temperature - (plante.temperature_max + plante.temperature_min)); //plus c'est proche de 0 mieux c'est
                    scoreHumidite = fabs(cellule->humidite - ((plante.humidite_max + plante.humidite_min) / 2.0f));//fabs(cellule->humidite - (plante.humidite_max + plante.humidite_min)); //plus c'est proche de 0 mieux c'est
                    scorePluviometrie = fabs(cellule->pluviometrie - ((plante.pluviometrie_max + plante.pluviometrie_min) / 2.0f));//fabs(cellule->pluviometrie - (plante.pluviometrie_max + plante.pluviometrie_min)); //plus c'est proche de 0 mieux c'est TODO changer 
                    score = scoreTemperature + scoreHumidite + scorePluviometrie;
                    if (score < bestScore || bestScore == 0) {
                        bestScore = score;
                        bestPlante = plante;
                    }
                }
            }
            //cout << "la meilleure plante est : " << bestPlante.nom << endl;
            cellule->plante = bestPlante;
            cellule->plante.age = rand() % (int)(bestPlante.age_max * 0.2f);
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
            cellule->plante.age += delta;//bug ça incrémente pas l'age de la plante morte TODO
            return;
        }
    }
    else{//si la plante n'est pas morte
        cellule->plante.age += delta;
        if (cellule->plante.age >= cellule->plante.age_max || cellule->plante.sante <= 0) {//si la plante est morte
            cellule->plante.age = 0;
            float taille_actuelle = cellule->plante.taille;
            
            //on cherche la plante morte correspondante avec le meme id en gros
            for (int i = 0; i < plantes_mortes.size(); i++) {
                if (cellule->plante.id == plantes_mortes[i].id) {
                    Plante planteMorteActuelle = plantes_mortes[i];
                    planteMorteActuelle.taille = taille_actuelle;
                    cellule->plante = planteMorteActuelle;
                    break;
                }
            }
            return;
        }
        else{
            if(cellule->plante.sante >= 1){//si la plante est en bonne santé
                //if (cellule->plante.sante <= 1){
                //    cellule->plante.sante = 0;
                //    cellule->plante.age = 0;
                //    float taille_actuelle = cellule->plante.taille;
                //    Plante planteMorteActuelle = plante_morte;
                //    planteMorteActuelle.taille = taille_actuelle;
                //    cellule->plante = planteMorteActuelle;
                //    return;
                //}
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
                        if (cellule->temperature >= plante.temperature_min &&
                            cellule->temperature <= plante.temperature_max &&
                            cellule->humidite >= plante.humidite_min &&
                            cellule->humidite <= plante.humidite_max && 
                            cellule->pluviometrie >= plante.pluviometrie_min &&
                            cellule->pluviometrie <= plante.pluviometrie_max &&
                            cellule->pente >= plante.pente_min &&
                            cellule->pente <= plante.pente_max) {
                                //printf("plante %s peut survivre\n", plante.nom.c_str());
                            nb_plantes_qui_peuvent_survivre++;
                        }
                    }
                    for (Plante plante : plantes) {
                        if (cellule->temperature >= plante.temperature_min && 
                            cellule->temperature <= plante.temperature_max &&
                            cellule->humidite >= plante.humidite_min && 
                            cellule->humidite <= plante.humidite_max && 
                            cellule->pluviometrie >= plante.pluviometrie_min &&
                            cellule->pluviometrie <= plante.pluviometrie_max &&
                            cellule->pente >= plante.pente_min &&
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
                        //cellule->plante.taille *= 1.01f;
                        // Utiliser un facteur de croissance qui dépend du temps écoulé
                        float growthFactor = 1.0f + (0.01f * delta);
                        cellule->plante.taille *= growthFactor;                
                        // S'assurer que la taille ne dépasse pas le maximum
                        if (cellule->plante.taille > cellule->plante.taille_max) {
                            cellule->plante.taille = cellule->plante.taille_max;
                        }
                    }
                    return;
                }
            }
        }
    }
    return;
}


int minTemp = 1;
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

int minPluv = 0;
int maxPluv = 10;
//pour la pluviometrie
Color GetPluviometrieColor(int rainfall, int minPluv, int maxPluv) {
    float normalizedPluv = (float)(rainfall - minPluv) / (maxPluv - minPluv);
    normalizedPluv = Clamp(normalizedPluv, 0.0f, 1.0f);
    
    // Interpolation entre blanc (sec) et bleu (humide)
    Color dryColor = WHITE;
    Color wetColor = BLUE;
    
    unsigned char r = (unsigned char)(dryColor.r + (wetColor.r - dryColor.r) * normalizedPluv);
    unsigned char g = (unsigned char)(dryColor.g + (wetColor.g - dryColor.g) * normalizedPluv);
    unsigned char b = (unsigned char)(dryColor.b + (wetColor.b - dryColor.b) * normalizedPluv);
    
    return (Color){r, g, b, 255};
}

int main(void) {
    // Initialisation
    const int screenWidth = 1920;//1280;//1920;
    const int screenHeight = 1080;//720;//1080;
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
    .position = (Vector3){ -5.0f, 0.0f, -5.0f },
    .target = (Vector3){ 0.0f, 0.0f, 0.0f },
    .up = (Vector3){ 0.0f, 1.0f, 0.0f },
    .fovy = 85.0f,
    .projection = CAMERA_PERSPECTIVE
    };

    //Lumière directionnelle
    //basic lighting shader
    Shader shader = LoadShader(TextFormat("include/shaders/resources/shaders/glsl%i/lighting.vs", GLSL_VERSION),TextFormat("include/shaders/resources/shaders/glsl%i/lighting.fs", GLSL_VERSION));
    
    //les ombres
    Shader shadowShader = LoadShader(TextFormat("include/shaders/resources/shaders/glsl%i/shadowmap.vs", GLSL_VERSION),TextFormat("include/shaders/resources/shaders/glsl%i/shadowmap.fs", GLSL_VERSION));
    
    //pour la pluie
    Shader postProcessShader;
    RenderTexture2D target;
    float rainIntensity = 0.5f; // Valeur entre 0.0 et 1.0
    postProcessShader = LoadShader("ressources/custom_shader/glsl330/post_pro.vs", 
                              "ressources/custom_shader/glsl330/post_pro.fs");
    target = LoadRenderTexture(screenWidth, screenHeight);

    //def des uniformes du shader
    int pluie_resolutionLoc = GetShaderLocation(postProcessShader, "resolution");
    int pluie_timeLoc = GetShaderLocation(postProcessShader, "time");
    int pluie_rainEffectLoc = GetShaderLocation(postProcessShader, "rainEffect");
    int pluie_rainIntensityLoc = GetShaderLocation(postProcessShader, "rainIntensity");

    //on donne la résolution au shader
    Vector2 resolution = { (float)screenWidth, (float)screenHeight };       

    //l'herbe
    Shader herbe_shader = LoadShader("ressources/custom_shader/glsl330/herbe_shader.vs","ressources/custom_shader/glsl330/herbe_shader.fs");
    // Configurez les locations du shader de l'ombre et des arbres
    shadowShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shadowShader, "viewPos");
    //arbres_shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(arbres_shader, "viewPos");
    //pour l'ombre    
    Vector3 lightDir = Vector3Normalize((Vector3){ 0.35f, -1.0f, -0.35f });
    Color lightColor = WHITE;
    Vector4 lightColorNormalized = ColorNormalize(lightColor);
    int lightDirLoc = GetShaderLocation(shadowShader, "lightDir");
    int lightColLoc = GetShaderLocation(shadowShader, "lightColor");
    //int lightDirArbresLoc = GetShaderLocation(arbres_shader, "lightDir");
    //int lightColArbresLoc = GetShaderLocation(arbres_shader, "lightColor");
    SetShaderValue(shadowShader, lightDirLoc, &lightDir, SHADER_UNIFORM_VEC3);
    SetShaderValue(shadowShader, lightColLoc, &lightColorNormalized, SHADER_UNIFORM_VEC4);

    //SetShaderValue(arbres_shader,lightDirArbresLoc, &lightDir, SHADER_UNIFORM_VEC3);
    //SetShaderValue(arbres_shader,lightColArbresLoc, &lightColorNormalized, SHADER_UNIFORM_VEC4);
    
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
    //int ambientLocArbres = GetShaderLocation(arbres_shader, "ambient");
    //int lightVPArbresLoc = GetShaderLocation(arbres_shader, "lightVP");
    //int shadowMapArbresLoc = GetShaderLocation(arbres_shader, "shadowMap");
    //SetShaderValue(arbres_shader, GetShaderLocation(arbres_shader, "shadowMapResolution"), &shadowMapResolution, SHADER_UNIFORM_INT);
    //le sol
    bool choisis = false;
    Image image_sol; // Load heightmap image (RAM)    
    //image de la temperature
    Image temperatureMap;
    Texture2D temperatureTexture;
    Mesh mesh_sol; // Generate heightmap mesh (RAM and VRAM)
    Model model_sol; // Load model from generated mesh
    Image image_texture_sol; //rocky_terrain_02_diff_1k.jpg
    Texture2D texture_sol; // Load map texture
    Shader shader_taille;
    int uvScaleLoc;
    Vector2 uvScale; // Plus grand = texture plus petite et répétée
    // Generate a texture with Perlin noise
    Image perlinNoiseImage; // Generate Perlin noise image
    Texture2D perlinNoiseTexture; // Load texture from image
    Vector3 mapPosition = { -2.0f, 0.0f, -2.0f };// Define model position
    //fin test sol

    //chargement du modèle et la texture test commentaire
    //Model model_mort  = LoadModel("models/arb_mort/scene.gltf");
    Model model_sapin = LoadModel("models/pine_tree/scene.glb");
    Model model_buisson_europe = LoadModel("models/buisson/foret_classique/scene.gltf");
    Texture2D texture_buisson_europe = LoadTexture("models/buisson/foret_classique/textures/gbushy_baseColor.png");
    model_buisson_europe.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture_buisson_europe;
    //Model model_herbe = LoadModel("models/herbe/untitled.glb");
    
    //pour l'herbe du sol
    Model model_herbe_instance = LoadModel("models/herbe/untitled2.glb");
    //forets temperee
    //0 bouleau bouleau_feuilles1
    Model model_bouleau1 = LoadModel("models/foret_tempere/arb_bouleau/bouleau_feuilles1.glb");
    Model model_mort_bouleau1 = LoadModel("models/foret_tempere/arb_bouleau/bouleau_mort1.glb");
    //1 bouleau bouleau_feuilles2
    Model model_bouleau2 = LoadModel("models/foret_tempere/arb_bouleau/bouleau_feuilles2.glb");
    Model model_mort_bouleau2 = LoadModel("models/foret_tempere/arb_bouleau/bouleau_mort2.glb");
    //2 erable erable_feuilles
    Model model_erable = LoadModel("models/foret_tempere/arb_erable/erable_feuilles.glb");
    Model model_mort_erable = LoadModel("models/foret_tempere/arb_erable/erable_mort.glb");
    //3 hetre hetre_feuilles
    Model model_hetre = LoadModel("models/foret_tempere/arb_hetre/hetre_feuilles.glb");
    Model model_mort_hetre = LoadModel("models/foret_tempere/arb_hetre/hetre_mort.glb");
    //4 chene oaks_feuilles
    Model model_chene = LoadModel("models/foret_tempere/arb_oak/oaks_feuilles.glb");
    Model model_mort_chene = LoadModel("models/foret_tempere/arb_oak/oaks_mort.glb");

    //foret tropicale humide 20°C à 35°C 75 à 95 % 2 000 à 5 000 mm
    //5 jungle1 jungle_feuillage
    //6 jungle2 jungle_feuillage2
    //7 jungle3 jungle_feuillage3
    
    //on applique la lumière sur toutes les plantes
    model_bouleau1.materials[0].shader = herbe_shader;
    for (int i = 0; i < model_bouleau1.materialCount; i++)
    {
        model_bouleau1.materials[i].shader = herbe_shader;
    }
    model_mort_bouleau1.materials[0].shader = herbe_shader;
    for (int i = 0; i < model_mort_bouleau1.materialCount; i++)
    {
        model_mort_bouleau1.materials[i].shader = herbe_shader;
    }

    model_bouleau2.materials[0].shader = herbe_shader;
    for (int i = 0; i < model_bouleau2.materialCount; i++)
    {
        model_bouleau2.materials[i].shader = herbe_shader;
    }
    model_mort_bouleau2.materials[0].shader = herbe_shader;
    for (int i = 0; i < model_mort_bouleau2.materialCount; i++)
    {
        model_mort_bouleau2.materials[i].shader = herbe_shader;
    }

    model_erable.materials[0].shader = herbe_shader;
    for (int i = 0; i < model_erable.materialCount; i++)
    {
        model_erable.materials[i].shader = herbe_shader;
    }
    model_mort_erable.materials[0].shader = herbe_shader;
    for (int i = 0; i < model_mort_erable.materialCount; i++)
    {
        model_mort_erable.materials[i].shader = herbe_shader;
    }
    
    model_hetre.materials[0].shader = herbe_shader;
    for (int i = 0; i < model_hetre.materialCount; i++)
    {
        model_hetre.materials[i].shader = herbe_shader;
    }
    model_mort_hetre.materials[0].shader = herbe_shader;
    for (int i = 0; i < model_mort_hetre.materialCount; i++)
    {
        model_mort_hetre.materials[i].shader = herbe_shader;
    }
    
    model_chene.materials[0].shader = herbe_shader;
    for (int i = 0; i < model_chene.materialCount; i++)
    {
        model_chene.materials[i].shader = herbe_shader;
    }
    model_mort_chene.materials[0].shader = herbe_shader;
    for (int i = 0; i < model_mort_chene.materialCount; i++)
    {
        model_mort_chene.materials[i].shader = herbe_shader;
    }
    //model_acacia.materials[0].shader = shadowShader;
    //for (int i = 0; i < model_acacia.materialCount; i++)
    //{
    //    model_acacia.materials[i].shader = shadowShader;
    //}
    //
    //model_acacia.materials[0].shader = herbe_shader;
    //for (int i = 0; i < model_acacia.materialCount; i++)
    //{
    //    model_acacia.materials[i].shader = herbe_shader;
    //}
//
    //model_acacia.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
    //
    //model_sapin.materials[0].shader = shadowShader;
    //for (int i = 0; i < model_sapin.materialCount; i++)
    //{
    //    model_sapin.materials[i].shader = shadowShader;
    //}
    //model_buisson_europe.materials[0].shader = shadowShader;
    //for (int i = 0; i < model_buisson_europe.materialCount; i++)
    //{
    //    model_buisson_europe.materials[i].shader = shadowShader;
    //}
    //
    

    //model_mort.materials[0].shader = shadowShader;
    //for (int i = 0; i < model_mort.materialCount; i++)
    //{
    //    model_mort.materials[i].shader = shadowShader;
    //}
    
    //model_herbe.materials[0].shader = shadowShader;
    //for (int i = 0; i < model_herbe.materialCount; i++)
    //{
    //    model_herbe.materials[i].shader = shadowShader;
    //}
    model_herbe_instance.materials[0].shader = herbe_shader;
    for (int i = 0; i < model_herbe_instance.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture.id; i++)
    {
        model_herbe_instance.materials[i].shader = herbe_shader;
    }
    
    
    // Après avoir chargé le shader
    if (herbe_shader.id == 0) {
        printf("Erreur lors du chargement du shader d'herbe!\n");
    }
    
    //model_sol.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
    //model_mort.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
    
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
    // Création d'une plante
    /*
    string nom;
    int id;
    int sante;
    int humidite_min;
    int humidite_max;
    int temperature_min;
    int temperature_max;
    int pluviometrie_min;
    int pluviometrie_max;
    int influence_humidite;
    int influence_temperature;
    float taille;
    float taille_max;
    float pente_min;
    float pente_max;
    float age;
    bool morte;
    int age_max;
    Model model;
    Color couleur;
    */

    /*
    Liste des plantes
    forets temperee 5°C à 25°C 60 à 80 % 500 à 1 500 mm
    0 bouleau bouleau_feuilles1
    1 bouleau bouleau_feuilles2
    2 erable erable_feuilles
    3 hetre hetre_feuilles
    4 chene oaks_feuilles

    foret tropicale humide 20°C à 35°C 75 à 95 % 2 000 à 5 000 mm
    5 jungle1 jungle_feuillage
    6 jungle2 jungle_feuillage2
    7 jungle3 jungle_feuillage3

    forets tropicale seche 25°C à 35°C 40 à 70 % 1 000 à 2 000 mm
    faut regler les soucis d'accacia
        */
    Color couleur = WHITE;
    int influence_temperature = GetRandomValue(-5, 5);
    int influence_humidite = GetRandomValue(-5, 5);

    Plante bouleau1("Bouleau1", 0, 100, 70, 80, 5, 10, 500, 1000, influence_temperature, influence_humidite, 0.005f, 0.01f, 0.0f, 0.2f, 0, false, 250, model_bouleau1, couleur);
    influence_temperature = GetRandomValue(-5, 5);
    influence_humidite = GetRandomValue(-5, 5);
    Plante bouleau2("Bouleau2", 1, 100, 75, 80, 5, 10, 500, 1000, influence_temperature, influence_humidite, 0.005f, 0.01f, 0.0f, 0.2f, 0, false, 250, model_bouleau2, couleur);
    influence_temperature = GetRandomValue(-5, 5);
    influence_humidite = GetRandomValue(-5, 5);
    Plante erable("Erable", 2, 100, 70, 80, 10, 25, 500, 1500, 0, 0, 0.01f, 0.04f, 0.0f, 01.f, 0, false, 350, model_erable, couleur);
    influence_temperature = GetRandomValue(-5, 5);
    influence_humidite = GetRandomValue(-5, 5);
    //Plante chene("Chene", 3, 100, 70, 80, 10, 15, 500, 1000, 0, 0, 0.005f, 0.01f, 0.01f, 0.2f, 0, false, 1000, model_chene, couleur);
    Plante bouleau_mort1("Bouleau_mort1", 0, 100, 0, 100, -50, 200, 0, 5000, 0, 0, 0.005f, 0.01f, 0.0f, 0.2f, 0, true, 50, model_mort_bouleau1, couleur);
    Plante bouleau_mort2("Bouleau_mort2", 1, 100, 0, 100, -50, 200, 0, 5000, 0, 0, 0.005f, 0.01f, 0.0f, 0.2f, 0, true, 50, model_mort_bouleau2, couleur);
    Plante erable_mort("Erable_mort", 2, 100, 0, 100, -50, 200, 0, 5000, 0, 0, 0.01f, 0.04f, 0.0f, 0.2f, 0, true, 50, model_mort_erable, couleur);
    //Plante chene_mort("chene_mort", 3, 100, 0, 100, -50, 200, 0, 5000, 0, 0, 0.005f, 0.01f, 0.0f, 0.2f, 0, true, 50, model_mort_chene, couleur);
    Plante hetre("Hetre", 3, 100, 70, 80, 10, 15, 1000, 1500, influence_temperature, influence_humidite, 0.005f, 0.01f, 0.01f, 0.2f, 0, false, 1000, model_hetre, couleur);
    Plante hetre_mort("Hetre_mort", 3, 100, 0, 100, -50, 200, 0, 5000, 0, 0, 0.005f, 0.01f, 0.01f, 0.2f, 0, true, 50, model_mort_hetre, couleur);
    //Plante acacia("Acacia", 4, 100, 70, 80, 10, 15, 500, 1000, 0, 0, 0.005f, 0.01f, 0.01f, 0.2f, 0, false, 1000, model_acacia, couleur);
    Plante chene("Chene", 4, 100, 70, 80, 10, 15, 500, 1000, influence_temperature, influence_humidite, 0.005f, 0.01f, 0.01f, 0.2f, 0, false, 1000, model_chene, couleur);
    Plante chene_mort("Chene_mort", 4, 100, 0, 100, -50, 200, 0, 5000, 0, 0, 0.005f, 0.01f, 0.01f, 0.2f, 0, true, 50, model_mort_chene, couleur);


    Plante vide("Vide", 10, 100, 0, 0, 0, 0, 0, 0, 0, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0, false, 100, emptyModel, couleur);

    std::vector<Plante> plantes = {bouleau1, bouleau2, erable, hetre, chene};//pour les plantes vivantes
    std::vector<Plante> plantes_mortes = {bouleau_mort1, bouleau_mort2, erable_mort, hetre_mort, chene_mort};//pour les plantes mortes
    //ajout des différentes météo
    /*
    temperature_min: température minimale du biome (en degrés)
    temperature_max: température maximale du biome (en degrés)
    humidite_min: humidité minimale du biome (en pourcentage)
    humidite_max: humidité maximale du biome (en pourcentage)
    pluviometrie_min: pluviométrie minimale du biome (en mm)
    pluviometrie_max: pluviométrie maximale du biome (en mm)
    */
    Biome biome_base("Base", 0, 0, 0, 0, 0, 0, Color{255, 255, 255, 255}, 1.0f, true); // Un biome de base pour le début du programme
    //forets temperee 5°C à 25°C 60 à 80 % 500 à 1 500 mm
    Biome biome_tempere("Tempere", 5, 25, 60, 80, 500, 1500, Color{255, 141, 34, 255}, 1.0f, false);//on doit en metre au moin une sur true
    
    //foret tropicale humide 20°C à 35°C 75 à 95 % 2 000 à 5 000 mm
    Biome biome_tropic_humide("Tropic humide", 20, 35, 75, 95, 2000, 5000, Color{0, 161, 231, 255}, 1.0f, false);
    
    //forets tropicale seche 25°C à 35°C 40 à 70 % 1 000 à 2 000 mm
    Biome biome_tropic_sec("Tropic sec", 25, 35, 40, 70, 1000, 2000, Color{255, 255, 255, 255}, 1.01f, false);

    //biome biome_brouillard("Brouillard", 0, 0, Color{255, 255, 255, 255}, 0.1f);
    std::vector<Biome> les_biome = {biome_base, biome_tempere, biome_tropic_humide, biome_tropic_sec};
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
    std::vector<std::vector<GridCell>> grille(GRID_SIZE, std::vector<GridCell>(GRID_SIZE, GridCell(0,{0,0,0}, vide.model, true, false, 20, 50, 500, 0.0f, vide)));
    //ajoute la grille du sol d'herbe type SolHerbe
    //le terrain
    Vector3 taille_terrain = { 4, 2, 4 }; // Taille du terrain
    int humidite_moyenne = 0;
    int temperature_moyenne = 0;
    int pluviometrie_moyenne = 0;
    
    //DisableCursor();// Limit cursor to relative movement inside the window
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
    float pluviometrie_modifieur = 0;


    //couleur qui va changer en fonction de la santé je la decale ici pour pas la declarer a chaque frame
    Color couleur_sante = WHITE;
    // Boucle principale
    float windStrength = 0.7f;//force du vent
    float windSpeed = 1.0f;//vitesse du vent  
    float delta = 0.0f;
    while (!WindowShouldClose()) {
        switch (currentScreen)
        {
            case 0:{
                // Écran de chargement
                //EnableCursor();
                if (!initializationDone) {
                    switch (loadingStage) {
                        case 0: {
                                
                                    //choix du terrain qu'on veut
                                    ClearBackground(BLACK);//TODO changer en raywhite
                                    DrawText("Choix du terrain", GetScreenWidth() / 2 - MeasureText("Choix du terrain", 40) / 2, GetScreenHeight() / 2 - 60, 40, RAYWHITE);
                                    if (GuiButton((Rectangle){ screenWidth / 2 - 100, screenHeight / 2 - 100, 200, 30 }, "Terrain 1")) {
                                        image_sol = LoadImage("ressources/heightmap.png");     // Load heightmap image (RAM)
                                        printf("heightmap.png\n");
                                        choisis = true;
                                    }
                    
                                    if (GuiButton((Rectangle){ screenWidth / 2 - 100, screenHeight / 2 - 50, 200, 30 }, "Terrain 2")) {
                                        image_sol = LoadImage("ressources/plaine_hm.png");     // Load heightmap image (RAM)
                                        printf("plaine_hm.png\n");
                                        choisis = true;
                                    }
                    
                                    if (GuiButton((Rectangle){ screenWidth / 2 - 100, screenHeight / 2, 200, 30 }, "Terrain 3")) {
                                        image_sol = LoadImage("ressources/paris_hm.png");     // Load heightmap image (RAM)
                                        printf("paris_hm.png\n");
                                        choisis = true;
                                    }
                    
                                    if (GuiButton((Rectangle){ screenWidth / 2 - 100, screenHeight / 2 + 50, 200, 30 }, "Terrain 4")) {
                                        image_sol = LoadImage("ressources/test.png");     // Load heightmap image (RAM)
                                        printf("test.png\n");
                                        choisis = true;
                                    }
                                //EndDrawing();
                                if (choisis){
                                printf("ont est ici");
                                //image de la temperature
                                temperatureMap = GenImageColor(GRID_SIZE, GRID_SIZE, WHITE);
                                temperatureTexture = LoadTextureFromImage(temperatureMap);
                                UnloadImage(temperatureMap);
                                printf("ont a fini ici");   
                                mesh_sol = GenMeshHeightmap(image_sol, (Vector3){ 40, 20, 40 }); // Generate heightmap mesh (RAM and VRAM)
                                model_sol = LoadModelFromMesh(mesh_sol); // Load model from generated mesh
                                image_texture_sol = LoadImage("ressources/compress_terrain_texture_tiede.jpg"); //rocky_terrain_02_diff_1k.jpg
                                printf("fin chargement image texture sol\n");
                                texture_sol = LoadTextureFromImage(image_texture_sol); // Load map texture
                                shader_taille = LoadShader("include/shaders/resources/shaders/glsl100/base.vs", "include/shaders/resources/shaders/glsl100/base.fs");
                                uvScaleLoc = GetShaderLocation(shader_taille, "uvScale");
                                uvScale = {10.0f, 10.0f}; // Plus grand = texture plus petite et répétée
                                SetShaderValue(shader_taille, uvScaleLoc, &uvScale, SHADER_UNIFORM_VEC2);
                                model_sol.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
                                model_sol.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture_sol; // Set map diffuse texture
                                // Generate a texture with Perlin noise
                                perlinNoiseImage = GenImagePerlinNoise(256, 256, 0, 0, 10.0f); // Generate Perlin noise image
                                perlinNoiseTexture = LoadTextureFromImage(perlinNoiseImage); // Load texture from image
                                UnloadImage(perlinNoiseImage); // Unload image from RAM
                                //TODO voir pourquoi on peut pas l'appliquer sur le sol
                                model_sol.materials[0].maps[MATERIAL_MAP_METALNESS].texture = perlinNoiseTexture; // Set map metalness texture
                                model_sol.materials[0].maps[MATERIAL_MAP_NORMAL].texture = perlinNoiseTexture; // Set map normal texture
                                model_sol.materials[0].maps[MATERIAL_MAP_ROUGHNESS].texture = perlinNoiseTexture; // Set map roughness texture
                                model_sol.materials[0].maps[MATERIAL_MAP_EMISSION].texture = perlinNoiseTexture; // Set map emission texture
                                //model_sol.materials[0].shader = shader_taille; // Assign the shader to the model ça sert à rien
                                // Set the shader for the model
                                model_sol.materials[0].shader = shadowShader;
                                printf("ont est ici");
                                loadingStage++;
                            }
                        }break;
                        case 1: {
                            // Initialisation de la grille
                            for (int x = 0; x < GRID_SIZE; x++) {
                                for (int z = 0; z < GRID_SIZE; z++) {                                
                                    float posX = x * (taille_terrain.x / GRID_SIZE) - (taille_terrain.x / 2.0f);//(3.0f / GRID_SIZE) - 1.0f;  //espace entre les plantes pour 10 = 0.3f - 1.0f, pour 100 = 0.03f - 1.0f 
                                    float posZ = z * (taille_terrain.z / GRID_SIZE) - (taille_terrain.z / 2.0f);//(3.0f / GRID_SIZE) - 1.0f;
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
                                    int pluviometrie = (int) random_flottant(0, 100); // Pluviométrie aléatoire entre PLUVI_MIN et PLUVI_MAX
                                    humidite_moyenne += humidite;
                                    temperature_moyenne += temperature;//TODO voir si c'est utile
                                    pluviometrie_moyenne += pluviometrie;
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
                                    grille[x][z].plante.age = rand() % vide.age_max;//opti pour pas que toutes les plantes soient calculées en meme temps
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
                        }break;        
                ////ecrant de chargement  Copyright (c) 2021-2025 Ramon Santamaria (@raysan5)
                        case 2: {
                            InitGrassParticles( taille_terrain, image_sol);
                            InitPluieParticules(taille_terrain, image_sol);
                            
                                loadingStage++;
                        } break;
                        case 3:{
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
                int loadingPercentage = (loadingStage * 100) / 5; // 5 étapes au total
                if (loadingStage > 1){
                    
                    //ClearBackground(RAYWHITE);
                    ClearBackground(BLACK);//TODO changer en raywhite
                    DrawText("CHARGEMENT", GetScreenWidth() / 2 - MeasureText("CHARGEMENT", 40) / 2, GetScreenHeight() / 2 - 60, 40, RAYWHITE);
                    DrawText(TextFormat("%d%%", loadingPercentage), GetScreenWidth() / 2 - MeasureText("100%", 20) / 2, GetScreenHeight() / 2, 20, RAYWHITE);
                    DrawText(TextFormat("Étape %d/5", loadingStage + 1), GetScreenWidth() / 2 - MeasureText("Étape 5/5", 20) / 2, GetScreenHeight() / 2 + 30, 20, DARKGRAY);
                    //EndDrawing();

                    // Passage à l'écran principal uniquement quand tout est initialisé
                    if (initializationDone && frameCounter > 120) {
                        currentScreen = 1;
                    }
                }
            } break;
            case 1:{
            //faut appliquer la modification de température à toutes les cases une seule fois
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

            static int last_pluv_modif = 0; // Stocker la dernière valeur appliquée
            int pluv_modif = (int)pluviometrie_modifieur;

            if (pluv_modif != last_pluv_modif) { // Vérifier si la valeur a changé
                int delta = pluv_modif - last_pluv_modif; // Calculer la différence
                for (int x = 0; x < GRID_SIZE; x++) {
                    for (int z = 0; z < GRID_SIZE; z++) {
                        grille[x][z].pluviometrie += delta; // Modifier la pluviometrie de chaque case

                        //comme ça la pluviometrie reste dans la plage 0-100
                        grille[x][z].pluviometrie = Clamp(grille[x][z].pluviometrie, 0, 100);
                    
                        if (grille[x][z].pluviometrie < minPluv) minPluv = grille[x][z].pluviometrie;
                        if (grille[x][z].pluviometrie > maxPluv) maxPluv = grille[x][z].pluviometrie;
                    }
                }
                last_pluv_modif = pluv_modif; // Mettre à jour la dernière valeur appliquée
            }

            //le biome : on redéfini les valeurs de température etc
            int temperature_modifieur_min = get_biome_temperature_min(cherche_le_biome_actuelle(les_biome));//get_biome_temperature(cherche_le_biome_actuelle(les_biome));
            int temperature_modifieur_max = get_biome_temperature_max(cherche_le_biome_actuelle(les_biome));

            int humidite_modifieur_min = get_biome_humidite_min(cherche_le_biome_actuelle(les_biome));
            int humidite_modifieur_max = get_biome_humidite_max(cherche_le_biome_actuelle(les_biome));

            int pluviometrie_modifieur_min = get_biome_pluviometrie_min(cherche_le_biome_actuelle(les_biome));
            int pluviometrie_modifieur_max = get_biome_pluviometrie_max(cherche_le_biome_actuelle(les_biome));

            //bloc pour changer toutes les températures et humidités des cases de la grille avec les modifieurs
            //on verifie si les valeurs ont changé pour éviter de recalculer
            static int prev_temp_min = temperature_modifieur_min;
            static int prev_temp_max = temperature_modifieur_max;
            static int prev_hum_min = humidite_modifieur_min;
            static int prev_hum_max = humidite_modifieur_max;
            static int prev_pluv_min = pluviometrie_modifieur_min;
            static int prev_pluv_max = pluviometrie_modifieur_max;

            //check si ça a changé
            if (prev_temp_min != temperature_modifieur_min || 
                prev_temp_max != temperature_modifieur_max || 
                prev_hum_min != humidite_modifieur_min || 
                prev_hum_max != humidite_modifieur_max ||
                prev_pluv_min != pluviometrie_modifieur_min ||
                prev_pluv_max != pluviometrie_modifieur_max) {

                minTemp = temperature_modifieur_min;
                maxTemp = temperature_modifieur_max;
                minHum = humidite_modifieur_min;
                maxHum = humidite_modifieur_max;
                minPluv = pluviometrie_modifieur_min;
                maxPluv = pluviometrie_modifieur_max;

                //update les cellules avec les nouvelles valeurs
                for (int x = 0; x < GRID_SIZE; x++) {
                    for (int z = 0; z < GRID_SIZE; z++) {
                        //on genere des valeurs random dans une fourchette
                        grille[x][z].temperature = GetRandomValue(temperature_modifieur_min, temperature_modifieur_max);
                        
                        //la meme pour l'humidité
                        grille[x][z].humidite = GetRandomValue(humidite_modifieur_min, humidite_modifieur_max);

                        //et la pluviometrie
                        grille[x][z].pluviometrie = GetRandomValue(pluviometrie_modifieur_min, pluviometrie_modifieur_max);
                        
                        // Update min/max tracking variables for display
                        if (grille[x][z].temperature < minTemp) minTemp = grille[x][z].temperature;
                        if (grille[x][z].temperature > maxTemp) maxTemp = grille[x][z].temperature;
                        if (grille[x][z].humidite < minHum) minHum = grille[x][z].humidite;
                        if (grille[x][z].humidite > maxHum) maxHum = grille[x][z].humidite;
                        if (grille[x][z].pluviometrie < minPluv) minPluv = grille[x][z].pluviometrie;
                        if (grille[x][z].pluviometrie > maxPluv) maxPluv = grille[x][z].pluviometrie;
                    }
                }
                
                // Update previous values to current
                prev_temp_min = temperature_modifieur_min;
                prev_temp_max = temperature_modifieur_max;
                prev_hum_min = humidite_modifieur_min;
                prev_hum_max = humidite_modifieur_max;
                prev_pluv_min = pluviometrie_modifieur_min;
                prev_pluv_max = pluviometrie_modifieur_max;
                
                printf("Biome changed: Temp range [%d, %d], Humidity range [%d, %d]\n", 
                       temperature_modifieur_min, temperature_modifieur_max,
                       humidite_modifieur_min, humidite_modifieur_max);
            }
            //hum_modifieur = get_biome_humidite(cherche_la_biome_actuelle(les_biome));
            cloudThreshold = get_biome_densite_nuage(cherche_le_biome_actuelle(les_biome));

            float delta = GetFrameTime() * simulationSpeed;

            static float accumulatedTime = 0.0f;
            accumulatedTime += delta * 0.5f; // Contrôle la vitesse d'animation des nuages
            float timeValue = accumulatedTime;

            //const float driftSpeed = 0.2f; // Vitesse de déplacement des nuages

            // Faire dériver les nuages horizontalement
            for (auto& nuage : grandsNuages) {
                // Distance parcourue depuis la position initiale
                static float distanceParcourue = 0.0f;
                float distanceDeReset = taille_terrain.x * 3.0f; // La largeur du nuage

                // Déplacer le nuage
                for (size_t i = 0; i < nuage.positions.size(); i++) {
                    nuage.positions[i].x += delta * nuage.vitesseDefile;
                }

                // Mettre à jour la distance parcourue
                distanceParcourue += delta * nuage.vitesseDefile;

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
            //pdate la camera view vector
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
                    terrainColorImage = ImageCopy(tempImage); // Copie pour lecture ultérieure
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
                    terrainColorImage = ImageCopy(humImage); // Copie pour lecture ultérieure
                    UnloadImage(humImage);
                    model_sol.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = temperatureTexture;
                } else {
                    // Mode normal : remettre la texture normale
                    model_sol.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture_sol;
                }
            }
            if (IsKeyPressed(KEY_P)) {
                viewMode = (viewMode == MODE_NORMAL) ? MODE_PLUVIOMETRIE : MODE_NORMAL;

                // Changer la texture du sol en fonction du mode
                if (viewMode == MODE_PLUVIOMETRIE) {
                    // Mode pluviométrie : mettre à jour la texture de pluviométrie
                    Image pluvImage = GenImageColor(GRID_SIZE, GRID_SIZE, WHITE);
                    for (int x = 0; x < GRID_SIZE; x++) {
                        for (int z = 0; z < GRID_SIZE; z++) {
                            Color pluvColor = GetPluviometrieColor(grille[x][z].pluviometrie, minPluv, maxPluv);
                            ImageDrawPixel(&pluvImage, x, z, pluvColor);
                        }
                    }
                    UpdateTexture(temperatureTexture, pluvImage.data);
                    terrainColorImage = ImageCopy(pluvImage); // Copie pour lecture ultérieure
                    UnloadImage(pluvImage);
                    model_sol.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = temperatureTexture;
                } else {
                    // Mode normal : remettre la texture normale
                    model_sol.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture_sol;
                }
            }
            if (viewMode == MODE_NORMAL){
                useTerrainColorForGrass = false;
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
                    verifier_plante(grille, &grille[x][z], plantes, plantes_mortes, vide, minTemp, maxTemp, minHum, maxHum, couleur_sante, delta);
                }
            }

            if (viewMode == MODE_TEMPERATURE) {
                useTerrainColorForGrass = true;
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
                useTerrainColorForGrass = true;
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
            if (viewMode == MODE_PLUVIOMETRIE) {
                useTerrainColorForGrass = true;
                Image pluvImage = GenImageColor(GRID_SIZE, GRID_SIZE, WHITE);

                //maj l'image avec les couleurs de pluviometrie
                for (int x = 0; x < GRID_SIZE; x++) {
                    for (int z = 0; z < GRID_SIZE; z++) {
                        Color pluvColor = GetPluviometrieColor(grille[x][z].pluviometrie, minPluv, maxPluv);
                        ImageDrawPixel(&pluvImage, x, z, pluvColor);
                    }
                }

                //maj la texture
                UpdateTexture(temperatureTexture, pluvImage.data);
                UnloadImage(pluvImage);

                //applique la texture de pluviometrie au sol
                if (viewMode == MODE_PLUVIOMETRIE) {
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

            //lightColor = GetSunColor(timeOfDay);  //couleur en fonction du temps
            //on ajoute la couleur de la météo
            Color couleur_biome = get_biome_couleur(cherche_le_biome_actuelle(les_biome));
            
            lightColor = (Color){
                GetSunColor(timeOfDay).r/2 + couleur_biome.r/2,
                GetSunColor(timeOfDay).g/2 + couleur_biome.g/2,
                GetSunColor(timeOfDay).b/2 + couleur_biome.b/2,
                GetSunColor(timeOfDay).a/2 + couleur_biome.a/2
            };
            
            lightColorNormalized = ColorNormalize(lightColor);
            

            SetShaderValue(shadowShader, lightColLoc, &lightColorNormalized, SHADER_UNIFORM_VEC4);
            SetShaderValue(shadowShader, lightDirLoc, &lightDir, SHADER_UNIFORM_VEC3);

            //test temp TODO
            SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "lightPos"), &lightDir, SHADER_UNIFORM_VEC3);
            SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "lightColor"), &lightColorNormalized, SHADER_UNIFORM_VEC4);

            SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "lightDir"), &lightDir, SHADER_UNIFORM_VEC3);
            time += delta;  // Incrémenter le temps

            SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "time"), &time, SHADER_UNIFORM_FLOAT);

            SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "windStrength"), &windStrength, SHADER_UNIFORM_FLOAT);
            SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "windSpeed"), &windSpeed, SHADER_UNIFORM_FLOAT);
            SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "noiseScale"), &noiseScale, SHADER_UNIFORM_FLOAT);
            SetShaderValueTexture(herbe_shader, GetShaderLocation(herbe_shader, "noiseTexture"), noiseTexture);
            } break;
            default: break;
        }
        // Rendu final (vue normale)
        //BeginDrawing();
        switch(currentScreen)
            {
                case 0:
                {
                }break;
                case 1:
                {
                    BeginTextureMode(target);
                        //ClearBackground(BLACK);
                        //on dessine les ombres ici
                        Matrix lightView;
                        Matrix lightProj;
                        BeginTextureMode(shadowMap);
                            //ClearBackground(SKYBLUE);
                            //ClearBackground(BLACK);//TODO changer en SKYBLUE

                            BeginMode3D(lightCam);
                                lightView = rlGetMatrixModelview();
                                lightProj = rlGetMatrixProjection();
                                dessine_scene(camera, image_sol, taille_terrain, model_sol, model_buisson_europe, plantes, grille, viewMode, minTemp, maxTemp, minHum, maxHum, mapPosition);
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
                        BeginMode3D(camera);
                        
                            Color grassColor = GetGrassColorFromTime(timeOfDay); // timeOfDay = 0.0f → 24.0f
                            for (int i = 0; i < MAX_GRASS; i++) {
                                DrawGrassQuad(grass[i], grassColor, useTerrainColorForGrass, terrainColorImage, taille_terrain);
                            }
                        
                            for (int i = 0; i < GOUTE_PLUIE; i++){
                                DrawPluieQuad(la_pluie[i], image_sol, taille_terrain, pleut, delta);
                            }
                        
                            isGrass = 2;
                            SetShaderValue(herbe_shader, GetShaderLocation(herbe_shader, "isGrass"), &isGrass, SHADER_UNIFORM_INT);
                            dessine_scene(camera, image_sol, taille_terrain, model_sol, model_buisson_europe, plantes, grille, viewMode, minTemp, maxTemp, minHum, maxHum, mapPosition);
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

                    EndTextureMode();


// Mise à jour des uniformes du shader
float currentTime = (float)GetTime();
SetShaderValue(postProcessShader, pluie_timeLoc, &currentTime, SHADER_UNIFORM_FLOAT);
int rainEffectEnabled = 1;//pleut ? 1 : 0;
SetShaderValue(postProcessShader, pluie_rainEffectLoc, &rainEffectEnabled, SHADER_UNIFORM_INT);
SetShaderValue(postProcessShader, pluie_rainIntensityLoc, &rainIntensity, SHADER_UNIFORM_FLOAT);
SetShaderValue(postProcessShader, pluie_resolutionLoc, &resolution, SHADER_UNIFORM_VEC2);

// Ajoutez aussi un contrôle pour l'intensité de la pluie dans l'interface
if (pleut) {
    rainIntensity = Clamp(frequence_pluie / 100.0f, 0.1f, 1.0f);
}
// Rendu final avec le shader de post-processing
BeginDrawing();
//ClearBackground(BLACK);

// Dessiner la texture avec le shader de post-processing
BeginShaderMode(postProcessShader);
DrawTextureRec(target.texture, 
              (Rectangle){ 0, 0, (float)target.texture.width, -(float)target.texture.height }, 
              (Vector2){ 0, 0 }, 
              WHITE);
EndShaderMode();

printf("Target texture: id=%u, width=%d, height=%d\n", 
       target.texture.id, target.texture.width, target.texture.height);
        //DrawGrid(20, 1.0f);
        
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
        if (viewMode == MODE_PLUVIOMETRIE) {
            DrawText("Mode pluviometrie - Appuyez sur P pour revenir", 10, 60, 20, BLACK);
            // Optionnel : afficher une échelle de température
            DrawText(TextFormat("Min: %d", minPluv), 10, 80, 20, BLUE);
            DrawText(TextFormat("Max: %d", maxPluv), 10, 100, 20, RED);
        }
        DrawText(" d'objets 3D - Utilisez la souris pour naviguer", 10, 10, 20, DARKGRAY);
        DrawText("Maintenez le clic droit pour tourner la scène", 10, 25, 20, DARKGRAY);
        if (GuiButton((Rectangle){ 100, 370, 200, 30 }, "Tempere")) {
            for (auto& biome : les_biome) {
                biome.biome_actuelle = (biome.nom == "Tempere");
            }
        }

        if (GuiButton((Rectangle){ 100, 410, 200, 30 }, "Tropic humide")) {
            for (auto& biome : les_biome) {
                biome.biome_actuelle = (biome.nom == "Tropic humide");
            }
        }

        if (GuiButton((Rectangle){ 100, 450, 200, 30 }, "Tropic sec")) {
            for (auto& biome : les_biome) {
                biome.biome_actuelle = (biome.nom == "Tropic sec");
            }
        }
        //GuiSliderBar((Rectangle){ 100, 190, 200, 20 }, "Noise Scale", TextFormat("%.2f", noiseScale), &noiseScale, 1.0f, 20.0f);
        //GuiSliderBar((Rectangle){ 100, 220, 200, 20 }, "Cloud Threshold", TextFormat("%.2f", cloudThreshold), &cloudThreshold, 0.0f, 1.5f);
        GuiSliderBar((Rectangle){ 100, 250, 200, 20 }, "Temperature", TextFormat("%d", temperature_modifieur), (float*)&temperature_modifieur, -30.0, 30.0);
        GuiSliderBar((Rectangle){ 100, 280, 200, 20 }, "Humidite", TextFormat("%d", hum_modifieur), &hum_modifieur, 0.0f, 100.0f);
        // Sliders for wind parameters
        GuiSliderBar((Rectangle){ 100, 310, 200, 20 }, "Wind Speed", TextFormat("%.2f", windSpeed), &windSpeed, 0.0f, 4.0f);
        //GuiSliderBar((Rectangle){ 100, 340, 200, 20 }, "Wind Strength", TextFormat("%.2f", windStrength), &windStrength, 0.0f, 2.0f); //ca sert à rien
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
        GuiSliderBar((Rectangle){ 100, 100, 300, 20 }, "Time of Day", TextFormat("%.0f:00", timeOfDay), &timeOfDay, 0.0f, 24.0f);
        GuiSliderBar((Rectangle){ 50, 50, 300, 20 }, "frequence pluie",TextFormat("%.0f:00",frequence_pluie), &frequence_pluie, 0.0f, 100.0f);
        GuiSliderBar((Rectangle){ 100, 80, 200, 20 }, "Simulation Speed", TextFormat("%.1fx", simulationSpeed), &simulationSpeed, 0.1f, 10.0f);
        

        static float accumTime = 0.0f;

        if(chrono_lance == true){
            accumTime += delta;
            pleut = true;
            if(is_time_expired(1.0f, start_time)){
                printf("temps expiré\n");
                pleut = false;
                random_pluie = 0.0f;
                chrono_lance = false;
                accumTime = 0.0f;
            }
        }else{
            accumTime += delta;
            //if(accumTime >= 0.5f){
                random_pluie = GetRandomValue(1, 100);
                printf("random pluie : %f\n", random_pluie);
                accumTime = 0.0f;
                if (random_pluie <= frequence_pluie){
                    printf("on lance le chrono\n");
                    lancer_chrono(start_time);
                    pleut = true;
                }else{
                    pleut = false;
                //}
            }
        }
        
        // Affichage de l'heure
        DrawText(TextFormat("Time: %.0f:00", timeOfDay), 310, 20, 20, DARKGRAY);
            } break;
        default: break;
        }
        EndDrawing();
    }

    UnloadShader(shader);
    UnloadShader(shadowShader);
    UnloadShader(shader_taille);
    UnloadShader(herbe_shader);
    // Désallocation des ressources
    UnloadModel(model_herbe_instance);
    UnloadModel(model_bouleau1);
    UnloadModel(model_bouleau2);
    UnloadModel(model_mort_bouleau1);
    UnloadModel(model_mort_bouleau2);
    UnloadModel(model_hetre);
    UnloadModel(model_mort_hetre);
    UnloadModel(model_chene);
    UnloadModel(model_mort_chene);
    //UnloadModel(model_mort);
    //UnloadModel(model_sapin);
    //UnloadTexture(texture_sapin);
    //UnloadModel(model_buisson_europe);
    //UnloadTexture(texture_buisson_europe);
    //UnloadModel(model_chene);
    UnloadModel(model_sol);
    UnloadTexture(texture_sol);
    UnloadTexture(temperatureTexture);
    UnloadShadowmapRenderTexture(shadowMap);
    // Dans la section de nettoyage (juste avant CloseWindow())
    UnloadShader(postProcessShader);
    UnloadRenderTexture(target);

    // Clear the memory of other resources
    printf("model herbe unload\n");
    UnloadImage(image_sol);
    printf("image sol unload\n");
    UnloadImage(image_texture_sol);
    printf("image texture sol unload\n");
    CloseWindow();

    return 0;
}

void dessine_scene(Camera camera, Image image_sol, Vector3 taille_terrain, Model model_sol, Model emptyModel, std::vector<Plante> plantes, std::vector<std::vector<GridCell>> grille, int viewMode, int &minTemp, int &maxTemp, int &minHum, int &maxHum, Vector3 mapPosition) {
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
        } else if(viewMode == MODE_PLUVIOMETRIE){
            if (sceneObjects[i].model == &model_sol) {
                // Le sol utilise déjà la texture de pluviometrie
                //DrawCubeV((Vector3){0,0,0 }, (Vector3){taille_terrain.x, 0.2f, taille_terrain.z}, GRAY);
                DrawModel(*sceneObjects[i].model, sceneObjects[i].position, 0.1f, WHITE);
            } else {
                // Pour les autres objets, utilisez la couleur de la pluviometrie
                for (int x = 0; x < GRID_SIZE; x++) {
                    for (int z = 0; z < GRID_SIZE; z++) {
                        if (Vector3Equals(grille[x][z].position, sceneObjects[i].position)) {
                            Color pluvColor = GetPluviometrieColor(grille[x][z].pluviometrie, minPluv, maxPluv);
                            DrawModel(*sceneObjects[i].model, sceneObjects[i].position, 1.0f, pluvColor);
                            break;
                        }
                    }
                }
            }
        }
    }
}