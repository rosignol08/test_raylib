#pragma once
#include "../entities/plante.h"
#include "../environment/meteo.h"
#include <vector>
#include <raylib.h>

#define GRID_SIZE 30

class Simulation {
public:
    Simulation();
    ~Simulation();
    
    void initialize();
    void update(float deltaTime);
    
    // Getters pour le rendu
    const std::vector<std::vector<GridCell>>& getGrid() const { return grille; }
    const std::vector<Plante>& getPlants() const { return plantes; }
    const std::vector<Meteo>& getWeatherTypes() const { return les_meteo; }
    
    // Contrôle du mode de vue
    void setViewMode(int mode) { viewMode = mode; }
    int getViewMode() const { return viewMode; }
    
    // Contrôle météo
    void changeWeather(const std::string& weatherName);
    
    // Plages température/humidité pour visualisation
    int getMinTemp() const { return minTemp; }
    int getMaxTemp() const { return maxTemp; }
    int getMinHum() const { return minHum; }
    int getMaxHum() const { return maxHum; }
    
    // Infos terrain
    Vector3 getTerrainSize() const { return taille_terrain; }
    Image getTerrainImage() const { return image_sol; }
    Vector3 getMapPosition() const { return mapPosition; }

    // Pour le chargement du terrain
    void loadTerrain(const char* heightmapPath);

private:
    // Grille et données de simulation
    std::vector<std::vector<GridCell>> grille;
    std::vector<Plante> plantes;
    std::vector<Meteo> les_meteo;
    
    // Terrain
    Image image_sol;
    Vector3 taille_terrain;
    Vector3 mapPosition;
    
    // Mode de vue
    int viewMode;
    
    // Suivi température/humidité
    int minTemp, maxTemp, minHum, maxHum;
    
    // Modificateurs météo
    float temperature_modifieur;
    float hum_modifieur;
    
    // Couleur santé
    Color couleur_sante;
    
    // Méthodes privées
    void updateGrid();
    void updateWeatherEffects();
    void updateTemperatureHumidityRanges();
    void initializePlants();
    void initializeWeather();
    void initializeGrid();
};