#include "utils.h"
#include "constants.h"
#include "../ecosystem/EcosystemManager.h"

#include <raygui.h>
#include <cmath>
#include <cstdlib>
#include <algorithm>

// Génère un float aléatoire entre 0.0 et 1.0
float frand() {
    return (float)GetRandomValue(0, 10000) / 10000.0f;
}

float random_flottant(float min, float max) {
    return min + (rand() / (float)RAND_MAX) * (max - min);
}

// Fonction pour clamp une valeur entre un minimum et un maximum
float clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

float GetHeightFromTerrain(Vector3 position, Image heightmap, Vector3 terrainSize) {
    int mapX = (int)((position.x + terrainSize.x / 2.0f) * heightmap.width / terrainSize.x);
    int mapZ = (int)((position.z + terrainSize.z / 2.0f) * heightmap.height / terrainSize.z);

    mapX = clamp(mapX, 0, heightmap.width - 1);
    mapZ = clamp(mapZ, 0, heightmap.height - 1);

    Color pixel = GetImageColor(heightmap, mapX, mapZ);
    return (pixel.r / 255.0f) * terrainSize.y;
}

void DrawUI(int viewMode, int minTemp, int maxTemp, int minHum, int maxHum, int minPluv, int maxPluv,
           float& timeOfDay, float& windSpeed, float& windStrength, std::vector<Biome>& les_biomes) {
    
    // Affichage des informations selon le mode
    if (viewMode == MODE_TEMPERATURE) {
        DrawText("Mode température - Appuyez sur T pour revenir", 10, 60, 20, BLACK);
        DrawText(TextFormat("Min: %d°C", minTemp), 10, 80, 20, BLUE);
        DrawText(TextFormat("Max: %d°C", maxTemp), 10, 100, 20, RED);
    }
    else if (viewMode == MODE_HUMIDITE) {
        DrawText("Mode humidité - Appuyez sur Y pour revenir", 10, 60, 20, BLACK);
        DrawText(TextFormat("Min: %d%%", minHum), 10, 80, 20, BLUE);
        DrawText(TextFormat("Max: %d%%", maxHum), 10, 100, 20, GREEN);
    }
    else if (viewMode == MODE_PLUVIOMETRIE) {
        DrawText("Mode pluviométrie - Appuyez sur P pour revenir", 10, 60, 20, BLACK);
        DrawText(TextFormat("Min: %d mm", minPluv), 10, 80, 20, WHITE);
        DrawText(TextFormat("Max: %d mm", maxPluv), 10, 100, 20, BLUE);
    }

    // Instructions
    DrawText("Maintenez le clic droit pour tourner la scène", 10, 25, 20, DARKGRAY);
    
    // Boutons de sélection des biomes
    if (GuiButton((Rectangle){ 100, 370, 200, 30 }, "Tempéré")) {
        for (auto& biome : les_biomes) {
            biome.biome_actuelle = (biome.nom == "Tempere");
        }
    }

    if (GuiButton((Rectangle){ 100, 410, 200, 30 }, "Tropical humide")) {
        for (auto& biome : les_biomes) {
            biome.biome_actuelle = (biome.nom == "Tropic humide");
        }
    }

    if (GuiButton((Rectangle){ 100, 450, 200, 30 }, "Tropical sec")) {
        for (auto& biome : les_biomes) {
            biome.biome_actuelle = (biome.nom == "Tropic sec");
        }
    }

    // Sliders pour les paramètres
    GuiSliderBar((Rectangle){ 100, 310, 200, 20 }, "Wind Speed", TextFormat("%.2f", windSpeed), &windSpeed, 0.0f, 7.0f);
    GuiSliderBar((Rectangle){ 100, 340, 200, 20 }, "Wind Strength", TextFormat("%.2f", windStrength), &windStrength, 0.0f, 2.0f);
    
    // Slider pour l'heure
    GuiSliderBar((Rectangle){ 100, 100, 300, 20 }, "Time of Day", TextFormat("%.0f:00", timeOfDay), &timeOfDay, 0.0f, 24.0f);
    
    // Affichage de l'heure
    DrawText(TextFormat("Time: %.0f:00", timeOfDay), 310, 20, 20, DARKGRAY);
    
    // FPS
    DrawFPS(10, 40);
}

void UpdateClouds(std::vector<Nuage>& grandsNuages, float deltaTime, Vector3 taille_terrain) {
    for (auto& nuage : grandsNuages) {
        static float distanceParcourue = 0.0f;
        float distanceDeReset = taille_terrain.x * 3.0f;

        // Déplacer le nuage
        for (size_t i = 0; i < nuage.positions.size(); i++) {
            nuage.positions[i].x += deltaTime * nuage.vitesseDefile;
        }

        // Mettre à jour la distance parcourue
        distanceParcourue += deltaTime * nuage.vitesseDefile;

        // Vérifier si le nuage a parcouru sa propre largeur
        if (distanceParcourue >= distanceDeReset) {
            // Réinitialiser la position du nuage
            for (size_t i = 0; i < nuage.positions.size(); i++) {
                nuage.positions[i].x = -taille_terrain.x;
            }
            distanceParcourue = 0.0f;
        }
    }
}

void HandleCloudRegeneration(std::vector<Nuage>& grandsNuages, float& cloudThreshold, 
                           float& noiseScale, Vector3 taille_terrain) {
    static float lastCloudThreshold = cloudThreshold;
    static float lastNoiseScale = noiseScale;
    
    if (fabsf(cloudThreshold - lastCloudThreshold) > 0.01f || fabsf(noiseScale - lastNoiseScale) > 0.1f) {
        // Régénérer les textures des nuages avec les nouveaux paramètres
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
}

void CleanupClouds(std::vector<Nuage>& grandsNuages) {
    for (auto& nuage : grandsNuages) {
        for (size_t i = 0; i < nuage.textures.size(); i++) {
            UnloadTexture(nuage.textures[i]);
        }
        for (size_t i = 0; i < nuage.plans.size(); i++) {
            UnloadModel(nuage.plans[i]);
        }
    }
    grandsNuages.clear();
}

int CompareSceneObjects(const void *a, const void *b) {
    SceneObject *objA = (SceneObject *)a;
    SceneObject *objB = (SceneObject *)b;
    return (objA->depth < objB->depth) - (objA->depth > objB->depth); // Tri décroissant
}

// ---