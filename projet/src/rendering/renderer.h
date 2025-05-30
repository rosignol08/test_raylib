// src/rendering/Renderer.h
#ifndef RENDERER_H
#define RENDERER_H

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <vector>
#include "../ecosystem/gridCell.h"
#include "../ecosystem/biome.h"
#include "../meteo/nuages.h"

#define MAX_GRASS 10000
#define SHADOWMAP_RESOLUTION 2048
#define GLSL_VERSION 330

typedef struct GrassQuad {
    Vector3 position;
    float rotationX;
    float rotationY;
    Vector2 size;
    Color color;
} GrassQuad;

class Renderer {
private:
    // Shaders et rendu
    Shader shadowShader;
    Shader herbe_shader;
    RenderTexture2D shadowMap;
    Camera3D lightCam;
    
    // Herbe
    GrassQuad grass[MAX_GRASS];
    Model model_herbe_instance;
    
    // Textures
    Texture2D noiseTexture;
    Texture2D temperatureTexture;
    Image terrainColorImage;
    
    // Variables d'animation
    float time;
    float windTextureTileSize;
    float windVerticalStrength;
    Vector2 windHorizontalDirection;
    
    // Éclairage
    Vector3 lightDir;
    Color lightColor;
    Vector4 lightColorNormalized;
    
    // Locations des uniforms
    int lightDirLoc, lightColLoc, ambientLoc, lightVPLoc, shadowMapLoc;
    int timeLocation, windStrengthLocation, windSpeedLocation, isGrassLocation;
    int windTextureTileSizeLocation, windVerticalStrengthLocation, windHorizontalDirectionLocation;
    int noiseTextureLoc, noiseScaleLoc;

public:
    Renderer();
    ~Renderer();
    
    void Initialize();
    void Cleanup();
    
    // Gestion de l'herbe
    void InitGrassParticles(Vector3 taille_terrain, Image image_sol);
    void RenderGrass(float timeOfDay, int viewMode);
    
    // Rendu principal
    void RenderShadows(Camera camera, Model& model_sol, std::vector<std::vector<GridCell>>& grille, 
                      std::vector<Nuage>& grandsNuages, Vector3 mapPosition);
    void RenderScene(Camera camera, Image image_sol, Vector3 taille_terrain, Model& model_sol, 
                    std::vector<std::vector<GridCell>>& grille, int viewMode, 
                    int minTemp, int maxTemp, int minHum, int maxHum, Vector3 mapPosition);
    void RenderClouds(std::vector<Nuage>& grandsNuages);
    
    // Mise à jour
    void UpdateLighting(float timeOfDay, std::vector<Biome>& les_biomes);
    void UpdateWind(float windStrength, float windSpeed, float deltaTime);
    
    // Utilitaires
    void DrawGrassQuad(GrassQuad g, Color baseColor, bool useTerrainColor, Image terrainImage, Vector3 taille_terrain);
    Color GetGrassColorFromTime(float timeOfDay);
    Color GetSunColor(float timeOfDay);
};

// Fonctions utilitaires (hors classe)
void InitializeTerrain(Image& image_sol, Model& model_sol, Vector3& taille_terrain);
void UnloadTerrainResources(Image& image_sol);
void HandleViewModeInput(int& viewMode, std::vector<std::vector<GridCell>>& grille, 
                        Model& model_sol, int& minTemp, int& maxTemp, int& minHum, int& maxHum, 
                        int& minPluv, int& maxPluv);

// Fonctions de couleur
Color GetTemperatureColor(int temperature, int minTemp, int maxTemp);
Color GetHumidityColor(int humidity, int minHum, int maxHum);
Color GetPluviometrieColor(int rainfall, int minPluv, int maxPluv);

// Fonction de shadow mapping (à implémenter)
RenderTexture2D LoadShadowmapRenderTexture(int width, int height);
void UnloadShadowmapRenderTexture(RenderTexture2D target);

#endif