// src/utils/Utils.h
#ifndef UTILS_H
#define UTILS_H

#include <raylib.h>
#include <vector>
#include "../ecosystem/gridCell.h"
#include "../ecosystem/biome.h"
#include "../meteo/nuages.h"

// Fonctions mathématiques utilitaires
float frand();
float random_flottant(float min, float max);
float clamp(float value, float min, float max);

// Fonctions de terrain
float GetHeightFromTerrain(Vector3 position, Image heightmap, Vector3 terrainSize);

// Fonctions d'interface utilisateur
void DrawUI(int viewMode, int minTemp, int maxTemp, int minHum, int maxHum, int minPluv, int maxPluv,
           float& timeOfDay, float& windSpeed, float& windStrength, std::vector<Biome>& les_biomes);

// Fonctions de gestion des nuages
void UpdateClouds(std::vector<Nuage>& grandsNuages, float deltaTime, Vector3 taille_terrain);
void HandleCloudRegeneration(std::vector<Nuage>& grandsNuages, float& cloudThreshold, 
                           float& noiseScale, Vector3 taille_terrain);
void CleanupClouds(std::vector<Nuage>& grandsNuages);

// Fonctions de comparaison pour le tri
typedef struct {
    Vector3 position;
    Model *model;
    float depth;
    Color color;
} SceneObject;

int CompareSceneObjects(const void *a, const void *b);

#endif

