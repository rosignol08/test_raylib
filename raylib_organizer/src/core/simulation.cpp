#include "simulation.h"
#include "../utils/math_utils.h"
#include <iostream>

Simulation::Simulation() 
    : viewMode(0), temperature_modifieur(0), hum_modifieur(0),
      minTemp(0), maxTemp(10), minHum(0), maxHum(10),
      couleur_sante(WHITE) {
    
    taille_terrain = { 4, 2, 4 };
    mapPosition = { -2.0f, 0.0f, -2.0f };
}

Simulation::~Simulation() {
    if (image_sol.data != NULL) {
        UnloadImage(image_sol);
    }
}

void Simulation::initialize() {
    initializePlants();
    initializeWeather();
    // Note: loadTerrain doit être appelé avant initializeGrid
}

void Simulation::loadTerrain(const char* heightmapPath) {
    image_sol = LoadImage(heightmapPath);
    initializeGrid();
}

void Simulation::initializePlants() {
    // Créer les plantes (depuis main.cpp)
    // Note: vous devrez passer les modèles depuis le renderer ou les charger ici
    Color couleur = WHITE;
    
    // Pour l'instant, on crée avec des modèles vides - à adapter
    Model emptyModel = { 0 }; // Modèle temporaire
    
    Plante buisson("Buisson", 100, 15, 30, 10, 30, 3, 1, 0.05f, 0.1f, 0.01f, 0.5f, 0, false, 1000, emptyModel, couleur);
    Plante accacia("Acacia", 100, 10, 20, 10, 30, 2, 1, 0.005f, 0.1f, 0.0f, 0.5f, 0, false, 1000, emptyModel, couleur);
    Plante sapin("Sapin", 100, 0, 30, -30, 20, 1, 1, 0.005f, 0.1f, 0.0f, 0.3f, 0, false, 1000, emptyModel, couleur);
    
    plantes = {buisson, accacia, sapin};
}

void Simulation::initializeWeather() {
    // Créer les types de météo (depuis main.cpp)
    Meteo meteo_soleil("Soleil", 20, 0, Color{255, 141, 34, 255}, 1.1f, true);
    Meteo meteo_pluie("Pluie", 10, 0, Color{0, 161, 231, 255}, 0.4f, false);
    Meteo meteo_neige("Neige", -10, 0, Color{255, 255, 255, 255}, 0.01f, false);
    
    les_meteo = {meteo_soleil, meteo_pluie, meteo_neige};
}

void Simulation::initializeGrid() {
    // Initialisation de la grille (depuis main.cpp)
    grille.resize(GRID_SIZE, std::vector<GridCell>(GRID_SIZE));
    
    // Plante vide pour initialisation
    Model emptyModel = { 0 }; // Modèle temporaire
    Plante vide("Vide", 100, 0, 0, 0, 0, 0, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0, false, 100, emptyModel, WHITE);
    
    int humidite_moyenne = 0;
    int temperature_moyenne = 0;
    
    for (int x = 0; x < GRID_SIZE; x++) {
        for (int z = 0; z < GRID_SIZE; z++) {
            float posX = x * (taille_terrain.x / GRID_SIZE) - (taille_terrain.x / 2.0f);
            float posZ = z * (taille_terrain.z / GRID_SIZE) - (taille_terrain.z / 2.0f);
            
            // Ajouter une irrégularité aux positions
            float offsetX = MathUtils::randomFloat(-0.1f, 0.1f);
            float offsetZ = MathUtils::randomFloat(-0.1f, 0.1f);
            
            posX += offsetX;
            posZ += offsetZ;
            
            // Obtenir la hauteur du terrain
            float height = MathUtils::getHeightFromTerrain((Vector3){ posX, 0.0f, posZ }, image_sol, taille_terrain);
            
            // Générer température et humidité
            int temperature = (int) MathUtils::randomFloat(0, 20);
            int humidite = (int) MathUtils::randomFloat(0, 30);
            humidite_moyenne += humidite;
            temperature_moyenne += temperature;
            
            // Calcul de la pente
            float tauxPente = MathUtils::calculateSlope((Vector3){ posX, 0.0f, posZ }, image_sol, taille_terrain);
            
            // Initialiser la cellule
            grille[x][z] = GridCell(x*GRID_SIZE+z, (Vector3){ posX, height, posZ }, vide.model, true, false, temperature, humidite, tauxPente, vide);
            grille[x][z].plante.age = rand() % vide.age_max;
            
            // Transformation du modèle
            float taille = grille[x][z].plante.taille;
            Matrix transform = MatrixIdentity();
            transform = MatrixMultiply(transform, MatrixScale(taille, taille, taille));
            float randomRotationX = MathUtils::randomFloat(0.0f, 2.0f * PI);
            randomRotationX = DEG2RAD * randomRotationX;
            transform = MatrixMultiply(transform, MatrixRotateX(randomRotationX));
            grille[x][z].model.transform = transform;
        }
    }
}

