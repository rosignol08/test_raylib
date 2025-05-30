#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include "ecosystem/EcosystemManager.h"  
#include "rendering/renderer.h"
#include "rendering/lightning.h"        
#include "utils/utils.h"  

#include "ecosystem/plante.h"
#include "ecosystem/gridCell.h"
#include "ecosystem/biome.h"
#include "rendering/renderer.h"
#include "utils/constants.h"  
#define RLIGHTS_IMPLEMENTATION
#if defined(_WIN32) || defined(_WIN64)
#include <shaders/rlights.h>
#elif defined(__linux__)
#include <shaders/rlights.h>
#endif

int main(void) {
    // Initialisation
    const int screenWidth = 1920;
    const int screenHeight = 1080;
    int currentScreen = 0;  // 0 = menu, 1 = jeu
    int loadingStage = 0;
    bool initializationDone = false;
    
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "raylib - Projet tutore");
    rlDisableBackfaceCulling();
    rlEnableColorBlend();
    rlSetBlendMode(RL_BLEND_ALPHA);

    // Caméra
    Camera camera = { 
        .position = (Vector3){ -5.0f, 0.0f, -5.0f },
        .target = (Vector3){ 0.0f, 0.0f, 0.0f },
        .up = (Vector3){ 0.0f, 1.0f, 0.0f },
        .fovy = 85.0f,
        .projection = CAMERA_PERSPECTIVE
    };

    // Variables du système de rendu
    Renderer renderer;
    
    // Variables du système d'écosystème
    std::vector<std::vector<GridCell>> grille;
    std::vector<Plante> plantes;
    std::vector<Plante> plantes_mortes;
    std::vector<Biome> les_biomes;
    
    // Variables météo
    std::vector<Nuage> grandsNuages;
    float timeOfDay = 12.0f;
    float windStrength = 0.7f;
    float windSpeed = 1.0f;
    
    // Variables de terrain
    Image image_sol;
    Model model_sol;
    Vector3 taille_terrain = { 4, 2, 4 };
    Vector3 mapPosition = { -2.0f, 0.0f, -2.0f };
    bool choisis = false;
    
    // Modèles 3D
    Model model_bouleau1, model_bouleau2, model_erable;
    Model model_mort_bouleau1, model_mort_bouleau2, model_mort_erable;
    Model emptyModel;
    
    // Variables de visualisation
    int viewMode = MODE_NORMAL;
    int minTemp = 1, maxTemp = 10;
    int minHum = 0, maxHum = 10;
    int minPluv = 0, maxPluv = 10;
    
    // Variables d'interface
    float cloudThreshold = 0.6f;
    float noiseScale = 10.0f;
    
    SetTargetFPS(60);
    int frameCounter = 0;

    // Boucle principale
    while (!WindowShouldClose()) {
        switch (currentScreen) {
            case 0: {
                // Écran de menu/chargement
                if (!initializationDone) {
                    switch (loadingStage) {
                        case 0: {
                            printf("=== DEBUT PROGRAMME ===\n");
printf("Screen: %d, Stage: %d\n", currentScreen, loadingStage);

// Dans la boucle case 0:
printf("Dans l'écran de menu, loadingStage = %d\n", loadingStage);
                            // Choix du terrain
                            if (GuiButton((Rectangle){ screenWidth / 2 - 100, screenHeight / 2 - 100, 200, 30 }, "Terrain 1")) {
                                image_sol = LoadImage("assets/heightmap.png");
                                choisis = true;
                            }
                            if (GuiButton((Rectangle){ screenWidth / 2 - 100, screenHeight / 2 - 50, 200, 30 }, "Terrain 2")) {
                                image_sol = LoadImage("assets/plaine_hm.png");
                                choisis = true;
                            }
                            if (GuiButton((Rectangle){ screenWidth / 2 - 100, screenHeight / 2, 200, 30 }, "Terrain 3")) {
                                image_sol = LoadImage("assets/paris_hm.png");
                                choisis = true;
                            }
                            if (GuiButton((Rectangle){ screenWidth / 2 - 100, screenHeight / 2 + 50, 200, 30 }, "Terrain 4")) {
                                image_sol = LoadImage("assets/test.png");
                                choisis = true;
                            }
                            
                            if (choisis) {
                                // Initialiser le renderer
                                renderer.Initialize();
                                
                                // Charger les modèles
                                LoadModels(model_bouleau1, model_bouleau2, model_erable,
                                          model_mort_bouleau1, model_mort_bouleau2, model_mort_erable, emptyModel);
                                
                                // Initialiser le terrain
                                InitializeTerrain(image_sol, model_sol, taille_terrain);
                                
                                loadingStage++;
                            }
                        } break;
                        
                        case 1: {
                            // Initialiser la grille d'écosystème
                            InitializeEcosystem(grille, plantes, plantes_mortes, les_biomes, 
                                              model_bouleau1, model_bouleau2, model_erable,
                                              model_mort_bouleau1, model_mort_bouleau2, model_mort_erable,
                                              emptyModel, image_sol, taille_terrain);
                            loadingStage++;
                        } break;
                        
                        case 2: {
                            // Initialiser l'herbe
                            renderer.InitGrassParticles(taille_terrain, image_sol);
                            loadingStage++;
                        } break;
                        
                        case 3: {
                            // Initialiser les nuages
                            grandsNuages.push_back(GenererGrandNuage({-taille_terrain.x, 4.0f, 0.0f}, 
                                                                   taille_terrain.x * 3.0f, taille_terrain.x * 3.0f, 
                                                                   1, cloudThreshold, noiseScale));
                            grandsNuages.push_back(GenererGrandNuage({-taille_terrain.x, 3.0f, 0.0f}, 
                                                                   taille_terrain.x * 3.0f, taille_terrain.x * 3.0f, 
                                                                   1, cloudThreshold, noiseScale));
                            grandsNuages.push_back(GenererGrandNuage({-taille_terrain.x, 2.0f, 0.0f}, 
                                                                   taille_terrain.x * 3.0f, taille_terrain.x * 3.0f, 
                                                                   1, cloudThreshold, noiseScale));
                            loadingStage++;
                            initializationDone = true;
                        } break;
                    }
                }
                
                frameCounter++;
                int loadingPercentage = (loadingStage * 100) / 4;
                
                if (loadingStage > 0) {
                    BeginDrawing();
                    ClearBackground(RAYWHITE);
                    DrawText("CHARGEMENT", GetScreenWidth() / 2 - MeasureText("CHARGEMENT", 40) / 2, 
                            GetScreenHeight() / 2 - 60, 40, DARKGRAY);
                    DrawText(TextFormat("%d%%", loadingPercentage), 
                            GetScreenWidth() / 2 - MeasureText("100%", 20) / 2, GetScreenHeight() / 2, 20, DARKGRAY);
                    EndDrawing();
                    
                    if (initializationDone && frameCounter > 120) {
                        currentScreen = 1;
                    }
                }
            } break;
            
            case 1: {
                // Jeu principal
                float dt = GetFrameTime();
                
                // Gestion des biomes
                UpdateBiomes(les_biomes, grille, minTemp, maxTemp, minHum, maxHum, minPluv, maxPluv);
                
                // Mise à jour des nuages
                UpdateClouds(grandsNuages, dt, taille_terrain);
                
                // Gestion de la caméra
                HandleCameraInput(camera);
                
                // Gestion des modes d'affichage
                HandleViewModeInput(viewMode, grille, model_sol, minTemp, maxTemp, minHum, maxHum, minPluv, maxPluv);
                
                // Mise à jour de l'écosystème
                UpdateEcosystem(grille, plantes, plantes_mortes, minTemp, maxTemp, minHum, maxHum);
                
                // Mise à jour du système de rendu
                renderer.UpdateLighting(timeOfDay, les_biomes);
                renderer.UpdateWind(windStrength, windSpeed, dt);
                
                // Rendu
                BeginDrawing();
                
                // Rendu des ombres
                renderer.RenderShadows(camera, model_sol, grille, grandsNuages, mapPosition);
                
                // Rendu principal
                ClearBackground(SKYBLUE);
                BeginMode3D(camera);
                
                // Rendu de l'herbe
                renderer.RenderGrass(timeOfDay, viewMode);
                
                // Rendu de la scène
                renderer.RenderScene(camera, image_sol, taille_terrain, model_sol, grille, 
                                   viewMode, minTemp, maxTemp, minHum, maxHum, mapPosition);
                
                // Rendu des nuages
                renderer.RenderClouds(grandsNuages);
                
                EndMode3D();
                
                // Interface utilisateur
                DrawUI(viewMode, minTemp, maxTemp, minHum, maxHum, minPluv, maxPluv, 
                       timeOfDay, windSpeed, windStrength, les_biomes);
                
                // Gestion des nuages (régénération si paramètres changent)
                HandleCloudRegeneration(grandsNuages, cloudThreshold, noiseScale, taille_terrain);
                
                DrawFPS(10, 40);
                EndDrawing();
            } break;
            
            default: break;
        }
    }

    // Nettoyage
    renderer.Cleanup();
    UnloadEcosystemResources(model_bouleau1, model_bouleau2, model_erable,
                           model_mort_bouleau1, model_mort_bouleau2, model_mort_erable,
                           model_sol, emptyModel);
    UnloadTerrainResources(image_sol);
    CleanupClouds(grandsNuages);
    
    CloseWindow();
    return 0;
}