void Simulation::update(float deltaTime) {
    updateWeatherEffects();
    updateGrid();
    updateTemperatureHumidityRanges();
}

void Simulation::updateWeatherEffects() {
    // Logique modificateurs météo (depuis main.cpp)
    temperature_modifieur = get_meteo_temperature(cherche_la_meteo_actuelle(les_meteo));
    hum_modifieur = get_meteo_humidite(cherche_la_meteo_actuelle(les_meteo));
    
    // Appliquer temperature_modifieur à toutes les cases
    static int last_temp_modif = 0;
    int temp_modif = (int)temperature_modifieur;

    if (temp_modif != last_temp_modif) {
        int delta = temp_modif - last_temp_modif;
        for (int x = 0; x < GRID_SIZE; x++) {
            for (int z = 0; z < GRID_SIZE; z++) {
                grille[x][z].temperature += delta;
                if (grille[x][z].temperature < minTemp) minTemp = grille[x][z].temperature;
                if (grille[x][z].temperature > maxTemp) maxTemp = grille[x][z].temperature;
            }
        }
        last_temp_modif = temp_modif;
    }

    // Appliquer hum_modifieur
    static int last_hum_modif = 0;
    int hum_modif = (int)hum_modifieur;

    if (hum_modif != last_hum_modif) {
        int delta = hum_modif - last_hum_modif;
        for (int x = 0; x < GRID_SIZE; x++) {
            for (int z = 0; z < GRID_SIZE; z++) {
                grille[x][z].humidite += delta;
                grille[x][z].humidite = Clamp(grille[x][z].humidite, 0, 100);
                
                if (grille[x][z].humidite < minHum) minHum = grille[x][z].humidite;
                if (grille[x][z].humidite > maxHum) maxHum = grille[x][z].humidite;
            }
        }
        last_hum_modif = hum_modif;
    }
}

void Simulation::updateGrid() {
    // Logique vérification plantes (depuis main.cpp)
    Plante plante_morte("Morte", 100, 0, 100, -50, 200, 0, 0, 0.000250f, 0.000250f, 0.0f, 1.0f, 0, true, 50, Model{0}, WHITE);
    Plante vide("Vide", 100, 0, 0, 0, 0, 0, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0, false, 100, Model{0}, WHITE);
    
    for (int x = 0; x < GRID_SIZE; x++) {
        for (int z = 0; z < GRID_SIZE; z++) {
            // Appeler votre fonction verifier_plante ici
            // verifier_plante(grille, &grille[x][z], plantes, plante_morte, vide, minTemp, maxTemp, minHum, maxHum, couleur_sante);
        }
    }
}

void Simulation::updateTemperatureHumidityRanges() {
    // Mettre à jour les plages min/max (depuis dessine_scene)
    minTemp = 0;
    maxTemp = 1;
    for (int x = 0; x < GRID_SIZE; x++) {
        for (int z = 0; z < GRID_SIZE; z++) {
            if (grille[x][z].temperature < minTemp) minTemp = grille[x][z].temperature;
            if (grille[x][z].temperature > maxTemp) maxTemp = grille[x][z].temperature;
            
            grille[x][z].humidite = Clamp(grille[x][z].humidite, 0, 100);
            if (grille[x][z].humidite < minHum) minHum = grille[x][z].humidite;
            if (grille[x][z].humidite > maxHum) maxHum = grille[x][z].humidite;
        }
    }
}

void Simulation::changeWeather(const std::string& weatherName) {
    for (auto& meteo : les_meteo) {
        meteo.meteo_actuelle = (meteo.nom == weatherName);
    }
}